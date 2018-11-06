#include "stdbool.h"
#include "ui_elements_s.h"
#include "dposutils.h"
#include "structs.h"

static bagl_element_t *io_seproxyhal_touch_exit(const bagl_element_t *e);
static bagl_element_t *io_seproxyhal_touch_approve(const bagl_element_t *e);
static bagl_element_t *io_seproxyhal_touch_deny(const bagl_element_t *e);

static void sign(cx_ecfp_private_key_t *privateKey, void *whatToSign, uint32_t length, unsigned char *output, bool isTx);

static void ui_idle(void);

static unsigned char display_text_part(void);

static void ui_text(void);

static void ui_approval(void);


unsigned int bagl_ui_address_review_nanos_button(unsigned int button_mask, unsigned int button_mask_counter);
unsigned int bagl_ui_warning_msg_possible_tx_button(unsigned int button_mask, unsigned int button_mask_counter);
unsigned int bagl_ui_approval_nanos_button(unsigned int button_mask, unsigned int button_mask_counter);

unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len);

unsigned int bagl_ui_text_review_nanos_button(unsigned int button_mask, unsigned int button_mask_counter);

void nullifyPrivKeyInContext();

void getSignContext(uint8_t *dataBuffer, signContext_t *whereTo);

void handleGetPublic(uint8_t *bip32DataBuffer);

void createPublicKeyResponse();