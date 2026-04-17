/***************************************************************************//**
 * @file
 * @brief Security code for a normal (non-Trust Center) node.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "app/framework/include/af.h"
#define USE_REAL_SECURITY_PROTOTYPES
#include "app/framework/security/af-security.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif //SL_COMPONENT_CATALOG_PRESENT
//------------------------------------------------------------------------------

sl_status_t zaNodeSecurityInit(bool centralizedNetwork)
{
  sl_zigbee_initial_security_state_t state;
  sl_zigbee_extended_security_bitmask_t newExtended;
  sl_zigbee_extended_security_bitmask_t oldExtended;
  sl_status_t status;
  const sl_zigbee_af_security_profile_data_t *data = sli_zigbee_af_get_current_security_profile_data();

  if (data == NULL) {
    return SL_STATUS_FAIL;
  }

  memset(&state, 0, sizeof(sl_zigbee_initial_security_state_t));
  state.bitmask = data->nodeBitmask;
  newExtended = data->nodeExtendedBitmask;
  memmove(sl_zigbee_key_contents(&state.preconfiguredKey),
          sl_zigbee_key_contents(&data->preconfiguredKey),
          SL_ZIGBEE_ENCRYPTION_KEY_SIZE);

#if defined(ZA_CLI_FULL) || defined(SL_CATALOG_CLI_PRESENT)
  // This function will only be used if the full CLI is enabled,
  // and a value has been previously set via the "changekey" command.
  getLinkKeyFromCli(&(state.preconfiguredKey));
#endif

  sl_zigbee_get_extended_security_bitmask(&oldExtended);
  if ((oldExtended & SL_ZIGBEE_EXT_NO_FRAME_COUNTER_RESET) != 0U) {
    newExtended |= SL_ZIGBEE_EXT_NO_FRAME_COUNTER_RESET;
  }

  if (!centralizedNetwork) {
    state.bitmask |= SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE;
  }

  sl_zigbee_af_security_init_cb(&state, &newExtended, false); // trust center?

  sl_zigbee_af_security_println("set state to: 0x%04X", state.bitmask);
  status = sl_zigbee_set_initial_security_state(&state);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_security_println("security init node: 0x%02X", status);
    return status;
  }

  // Don't need to check on the status here, sl_zigbee_set_extended_security_bitmask
  // always returns SL_STATUS_OK.
  sl_zigbee_af_security_println("set extended security to: 0x%04X", newExtended);
  sl_zigbee_set_extended_security_bitmask(newExtended);

  sli_zigbee_af_clear_link_key_table();

  return SL_STATUS_OK;
}
