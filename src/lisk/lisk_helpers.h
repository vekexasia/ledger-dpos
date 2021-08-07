#ifndef lisk_HELPERS_H
#define lisk_HELPERS_H

#include <inttypes.h>
#include "os.h"

void lisk_swap_bytes(unsigned char *target, unsigned char *source, unsigned char size);
void lisk_write_u32_be(unsigned char *buffer, unsigned long int value);
void lisk_write_u32_le(unsigned char *buffer, unsigned long int value);
void lisk_write_u16_be(unsigned char *buffer, unsigned short int value);
void lisk_write_u16_le(unsigned char *buffer, unsigned short int value);
unsigned short int lisk_read_u16(unsigned char *buffer, unsigned char be, unsigned char skipSign);
unsigned long int lisk_read_u32(unsigned char *buffer, unsigned char be, unsigned char skipSign);
unsigned char lisk_secure_memcmp(void WIDE *buf1, void WIDE *buf2, unsigned short length);
unsigned char lisk_int_to_string(unsigned long int number, char *out);
char* itoa(int value, char* result, int base);
unsigned char lisk_hex_amount_to_displayable(unsigned char *amount, char *dest);
unsigned char lisk_encode_varint(unsigned long int value, unsigned char *dest);
void lisk_double_to_displayable(double f, int ndigits, char *dest);
uint64_t lisk_read_u64(unsigned char *buffer, unsigned char be, unsigned char skipSign);

void toHex(uint8_t what, char * whereTo);
uint8_t intToString(uint64_t amount, char *out);

#endif