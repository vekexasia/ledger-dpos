#include <stdio.h>
#include <string.h>

#include "../src/lisk/lisk32_address.h"

struct valid_address_data {
  const char* address;
  size_t scriptPubKeyLen;
  const uint8_t scriptPubKey[20];
};

static struct valid_address_data valid_address[] = {
    {
        "lsk24cd35u4jdq8szo3pnsqe5dsxwrnazyqqqg5eu",
        20,
        {
        0xc2, 0x47, 0xa4, 0x2e, 0x09,
        0xe6, 0xaa, 0xfd, 0x81, 0x88,
        0x21, 0xf7, 0x5b, 0x2f, 0x5b,
        0x0d, 0xe4, 0x7c, 0x82, 0x35
        }
    },
    {
        "lskxwnb4ubt93gz49w3of855yy9uzntddyndahm6s",
        20,
        {
        0x0d, 0xce, 0x64, 0xc0, 0xd3,
        0x6a, 0x3e, 0x04, 0xb6, 0xe8,
        0x67, 0x9e, 0xb5, 0xc6, 0x2d,
        0x80, 0x0f, 0x3d, 0x6a, 0x27
        }
    },
    {
        "lskzkfw7ofgp3uusknbetemrey4aeatgf2ntbhcds",
        20,
        {
        0x05, 0x3d, 0x77, 0x33, 0xdf,
        0x22, 0x21, 0x0d, 0xd0, 0xe6,
        0xb4, 0xec, 0x59, 0x5a, 0x29,
        0xcd, 0xb3, 0x3f, 0xfb, 0x07
        }
    },
};

static const char* invalid_address[] = {
    "24cd35u4jdq8szo3pnsqe5dsxwrnazyqqqg5eu"    ,  // missing prefix
    "lsk24cd35u4jdq8szo3pnsqe5dsxwrnazyqqqg5e"  ,  // incorrect length (length 40 instead of 41)
    "lsk24cd35u4jdq8szo3pnsqe5dsxwrnazyqqqg5euu",  // incorrect length (length 42 instead of 41)
    "LSK24cd35u4jdq8szo3pnsqe5dsxwrnazyqqqg5eu" ,  // incorrect prefix
    "tsk24cd35u4jdq8szo3pnsqe5dsxwrnazyqqqg5eu" ,  // incorrect prefix
    "lsk24cd35u4jdq8sz03pnsqe5dsxwrnazyqqqg5eu" ,  // invalid character (contains a zero)
    "lsk24Cd35u4jdq8szo3pnsqe5dsxwrnazyqqqg5eu" ,  // invalid character (contains an upper case 'C' instead of lower case 'c')
    "LSK24CD35U4JDQ8SZO3PNSQE5DSXWRNAZYQQQG5EU" ,  // invalid characters (all letters in upper case)
    "lsk24dc35u4jdq8szo3pnsqe5dsxwrnazyqqqg5eu" ,  // invalid checksum due to character swap
};

int main(void) {
  size_t i;
  int fail = 0;
  char * hrp = "lsk";

  // Valid Addresses
  for(i = 0; i < sizeof(valid_address) / sizeof(valid_address[0]); i++)
  {
    int ok = 1;
    // Lisk Encoding
    char enc_lisk[41];
    if(!lisk_addr_encode(enc_lisk, hrp, valid_address[i].scriptPubKey, valid_address[i].scriptPubKeyLen) ||
        strcmp(enc_lisk, valid_address[i].address) != 0)
    {
      printf("lisk_addr_encode fails: '%s'\n", valid_address[i].address);
      ok = 0;
    }

    // Lisk Decoding
    uint8_t data_decode[20];
    size_t data_len_decode = 0;
    bech32_encoding ret_enc = lisk_addr_decode(data_decode, &data_len_decode, hrp, enc_lisk);
    if(ret_enc != BECH32_ENCODING_BECH32 ||
       data_len_decode != 20 ||
       memcmp(data_decode, valid_address[i].scriptPubKey, data_len_decode) != 0)
    {
      printf("lisk_addr_decode fails: '%s'\n", valid_address[i].address);
      ok = 0;
    }
    fail += !ok;
  }

  // Invalid Addresses
  for(i = 0; i < sizeof(invalid_address) / sizeof(invalid_address[0]); i++)
  {
    int ok = 1;
    uint8_t data_decode[30];
    size_t data_len_decode = 0;
    bech32_encoding ret_enc = lisk_addr_decode(data_decode, &data_len_decode, hrp, invalid_address[i]);
    if(ret_enc == BECH32_ENCODING_BECH32)
    {
      printf("Invalid address seems valid: '%s'\n", invalid_address[i]);
      ok = 0;
    }
    fail += !ok;
  }

  printf("%i failures\n", fail);
  return fail != 0;
}