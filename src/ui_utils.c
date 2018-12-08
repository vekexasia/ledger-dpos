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
static unsigned int currentStep = 0;
static unsigned int totalSteps = 0;
processor_callback pcallback;

/**
 * Used to verify what is going to be displayed
 * @param element
 * @return 0 or 1
 */
const int uiprocessor(const bagl_element_t *element) {
  if (element->component.userid == 0x0) {
    return 1;
  }

  if ((element->component.type & (~BAGL_FLAG_TOUCHABLE)) == BAGL_NONE) {
    return 0;
  }
  if (element->component.userid == currentStep) {
    return 1;
  }
  return 0;
}