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
    uint8_t *msg;
    bool hasRequesterPublicKey;
    uint64_t sourceAddress;
    char sourceAddressStr[22 + ADDRESS_SUFFIX_LENGTH + 1];
    bool isTx;
    struct transaction tx;
} signContext_t;
typedef struct commContext_t {
    bool started;
    uint16_t read;
    uint16_t totalAmount;
    uint8_t *data;
    bool isDataInNVRAM;
} commContext_t;
#endif
extern uint8_t rawData [1000];
extern uint8_t N_rawData [NVRAM_MAX];
extern signContext_t signContext;
extern commContext_t commContext;

