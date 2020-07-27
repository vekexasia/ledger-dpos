//
// Created by andrea on 08/12/18.
//

#include "ui_utils.h"

/**
 * Line buffer (50chars)
 */
char lineBuffer[50];
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