#include "os_io_seproxyhal.h"

#define MAX_CHARS_PER_LINE 10

extern const ux_menu_entry_t menu_main[4];
extern const ux_menu_entry_t menu_about[3];

void satoshiToString(uint64_t amount, char *out);
void ui_idle();
extern ux_state_t ux;
