#include "os_io_seproxyhal.h"
#include "main.h"
#include "dposutils.h"
#include "structs.h"

const ux_menu_entry_t menu_main[];
const ux_menu_entry_t menu_about[];

/**
 * Line buffer (50chars)
 */
char lineBuffer[50];


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

#define CLEAN_SCREEN                    { {BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0}, NULL, 0, 0, 0, NULL, NULL, NULL}
#define ICON(which, userid, x, y, w, h) { {BAGL_ICON, userid, x, y, w, h,  0, 0, 0, 0xFFFFFF, 0x000000, 0, which}, NULL, 0, 0, 0, NULL, NULL, NULL }
#define ICON_LEFT(which, userid)        ICON(which, userid, 3, 12, 7, 7)
#define ICON_RIGHT(which, userid)       ICON(which, userid, 117, 13, 8, 6)
#define ICON_CROSS(userid)              ICON_LEFT(BAGL_GLYPH_ICON_CROSS, userid)
#define ICON_CHECK(userid)              ICON_RIGHT(BAGL_GLYPH_ICON_CHECK, userid)
#define ICON_DOWN(userid)               ICON_RIGHT(BAGL_GLYPH_ICON_DOWN, userid)
#define SECONDLINE(txt, userid) \
{ \
  { BAGL_LABELINE, 0x00, 23, 26, 82, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 26}, \
  lineBuffer, 0, 0, 0, NULL, NULL, NULL, \
}
#define LINEBUFFER              SECONDLINE(lineBuffer, 0x00)

#define TITLE_ITEM(txt, userid) \
{ \
  { BAGL_LABELINE, userid, 0, 12, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0}, \
  txt, 0, 0, 0, NULL, NULL, NULL, \
}

/**
 * Sign with address
 */
const bagl_element_t bagl_ui_approval_send_nanos[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Send from", 0x01),
  TITLE_ITEM("To", 0x02),
  TITLE_ITEM("Amount", 0x03),
  LINEBUFFER,
  ICON_DOWN(0x01),
  ICON_DOWN(0x02),
  ICON_CHECK(0x03),
  ICON_CROSS(0x00),
};

/**
 * Create second signature with address
 */
const bagl_element_t bagl_ui_secondsign_nanos[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Create second", 0x01),
  TITLE_ITEM("For account", 0x02),
  TITLE_ITEM("With public", 0x03),
  LINEBUFFER,
  ICON_DOWN(0x01),
  ICON_DOWN(0x02),
  ICON_CHECK(0x03),
  ICON_CROSS(0x00),
};

/**
 * Create second signature with address
 */
const bagl_element_t bagl_ui_regdelegate_nanos[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Register", 0x01),
  TITLE_ITEM("For account", 0x02),
  TITLE_ITEM("With name", 0x03),
  LINEBUFFER,
  ICON_DOWN(0x01),
  ICON_DOWN(0x02),
  ICON_CHECK(0x03),
  ICON_CROSS(0x00),
};


/**
 * Sign with address
 */
const bagl_element_t bagl_ui_approval_nanos[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Sign with", 0x01),
  LINEBUFFER,
  ICON_CHECK(0x00),
  ICON_CROSS(0x00),
};

void satoshiToString(uint64_t amount, char *out) {

  uint64_t partInt = amount / 100000000;
  uint64_t partDecimal = amount - (partInt*100000000l) ;

  uint8_t i = 0;

  // TODO: CAlc the # of digits for partInt
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


void lineBufferRegDelegateTxProcessor(signContext_t *signContext, uint8_t step) {
  os_memset(lineBuffer, 0, 20);
  switch (step) {
    case 1:
      os_memmove(lineBuffer, "delegate\0", 11);
      break;
    case 2:
      deriveAddressShortRepresentation(signContext->tx.recipientId, lineBuffer);
      break;
    case 3:
      os_memmove(lineBuffer, signContext->tx.shortDesc, 16);
      break;
  }
}


void lineBufferSecondSignProcessor(signContext_t *signContext, uint8_t step) {
  os_memset(lineBuffer, 0, 11);
  switch (step) {
    case 1:
      os_memmove(lineBuffer, "signature\0", 11);
      break;
    case 2:
      deriveAddressShortRepresentation(signContext->sourceAddress, lineBuffer);
      break;
    case 3:
      os_memmove(lineBuffer, signContext->tx.shortDesc, 16);
      break;
  }
}


void lineBufferSendTxProcessor(signContext_t *signContext, uint8_t step) {
  os_memset(lineBuffer, 0, 11);
  switch (step) {
    case 1:
      deriveAddressShortRepresentation(signContext->sourceAddress, lineBuffer);
      break;
    case 2:
      deriveAddressShortRepresentation(signContext->tx.recipientId, lineBuffer);
      break;
    case 3:
      satoshiToString(signContext->tx.amountSatoshi, lineBuffer);
      break;
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
  TITLE_ITEM("Verify text", 0x00),
  LINEBUFFER,
  ICON_CROSS(0x00),
  ICON_DOWN(0x00)
};