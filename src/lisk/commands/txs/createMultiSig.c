//
// Created by andrea on 09/12/18.
//

#include "createMultiSig.h"

#include "../lisk_internals.h"
#include "../signTx.h"
#include "../../approval.h"
#include "../../liskutils.h"
#include "os.h"

static uint8_t minKeys = 0;
static uint8_t lifetime = 0;

/**
 * Sign with address
 */

UX_STEP_NOCB(
  ux_sign_tx_multisig_flow_1_step, 
  pnn, 
  {
    &C_nanox_icon_eye,
    "Create",
    "multi-sig account",
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_multisig_flow_2_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    uint64_t address = deriveAddressFromPublic(&signContext.publicKey);
    deriveAddressStringRepresentation(address, lineBuffer);
  },
  {
    "Using",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_multisig_flow_3_step,
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
  ux_sign_tx_multisig_flow_4_step,
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
  ux_sign_tx_multisig_flow_5_step,
  pb,
  touch_approve(),
  {
    &C_nanox_icon_validate_14,
    "Confirm",
  });
UX_STEP_CB(
  ux_sign_tx_multisig_flow_6_step,
  pb,
  touch_deny(),
  {
    &C_nanox_icon_crossmark,
    "Reject",
  });
UX_FLOW(ux_sign_tx_multisig_flow,
  &ux_sign_tx_multisig_flow_1_step,
  &ux_sign_tx_multisig_flow_2_step,
  &ux_sign_tx_multisig_flow_3_step,
  &ux_sign_tx_multisig_flow_4_step,
  &ux_sign_tx_multisig_flow_5_step,
  &ux_sign_tx_multisig_flow_6_step);

static void ui_display_multisig() {
  ux_flow_init(0, ux_sign_tx_multisig_flow, NULL);
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
