// Copyright 2020 Joshua J Baker. All rights reserved.

#include "varint_msb.h"

uint64_t decode_varint(uint8_t* input, unsigned char *inputSize) {
  uint64_t ret = 0;
  *inputSize = 0;
  for (size_t i = 0; i < 8; i++) {
    ret |= (input[i] & 127) << (7 * i);
    (*inputSize)++;
    //If the next-byte flag is set
    if(!(input[i] & 128)) {
      break;
    }
  }
  return ret;
}
