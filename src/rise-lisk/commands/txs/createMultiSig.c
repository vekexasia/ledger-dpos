//
// Created by andrea on 09/12/18.
//

#include "createMultiSig.h"
#include "../signTx.h"
#include "../../approval.h"
#include "../../dposutils.h"
#include "../../../io.h"
#include "../../../ui_utils.h"
#include "os.h"

static uint8_t minKeys = 0;
static uint8_t lifetime = 0;

/**
 * Sign with address
 */
#if defined(TARGET_NANOS)
static const bagl_element_t ui_multisig_el[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Create", 0x01),
  TITLE_ITEM("Using", 0x02),
  TITLE_ITEM("Minimum", 0x03),
  TITLE_ITEM("Lifetime", 0x04),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_ARROW_RIGHT(0x03),
  ICON_CHECK(0x04),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

static void ui_processor_multisig(uint8_t step) {
  uint64_t tmp = 0;
  os_memset(lineBuffer, 0, 50);
  switch (step) {
    case 1:
      os_memmove(lineBuffer, "Multi-sig account\0", 17);
      break;
    case 2:
      tmp = deriveAddressFromPublic(&signContext.publicKey);
      deriveAddressStringRepresentation(tmp, lineBuffer);
      break;
    case 3:
      // Min keys
      tmp = intToString(minKeys, lineBuffer);
      os_memmove(lineBuffer+tmp, " keys\0", 6);
      break;
    case 4:
      // Lifetime
      tmp = intToString(lifetime, lineBuffer);
      os_memmove(lineBuffer+tmp, " hours\0", 7);
      break;
  }
}

static void ui_display_multisig() {
  ux.elements = ui_multisig_el;
  ux.elements_count = 11;
  totalSteps = 4;
  ui_processor = ui_processor_multisig;
}
#endif

#if defined(TARGET_NANOX)
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
#endif

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
