//
// Created by andrea on 09/12/18.
//

#include "sendTx.h"
#include "../../approval.h"
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
static const bagl_element_t ui_send_el[] = {
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

static uint8_t step_processor_send(uint8_t step) {
  if (step == 2 && curLength == 0) {
    return 4;
  }
  return step + 1;
}

static void ui_processor_send(uint8_t step) {
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

static void ui_display_send() {
  ux.elements = ui_send_el;
  ux.elements_count = 11;
  totalSteps = 4;
  step_processor = step_processor_send;
  ui_processor = ui_processor_send;
}
#endif

#if defined(TARGET_NANOX)
UX_STEP_NOCB(
  ux_sign_tx_send_flow_1_step, 
  pnn, 
  {
    &C_nanox_icon_eye,
    "Confirm",
    "send",
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_send_flow_2_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    uint64_t address = deriveAddressFromPublic(&signContext.publicKey);
    deriveAddressStringRepresentation(address, lineBuffer);
  },
  {
    "Send from",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_send_flow_3_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    deriveAddressStringRepresentation(transaction.recipientId, lineBuffer);
  },
  {
    "To",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_send_flow_4_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    os_memmove(lineBuffer, message, MIN(50, curLength));
    // ellipsis
    if (curLength > 46) {
      strcpy(&lineBuffer[46], "...");
    }
  },
  {
    "Message",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_send_flow_5_step,
  bn,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    satoshiToString(transaction.amountSatoshi, lineBuffer);
  },
  {
    "Amount",
    lineBuffer,
  });
UX_STEP_CB(
  ux_sign_tx_send_flow_6_step,
  pb,
  touch_approve(),
  {
    &C_nanox_icon_validate_14,
    "Confirm",
  });
UX_STEP_CB(
  ux_sign_tx_send_flow_7_step,
  pb,
  touch_deny(),
  {
    &C_nanox_icon_crossmark,
    "Reject",
  });

const ux_flow_step_t * ux_sign_tx_send_flow[8];

static void ui_display_send() {
  int step = 0;
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_1_step;
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_2_step;
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_3_step;
  if (curLength > 0) ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_4_step;
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_5_step;
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_6_step;
  ux_sign_tx_send_flow[step++] = &ux_sign_tx_send_flow_7_step;
  ux_sign_tx_send_flow[step++] = FLOW_END_STEP;

  ux_flow_init(0, ux_sign_tx_send_flow, NULL);
}
#endif

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

void tx_end_send(transaction_t *tx) {
  // Remove signature and/or secondSignature from message.
  curLength = totalLengthAfterAsset - (totalLengthAfterAsset / 64) * 64;
  ui_display_send();
}
