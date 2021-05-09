//
// Created by andrea on 09/12/18.
//

#include "voteTx.h"

#include "../../lisk_approval.h"
#include "../../lisk_utils.h"
#include "../lisk_internals.h"

static uint8_t votesAdded = 0;
static uint8_t votesRemoved = 0;

/**
 * Create second signature with address
 */

UX_STEP_NOCB(
  old_ux_sign_tx_vote_flow_1_step, 
  pnn, 
  {
    &C_nanox_icon_eye,
    "Confirm",
    "vote",
  });
UX_STEP_NOCB_INIT(
  old_ux_sign_tx_vote_flow_2_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    uint64_t address = deriveLegacyAddressFromPublic(&public_key);
    deriveLegacyAddressStringRepresentation(address, lineBuffer);
  },
  {
    "Vote from",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  old_ux_sign_tx_vote_flow_3_step,
  bn,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    intToString(votesAdded, lineBuffer);
  },
  {
    "Added",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  old_ux_sign_tx_vote_flow_4_step,
  bn,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    intToString(votesRemoved, lineBuffer);
  },
  {
    "Removed",
    lineBuffer,
  });
UX_STEP_CB(
  old_ux_sign_tx_vote_flow_5_step,
  pb,
  touch_approve(),
  {
    &C_nanox_icon_validate_14,
    "Confirm",
  });
UX_STEP_CB(
  old_ux_sign_tx_vote_flow_6_step,
  pb,
  touch_deny(),
  {
    &C_nanox_icon_crossmark,
    "Reject",
  });
UX_FLOW(old_ux_sign_tx_vote_flow,
  &old_ux_sign_tx_vote_flow_1_step,
  &old_ux_sign_tx_vote_flow_2_step,
  &old_ux_sign_tx_vote_flow_3_step,
  &old_ux_sign_tx_vote_flow_4_step,
  &old_ux_sign_tx_vote_flow_5_step,
  &old_ux_sign_tx_vote_flow_6_step);

static void ui_display_vote() {
  ux_flow_init(0, old_ux_sign_tx_vote_flow, NULL);
}

void tx_init_vote() {
  votesAdded = 0;
  votesRemoved = 0;
}

void tx_chunk_vote(uint8_t * data, uint8_t length, commPacket_t *sourcePacket, transaction_t *tx) {
  uint16_t i;
  for (i=0; i< length; i++) {
    if (data[i] == '+') {
      votesAdded++;
    } else if (data[i] == '-') {
      votesRemoved++;
    }
  }
}

void tx_end_vote(transaction_t *tx) {
  ui_display_vote();
}
