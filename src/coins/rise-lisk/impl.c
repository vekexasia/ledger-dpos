//
// Created by andrea on 08/12/18.
//

#include "impl.h"
#include "../../io.h"
#include "./ed25519.h"
#include "./dposutils.h"
#include "./ui_elements_s.h"

#define INS_GET_PUBLIC_KEY 0x04
#define INS_SIGN 0x05
#define INS_SIGN_MSG 0x06





void innerHandleCommPacket(comPacket_t packet, commContext_t context) {
  switch (context.command) {
    case INS_GET_PUBLIC_KEY:

  }
}


bool innerProcessCommPacket(volatile unsigned int *flags, commPacket_t lastPacket, commContext_t context) {
  uint8_t tmp, tmp2;

  switch(context.command) {
    case INS_GET_PUBLIC_KEY:
      tmp = lastPacket.data;
      // Derive pubKey
      derivePrivatePublic(commContext.data + 1, &signContext.privateKey, &signContext.publicKey);
      os_memset(&signContext.privateKey, 0, sizeof(signContext.privateKey));

      if (tmp == true) { // show address?
        // Show on ledger
        *flags |= IO_ASYNCH_REPLY;
        ui_address();
      } else {
        createPublicKeyResponse();
      }
  }
}