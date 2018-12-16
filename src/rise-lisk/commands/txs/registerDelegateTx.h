//
// Created by andrea on 09/12/18.
//
#include "../../../io.h"
#include "../signTx.h"
#ifndef PROJECT_REGISTERDELEGATETX_H
#define PROJECT_REGISTERDELEGATETX_H
void tx_init_regdel();

void tx_chunk_regdel(uint8_t * data, uint8_t length, commPacket_t *sourcePacket, transaction_t *tx);

void tx_end_regdel(transaction_t *tx);

void checkUsernameValidity();

#endif //PROJECT_REGISTERDELEGATETX_H
