#include "5_2_unlock_token.h"

#include "../../lisk_approval.h"
#include "../../lisk_utils.h"
#include "../../ui_elements_s.h"
#include "../../lisk_internals.h"
#include "common_parser.h"

#include <stdlib.h>

UX_STEP_NOCB(
  ux_sign_tx_unlock_token_flow_1_step, 
  pnn, 
  {
    &C_nanox_icon_eye,
    "Unlock",
    "tokens",
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_unlock_token_flow_2_step,
  bnnn_paging,
  {
    memset(lineBuffer, 0, sizeof(lineBuffer));
    memmove(lineBuffer, &reqContext.account.addressLisk32, ADDRESS_LISK32_LENGTH);
    lineBuffer[ADDRESS_LISK32_LENGTH] = '\0';
  },
  {
    "From account",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_unlock_token_flow_3_step,
  bnnn_paging,
  {
    memset(lineBuffer, 0, sizeof(lineBuffer));
    itoa(txContext.tx_asset._5_2_unlock_token.n_unlock, lineBuffer, 10);
  },
  {
    "Num. Unlock",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_unlock_token_flow_4_step,
  bn,
  {
    memset(lineBuffer, 0, sizeof(lineBuffer));
    satoshiToString(txContext.tx_asset._5_2_unlock_token.totAmountUnlock, lineBuffer);
  },
  {
    "Total Amount Unlock",
    lineBuffer,
});
UX_STEP_NOCB_INIT(
  ux_sign_tx_unlock_token_flow_5_step,
  bn,
  {
    memset(lineBuffer, 0, sizeof(lineBuffer));
    satoshiToString(txContext.fee, lineBuffer);
  },
  {
    "Fee",
    lineBuffer,
  });
UX_STEP_CB(
  ux_sign_tx_unlock_token_flow_6_step,
  pb,
  touch_approve(),
  {
    &C_nanox_icon_validate_14,
    "Confirm",
  });
UX_STEP_CB(
  ux_sign_tx_unlock_token_flow_7_step,
  pb,
  touch_deny(),
  {
    &C_nanox_icon_crossmark,
    "Reject",
  });

const ux_flow_step_t * ux_sign_tx_unlock_token_flow[8];

static void ui_display_unlock_token() {
  int step = 0;
  ux_sign_tx_unlock_token_flow[step++] = &ux_sign_tx_unlock_token_flow_1_step;
  ux_sign_tx_unlock_token_flow[step++] = &ux_sign_tx_unlock_token_flow_2_step;
  if(txContext.tx_asset._5_2_unlock_token.n_unlock > 0) {
    ux_sign_tx_unlock_token_flow[step++] = &ux_sign_tx_unlock_token_flow_3_step;
    ux_sign_tx_unlock_token_flow[step++] = &ux_sign_tx_unlock_token_flow_4_step;
  }
  ux_sign_tx_unlock_token_flow[step++] = &ux_sign_tx_unlock_token_flow_5_step;
  ux_sign_tx_unlock_token_flow[step++] = &ux_sign_tx_unlock_token_flow_6_step;
  ux_sign_tx_unlock_token_flow[step++] = &ux_sign_tx_unlock_token_flow_7_step;
  ux_sign_tx_unlock_token_flow[step++] = FLOW_END_STEP;

  ux_flow_init(0, ux_sign_tx_unlock_token_flow, NULL);
}

void tx_parse_specific_5_2_unlock_token() {
  /**
   * TX_ASSET
   * unlockObjects[]:
   *    - delegateAddr - 20 bytes
   *    - amount -> uint64
   *    - unlockHeight -> uint32
   *
   */
  unsigned char binaryKey = 0;
  uint32_t tmpSize = 0;
  uint32_t assetSize = 0;
  uint32_t objectSizeToBeParsed;
  unsigned char tmpAddress[ADDRESS_HASH_LENGTH];
  int64_t tmpAmount = 0;

  while (txContext.tx_parsing_group == TX_ASSET)
  {
    switch(txContext.tx_parsing_state) {
    case BEGINNING:
      // Only first time parse binaryKey and complete assetSize 
      is_available_to_parse(10);
      binaryKey = (unsigned char) transaction_get_varint();
      assetSize = (uint32_t) transaction_get_varint();
    case PLACEHOLDER:
      txContext.tx_parsing_state = PLACEHOLDER;
      is_available_to_parse(10);
      // Parse binaryKey and vote object syze
      binaryKey = (unsigned char) transaction_get_varint();
      // Assets is serialized as bytes, varint first for the size
      txContext.tx_asset._5_2_unlock_token.lastObjectSize = (uint32_t) transaction_get_varint();

    case _5_2_UNLOCK_ADDRESS:
      txContext.tx_parsing_state = _5_2_UNLOCK_ADDRESS;
      objectSizeToBeParsed = MIN(txContext.tx_asset._5_2_unlock_token.lastObjectSize, txContext.bytesRemaining);
      is_available_to_parse(objectSizeToBeParsed);
      binaryKey = (unsigned char) transaction_get_varint();
      tmpSize = (uint32_t) transaction_get_varint();
      if(tmpSize != ADDRESS_HASH_LENGTH) {
        THROW(INVALID_PARAMETER);
      }
      transaction_memmove(tmpAddress, txContext.bufferPointer, ADDRESS_HASH_LENGTH);

    case _5_2_UNLOCK_AMOUNT:
      txContext.tx_parsing_state = _5_2_UNLOCK_AMOUNT;
      binaryKey = (unsigned char) transaction_get_varint();
      tmpAmount = transaction_get_varint();
      txContext.tx_asset._5_2_unlock_token.n_unlock++;
      txContext.tx_asset._5_2_unlock_token.totAmountUnlock += tmpAmount;

    case _5_2_UNLCOK_BLOCK_HEIGHT:
      txContext.tx_parsing_state = _5_2_UNLCOK_BLOCK_HEIGHT;
      binaryKey = (unsigned char) transaction_get_varint();
      tmpAmount = transaction_get_varint();

      //Check if we have parsed all
      if(txContext.bytesRemaining == 0) {
        txContext.tx_parsing_group = CHECK_SANITY_BEFORE_SIGN;
        txContext.tx_parsing_state = BEGINNING;
      } else {
        txContext.tx_parsing_group = TX_ASSET;
        txContext.tx_parsing_state = PLACEHOLDER;
      }
      break;
    default:
      THROW(INVALID_STATE);
    }
  }

}

void tx_finalize_5_2_unlock_token() {
  ui_display_unlock_token();
}
