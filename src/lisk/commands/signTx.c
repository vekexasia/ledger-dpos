//
// Created by andrea on 09/12/18.
//

#include "signTx.h"

#include "../../ui_utils.h"
#include "../lisk_approval.h"
#include "../lisk_internals.h"
#include "../lisk_utils.h"
#include "./txs/2_0_transfer.h"
#include "./txs/4_0_register_multisignature_group.h"
#include "./txs/5_0_register_delegate.h"
#include "./txs/5_1_vote_delegate.h"
#include "./txs/5_2_unlock_token.h"
#include "./txs/1000_0_reclaim.h"
#include "./txs/common_parser.h"
#include "cx.h"

typedef void (*tx_parse_fn)();
typedef void (*tx_end_fn)();

tx_parse_fn tx_parse;
tx_end_fn tx_end;

static void ui_display_sign_tx() {
  // Delegate showing UI to the transaction type handlers
  tx_end();
}

void handleSignTxPacket(commPacket_t *packet, commContext_t *context) {

  uint32_t headerBytesRead = 0;

  // if first packet with signing header
  if (packet->first) {

    txContext.tx_parsing_state = BEGINNING;
    txContext.tx_parsing_group = COMMON;
    txContext.bufferPointer = NULL;

    // IMPORTANT this logic below only works if the first packet contains the needed information (Which it should)
    // Set signing context by parsing bip32paths and other info. function returns number of bytes read (not part of TX)
    headerBytesRead = setReqContextForSign(packet);
    txContext.totalTxBytes = reqContext.signableContentLength;
    txContext.bytesRemaining = reqContext.signableContentLength;
  }

    //insert at beginning saveBufferForNextChunk if present
    if(txContext.saveBufferLength > 0) {
      //Shift TX payload (without header) of saveBufferLength bytes on the right
      os_memmove(
          packet->data + headerBytesRead + txContext.saveBufferLength,
          packet->data + headerBytesRead,
          packet->length - headerBytesRead
      );
      //Copy saved buffer in the correct position (beginning of new tx data)
      os_memmove(
          packet->data + headerBytesRead,
          txContext.saveBufferForNextChunk,
          txContext.saveBufferLength
      );
      packet->length += txContext.saveBufferLength;
      txContext.saveBufferLength = 0;
      os_memset(txContext.saveBufferForNextChunk, 0, sizeof(txContext.saveBufferForNextChunk));
    }

    txContext.bufferPointer = packet->data + headerBytesRead;
    txContext.bytesChunkRemaining = packet->length - headerBytesRead;

    BEGIN_TRY {
      TRY {
        switch(txContext.tx_parsing_group) {
        case COMMON:
          parse_group_common();
        case TX_ASSET:
          if(txContext.tx_parsing_group != TX_ASSET) {
            THROW(INVALID_STATE);
          }
          setupTxSpecificHandlers();
          tx_parse();
        case CHECK_SANITY_BEFORE_SIGN:
          if(txContext.tx_parsing_group != CHECK_SANITY_BEFORE_SIGN) {
            THROW(INVALID_STATE);
          }
          check_sanity_before_sign();
          break;
        default:
          THROW(INVALID_STATE);
        }
      }
      CATCH_OTHER(e) {
        if(e == NEED_NEXT_CHUNK) {
          os_memmove(txContext.saveBufferForNextChunk, txContext.bufferPointer, txContext.bytesChunkRemaining);
          txContext.saveBufferLength = txContext.bytesChunkRemaining;
        } else {
          //Unexpected Error during parsing. Let the client know
          THROW(e);
        }
      }
      FINALLY {}
    }
    END_TRY;
}

void finalizeSignTx(volatile unsigned int *flags) {
  if(txContext.tx_parsing_group != TX_PARSED || txContext.tx_parsing_state != READY_TO_SIGN)
    THROW(INVALID_STATE);

  txContext.signableDataLength = txContext.bytesRead;
  ui_display_sign_tx();

  // We set the flag after displaying UI as there might be some validation code
  // that throws in the final tx_end call, that should be returned synchronously
  *flags |= IO_ASYNCH_REPLY;
}

void setupTxSpecificHandlers() {
  switch (txContext.module_id) {
  case TX_MODULE_ID_TOKEN:
  {
    if(txContext.asset_id == TX_ASSET_ID_TRANSFER) {
      tx_parse = tx_parse_specific_2_0_trasfer;
      tx_end = tx_finalize_2_0_trasfer;
    } else {
      THROW(NOT_SUPPORTED);
    }
    break;
  }
  case TX_MODULE_ID_MULTISIG:
  {
    if(txContext.asset_id == TX_ASSET_ID_REGISTER_MULTISIG_GROUP) {
      tx_parse = tx_parse_specific_4_0_register_multisignature_group;
      tx_end = tx_finalize_4_0_register_multisignature_group;
    } else {
      THROW(NOT_SUPPORTED);
    }
    break;
  }
  case TX_MODULE_ID_DPOS:
  {
    if(txContext.asset_id == TX_ASSET_ID_REGISTER_DELEGATE) {
      tx_parse = tx_parse_specific_5_0_register_delegate;
      tx_end = tx_finalize_5_0_register_delegate;
    } else if(txContext.asset_id == TX_ASSET_ID_VOTE_DELEGATE) {
      tx_parse = tx_parse_specific_5_1_vote_delegate;
      tx_end = tx_finalize_5_1_vote_delegate;
    } else if(txContext.asset_id == TX_ASSET_ID_UNLOCK_TOKEN) {
      tx_parse = tx_parse_specific_5_2_unlock_token;
      tx_end = tx_finalize_5_2_unlock_token;
    } else {
      THROW(NOT_SUPPORTED);
    }
    break;
  }
  case TX_MODULE_ID_LEGACY:
  {
    if(txContext.asset_id == TX_ASSET_ID_RECLAIM) {
      tx_parse = tx_parse_specific_1000_0_reclaim;
      tx_end = tx_finalize_1000_0_reclaim;
    } else {
      THROW(NOT_SUPPORTED);
    }
    break;
  }
  default:
    THROW(NOT_SUPPORTED);
  }
}