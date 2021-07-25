#include "bolos_target.h"
#include "../lisk_internals.h"
#include "../../io.h"

#ifndef PROJECT_SIGNTX_H
#define PROJECT_SIGNTX_H

void handleSignTxPacket(commPacket_t *packet, commContext_t *context);
void finalizeSignTx(volatile unsigned int *flags);

void setupTxSpecificHandlers();

#endif //PROJECT_SIGNTX_H
