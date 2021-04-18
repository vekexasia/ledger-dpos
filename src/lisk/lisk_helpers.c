#include "lisk_helpers.h"
#include <stdlib.h>

#define SCRATCH_SIZE 21

void lisk_swap_bytes(unsigned char *target, unsigned char *source, unsigned char size) {
  unsigned char i;
  for (i = 0; i < size; i++) {
    target[i] = source[size - 1 - i];
  }
}

void lisk_write_u32_be(unsigned char *buffer, unsigned long int value) {
  buffer[0] = ((value >> 24) & 0xff);
  buffer[1] = ((value >> 16) & 0xff);
  buffer[2] = ((value >> 8) & 0xff);
  buffer[3] = (value & 0xff);
}

void lisk_write_u32_le(unsigned char *buffer, unsigned long int value) {
  buffer[0] = (value & 0xff);
  buffer[1] = ((value >> 8) & 0xff);
  buffer[2] = ((value >> 16) & 0xff);
  buffer[3] = ((value >> 24) & 0xff);
}

void lisk_write_u16_be(unsigned char *buffer, unsigned short int value) {
  buffer[0] = ((value >> 8) & 0xff);
  buffer[1] = (value & 0xff);
}

void lisk_write_u16_le(unsigned char *buffer, unsigned short int value) {
  buffer[0] = (value & 0xff);
  buffer[1] = ((value >> 8) & 0xff);
}


unsigned short int lisk_read_u16(unsigned char *buffer, unsigned char be, unsigned char skipSign) {
  unsigned char i;
  unsigned short int result = 0;
  unsigned char shiftValue = (be ? 8 : 0);
  for (i = 0; i < 2; i++) {
    unsigned char x = (unsigned char)buffer[i];
    if ((i == 0) && skipSign) {
      x &= 0x7f;
    }
    result += ((unsigned short int)x) << shiftValue;
    if (be) {
      shiftValue -= 8;
    } else {
      shiftValue += 8;
    }
  }
  return result;
}

unsigned long int lisk_read_u32(unsigned char *buffer, unsigned char be, unsigned char skipSign) {
  unsigned char i;
  unsigned long int result = 0;
  unsigned char shiftValue = (be ? 24 : 0);
  for (i = 0; i < 4; i++) {
    unsigned char x = (unsigned char)buffer[i];
    if ((i == 0) && skipSign) {
      x &= 0x7f;
    }
    result += ((unsigned long int)x) << shiftValue;
    if (be) {
      shiftValue -= 8;
    } else {
      shiftValue += 8;
    }
  }
  return result;
}

uint64_t lisk_read_u64(unsigned char *buffer, unsigned char be, unsigned char skipSign) {
  unsigned char i;
  uint64_t result = 0;
  unsigned char shiftValue = (be ? 56 : 0);
  for (i = 0; i < 8; i++) {
    unsigned char x = (unsigned char)buffer[i];
    if ((i == 0) && skipSign) {
      x &= 0x7f;
    }
    result += ((uint64_t)x) << shiftValue;
    if (be) {
      shiftValue -= 8;
    } else {
      shiftValue += 8;
    }
  }
  return result;
}

unsigned char lisk_secure_memcmp(void WIDE *buf1, void WIDE *buf2, unsigned short length) {
  unsigned char error = 0;
  while (length--) {
    error |= ((unsigned char WIDE *)buf1)[length] ^
             ((unsigned char WIDE *)buf2)[length];
  }
  if (length != 0xffff) {
    return 1;
  }
  return error;
}

unsigned char lisk_int_to_string(unsigned long int amount, char *out) {
  unsigned char i = 0;
  if (amount == 0) {
    out[0] = '0';
    i = 1;
  } else {
    unsigned long int part = amount;
    while(part > 0) {
      out[i++] = (unsigned char) (part % 10 + '0');
      part /= 10;
    }
  }
  out[i] = '\0';
  unsigned char j = 0;
  for (j=0; j<i/2; j++) {
    char swap = out[j];
    out[j] = out[i-1-j];
    out[i-1-j] = swap;
  }

  return i;
}

unsigned char lisk_bin_to_hex(unsigned char *in, size_t in_size, char *out, size_t out_size) {

    unsigned char out_len = in_size * 2;
    if (out_size < out_len + 1) THROW(INVALID_PARAMETER);

    unsigned char *src = in;
    for (size_t i = 0; i < in_size; i++) {
        out[i*2]   = "0123456789ABCDEF"[src[i] >> 4];
        out[i*2+1] = "0123456789ABCDEF"[src[i] & 0x0F];
    }
    return out_len;
}

unsigned char lisk_hex_amount_to_displayable(unsigned char *amount, char *dest) {

  if ((!amount) || (!dest)) THROW(INVALID_PARAMETER);

  return lisk_bin_to_hex(amount, 32, dest, 100);
#if 0
  unsigned char LOOP1 = 13;
  unsigned char LOOP2 = 8;
  unsigned short scratch[SCRATCH_SIZE];
  unsigned char offset = 0;
  unsigned char nonZero = 0;
  unsigned char i;
  unsigned char targetOffset = 0;
  unsigned char workOffset;
  unsigned char j;
  unsigned char nscratch = SCRATCH_SIZE;
  unsigned char smin = nscratch - 2;
  unsigned char comma = 0;

  for (i = 0; i < SCRATCH_SIZE; i++) {
    scratch[i] = 0;
  }
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      unsigned char k;
      unsigned short shifted_in =
              (((amount[i] & 0xff) & ((1 << (7 - j)))) != 0) ? (short)1
                                                             : (short)0;
      for (k = smin; k < nscratch; k++) {
        scratch[k] += ((scratch[k] >= 5) ? 3 : 0);
      }
      if (scratch[smin] >= 8) {
        smin -= 1;
      }
      for (k = smin; k < nscratch - 1; k++) {
        scratch[k] =
                ((scratch[k] << 1) & 0xF) | ((scratch[k + 1] >= 8) ? 1 : 0);
      }
      scratch[nscratch - 1] = ((scratch[nscratch - 1] << 1) & 0x0F) |
                              (shifted_in == 1 ? 1 : 0);
    }
  }

  for (i = 0; i < LOOP1; i++) {
    if (!nonZero && (scratch[offset] == 0)) {
      offset++;
    } else {
      nonZero = 1;
      dest[targetOffset++] = scratch[offset++] + '0';
    }
  }
  if (targetOffset == 0) {
    dest[targetOffset++] = '0';
  }
  workOffset = offset;
  for (i = 0; i < LOOP2; i++) {
    unsigned char allZero = 1;
    unsigned char j;
    for (j = i; j < LOOP2; j++) {
      if (scratch[workOffset + j] != 0) {
        allZero = 0;
        break;
      }
    }
    if (allZero) {
      break;
    }
    if (!comma) {
      dest[targetOffset++] = '.';
      comma = 1;
    }
    dest[targetOffset++] = scratch[offset++] + '0';
  }
  return targetOffset;
#endif
}

unsigned char lisk_encode_varint(unsigned long int value, unsigned char *dest) {
  uint8_t tmp;
  if (value <= 0xfc) {
    os_memmove(dest, &value, 1);
    return 1;
  } else if (value <= 0xffff) {
    tmp = 0xfd;
    os_memmove(dest, &tmp, 1);
    os_memmove(dest + 1, &value, 2);
    return 3;
  } else if (value <= 0xffffffff) {
    tmp = 0xfe;
    os_memmove(dest, &tmp, 1);
    os_memmove(dest + 1, &value, 4);
    return 5;
  } else if (value <= 0xffffffffffffffff) {
    tmp = 0xff;
    os_memmove(dest, &tmp, 1);
    os_memmove(dest + 1, &value, 8);
    return 9;
  }
  // Error
  THROW(INVALID_PARAMETER);
}

void lisk_double_to_displayable(double f, int ndigits, char *dest) {
  gcvt(f, ndigits, dest);
}

const char hexChars[] = "0123456789abcdef";

void toHex(uint8_t what, char * whereTo) {
  whereTo[0] = hexChars[what / 16];
  whereTo[1] = hexChars[what % 16];
}

uint8_t intToString(uint64_t amount, char *out) {
  uint8_t i = 0;
  if (amount == 0) {
    out[0] = '0';
    i = 1;
  } else {
    uint64_t part = amount;
    while(part > 0) {
      out[i++] = (uint8_t) (part % 10 + '0');
      part /= 10;
    }
  }
  out[i] = '\0';
  uint8_t j = 0;
  for (j=0; j<i/2; j++) {
    char swap = out[j];
    out[j] = out[i-1-j];
    out[i-1-j] = swap;
  }
  return i;
}