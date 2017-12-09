#include "os_io_seproxyhal.h"
#include "dposutils.h"
#include <inttypes.h>
#include <stdbool.h>


#ifndef SIGN_STRUCTS
#define SIGN_STRUCTS
typedef struct signContext_t {
    cx_ecfp_private_key_t privateKey;
    cx_ecfp_public_key_t publicKey;
    uint16_t msgLength;
    uint8_t msg[500];
    bool hasRequesterPublicKey;
    uint64_t sourceAddress;
    char sourceAddressStr[22 + ADDRESS_SUFFIX_LENGTH + 1];
    bool isTx;
    struct transaction tx;
} signContext_t;

#endif
extern signContext_t signContext;

