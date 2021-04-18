#include "../glyphs.h"
#include "../ui_utils.h"
#include "lisk_utils.h"
#include "os_io_seproxyhal.h"
#include <string.h>

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

/**
 * Sets ui to idle.
 */
void ui_idle() {
  if(G_ux.stack_count == 0) {
    ux_stack_push();
  }
  ux_flow_init(0, ux_idle_flow, NULL);
}
