// Copyright 2014 Eduardo Sorribas http://sorribas.org
// https://github.com/sorribas/varint.c

#include "varint_msb.h"

static const char MSB = 0x80;

uint64_t decode_varint(uint8_t* buf, unsigned char* inputSize) {
  uint64_t result = 0;
  int bits = 0;
  uint8_t *ptr = buf;
  unsigned long long ll;
  while (*ptr & MSB) {
    ll = *ptr;
    result += ((ll & 0x7F) << bits);
    ptr++;
    bits += 7;
  }
  ll = *ptr;
  result += ((ll & 0x7F) << bits);

  if (inputSize != NULL) *inputSize = ptr - buf + 1;

  return result;
}