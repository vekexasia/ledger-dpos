//
// Created by andrea on 09/12/18.
//
#include "../../../../io.h"
#include "../signTx.h"
#ifndef PROJECT_SENDTX_H
#define PROJECT_SENDTX_H

void tx_init_send();

void tx_chunk_send(commPacket_t *packet, transaction_t *tx);

void tx_end_send(transaction_t *tx);

#endif //PROJECT_SENDTX_H
