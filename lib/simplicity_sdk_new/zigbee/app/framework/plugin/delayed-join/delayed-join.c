/***************************************************************************//**
 * @file
 * @brief Code for delayed join.
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "app/framework/util/common.h"
#include "app/framework/util/util.h"

#include "app/framework/util/af-main.h"
#include "app/framework/security/af-security.h"
#include "delayed-join.h"

// *****************************************************************************
// Globals

// *****************************************************************************
// Functions

void sli_zigbee_af_delayed_join_trust_center_join_callback(sl_802154_short_addr_t newNodeId,
                                                           sl_802154_long_addr_t newNodeEui64,
                                                           sl_zigbee_device_update_t status,
                                                           sl_802154_short_addr_t parentOfNewNode)
{
  UNUSED_VAR(newNodeEui64);

  sl_zigbee_af_core_println("%s: 0x%04X %s %02X %s: 0x%04X",
                            SL_ZIGBEE_AF_PLUGIN_DELAYED_JOIN_PLUGIN_NAME,
                            newNodeId,
                            " is attempting to do ",
                            status,
                            " with parentId",
                            parentOfNewNode);
}

void sl_zigbee_af_delayed_join_stack_status_cb(sl_status_t status)
{
#if defined(EZSP_HOST)
  if (sl_zigbee_af_delayed_join_is_activated()) {
    if (status == SL_STATUS_NETWORK_UP) {
      sl_status_t status = sl_zigbee_af_set_ezsp_policy(SL_ZIGBEE_EZSP_TRUST_CENTER_POLICY,
                                                        (SL_ZIGBEE_EZSP_DECISION_ALLOW_JOINS
                                                         | SL_ZIGBEE_EZSP_DECISION_ALLOW_UNSECURED_REJOINS
                                                         | SL_ZIGBEE_EZSP_DECISION_DEFER_JOINS),
                                                        "Trust Center Policy",
                                                        "Delay joins");
    }
    if (SL_STATUS_OK != status) {
      sl_zigbee_af_core_println("%s: %s: 0x%08X",
                                SL_ZIGBEE_AF_PLUGIN_DELAYED_JOIN_PLUGIN_NAME,
                                "failed to configure delayed joining",
                                status);
    }
  }
#else
  UNUSED_VAR(status);
#endif // EZSP_HOST
}

void sl_zigbee_af_delayed_join_set_network_key_timeout(uint8_t timeout)
{
  #if defined(EZSP_HOST)
  if (SL_STATUS_OK != sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_NWK_KEY_TIMEOUT, 1, &timeout)) {
    sl_zigbee_af_core_println("%s: %s",
                              SL_ZIGBEE_AF_PLUGIN_DELAYED_JOIN_PLUGIN_NAME,
                              "\nCould not set the network timeout value. Acceptable timeout range[3,255]\n");
  }
  #else
  if (SL_STATUS_OK != sl_zigbee_setup_delayed_join(timeout)) {
    sl_zigbee_af_core_println("%s: %s",
                              SL_ZIGBEE_AF_PLUGIN_DELAYED_JOIN_PLUGIN_NAME,
                              "\nCould not set the network timeout value. Acceptable timeout range[3,255]\n");
  }
  #endif
}

void sl_zigbee_af_delayed_join_activate(bool activated)
{
#if defined(EZSP_HOST)
  if (SL_STATUS_OK != sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_DELAYED_JOIN_ACTIVATION, 1, (uint8_t*)&activated)) {
    sl_zigbee_af_core_println("%s: %s",
                              SL_ZIGBEE_AF_PLUGIN_DELAYED_JOIN_PLUGIN_NAME,
                              "failed to activate TC delayed joining\n");
  }
#else
  sl_zigbee_delayed_join_activate(activated);
#endif
}

bool sl_zigbee_af_delayed_join_is_activated(void)
{
  bool isActivated = false;
#if defined(EZSP_HOST)
  uint8_t valueLength = 1;
  (void) sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_DELAYED_JOIN_ACTIVATION,
                                  &valueLength,
                                  (uint8_t*)&isActivated);
#else
  isActivated = sl_zigbee_delayed_join_is_activated();
#endif
  return isActivated;
}
