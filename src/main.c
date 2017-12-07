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
#include "os.h"
#include "cx.h"
#include "os_io_seproxyhal.h"
#include "ui_elements_s.h"

#include "dposutils.h"

#define INS_GET_PUBLIC_KEY 0x04
#define INS_SIGN 0x05
#define INS_SIGN_MSG 0x06
#define INS_ECHO 0x07


static unsigned int current_text_pos; // parsing cursor in the text to display
static unsigned int text_y;           // current location of the displayed text
static unsigned char hashTainted;     // notification to restart the hash
#define MAX_CHARS_PER_LINE 10

uint8_t deriveAddressShortRepresentation(uint64_t encodedAddress, char *output);

static const bagl_element_t *io_seproxyhal_touch_exit(const bagl_element_t *e);

static const bagl_element_t *io_seproxyhal_touch_approve(const bagl_element_t *e);

static const bagl_element_t *io_seproxyhal_touch_deny(const bagl_element_t *e);

static void sign(cx_ecfp_private_key_t *privateKey, void *whatToSign, uint32_t length, unsigned char *output);

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

static void ui_idle(void);

static unsigned char display_text_part(void);

static void ui_text(void);

static void ui_approval(void);


static const bagl_element_t *io_seproxyhal_touch_exit(const bagl_element_t *e);



typedef struct signContext_t {
    cx_ecfp_private_key_t privateKey;
    cx_ecfp_public_key_t publicKey;
    uint16_t msgLength;
    uint8_t msg[500];
    bool hasRequesterPublicKey;
    uint64_t sourceAddress;
    char sourceAddressStr[22 + ADDRESS_SUFFIX_LENGTH + 1];
} signContext_t;

/**
 * Contains a sign context with nullified private key.
 */
signContext_t signContext;

ux_state_t ux;

// ********************************************************************************
// Ledger Nano S specific UI
// ********************************************************************************

// APpENA AGGIUNTO
static unsigned int
bagl_ui_approval_nanos_button(unsigned int button_mask,
                              unsigned int button_mask_counter) {
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



static const bagl_element_t *io_seproxyhal_touch_deny(const bagl_element_t *e) {
  hashTainted = 1;
  G_io_apdu_buffer[0] = 0x69;
  G_io_apdu_buffer[1] = 0x85;
  // Send back the response, do not restart the event loop
  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
  // Display back the original UX
  ui_idle();
  return 0; // do not redraw the widget
}

// unsigned int io_seproxyhal_touch_exit(const bagl_element_t *e) {
static const bagl_element_t *io_seproxyhal_touch_exit(const bagl_element_t *e) {
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

enum UI_STATE {
    UI_IDLE, UI_TEXT, UI_APPROVAL
};

enum UI_STATE uiState;


ux_state_t ux;

static unsigned int
bagl_ui_text_review_nanos_button(unsigned int button_mask,
                                 unsigned int button_mask_counter) {
  switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      if (!display_text_part()) {
        ui_approval();
      } else {
        UX_REDISPLAY();
      }
      break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
      io_seproxyhal_touch_deny(NULL);
      break;
  }
  return 0;
}


static void ui_approval(void) {
  uiState = UI_APPROVAL;
  deriveAddressShortRepresentation(signContext.sourceAddress, lineBuffer);


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


#define MAX_BIP32_PATH 10
#define P1_CONFIRM 0x01
#define P1_NON_CONFIRM 0x00
#define P2_SECP256K1 0x40
#define P2_ED25519 0x80
#define P2_NO_CHAINCODE 0x00
#define P2_CHAINCODE 0x01
#define MAX_RAW_TX 300
#define OFFSET_P1 2
#define OFFSET_P2 3
#define OFFSET_LC 4

#define OFFSET_CDATA 5

#define TXTYPE_SEND 0
#define TXTYPE_CREATESIGNATURE 1
#define TXTYPE_REGISTERDELEGATE 2
#define TXTYPE_VOTE 3
#define TXTYPE_CREATEMULTISIG 4

typedef struct transaction {
    uint8_t type;
    uint64_t recipientId;
    uint64_t amountSatoshi;
};



struct response {
    uint8_t n;
    uint8_t *what[8];
    uint16_t whatLength[8];
} response;


void addToResponse(void *what, uint16_t length) {
  response.what[response.n] = what;
  response.whatLength[response.n] = length;
  response.n = response.n + 1;
}

void initResponse() {
  response.n = 0;
}


unsigned int flushResponseToIO(uint8_t offset) {
  // Write how many infos toWrite
  os_memmove(G_io_apdu_buffer + offset, &(response.n), 1);
  unsigned int total = 1;
  uint8_t i = 0;
  for (i = 0; i < response.n; i++) {
    // Write length.
    os_memmove(G_io_apdu_buffer  + offset + total, &response.whatLength[i], 2);
    total += 2;
    // Write data
    os_memmove(G_io_apdu_buffer  + offset + total, response.what[i], response.whatLength[i]);
    total += response.whatLength[i];
  }
//     Reset.
  initResponse();

  return total;
}




static const bagl_element_t * io_seproxyhal_touch_approve(const bagl_element_t *e) {
  uint8_t status[2] = {0x90, 0x00};
  uint8_t signature[64];
  sign(&signContext.privateKey, signContext.msg, signContext.msgLength, signature);

  initResponse();
  addToResponse(signature, 64);
  addToResponse(status, 2);

  unsigned int tx = flushResponseToIO(0);
  // Status
  G_io_apdu_buffer[tx] = 0x90;
  G_io_apdu_buffer[tx+1] = 0x00;

  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx+2);

  // Display back the original UX
  ui_idle();
  return 0; // do not redraw the widget
}




void parseTransaction(uint8_t *txBytes, uint32_t length, bool hasRequesterPublicKey, struct transaction *out) {
  out->type = txBytes[0];
  uint32_t recIndex = 1 /*type*/
                      + 4 /*timestamp*/
                      + 32 /*senderPublicKey */
                      + (hasRequesterPublicKey == true ? 32 : 0) /*requesterPublicKey */;
  out->recipientId = deriveAddressFromUintArray(&txBytes[recIndex], false);
  uint8_t i = 0;
  out->amountSatoshi = 0;
  for (i = 0; i < 8; i++) {
    out->amountSatoshi += txBytes[recIndex + 8 + i] << (i);
  }

}


/**
 * Signs an arbitrary message given the privateKey and the info
 * @param privateKey the privateKey to be used
 * @param whatToSign the message to sign
 * @param length the length of the message ot sign
 * @param output
 */
void sign(cx_ecfp_private_key_t *privateKey, void *whatToSign, uint32_t length, unsigned char *output) {
  cx_eddsa_sign(privateKey, NULL, CX_LAST, CX_SHA512, whatToSign, length, output);
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
void handleGetPublic(uint8_t *bip32DataBuffer, volatile unsigned int *tx) {
  cx_ecfp_private_key_t privateKey;
  cx_ecfp_public_key_t publicKey;
  uint8_t encodedPkey[32];

  derivePrivatePublic(bip32DataBuffer, &privateKey, &publicKey);
  getEncodedPublicKey(&publicKey, encodedPkey);
  os_memset(&privateKey, 0, sizeof(privateKey));

  initResponse();
  addToResponse(encodedPkey, 32);
  *tx = flushResponseToIO(0);
}

/**
 * Reads the databuffer and sets different data on a signContext which is then returned
 * @param dataBuffer the  buffer to read from.
 * @param whereTo
 * @return the signContext.
 */
void getSignContext(uint8_t *dataBuffer, signContext_t *whereTo) {
  uint32_t bytesRead = derivePrivatePublic(dataBuffer, &(*whereTo).privateKey, &(*whereTo).publicKey);
  (*whereTo).msgLength = (*(dataBuffer + bytesRead)) << 8;
  (*whereTo).msgLength += (*(dataBuffer + bytesRead + 1));
  bytesRead += 2;
  (*whereTo).hasRequesterPublicKey = *(dataBuffer + bytesRead);
  bytesRead++;

  // REad message

  os_memmove((*whereTo).msg, dataBuffer + bytesRead, (*whereTo).msgLength);
//  whereTo.msg[whereTo.msgLength] = '\0';

  (*whereTo).sourceAddress = deriveAddressFromPublic(&(*whereTo).publicKey);

  deriveAddressStringRepresentation((*whereTo).sourceAddress, (*whereTo).sourceAddressStr);
}

void calcNullifiedSignContext(uint8_t *dataBuffer) {
  getSignContext(dataBuffer, &signContext);

  os_memset(&signContext.privateKey, 0, sizeof(signContext.privateKey));
}

/**
 * Handles the signing of a message
 * @param dataBuffer
 * @param flags
 * @param tx
 */
void handleSignMSG(uint8_t *dataBuffer, volatile unsigned int *flags, volatile unsigned int *tx) {

  getSignContext(dataBuffer, &signContext);

  unsigned char signature[64];
  unsigned char msg[signContext.msgLength];
  os_memmove(msg, signContext.msg, signContext.msgLength);
  sign(&signContext.privateKey, msg, signContext.msgLength, signature);

  // Clean memory!
  os_memset(&signContext.privateKey, 0, sizeof(signContext.privateKey));

  initResponse();
  addToResponse(signature, 64);
//  addToResponse(signContext.sourceAddressStr, strlen(signContext.sourceAddressStr));
  *tx = flushResponseToIO(0);
}

void handleSignTX(uint8_t *dataBuffer, volatile unsigned int *flags, volatile unsigned int *tx) {
  getSignContext(dataBuffer, &signContext);

  unsigned char signature[64];

  struct transaction txOut;

  parseTransaction(signContext.msg, signContext.msgLength, signContext.hasRequesterPublicKey, &txOut);


  unsigned char data[signContext.msgLength];
  os_memmove(data, signContext.msg, signContext.msgLength);
  sign(&signContext.privateKey, data, signContext.msgLength, signature);

  // Clean memory!
  os_memset(&signContext.privateKey, 0, sizeof(signContext.privateKey));

  if (txOut.type == TXTYPE_SEND || 1) {
    initResponse();
    addToResponse(signature, 64);
    *tx = flushResponseToIO(0);
  } else {
    initResponse();
    addToResponse(&txOut.type, 1);
    addToResponse(&txOut.amountSatoshi, 8);
    addToResponse(&txOut.recipientId, 8);
    *tx = flushResponseToIO(0);
  }

}
static void lisk_main(void) {
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
              case INS_ECHO:
                getSignContext(G_io_apdu_buffer + 2, &signContext);
                initResponse();
                addToResponse(&signContext.msgLength, 2);
                addToResponse("ciao", 4);
                addToResponse(&signContext.sourceAddress, 8);
                addToResponse(signContext.msg, signContext.msgLength);
                addToResponse(signContext.sourceAddressStr, 24);
                tx = flushResponseToIO(0);
                THROW(0x9000);
                break;
              case INS_GET_PUBLIC_KEY: // echo
//                                tx = rx;
                handleGetPublic(G_io_apdu_buffer + 2, &tx);
                THROW(0x9000);
                break;
              case INS_SIGN:
                handleSignTX(G_io_apdu_buffer + 2, &flags, &tx);
                THROW(0x9000);
                break;

              case INS_SIGN_MSG:
//                                handleSignMSG(G_io_apdu_buffer+2, &flags, &tx);
                getSignContext(G_io_apdu_buffer + 2, &signContext);
                lineBufferLength = (uint8_t) (signContext.msgLength > 50 ? 50 : signContext.msgLength);
                os_memmove(linesBuffer, signContext.msg, lineBufferLength);
                flags |= IO_ASYNCH_REPLY;

                ui_text();

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
        if (!display_text_part()) {
          ui_approval();
        } else {
          UX_REDISPLAY();

        }
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


// Pick the text elements to display
static unsigned char display_text_part() {

  int charsToDisplay = MIN(lineBufferLength - current_text_pos,  MAX_CHARS_PER_LINE);
  if (charsToDisplay <=0) {
    return 0;
  }
  os_memmove(lineBuffer, linesBuffer+current_text_pos, charsToDisplay);
  lineBuffer[charsToDisplay] = '\0';
  current_text_pos += charsToDisplay;
#ifdef TARGET_BLUE
  os_memset(bagl_ui_text, 0, sizeof(bagl_ui_text));
  bagl_ui_text[0].component.type = BAGL_LABEL;
  bagl_ui_text[0].component.x = 4;
  bagl_ui_text[0].component.y = text_y;
  bagl_ui_text[0].component.width = 320;
  bagl_ui_text[0].component.height = TEXT_HEIGHT;
  // element.component.fill = BAGL_FILL;
  bagl_ui_text[0].component.fgcolor = 0x000000;
  bagl_ui_text[0].component.bgcolor = 0xf9f9f9;
  bagl_ui_text[0].component.font_id = DEFAULT_FONT;
  bagl_ui_text[0].text = lineBuffer;
  text_y += TEXT_HEIGHT + TEXT_SPACE;
#endif
  return 1;
}

static void ui_text(void) {
  current_text_pos = 0;
  uiState = UI_TEXT;
  display_text_part();
#ifdef TARGET_BLUE
  UX_DISPLAY(bagl_ui_text, NULL);
#else
  UX_DISPLAY(bagl_ui_text_review_nanos, NULL);
#endif
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

  BEGIN_TRY
    {
      TRY
        {
          io_seproxyhal_init();

          // Consider using an internal storage thingy here

          USB_power(0);
          USB_power(1);

          ui_idle();

          lisk_main();
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
