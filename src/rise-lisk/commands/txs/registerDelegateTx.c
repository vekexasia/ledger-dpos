//
// Created by andrea on 09/12/18.
//

#include "registerDelegateTx.h"
#include "../signTx.h"
#include "../../approval.h"
#include "../../dposutils.h"
#include "../../../io.h"
#include "../../../ui_utils.h"

#define USERNAME_MAX_LEN 20

static char username[USERNAME_MAX_LEN];
static uint8_t usernameLength;
static uint16_t readBytes;

/**
 * Create second signature with address
 */
#if defined(TARGET_NANOS)
static const bagl_element_t ui_regdelegate_el[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Register", 0x01),
  TITLE_ITEM("For account", 0x02),
  TITLE_ITEM("With name", 0x03),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_CHECK(0x03),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

static void ui_processor_regDelegate(uint8_t step) {
  os_memset(lineBuffer, 0, 50);
  uint64_t address;
  switch (step) {
    case 1:
      os_memmove(lineBuffer, "delegate\0", 11);
      break;
    case 2:
      address = deriveAddressFromPublic(&signContext.publicKey);
      deriveAddressStringRepresentation(address, lineBuffer);
      break;
    case 3:
      os_memmove(lineBuffer, username, usernameLength);
      break;
  }
}

static void ui_display_regdelegate() {
  ux.elements = ui_regdelegate_el;
  ux.elements_count = 9;
  totalSteps = 3;
  ui_processor = ui_processor_regDelegate;
}
#endif

#if defined(TARGET_NANOX)
UX_STEP_NOCB(
  ux_sign_tx_regdelegate_flow_1_step, 
  pnn, 
  {
    &C_nanox_icon_eye,
    "Register",
    "delegate",
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_regdelegate_flow_2_step,
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
  ux_sign_tx_regdelegate_flow_3_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    os_memmove(lineBuffer, username, usernameLength);
  },
  {
    "With name",
    lineBuffer,
  });
UX_STEP_CB(
  ux_sign_tx_regdelegate_flow_4_step,
  pb,
  touch_approve(),
  {
    &C_nanox_icon_validate_14,
    "Confirm",
  });
UX_STEP_CB(
  ux_sign_tx_regdelegate_flow_5_step,
  pb,
  touch_deny(),
  {
    &C_nanox_icon_crossmark,
    "Reject",
  });
UX_FLOW(ux_sign_tx_regdelegate_flow,
  &ux_sign_tx_regdelegate_flow_1_step,
  &ux_sign_tx_regdelegate_flow_2_step,
  &ux_sign_tx_regdelegate_flow_3_step,
  &ux_sign_tx_regdelegate_flow_4_step,
  &ux_sign_tx_regdelegate_flow_5_step);

static void ui_display_regdelegate() {
  ux_flow_init(0, ux_sign_tx_regdelegate_flow, NULL);
}
#endif

static void checkUsernameValidity() {
  if (usernameLength > USERNAME_MAX_LEN) {
    THROW(INVALID_PARAMETER);
  }
  uint8_t i;
  for (i = 0; i < usernameLength; i++) {
    char c = username[i];
    if (
      !(c >= 'a' && c <= 'z') &&
      !(c >= '0' && c <= '9') &&
      !(c == '!' || c == '@' || c == '$' || c == '&' || c == '_' || c == '.')) {
      THROW(INVALID_PARAMETER);
    }
  }
}

void tx_init_regdel() {
  os_memset(username, 0, USERNAME_MAX_LEN);
  usernameLength = 0;
  readBytes = 0;
}

void tx_chunk_regdel(uint8_t * data, uint8_t length, commPacket_t *sourcePacket, transaction_t *tx) {
  if (readBytes < USERNAME_MAX_LEN) {
    uint8_t appendCount = MIN(length, USERNAME_MAX_LEN - readBytes);
    os_memmove(&username[readBytes], data, appendCount);
  }

  readBytes += length;
}

void tx_end_regdel(transaction_t *tx) {
  //Calculate the exact username length by removing signatures
  usernameLength = readBytes - MIN(readBytes / 64, 2) * 64;
  checkUsernameValidity();

  ui_display_regdelegate();
}
