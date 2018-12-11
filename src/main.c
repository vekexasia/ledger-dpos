/*******************************************************************************
*   Ledger Blue
*   (c) 2016 Ledger
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include <stdbool.h>
#include <inttypes.h>
#include <bagl.h>
#include <memory.h>
#include "os.h"
#include "main.h"
//#include "coins/rise-lisk/ui_elements_s.h"
#include "io.h"
#include "coins/rise-lisk/impl.h"
#define INS_COM_START 89
#define INS_COM_CONTINUE 90
#define INS_COM_END 91

#define INS_PING 0x08
#define INS_VERSION 0x09


static unsigned int text_y;           // current location of the displayed text
short crc; // holds the crc16 of the content.
short prevCRC; // holds the crc16 of the prevpacket for comm layer.
unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];


// Don't need to change?
unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
  switch (channel & ~(IO_FLAGS)) {
    case CHANNEL_KEYBOARD:
      break;

      // multiplexed io exchange over a SPI channel and TLV encapsulated protocol
    case CHANNEL_SPI:
      if (tx_len) {
        io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

        if (channel & IO_RESET_AFTER_REPLIED) {
          reset();
        }
        return 0; // nothing received from the master so far (it's a tx
        // transaction)
      } else {
        return io_seproxyhal_spi_recv(G_io_apdu_buffer,
                                      sizeof(G_io_apdu_buffer), 0);
      }

    default:
      THROW(INVALID_PARAMETER);
  }
  return 0;
}


/**
 * Handles the start communication packet
 */
void handleStartCommPacket() {
  commContext.started = true;
  commContext.read = 0;
  commContext.crc16 = 0;
  commContext.totalAmount = 0;

  commContext.totalAmount = G_io_apdu_buffer[5] << 8;
  commContext.totalAmount += G_io_apdu_buffer[6];

  prevCRC = 0;
  initResponse();
  addToResponse(&commContext.totalAmount, 2);
}

/**
 * Handles a single communication packet.
 */
void handleCommPacket() {
  if (commContext.started == false) {
    THROW(0x9802); // CODE_NOT_INITIALIZED
  }

  if (commContext.read == 0) {
    // IF first packet we read command and strip it away from the data packet
    commContext.command = G_io_apdu_buffer[5];
    os_memmove(commPacket.data, G_io_apdu_buffer + 6, G_io_apdu_buffer[4] - 1);
    commPacket.length = G_io_apdu_buffer[4] - 1;
    commPacket.first = true;
  } else {
    os_memmove(commPacket.data, G_io_apdu_buffer + 5, G_io_apdu_buffer[4]);
    commPacket.length = G_io_apdu_buffer[4];
    commPacket.first = false;
  }

  PRINTF("handleCompacket\n");
  initResponse();
  commContext.read += G_io_apdu_buffer[4];

  if (commContext.read <= commContext.totalAmount) {
    PRINTF("innerHandleComPacket\n");
    // Allow real implementation to handle current comm Packet. (and possibly throw if error occurred)
    innerHandleCommPacket(&commPacket, &commContext);
    PRINTF("POST innerHandleComPacket\n");

    // Compute current crc and replace it with the prevOne.
    crc = cx_crc16(G_io_apdu_buffer + 5, G_io_apdu_buffer[4]);
    PRINTF("postCRC %d %d\n", crc, prevCRC);
    prevCRC = commContext.crc16;
    commContext.crc16 = crc;
    addToResponse(&crc, 2);
    addToResponse(&prevCRC, 2);
    PRINTF("Add to response\n");
  } else {
    // Somehow the totalAmount of data sent is wrong. hence we set the thing as unstarted.
    commContext.started = false;
    THROW(0x6700); // INCORRECT_LENGTH
  }

}



void processCommPacket(volatile unsigned int *flags) {
  PRINTF("Compacket data:\n %.*H \n\n", commPacket.length, commPacket.data);

  if (commContext.command == INS_VERSION) {
    initResponse();
    addToResponse(APPVERSION, 5);
    addToResponse(COINIDSTR, strlen(COINIDSTR));
  } else if (commContext.command == INS_PING) {
    initResponse();
    char * pong = "PONG";
    addToResponse(pong, 4);
  } else {
    innerProcessCommPacket(flags, &commPacket, &commContext);
  }

  commContext.started = false;
  commContext.read = 0;
}

static void dpos_main(void) {
  volatile unsigned int rx = 0;
  volatile unsigned int tx = 0;
  volatile unsigned int flags = 0;
  // DESIGN NOTE: the bootloader ignores the way APDU are fetched. The only
  // goal is to retrieve APDU.
  // When APDU are to be fetched from multiple IOs, like NFC+USB+BLE, make
  // sure the io_event is called with a
  // switch event, before the apdu is replied to the bootloader. This avoid
  // APDU injection faults.
  for (;;) {
    volatile unsigned short sw = 0;

    BEGIN_TRY
      {
        TRY
          {
            rx = tx;
            tx = 0; // ensure no race in catch_other if io_exchange throws
            // an error
            rx = io_exchange(CHANNEL_APDU | flags, rx);
            flags = 0;

            // no apdu received, well, reset the session, and reset the
            // bootloader configuration
            if (rx == 0) {
              THROW(0x6982);
            }

            PRINTF("\n\n## comm incoming %d ##\n\n", G_io_apdu_buffer[1]);
            switch (G_io_apdu_buffer[1]) {
              case INS_COM_START:
                PRINTF("start\n");
                handleStartCommPacket();
                tx = flushResponseToIO(G_io_apdu_buffer);
                THROW(0x9000);
                break;
              case INS_COM_CONTINUE:
                PRINTF("continue\n");
                handleCommPacket();
                PRINTF("after handleComPacket\n");
                tx = flushResponseToIO(G_io_apdu_buffer);
                PRINTF("flush\n");
                THROW(0x9000);
                break;
              case INS_COM_END:
                PRINTF("end\n");
                processCommPacket(&flags);
                tx = flushResponseToIO(G_io_apdu_buffer);
                THROW(0x9000);
                break;
              case 0xFF: // return to dashboard
                goto return_to_dashboard;

              default:
                THROW(0x6D00);
                break;
            }
          }
        CATCH_OTHER(e)
          {
            switch (e & 0xF000) {
              case 0x6000:
              case 0x9000:
                sw = e;
                break;
              default:
                sw = 0x6800 | (e & 0x7FF);
                break;
            }
            // Unexpected exception => report
            G_io_apdu_buffer[tx] = sw >> 8;
            G_io_apdu_buffer[tx + 1] = sw;
            tx += 2;
          }
        FINALLY
        {
        }
      }
    END_TRY;
  }

  return_to_dashboard:
  return;
}


void io_seproxyhal_display(const bagl_element_t *element) {
  io_seproxyhal_display_default((bagl_element_t *) element);
}

unsigned char io_event(unsigned char channel) {
  // nothing done with the event, throw an error on the transport layer if
  // needed

  // can't have more than one tag in the reply, not supported yet.
  switch (G_io_seproxyhal_spi_buffer[0]) {
    case SEPROXYHAL_TAG_FINGER_EVENT:
    UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
      break;

    case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT: // for Nano S
    UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
      break;

    case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
//      if ((uiState == UI_TEXT) &&
//          (os_seph_features() &
//           SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_SCREEN_BIG)) {
//        ui_approval();
//      } else {
//        UX_DISPLAYED_EVENT();
//      }
      UX_DISPLAYED_EVENT();
      break;

      // unknown events are acknowledged
    default:
    UX_DEFAULT_EVENT();
      break;
  }

  // close the event if not done previously (by a display or whatever)
  if (!io_seproxyhal_spi_is_status_sent()) {
    io_seproxyhal_general_status();
  }

  // command has been processed, DO NOT reset the current APDU transport
  return 1;
}



__attribute__((section(".boot"))) int main(void) {
  // exit critical section
  __asm volatile("cpsie i");

  UX_INIT();
  // Set ui state to idle.

  // ensure exception will work as planned
  os_boot();
  commContext.read = 0;
  commContext.started = false;

  BEGIN_TRY
    {
      TRY
        {
          io_seproxyhal_init();

          // Consider using an internal storage thingy here

          USB_power(0);
          USB_power(1);

          ui_idle();

          dpos_main();
        }
      CATCH_OTHER(e)
        {
        }
      FINALLY
      {
      }
    }
  END_TRY;
}
