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

  unsigned char binaryKey = 0;
  uint32_t tmpSize = 0;

  switch(txContext.tx_parsing_state) {
    case BEGINNING:
      // Reset transaction state
    case NETWORK_ID:
      txContext.tx_parsing_state = NETWORK_ID;
      transaction_memmove(txContext.network_id, txContext.bufferPointer, NETWORK_ID_LENGTH);
      PRINTF("txContext.network_id:\n %.*H \n\n", NETWORK_ID_LENGTH, txContext.network_id);
      //no break is intentional
    case MODULE_ID:
      txContext.tx_parsing_state = MODULE_ID;
      // Lets be conservative
      is_available_to_parse(10);
      binaryKey = (unsigned char) transaction_get_varint();
      PRINTF("binaryKey MODULE_ID:\n %X \n\n", binaryKey);
      txContext.module_id = (uint32_t) transaction_get_varint();
      PRINTF("txContext.module_id:\n %u \n\n", txContext.module_id);
      //no break is intentional
    case ASSET_ID:
      txContext.tx_parsing_state = ASSET_ID;
      // Lets be conservative
      is_available_to_parse(10);
      binaryKey = (unsigned char) transaction_get_varint();
      PRINTF("binaryKey ASSET_ID:\n %X \n\n", binaryKey);
      txContext.asset_id = (uint32_t) transaction_get_varint();
      PRINTF("txContext.asset_id:\n %u \n\n", txContext.asset_id);
      // TODO Check if moduleID and assetId provided are supported
      //no break is intentional
    case NONCE:
      txContext.tx_parsing_state = NONCE;
      // Lets be conservative
      is_available_to_parse(10);
      binaryKey = (unsigned char) transaction_get_varint();
      PRINTF("binaryKey NONCE:\n %X \n\n", binaryKey);
      txContext.nonce = transaction_get_varint();
      {
        os_memset(lineBuffer, 0, sizeof(lineBuffer));
        uint64_to_string(txContext.nonce, lineBuffer);
        PRINTF("txContext.nonce:\n %s \n\n", lineBuffer);
      }
    case FEE:
      txContext.tx_parsing_state = FEE;
      // Lets be conservative
      is_available_to_parse(10);
      binaryKey = (unsigned char) transaction_get_varint();
      PRINTF("binaryKey FEE:\n %X \n\n", binaryKey);
      txContext.fee = transaction_get_varint();
      {
        os_memset(lineBuffer, 0, sizeof(lineBuffer));
        uint64_to_string(txContext.fee, lineBuffer);
        PRINTF("txContext.fee:\n %s \n\n", lineBuffer);
      }
    case SENDER_PUBKEY:
      txContext.tx_parsing_state = SENDER_PUBKEY;
      is_available_to_parse(ADDRESS_HASH_LENGTH + 2); // + binaryKey + byteLength
      binaryKey = (unsigned char) transaction_get_varint();
      PRINTF("binaryKey SENDER_PUBKEY:\n %X \n\n", binaryKey);
      tmpSize = (uint32_t) transaction_get_varint();
      PRINTF("txContext.senderPublicKey tmpSize:\n %u \n\n", tmpSize);
      if(tmpSize != ADDRESS_HASH_LENGTH) {
        THROW(INVALID_PARAMETER);
      }
      transaction_memmove(txContext.senderPublicKey, txContext.bufferPointer, ADDRESS_HASH_LENGTH);
      PRINTF("txContext.senderPublicKey:\n %.*H \n\n", ADDRESS_HASH_LENGTH, txContext.senderPublicKey);
      txContext.tx_parsing_state = BEGINNING;
      txContext.tx_parsing_group = TX_SPECIFIC;

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

/*
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
    result = lisk_read_u32(txContext.bufferPointer, 0, 0);
    transaction_offset_increase(4);
    return result;
  } else {
    transaction_offset_increase(1);
    // uint64?
    // L_DEBUG_APP(("Varint parsing failed\n"))
    // PRINTF("Varint parsing failed \n\n");
    // THROW(INVALID_PARAMETER);
  }
}
 */

uint64_t transaction_get_varint(void) {
  unsigned char bytesRead;
  PRINTF("buffer before decode_varint: %.*H \n", 8, txContext.bufferPointer);
  uint64_t result = decode_varint(txContext.bufferPointer, &bytesRead);
  PRINTF("transaction_get_varint bytesRead: %u \n", bytesRead);
  PRINTF("transaction_get_varint result: %.*H  \n", 8, &result);
  transaction_offset_increase(bytesRead);
  return result;
}

void transaction_memmove(unsigned char *dest, unsigned char *src, size_t nBytes)
{
  is_available_to_parse(nBytes);
  os_memmove(dest, src, nBytes);
  transaction_offset_increase(nBytes);
}