#ifndef LISK_CONSTANTS_H
#define LISK_CONSTANTS_H

#define NEED_NEXT_CHUNK 0x6866

#define MAX_BIP32_PATH 10
#define MAX_BIP32_PATH_LENGTH (4 * MAX_BIP32_PATH) + 1

#define TX_MODULE_ID_TOKEN 2
#define TX_MODULE_ID_MULTISIG 4
#define TX_MODULE_ID_DPOS 5
#define TX_MODULE_ID_LEGACY 1000

#define TX_ASSET_ID_TRANSFER 0
#define TX_ASSET_ID_REGISTER_MULTISIG_GROUP 0
#define TX_ASSET_ID_REGISTER_DELEGATE 0
#define TX_ASSET_ID_VOTE_DELEGATE 1
#define TX_ASSET_ID_UNLOCK_TOKEN 2
#define TX_ASSET_ID_RECLAIM 0

#define ENCODED_PUB_KEY 32
#define ADDRESS_HASH_LENGTH 20 // sha256(pubkey) -> first 20 bytes
#define ADDRESS_LISK32_LENGTH 41 // "lsk" + lisk32 encoded

#define DATA_MAX_LENGTH 200 // data field max 64 utf8 char. let's be conservative
#define DELEGATE_MAX_LENGTH 80 // max 20 utf8 chars. let's be conservative

#define NETWORK_ID_LENGTH 32

#define DIGEST_LENGTH 32
#define SIGNATURE_LENGTH 64

#endif
