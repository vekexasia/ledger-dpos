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
static uint16_t totalLengthAfterAsset;

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

static uint8_t stepProcessor_send(uint8_t step) {
  if (step == 2 && curLength == 0) {
    return 4;
  }
  return step + 1;
}

static void uiProcessor_send(uint8_t step) {
  uint64_t address;
  os_memset(lineBuffer, 0, 50);
  switch (step) {
    case 1:
      address = deriveAddressFromPublic(&signContext.publicKey);
      deriveAddressStringRepresentation(address, lineBuffer);
      break;
    case 2:
      deriveAddressStringRepresentation(transaction.recipientId, lineBuffer);
      break;
    case 3:
      os_memmove(lineBuffer, message, MIN(50, curLength));
      // ellipsis
      if (curLength > 46) {
        os_memmove(lineBuffer + 46, "...\0", 4);
      }
      break;
    case 4:
      satoshiToString(transaction.amountSatoshi, lineBuffer);
  }
}

void tx_init_send() {
  curLength = 0;
  totalLengthAfterAsset = 0;
  os_memset(message, 0, 64);
}

void tx_chunk_send(uint8_t * data, uint8_t length, commPacket_t *sourcePacket, transaction_t *tx) {
  uint8_t toReadLength = MAX(0, MIN(64 - curLength, length));
  os_memmove(message+curLength, data, toReadLength);
  curLength += toReadLength;
  totalLengthAfterAsset += length;
}

void tx_end_send(transaction_t *tx) {
  // Remove signature and/or secondSignature from message.
  curLength = totalLengthAfterAsset - (totalLengthAfterAsset / 64) * 64;
  ux.elements = ui_send_nano;
  ux.elements_count = 11;
  totalSteps = 4;
  step_processor = stepProcessor_send;
  ui_processor = uiProcessor_send;
}
