#include "5_1_vote_delegate.h"

#include "../../lisk_approval.h"
#include "../../lisk_utils.h"
#include "../../ui_elements_s.h"
#include "../../lisk_internals.h"
#include "common_parser.h"

UX_STEP_NOCB(
  ux_sign_tx_vote_delegate_flow_1_step, 
  pnn, 
  {
    &C_nanox_icon_eye,
    "Vote",
    "delegates",
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_vote_delegate_flow_2_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    os_memmove(lineBuffer, &reqContext.account.addressLisk32, ADDRESS_LISK32_LENGTH);
    lineBuffer[ADDRESS_LISK32_LENGTH] = '\0';
  },
  {
    "From account",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_vote_delegate_flow_3_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    lisk_int_to_string(txContext.tx_asset._5_1_vote_delegate.n_vote, lineBuffer);
  },
  {
    "Num. Added Votes",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_vote_delegate_flow_4_step,
  bn,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    satoshiToString(txContext.tx_asset._5_1_vote_delegate.totAmountVote, lineBuffer);
  },
  {
    "Total Amount Added",
    lineBuffer,
});
UX_STEP_NOCB_INIT(
  ux_sign_tx_vote_delegate_flow_5_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    lisk_int_to_string(txContext.tx_asset._5_1_vote_delegate.n_unvote, lineBuffer);
  },
  {
    "Num. Removed Votes",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_vote_delegate_flow_6_step,
  bn,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    satoshiToString(txContext.tx_asset._5_1_vote_delegate.totAmountUnVote, lineBuffer);
  },
  {
    "Total Amount Removed",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_vote_delegate_flow_7_step,
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
  ux_sign_tx_vote_delegate_flow_8_step,
  pb,
  touch_approve(),
  {
    &C_nanox_icon_validate_14,
    "Confirm",
  });
UX_STEP_CB(
  ux_sign_tx_vote_delegate_flow_9_step,
  pb,
  touch_deny(),
  {
    &C_nanox_icon_crossmark,
    "Reject",
  });

const ux_flow_step_t * ux_sign_tx_vote_delegate_flow[9];

static void ui_display_vote_delegate() {
  int step = 0;
  ux_sign_tx_vote_delegate_flow[step++] = &ux_sign_tx_vote_delegate_flow_1_step;
  ux_sign_tx_vote_delegate_flow[step++] = &ux_sign_tx_vote_delegate_flow_2_step;
  if(txContext.tx_asset._5_1_vote_delegate.n_vote > 0) {
    ux_sign_tx_vote_delegate_flow[step++] = &ux_sign_tx_vote_delegate_flow_3_step;
    ux_sign_tx_vote_delegate_flow[step++] = &ux_sign_tx_vote_delegate_flow_4_step;
  }
  if(txContext.tx_asset._5_1_vote_delegate.n_unvote > 0) {
    ux_sign_tx_vote_delegate_flow[step++] = &ux_sign_tx_vote_delegate_flow_5_step;
    ux_sign_tx_vote_delegate_flow[step++] = &ux_sign_tx_vote_delegate_flow_6_step;
  }
  ux_sign_tx_vote_delegate_flow[step++] = &ux_sign_tx_vote_delegate_flow_7_step;
  ux_sign_tx_vote_delegate_flow[step++] = &ux_sign_tx_vote_delegate_flow_8_step;
  ux_sign_tx_vote_delegate_flow[step++] = &ux_sign_tx_vote_delegate_flow_9_step;
  ux_sign_tx_vote_delegate_flow[step++] = FLOW_END_STEP;

  ux_flow_init(0, ux_sign_tx_vote_delegate_flow, NULL);
}

void tx_parse_specific_5_1_vote_delegate() {
  /**
   * TX_ASSET
   * votes[]:
   *    - delegateAddr - 20 bytes
   *    - amount -> signed!! int64
   *
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
    PRINTF("binaryKey _5_0_REG_DELEGATE_TX asset:\n %X \n\n", binaryKey);
    // Assets is serialized as bytes, varint first for the size
    tmpSize = (uint32_t) transaction_get_varint();
    PRINTF("asset size _5_0_REG_DELEGATE_TX:\n %u \n\n", tmpSize);
  case _5_0_REG_DELEGATE_USERNAME:
    txContext.tx_parsing_state = _5_0_REG_DELEGATE_USERNAME;
    // This is the last field, check if we have the last chunk
    binaryKey = (unsigned char) transaction_get_varint();
    PRINTF("binaryKey _5_0_REG_DELEGATE_USERNAME:\n %X \n\n", binaryKey);
    txContext.tx_asset._5_0_reg_delegate.delegateLength = (uint32_t) transaction_get_varint();
    PRINTF("txContext.asset.delegateLength:\n %u \n\n", txContext.tx_asset._5_0_reg_delegate.delegateLength);
    transaction_memmove(txContext.tx_asset._5_0_reg_delegate.delegate,
                        txContext.bufferPointer,
                        txContext.tx_asset._5_0_reg_delegate.delegateLength);
    checkUsernameValidity();
    {
      os_memset(lineBuffer, 0, sizeof(lineBuffer));
      os_memmove(lineBuffer, txContext.tx_asset._5_0_reg_delegate.delegate, txContext.tx_asset._5_0_reg_delegate.delegateLength);
      PRINTF("txContext.asset.delegate:\n %s \n\n", lineBuffer);
    }
    txContext.tx_parsing_group = CHECK_SANITY_BEFORE_SIGN;
    txContext.tx_parsing_state = BEGINNING;
    break;
  default:
    THROW(INVALID_STATE);
  }

}

void tx_finalize_5_1_vote_delegate() {
  ui_display_vote_delegate();
}
