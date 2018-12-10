#include "stdbool.h"
#include "coins/rise-lisk/ui_elements_s.h"
#include "coins/rise-lisk/dposutils.h"

static bagl_element_t *io_seproxyhal_touch_exit(const bagl_element_t *e);
static bagl_element_t *io_seproxyhal_touch_approve(const bagl_element_t *e);
static bagl_element_t *io_seproxyhal_touch_deny(const bagl_element_t *e);

static unsigned char display_text_part(void);

unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len);

unsigned int bagl_ui_text_review_nanos_button(unsigned int button_mask, unsigned int button_mask_counter);
