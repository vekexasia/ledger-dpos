#include <inttypes.h>
#include <stdbool.h>

#include "lisk_internals.h"
#include "os.h"

/**
 * Derive private and public from bip32
* @param account that contains path information
* @param privateKey
* @param publicKey
*/
void derivePrivatePublic(local_address_t *account, cx_ecfp_private_key_t *privateKey, cx_ecfp_public_key_t *publicKey);

/**
 * From Buffer to Path Array
 * @param bip32PathBuffer
 * @param bip32PathLength
 * @param out_bip32PathArray
 */
void bip32_buffer_to_array(
    uint8_t *bip32DataBuffer,
    uint8_t bip32PathLength,
    uint32_t *out_bip32Path);

/**
 * Signs an arbitrary message given the privateKey and the info
 * @param privateKey the privateKey to be used
 * @param whatToSign the message to sign
 * @param length the length of the message ot sign
 * @param output
 */
void sign(cx_ecfp_private_key_t *privateKey, void *whatToSign, uint32_t length, unsigned char *output);