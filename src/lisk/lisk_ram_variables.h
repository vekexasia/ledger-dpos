#ifndef LISK_RAM_VARIABLES_H
#define LISK_RAM_VARIABLES_H

#include "os.h"
#include "cx.h"
#include "lisk_context.h"

extern request_context_t reqContext;
extern transaction_context_t txContext;

extern cx_ecfp_private_key_t private_key;
extern cx_ecfp_public_key_t public_key;

// Stack Overflow check utils.
// This symbol is defined by the link script to be at the start of the stack area.
extern unsigned long _stack;
#define STACK_CANARY (*((volatile uint32_t*) &_stack))
void init_canary();
void check_canary();

#endif
