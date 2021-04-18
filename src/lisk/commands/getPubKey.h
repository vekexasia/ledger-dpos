//
// Created by andrea on 08/12/18.
//
#include <inttypes.h>
#include <stdbool.h>

#include "../lisk_internals.h"

#ifndef LEDGER_NANO2_GETPUBKEY_H
#define LEDGER_NANO2_GETPUBKEY_H

void handleGetPublicKey(volatile unsigned int *flags, commPacket_t *packet);

#endif //LEDGER_NANO2_GETPUBKEY_H
