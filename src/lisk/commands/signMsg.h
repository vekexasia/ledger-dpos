//
// Created by andrea on 09/12/18.
//
#include "../../io.h"

#ifndef LEDGER_NANO2_SIGNMSG_H
#define LEDGER_NANO2_SIGNMSG_H

void handleSignMessagePacket(commPacket_t *packet, commContext_t *context);
void prepareMsgLineBuffer(commPacket_t *packet, uint32_t headerBytesRead);
void processSignMessage(volatile unsigned int *flags);

#endif //LEDGER_NANO2_SIGNMSG_H
