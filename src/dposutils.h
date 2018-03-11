#include <stdbool.h>
#define TXTYPE_SEND 0
#define TXTYPE_CREATESIGNATURE 1
#define TXTYPE_REGISTERDELEGATE 2
#define TXTYPE_VOTE 3
#define TXTYPE_CREATEMULTISIG 4


#define ADDRESS_SUFFIX "L"
#define ADDRESS_SUFFIX_LENGTH 1
#ifndef STRUCT_TX
#define STRUCT_TX
typedef struct transaction {
    uint8_t type;
    uint64_t recipientId;
    uint64_t amountSatoshi;
    char shortDesc[21];
};

#endif

/**
 * Gets a bigendian representation of the usable publicKey
 * @param publicKey the raw public key containing both coordinated for the elliptic curve
 * @param encoded result holder
 */
void getEncodedPublicKey(cx_ecfp_public_key_t *publicKey, uint8_t *encoded);


/**
 * Derive address uint64_t value from a byte array.
 * @param source source byte array where the next 8 bytes represent the address
 * @param rev whether or not to reverse the info. (publickey address derivation => true)
 * @return the encoded address in uint64_t data.
 */
uint64_t deriveAddressFromUintArray(uint8_t *source, bool rev);


/**
 * Returns a string representation of the encoded Address
 * @param encodedAddress address to represent
 * @param output the output where the string representation will be returned
 * @return the length of the string representation.
 */
uint8_t deriveAddressStringRepresentation(uint64_t encodedAddress, char *output);


/**
 * Derive address associated to the specific publicKey.
 * @param publicKey original publicKey
 * @return the encoded address.
 */
uint64_t deriveAddressFromPublic(cx_ecfp_public_key_t *publicKey);

/**
 * Parse a transaction.
 * @param txBytes transaction bytes as returned by getBytes.
 * @param length length of bytes
 * @param hasRequesterPublicKey true if the tx has requesterPublicKey field set
 * @param out output
 */
void parseTransaction(uint8_t *txBytes, uint32_t length, bool hasRequesterPublicKey, struct transaction *out);

