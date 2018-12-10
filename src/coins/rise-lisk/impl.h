//
// Created by andrea on 08/12/18.
//
#include <stdbool.h>
#include "../../io.h"

#ifndef LEDGER_NANO2_IMPL_H
#define LEDGER_NANO2_IMPL_H
void innerHandleCommPacket(commPacket_t *packet, commContext_t *context);
bool innerProcessCommPacket(volatile unsigned int *flags, commPacket_t *lastPacket, commContext_t *context);
#endif //LEDGER_NANO2_IMPL_H
