// Copyright 2020 Joshua J Baker. All rights reserved.

#ifndef VARINT_MSB_H
#define VARINT_MSB_H

#include <stdint.h>
#include <stddef.h>

uint64_t decode_varint(uint8_t* input, unsigned char *inputSize);

#endif
