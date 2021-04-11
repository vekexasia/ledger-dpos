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
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "lisk32_address.h"

static uint32_t bech32_polymod_step(uint32_t pre) {
  uint8_t b = pre >> 25;
  return ((pre & 0x1FFFFFF) << 5) ^
         (-((b >> 0) & 1) & 0x3b6a57b2UL) ^
         (-((b >> 1) & 1) & 0x26508e6dUL) ^
         (-((b >> 2) & 1) & 0x1ea119faUL) ^
         (-((b >> 3) & 1) & 0x3d4233ddUL) ^
         (-((b >> 4) & 1) & 0x2a1462b3UL);
}

static uint32_t bech32_final_constant(bech32_encoding enc) {
  if (enc == BECH32_ENCODING_BECH32) return 1;
  assert(0);
}

//static const char* charset = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";
static const char* charset = "zxvcpmbn3465o978uyrtkqew2adsjhfg";

static const int8_t charset_rev[128] = {
//  00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
//  10  11  12  13  14  15  16  17  18  19  1A  1B  1C  1D  1E  1F
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
//  20  21  22  23  24  25  26  27  28  29  2A  2B  2C  2D  2E  2F
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
//  30  31  32  33  34  35  36  37  38  39  3A  3B  3C  3D  3E  3F
    -1, -1, 24,  8,  9, 11, 10, 14, 15, 13, -1, -1, -1, -1, -1, -1,
//  40  41  42  43  44  45  46  47  48  49  4A  4B  4C  4D  4E  4F
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
//  50  51  52  53  54  55  56  57  58  59  5A  5B  5C  5D  5E  5F
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
//  60  61  62  63  64  65  66  67  68  69  6A  6B  6C  6D  6E  6F
    -1, 25,  6,  3, 26, 22, 30, 31, 29, -1, 28, 20, -1,  5,  7, 12,
//  70  71  72  73  74  75  76  77  78  79  7A  7B  7C  7D  7E  7F
     4, 21, 18, 27, 19, 16,  2, 23,  1, 17,  0, -1, -1, -1, -1, -1,
};

int lisk32_encode(char *output, const char *hrp, const uint8_t *data, size_t data_len) {
  uint32_t chk = 1;
  size_t i = 0;
  // Check hrp
  while (hrp[i] != 0) {
    int ch = hrp[i];
    if (ch < 33 || ch > 126) return 0;
    if (ch >= 'A' && ch <= 'Z') return 0;
    ++i;
  }
  // Copy hrp
  while (*hrp != 0) {
    *(output++) = *(hrp++);
  }
  // Data part
  for (i = 0; i < data_len; ++i) {
    if (*data >> 5) return 0;
    chk = bech32_polymod_step(chk) ^ (*data);
    *(output++) = charset[*(data++)];
  }
  // Checksum
  for (i = 0; i < 6; ++i) {
    chk = bech32_polymod_step(chk);
  }
  chk ^= bech32_final_constant(BECH32_ENCODING_BECH32);
  for (i = 0; i < 6; ++i) {
    *(output++) = charset[(chk >> ((5 - i) * 5)) & 0x1f];
  }
  *output = 0;
  return 1;
}

bech32_encoding lisk32_decode(uint8_t *data, size_t *data_len, const char* hrp_expected, const char *input) {
  uint32_t chk = 1;
  size_t i;
  size_t input_len = strlen(input);
  size_t hrp_len = strlen(hrp_expected);
  int have_lower = 0, have_upper = 0;
  // CHeck input size
  if (input_len != hrp_len + 38) {
    return BECH32_ENCODING_NONE;
  }
  *data_len = 38;
  *(data_len) -= 6;
  // Check hrp
  if (strncmp(input, hrp_expected, hrp_len) != 0) return 0;
  i = hrp_len;
  while (i < input_len) {
    int v = (input[i] & 0x80) ? -1 : charset_rev[(int)input[i]];
    if (input[i] >= 'a' && input[i] <= 'z') have_lower = 1;
    if (input[i] >= 'A' && input[i] <= 'Z') have_upper = 1;
    if (v == -1) { // Not supported char
      return BECH32_ENCODING_NONE;
    }
    chk = bech32_polymod_step(chk) ^ v;
    if (i + 6 < input_len) {
      data[i - (hrp_len)] = v;
    }
    ++i;
  }
  if (have_lower && have_upper) {
    return BECH32_ENCODING_NONE;
  }
  if (chk == bech32_final_constant(BECH32_ENCODING_BECH32)) {
    return BECH32_ENCODING_BECH32;
  } else {
    return BECH32_ENCODING_NONE;
  }
}

static int convert_bits(uint8_t* out, size_t* outlen, int outbits, const uint8_t* in, size_t inlen, int inbits, int pad) {
  uint32_t val = 0;
  int bits = 0;
  uint32_t maxv = (((uint32_t)1) << outbits) - 1;
  while (inlen--) {
    val = (val << inbits) | *(in++);
    bits += inbits;
    while (bits >= outbits) {
      bits -= outbits;
      out[(*outlen)++] = (val >> bits) & maxv;
    }
  }
  if (pad) {
    if (bits) {
      out[(*outlen)++] = (val << (outbits - bits)) & maxv;
    }
  } else if (((val << (outbits - bits)) & maxv) || bits >= inbits) {
    return 0;
  }
  return 1;
}

int lisk_addr_encode(char *output, const char *hrp, const uint8_t *data, size_t data_len) {
  uint8_t buffer[100];
  memset(buffer, 0, 100);
  size_t buffer_len = 0;
  if (data_len != 20) return 0;
  convert_bits(buffer, &buffer_len, 5, data, data_len, 8, 0);
  return lisk32_encode(output, hrp, buffer, buffer_len);
}

int lisk_addr_decode(uint8_t* data, size_t* data_len, const char* hrp_expected, const char* addr) {
  uint8_t buffer[100];
  memset(buffer, 0, 100);
  size_t buffer_len = 0;
  bech32_encoding enc = lisk32_decode(buffer, &buffer_len, hrp_expected, addr);
  if (enc == BECH32_ENCODING_NONE) return 0;
  *data_len = 0;
  if (!convert_bits(data, data_len, 8, buffer, buffer_len, 5, 0)) return 0;
  if (*data_len != 20) return 0;
  return 1;
}