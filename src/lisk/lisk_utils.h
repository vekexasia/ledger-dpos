#ifndef LISK_UTILS_H
#define LISK_UTILS_H

#include <stdbool.h>
#include <inttypes.h>

#include "lisk_internals.h"
#include "os.h"
#include "../io.h"

typedef struct signContext_t {
    uint16_t signableContentLength;
    uint8_t reserved;

    // Holds digest to sign
    uint8_t digest[32];
} signContext_t;

extern signContext_t signContext;

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

void satoshiToString(uint64_t amount, char *out);

uint32_t setSignContext(commPacket_t *packet);

/**
 * Reads the packet for getPubKey requests, sets the reqContext.
 * @param packet the  buffer of communication packet.
 * @return number of bytes read
 */
uint32_t setReqContextForGetPubKey(commPacket_t *packet);

#endif