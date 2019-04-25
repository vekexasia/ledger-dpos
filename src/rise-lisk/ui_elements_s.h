#include "os_io_seproxyhal.h"
#include "os.h"

void satoshiToString(uint64_t amount, char *out);
void ui_idle();


#ifdef TARGET_NANOX
#include "ux.h"
extern ux_state_t G_ux;
extern bolos_ux_params_t G_ux_params;
#else // TARGET_NANOS
extern const ux_menu_entry_t menu_main[4];
extern const ux_menu_entry_t menu_about[3];
extern ux_state_t ux;
#endif // TARGET_NANOX
