#include "stdbool.h"
#include "os.h"
#include "cx.h"
#define MAX_BIP32_PATH 10

#define INS_GET_PUBLIC_KEY 0x04
#define INS_SIGN 0x05
#define INS_SIGN_MSG 0x06

#define TXTYPE_SEND 0
#define TXTYPE_CREATESIGNATURE 1
#define TXTYPE_REGISTERDELEGATE 2
#define TXTYPE_VOTE 3
#define TXTYPE_CREATEMULTISIG 4


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
void sign(cx_ecfp_private_key_t *privateKey, void *whatToSign, uint32_t length, unsigned char *output) {
  cx_eddsa_sign(privateKey, NULL, CX_SHA512, whatToSign, length, NULL, 0, output, CX_SHA512_SIZE, NULL);

}


void ui_signtx(uint8_t steps, uint8_t uielements) {
  currentStep = 1;
  totalSteps = steps;
  pcallback(&signContext, 1);
  // IMPLEMENT BLUE
  ux.elements = bagl_ui_sign_tx;
  ux.elements_count = uielements;
  ux.button_push_handler = bagl_ui_sign_tx_button;
  ux.elements_preprocessor = signprocessor;
  UX_WAKE_UP();
  UX_REDISPLAY();
}

/**
 * Handles the sign transaction command.
 * @param dataBuffer
 */
void handleSignTX(uint8_t *dataBuffer) {
  getSignContext(dataBuffer, &signContext);
  parseTransaction(signContext.msg, signContext.msgLength, signContext.hasRequesterPublicKey, &signContext.tx);
  signContext.isTx = true;

  if (signContext.tx.type == TXTYPE_SEND) {
    pcallback = lineBufferSendTxProcessor;
    bagl_ui_sign_tx = bagl_ui_approval_send_nanos;
    ui_signtx(4, sizeof(bagl_ui_approval_send_nanos)/sizeof(bagl_ui_approval_send_nanos[0]));
  } else if (signContext.tx.type == TXTYPE_REGISTERDELEGATE) {
    pcallback = lineBufferRegDelegateTxProcessor;
    bagl_ui_sign_tx = bagl_ui_regdelegate_nanos;
    ui_signtx(3, sizeof(bagl_ui_regdelegate_nanos)/sizeof(bagl_ui_regdelegate_nanos[0]));
  } else if (signContext.tx.type == TXTYPE_CREATESIGNATURE) {
    pcallback = lineBufferSecondSignProcessor;
    bagl_ui_sign_tx = bagl_ui_secondsign_nanos;
    ui_signtx(3, sizeof(bagl_ui_secondsign_nanos)/sizeof(bagl_ui_secondsign_nanos[0]));
  } else if (signContext.tx.type == TXTYPE_VOTE) {
    pcallback = lineBufferVoteProcessor;
    bagl_ui_sign_tx = bagl_ui_vote_nanos;
    ui_signtx(3, sizeof(bagl_ui_vote_nanos)/sizeof(bagl_ui_vote_nanos[0]));
  } else if (signContext.tx.type == TXTYPE_CREATEMULTISIG) {
    pcallback = lineBufferMultisigProcessor;
    bagl_ui_sign_tx = bagl_ui_multisignature_nanos;
    ui_signtx(4, sizeof(bagl_ui_multisignature_nanos)/sizeof(bagl_ui_multisignature_nanos[0]));
  } else {
    THROW(0x6a80);
  }
}

void innerHandleCommPacket(comPacket_t packet, commContext_t context) {
  switch (context.command) {
    case INS_GET_PUBLIC_KEY:

  }
}

bool innerProcessCommPacket(volatile unsigned int *flags, commPacket_t packet, commContext_t context) {

}
