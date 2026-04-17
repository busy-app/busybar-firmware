/***************************************************************************//**
 * @file trust-center-baremetal-wrapper.c
 * @brief internal implementations for 'trust-center' as a thin-wrapper
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
// automatically generated from trust-center.h.  Do not manually edit
#include "stack/include/trust-center.h"
#include "stack/internal/inc/trust-center-internal-def.h"

sl_status_t sl_zigbee_broadcast_network_key_switch(void)
{
  return sli_zigbee_stack_broadcast_network_key_switch();
}

sl_status_t sl_zigbee_broadcast_next_network_key(sl_zigbee_key_data_t *key)
{
  return sli_zigbee_stack_broadcast_next_network_key(key);
}

sl_zigbee_app_link_key_request_policy_t sl_zigbee_get_app_link_key_request_policy(void)
{
  return sli_zigbee_stack_get_app_link_key_request_policy();
}

uint16_t sl_zigbee_get_transient_device_table_timeout_ms(void)
{
  return sli_zigbee_stack_get_transient_device_table_timeout_ms();
}

sl_zigbee_tc_link_key_request_policy_t sl_zigbee_get_trust_center_link_key_request_policy(void)
{
  return sli_zigbee_stack_get_trust_center_link_key_request_policy();
}

sl_status_t sl_zigbee_send_remove_device(sl_802154_short_addr_t destShort,
                                         sl_802154_long_addr_t destLong,
                                         sl_802154_long_addr_t deviceToRemoveLong)
{
  return sli_zigbee_stack_send_remove_device(destShort,
                                             destLong,
                                             deviceToRemoveLong);
}

sl_status_t sl_zigbee_send_trust_center_link_key(sl_802154_short_addr_t destinationNodeId,
                                                 sl_802154_long_addr_t destinationEui64)
{
  return sli_zigbee_stack_send_trust_center_link_key(destinationNodeId,
                                                     destinationEui64);
}

sl_status_t sl_zigbee_send_unicast_network_key_update(sl_802154_short_addr_t targetShort,
                                                      sl_802154_long_addr_t targetLong,
                                                      const sl_zigbee_key_data_t *newKey)
{
  return sli_zigbee_stack_send_unicast_network_key_update(targetShort,
                                                          targetLong,
                                                          newKey);
}

void sl_zigbee_set_app_link_key_request_policy(sl_zigbee_app_link_key_request_policy_t policy)
{
  sli_zigbee_stack_set_app_link_key_request_policy(policy);
}

sl_status_t sl_zigbee_set_transient_device_table_timeout_ms(uint16_t timeout)
{
  return sli_zigbee_stack_set_transient_device_table_timeout_ms(timeout);
}

void sl_zigbee_set_trust_center_link_key_request_policy(sl_zigbee_tc_link_key_request_policy_t policy)
{
  sli_zigbee_stack_set_trust_center_link_key_request_policy(policy);
}

sl_status_t sl_zigbee_unicast_current_network_key(sl_802154_short_addr_t targetShort,
                                                  sl_802154_long_addr_t targetLong,
                                                  sl_802154_short_addr_t parentShortId)
{
  return sli_zigbee_stack_unicast_current_network_key(targetShort,
                                                      targetLong,
                                                      parentShortId);
}
