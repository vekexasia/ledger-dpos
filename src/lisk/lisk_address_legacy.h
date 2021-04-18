#ifndef _LISK_LEGACY_ADDR_H_
#define _LISK_LEGACY_ADDR_H_ 1

#include <stdint.h>

#include "lisk_internals.h"
#include "os.h"

/**
 * Derive address uint64_t value from a byte array.
 * @param source source byte array where the next 8 bytes represent the address
 * @param rev whether or not to reverse the info. (publickey address derivation => true)
 * @return the encoded address in uint64_t data.
 */
uint64_t deriveLegacyAddressFromUintArray(uint8_t *source, bool rev);


/**
 * Returns a string representation of the encoded Address
 * @param encodedAddress address to represent
 * @param output the output where the string representation will be returned
 * @return the length of the string representation.
 */
uint8_t deriveLegacyAddressStringRepresentation(uint64_t encodedAddress, char *output);


/**
 * Derive address associated to the specific publicKey.
 * @param publicKey original publicKey
 * @return the encoded address.
 */
uint64_t deriveLegacyAddressFromPublic(cx_ecfp_public_key_t *publicKey);

#endif