#ifndef NULS_COMMON_PARSER_H
#define NULS_COMMON_PARSER_H

void parse_group_common();
void parse_group_coin_input();
void parse_group_coin_output();
void check_sanity_before_sign();

// Parser Utils
void cx_hash_finalize(unsigned char *dest, unsigned char size);
void cx_hash_increase(unsigned char value);
void transaction_offset_increase(unsigned char value);
void is_available_to_parse(unsigned char x);
unsigned long int transaction_get_varint(void);
void get_address_from_owner(unsigned char *owner, unsigned long int ownerLength, unsigned char *address_out);

// target = a + b
unsigned char transaction_amount_add_be(unsigned char *target, unsigned char *a, unsigned char *b);

// target = a - b
unsigned char transaction_amount_sub_be(unsigned char *target, unsigned char *a, unsigned char *b);


#endif
