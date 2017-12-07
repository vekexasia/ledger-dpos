#include "os_io_seproxyhal.h"
#include "main.h"
#include "dposutils.h"

const ux_menu_entry_t menu_main[];
const ux_menu_entry_t menu_about[];

/**
 * TEMP global var used to print a lot of data (50chars)
 */
char linesBuffer[50];
/**
 * Actually used in the display.
 */
char lineBuffer[11];
/**
 * Coupled with linesBuffer its used to know how long is the buffer to print
 * and split into chunks.
 */
uint8_t lineBufferLength = 0;

const ux_menu_entry_t menu_main[] = {
  {NULL, NULL, 0, NULL, "Use wallet to", "view accounts", 33, 12},
  {menu_about, NULL, 0, NULL, "About", NULL, 0, 0},
  {NULL, os_sched_exit, 0, NULL, "Quit app", NULL, 50, 29},
  UX_MENU_END
};
const ux_menu_entry_t menu_about[] = {
  {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
  {menu_main, NULL, 2, NULL, "Back", NULL, 61, 40},
  UX_MENU_END
};

#define CLEAN_SCREEN { {BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0}, NULL, 0, 0, 0, NULL, NULL, NULL}
#define ICON_CROSS { {BAGL_ICON, 0x00, 3, 12, 7, 7,  0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_CROSS}, NULL, 0, 0, 0, NULL, NULL, NULL }
#define ICON_CHECK { {BAGL_ICON, 0x00, 117, 13, 8, 6,  0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_CHECK}, NULL, 0, 0, 0, NULL, NULL, NULL }
#define ICON_DOWN  { {BAGL_ICON, 0x00, 117, 13, 8, 6,  0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN}, NULL, 0, 0, 0, NULL, NULL, NULL }

void satoshiToString(uint64_t amount, uint8_t *out,
                     uint32_t outlen) {

  uint64_t partInt = amount / 10000000;
  uint64_t partDecimal = amount - partInt;

  uint8_t i = 0;

  // TODO: CAlc the # of digits for partInt
  while(partInt > 0) {
    out[i++] = (uint8_t) (partInt / 10 + '0');

  }

  if (partDecimal > 0) {
    out[i++] = '.';
    uint32_t satoshi = 100000000;
    while (satoshi > 0 && partDecimal > 0) {
      out[i++] = (uint8_t) (partDecimal / satoshi + '0');
      partDecimal -= (partDecimal/satoshi) * satoshi;
      satoshi /= 10;
    }
  }


}


/**
 * Sign with address
 */
const bagl_element_t bagl_ui_approval_send_nanos[] = {
  CLEAN_SCREEN,
  {
    {BAGL_LABELINE, 0x02, 0, 12, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
     BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
    "Send", 0, 0, 0, NULL, NULL, NULL,
  },
  {
    {BAGL_LABELINE, 0x02, 23, 26, 82, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000,
     BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 26},
    lineBuffer, 0, 0, 0, NULL, NULL, NULL,
  },
  ICON_CHECK,
  ICON_CROSS,
};


/**
 * Sign with address
 */
const bagl_element_t bagl_ui_approval_nanos[] = {
  CLEAN_SCREEN,
  {
    {BAGL_LABELINE, 0x01, 0, 12, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
     BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
    "Send from", 0, 0, 0, NULL, NULL, NULL,
  },
  {
    {BAGL_LABELINE, 0x02, 0, 12, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
     BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
    "To", 0, 0, 0, NULL, NULL, NULL,
  },
  {
    {BAGL_LABELINE, 0x03, 0, 12, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
     BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
    "Amount", 0, 0, 0, NULL, NULL, NULL,
  },
  {
    {BAGL_LABELINE, 0x00, 23, 26, 82, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000,
     BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 26},
    lineBuffer, 0, 0, 0, NULL, NULL, NULL,
  },

  ICON_CHECK,
  ICON_CROSS,
};

void lineBufferSendTxProcessor(signContext_t *signContext, uint8_t step) {
  switch (step + 1) {
    case 1:
      deriveAddressShortRepresentation(signContext->sourceAddress, lineBuffer);
      break;
    case 2:
      deriveAddressShortRepresentation(signContext->tx.recipientId, lineBuffer);
      break;
    case 3:

      signContext->tx.amountSatoshi
  }
}


/**
 * Review text to sign (message)
 */
const bagl_element_t bagl_ui_text_review_nanos[] = {
  // {
  //     {type, userid, x, y, width, height, stroke, radius, fill, fgcolor,
  //      bgcolor, font_id, icon_id},
  //     text,
  //     touch_area_brim,
  //     overfgcolor,
  //     overbgcolor,
  //     tap,
  //     out,
  //     over,
  // },
  CLEAN_SCREEN,
  {
    {BAGL_LABELINE, 0x02, 0, 12, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
     BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
    "Verify text", 0, 0, 0, NULL, NULL, NULL,
  },
  {
    {BAGL_LABELINE, 0x02, 23, 26, 82, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000,
     BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 26},
    lineBuffer, 0, 0, 0, NULL, NULL, NULL,
  },
  ICON_CROSS,
  ICON_DOWN
};