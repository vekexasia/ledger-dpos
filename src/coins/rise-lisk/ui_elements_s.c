#include <string.h>
#include "os_io_seproxyhal.h"
#include "dposutils.h"
#include "../../glyphs.h"
#include "../../ui_utils.h"

const ux_menu_entry_t menu_main[];
const ux_menu_entry_t menu_about[];


#define __NAME3(a, b, c) a##b##c
#define NAME3(a, b, c) __NAME3(a, b, c)

const ux_menu_entry_t menu_main[] = {
  {NULL, NULL, 0, &NAME3(C_badge_, COINID, ) , "Use wallet to", "view accounts", 33, 12},
  {menu_about, NULL, 0, NULL, "About", NULL, 0, 0},
  {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
  UX_MENU_END
};

const ux_menu_entry_t menu_about[] = {
  {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
  {NULL, NULL, 0, NULL, "Developer", "vekexasia", 0, 0},
  {menu_main, NULL, 2, &C_icon_back, "Back", NULL, 61, 40},
  UX_MENU_END
};

/**
 * Sign with address
 */
const bagl_element_t bagl_ui_approval_send_nanos[] = {
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

/**
 * Create second signature with address
 */
const bagl_element_t bagl_ui_secondsign_nanos[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Create second", 0x01),
  TITLE_ITEM("For account", 0x02),
  TITLE_ITEM("With public", 0x03),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_CHECK(0x03),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

/**
 * Create second signature with address
 */
const bagl_element_t bagl_ui_regdelegate_nanos[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Register", 0x01),
  TITLE_ITEM("For account", 0x02),
  TITLE_ITEM("With name", 0x03),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_CHECK(0x03),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

/**
 * Create second signature with address
 */
const bagl_element_t bagl_ui_vote_nanos[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Vote from", 0x01),
  TITLE_ITEM("Added", 0x02),
  TITLE_ITEM("Removed", 0x03),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_CHECK(0x03),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

/**
 * Sign with address
 */
const bagl_element_t bagl_ui_approval_nanos[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Sign with", 0x01),
  ICON_CHECK(0x00),
  ICON_CROSS(0x00),
  LINEBUFFER,
};


/**
 * Sign with address
 */
const bagl_element_t bagl_ui_multisignature_nanos[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Create", 0x01),
  TITLE_ITEM("Using", 0x02),
  TITLE_ITEM("Minimum", 0x03),
  TITLE_ITEM("Lifetime", 0x04),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_ARROW_RIGHT(0x03),
  ICON_CHECK(0x04),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

/**
 * Review text to sign (message)
 */
const bagl_element_t bagl_ui_text_review_nanos[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Verify text", 0x00),
  ICON_CROSS(0x00),
  ICON_CHECK(0x00),
  LINEBUFFER,
};

/**
 * Review text to sign (message)
 */
const bagl_element_t bagl_ui_address_review_nanos[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Verify Address", 0x00),
  ICON_CROSS(0x00),
  ICON_CHECK(0x00),
  LINEBUFFER,
};

uint8_t intToString(uint64_t amount, char *out) {
  uint8_t i = 0;
  if (amount == 0) {
    out[0] = '0';
    i = 1;
  } else {
    uint64_t part = amount;
    while(part > 0) {
      out[i++] = (uint8_t) (part % 10 + '0');
      part /= 10;
    }
  }
  out[i] = '\0';
  uint8_t j = 0;
  for (j=0; j<i/2; j++) {
    char swap = out[j];
    out[j] = out[i-1-j];
    out[i-1-j] = swap;
  }

  return i;
}

void satoshiToString(uint64_t amount, char *out) {

  uint64_t partInt = amount / 100000000;
  uint64_t partDecimal = amount - (partInt*100000000l) ;

  uint8_t i = 0;

  // TODO: Calc the # of digits for partInt
  while(partInt > 0) {
    out[i++] = (uint8_t) (partInt % 10 + '0');
    partInt /=10;
  }


  // Swap elements
  uint8_t j = 0;
  uint8_t tmp;
  for (; j<i/2; j++) {
    tmp = out[j];
    out[j] = out[i-1-j];
    out[i-1-j] = tmp;
  }


  if (partDecimal > 0) {
    out[i++] = '.';
    uint32_t satoshi = 10000000;
    while (satoshi > 0 && partDecimal > 0) {
      out[i++] = (uint8_t) (partDecimal / satoshi + '0');
      partDecimal -= (partDecimal/satoshi) * satoshi;
      satoshi /= 10;
    }
  }


}


void lineBufferRegDelegateTxProcessor(signContext_t *signContext, uint8_t step) {
  os_memset(lineBuffer, 0, 50);
  switch (step) {
    case 1:
      os_memmove(lineBuffer, "delegate\0", 11);
      break;
    case 2:
      deriveAddressStringRepresentation(signContext->sourceAddress, lineBuffer);
      break;
    case 3:
      os_memmove(lineBuffer, signContext->tx.shortDesc, 22);
      break;
  }
}


void lineBufferSecondSignProcessor(signContext_t *signContext, uint8_t step) {
  os_memset(lineBuffer, 0, 50);
  switch (step) {
    case 1:
      os_memmove(lineBuffer, "signature\0", 11);
      break;
    case 2:
      deriveAddressStringRepresentation(signContext->sourceAddress, lineBuffer);
      break;
    case 3:
      os_memmove(lineBuffer, signContext->tx.shortDesc, 22);
      break;
  }
}


void lineBufferVoteProcessor(signContext_t *signContext, uint8_t step) {
  uint64_t tmp = 0;
  os_memset(lineBuffer, 0, 50);
  switch (step) {
    case 1:
      deriveAddressStringRepresentation(signContext->sourceAddress, lineBuffer);
      break;
    case 2:
      // Added number
      tmp += signContext->tx.shortDesc[0];
      tmp = intToString(tmp, lineBuffer);
      break;
    case 3:
      // Removed number
      tmp += signContext->tx.shortDesc[1];
      tmp = intToString(tmp, lineBuffer);
      break;
  }
  if (tmp != 0) {
    os_memmove(lineBuffer+tmp, " votes\0", 7);
  }
}


void lineBufferSendTxProcessor(signContext_t *signContext, uint8_t step) {
  os_memset(lineBuffer, 0, 50);
  switch (step) {
    case 1:
      deriveAddressStringRepresentation(signContext->sourceAddress, lineBuffer);
      break;
    case 2:
      deriveAddressStringRepresentation(signContext->tx.recipientId, lineBuffer);
      break;
    case 3:
      os_memmove(lineBuffer, signContext->tx.message, MIN(50, strlen(signContext->tx.message)));
      if (strlen(signContext->tx.message) > 47) {
        os_memmove(lineBuffer+47, "...", 3);
      }
      break;
    case 4:
      satoshiToString(signContext->tx.amountSatoshi, lineBuffer);
      break;
  }
}


void lineBufferMultisigProcessor(signContext_t *signContext, uint8_t step) {
  uint64_t tmp = 0;
  os_memset(lineBuffer, 0, 50);
  switch (step) {
    case 1:
      os_memmove(lineBuffer, "Multi-sig account\0", 17);
      break;
    case 2:
      deriveAddressStringRepresentation(signContext->sourceAddress, lineBuffer);
      break;
    case 3:
      // Min keys
      tmp = intToString(signContext->tx.shortDesc[0], lineBuffer);
      os_memmove(lineBuffer+tmp, " keys\0", 6);
      break;
    case 4:
      // Lifetime
      tmp = intToString(signContext->tx.shortDesc[1], lineBuffer);
      os_memmove(lineBuffer+tmp, " hours\0", 7);
      break;
  }
}