#ifndef lisk_HELPERS_H
#define lisk_HELPERS_H

#include <inttypes.h>
#include "os.h"

unsigned short int lisk_read_u16(unsigned char *buffer, unsigned char be, unsigned char skipSign);
unsigned long int lisk_read_u32(unsigned char *buffer, unsigned char be, unsigned char skipSign);
unsigned char lisk_secure_memcmp(void WIDE *buf1, void WIDE *buf2, unsigned short length);
unsigned char lisk_int_to_string(unsigned long int number, char *out);
char* itoa(int value, char* result, int base);
unsigned char lisk_encode_varint(unsigned long int value, unsigned char *dest);
uint64_t lisk_read_u64(unsigned char *buffer, unsigned char be, unsigned char skipSign);

#endif