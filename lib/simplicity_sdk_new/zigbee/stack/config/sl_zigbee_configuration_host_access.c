/***************************************************************************//**
 * @file
 * @brief Setters and getters for host apps to access certain configuration
 * values from sl_zigbee_configuration.c.  Host definitions for these APIs wrap
 * around EZSP configuration access calls.

 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_zigbee_types.h"
#include "ezsp-enum.h"
#include "app/util/ezsp/ezsp-utils.h"
#include "app/framework/include/af-types.h"
#include "app/util/ezsp/command-prototypes.h"
#include "app/framework/util/af-main.h"

sl_status_t sl_zigbee_set_multicast_table_size(uint8_t size)
{
  sl_status_t status = sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_MULTICAST_TABLE_SIZE,
                                                          (size),
                                                          "multicast table size");
  return status;
}

uint8_t sl_zigbee_get_multicast_table_size(void)
{
  uint16_t multicast_table_size;
  (void) sl_zigbee_ezsp_get_configuration_value(SL_ZIGBEE_EZSP_CONFIG_MULTICAST_TABLE_SIZE,
                                                (uint16_t*)&multicast_table_size);
  return (uint8_t)multicast_table_size;
}

sl_zigbee_multicast_table_entry_t* sl_zigbee_get_multicast_table(sl_zigbee_multicast_table_entry_t* table)
{
  uint8_t multicast_table_size = sl_zigbee_get_multicast_table_size();
  for (uint8_t i = 0; i < multicast_table_size; i++) {
    (void) sl_zigbee_ezsp_get_multicast_table_entry(i, &(table[i]));
  }
  return table;
}

sl_status_t sl_zigbee_set_multicast_table(sl_zigbee_multicast_table_entry_t* entry)
{
  uint8_t multicast_table_size = sl_zigbee_get_multicast_table_size();
  for (uint8_t i = 0; i < multicast_table_size; i++) {
    sl_status_t status = sl_zigbee_ezsp_set_multicast_table_entry(i, &(entry[i]));
    if (status != SL_STATUS_OK) {
      return status;
    }
  }
  return SL_STATUS_OK;
}

void sl_zigbee_set_aps_ack_timeout_ms(uint16_t timeout)
{
  (void) sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_APS_ACK_TIMEOUT,
                                            (timeout),
                                            "APS ACK timeout");
}

uint16_t sl_zigbee_get_aps_ack_timeout_ms(void)
{
  uint16_t timeout_ms;
  (void) sl_zigbee_ezsp_get_configuration_value(SL_ZIGBEE_EZSP_CONFIG_APS_ACK_TIMEOUT, &(timeout_ms));
  return timeout_ms;
}

sl_status_t sl_zigbee_set_binding_table_size(uint8_t size)
{
  sl_status_t status = sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_BINDING_TABLE_SIZE,
                                                          (size),
                                                          "binding table size");
  return status;
}

uint8_t sl_zigbee_get_binding_table_size(void)
{
  uint16_t binding_table_size;
  (void) sl_zigbee_ezsp_get_configuration_value(SL_ZIGBEE_EZSP_CONFIG_BINDING_TABLE_SIZE, (uint16_t*)&(binding_table_size));
  return (uint8_t)binding_table_size;
}

sl_status_t sl_zigbee_set_max_end_device_children(uint8_t max)
{
  sl_status_t status = sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_MAX_END_DEVICE_CHILDREN,
                                                          (max),
                                                          "max end device children");
  return status;
}

uint8_t sl_zigbee_get_max_end_device_children(void)
{
  uint16_t max_end_device_children;
  (void) sl_zigbee_ezsp_get_configuration_value(SL_ZIGBEE_EZSP_CONFIG_MAX_END_DEVICE_CHILDREN,
                                                (uint16_t*)&(max_end_device_children));
  return (uint8_t)max_end_device_children;
}

sl_status_t sl_zigbee_set_transient_device_table_timeout_ms(uint16_t timeout)
{
  (void) timeout;
  sl_status_t status = SL_STATUS_OK;
#ifdef SL_CATALOG_ZIGBEE_DELAYED_JOIN_PRESENT
  if (timeout < SL_ZIGBEE_TRANSIENT_DEVICE_MINIMUM_TIMEOUT_MS) {
    return SL_STATUS_FAIL;
  }
  status = sl_zigbee_af_set_ezsp_value(SL_ZIGBEE_EZSP_VALUE_TRANSIENT_DEVICE_TIMEOUT,
                                       2,
                                       (uint8_t*)&timeout,
                                       "default timeout for transient device table");
#endif
  return status;
}

uint16_t sl_zigbee_get_transient_device_table_timeout_ms(void)
{
  uint16_t transient_device_timeout = 0;
  uint8_t valueLength = 2;
  (void) sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_TRANSIENT_DEVICE_TIMEOUT,
                                  &valueLength,
                                  (uint8_t*)&(transient_device_timeout));
  return transient_device_timeout;
}

sl_zigbee_mac_passthrough_type_t sl_zigbee_get_mac_passthrough_flags(void)
{
  sl_zigbee_mac_passthrough_type_t value = SL_802154_PASSTHROUGH_NONE;
  uint8_t valueLength = 1;
  (void) sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_MAC_PASSTHROUGH_FLAGS,
                                  &valueLength,
                                  (uint8_t*)&value);
  return value;
}

void sl_zigbee_set_mac_passthrough_flags(sl_zigbee_mac_passthrough_type_t flags)
{
  (void) sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_MAC_PASSTHROUGH_FLAGS,
                                  1,
                                  (uint8_t*)&flags);
}

sl_802154_short_addr_t sl_zigbee_get_embernet_passthrough_source_address(void)
{
  sl_802154_short_addr_t address = SL_ZIGBEE_NULL_NODE_ID;
  uint8_t valueLength = 2;
  (void) sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_EMBERNET_PASSTHROUGH_SOURCE_ADDRESS,
                                  &valueLength,
                                  (uint8_t*)&(address));
  return address;
}

void sl_zigbee_set_embernet_passthrough_source_address(sl_802154_short_addr_t address)
{
  (void) sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_MAC_PASSTHROUGH_FLAGS,
                                  2,
                                  (uint8_t*)&address);
}

void sl_zigbee_set_transient_key_timeout_s(uint16_t seconds)
{
  (void) sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_TRANSIENT_KEY_TIMEOUT_S,
                                  2,
                                  (uint8_t*)&seconds);
}

uint16_t sl_zigbee_get_transient_key_timeout_ms(void)
{
  uint16_t timeout = 0;
  uint8_t valueLength = 2;
  (void) sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_TRANSIENT_KEY_TIMEOUT_S,
                                  &valueLength,
                                  (uint8_t*)&(timeout));
  return timeout;
}

sl_zigbee_assume_trust_center_concentrator_type_t sl_zigbee_get_assumed_trust_center_concentrator_type(void)
{
  uint16_t type = SL_ZIGBEE_ASSUME_TRUST_CENTER_IS_NOT_A_CONCENTRATOR;
  (void) sl_zigbee_ezsp_get_configuration_value(SL_ZIGBEE_EZSP_CONFIG_ASSUME_TC_CONCENTRATOR_TYPE,
                                                (uint16_t*)&type);
  return (sl_zigbee_assume_trust_center_concentrator_type_t)type;
}

void sl_zigbee_set_assumed_trust_center_concentrator_type(sl_zigbee_assume_trust_center_concentrator_type_t type)
{
  (void) sl_zigbee_ezsp_set_configuration_value(SL_ZIGBEE_EZSP_CONFIG_ASSUME_TC_CONCENTRATOR_TYPE,
                                                type);
}

//convert between the general EZSP decision type and individual policy types
#define EZSP_KEY_REQUEST_POLICY_MASK 0x0F
#define EZSP_TC_KEY_REQUEST_TYPE  0x50
#define EZSP_APP_KEY_REQUEST_TYPE 0x60

sl_zigbee_tc_link_key_request_policy_t sl_zigbee_get_trust_center_link_key_request_policy(void)
{
  sl_zigbee_tc_link_key_request_policy_t policy;
  (void) sl_zigbee_ezsp_get_policy(SL_ZIGBEE_EZSP_TC_KEY_REQUEST_POLICY,
                                   &policy);
  return (policy &= EZSP_KEY_REQUEST_POLICY_MASK);
}

void sl_zigbee_set_trust_center_link_key_request_policy(sl_zigbee_tc_link_key_request_policy_t policy)
{
  (void) sl_zigbee_ezsp_set_policy(SL_ZIGBEE_EZSP_TC_KEY_REQUEST_POLICY,
                                   (policy |= EZSP_TC_KEY_REQUEST_TYPE));
}

sl_zigbee_app_link_key_request_policy_t sl_zigbee_get_app_link_key_request_policy(void)
{
  sl_zigbee_app_link_key_request_policy_t policy;
  (void) sl_zigbee_ezsp_get_policy(SL_ZIGBEE_EZSP_APP_KEY_REQUEST_POLICY,
                                   &policy);
  return (policy &= EZSP_KEY_REQUEST_POLICY_MASK);
}

void sl_zigbee_set_app_link_key_request_policy(sl_zigbee_app_link_key_request_policy_t policy)
{
  (void) sl_zigbee_ezsp_set_policy(SL_ZIGBEE_EZSP_APP_KEY_REQUEST_POLICY,
                                   (policy |= EZSP_APP_KEY_REQUEST_TYPE));
}

void sl_zigbee_set_source_route_table_size(uint8_t size)
{
  (void) sl_zigbee_ezsp_set_configuration_value(SL_ZIGBEE_EZSP_CONFIG_SOURCE_ROUTE_TABLE_SIZE,
                                                size);
}

uint8_t sl_zigbee_get_source_route_table_size(void)
{
  uint16_t source_route_table_size;
  (void) sl_zigbee_ezsp_get_configuration_value(SL_ZIGBEE_EZSP_CONFIG_SOURCE_ROUTE_TABLE_SIZE,
                                                (uint16_t*)&source_route_table_size);
  return (uint8_t)source_route_table_size;
}

void sl_zigbee_set_route_table_size(uint8_t size)
{
  (void) sl_zigbee_ezsp_set_configuration_value(SL_ZIGBEE_EZSP_CONFIG_ROUTE_TABLE_SIZE,
                                                size);
}

uint8_t sl_zigbee_get_route_table_size(void)
{
  uint16_t size = 0;
  (void) sl_zigbee_ezsp_get_configuration_value(SL_ZIGBEE_EZSP_CONFIG_ROUTE_TABLE_SIZE,
                                                (uint16_t*)&(size));
  return (uint8_t)size;
}

void sli_zigbee_set_end_device_configuration(uint8_t end_device_configuration)
{
  (void) sl_zigbee_ezsp_set_configuration_value(SL_ZIGBEE_EZSP_CONFIG_END_DEVICE_CONFIGURATION,
                                                end_device_configuration);
}

void sl_zigbee_set_join_uses_install_code(bool enable)
{
  (void) sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_JOIN_USE_INSTALL_CODE_ENABLE, 1, (uint8_t*)&enable);
}

bool sl_zigbee_get_join_uses_install_code(void)
{
  bool enabled = false;
  uint8_t valueLength = 1;
  (void) sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_JOIN_USE_INSTALL_CODE_ENABLE,
                                  &valueLength,
                                  (uint8_t*)&enabled);
  return enabled;
}

bool sl_zigbee_gp_sink_table_remove_group(uint8_t index,
                                          uint16_t sinkGroupId,
                                          uint16_t assignedAlias)
{
  uint16_t gpSinkTableSize = 0;
  sl_zigbee_gp_sink_table_entry_t sinkEntry;
  if (SL_STATUS_OK != sl_zigbee_ezsp_gp_sink_table_get_entry(index, &sinkEntry)) {
    return false;
  }
  if (SL_STATUS_OK != sl_zigbee_ezsp_get_configuration_value(SL_ZIGBEE_EZSP_CONFIG_GP_SINK_TABLE_SIZE, (uint16_t*)&gpSinkTableSize)) {
    return false;
  }
  if ((index >= (uint8_t)gpSinkTableSize) || (sinkEntry.status != SL_ZIGBEE_GP_SINK_TABLE_ENTRY_STATUS_ACTIVE)) {
    // Index is out of range or entry isn't active, so can't remove
    return false;
  }

  // Search for entry and remove if we get a match
  for (uint8_t i = 0; i < GP_SINK_LIST_ENTRIES; i++) {
    if (sinkEntry.sinkList[i].type == SL_ZIGBEE_GP_SINK_TYPE_GROUPCAST
        && sinkEntry.sinkList[i].target.groupList.groupID == sinkGroupId
        && sinkEntry.sinkList[i].target.groupList.alias == assignedAlias) {
      sinkEntry.sinkList[i].type = SL_ZIGBEE_GP_SINK_TYPE_UNUSED;
      sinkEntry.sinkList[i].target.groupList.alias = 0;
      sinkEntry.sinkList[i].target.groupList.groupID = 0;

      // No need to check whether the entry actually changed. Token manager won't perform
      // the write if the stored token is the same as the token we intend to write.
      if (SL_STATUS_OK != sl_zigbee_ezsp_gp_sink_table_set_entry(index, &sinkEntry)) {
        return false;
      }
      return true;
    }
  }
  return false;
}

uint8_t sl_zigbee_get_stack_compliance_revision(void)
{
  uint8_t revision;
  uint8_t valueLength = 1;
  (void) sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_STACK_REVISION,
                                  &valueLength,
                                  (uint8_t*)&revision);
  return revision;
}
