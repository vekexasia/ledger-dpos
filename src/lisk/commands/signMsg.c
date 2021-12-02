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

char message[100];

void handleSignMessagePacket(commPacket_t *packet) {

  uint32_t headerBytesRead = 0;

  // if first packet with signing header
  if ( packet->first ) {

    // Reset sha256 and context
    memset(&reqContext, 0, sizeof(reqContext));
    memset(&txContext, 0, sizeof(txContext));
    cx_sha256_init(&txContext.sha256);
    txContext.bufferPointer = NULL;

    // IMPORTANT this logic below only works if the first packet contains the needed information (Which it should)
    // Set signing context by parsing bip32paths and other info. function returns number of bytes read (not part of TX)
    headerBytesRead = setReqContextForSign(packet);
    txContext.totalTxBytes = reqContext.signableContentLength;
    txContext.bufferPointer = packet->data + headerBytesRead;
    txContext.bytesChunkRemaining = packet->length - headerBytesRead;

    // Signing header.
    uint8_t varint[9] = {0};
    uint16_t prefixLength = strlen(SIGNED_MESSAGE_PREFIX);
    uint8_t varintLength = lisk_encode_varint(prefixLength, varint);

    cx_hash(&txContext.sha256.header, 0, varint, varintLength, NULL, 0);
    cx_hash(&txContext.sha256.header, 0, SIGNED_MESSAGE_PREFIX, prefixLength, NULL, 0);

    // Signing msg
    memset(varint, 0, sizeof(varint));
    varintLength = lisk_encode_varint(reqContext.signableContentLength, varint);
    cx_hash(&txContext.sha256.header, 0, varint, varintLength, NULL, 0);

    //Prepare LineBuffer to show
    prepareMsgLineBuffer(); //Enough data here for display purpose
  }
  txContext.bufferPointer = packet->data + headerBytesRead;
  txContext.bytesChunkRemaining = packet->length - headerBytesRead;
  cx_hash(&txContext.sha256.header, 0, txContext.bufferPointer , txContext.bytesChunkRemaining, NULL, 0);
}

void prepareMsgLineBuffer() {
  memset(message, 0, sizeof(message));
  uint8_t msgDisplayLenth = MIN(sizeof(message), txContext.totalTxBytes);
  memmove(message, txContext.bufferPointer, msgDisplayLenth);

  if (msgDisplayLenth > 96) {
    memmove(message + 96, "...\0", 4);
  }

  uint8_t npc = 0; //Non Printable Chars Counter
  for (uint8_t i=0; i < msgDisplayLenth; i++) {
    npc += IS_PRINTABLE(message[i]) ?
           0 /* Printable Char */:
           1 /* Non Printable Char */;
  }

  // We rewrite the line buffer to <binary data> in case >= than 40% is non printable
  // or first char is not printable.
  if ((npc*100) / msgDisplayLenth >= 40 || ! IS_PRINTABLE(message[0])) {
    memmove(message, "< binary data >\0", 16);
  }
}

UX_STEP_NOCB(
  ux_sign_message_flow_1_step,
  pnn, 
  {
    &C_nanox_icon_eye,
    "Confirm sign",
    "message",
  });
UX_STEP_NOCB_INIT(
  ux_sign_message_flow_2_step,
  bnnn_paging,
  {
    memset(lineBuffer, 0, sizeof(lineBuffer));
    memmove(lineBuffer, message, 100);
  },
  {
    "Message",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_message_flow_3_step,
  bnnn_paging,
  {
    memset(lineBuffer, 0, sizeof(lineBuffer));
    memmove(lineBuffer, &reqContext.account.addressLisk32, ADDRESS_LISK32_LENGTH);
    lineBuffer[ADDRESS_LISK32_LENGTH] = '\0';
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
  // Close sha256 and hash again
  cx_hash_finalize_msg();

  // Init user flow.
  *flags |= IO_ASYNCH_REPLY;
  ui_display_sign_message();
}
