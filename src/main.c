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
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <bagl.h>
#include "os.h"
#include "cx.h"
#include "os_io_seproxyhal.h"
#include "ui_elements_s.h"
#include "io_protocol.h"
#include "dposutils.h"
#include "main.h"
#include "structs.h"
#define INS_COM_START 89
#define INS_COM_CONTINUE 90
#define INS_COM_END 91

#define INS_GET_PUBLIC_KEY 0x04
#define INS_SIGN 0x05
#define INS_SIGN_MSG 0x06
#define INS_ECHO 0x07
#define INS_PING 0x08


static unsigned int current_text_pos; // parsing cursor in the text to display
static unsigned int currentStep;
static unsigned int totalSteps;
static unsigned int text_y;           // current location of the displayed text
short crc; // holds the crc16 of the content.
unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

typedef void (*processor_callback)(signContext_t *ctx, uint8_t step);

processor_callback pcallback;
static bagl_element_t *bagl_ui_sign_tx; // Real holder of the ui array.

enum UI_STATE {
    UI_IDLE, UI_TEXT, UI_APPROVAL
};

enum UI_STATE uiState;


ux_state_t ux;

#define MAX_BIP32_PATH 10







static void ui_text(void) {
  current_text_pos = 0;
  uiState = UI_TEXT;
#ifdef TARGET_BLUE
  UX_DISPLAY(bagl_ui_text, NULL);
#else
  UX_DISPLAY(bagl_ui_text_review_nanos, NULL);
#endif
}

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


static void ui_signtx(uint8_t steps, uint8_t uielements) {
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

/**/

bagl_element_t *io_seproxyhal_touch_deny(const bagl_element_t *e) {
  G_io_apdu_buffer[0] = 0x69;
  G_io_apdu_buffer[1] = 0x85;
  nullifyContext();
  // Send back the response, do not restart the event loop
  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
  // Display back the original UX
  ui_idle();
  return 0; // do not redraw the widget
}

// unsigned int io_seproxyhal_touch_exit(const bagl_element_t *e) {
static bagl_element_t *io_seproxyhal_touch_exit(const bagl_element_t *e) {
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


static void ui_approval(void) {
  uiState = UI_APPROVAL;
  deriveAddressStringRepresentation(signContext.sourceAddress, lineBuffer);


#ifdef TARGET_BLUE
  UX_DISPLAY(bagl_ui_approval_blue, NULL);
#else
  UX_DISPLAY(bagl_ui_approval_nanos, NULL);
#endif
}


static void ui_idle(void) {
  uiState = UI_IDLE;
  UX_MENU_DISPLAY(0, menu_main, NULL);
}




static bagl_element_t * io_seproxyhal_touch_approve(const bagl_element_t *e) {
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
 * Signs an arbitrary message given the privateKey and the info
 * @param privateKey the privateKey to be used
 * @param whatToSign the message to sign
 * @param length the length of the message ot sign
 * @param isTx wether we're signing a tx or a text
 * @param output
 */
void sign(cx_ecfp_private_key_t *privateKey, void *whatToSign, uint32_t length, unsigned char *output, bool isTx) {
  if (isTx == true) {
    uint8_t hash[32];
    cx_hash_sha256(whatToSign, length, hash);
    cx_eddsa_sign(privateKey, NULL, CX_SHA512, hash, 32, NULL, 0, output, 0);
  } else {
    cx_eddsa_sign(privateKey, NULL, CX_SHA512, whatToSign, length, NULL, 0, output, 0);
  }

}

/**
 *
 * @param bip32DataBuffer
 * @param privateKey
 * @param publicKey
 * @return read data to derive private public
 */
uint32_t
derivePrivatePublic(uint8_t *bip32DataBuffer, cx_ecfp_private_key_t *privateKey, cx_ecfp_public_key_t *publicKey) {
  uint8_t bip32PathLength = bip32DataBuffer[0];
  uint32_t i;
  uint32_t bip32Path[MAX_BIP32_PATH];
  uint8_t privateKeyData[33];

  uint32_t readData = 1; // 1byte length
  bip32DataBuffer += 1;

  if ((bip32PathLength < 0x01) || (bip32PathLength > MAX_BIP32_PATH)) {
    THROW(0x6a80);
  }


  for (i = 0; i < bip32PathLength; i++) {
    bip32Path[i] = (bip32DataBuffer[0] << 24) | (bip32DataBuffer[1] << 16) |
                   (bip32DataBuffer[2] << 8) | (bip32DataBuffer[3]);
    bip32DataBuffer += 4;
    readData += 4;
  }
  os_perso_derive_node_bip32(CX_CURVE_Ed25519, bip32Path, bip32PathLength,
                             privateKeyData,
                             NULL /* CHAIN CODE */);

  cx_ecfp_init_private_key(CX_CURVE_Ed25519, privateKeyData, 32, privateKey);

  cx_ecfp_generate_pair(CX_CURVE_Ed25519, publicKey, privateKey, 1);

  // Clean up!
  os_memset(privateKeyData, 0, sizeof(privateKeyData));

  return readData;
}

/**
 * Handle publicKey request given the bip32Db
 * @param bip32DataBuffer
 * @param tx
 */
void handleGetPublic(uint8_t *bip32DataBuffer) {
  cx_ecfp_private_key_t privateKey;
  cx_ecfp_public_key_t publicKey;

  derivePrivatePublic(bip32DataBuffer, &privateKey, &publicKey);
  getEncodedPublicKey(&publicKey, rawData);
  os_memset(&privateKey, 0, sizeof(privateKey));

  initResponse();
  addToResponse(rawData, 32);
  uint64_t address = deriveAddressFromPublic(&publicKey);
  uint8_t length = deriveAddressStringRepresentation(address, (char *) (rawData + 32));
  addToResponse(rawData+32, length);
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
  bytesRead += 2;
  whereTo->hasRequesterPublicKey = *(dataBuffer + bytesRead);
  bytesRead++;

  // REad message
  whereTo->msg = dataBuffer + bytesRead;
//  os_memmove(whereTo->msg, dataBuffer + bytesRead, whereTo->msgLength);
//  whereTo.msg[whereTo.msgLength] = '\0';

  whereTo->sourceAddress = deriveAddressFromPublic(&whereTo->publicKey);

  deriveAddressStringRepresentation(whereTo->sourceAddress, whereTo->sourceAddressStr);

}

void nullifyContext() {
  os_memset(&signContext.privateKey, 0, sizeof(signContext.privateKey));
}


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
  }
}


void handleStartCommPacket() {
  if (commContext.started) {
    THROW(0x6D00);
//    return;
  }
  commContext.started = true;
  commContext.read = 0;
  commContext.isDataInNVRAM = false; // For now.

  uint16_t totalAmountOfData = G_io_apdu_buffer[5] << 8;
  totalAmountOfData += G_io_apdu_buffer[6];

  // Debug
  initResponse();
  os_memmove(rawData, &totalAmountOfData, 2);
  addToResponse(rawData, 2);

  if (totalAmountOfData > 800) {
    commContext.isDataInNVRAM = true;
    commContext.data = PIC(&N_rawData);
  } else {
    commContext.data = rawData;
  }
//  commContext.data = rawData;
}
void handleCommPacket() {
  if (commContext.isDataInNVRAM) {
    nvm_write(commContext.data + commContext.read, G_io_apdu_buffer + 5, G_io_apdu_buffer[4]);
  } else {
    os_memmove(commContext.data + commContext.read, G_io_apdu_buffer + 5, G_io_apdu_buffer[4]);
  }

  initResponse();
  commContext.read += G_io_apdu_buffer[4];
  crc = cx_crc16(commContext.data, commContext.read);
  addToResponse(&crc, 2);
}



void processCommPacket(volatile unsigned int *flags) {

  switch(commContext.data[0]) {
    case INS_PING:
      initResponse();
      char * pong = "PONG";
      addToResponse(pong, 4);
      break;
    case INS_GET_PUBLIC_KEY:
      handleGetPublic(commContext.data + 1);
      break;
    case INS_SIGN_MSG:
      getSignContext(commContext.data + 1, &signContext);
      signContext.isTx = false;
      os_memset(lineBuffer, 0, 50);
      os_memmove(lineBuffer, signContext.msg, MIN(50, signContext.msgLength));
      *flags |= IO_ASYNCH_REPLY;
      ui_text();
      break;
    case INS_SIGN:
      handleSignTX(commContext.data + 1);
      *flags |= IO_ASYNCH_REPLY;
      break;

    default:
      THROW(0x6D00);
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



static void lisk_main2(void) {
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
//                // Handle Input?
//                if (G_io_apdu_buffer[0] != 0x80) {
//                    THROW(0x6E00);
//                }

            // unauthenticated instruction
            switch (G_io_apdu_buffer[1]) {
              case 0x00: // reset
                flags |= IO_RESET_AFTER_REPLIED;
                THROW(0x9000);
                break;

              case 0x01: // case 1
                THROW(0x9000);
                break;
              case INS_PING:
                initResponse();
                char * pong = "PONG";
                addToResponse(pong, 4);
                tx = flushResponseToIO(G_io_apdu_buffer);
                THROW(0x9000);
              case INS_GET_PUBLIC_KEY:
                handleGetPublic(G_io_apdu_buffer + 2);
                tx = flushResponseToIO(G_io_apdu_buffer);
                THROW(0x9000);
                break;
              case INS_SIGN:
                handleSignTX(G_io_apdu_buffer + 2);
                flags |= IO_ASYNCH_REPLY;

                break;

              case INS_SIGN_MSG:
//                                handleSignMSG(G_io_apdu_buffer+2, &flags, &tx);
                getSignContext(G_io_apdu_buffer + 2, &signContext);
                os_memset(lineBuffer, 0, 50);
                os_memmove(lineBuffer, signContext.msg, MIN(50, signContext.msgLength));
                flags |= IO_ASYNCH_REPLY;

//                ui_text();
                io_seproxyhal_touch_approve(NULL);

//                                THROW(0x9000);
                break;

//                            case INS_GET_PUBLIC_KEY:
//                                __test(G_io_apdu_buffer[OFFSET_P1],
//                                                   G_io_apdu_buffer[OFFSET_P2],
//                                                   G_io_apdu_buffer + OFFSET_CDATA, &flags, &tx);
//                                THROW(0x9000);
//                                break;
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
  current_text_pos = 0;
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
