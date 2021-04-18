//
// Created by andrea on 09/12/18.
//

#include "sendTx.h"

#include "../../lisk_approval.h"
#include "../../lisk_utils.h"
#include "../../ui_elements_s.h"
#include "../lisk_internals.h"
#include "../signTx.h"

static char message[64];
static uint8_t curLength;
static uint16_t totalLengthAfterAsset;

/**
 * Sign with address
 */

UX_STEP_NOCB(
  ux_sign_tx_send_flow_1_step, 
  pnn, 
  {
    &C_nanox_icon_eye,
    "Confirm",
    "send",
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_send_flow_2_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    uint64_t address = deriveAddressFromPublic(&public_key);
    deriveAddressStringRepresentation(address, lineBuffer);
  },
  {
    "Send from",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_send_flow_3_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    deriveAddressStringRepresentation(transaction.recipientId, lineBuffer);
  },
  {
    "To",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_send_flow_4_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    os_memmove(lineBuffer, message, MIN(50, curLength));
    // ellipsis
    if (curLength > 46) {
      strcpy(&lineBuffer[46], "...");
    }
  },
  {
    "Message",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_send_flow_5_step,
  bn,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    satoshiToString(transaction.amountSatoshi, lineBuffer);
  },
  {
    "Amount",
    lineBuffer,
  });
UX_STEP_CB(
  ux_sign_tx_send_flow_6_step,
  pb,
  touch_approve(),
  {
    &C_nanox_icon_validate_14,
    "Confirm",
  });
UX_STEP_CB(
  ux_sign_tx_send_flow_7_step,
  pb,
  touch_deny(),
  {
    &C_nanox_icon_crossmark,
    "Reject",
  });

const ux_flow_step_t * ux_sign_tx_send_flow[8];

static void ui_display_send() {
  int step = 0;
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_1_step;
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_2_step;
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_3_step;
  if (curLength > 0) ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_4_step;
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_5_step;
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_6_step;
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_7_step;
  ux_sign_tx_send_flow[step++] = FLOW_END_STEP;

  ux_flow_init(0, ux_sign_tx_send_flow, NULL);
}

void tx_init_send() {
  curLength = 0;
  totalLengthAfterAsset = 0;
  os_memset(message, 0, 64);
}

void tx_chunk_send(uint8_t * data, uint8_t length, commPacket_t *sourcePacket, transaction_t *tx) {
  uint8_t toReadLength = MAX(0, MIN(64 - curLength, length));
  os_memmove(message+curLength, data, toReadLength);
  curLength += toReadLength;
  totalLengthAfterAsset += length;
}

void tx_end_send(transaction_t *tx) {
  // Remove signature and/or secondSignature from message.
  curLength = totalLengthAfterAsset - (totalLengthAfterAsset / 64) * 64;
  ui_display_send();
}
