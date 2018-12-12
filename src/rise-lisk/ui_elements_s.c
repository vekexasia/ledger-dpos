#include <string.h>
#include "os_io_seproxyhal.h"
#include "dposutils.h"
#include "../glyphs.h"
#include "../ui_utils.h"

const ux_menu_entry_t menu_main[];
const ux_menu_entry_t menu_about[];
ux_state_t ux;

#define __NAME3(a, b, c) a##b##c
#define NAME3(a, b, c) __NAME3(a, b, c)

const ux_menu_entry_t menu_main[] = {
  {NULL, NULL, 0, &NAME3(C_badge_, COINID, ) , "Use wallet to", "view accounts", 33, 12},
  {menu_about, NULL, 0, NULL, "About", NULL, 0, 0},
  {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
  UX_MENU_END
};

const ux_menu_entry_t menu_about[] = {
  {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
  {NULL, NULL, 0, NULL, "Developer", "vekexasia", 0, 0},
  {menu_main, NULL, 2, &C_icon_back, "Back", NULL, 61, 40},
  UX_MENU_END
};


void satoshiToString(uint64_t amount, char *out) {

  uint64_t partInt = amount / 100000000;
  uint64_t partDecimal = amount - (partInt*100000000l) ;

  uint8_t i = 0;

  // TODO: Calc the # of digits for partInt
  while(partInt > 0) {
    out[i++] = (uint8_t) (partInt % 10 + '0');
    partInt /=10;
  }


  // Swap elements
  uint8_t j = 0;
  uint8_t tmp;
  for (; j<i/2; j++) {
    tmp = out[j];
    out[j] = out[i-1-j];
    out[i-1-j] = tmp;
  }


  if (partDecimal > 0) {
    out[i++] = '.';
    uint32_t satoshi = 10000000;
    while (satoshi > 0 && partDecimal > 0) {
      out[i++] = (uint8_t) (partDecimal / satoshi + '0');
      partDecimal -= (partDecimal/satoshi) * satoshi;
      satoshi /= 10;
    }
  }


}






/**
 * Sets ui to idle.
 */
void ui_idle() {
  UX_MENU_DISPLAY(0, menu_main, NULL);
}
