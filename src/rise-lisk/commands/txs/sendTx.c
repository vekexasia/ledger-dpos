//
// Created by andrea on 09/12/18.
//

#include "sendTx.h"
#include "../../dposutils.h"
#include "../../../io.h"
#include "../../../ui_utils.h"
#include "../../ui_elements_s.h"
#include "../signTx.h"

static char message[64];
static uint8_t curLength;
/**
 * Sign with address
 */
static const bagl_element_t ui_send_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Send from", 0x01),
  TITLE_ITEM("To", 0x02),
  TITLE_ITEM("Message", 0x03),
  TITLE_ITEM("Amount", 0x04),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_ARROW_RIGHT(0x03),
  ICON_CHECK(0x04),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

static uint8_t stepProcessor_send_ui(uint8_t step) {
  uint64_t address;
  os_memset(lineBuffer, 0, 50);
  switch (step) {
    case 1:
      address = deriveAddressFromPublic(&signContext.publicKey);
      deriveAddressStringRepresentation(address, lineBuffer);
      return 2;
    case 2:
      deriveAddressStringRepresentation(transaction.recipientId, lineBuffer);
      if (curLength == 0) {
        return 4;
      }
      return 3;
    case 3:
      os_memmove(lineBuffer, message, MIN(50, curLength));
      // ellipsis
      if (curLength > 47) {
        os_memmove(lineBuffer + 47, "...", 3);
      }
      return 4;
    case 4:
      satoshiToString(transaction.amountSatoshi, lineBuffer);
  }
  return 0;
}


void tx_init_send() {
  curLength = 0;
  os_memset(message, 0, 64);
}

void tx_chunk_send(commPacket_t *packet, transaction_t *tx) {
  os_memmove(message+curLength, packet->data, MIN(64 - curLength, packet->length));
}

void tx_end_send(transaction_t *tx) {
  ux.elements = ui_send_nano;
  ux.elements_count = 11;
  totalSteps = 4;
  step_processor = stepProcessor_send_ui;
}
