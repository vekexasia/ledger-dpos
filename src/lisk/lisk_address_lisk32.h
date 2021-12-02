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

#ifndef _LISK32_ADDR_H_
#define _LISK32_ADDR_H_ 1

#include <stdint.h>

/** Supported encodings. */
typedef enum {
  BECH32_ENCODING_NONE,
  BECH32_ENCODING_BECH32
} bech32_encoding;

/** Encode a Lisk32 or string
 *
 *  Out: output:  Pointer to a buffer of size strlen(hrp) + data_len + 8 that
 *                will be updated to contain the null-terminated Bech32 string.
 *  In: hrp :     Pointer to the null-terminated human readable part.
 *      data :    Pointer to an array of 5-bit values.
 *      data_len: Length of the data array.
 *  Returns 1 if successful.
 */
int lisk32_encode(
    char *output,
    const char *hrp,
    const uint8_t *data,
    size_t data_len
);

/** Decode a Lisk32 address
 *
 *  Out: data:     Pointer to a buffer of size strlen(input) - 8 that will
 *                 hold the encoded 5-bit data values.
 *       data_len: Pointer to a size_t that will be updated to be the number
 *                 of entries in data.
 *  In: hrp_expected: Pointer to a buffer that contain the expected hrp.
 *      input:     Pointer to a null-terminated Bech32 string.
 *  Returns BECH32_ENCODING_BECH32 to indicate decoding was successful
 *          BECH32_ENCODING_NONE is returned if decoding failed.
 */
bech32_encoding lisk32_decode(
    uint8_t *data,
    size_t *data_len,
    const char *hrp_expected,
    const char *input
);

/** Encode a Lisk address
 *
 *  Out: output:   Pointer to a buffer of size 73 + strlen(hrp) that will be
 *                 updated to contain the null-terminated address.
 *  In:  hrp:      Pointer to the null-terminated human readable part to use
 *                 (chain/network specific).
 *       data:     Data bytes for the lisk address program (20 bytes).
 *                 ex: 0xc247a42e09e6aafd818821f75b2f5b0de47c8235
 *       data_len: Number of data bytes in prog.
 *  Returns 1 if successful.
 */
int lisk_addr_encode(
    char *output,
    const char *hrp,
    const uint8_t *data,
    size_t data_len
);

/** Decode a Lisk address
 *
 *  Out: data:     Pointer to a buffer of size 20 that will be updated to
 *                 contain the address bytes.
 *       data_len: Pointer to a size_t that will be updated to contain the length
 *                 of bytes in data (20).
 *  In: hrp_expected: Pointer to the null-terminated human readable part that is
 *                 expected (chain/network specific).
 *       addr:     Pointer to the null-terminated address.
 *                 ex: lsk24cd35u4jdq8szo3pnsqe5dsxwrnazyqqqg5eu
 *  Returns 1 if successful.
 */
int lisk_addr_decode(
    uint8_t* data,
    size_t* data_len,
    const char* hrp_expected,
    const char* addr
);

#endif