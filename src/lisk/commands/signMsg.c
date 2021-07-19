//
// Created by andrea on 09/12/18.
//

#include "signMsg.h"

#include "../lisk_approval.h"
#include "../lisk_internals.h"
#include "../lisk_utils.h"
#include "cx.h"
#include "os.h"
#include "txs/common_parser.h"

void handleSignMessagePacket(commPacket_t *packet, commContext_t *context) {

  uint32_t headerBytesRead = 0;

  // if first packet with signing header
  if ( packet->first ) {
    // Reset sha256 and context
    os_memset(&reqContext, 0, sizeof(reqContext));
    os_memset(&txContext, 0, sizeof(txContext));
    cx_sha256_init(&txContext.txHash);
    txContext.bufferPointer = NULL;

    // IMPORTANT this logic below only works if the first packet contains the needed information (Which it should)
    // Set signing context by parsing bip32paths and other info. function returns number of bytes read (not part of TX)
    headerBytesRead = setReqContextForSign(packet);
    txContext.totalTxBytes = reqContext.signableContentLength;

    // Signing header.
    uint8_t varint[9] = {0};
    uint64_t prefixLength = strlen(SIGNED_MESSAGE_PREFIX);
    uint8_t varintLength = lisk_encode_varint(prefixLength, varint);

    cx_hash(&txContext.txHash.header, 0, varint, varintLength, NULL, 0);
    cx_hash(&txContext.txHash.header, 0, SIGNED_MESSAGE_PREFIX, prefixLength, NULL, 0);

    // Signing msg
    os_memset(varint, 0, sizeof(varint));
    varintLength = lisk_encode_varint(reqContext.signableContentLength, varint);
    cx_hash(&txContext.txHash.header, 0, varint, varintLength, NULL, 0);

    //Prepare LineBuffer to show
    prepareMsgLineBuffer(packet, headerBytesRead); //Enough data here for display purpose
  }

  cx_hash(&txContext.txHash.header, 0, packet->data + headerBytesRead , packet->length - headerBytesRead, NULL, 0);
}

void prepareMsgLineBuffer(commPacket_t *packet, uint32_t headerBytesRead) {
  os_memset(lineBuffer, 0, sizeof(lineBuffer));
  uint8_t msgDisplayLenth = MIN(sizeof(lineBuffer), packet->length - headerBytesRead);
  os_memmove(lineBuffer, packet->data + headerBytesRead, msgDisplayLenth);

  if (msgDisplayLenth > 96) {
    os_memmove(lineBuffer+96, "...\0", 4);
  }

  uint8_t npc = 0; //Non Printable Chars Counter
  for (uint8_t i=0; i < msgDisplayLenth; i++) {
    npc += IS_PRINTABLE(lineBuffer[i]) ?
           0 /* Printable Char */:
           1 /* Non Printable Char */;
  }

  // We rewrite the line buffer to <binary data> in case >= than 40% is non printable or first char is not printable.
  if ((npc*100) / msgDisplayLenth >= 40 || ! IS_PRINTABLE(lineBuffer[0])) {
    os_memmove(lineBuffer, "< binary data >\0", 16);
  }
}

UX_STEP_NOCB(
  old_ux_sign_message_flow_1_step, 
  pnn, 
  {
    &C_nanox_icon_eye,
    "Confirm sign",
    "message",
  });
UX_STEP_NOCB(
  old_ux_sign_message_flow_2_step,
  bnnn_paging,
  {
    "Message",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  old_ux_sign_message_flow_3_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    os_memmove(lineBuffer, &reqContext.account.addressLisk32, ADDRESS_LISK32_LENGTH);
    lineBuffer[ADDRESS_LISK32_LENGTH] = '\0';
  },
  {
    "Sign with",
    lineBuffer,
  });
UX_STEP_CB(
  old_ux_sign_message_flow_4_step,
  pb,
  touch_approve(),
  {
    &C_nanox_icon_validate_14,
    "Confirm",
  });
UX_STEP_CB(
  old_ux_sign_message_flow_5_step,
  pb,
  touch_deny(),
  {
    &C_nanox_icon_crossmark,
    "Reject",
  });
UX_FLOW(old_ux_sign_message_flow,
  &old_ux_sign_message_flow_1_step,
  &old_ux_sign_message_flow_2_step,
  &old_ux_sign_message_flow_3_step,
  &old_ux_sign_message_flow_4_step,
  &old_ux_sign_message_flow_5_step);

static void ui_display_sign_message(void) {
  ux_flow_init(0, old_ux_sign_message_flow, NULL);
}

void processSignMessage(volatile unsigned int *flags) {
  // Close sha256 and hash again
  cx_hash_finalize(txContext.digest, DIGEST_LENGTH);

  // Init user flow.
  *flags |= IO_ASYNCH_REPLY;
  ui_display_sign_message();
}
