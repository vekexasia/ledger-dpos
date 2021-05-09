#ifndef NULS_CONTEXT_H
#define NULS_CONTEXT_H


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
    TX_SPECIFIC = 0x01,
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

    /** Data Fields - TX Specifics */
    PLACEHOLDER = 0x10,

    /** Ready to be signed */
    READY_TO_SIGN = 0xff
};
typedef enum transaction_parsing_state_e transaction_parsing_state_t;


typedef struct tx_type_specific_2_0_transfer {
  uint64_t amount;
  unsigned char recipientAddress[ADDRESS_HASH_LENGTH];
  unsigned char data[MAX_DATA_LENGTH];
} tx_type_specific_2_0_transfer_t;

typedef union tx_fields {
  tx_type_specific_2_0_transfer_t tx_2_0_transfer;
  // TODO
} tx_fields_t;

typedef struct transaction_context {

    /** Full transaction hash context */
    cx_sha256_t txHash;

    /** Holds digest to sign */
    uint8_t digest[DIGEST_LENGTH];

    uint32_t module_id;
    uint32_t asset_id;

    /** Group of the transaction parsing, type transaction_parsing_group_t */
    uint8_t tx_parsing_group;

    /** State of the transaction parsing, type transaction_parsing_state_t */
    uint8_t tx_parsing_state;

    /** Bytes parsed */
    uint16_t bytesRead;

    /** Bytes to be parsed (in the chunk) */
    uint16_t bytesChunkRemaining;

    /** Current pointer to the transaction buffer for the transaction parser */
    unsigned char *bufferPointer;

    /** Bytes to be parsed (in the next chunk) */
    unsigned char saveBufferForNextChunk[100];
    uint16_t saveBufferLength;

    /** Total Tx Bytes */
    uint16_t totalTxBytes;

    /** Fields to Display  */
    uint64_t fees;
    tx_fields_t tx_fields;
} transaction_context_t;

#endif
