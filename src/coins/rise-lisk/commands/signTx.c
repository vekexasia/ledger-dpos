//
// Created by andrea on 09/12/18.
//

#include "signTx.h"
#include "../../../io.h"
#include "../dposutils.h"
#include "./txs/sendTx.h"
#include "./txs/voteTx.h"
#include "./txs/createMultiSig.h"
#include "./txs/createSignatureTx.h"
#include "./txs/registerDelegateTx.h"
#include "../approval.h"
#include "../../../ui_utils.h"

#define TXTYPE_SEND 0
#define TXTYPE_CREATESIGNATURE 1
#define TXTYPE_REGISTERDELEGATE 2
#define TXTYPE_VOTE 3
#define TXTYPE_CREATEMULTISIG 4

typedef void (*tx_init_fn)();
typedef void (*tx_chunk_fn)(commPacket_t *packet, transaction_t *tx);
typedef void (*tx_end_fn)(transaction_t *tx);

tx_init_fn tx_init;
tx_chunk_fn tx_chunk;
tx_end_fn tx_end;

step_processor_fn step_processor;

static const bagl_element_t sign_message_ui[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Verify text", 0x00),
  ICON_CROSS(0x00),
  ICON_CHECK(0x00),
  LINEBUFFER,
};
static cx_sha256_t txHash;
transaction_t transaction;

/**
 * Used to verify what is going to be displayed
 * @param element
 * @return 0 or 1
 */
static const int signtx_ui_preprocessor(const bagl_element_t *element) {
  if (element->component.userid == 0x0) {
    return 1;
  }

  if ((element->component.type & (~BAGL_FLAG_TOUCHABLE)) == BAGL_NONE) {
    return 0;
  }
  if (element->component.userid == currentStep) {
    return 1;
  }
  return 0;
}
static void ui_sign_tx_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      if (currentStep < totalSteps) {
        currentStep = step_processor(currentStep);
        UX_REDISPLAY();
      } else {
        touch_approve();
      }
      break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
      touch_deny(NULL);
      break;
  }
}

void handleSignTxPacket(commPacket_t *packet, commContext_t *context) {
  // if first packet with signing header
  if ( packet->first ) {
    // IMPORTANT this logic below only works if the first packet contains the needed information (Which it should)
    // Set signing context from first packet and patches the .data and .length by removing header length
    setSignContext(packet);

    // Reset sha256
    cx_sha256_init(&txHash);

    // fetch transaction type
    transaction.type = packet->data[0];
    uint32_t recIndex = 1 /*type*/
                        + 4 /*timestamp*/
                        + 32 /*senderPublicKey */
                        + (signContext.reserved == true ? 32 : 0) /*requesterPublicKey */;
    transaction.recipientId = deriveAddressFromUintArray(packet->data + recIndex, false);
    uint32_t i = 0;

    transaction.amountSatoshi = 0;
    for (i = 0; i < 8; i++) {
      transaction.amountSatoshi |= ((uint64_t )packet->data[recIndex + 8 + i]) << (8*i);
    }

    os_memset(transaction.shortDesc, 0, 22);

    if (transaction.type == TXTYPE_SEND) {
      tx_init  = tx_init_send;
      tx_chunk = tx_chunk_send;
      tx_end   = tx_end_send;
    } else if (transaction.type == TXTYPE_CREATEMULTISIG ) {
      tx_init  = tx_init_multisig;
      tx_chunk = tx_chunk_multisig;
      tx_end   = tx_end_multisig;
    } else if (transaction.type == TXTYPE_VOTE ) {
      tx_init  = tx_init_vote;
      tx_chunk = tx_chunk_vote;
      tx_end   = tx_end_vote;
    } else if (transaction.type == TXTYPE_REGISTERDELEGATE ) {
      tx_init  = tx_init_regdel;
      tx_chunk = tx_chunk_regdel;
      tx_end   = tx_end_regdel;
    } else if (transaction.type == TXTYPE_CREATESIGNATURE) {
      tx_init  = tx_init_2ndsig;
      tx_chunk = tx_chunk_2ndsig;
      tx_end   = tx_end_2ndsig;
    }
    tx_init();
  }

  tx_chunk(packet, &transaction);

  cx_hash(&txHash, NULL, packet->data, packet->length, NULL, NULL);
}

void finalizeSignTx(volatile unsigned int *flags) {
  uint8_t finalHash[32];

  // Close first sha256
  cx_hash(&txHash, CX_LAST, finalHash, 0, NULL, NULL);

  os_memmove(&signContext.digest, txHash.acc, 32);

  // Init user flow.
  *flags |= IO_ASYNCH_REPLY;
  tx_end(&transaction);
  currentStep = 1;

  ux.button_push_handler = ui_sign_tx_button;
  ux.elements_preprocessor = signtx_ui_preprocessor;

  UX_WAKE_UP();
  UX_REDISPLAY();
}
