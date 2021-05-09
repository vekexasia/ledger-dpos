//
// Created by andrea on 09/12/18.
//

#include "createMultiSig.h"

#include "../../lisk_approval.h"
#include "../../lisk_utils.h"
#include "../lisk_internals.h"
#include "os.h"

static uint8_t minKeys = 0;
static uint8_t lifetime = 0;

/**
 * Sign with address
 */

UX_STEP_NOCB(
  old_ux_sign_tx_multisig_flow_1_step, 
  pnn, 
  {
    &C_nanox_icon_eye,
    "Create",
    "multi-sig account",
  });
UX_STEP_NOCB_INIT(
  old_ux_sign_tx_multisig_flow_2_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    uint64_t address = deriveLegacyAddressFromPublic(&public_key);
    deriveLegacyAddressStringRepresentation(address, lineBuffer);
  },
  {
    "Using",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  old_ux_sign_tx_multisig_flow_3_step,
  bn,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    uint64_t len = intToString(minKeys, lineBuffer);
    strcpy(&lineBuffer[len], " keys");
  },
  {
    "Minimum",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  old_ux_sign_tx_multisig_flow_4_step,
  bn,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    uint64_t len = intToString(lifetime, lineBuffer);
    strcpy(&lineBuffer[len], " hours");
  },
  {
    "Lifetime",
    lineBuffer,
  });
UX_STEP_CB(
  old_ux_sign_tx_multisig_flow_5_step,
  pb,
  touch_approve(),
  {
    &C_nanox_icon_validate_14,
    "Confirm",
  });
UX_STEP_CB(
  old_ux_sign_tx_multisig_flow_6_step,
  pb,
  touch_deny(),
  {
    &C_nanox_icon_crossmark,
    "Reject",
  });
UX_FLOW(old_ux_sign_tx_multisig_flow,
  &old_ux_sign_tx_multisig_flow_1_step,
  &old_ux_sign_tx_multisig_flow_2_step,
  &old_ux_sign_tx_multisig_flow_3_step,
  &old_ux_sign_tx_multisig_flow_4_step,
  &old_ux_sign_tx_multisig_flow_5_step,
  &old_ux_sign_tx_multisig_flow_6_step);

static void ui_display_multisig() {
  ux_flow_init(0, old_ux_sign_tx_multisig_flow, NULL);
}

void tx_init_multisig(){
  minKeys = 0;
  lifetime = 0;
}

void tx_chunk_multisig(uint8_t * data, uint8_t length, commPacket_t *sourcePacket, transaction_t *tx){
  if (minKeys == 0) {
    minKeys = data[0];
    lifetime = data[1];
  }
}

void tx_end_multisig(transaction_t *tx) {
  ui_display_multisig();
}
