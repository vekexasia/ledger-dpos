//
// Created by andrea on 09/12/18.
//

#include "signMsg.h"
#include "../../../io.h"
#include "../../../ui_utils.h"
#include "cx.h"
#include "os.h"
#include "../dposutils.h"
#include "../approval.h"

const bagl_element_t sign_message_ui[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Verify text", 0x00),
  ICON_CROSS(0x00),
  ICON_CHECK(0x00),
  LINEBUFFER,
 };

static cx_sha256_t messageHash;

void handleSignMessagePacket(commPacket_t *packet, commContext_t *context) {
  // if first packet with signing header
  if ( packet->first ) {
    // Set signing context from first packet and patches the .data and .length by removing header length

    uint8_t varint[9];
    uint8_t varintLength;
    uint64_t tmp;
    uint32_t headersLength = setSignContext(packet);

    // Rest sha256
    cx_sha256_init(&messageHash);

    // Signing header.

    tmp = strlen(SIGNED_MESSAGE_PREFIX);

    varintLength = encodeVarInt(tmp, varint);
//      PRINTF("VarInt %.*H", varintLength, varint);
    cx_hash(&messageHash, 0, varint, varintLength, NULL, 0);
    cx_hash(&messageHash, 0, SIGNED_MESSAGE_PREFIX, strlen(SIGNED_MESSAGE_PREFIX), NULL, 0);

    varintLength = encodeVarInt(context->totalAmount - headersLength - 1, varint);
//    PRINTF("VarInt %.*H", varintLength, varint);
//    PRINTF("Total Amount %d", context->totalAmount - headersLength - 1);
    cx_hash(&messageHash, 0, varint, varintLength, NULL, 0);

    os_memset(lineBuffer, 0, 50);
    os_memmove(lineBuffer, packet->data, MIN(50, packet->length));
//    PRINTF("Signing %s %d", lineBuffer, packet->length);
  }

  cx_hash(&messageHash, 0, packet->data, packet->length, NULL, 0);

}

unsigned int sign_message_ui_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      ui_approval();
      break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
      touch_deny(NULL);
      break;
  }
  return 0;
}


void processSignMessage(volatile unsigned int *flags) {
  uint8_t preFinalHash[32];
  uint8_t finalHash[32];

  // Close first sha256
  cx_hash(&messageHash, CX_LAST, NULL, 0, preFinalHash, 32);

  // Second sha256
  cx_hash_sha256(preFinalHash, 32, finalHash, 32);
//  PRINTF("HASH is: %.*h\n", 32, preFinalHash);
//  PRINTF("2. HASH is: %.*h\n", 32, finalHash);

  os_memmove(signContext.digest, finalHash, 32);
  // Init user flow.
  *flags |= IO_ASYNCH_REPLY;
  UX_DISPLAY(sign_message_ui, NULL);
//  sign(&signContext.privateKey, signContext.digest, 32, signedData);
//  PRINTF("2. SignedData is: %.*h\n", 64, signedData);

//  touch_approve();
}
