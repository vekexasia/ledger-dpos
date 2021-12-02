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
      //no break is intentional
    case MODULE_ID:
      txContext.tx_parsing_state = MODULE_ID;
      // Lets be conservative
      is_available_to_parse(10);
      binaryKey = (unsigned char) transaction_get_varint();
      txContext.module_id = (uint32_t) transaction_get_varint();
      //no break is intentional
    case ASSET_ID:
      txContext.tx_parsing_state = ASSET_ID;
      // Lets be conservative
      is_available_to_parse(10);
      binaryKey = (unsigned char) transaction_get_varint();
      txContext.asset_id = (uint32_t) transaction_get_varint();
      //no break is intentional
    case NONCE:
      txContext.tx_parsing_state = NONCE;
      // Lets be conservative
      is_available_to_parse(10);
      binaryKey = (unsigned char) transaction_get_varint();
      txContext.nonce = transaction_get_varint();
    case FEE:
      txContext.tx_parsing_state = FEE;
      // Lets be conservative
      is_available_to_parse(10);
      binaryKey = (unsigned char) transaction_get_varint();
      txContext.fee = transaction_get_varint();
    case SENDER_PUBKEY:
      txContext.tx_parsing_state = SENDER_PUBKEY;
      is_available_to_parse(ENCODED_PUB_KEY + 2); // + binaryKey + byteLength
      binaryKey = (unsigned char) transaction_get_varint();
      tmpSize = (uint32_t) transaction_get_varint();
      if(tmpSize != ENCODED_PUB_KEY) {
        THROW(INVALID_PARAMETER);
      }
      transaction_memmove(txContext.senderPublicKey, txContext.bufferPointer, ENCODED_PUB_KEY);
      // Check that is equal to what we have in the request
      if(lisk_secure_memcmp(reqContext.account.encodedPublicKey, txContext.senderPublicKey, ENCODED_PUB_KEY) != 0) {
        THROW(INVALID_PARAMETER);
      }
      txContext.tx_parsing_group = TX_ASSET;
      txContext.tx_parsing_state = BEGINNING;

      break;
    default:
      THROW(INVALID_STATE);
  }
}


void check_sanity_before_sign() {
  //Sanity checks about final parsing state
  if(txContext.bytesChunkRemaining != 0 || txContext.bytesRead != txContext.totalTxBytes) {
    THROW(INVALID_STATE);
  }
  txContext.tx_parsing_group = TX_PARSED;
  txContext.tx_parsing_state = READY_TO_SIGN;
}


// Parser Utils
void cx_hash_finalize_msg() {
  unsigned char tmpHash[DIGEST_LENGTH];
  cx_sha256_t localHash;

  cx_hash(&txContext.sha256.header, CX_LAST, NULL, 0, tmpHash, DIGEST_LENGTH);
  // Rehash
  cx_sha256_init(&localHash);
  cx_hash(&localHash.header, CX_LAST, tmpHash, DIGEST_LENGTH, txContext.signableData, DIGEST_LENGTH);
  txContext.signableDataLength = DIGEST_LENGTH;
}

void transaction_offset_increase(unsigned char value) {
  memmove(txContext.signableData + txContext.bytesRead, txContext.bufferPointer, value);
  txContext.bytesRead += value;
  txContext.bufferPointer += value;
  txContext.bytesRemaining -= value;
  txContext.bytesChunkRemaining -= value;
}

void is_available_to_parse(unsigned char x) {
  if(txContext.bytesChunkRemaining < x)
    THROW(NEED_NEXT_CHUNK);
}

uint64_t transaction_get_varint(void) {
  unsigned char bytesRead;
  uint64_t result = decode_varint(txContext.bufferPointer, &bytesRead);
  transaction_offset_increase(bytesRead);
  return result;
}

int64_t transaction_get_varint_signed(void) {
  unsigned char bytesRead;
  uint64_t tmpRes = decode_varint(txContext.bufferPointer, &bytesRead);
  int64_t result = (int64_t) tmpRes;
  if(result % 2 == 0) {
    result = result / 2;
  }
  else {
    result = -(result + 1) / 2;
  }
  transaction_offset_increase(bytesRead);
  return result;
}

void transaction_memmove(unsigned char *dest, unsigned char *src, size_t nBytes)
{
  is_available_to_parse(nBytes);
  memmove(dest, src, nBytes);
  transaction_offset_increase(nBytes);
}