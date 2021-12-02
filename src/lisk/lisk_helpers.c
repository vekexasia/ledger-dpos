#include "lisk_helpers.h"
#include <stdlib.h>

#define SCRATCH_SIZE 21

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

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
char* itoa(int value, char* result, int base) {
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}

unsigned char lisk_encode_varint(unsigned long int value, unsigned char *dest) {
  uint8_t tmp;
  if (value <= 0xfc) {
    memmove(dest, &value, 1);
    return 1;
  } else if (value <= 0xffff) {
    tmp = 0xfd;
    memmove(dest, &tmp, 1);
    memmove(dest + 1, &value, 2);
    return 3;
  } else if (value <= 0xffffffff) {
    tmp = 0xfe;
    memmove(dest, &tmp, 1);
    memmove(dest + 1, &value, 4);
    return 5;
  } else if (value <= 0xffffffffffffffff) {
    tmp = 0xff;
    memmove(dest, &tmp, 1);
    memmove(dest + 1, &value, 8);
    return 9;
  }
  // Error
  THROW(INVALID_PARAMETER);
}
