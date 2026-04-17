/***************************************************************************//**
 * @file
 * @brief Definitions for the Update App Link Key plugin, which provides a way
 *        for devices to request an APS link key with another non-trust center
 *        device.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "update-app-link-key.h"

#define R21_COMPLIANCE_REVISION 21
static bool inRequest = false;

extern uint8_t sl_zigbee_get_stack_compliance_revision(void);

// -----------------------------------------------------------------------------
// Public API

sl_status_t sl_zigbee_af_update_app_link_key(sl_802154_long_addr_t eui)
{
  if (sl_zigbee_get_stack_compliance_revision() < R21_COMPLIANCE_REVISION) {
    // If the stack is pre-R21, we cannot fetch an app link key.
    sl_zigbee_af_core_println("%s: %s",
                              SL_ZIGBEE_AF_PLUGIN_UPDATE_APP_LINK_KEY_PLUGIN_NAME,
                              "Stack revision is pre-R21, cannot fetch app link key");
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (inRequest) {
    return SL_STATUS_NOT_READY;
  }

  sl_status_t status;

  status = sl_zigbee_update_app_link_key(eui);
  if (status == SL_STATUS_OK) {
    inRequest = true;
  }

  return status;
}

void sl_zigbee_af_update_app_link_key_zigbee_key_establishment_cb(sl_802154_long_addr_t partner,
                                                                  sl_zigbee_key_status_t status)
{
  if (inRequest) {
    sl_zigbee_af_core_print("%s:", SL_ZIGBEE_AF_PLUGIN_UPDATE_APP_LINK_KEY_PLUGIN_NAME);

    switch (status) {
      case SL_ZIGBEE_VERIFY_LINK_KEY_SUCCESS:
        sl_zigbee_af_core_print(" New key established:");
      // fall through
      case SL_ZIGBEE_APP_LINK_KEY_ESTABLISHED: // not an error condition. Should be followed by SL_ZIGBEE_VERIFY_LINK_KEY_SUCCESS
      case SL_ZIGBEE_PARTNER_KEY_UPDATE_TIMEOUT:
      case SL_ZIGBEE_FAILED_GET_AUTH_SECURITY:
      case SL_ZIGBEE_BAD_AUTH_SECURITY_RSP:
      case SL_ZIGBEE_INITIATOR_FAILED_REQUEST_KEY:
      case SL_ZIGBEE_VERIFY_LINK_KEY_FAILURE:
        sl_zigbee_af_core_println(" status: 0x%02X", status);
        sl_zigbee_af_core_print("Partner: ");
        sl_zigbee_af_print_big_endian_eui64(partner);
        sl_zigbee_af_core_println("");
        // Anything other than SL_ZIGBEE_APP_LINK_KEY_ESTABLISHED is a final state
        inRequest = (status == SL_ZIGBEE_APP_LINK_KEY_ESTABLISHED);
        sl_zigbee_af_update_app_link_key_status_cb(status);
        break;
      default:
        break;
    }
  } else {
    // We're not in request, which likely means that another node must have performed partner
    // link key update with us. Log a message on the console so long as the partner is not the
    // TC, since the update-tc-link-key component will handle that
    sl_802154_long_addr_t tc_eui = { 0 };
    (void)sl_zigbee_lookup_eui64_by_node_id(SL_ZIGBEE_ZIGBEE_COORDINATOR_ADDRESS, tc_eui);
    if ((status == SL_ZIGBEE_VERIFY_LINK_KEY_SUCCESS) && memcmp(tc_eui, partner, EUI64_SIZE)) {
      sl_zigbee_af_core_print("Partner link key established with: ");
      sl_zigbee_af_print_big_endian_eui64(partner);
      sl_zigbee_af_core_println("");
    }
  }
}
