#include "lisk_approval.h"

#include "lisk_internals.h"
#include "./ui_elements_s.h"
#include "os_io_seproxyhal.h"

/**
 * Sign with address
 */
void touch_deny() {
  G_io_apdu_buffer[0] = 0x69;
  G_io_apdu_buffer[1] = 0x85;

  // Allow restart of operation
  commContext.started = false;
  commContext.read = 0;

  // Kill private key - shouldn't be necessary but just in case.
  os_memset(&private_key, 0, sizeof(private_key));

  // Send back the response, do not restart the event loop
  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);

  // Display back the original UX
  ui_idle();
}

void old_touch_approve() {
  uint8_t signature[64];
  sign(&private_key, signContext.digest, 32, signature);
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

void touch_approve() {
  uint8_t signature[SIGNATURE_LENGTH];
  sign(&private_key, txContext.digest, DIGEST_LENGTH, signature);
  initResponse();
  addToResponse(signature, SIGNATURE_LENGTH);

  unsigned int tx = flushResponseToIO(G_io_apdu_buffer);
  G_io_apdu_buffer[tx]   = 0x90;
  G_io_apdu_buffer[tx+1] = 0x00;

  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx+2);

  reset_contexts();

  // Display back the original UX
  ui_idle();
}
