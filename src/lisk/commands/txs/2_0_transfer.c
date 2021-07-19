#include "2_0_transfer.h"

#include "../../lisk_approval.h"
#include "../../lisk_utils.h"
#include "../../ui_elements_s.h"
#include "../lisk_internals.h"

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
    // uint64_t address = deriveLegacyAddressFromPublic(&public_key);
    // deriveLegacyAddressStringRepresentation(address, lineBuffer);
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
    // deriveLegacyAddressStringRepresentation(transaction.recipientId, lineBuffer);
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
    /*
    os_memmove(lineBuffer, message, MIN(50, curLength));
    // ellipsis
    if (curLength > 46) {
      strcpy(&lineBuffer[46], "...");
    }
    */
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
    // satoshiToString(transaction.amountSatoshi, lineBuffer);
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
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_4_step;
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_5_step;
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_6_step;
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_7_step;
  ux_sign_tx_send_flow[step++] = FLOW_END_STEP;

  ux_flow_init(0, ux_sign_tx_send_flow, NULL);
}

void tx_parse_specific_2_0_trasfer() {
  /* TX Structure:
   *
   * TX_SPECIFIC
   * - reuse PLACEHOLDER for txData, only /w 1 byte for len = 0
   * */

  //NB: There are no break in this switch. This is intentional.
  switch(txContext.tx_parsing_state) {
    case BEGINNING:
    case PLACEHOLDER:
      txContext.tx_parsing_state = PLACEHOLDER;

    default:
      THROW(INVALID_STATE);
    }
}

void tx_finalize_2_0_trasfer() {
  ui_display_send();
}
