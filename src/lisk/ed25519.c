#include "ed25519.h"
#include "cx.h"

#define LIBN_CURVE CX_CURVE_Ed25519
#define LIBN_SEED_KEY "ed25519 seed"


void derivePrivatePublic(local_address_t *account, cx_ecfp_private_key_t *privateKey, cx_ecfp_public_key_t *publicKey) {
  uint8_t privateKeyData[33];

  os_perso_derive_node_bip32_seed_key(
          HDW_ED25519_SLIP10, LIBN_CURVE,
          account->path, account->pathLength,
          privateKeyData, NULL,
          (unsigned char *)LIBN_SEED_KEY, sizeof(LIBN_SEED_KEY)
  );

  cx_ecfp_init_private_key(CX_CURVE_Ed25519, privateKeyData, 32, privateKey);

  cx_ecfp_generate_pair(CX_CURVE_Ed25519, publicKey, privateKey, 1);

  // Clean up!
  os_memset(privateKeyData, 0, sizeof(privateKeyData));
}

void bip32_buffer_to_array(uint8_t *bip32DataBuffer, uint8_t bip32PathLength, uint32_t *out_bip32Path) {
  if ((bip32PathLength < 0x01) || (bip32PathLength > MAX_BIP32_PATH)) {
    THROW(0x6a80 + bip32PathLength);
  }

  uint32_t i;
  for (i = 0; i < bip32PathLength; i++) {
    out_bip32Path[i] = lisk_read_u32(bip32DataBuffer, 1, 0);
    bip32DataBuffer += 4;
  }
}

void sign(cx_ecfp_private_key_t *privateKey, void *whatToSign, uint32_t length, unsigned char *output) {
  // 2nd param was null
  cx_eddsa_sign(privateKey, 0, CX_SHA512, whatToSign, length, NULL, 0, output, CX_SHA512_SIZE, NULL);
}
