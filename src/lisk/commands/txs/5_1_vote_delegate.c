#include "5_1_vote_delegate.h"

#include "../../lisk_approval.h"
#include "../../lisk_utils.h"
#include "../../ui_elements_s.h"
#include "../../lisk_internals.h"
#include "common_parser.h"

#include <stdlib.h>

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

const ux_flow_step_t * ux_sign_tx_vote_delegate_flow[10];

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

  PRINTF("\n before ux_flow_init \n ");

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
      PRINTF("binaryKey _5_1_VOTE_TX asset:\n %X \n\n", binaryKey);
      assetSize = (uint32_t) transaction_get_varint();
      PRINTF("assetSize _5_1_VOTE_TX:\n %u \n\n", assetSize);
    case PLACEHOLDER:
      txContext.tx_parsing_state = PLACEHOLDER;
      is_available_to_parse(10);
      // Parse binaryKey and vote object syze
      binaryKey = (unsigned char) transaction_get_varint();
      PRINTF("binaryKey _5_1_VOTE_TX vote object :\n %X \n\n", binaryKey);
      // Assets is serialized as bytes, varint first for the size
      txContext.tx_asset._5_1_vote_delegate.lastObjectSize = (uint32_t) transaction_get_varint();
      PRINTF("vote object size _5_1_VOTE_TX:\n %u \n\n", txContext.tx_asset._5_1_vote_delegate.lastObjectSize);

    case _5_1_VOTE_DELEGATE_ADDRESS:
      txContext.tx_parsing_state = _5_1_VOTE_DELEGATE_ADDRESS;
      objectSizeToBeParsed = MIN(txContext.tx_asset._5_1_vote_delegate.lastObjectSize, txContext.bytesRemaining);
      is_available_to_parse(objectSizeToBeParsed);
      binaryKey = (unsigned char) transaction_get_varint();
      PRINTF("binaryKey _5_1_VOTE_DELEGATE_ADDRESS:\n %X \n\n", binaryKey);
      tmpSize = (uint32_t) transaction_get_varint();
      PRINTF("_5_1_VOTE_DELEGATE_ADDRESS tmpSize:\n %u \n\n", tmpSize);
      if(tmpSize != ADDRESS_HASH_LENGTH) {
        THROW(INVALID_PARAMETER);
      }
      transaction_memmove(tmpAddress, txContext.bufferPointer, ADDRESS_HASH_LENGTH);
      //transaction_offset_increase(ADDRESS_HASH_LENGTH);
      PRINTF("tmpAddress _5_1_VOTE_DELEGATE_ADDRESS:\n %.*H \n\n", ADDRESS_HASH_LENGTH, tmpAddress);

    case _5_1_VOTE_DELEGATE_AMOUNT:
      txContext.tx_parsing_state = _5_1_VOTE_DELEGATE_AMOUNT;
      binaryKey = (unsigned char) transaction_get_varint();
      PRINTF("binaryKey _5_1_VOTE_DELEGATE_AMOUNT:\n %X \n\n", binaryKey);
      tmpAmount = transaction_get_varint_signed();
      os_memset(lineBuffer, 0, sizeof(lineBuffer));
      itoa(tmpAmount, lineBuffer, 10);
      PRINTF("_5_1_VOTE_DELEGATE_AMOUNT tmpAmount:\n %s \n\n", lineBuffer);
      if(tmpAmount >= 0) {
        txContext.tx_asset._5_1_vote_delegate.n_vote++;
        txContext.tx_asset._5_1_vote_delegate.totAmountVote += llabs(tmpAmount);
      } else {
        txContext.tx_asset._5_1_vote_delegate.n_unvote++;
        txContext.tx_asset._5_1_vote_delegate.totAmountUnVote += llabs(tmpAmount);
      }

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

void tx_finalize_5_1_vote_delegate() {
  PRINTF("\n tx_finalize_5_1_vote_delegate before ui_display_vote_delegate \n ");
  ui_display_vote_delegate();
}
