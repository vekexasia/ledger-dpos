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
 * Get first 160 bits of SHA256(pubkey)
 * @param publicKey the raw public key containing both coordinated for the elliptic curve
 * @param encoded result holder
 */
void getPubKeyHash160(uint8_t *encodedPublicKey, uint8_t *encoded);

void uint64_to_string(uint64_t amount, char *out);
void satoshiToString(uint64_t amount, char *out);

uint32_t setSignContext(commPacket_t *packet);

/**
 * Reads the packet for Sign requests (tx and msg), sets the reqContext.
 * @param packet the  buffer of communication packet.
 * @return number of bytes read
 */
uint32_t setReqContextForSign(commPacket_t *packet);

/**
 * Reads the packet for getPubKey requests, sets the reqContext.
 * @param packet the  buffer of communication packet.
 * @return number of bytes read
 */
uint32_t setReqContextForGetPubKey(commPacket_t *packet);

/**
 * Kill Private key and reset all the contexts (reqContext, txContext, commContext, commPacket)
 */
void reset_contexts();

#endif