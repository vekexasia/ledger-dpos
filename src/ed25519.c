#include "stdbool.h"
#include "os.h"
#include "cx.h"
#define MAX_BIP32_PATH 10

/**
 *
 * @param bip32DataBuffer
 * @param privateKey
 * @param publicKey
 * @return read data to derive private public
 */
uint32_t derivePrivatePublic(uint8_t *bip32DataBuffer, cx_ecfp_private_key_t *privateKey, cx_ecfp_public_key_t *publicKey) {
  uint8_t bip32PathLength = bip32DataBuffer[0];
  uint32_t i;
  uint32_t bip32Path[MAX_BIP32_PATH];
  uint8_t privateKeyData[33];

  uint32_t readData = 1; // 1byte length
  bip32DataBuffer += 1;

  if ((bip32PathLength < 0x01) || (bip32PathLength > MAX_BIP32_PATH)) {
    THROW(0x6a80);
  }


  for (i = 0; i < bip32PathLength; i++) {
    bip32Path[i] = (bip32DataBuffer[0] << 24) | (bip32DataBuffer[1] << 16) |
                   (bip32DataBuffer[2] << 8) | (bip32DataBuffer[3]);
    bip32DataBuffer += 4;
    readData += 4;
  }
  os_perso_derive_node_bip32(CX_CURVE_Ed25519, bip32Path, bip32PathLength,
                             privateKeyData,
                             NULL /* CHAIN CODE */);

  cx_ecfp_init_private_key(CX_CURVE_Ed25519, privateKeyData, 32, privateKey);

  cx_ecfp_generate_pair(CX_CURVE_Ed25519, publicKey, privateKey, 1);

  // Clean up!
  os_memset(privateKeyData, 0, sizeof(privateKeyData));

  return readData;
}


/**
 * Signs an arbitrary message given the privateKey and the info
 * @param privateKey the privateKey to be used
 * @param whatToSign the message to sign
 * @param length the length of the message ot sign
 * @param isTx wether we're signing a tx or a text
 * @param output
 */
void sign(cx_ecfp_private_key_t *privateKey, void *whatToSign, uint32_t length, unsigned char *output, bool isTx) {
  if (isTx == true) {
    uint8_t hash[32];
    cx_hash_sha256(whatToSign, length, hash);
    cx_eddsa_sign(privateKey, NULL, CX_SHA512, hash, 32, NULL, 0, output, 0);
  } else {
    cx_eddsa_sign(privateKey, NULL, CX_SHA512, whatToSign, length, NULL, 0, output, 0);
  }

}