#include "os_io_seproxyhal.h"
#include "structs.h"

#define MAX_CHARS_PER_LINE 10

extern const ux_menu_entry_t menu_main[4];
extern const ux_menu_entry_t menu_about[3];

extern const bagl_element_t bagl_ui_approval_send_nanos[9];
extern const bagl_element_t bagl_ui_regdelegate_nanos[9];
extern const bagl_element_t bagl_ui_secondsign_nanos[9];
extern const bagl_element_t bagl_ui_approval_nanos[5];
extern const bagl_element_t bagl_ui_warning_msg_possible_tx[5];
extern const bagl_element_t bagl_ui_text_review_nanos[5];
extern const bagl_element_t bagl_ui_multisignature_nanos[11];
extern const bagl_element_t bagl_ui_vote_nanos[9];
extern const bagl_element_t bagl_ui_address_review_nanos[5];
extern char lineBuffer[50];

void satoshiToString(uint64_t amount, char *out);
void lineBufferVoteProcessor(signContext_t *signContext, uint8_t step);
void lineBufferSendTxProcessor(signContext_t *signContext, uint8_t step);
void lineBufferSecondSignProcessor(signContext_t *signContext, uint8_t step);
void lineBufferRegDelegateTxProcessor(signContext_t *signContext, uint8_t step);
void lineBufferMultisigProcessor(signContext_t *signContext, uint8_t step);