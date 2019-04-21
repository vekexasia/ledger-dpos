//
// Created by andrea on 08/12/18.
//

#include "getPubKey.h"

#include "../ed25519.h"
#include "../dposutils.h"
#include "../ui_elements_s.h"
#include "../../ui_utils.h"
#include "../../io.h"
#include "../approval.h"
#include "os.h"

uint8_t pubKeyResponseBuffer[32+22];

/**
 * Creates the response for the getPublicKey command.
 * It returns both publicKey and derived Address
 */
static void createPublicKeyResponse() {
  initResponse();
  getEncodedPublicKey(&signContext.publicKey, pubKeyResponseBuffer);
  addToResponse(pubKeyResponseBuffer, 32);
  uint64_t address = deriveAddressFromPublic(&signContext.publicKey);
  uint8_t length = deriveAddressStringRepresentation(address, (char *) (pubKeyResponseBuffer + 32));

  addToResponse(pubKeyResponseBuffer + 32, length);
}

static void sendPublicKeyResponse() {
  createPublicKeyResponse();

  unsigned int tx = flushResponseToIO(G_io_apdu_buffer);
  G_io_apdu_buffer[tx]   = 0x90;
  G_io_apdu_buffer[tx+1] = 0x00;

  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx+2);
}

#if defined(TARGET_NANOS)
static const bagl_element_t verify_address_ui[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Verify Address", 0x00),
  ICON_CROSS(0x00),
  ICON_CHECK(0x00),
  LINEBUFFER,
};

unsigned int verify_address_ui_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      sendPublicKeyResponse();
      // Display back the original UX
      ui_idle();
      break;
    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
      touch_deny();
      break;
  }
  return 0;
}

static void ui_display_verify_address(void) {
  uint64_t address = deriveAddressFromPublic(&signContext.publicKey);
  uint8_t length = deriveAddressStringRepresentation(address, lineBuffer);
  lineBuffer[length] = '\0';
  UX_DISPLAY(verify_address_ui, NULL);
}
#endif

#if defined(TARGET_NANOX)
UX_STEP_NOCB(
  ux_verify_address_flow_1_step, 
  pnn, 
  {
    &C_nanox_icon_eye,
    "Confirm",
    "address",
  });
UX_STEP_NOCB_INIT(
  ux_verify_address_flow_2_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    uint64_t address = deriveAddressFromPublic(&signContext.publicKey);
    deriveAddressStringRepresentation(address, lineBuffer);
  },
  {
    "Address",
    lineBuffer,
  });
UX_STEP_CB(
  ux_verify_address_flow_3_step,
  pb,
  {
      sendPublicKeyResponse();
      ui_idle();
  },
  {
    &C_nanox_icon_validate_14,
    "Confirm",
  });
UX_STEP_CB(
  ux_verify_address_flow_4_step,
  pb,
  touch_deny(),
  {
    &C_nanox_icon_crossmark,
    "Reject",
  });
UX_FLOW(ux_verify_address_flow,
  &ux_verify_address_flow_1_step,
  &ux_verify_address_flow_2_step,
  &ux_verify_address_flow_3_step,
  &ux_verify_address_flow_4_step);

static void ui_display_verify_address(void) {
  ux_flow_init(0, ux_verify_address_flow, NULL);
}
#endif

void handleGetPublicKey(volatile unsigned int *flags, uint8_t *bip32Path, bool confirmationRequest) {
  // Derive pubKey
  derivePrivatePublic(bip32Path, &signContext.privateKey, &signContext.publicKey);
  os_memset(&signContext.privateKey, 0, sizeof(signContext.privateKey));

  if (confirmationRequest == true) { // show address?
    // Show on ledger
    *flags |= IO_ASYNCH_REPLY;
    ui_display_verify_address();
  } else {
    createPublicKeyResponse();
  }
}
