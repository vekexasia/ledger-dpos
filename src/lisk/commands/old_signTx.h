//
// Created by andrea on 09/12/18.
//
#include "bolos_target.h"
#include "../../io.h"
#ifndef PROJECT_OLD_SIGNTX_H
#define PROJECT_OLD_SIGNTX_H

typedef void (*transaction_chunk_handler)(commPacket_t packet);

typedef struct transaction {
    uint8_t type;
    uint64_t recipientId;
    uint64_t amountSatoshi;
    uint32_t totalBytes;
} transaction_t;

extern transaction_t transaction;

void old_handleSignTxPacket(commPacket_t *packet, commContext_t *context);
void old_finalizeSignTx(volatile unsigned int *flags);

#endif //PROJECT_OLD_SIGNTX_H
