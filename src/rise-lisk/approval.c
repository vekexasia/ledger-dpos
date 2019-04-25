//
// Created by andrea on 09/12/18.
//

#include "approval.h"
#include "../ui_utils.h"
#include "../io.h"
#include "./dposutils.h"
#include "./ui_elements_s.h"
#include "os_io_seproxyhal.h"
#include "ed25519.h"


void touch_deny() {
  G_io_apdu_buffer[0] = 0x69;
  G_io_apdu_buffer[1] = 0x85;

  // Allow restart of operation
  commContext.started = false;
  commContext.read = 0;

  // Kill private key - shouldn't be necessary but just in case.
  os_memset(&signContext.privateKey, 0, sizeof(signContext.privateKey));

  // Send back the response, do not restart the event loop
  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
  // Display back the original UX
  ui_idle();
}

void touch_approve() {
  uint8_t signature[64];
  sign(&signContext.privateKey, signContext.digest, 32, signature);
  initResponse();
  addToResponse(signature, 64);

  // Allow restart of operation
  commContext.started = false;
  commContext.read = 0;

  unsigned int tx = flushResponseToIO(G_io_apdu_buffer);
  G_io_apdu_buffer[tx]   = 0x90;
  G_io_apdu_buffer[tx+1] = 0x00;

  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx+2);

  // Display back the original UX
  ui_idle();
}

#if defined(TARGET_NANOS)

/**
 * Sign with address
 */
static const bagl_element_t approval_nano_ui[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Sign with", 0x01),
  ICON_CHECK(0x00),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

unsigned int approval_nano_ui_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      touch_approve();
      break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
      touch_deny();
      break;
  }
  return 0;
}

#elif defined(TARGET_NANOX)
//////////////////////////////////////////////////////////////////////

UX_STEP_VALID(
    ux_approval_1_step,
    pnn,
    touch_approve(),
    {
      &C_icon_validate_14,
      "Sign with",
      lineBuffer,
    });
UX_STEP_VALID(
    ux_approval_2_step, 
    pb, 
    touch_deny(),
    {
      &C_icon_crossmark,
      "Reject",
    });
UX_FLOW(ux_approval,
  &ux_approval_1_step,
  &ux_approval_2_step
);

//////////////////////////////////////////////////////////////////////
#endif

void ui_approval() {
  uint64_t address = deriveAddressFromPublic(&signContext.publicKey);
  deriveAddressStringRepresentation(address, lineBuffer);
#if defined(TARGET_NANOS)
  UX_DISPLAY(approval_nano_ui, NULL)
#elif defined(TARGET_NANOX)
  ux_flow_init(0, ux_approval, NULL);
#endif // #if TARGET_ID
}



