//
// Created by andrea on 09/12/18.
//

#include "registerDelegateTx.h"
#include "../signTx.h"
#include "../../dposutils.h"
#include "../../../../io.h"
#include "../../../../ui_utils.h"

static char username[21];
static uint8_t read;

/**
 * Create second signature with address
 */
static const bagl_element_t ui_regdelegate_nano[] = {
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

static uint8_t stepProcessor_regDelegate(uint8_t step) {
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
      os_memmove(lineBuffer, username, read);
      break;
  }
  return step + 1;
}


void tx_init_regdel() {
  os_memset(username, 0, 21);
  read = 0;
}

void tx_chunk_regdel(commPacket_t *packet, transaction_t *tx) {
  // TODO: check username validity.
  os_memmove(username + read, packet->data, MIN(20-read, packet->length ));
  read += MIN(20-read, packet->length);
}

void tx_end_regdel(transaction_t *tx) {
  // chunk username
  uint8_t i;
  for (i=0; i<read; i++) {
    char c = username[i];
    if (
      !(c >= 'a' && c <= 'z') &&
      !(c >= '0' && c <= '9') &&
      !(c == '!' || c == '@' || c == '$' || c == '&' || c == '_' || c == '.')) {
      username[i] = '\0';
      read = i;
    }
  }

  // set ui stuff.
  ux.elements = ui_regdelegate_nano;
  ux.elements_count = 9;
  totalSteps = 3;
  step_processor = stepProcessor_regDelegate;

}