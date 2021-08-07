#include "5_0_register_delegate.h"

#include "../../lisk_approval.h"
#include "../../lisk_utils.h"
#include "../../ui_elements_s.h"
#include "../../lisk_internals.h"
#include "common_parser.h"

UX_STEP_NOCB(
  ux_sign_tx_regmultisig_flow_1_step, 
  pnn, 
  {
    &C_nanox_icon_eye,
    "Register",
    "multisignature",
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_regmultisig_flow_2_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    os_memmove(lineBuffer, &reqContext.account.addressLisk32, ADDRESS_LISK32_LENGTH);
    lineBuffer[ADDRESS_LISK32_LENGTH] = '\0';
  },
  {
    "From account",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_regmultisig_flow_3_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    itoa(txContext.tx_asset._4_0_reg_multisig.n_keys, lineBuffer, 10);
  },
  {
    "Number of keys",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_regmultisig_flow_4_step,
  bnnn_paging,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    itoa(txContext.tx_asset._4_0_reg_multisig.n_mandatoryKeys, lineBuffer, 10);
  },
  {
    "Mandatory keys",
    lineBuffer,
  });
UX_STEP_NOCB_INIT(
  ux_sign_tx_regmultisig_flow_5_step,
  bn,
  {
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    satoshiToString(txContext.fee, lineBuffer);
  },
  {
    "Fee",
    lineBuffer,
  });
UX_STEP_CB(
  ux_sign_tx_regmultisig_flow_6_step,
  pb,
  touch_approve(),
  {
    &C_nanox_icon_validate_14,
    "Confirm",
  });
UX_STEP_CB(
  ux_sign_tx_regmultisig_flow_7_step,
  pb,
  touch_deny(),
  {
    &C_nanox_icon_crossmark,
    "Reject",
  });
UX_FLOW(ux_sign_tx_regmultisig_flow,
  &ux_sign_tx_regmultisig_flow_1_step,
  &ux_sign_tx_regmultisig_flow_2_step,
  &ux_sign_tx_regmultisig_flow_3_step,
  &ux_sign_tx_regmultisig_flow_4_step,
  &ux_sign_tx_regmultisig_flow_5_step,
  &ux_sign_tx_regmultisig_flow_6_step,
  &ux_sign_tx_regmultisig_flow_7_step);

static void ui_display_regmultisig() {
  ux_flow_init(0, ux_sign_tx_regmultisig_flow, NULL);
}

static void checkKeysValidity() {
  PRINTF("\n checkKeysValidity() \n");
  uint32_t totKeys = txContext.tx_asset._4_0_reg_multisig.n_mandatoryKeys + txContext.tx_asset._4_0_reg_multisig.n_optionalKeys;
  if(txContext.tx_asset._4_0_reg_multisig.n_keys != totKeys)
    THROW(INVALID_PARAMETER);
}

void tx_parse_specific_4_0_register_multisignature_group() {
  /**
   * TX_ASSET
   * - numberOfSignatures - uint32
   * - mandatoryKeys[] - 32 bytes
   * - optionalKeys[] - 32 bytes
   */
  unsigned char binaryKey = 0;
  uint32_t tmpSize = 0;

  while (txContext.tx_parsing_group == TX_ASSET)
  {
    switch(txContext.tx_parsing_state) {
    case BEGINNING:
    case PLACEHOLDER:
      txContext.tx_parsing_state = PLACEHOLDER;
      // let's be conservative
      is_available_to_parse(10);
      // Parse binaryKey and asset object size
      binaryKey = (unsigned char) transaction_get_varint();
      PRINTF("binaryKey _4_0_REG_MULTISIG_TX asset :\n %X \n\n", binaryKey);
      // Assets is serialized as bytes, varint first for the size
      tmpSize = (uint32_t) transaction_get_varint();
      PRINTF("asset object size _4_0_REG_MULTISIG_TX:\n %u \n\n", tmpSize);

    case _4_0_REG_MULTISIG_NUMKEYS:
      txContext.tx_parsing_state = _4_0_REG_MULTISIG_NUMKEYS;
      binaryKey = (unsigned char) transaction_get_varint();
      PRINTF("binaryKey _4_0_REG_MULTISIG_NUMKEYS:\n %X \n\n", binaryKey);
      txContext.tx_asset._4_0_reg_multisig.n_keys = (uint32_t) transaction_get_varint();
      PRINTF("txContext.asset.n_keys:\n %u \n\n", txContext.tx_asset._4_0_reg_multisig.n_keys);

    case _4_0_REG_MULTISIG_KEYS:
      txContext.tx_parsing_state = _4_0_REG_MULTISIG_KEYS;
      is_available_to_parse(ENCODED_PUB_KEY + 2); // pubkey + binarykey + size
      binaryKey = (unsigned char) transaction_get_varint();
      PRINTF("binaryKey _4_0_REG_MULTISIG_KEYS:\n %X \n\n", binaryKey);
      tmpSize = (uint32_t) transaction_get_varint();
      PRINTF("size _4_0_REG_MULTISIG_KEYS:\n %u \n\n", tmpSize);
      // just increase the pubkey, no need to save it
      transaction_offset_increase(ENCODED_PUB_KEY);

      if(binaryKey == 0x12)
        txContext.tx_asset._4_0_reg_multisig.n_mandatoryKeys++;
      else
        txContext.tx_asset._4_0_reg_multisig.n_optionalKeys++;

      // Exit the loop if there are no bytes to parse
      if(txContext.bytesRemaining == 0) {
        checkKeysValidity();
        txContext.tx_parsing_group = CHECK_SANITY_BEFORE_SIGN;
        txContext.tx_parsing_state = BEGINNING;
      }
      break;
    default:
      THROW(INVALID_STATE);
    }
  }
}

void tx_finalize_4_0_register_multisignature_group() {
  ui_display_regmultisig();
}
