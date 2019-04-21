#include <string.h>
#include "os_io_seproxyhal.h"
#include "dposutils.h"
#include "../glyphs.h"
#include "../ui_utils.h"

#if defined(TARGET_NANOS)
const ux_menu_entry_t menu_main[];
const ux_menu_entry_t menu_about[];
ux_state_t ux;

#define __NAME2(a, b) a##b
#define NAME2(a, b) __NAME2(a, b)

const ux_menu_entry_t menu_main[] = {
  {NULL, NULL, 0, &NAME2(C_nanos_, COINID) , "Use wallet to", "view accounts", 33, 12},
  {menu_about, NULL, 0, NULL, "About", NULL, 0, 0},
  {NULL, os_sched_exit, 0, &C_nanos_icon_dashboard, "Quit app", NULL, 50, 29},
  UX_MENU_END
};

const ux_menu_entry_t menu_about[] = {
  {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
  {NULL, NULL, 0, NULL, "Developer", "vekexasia", 0, 0},
  {menu_main, NULL, 2, &C_nanos_icon_back, "Back", NULL, 61, 40},
  UX_MENU_END
};
#endif

#if defined(TARGET_NANOX)
ux_state_t G_ux;
bolos_ux_params_t G_ux_params;

UX_STEP_NOCB(
  ux_idle_flow_1_step,
  nn,
  {
    "Application",
    "is ready",
  });
UX_STEP_NOCB(
  ux_idle_flow_2_step,
  bn,
  {
    "Version",
    APPVERSION,
  });
UX_STEP_CB(
  ux_idle_flow_3_step,
  pb,
  os_sched_exit(-1),
  {
    &C_nanox_icon_dashboard,
    "Quit",
  });
UX_FLOW(ux_idle_flow,
  &ux_idle_flow_1_step,
  &ux_idle_flow_2_step,
  &ux_idle_flow_3_step);
#endif

/**
 * Sets ui to idle.
 */
void ui_idle() {
  #if defined(TARGET_NANOS)
  UX_MENU_DISPLAY(0, menu_main, NULL);
  #endif

  #if defined(TARGET_NANOX)
  if(G_ux.stack_count == 0) {
    ux_stack_push();
  }
  ux_flow_init(0, ux_idle_flow, NULL);
  #endif
}
