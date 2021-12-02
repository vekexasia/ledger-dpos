#ifndef LISK_CONTEXT_H
#define LISK_CONTEXT_H

#define MAX_PAYLOAD_SIZE 448

#include "lisk_constants.h"
#include "stdbool.h"
#include "os.h"
#include "cx.h"

/**
 * Request Context
 */

typedef struct local_address {
    //uint8_t compressedPublicKey[33];
    uint32_t path[MAX_BIP32_PATH];
    uint8_t pathLength;
    uint8_t encodedPublicKey[ENCODED_PUB_KEY];
    uint8_t addressHash[ADDRESS_HASH_LENGTH];
    char addressLisk32[ADDRESS_LISK32_LENGTH];
} local_address_t;

typedef struct request_context {
    uint8_t showConfirmation;
    local_address_t account;
    //For signature
    uint16_t signableContentLength;
} request_context_t;


/**
 * Transaction Context
 */

/** Current state of an untrusted transaction hashing */
enum transaction_parsing_group_e {
    /** No transaction in progress */
    COMMON = 0x00,
    TX_ASSET = 0x01,
    CHECK_SANITY_BEFORE_SIGN = 0x04,
    TX_PARSED = 0x05
};
typedef enum transaction_parsing_group_e transaction_parsing_group_t;

enum transaction_parsing_state_e {
    /** No transaction in progress. Used also as group start*/
    BEGINNING = 0x00,
    /** Commmon Fields */
    NETWORK_ID = 0x01,
    MODULE_ID = 0x02,
    ASSET_ID = 0x03,
    NONCE = 0x04,
    FEE = 0x05,
    SENDER_PUBKEY = 0x06,

    /** Asset Fields - TX Specifics */
    PLACEHOLDER = 0x10,

    // send moduleID:2, assetID:0
    _2_0_SENDTX_AMOUNT = 0x11,
    _2_0_SENDTX_RECIPIENT_ADDR = 0x12,
    _2_0_SENDTX_DATA = 0x13,

    // reg multisig moduleID:4, assetID:0
    _4_0_REG_MULTISIG_NUMKEYS = 0x21,
    _4_0_REG_MULTISIG_KEYS = 0x22,

    // reg delegate moduleID:5, assetID:0
    _5_0_REG_DELEGATE_USERNAME = 0x31,

    // vote delegate moduleID:5, assetID:1
    _5_1_VOTE_DELEGATE_ADDRESS = 0x41,
    _5_1_VOTE_DELEGATE_AMOUNT = 0x42,

    // vote delegate moduleID:5, assetID:2
    _5_2_UNLOCK_ADDRESS = 0x51,
    _5_2_UNLOCK_AMOUNT = 0x52,
    _5_2_UNLCOK_BLOCK_HEIGHT = 0x53,

    // reclaim moduleID:1000, assetID:0
    _1000_0_RECLAIM_AMOUNT = 0x61,

    /** Ready to be signed */
    READY_TO_SIGN = 0xFF
};
typedef enum transaction_parsing_state_e transaction_parsing_state_t;


typedef struct tx_asset_2_0_transfer {
  uint64_t amount;
  unsigned char recipientAddress[ADDRESS_HASH_LENGTH];
  unsigned char data[DATA_MAX_LENGTH];
  uint32_t dataLength;
} tx_asset_2_0_transfer_t;

typedef struct tx_asset_5_0_register_delegate {
  unsigned char delegate[DELEGATE_MAX_LENGTH];
  uint32_t delegateLength;
} tx_asset_5_0_register_delegate_t;

typedef struct tx_asset_4_0_reg_multisign {
  uint32_t n_keys;
  uint32_t n_mandatoryKeys;
  uint32_t n_optionalKeys;
} tx_asset_4_0_reg_multisign_t;

typedef struct tx_asset_5_1_vote_delegate {
  uint32_t n_vote;
  uint32_t n_unvote;
  uint64_t totAmountVote;
  uint64_t totAmountUnVote;
  uint32_t lastObjectSize;
} tx_asset_5_1_vote_delegate_t;

typedef struct tx_asset_5_2_unlock_token {
  uint32_t n_unlock;
  uint64_t totAmountUnlock;
  uint32_t lastObjectSize;
} tx_asset_5_2_unlock_token_t;

typedef struct tx_asset_1000_0_reclaim {
  uint64_t amount;
} tx_asset_1000_0_reclaim_t;

typedef union tx_asset {
  tx_asset_2_0_transfer_t _2_0_transfer;
  tx_asset_4_0_reg_multisign_t _4_0_reg_multisig;
  tx_asset_5_0_register_delegate_t _5_0_reg_delegate;
  tx_asset_5_1_vote_delegate_t _5_1_vote_delegate;
  tx_asset_5_2_unlock_token_t _5_2_unlock_token;
  tx_asset_1000_0_reclaim_t _1000_0_reclaim;
  // TODO
} tx_asset_t;

typedef struct transaction_context {

    cx_sha256_t sha256;

    /** Holds digest to sign */
    uint8_t signableData[MAX_PAYLOAD_SIZE];
    uint16_t signableDataLength;

    // Common fields
    uint8_t network_id[NETWORK_ID_LENGTH];

    uint32_t module_id;
    uint32_t asset_id;
    uint64_t nonce;
    uint64_t fee;
    unsigned char senderPublicKey[ENCODED_PUB_KEY];

    tx_asset_t tx_asset;

    /** Group of the transaction parsing, type transaction_parsing_group_t */
    uint8_t tx_parsing_group;

    /** State of the transaction parsing, type transaction_parsing_state_t */
    uint8_t tx_parsing_state;

    /** Bytes parsed */
    uint16_t bytesRead;
    uint16_t bytesRemaining;

    /** Bytes to be parsed (in the chunk) */
    uint16_t bytesChunkRemaining;

    /** Current pointer to the transaction buffer for the transaction parser */
    unsigned char *bufferPointer;

    /** Bytes to be parsed (in the next chunk) */
    unsigned char saveBufferForNextChunk[100];
    uint16_t saveBufferLength;

    /** Total Tx Bytes */
    uint16_t totalTxBytes;
} transaction_context_t;

#endif
