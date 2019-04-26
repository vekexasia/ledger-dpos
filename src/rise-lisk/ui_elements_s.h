#include "os_io_seproxyhal.h"

#if defined(TARGET_NANOS)
extern const ux_menu_entry_t menu_main[4];
extern const ux_menu_entry_t menu_about[3];
extern ux_state_t ux;
#endif

void ui_idle();
