//
// Created by andrea on 09/12/18.
//
#include "../../../../io.h"
#include "../signTx.h"
#ifndef PROJECT_VOTETX_H
#define PROJECT_VOTETX_H
void tx_init_vote();

void tx_chunk_vote(commPacket_t *packet, transaction_t *tx);

void tx_end_vote(transaction_t *tx);
#endif //PROJECT_VOTETX_H
