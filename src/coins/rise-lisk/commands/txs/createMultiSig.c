//
// Created by andrea on 09/12/18.
//

#include "createMultiSig.h"
#include "../signTx.h"
#include "../../dposutils.h"
#include "../../../../io.h"
#include "../../../../ui_utils.h"
#include "os.h"

static uint8_t minKeys = 0;
static uint8_t lifetime = 0;

/**
 * Sign with address
 */
static const bagl_element_t ui_multisig_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Create", 0x01),
  TITLE_ITEM("Using", 0x02),
  TITLE_ITEM("Minimum", 0x03),
  TITLE_ITEM("Lifetime", 0x04),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_ARROW_RIGHT(0x03),
  ICON_CHECK(0x04),
  ICON_CROSS(0x00),
  LINEBUFFER,
};


static uint8_t stepProcessor_multi(uint8_t step) {
  uint64_t tmp = 0;
  os_memset(lineBuffer, 0, 50);
  switch (step) {
    case 1:
      os_memmove(lineBuffer, "Multi-sig account\0", 17);
      break;
    case 2:
      tmp = deriveAddressFromPublic(&signContext.publicKey);
      deriveAddressStringRepresentation(tmp, lineBuffer);
      break;
    case 3:
      // Min keys
      tmp = intToString(minKeys, lineBuffer);
      os_memmove(lineBuffer+tmp, " keys\0", 6);
      break;
    case 4:
      // Lifetime
      tmp = intToString(lifetime, lineBuffer);
      os_memmove(lineBuffer+tmp, " hours\0", 7);
      break;
  }
  return step + 1;
}

void tx_init_multisig(){
  minKeys = 0;
  lifetime = 0;
}

void tx_chunk_multisig(commPacket_t *packet, transaction_t *tx){
  if (minKeys == 0) {
    minKeys = packet->data[0];
    lifetime = packet->data[1];
  }
}

void tx_end_multisig(transaction_t *tx){
  ux.elements = ui_multisig_nano;
  ux.elements_count = 11;
  totalSteps = 4;
  step_processor = stepProcessor_multi;
}