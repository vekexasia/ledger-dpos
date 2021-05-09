//
// Created by andrea on 08/12/18.
//

#include "impl.h"
#include "../io.h"
#include "./commands/getPubKey.h"
#include "./commands/signMsg.h"
#include "./commands/old_signTx.h"
#include "./ed25519.h"
#include "./lisk_utils.h"
#include "./ui_elements_s.h"

#define INS_GET_PUBLIC_KEY 0x04
#define INS_SIGN 0x05
#define INS_SIGN_MSG 0x06
#define INS_PING 0x08
#define INS_VERSION 0x09

void innerHandleCommPacket(commPacket_t *packet, commContext_t *context) {
  switch (context->command) {
    case INS_GET_PUBLIC_KEY:
      break;
    case INS_SIGN_MSG:
      handleSignMessagePacket(packet, context);
      break;
    case INS_SIGN:
      handleSignTxPacket(packet, context);
      break;
    case INS_PING:
    case INS_VERSION:
      // Ping and version are handled at main scope level (main.c)
      break;
    default:
      THROW(0x6a81); // INCORRECT_DATA
  }
}

bool innerProcessCommPacket(volatile unsigned int *flags, commPacket_t *lastPacket, commContext_t *context) {
  switch(context->command) {
    case INS_GET_PUBLIC_KEY:
      handleGetPublicKey(flags, lastPacket);
      break;
    case INS_SIGN_MSG:
      processSignMessage(flags);
      break;
    case INS_SIGN:
      finalizeSignTx(flags);
      break;
    default:
      THROW(0x6a82); // INCORRECT_DATA
  }
  return true;
}
