#include <inttypes.h>
#include <stdbool.h>
#include "os.h"

/**
 * Derive private and public from bip32
 */
uint32_t derivePrivatePublic(uint8_t *bip32DataBuffer, cx_ecfp_private_key_t *privateKey, cx_ecfp_public_key_t *publicKey);

void sign(cx_ecfp_private_key_t *privateKey, void *whatToSign, uint32_t length, unsigned char *output);