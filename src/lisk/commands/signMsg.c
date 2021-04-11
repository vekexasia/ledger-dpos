//
// Created by andrea on 09/12/18.
//

#include "signMsg.h"
#include "../../io.h"
#include "../../ui_utils.h"
#include "cx.h"
#include "os.h"
#include "../liskutils.h"
#include "../approval.h"

static cx_sha256_t messageHash;
static char message[50];

void handleSignMessagePacket(commPacket_t *packet, commContext_t *context) {
  // if first packet with signing header
  if ( packet->first ) {
    // Set signing context from first packet and patches the .data and .length by removing header length
    uint8_t varint[9];
    uint32_t headersLength = setSignContext(packet);

    // Rest sha256
    cx_sha256_init(&messageHash);

    // Signing header.
    uint64_t prefixLength = strlen(SIGNED_MESSAGE_PREFIX);
    uint8_t varintLength = encodeVarInt(prefixLength, varint);
    cx_hash(&messageHash.header, 0, varint, varintLength, NULL, 0);
    cx_hash(&messageHash.header, 0, (unsigned char *)SIGNED_MESSAGE_PREFIX, prefixLength, NULL, 0);

    varintLength = encodeVarInt(context->totalAmount - headersLength - 1, varint);
    cx_hash(&messageHash.header, 0, varint, varintLength, NULL, 0);

    prepareMsgLineBuffer(packet); //Enough data here for display purpose
  }

  cx_hash(&messageHash.header, 0, packet->data, packet->length, NULL, 0);

}

void prepareMsgLineBuffer(commPacket_t *packet) {
  os_memset(message, 0, sizeof(message));
  uint8_t msgDisplayLenth = MIN(sizeof(message), packet->length);
  os_memmove(message, packet->data, msgDisplayLenth);
  if (msgDisplayLenth > sizeof(message) - 4) {
    os_memmove(message+(sizeof(message) - 4), "...\0", 4);
  }

  uint8_t npc = 0; //Non Printable Chars Counter
  for (uint8_t i=0; i < msgDisplayLenth; i++) {
    npc += IS_PRINTABLE(message[i]) ?
            0 /* Printable Char */:
            1 /* Non Printable Char */;
  }

  // We rewrite the line buffer to <binary data> in case >= than 40% is non printable or first char is not printable.
  if ((npc*100) / msgDisplayLenth >= 40 || ! IS_PRINTABLE(message[0])) {
    os_memmove(message, "< binary data >\0", 16);
  }
}

UX_STEP_NOCB(
  ux_sign_message_flow_1_step, 
  pnn, 
  {
    &C_nanox_icon_eye,
    "Confirm",
    "message",
  });
UX_STEP_NOCB(
  ux_sign_message_flow_2_step,
  bnnn_paging,
  {
    "Message",
    message,
  });
UX_STEP_NOCB_INIT(
  ux_sign_message_flow_3_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    uint64_t address = deriveAddressFromPublic(&signContext.publicKey);
    deriveAddressStringRepresentation(address, lineBuffer);
  },
  {
    "Sign with",
    lineBuffer,
  });
UX_STEP_CB(
  ux_sign_message_flow_4_step,
  pb,
  touch_approve(),
  {
    &C_nanox_icon_validate_14,
    "Confirm",
  });
UX_STEP_CB(
  ux_sign_message_flow_5_step,
  pb,
  touch_deny(),
  {
    &C_nanox_icon_crossmark,
    "Reject",
  });
UX_FLOW(ux_sign_message_flow,
  &ux_sign_message_flow_1_step,
  &ux_sign_message_flow_2_step,
  &ux_sign_message_flow_3_step,
  &ux_sign_message_flow_4_step,
  &ux_sign_message_flow_5_step);

static void ui_display_sign_message(void) {
  ux_flow_init(0, ux_sign_message_flow, NULL);
}

void processSignMessage(volatile unsigned int *flags) {
  uint8_t preFinalHash[sizeof(signContext.digest)];
  uint8_t finalHash[sizeof(signContext.digest)];

  // Close first sha256
  cx_hash(&messageHash.header, CX_LAST, NULL, 0, preFinalHash, sizeof(preFinalHash));

  // Second sha256
  cx_hash_sha256(preFinalHash, sizeof(preFinalHash), finalHash, sizeof(finalHash));
  os_memmove(signContext.digest, finalHash, sizeof(finalHash));

  // Init user flow.
  *flags |= IO_ASYNCH_REPLY;
  ui_display_sign_message();
}
