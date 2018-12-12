//
// Created by andrea on 08/12/18.
//
#include <inttypes.h>
#include <stdbool.h>

#ifndef LEDGER_NANO2_GETPUBKEY_H
#define LEDGER_NANO2_GETPUBKEY_H
void handleGetPublicKey(volatile unsigned int *flags, uint8_t *bip32Path, bool confirmationRequest);
#endif //LEDGER_NANO2_GETPUBKEY_H
