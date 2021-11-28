//
// Created by andrea on 08/12/18.
//

#include "getPubKey.h"

#include "../ed25519.h"
#include "../lisk_approval.h"
#include "../lisk_internals.h"
#include "../lisk_utils.h"
#include "../ui_elements_s.h"
#include "os.h"

/**
 * Creates the response for the getPublicKey command.
 * It returns both publicKey and derived Address
 */
static void createPublicKeyResponse() {
  initResponse();
  addToResponse(reqContext.account.encodedPublicKey, ENCODED_PUB_KEY);
  addToResponse(reqContext.account.addressHash, ADDRESS_HASH_LENGTH);
  addToResponse(reqContext.account.addressLisk32, ADDRESS_LISK32_LENGTH);
}

static void sendPublicKeyResponse() {
  createPublicKeyResponse();

  unsigned int tx = flushResponseToIO(G_io_apdu_buffer);
  G_io_apdu_buffer[tx]   = 0x90;
  G_io_apdu_buffer[tx+1] = 0x00;

  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx+2);
}

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
    memset(lineBuffer, 0, sizeof(lineBuffer));
    os_memmove(lineBuffer, &reqContext.account.addressLisk32, ADDRESS_LISK32_LENGTH);
    lineBuffer[ADDRESS_LISK32_LENGTH] = '\0';
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

void handleGetPublicKey(volatile unsigned int *flags, commPacket_t *packet) {
  //reset contexts
  memset(&reqContext, 0, sizeof(reqContext));
  memset(&txContext, 0, sizeof(txContext));

  setReqContextForGetPubKey(packet); //address is derived there

  if (reqContext.showConfirmation == true) { // show address?
    // Show on ledger
    *flags |= IO_ASYNCH_REPLY;
    ui_display_verify_address();
  } else {
    createPublicKeyResponse();
  }
}
