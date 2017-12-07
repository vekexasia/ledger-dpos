//
// Created by andrea on 02/11/17.
//

#include "os_io_seproxyhal.h"

const ux_menu_entry_t menu_main[];
const ux_menu_entry_t menu_about[];

char linesBuffer[50];
char lineBuffer[11];
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

#define ICON_CROSS { {BAGL_ICON, 0x00, 3, 12, 7, 7,  0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_CROSS}, NULL, 0, 0, 0, NULL, NULL, NULL }
#define ICON_CHECK { {BAGL_ICON, 0x00, 117, 13, 8, 6,  0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_CHECK}, NULL, 0, 0, 0, NULL, NULL, NULL }
#define ICON_DOWN  { {BAGL_ICON, 0x00, 117, 13, 8, 6,  0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN}, NULL, 0, 0, 0, NULL, NULL, NULL }

const bagl_element_t bagl_ui_approval_nanos[] = {
  {
    {BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0},
    NULL, 0, 0, 0, NULL, NULL, NULL,
  },
  {
    {BAGL_LABELINE,  0x02, 0, 12, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
    "Sign with", 0, 0, 0, NULL, NULL, NULL,
  },

  {
    {BAGL_LABELINE,  0x02, 23,  26, 82, 11,  0, 0, 0, 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 26},
    lineBuffer, 0, 0, 0, NULL, NULL, NULL,
  },
  ICON_CHECK,
  ICON_CROSS
};


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
  {
    {BAGL_RECTANGLE, 0x00, 0,   0,  128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0},
    NULL, 0, 0, 0, NULL, NULL, NULL,
  },
  {
    {BAGL_LABELINE,  0x02, 0,   12, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
    "Verify text", 0, 0, 0, NULL, NULL, NULL,
  },
  {
    {BAGL_LABELINE, 0x02, 23, 26, 82, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 26},
    lineBuffer, 0, 0, 0, NULL, NULL, NULL,
  },
  ICON_CROSS,
  ICON_DOWN
};