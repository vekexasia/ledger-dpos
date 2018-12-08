//
// Created by andrea on 08/12/18.
//

#include "getPubKey.h"

#include "../ed25519.h"
#include "../dposutils.h"
#include "../ui_elements_s.h"
#include "../../../ui_utils.h"
#include "../../../io.h"


/**
 * Creates the response for the getPublicKey command.
 * It returns both publicKey and derived Address
 */
void createPublicKeyResponse() {
  initResponse();
  getEncodedPublicKey(&signContext.publicKey, rawData);
  addToResponse(rawData, 32);
  uint64_t address = deriveAddressFromPublic(&signContext.publicKey);
  uint8_t length = deriveAddressStringRepresentation(address, (char *) (rawData + 32));

  addToResponse(rawData + 32, length);
}

static void ui_address(void) {
  uint64_t address = deriveAddressFromPublic(&signContext.publicKey);
  uint8_t length = deriveAddressStringRepresentation(address, lineBuffer);
  lineBuffer[length] = '\0';
  UX_DISPLAY(bagl_ui_address_review_nanos, NULL);
}


unsigned int bagl_ui_address_review_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      createPublicKeyResponse();

      unsigned int tx = flushResponseToIO(G_io_apdu_buffer);
      G_io_apdu_buffer[tx]   = 0x90;
      G_io_apdu_buffer[tx+1] = 0x00;

      io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx+2);

      // Display back the original UX
      ui_idle();
      return 0; // do not redraw the widget
      break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
      io_seproxyhal_touch_deny(NULL);
      break;
  }
  return 0;
}




public void handleGetPublicKey(volatile unsigned int *flags, uint8_t *bip32Path, bool confirmationRequest) {
  // Derive pubKey
  derivePrivatePublic(bip32Path, &signContext.privateKey, &signContext.publicKey);
  os_memset(&signContext.privateKey, 0, sizeof(signContext.privateKey));

  if (confirmationRequest == true) { // show address?
    // Show on ledger
    *flags |= IO_ASYNCH_REPLY;
    ui_address();
  } else {
    createPublicKeyResponse();
  }
}