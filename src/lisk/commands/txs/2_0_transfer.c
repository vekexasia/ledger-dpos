#include "2_0_transfer.h"

#include "../../lisk_approval.h"
#include "../../lisk_utils.h"
#include "../../ui_elements_s.h"
#include "../../lisk_internals.h"
#include "common_parser.h"

UX_STEP_NOCB(
  ux_sign_tx_send_flow_1_step,
  pnn, 
  {
    &C_nanox_icon_eye,
    "Confirm sign",
    "transaction",
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_send_flow_2_step,
  bnnn_paging,
  {
    memset(lineBuffer, 0, sizeof(lineBuffer));
    memmove(lineBuffer, &reqContext.account.addressLisk32, ADDRESS_LISK32_LENGTH);
    lineBuffer[ADDRESS_LISK32_LENGTH] = '\0';
  },
  {
    "Send from",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_send_flow_3_step,
  bnnn_paging,
  {
    memset(lineBuffer, 0, sizeof(lineBuffer));
    lisk_addr_encode(lineBuffer, LISK32_ADDRESS_PREFIX,
                     txContext.tx_asset._2_0_transfer.recipientAddress, ADDRESS_HASH_LENGTH);
    lineBuffer[ADDRESS_LISK32_LENGTH] = '\0';
  },
  {
    "To",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_send_flow_4_step,
  bnnn_paging,
  {
    memset(lineBuffer, 0, sizeof(lineBuffer));
    memmove(lineBuffer,
            txContext.tx_asset._2_0_transfer.data,
            txContext.tx_asset._2_0_transfer.dataLength);
  },
  {
    "Message",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_send_flow_5_step,
  bn,
  {
    memset(lineBuffer, 0, sizeof(lineBuffer));
    satoshiToString(txContext.tx_asset._2_0_transfer.amount, lineBuffer);
  },
  {
    "Amount",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_send_flow_6_step,
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
  ux_sign_tx_send_flow_7_step,
  pb,
  touch_approve(),
  {
    &C_nanox_icon_validate_14,
    "Confirm",
  });
UX_STEP_CB(
  ux_sign_tx_send_flow_8_step,
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
  if(txContext.tx_asset._2_0_transfer.dataLength > 0) {
    ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_4_step;
  }
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_5_step;
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_6_step;
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_7_step;
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_8_step;
  ux_sign_tx_send_flow[step++] = FLOW_END_STEP;

  ux_flow_init(0, ux_sign_tx_send_flow, NULL);
}

void tx_parse_specific_2_0_trasfer() {
  /**
   * TX_ASSET
   * - Amount -> uint64
   * - RecipientAddress -> 20 bytes
   * - Data -> String 0-64 chars
   */

  unsigned char binaryKey = 0;
  uint32_t tmpSize = 0;

  //NB: There are no break in this switch. This is intentional.
  switch(txContext.tx_parsing_state) {
    case BEGINNING:
    case PLACEHOLDER:
      txContext.tx_parsing_state = PLACEHOLDER;
      // Lets be conservative, check if we have the last chunk
      // It's enough for the whole asset object
      if(txContext.bytesChunkRemaining != txContext.bytesRemaining) {
        THROW(NEED_NEXT_CHUNK);
      }
      binaryKey = (unsigned char) transaction_get_varint();
      // Assets is serialized as bytes, varint first for the size
      tmpSize = (uint32_t) transaction_get_varint();
    case _2_0_SENDTX_AMOUNT:
      txContext.tx_parsing_state = _2_0_SENDTX_AMOUNT;
      binaryKey = (unsigned char) transaction_get_varint();
      txContext.tx_asset._2_0_transfer.amount = transaction_get_varint();
    case _2_0_SENDTX_RECIPIENT_ADDR:
      txContext.tx_parsing_state = _2_0_SENDTX_RECIPIENT_ADDR;
      binaryKey = (unsigned char) transaction_get_varint();
      tmpSize = (uint32_t) transaction_get_varint();
      if(tmpSize != ADDRESS_HASH_LENGTH) {
        THROW(INVALID_PARAMETER);
      }
      transaction_memmove(txContext.tx_asset._2_0_transfer.recipientAddress, txContext.bufferPointer, ADDRESS_HASH_LENGTH);
    case _2_0_SENDTX_DATA:
      txContext.tx_parsing_state = _2_0_SENDTX_DATA;
      binaryKey = (unsigned char) transaction_get_varint();
      txContext.tx_asset._2_0_transfer.dataLength = (uint32_t) transaction_get_varint();
      if(txContext.tx_asset._2_0_transfer.dataLength > 0) {
        transaction_memmove(txContext.tx_asset._2_0_transfer.data,
                            txContext.bufferPointer,
                            txContext.tx_asset._2_0_transfer.dataLength);
      }
      txContext.tx_parsing_group = CHECK_SANITY_BEFORE_SIGN;
      txContext.tx_parsing_state = BEGINNING;
      break;
    default:
      THROW(INVALID_STATE);
    }
}

void tx_finalize_2_0_trasfer() {
  ui_display_send();
}
