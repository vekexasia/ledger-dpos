/* Copyright (c) 2021 hirish
 * Copyright (c) 2017, 2021 Pieter Wuille
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "lisk_address_legacy.h"

uint64_t deriveLegacyAddressFromUintArray(uint8_t *source, bool rev) {

  uint8_t address[8];
  uint8_t i;
  for (i = 0; i < 8; i++) {
    address[i] = source[rev == true ? 7 - i : i];
  }

  uint64_t encodedAddress = 0;
  for (i = 0; i < 8; i++) {
    encodedAddress = encodedAddress << 8;
    encodedAddress += address[i];
  }

  return encodedAddress;
}

uint8_t deriveLegacyAddressStringRepresentation(uint64_t encodedAddress, char *output) {

  char brocca[22];
  uint8_t i = 0;
  while (encodedAddress > 0) {
    uint64_t remainder = encodedAddress % 10;
    encodedAddress = encodedAddress / 10;
    brocca[i++] = (char) (remainder + '0');
  }

  uint8_t total = i;
  for (i = 0; i < total; i++) {
    output[total - 1 - i] = brocca[i];
  }

  memmove(&output[total], LEGACY_ADDRESS_SUFFIX, LEGACY_ADDRESS_SUFFIX_LENGTH);
  output[total + LEGACY_ADDRESS_SUFFIX_LENGTH] = '\0'; // for strlen
  return (uint8_t) (total + LEGACY_ADDRESS_SUFFIX_LENGTH /*suffix*/);
}

uint64_t deriveLegacyAddressFromPublic(cx_ecfp_public_key_t *publicKey) {
  uint8_t encodedPkey[32];
  getEncodedPublicKey(publicKey, encodedPkey);

  unsigned char hashedPkey[32];
  cx_hash_sha256(encodedPkey, 32, hashedPkey, 32);

  return deriveLegacyAddressFromUintArray(hashedPkey, true);
}