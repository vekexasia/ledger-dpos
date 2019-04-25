//
// Created by andrea on 09/12/18.
//

#include "signMsg.h"
#include "../../io.h"
#include "../../ui_utils.h"
#include "cx.h"
#include "os.h"
#include "../dposutils.h"
#include "../approval.h"


static cx_sha256_t messageHash;

void handleSignMessagePacket(commPacket_t *packet, commContext_t *context) {
  // if first packet with signing header
  if ( packet->first ) {
    // Set signing context from first packet and patches the .data and .length by removing header length
    uint8_t varint[9];
    uint32_t headersLength = setSignContext(packet);

    // Rest sha256
    cx_sha256_init(&messageHash);

    // Signing header.
    uint64_t prefixLength = strlen(SIGNED_MESSAGE_PREFIX);
    uint8_t varintLength = encodeVarInt(prefixLength, varint);
    cx_hash(&messageHash, 0, varint, varintLength, NULL, 0);
    cx_hash(&messageHash, 0, SIGNED_MESSAGE_PREFIX, prefixLength, NULL, 0);

    varintLength = encodeVarInt(context->totalAmount - headersLength - 1, varint);
    cx_hash(&messageHash, 0, varint, varintLength, NULL, 0);

    prepareMsgLineBuffer(packet); //Enough data here for display purpose
  }

  cx_hash(&messageHash, 0, packet->data, packet->length, NULL, 0);

}

void prepareMsgLineBuffer(commPacket_t *packet) {
  os_memset(lineBuffer, 0, 50);
  uint8_t msgDisplayLenth = MIN(50, packet->length);
  os_memmove(lineBuffer, packet->data, msgDisplayLenth);
  if (msgDisplayLenth > 46) {
    os_memmove(lineBuffer+46, "...\0", 4);
  }

  uint8_t npc = 0; //Non Printable Chars Counter
  for (uint8_t i=0; i < msgDisplayLenth; i++) {
    npc += IS_PRINTABLE(lineBuffer[i]) ?
            0 /* Printable Char */:
            1 /* Non Printable Char */;
  }

  // We rewrite the line buffer to <binary data> in case >= than 40% is non printable or first char is not printable.
  if ((npc*100) / msgDisplayLenth >= 40 || ! IS_PRINTABLE(lineBuffer[0])) {
    os_memmove(lineBuffer, "< binary data >\0", 16);
  }
}

#if defined(TARGET_NANOS)

const bagl_element_t sign_message_ui[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Verify text", 0x00),
  ICON_CROSS(0x00),
  ICON_CHECK(0x00),
  LINEBUFFER,
 };

unsigned int sign_message_ui_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      ui_approval();
      break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
      touch_deny(NULL);
      break;
  }
  return 0;
}

#elif defined(TARGET_NANOX)
//////////////////////////////////////////////////////////////////////

UX_STEP_VALID(
    ux_sign_message_1_step,
    pnn,
    ui_approval(),
    {
      &C_icon_validate_14,
      "Verify text",
      lineBuffer,
    });
UX_STEP_VALID(
    ux_sign_message_2_step, 
    pb, 
    touch_deny(NULL),
    {
      &C_icon_crossmark,
      "Reject",
    });
UX_FLOW(ux_sign_message,
  &ux_sign_message_1_step,
  &ux_sign_message_2_step
);

//////////////////////////////////////////////////////////////////////
#endif

void processSignMessage(volatile unsigned int *flags) {
  uint8_t preFinalHash[32];
  uint8_t finalHash[32];

  // Close first sha256
  cx_hash(&messageHash, CX_LAST, NULL, 0, preFinalHash, 32);

  // Second sha256
  cx_hash_sha256(preFinalHash, 32, finalHash, 32);
  os_memmove(signContext.digest, finalHash, 32);

  // Init user flow.
  *flags |= IO_ASYNCH_REPLY;
#if defined(TARGET_NANOS)
  UX_DISPLAY(sign_message_ui, NULL);
#elif defined(TARGET_NANOX)
  ux_flow_init(0, ux_sign_message, NULL);
#endif // #if TARGET_ID
}
