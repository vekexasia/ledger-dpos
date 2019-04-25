//
// Created by andrea on 09/12/18.
//

#include "voteTx.h"
#include "../../../ui_utils.h"
#include "../signTx.h"
#include "../../dposutils.h"

static uint8_t votesAdded = 0;
static uint8_t votesRemoved = 0;

/**
 * Create second signature with address
 */

#if defined(TARGET_NANOS)

const bagl_element_t ui_vote_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Vote from", 0x01),
  TITLE_ITEM("Added", 0x02),
  TITLE_ITEM("Removed", 0x03),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_CHECK(0x03),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

#endif

static void stepProcessor_vote(uint8_t step) {
  uint64_t address;
  os_memset(lineBuffer, 0, 50);
  switch (step) {
    case 1:
      address = deriveAddressFromPublic(&signContext.publicKey);
      deriveAddressStringRepresentation(address, lineBuffer);
      break;
    case 2:
      // Added number
      intToString(votesAdded, lineBuffer);
      break;
    case 3:
      // Removed number
      intToString(votesRemoved, lineBuffer);
      break;
  }
}

void tx_init_vote() {
  votesAdded = 0;
  votesRemoved = 0;
}

void tx_chunk_vote(uint8_t * data, uint8_t length, commPacket_t *sourcePacket, transaction_t *tx) {
  uint16_t i;
  for (i=0; i< length; i++) {
    if (data[i] == '+') {
      votesAdded++;
    } else if (data[i] == '-') {
      votesRemoved++;
    }
  }
}

#if defined(TARGET_NANOS)

void tx_end_vote(transaction_t *tx) {
  ux.elements = ui_vote_nano;
  ux.elements_count = 9;
  totalSteps = 3;
  ui_processor = stepProcessor_vote;
}

#elif defined(TARGET_NANOX)

void tx_end_vote(transaction_t *tx){
 THROW(ERROR_FEATURE_NOT_YET_SUPPORTED);
}

#endif