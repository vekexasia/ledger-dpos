//
// Created by andrea on 09/12/18.
//

#include "createSignatureTx.h"
#include "../../dposutils.h"
#include "../../../ui_utils.h"

static uint8_t pubkey[32];
static uint8_t read;

/**
 * Create second signature with address
 */
static const bagl_element_t ui_2ndsign_nano[] = {
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


static uint8_t stepProcessor_2nd_sign(uint8_t step) {
  os_memset(lineBuffer, 0, 50);
  uint64_t address;
  uint8_t i;
  switch (step) {
    case 1:
      os_memmove(lineBuffer, "signature\0", 11);
      break;
    case 2:
      address = deriveAddressFromPublic(&signContext.publicKey);
      deriveAddressStringRepresentation(address, lineBuffer);
      break;
    case 3:
      for (i = 0; i < 3; i++) {
        toHex(pubkey[i], lineBuffer + i * 2);
      }
      lineBuffer[6] = '.';
      lineBuffer[7] = '.';
      for (i = 0; i < 3; i++) {
        toHex(pubkey[i + 32 - 3], lineBuffer + 8 + i * 2);
      }
      lineBuffer[14] = '\0';
      break;
  }
  return step + 1;
}

void tx_init_2ndsig() {
  PRINTF("tx_init_2ndsig\n");
  os_memset(pubkey, 0, 32);
  read = 0;
}

void tx_chunk_2ndsig(commPacket_t *packet, transaction_t *tx) {
  PRINTF("tx_chunk_2ndsig\n");
  PRINTF("read: %d | packet lenght: %d\n", read, packet->length);
  /*if (read + packet->length > 32) {
	  PRINTF("PRE THROW INVALID_PARAMETER\n");
    THROW(INVALID_PARAMETER);
  }*/
  PRINTF("NO THROW INVALID_PARAMETER\n");
  os_memmove(pubkey + read, packet->data, packet->length);

  read += packet->length;
}

void tx_end_2ndsig(transaction_t *tx) {
  ux.elements = ui_2ndsign_nano;
  ux.elements_count = 9;
  totalSteps = 3;
  step_processor = stepProcessor_2nd_sign;
}
