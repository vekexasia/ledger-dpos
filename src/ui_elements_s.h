#include "os_io_seproxyhal.h"

#define MAX_CHARS_PER_LINE 10

extern const ux_menu_entry_t menu_main[4];
extern const ux_menu_entry_t menu_about[3];

extern const bagl_element_t bagl_ui_approval_nanos[5];
extern const bagl_element_t bagl_ui_text_review_nanos[5];
extern char linesBuffer[50];
extern char lineBuffer[11];
extern uint8_t lineBufferLength;

void satoshiToString(uint64_t amount, uint8_t *out);