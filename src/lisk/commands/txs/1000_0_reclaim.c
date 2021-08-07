#include "1000_0_reclaim.h"

#include "../../lisk_approval.h"
#include "../../lisk_utils.h"
#include "../../ui_elements_s.h"
#include "../../lisk_internals.h"
#include "common_parser.h"

UX_STEP_NOCB(
  ux_sign_tx_reclaim_flow_1_step, 
  pnn, 
  {
    &C_nanox_icon_eye,
    "Reclaim",
    "account",
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_reclaim_flow_2_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    os_memmove(lineBuffer, &reqContext.account.addressLisk32, ADDRESS_LISK32_LENGTH);
    lineBuffer[ADDRESS_LISK32_LENGTH] = '\0';
  },
  {
    "For account",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_reclaim_flow_3_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    satoshiToString(txContext.tx_asset._1000_0_reclaim.amount, lineBuffer);
  },
  {
    "Amount",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_reclaim_flow_4_step,
  bn,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    satoshiToString(txContext.fee, lineBuffer);
  },
  {
    "Fee",
    lineBuffer,
  });
UX_STEP_CB(
  ux_sign_tx_reclaim_flow_5_step,
  pb,
  touch_approve(),
  {
    &C_nanox_icon_validate_14,
    "Confirm",
  });
UX_STEP_CB(
  ux_sign_tx_reclaim_flow_6_step,
  pb,
  touch_deny(),
  {
    &C_nanox_icon_crossmark,
    "Reject",
  });
UX_FLOW(ux_sign_tx_reclaim_flow,
  &ux_sign_tx_reclaim_flow_1_step,
  &ux_sign_tx_reclaim_flow_2_step,
  &ux_sign_tx_reclaim_flow_3_step,
  &ux_sign_tx_reclaim_flow_4_step,
  &ux_sign_tx_reclaim_flow_5_step,
  &ux_sign_tx_reclaim_flow_6_step);

static void ui_display_regdelegate() {
  ux_flow_init(0, ux_sign_tx_reclaim_flow, NULL);
}

void tx_parse_specific_1000_0_reclaim() {
  /**
   * TX_ASSET
   * - Amount -> uint64
   */
  unsigned char binaryKey = 0;
  uint32_t tmpSize = 0;

  switch(txContext.tx_parsing_state) {
  case BEGINNING:
  case PLACEHOLDER:
    txContext.tx_parsing_state = PLACEHOLDER;
    // Lets be conservative, check if we have the last chunk
    if(txContext.bytesChunkRemaining != txContext.bytesRemaining) {
      THROW(NEED_NEXT_CHUNK);
    }
    binaryKey = (unsigned char) transaction_get_varint();
    // Assets is serialized as bytes, varint first for the size
    tmpSize = (uint32_t) transaction_get_varint();
  case _1000_0_RECLAIM_AMOUNT:
    txContext.tx_parsing_state = _1000_0_RECLAIM_AMOUNT;
    // This is the last field, check if we have the last chunk
    binaryKey = (unsigned char) transaction_get_varint();
    txContext.tx_asset._1000_0_reclaim.amount = transaction_get_varint();

    txContext.tx_parsing_group = CHECK_SANITY_BEFORE_SIGN;
    txContext.tx_parsing_state = BEGINNING;
    break;
  default:
    THROW(INVALID_STATE);
  }

}

void tx_finalize_1000_0_reclaim() {
  ui_display_regdelegate();
}
