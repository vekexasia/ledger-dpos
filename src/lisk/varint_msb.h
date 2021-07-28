// Copyright 2014 Eduardo Sorribas http://sorribas.org
// https://github.com/sorribas/varint.c

#ifndef VARINT_MSB_H
#define VARINT_MSB_H

#include <stdint.h>
#include <stddef.h>

uint64_t decode_varint(uint8_t* input, unsigned char *inputSize);

#endif
