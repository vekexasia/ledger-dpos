#ifndef NULS_COMMON_PARSER_H
#define NULS_COMMON_PARSER_H

#include <stdint.h>
#include <stddef.h>

void parse_group_common();
void parse_group_coin_input();
void parse_group_coin_output();
void check_sanity_before_sign();

// Parser Utils
void cx_hash_finalize(unsigned char *dest, unsigned char size);
void cx_hash_increase(unsigned char value);
void transaction_offset_increase(unsigned char value);
void is_available_to_parse(unsigned char x);

uint64_t transaction_get_varint(void);
void transaction_memmove(unsigned char *dest, unsigned char *src, size_t nBytes);

#endif
