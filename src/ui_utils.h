#include "os_io_seproxyhal.h"
#include "ux.h"

#ifndef LEDGER_NANO2_UI_UTILS_H
#define LEDGER_NANO2_UI_UTILS_H

extern char lineBuffer[100];
#define IS_PRINTABLE(c) ((c >= 0x20 && c <= 0x7e) || (c >= 0x80 && c <= 0xFF))

#define CLEAN_SCREEN                    { {BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0}, NULL, 0, 0, 0, NULL, NULL, NULL}
#define ICON(which, userid, x, y, w, h) { {BAGL_ICON, userid, x, y, w, h,  0, 0, 0, 0xFFFFFF, 0x000000, 0, which}, NULL, 0, 0, 0, NULL, NULL, NULL }
#define ICON_LEFT(which, userid)        ICON(which, userid, 3, 12, 7, 7)
#define ICON_RIGHT(which, userid)       ICON(which, userid, 117, 13, 8, 6)
#define ICON_CROSS(userid)              ICON_LEFT(BAGL_GLYPH_ICON_CROSS, userid)
#define ICON_CHECK(userid)              ICON_RIGHT(BAGL_GLYPH_ICON_CHECK, userid)
#define ICON_ARROW_RIGHT(userid)        ICON_RIGHT(BAGL_GLYPH_ICON_RIGHT, userid)
#define ICON_DOWN(userid)               ICON_RIGHT(BAGL_GLYPH_ICON_DOWN, userid)
#define SECONDLINE(txt, userid) \
{ \
  { BAGL_LABELINE, 0x00, 23, 26, 82, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 26}, \
  txt, 0, 0, 0, NULL, NULL, NULL, \
}
#define LINEBUFFER              SECONDLINE(lineBuffer, 0x00)

#define TITLE_ITEM(txt, userid) \
{ \
  { BAGL_LABELINE, userid, 0, 12, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0}, \
  txt, 0, 0, 0, NULL, NULL, NULL, \
}

#endif //LEDGER_NANO2_UI_UTILS_H
