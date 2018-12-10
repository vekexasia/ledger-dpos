//
// Created by andrea on 09/12/18.
//
#include "../../../../io.h"
#include "../signTx.h"

#ifndef PROJECT_CREATESIGNATURETX_H
#define PROJECT_CREATESIGNATURETX_H
void tx_init_2ndsig();

void tx_chunk_2ndsig(commPacket_t *packet, transaction_t *tx);

void tx_end_2ndsig(transaction_t *tx);
#endif //PROJECT_CREATESIGNATURETX_H
