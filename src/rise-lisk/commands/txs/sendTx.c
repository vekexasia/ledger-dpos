//
// Created by andrea on 09/12/18.
//

#include "sendTx.h"
#include "../../dposutils.h"
#include "../../../io.h"
#include "../../../ui_utils.h"
#include "../../ui_elements_s.h"
#include "../signTx.h"

static char message[64];
static uint8_t curLength;
static uint16_t totalLengthAfterAsset;

/**
 * Sign with address
 */

#if defined(TARGET_NANOS)

static const bagl_element_t ui_send_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Send from", 0x01),
  TITLE_ITEM("To", 0x02),
  TITLE_ITEM("Message", 0x03),
  TITLE_ITEM("Amount", 0x04),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_ARROW_RIGHT(0x03),
  ICON_CHECK(0x04),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

#elif defined(TARGET_NANOX)

//////////////////////////////////////////////////////////////////////

void display_next_state(bool is_upper_border);

char caption[18];

UX_STEP_NOCB(ux_send_title_step, 
    pnn, 
    {
      &C_icon_eye,
      "Review",
      "transaction",
    });
UX_STEP_INIT(
    ux_init_upper_border,
    NULL,
    NULL,
    {
        display_next_state(true);
    });
UX_STEP_NOCB(
    ux_variable_display, 
    bnnn_paging,
    {
      .title = caption,
      .text = lineBuffer,
    });
UX_STEP_INIT(
    ux_init_lower_border,
    NULL,
    NULL,
    {
        display_next_state(false);
    });
UX_STEP_VALID(
    ux_send_approve_step, 
    pbb, 
    touch_approve(),
    {
      &C_icon_validate_14,
      "Accept",
      "and send",
    });
UX_STEP_VALID(
    ux_send_reject_step, 
    pb, 
    touch_deny(),
    {
      &C_icon_crossmark,
      "Reject",
    });
// confirm_full: confirm transaction / Amount: fullAmount / Address: fullAddress / Fees: feesAmount
UX_FLOW(ux_send,
  &ux_send_title_step,

  &ux_init_upper_border,
  &ux_variable_display,
  &ux_init_lower_border,

  &ux_send_approve_step,
  &ux_send_reject_step
);

uint8_t num_data;
volatile uint8_t current_data_index;
volatile uint8_t current_state;

#define INSIDE_BORDERS 0
#define OUT_OF_BORDERS 1

void set_state_data() {

    switch (current_data_index)
    {
        case 0:
            strncpy(caption, "Send from", sizeof(caption));
            uiProcessor_send(1);
            break;

        case 1:
            strncpy(caption, "Send to", sizeof(caption));
            uiProcessor_send(2);
            break;

        case 2:
            strncpy(caption, "Amount", sizeof(caption));
            uiProcessor_send(4);
            break;

        case 3:
            strncpy(caption, "Message", sizeof(caption));
            uiProcessor_send(3);
            break;
    
        default:
            THROW(0x6666);
            break;
    }

}

void display_next_state(bool is_upper_border){

    if(is_upper_border){ // walking over the first border
        if(current_state == OUT_OF_BORDERS){
            current_state = INSIDE_BORDERS;
            set_state_data();
            ux_flow_next();
        }
        else{
            if(current_data_index>0){
                current_data_index--;
                set_state_data();
                ux_flow_next();
            }
            else{
                current_state = OUT_OF_BORDERS;
                current_data_index = 0;
                ux_flow_prev();
            }
        }
    }
    else // walking over the second border
    {
        if(current_state == OUT_OF_BORDERS){
            current_state = INSIDE_BORDERS;
            set_state_data();
            ux_flow_prev();
        }
        else{
            if(num_data != 0 && current_data_index<num_data-1){
                current_data_index++;
                set_state_data();
                ux_flow_prev();
            }
            else{
                current_state = OUT_OF_BORDERS;
                ux_flow_next();
            }
        }
    }
    
}

//////////////////////////////////////////////////////////////////////
#endif // TARGET_NANOX


static uint8_t stepProcessor_send(uint8_t step) {
  if (step == 2 && curLength == 0) {
    return 4;
  }
  return step + 1;
}

static void uiProcessor_send(uint8_t step) {
  uint64_t address;
  os_memset(lineBuffer, 0, 50);
  switch (step) {
    case 1:
      address = deriveAddressFromPublic(&signContext.publicKey);
      deriveAddressStringRepresentation(address, lineBuffer);
      break;
    case 2:
      deriveAddressStringRepresentation(transaction.recipientId, lineBuffer);
      break;
    case 3:
      os_memmove(lineBuffer, message, MIN(50, curLength));
      // ellipsis
      if (curLength > 46) {
        os_memmove(lineBuffer + 46, "...\0", 4);
      }
      break;
    case 4:
      satoshiToString(transaction.amountSatoshi, lineBuffer);
  }
}

void tx_init_send() {
  curLength = 0;
  totalLengthAfterAsset = 0;
  os_memset(message, 0, 64);
}

void tx_chunk_send(uint8_t * data, uint8_t length, commPacket_t *sourcePacket, transaction_t *tx) {
  uint8_t toReadLength = MAX(0, MIN(64 - curLength, length));
  os_memmove(message+curLength, data, toReadLength);
  curLength += toReadLength;
  totalLengthAfterAsset += length;
}

#if defined(TARGET_NANOS)

void tx_end_send(transaction_t *tx) {

  curLength = totalLengthAfterAsset - (totalLengthAfterAsset / 64) * 64;
  // Remove signature and/or secondSignature from message.
  ux.elements = ui_send_nano;
  ux.elements_count = 11;
  totalSteps = 4;
  step_processor = stepProcessor_send;
  ui_processor = uiProcessor_send;
}

#elif defined(TARGET_NANOX)

void tx_end_send(transaction_t *tx) {

  curLength = totalLengthAfterAsset - (totalLengthAfterAsset / 64) * 64;
  
  current_state = OUT_OF_BORDERS;
  num_data = curLength == 0 ? 3 : 4;
  current_data_index = 0;

  ux_flow_init(0, ux_send, NULL);
}

#endif