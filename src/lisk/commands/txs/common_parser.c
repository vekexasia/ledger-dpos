#include "common_parser.h"
#include "../../lisk_internals.h"

/* TX Structure:
   *
   * COMMON
   * - networkId
   * - moduleId -> 4 Bytes
   * - assetId -> 4 Bytes
   *
   * */
void parse_group_common() {

  if(txContext.tx_parsing_group != COMMON) {
    THROW(INVALID_STATE);
  }

  switch(txContext.tx_parsing_state) {
    case BEGINNING:
      // Reset transaction state
    case NETWORK_ID:
      txContext.tx_parsing_state = NETWORK_ID;
      // TODO
      // is_available_to_parse(2);
      // transaction_offset_increase(2);
      //no break is intentional
    case MODULE_ID:
      txContext.tx_parsing_state = MODULE_ID;
      is_available_to_parse(4);
      transaction_offset_increase(4);
      //no break is intentional
      break;
    case ASSET_ID:
      txContext.tx_parsing_state = ASSET_ID;
      is_available_to_parse(4);
      transaction_offset_increase(4);
      //no break is intentional
      break;
    default:
      THROW(INVALID_STATE);
  }
}


void check_sanity_before_sign() {
  if(txContext.tx_parsing_group != CHECK_SANITY_BEFORE_SIGN) {
    THROW(INVALID_STATE);
  }
  //Sanity checks about final parsing state
  if(txContext.bytesChunkRemaining != 0 || txContext.bytesRead != txContext.totalTxBytes) {
    THROW(INVALID_STATE);
  }
  txContext.tx_parsing_group = TX_PARSED;
  txContext.tx_parsing_state = READY_TO_SIGN;
}


// Parser Utils
void cx_hash_finalize(unsigned char *dest, unsigned char size) {
  unsigned char fake[1];
  unsigned char tmpHash[DIGEST_LENGTH];
  cx_sha256_t localHash;

  cx_hash(&txContext.txHash.header, CX_LAST, fake, 0, tmpHash, DIGEST_LENGTH);
  // Rehash
  cx_sha256_init(&localHash);
  cx_hash(&localHash.header, CX_LAST, tmpHash, DIGEST_LENGTH, dest, size);
}

void cx_hash_increase(unsigned char value) {
  cx_hash(&txContext.txHash.header, 0, txContext.bufferPointer, value, NULL, 0);
}

void transaction_offset_increase(unsigned char value) {
  cx_hash_increase(value);
  txContext.bytesRead += value;
  txContext.bufferPointer += value;
  txContext.bytesChunkRemaining -= value;
}

void is_available_to_parse(unsigned char x) {
  if(txContext.bytesChunkRemaining < x)
    THROW(NEED_NEXT_CHUNK);
}

unsigned long int transaction_get_varint(void) {
  unsigned char firstByte;
  is_available_to_parse(1);
  firstByte = *txContext.bufferPointer;
  if (firstByte < 0xFD) {
    transaction_offset_increase(1);
    return firstByte;
  } else if (firstByte == 0xFD) {
    unsigned long int result;
    transaction_offset_increase(1);
    is_available_to_parse(2);
    result = (unsigned long int)(*txContext.bufferPointer) |
             ((unsigned long int)(*(txContext.bufferPointer + 1)) << 8);
    transaction_offset_increase(2);
    return result;
  } else if (firstByte == 0xFE) {
    unsigned long int result;
    transaction_offset_increase(1);
    is_available_to_parse(4);
    result = nuls_read_u32(txContext.bufferPointer, 0, 0);
    transaction_offset_increase(4);
    return result;
  } else {
    // L_DEBUG_APP(("Varint parsing failed\n"));
    THROW(INVALID_PARAMETER);
  }
}
