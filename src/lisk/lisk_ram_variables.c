

#include "lisk_ram_variables.h"

request_context_t reqContext;
transaction_context_t txContext;

cx_ecfp_private_key_t private_key;
cx_ecfp_public_key_t public_key;


void init_canary() {
  STACK_CANARY = 0xDEADBEEF;
}

void check_canary() {
  if (STACK_CANARY != 0xDEADBEEF)
    THROW(EXCEPTION_OVERFLOW);
}