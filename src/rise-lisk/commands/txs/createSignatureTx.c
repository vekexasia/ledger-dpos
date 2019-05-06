//
// Created by andrea on 09/12/18.
//

#include "createSignatureTx.h"
#include "../../approval.h"
#include "../../dposutils.h"
#include "../../../ui_utils.h"

static uint8_t pubkey[32];
static uint8_t read;

static void truncatedHexPubKey(char *outBuf) {
  uint8_t i;
  for (i = 0; i < 3; i++) {
    toHex(pubkey[i], outBuf + i * 2);
  }
  outBuf[6] = '.';
  outBuf[7] = '.';
  for (i = 0; i < 3; i++) {
    toHex(pubkey[i + 32 - 3], outBuf + 8 + i * 2);
  }
  outBuf[14] = '\0';
}

/**
 * Create second signature with address
 */
#if defined(TARGET_NANOS)
static const bagl_element_t ui_2ndsign_el[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Create second", 0x01),
  TITLE_ITEM("For account", 0x02),
  TITLE_ITEM("With public", 0x03),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_CHECK(0x03),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

static void ui_processor_2nd_sign(uint8_t step) {
  os_memset(lineBuffer, 0, 50);
  uint64_t address;
  switch (step) {
    case 1:
      os_memmove(lineBuffer, "signature\0", 11);
      break;
    case 2:
      address = deriveAddressFromPublic(&signContext.publicKey);
      deriveAddressStringRepresentation(address, lineBuffer);
      break;
    case 3:
      truncatedHexPubKey(lineBuffer);
      break;
  }
}

static void ui_display_2ndsig() {
  ux.elements = ui_2ndsign_el;
  ux.elements_count = 9;
  totalSteps = 3;
  ui_processor = ui_processor_2nd_sign;
}
#endif

#if defined(TARGET_NANOX)
UX_STEP_NOCB(
  ux_sign_tx_2ndsig_flow_1_step, 
  pnn, 
  {
    &C_nanox_icon_eye,
    "Create",
    "2nd signature",
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_2ndsig_flow_2_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    uint64_t address = deriveAddressFromPublic(&signContext.publicKey);
    deriveAddressStringRepresentation(address, lineBuffer);
  },
  {
    "For account",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_2ndsig_flow_3_step,
  bn,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    truncatedHexPubKey(lineBuffer);
  },
  {
    "With public",
    lineBuffer,
  });
UX_STEP_CB(
  ux_sign_tx_2ndsig_flow_4_step,
  pb,
  touch_approve(),
  {
    &C_nanox_icon_validate_14,
    "Confirm",
  });
UX_STEP_CB(
  ux_sign_tx_2ndsig_flow_5_step,
  pb,
  touch_deny(),
  {
    &C_nanox_icon_crossmark,
    "Reject",
  });
UX_FLOW(ux_sign_tx_2ndsig_flow,
  &ux_sign_tx_2ndsig_flow_1_step,
  &ux_sign_tx_2ndsig_flow_2_step,
  &ux_sign_tx_2ndsig_flow_3_step,
  &ux_sign_tx_2ndsig_flow_4_step,
  &ux_sign_tx_2ndsig_flow_5_step);

static void ui_display_2ndsig() {
  ux_flow_init(0, ux_sign_tx_2ndsig_flow, NULL);
}
#endif

void tx_init_2ndsig() {
  os_memset(pubkey, 0, 32);
  read = 0;
}

void tx_chunk_2ndsig(uint8_t * data, uint8_t length, commPacket_t *sourcePacket, transaction_t *tx) {
  os_memmove(pubkey + read, data, length);

  read += length;
}

void tx_end_2ndsig(transaction_t *tx) {
  ui_display_2ndsig();
}
