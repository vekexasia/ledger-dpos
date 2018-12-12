//
// Created by andrea on 09/12/18.
//
#include "../../io.h"
#ifndef PROJECT_SIGNTX_H
#define PROJECT_SIGNTX_H
typedef void (*transaction_chunk_handler)(commPacket_t packet);

typedef struct transaction {
    uint8_t type;
    uint64_t recipientId;
    uint64_t amountSatoshi;
    char shortDesc[22];
    uint32_t totalBytes;
} transaction_t;

extern transaction_t transaction;
typedef uint8_t (*step_processor_fn)(uint8_t curStep);
extern step_processor_fn step_processor;

void handleSignTxPacket(commPacket_t *packet, commContext_t *context);
void finalizeSignTx(volatile unsigned int *flags);

#endif //PROJECT_SIGNTX_H
