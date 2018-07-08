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
#include "ui_elements_s.h"
#include "io_protocol.h"
#include "ed25519.h"
#define INS_COM_START 89
#define INS_COM_CONTINUE 90
#define INS_COM_END 91

#define INS_GET_PUBLIC_KEY 0x04
#define INS_SIGN 0x05
#define INS_SIGN_MSG 0x06
#define INS_PING 0x08
#define INS_VERSION 0x09

#define IS_PRINTABLE(c) ((c >= 0x20 && c <= 0x7e) || (c >= 0x80 && c <= 0xFF))

static unsigned int currentStep;
static unsigned int totalSteps;
static unsigned int text_y;           // current location of the displayed text
short crc; // holds the crc16 of the content.
unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];
typedef void (*processor_callback)(signContext_t *ctx, uint8_t step);
processor_callback pcallback;
static bagl_element_t *bagl_ui_sign_tx; // Real holder of the ui array.
enum UI_STATE {
    UI_IDLE, UI_WARNING, UI_TEXT, UI_ADDRESS_REVIEW, UI_APPROVAL
};

enum UI_STATE uiState;
ux_state_t ux;


/**
 * Sets ui to idle.
 */
void ui_idle() {
  uiState = UI_IDLE;
  UX_MENU_DISPLAY(0, menu_main, NULL);
}

/**
 *
 */
void ui_approval(void) {
  uiState = UI_APPROVAL;
  deriveAddressStringRepresentation(signContext.sourceAddress, lineBuffer);


#ifdef TARGET_BLUE
  UX_DISPLAY(bagl_ui_approval_blue, NULL);
#else
  UX_DISPLAY(bagl_ui_approval_nanos, NULL);
#endif
}

static void ui_text(void) {
  uiState = UI_TEXT;
  UX_DISPLAY(bagl_ui_text_review_nanos, NULL);
}

static void ui_address(void) {
  uiState = UI_ADDRESS_REVIEW;

  uint64_t address = deriveAddressFromPublic(&signContext.publicKey);
  uint8_t length = deriveAddressStringRepresentation(address, lineBuffer);
  lineBuffer[length] = '\0';

  UX_DISPLAY(bagl_ui_address_review_nanos, NULL);
}


/**
 * Used to verify what is going to be displayed
 * @param element
 * @return 0 or 1
 */
const int signprocessor(const bagl_element_t *element) {
  if (element->component.userid == 0x0) {
    return 1;
  }

  if ((element->component.type & (~BAGL_FLAG_TOUCHABLE)) == BAGL_NONE) {
    return 0;
  }
  if (element->component.userid == currentStep) {
    return 1;
  }
  return 0;
}

/**
 * Cleans memory.
 */
void nullifyPrivKeyInContext() {
  os_memset(&signContext.privateKey, 0, sizeof(signContext.privateKey));
}

unsigned int bagl_ui_sign_tx_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      if (currentStep < totalSteps) {
        currentStep++;
        pcallback(&signContext, currentStep);
        UX_REDISPLAY();
      } else {
        io_seproxyhal_touch_approve(NULL);
      }
      break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
      io_seproxyhal_touch_deny(NULL);
      break;
  }
  return 0;
}


void ui_signtx(uint8_t steps, uint8_t uielements) {
  currentStep = 1;
  totalSteps = steps;
  pcallback(&signContext, 1);
  // IMPLEMENT BLUE
  ux.elements = bagl_ui_sign_tx;
  ux.elements_count = uielements;
  ux.button_push_handler = bagl_ui_sign_tx_button;
  ux.elements_preprocessor = signprocessor;
  UX_WAKE_UP();
  UX_REDISPLAY();
}





// ********************************************************************************
// Ledger Nano S specific UI
// ********************************************************************************

bagl_element_t *io_seproxyhal_touch_deny(const bagl_element_t *e) {
  G_io_apdu_buffer[0] = 0x69;
  G_io_apdu_buffer[1] = 0x85;

  // Allow restart of operation
  commContext.started = false;
  commContext.read = 0;

  nullifyPrivKeyInContext();
  // Send back the response, do not restart the event loop
  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
  // Display back the original UX
  ui_idle();
  return 0; // do not redraw the widget
}


unsigned int bagl_ui_approval_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      io_seproxyhal_touch_approve(NULL);
      break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
      io_seproxyhal_touch_deny(NULL);
      break;
  }
  return 0;
}

unsigned int bagl_ui_address_review_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      createPublicKeyResponse();

      unsigned int tx = flushResponseToIO(G_io_apdu_buffer);
      G_io_apdu_buffer[tx]   = 0x90;
      G_io_apdu_buffer[tx+1] = 0x00;

      io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx+2);

      // Display back the original UX
      ui_idle();
      return 0; // do not redraw the widget
      break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
      io_seproxyhal_touch_deny(NULL);
      break;
  }
  return 0;
}

unsigned int bagl_ui_text_review_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      ui_approval();
      break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
      io_seproxyhal_touch_deny(NULL);
      break;
  }
  return 0;
}

/**/

// unsigned int io_seproxyhal_touch_exit(const bagl_element_t *e) {
bagl_element_t *io_seproxyhal_touch_exit(const bagl_element_t *e) {
  // Go back to the dashboard
  os_sched_exit(0);
  return NULL;
}

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




bagl_element_t * io_seproxyhal_touch_approve(const bagl_element_t *e) {
  uint8_t signature[64];

  sign(&signContext.privateKey, signContext.msg, signContext.msgLength, signature, signContext.isTx);

  initResponse();
  addToResponse(signature, 64);

  unsigned int tx = flushResponseToIO(G_io_apdu_buffer);
  G_io_apdu_buffer[tx]   = 0x90;
  G_io_apdu_buffer[tx+1] = 0x00;

  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx+2);

  // Display back the original UX
  ui_idle();
  return 0; // do not redraw the widget
}


/**
 * Handle publicKey request given the bip32Db. It will derive the publickey from the
 * given bip32.
 * @param bip32DataBuffer
 */
void handleGetPublic(uint8_t *bip32DataBuffer) {
  derivePrivatePublic(bip32DataBuffer, &signContext.privateKey, &signContext.publicKey);
  nullifyPrivKeyInContext();
}

/**
 * Creates the response for the getPublicKey command.
 * It returns both publicKey and derived Address
 */
void createPublicKeyResponse() {
  initResponse();
  getEncodedPublicKey(&signContext.publicKey, rawData);
  addToResponse(rawData, 32);
  uint64_t address = deriveAddressFromPublic(&signContext.publicKey);
  uint8_t length = deriveAddressStringRepresentation(address, (char *) (rawData + 32));

  addToResponse(rawData + 32, length);
}

/**
 * Reads the databuffer and sets different data on a signContext which is then returned
 * @param dataBuffer the  buffer to read from.
 * @param whereTo
 * @return the signContext.
 */
void getSignContext(uint8_t *dataBuffer, signContext_t *whereTo) {
  uint32_t bytesRead = derivePrivatePublic(dataBuffer, &(*whereTo).privateKey, &(*whereTo).publicKey);
  whereTo->msgLength = (*(dataBuffer + bytesRead)) << 8;
  whereTo->msgLength += (*(dataBuffer + bytesRead + 1));
  if (whereTo->msgLength >= commContext.totalAmount) {
    THROW(0x6700); // INCORRECT_LENGTH
  }
  bytesRead += 2;
  whereTo->hasRequesterPublicKey = *(dataBuffer + bytesRead);
  bytesRead++;

  whereTo->msg = dataBuffer + bytesRead;

  whereTo->sourceAddress = deriveAddressFromPublic(&whereTo->publicKey);

  deriveAddressStringRepresentation(whereTo->sourceAddress, whereTo->sourceAddressStr);
}

/**
 * Handles the sign transaction command.
 * @param dataBuffer
 */
void handleSignTX(uint8_t *dataBuffer) {
  getSignContext(dataBuffer, &signContext);
  parseTransaction(signContext.msg, signContext.msgLength, signContext.hasRequesterPublicKey, &signContext.tx);
  signContext.isTx = true;

  if (signContext.tx.type == TXTYPE_SEND) {
    pcallback = lineBufferSendTxProcessor;
    bagl_ui_sign_tx = bagl_ui_approval_send_nanos;
    ui_signtx(3, sizeof(bagl_ui_approval_send_nanos)/sizeof(bagl_ui_approval_send_nanos[0]));
  } else if (signContext.tx.type == TXTYPE_REGISTERDELEGATE) {
    pcallback = lineBufferRegDelegateTxProcessor;
    bagl_ui_sign_tx = bagl_ui_regdelegate_nanos;
    ui_signtx(3, sizeof(bagl_ui_regdelegate_nanos)/sizeof(bagl_ui_regdelegate_nanos[0]));
  } else if (signContext.tx.type == TXTYPE_CREATESIGNATURE) {
    pcallback = lineBufferSecondSignProcessor;
    bagl_ui_sign_tx = bagl_ui_secondsign_nanos;
    ui_signtx(3, sizeof(bagl_ui_secondsign_nanos)/sizeof(bagl_ui_secondsign_nanos[0]));
  } else if (signContext.tx.type == TXTYPE_VOTE) {
    pcallback = lineBufferVoteProcessor;
    bagl_ui_sign_tx = bagl_ui_vote_nanos;
    ui_signtx(3, sizeof(bagl_ui_vote_nanos)/sizeof(bagl_ui_vote_nanos[0]));
  } else if (signContext.tx.type == TXTYPE_CREATEMULTISIG) {
    pcallback = lineBufferMultisigProcessor;
    bagl_ui_sign_tx = bagl_ui_multisignature_nanos;
    ui_signtx(4, sizeof(bagl_ui_multisignature_nanos)/sizeof(bagl_ui_multisignature_nanos[0]));
  } else {
    THROW(0x6a80);
  }
}

/**
 * Handles the start communication packet
 */
void handleStartCommPacket() {
  commContext.started = true;
  commContext.read = 0;
  commContext.totalAmount = 0;
  commContext.isDataInNVRAM = false; // For now.

  commContext.totalAmount = G_io_apdu_buffer[5] << 8;
  commContext.totalAmount += G_io_apdu_buffer[6];

  // Debug
  initResponse();
  os_memmove(rawData, &commContext.totalAmount, 2);
  addToResponse(rawData, 2);

  if (commContext.totalAmount > MAX(NVRAM_MAX, 1000)) {
    // We exceed the totalAmount of data possible.
    // we throw
    THROW(0x6a84); // NOT_ENOUGH_MEMORY_SPACE
  }

  if (commContext.totalAmount > 1000) {
    commContext.isDataInNVRAM = true;
    commContext.data = PIC(&N_rawData);
  } else {
    commContext.data = rawData;
  }

}

/**
 * Handles a single communication packet.
 */
void handleCommPacket() {
  if (commContext.started == false) {
    THROW(0x9802); // CODE_NOT_INITIALIZED
  }
  if (commContext.isDataInNVRAM) {
    nvm_write(commContext.data + commContext.read, G_io_apdu_buffer + 5, G_io_apdu_buffer[4]);
  } else {
    os_memmove(commContext.data + commContext.read, G_io_apdu_buffer + 5, G_io_apdu_buffer[4]);
  }

  initResponse();
  commContext.read += G_io_apdu_buffer[4];
  if (commContext.read <= commContext.totalAmount) {
    crc = cx_crc16(commContext.data, commContext.read);
    addToResponse(&crc, 2);
  } else {
    // Somehow the totalAmount of data sent is wrong. hence we set the thing as unstarted.
    commContext.started = false;
    THROW(0x6700); // INCORRECT_LENGTH
  }

}



void processCommPacket(volatile unsigned int *flags) {
  uint8_t tmp, tmp2;
  switch(commContext.data[0]) {
    case INS_VERSION:
      initResponse();
      addToResponse(APPVERSION, 5);
      addToResponse(COINIDSTR, strlen(COINIDSTR));
      break;
    case INS_PING:
      initResponse();
      char * pong = "PONG";
      addToResponse(pong, 4);
      break;
    case INS_GET_PUBLIC_KEY:
      // init response
      tmp = commContext.data[1];
      handleGetPublic(commContext.data + 2);

      if (tmp == true) { // show address?
        // Show on ledger
        *flags |= IO_ASYNCH_REPLY;
        ui_address();
      } else {
        createPublicKeyResponse();
      }


      break;
    case INS_SIGN_MSG:
      getSignContext(commContext.data + 1, &signContext);
      signContext.isTx = false;
      if (os_memcmp(SIGNED_MESSAGE_PREFIX, signContext.msg, strlen(SIGNED_MESSAGE_PREFIX)) != 0) {
        THROW(0x6a80);
      }

      uint8_t realMessageLength = signContext.msgLength - strlen(SIGNED_MESSAGE_PREFIX);
      os_memset(lineBuffer, 0, 50);
      os_memmove(lineBuffer, signContext.msg + strlen(SIGNED_MESSAGE_PREFIX), MIN(50, realMessageLength));

      tmp2 = 0; // Will contain the amount of chars that are non printable in string
      // If first char is non-ascii (binary data). Rewrite the whole message to show it
      for (tmp=0; tmp < MIN(50, realMessageLength); tmp++) {
        tmp2 += IS_PRINTABLE(lineBuffer[tmp]) ?
                0 /* Printable Char */:
                1 /* Non Printable Char */;
      }

      // We rewrite the line buffer to <binary data> in case >= than 40% is non printable or first char is not printable.
      if ((tmp2*100) / MIN(50, realMessageLength) >= 40 || ! IS_PRINTABLE(lineBuffer[0])) {
        // More than 30% of chars are binary. hence we rewrite the message to binary
        os_memmove(lineBuffer, "< binary data >\0", 16);
      }


      *flags |= IO_ASYNCH_REPLY;
      ui_text();
      break;
    case INS_SIGN:
      handleSignTX(commContext.data + 1);
      *flags |= IO_ASYNCH_REPLY;
      break;

    default:
      THROW(0x6a80); // INCORRECT_DATA
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

            switch (G_io_apdu_buffer[1]) {
              case INS_COM_START:
                handleStartCommPacket();
                tx = flushResponseToIO(G_io_apdu_buffer);
                THROW(0x9000);
                break;
              case INS_COM_CONTINUE:
                handleCommPacket();
                tx = flushResponseToIO(G_io_apdu_buffer);
                THROW(0x9000);
                break;
              case INS_COM_END:
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
      if ((uiState == UI_TEXT) &&
          (os_seph_features() &
           SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_SCREEN_BIG)) {
        ui_approval();
      } else {
        UX_DISPLAYED_EVENT();
      }
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
  uiState = UI_IDLE;

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
