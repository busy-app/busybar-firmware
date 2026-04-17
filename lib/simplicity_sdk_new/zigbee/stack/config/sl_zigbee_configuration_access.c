/***************************************************************************//**
 * @file
 * @brief Setters and getters to replace direct access to certain
 * data from sl_zigbee_configuration.c.

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

#include "stack/include/sl_zigbee.h"
#include "stack/include/message.h"
#include "stack/include/trust-center.h"
#include "stack/include/source-route.h"
#include "stack/include/security.h"
#include "stack/core/multi-pan.h"

extern sl_zigbee_multicast_table_entry_t sli_zigbee_multicast_table[];
extern uint8_t sli_zigbee_multicast_table_size;
extern uint16_t sli_zigbee_aps_ack_timeout_ms;
extern uint8_t sli_zigbee_binding_table_size;
extern uint8_t sli_zigbee_max_end_device_children;
extern uint16_t sli_zigbee_transient_device_timeout;
extern uint8_t sli_zigbee_stack_compliance_revision;
extern sl_zigbee_tc_link_key_request_policy_t sli_zigbee_trust_center_link_key_request_policies[];
extern sl_zigbee_app_link_key_request_policy_t sli_zigbee_app_link_key_request_policies[];
extern sl_zigbee_assume_trust_center_concentrator_type_t sli_zigbee_assumed_trust_center_concentrator_type;
extern uint16_t sli_zigbee_transient_key_timeout_s;
extern uint8_t sli_zigbee_route_table_size;
extern uint8_t sli_zigbee_source_route_table_size;
extern uint8_t sli_zigbee_end_device_configuration;
extern uint8_t sli_zigbee_supported_networks;
extern uint8_t sl_zigbee_key_table_size;
extern uint8_t sl_zigbee_child_table_size;
extern uint8_t sli_zigbee_zc_and_zr_count;
extern uint8_t sli_zigbee_certificate_table_size;

sl_zigbee_multicast_table_entry_t* sli_zigbee_stack_get_multicast_table(void)
{
  return sli_zigbee_multicast_table;
}

sl_status_t sli_zigbee_stack_set_multicast_table(const sl_zigbee_multicast_table_entry_t* entry)
{
  memmove(sli_zigbee_multicast_table,
          entry,
          sizeof(sl_zigbee_multicast_table_entry_t) * sli_zigbee_multicast_table_size);
  return SL_STATUS_OK;
}

sl_status_t sli_zigbee_stack_set_multicast_table_size(uint8_t size)
{
  sli_zigbee_multicast_table_size = size;
  return SL_STATUS_OK;
}

uint8_t sli_zigbee_stack_get_multicast_table_size(void)
{
  return sli_zigbee_multicast_table_size;
}

void sli_zigbee_stack_set_aps_ack_timeout_ms(uint16_t timeout)
{
  sli_zigbee_aps_ack_timeout_ms = timeout;
}

uint16_t sli_zigbee_stack_get_aps_ack_timeout_ms(void)
{
  return sli_zigbee_aps_ack_timeout_ms;
}

sl_status_t sli_zigbee_stack_set_binding_table_size(uint8_t size)
{
  sli_zigbee_binding_table_size = size;
  return SL_STATUS_OK;
}

uint8_t sli_zigbee_stack_get_binding_table_size(void)
{
  return sli_zigbee_binding_table_size;
}

sl_status_t sli_zigbee_stack_set_max_end_device_children(uint8_t max)
{
  sli_zigbee_max_end_device_children = max;
  return SL_STATUS_OK;
}

uint8_t sli_zigbee_stack_get_max_end_device_children(void)
{
  return sli_zigbee_max_end_device_children;
}

sl_status_t sli_zigbee_stack_set_transient_device_table_timeout_ms(uint16_t timeout)
{
  sl_status_t status = SL_STATUS_OK;
#ifdef SL_CATALOG_ZIGBEE_DELAYED_JOIN_PRESENT
  if (timeout >= SL_ZIGBEE_TRANSIENT_DEVICE_MINIMUM_TIMEOUT_MS) {
    sli_zigbee_transient_device_timeout = timeout;
  } else {
    status  = SL_STATUS_FAIL;
  }
#else
  (void)timeout;
#endif
  return status;
}

uint16_t sli_zigbee_stack_get_transient_device_table_timeout_ms(void)
{
  return sli_zigbee_transient_device_timeout;
}

uint8_t sli_zigbee_stack_get_stack_compliance_revision(void)
{
  return sli_zigbee_stack_compliance_revision;
}

void sli_zigbee_set_stack_compliance_revision(uint8_t revision)
{
  sli_zigbee_stack_compliance_revision = revision;
}

sl_zigbee_tc_link_key_request_policy_t sli_zigbee_stack_get_trust_center_link_key_request_policy(void)
{
  return sli_zigbee_trust_center_link_key_request_policies[sli_zigbee_get_network_index_for_forked_global()];
}

void sli_zigbee_stack_set_trust_center_link_key_request_policy(sl_zigbee_tc_link_key_request_policy_t policy)
{
  sli_zigbee_trust_center_link_key_request_policies[sli_zigbee_get_network_index_for_forked_global()] = policy;
}

sl_zigbee_app_link_key_request_policy_t sli_zigbee_stack_get_app_link_key_request_policy(void)
{
  return sli_zigbee_app_link_key_request_policies[sli_zigbee_get_network_index_for_forked_global()];
}

void sli_zigbee_stack_set_app_link_key_request_policy(sl_zigbee_app_link_key_request_policy_t policy)
{
  sli_zigbee_app_link_key_request_policies[sli_zigbee_get_network_index_for_forked_global()] = policy;
}

sl_zigbee_assume_trust_center_concentrator_type_t sli_zigbee_stack_get_assumed_trust_center_concentrator_type(void)
{
  return sli_zigbee_assumed_trust_center_concentrator_type;
}

void sli_zigbee_stack_set_assumed_trust_center_concentrator_type(sl_zigbee_assume_trust_center_concentrator_type_t type)
{
  sli_zigbee_assumed_trust_center_concentrator_type = type;
}

uint16_t sli_zigbee_stack_get_transient_key_timeout_s(void)
{
  return sli_zigbee_transient_key_timeout_s;
}

void sli_zigbee_stack_set_transient_key_timeout_s(uint16_t seconds)
{
  sli_zigbee_transient_key_timeout_s = seconds;
}

uint8_t sli_zigbee_stack_get_route_table_size(void)
{
  return sli_zigbee_route_table_size;
}

void sli_zigbee_stack_set_route_table_size(uint8_t size)
{
  sli_zigbee_route_table_size = size;
}

uint8_t sli_zigbee_stack_get_source_route_table_size(void)
{
  return sli_zigbee_source_route_table_size;
}
void sli_zigbee_stack_set_source_route_table_size(uint8_t size)
{
  sli_zigbee_source_route_table_size = size;
}

void sli_zigbee_set_end_device_configuration(uint8_t end_device_configuration)
{
  sli_zigbee_end_device_configuration = end_device_configuration;
}

uint8_t sli_zigbee_stack_get_supported_networks(void)
{
  return sli_zigbee_supported_networks;
}

uint8_t sli_zigbee_stack_get_key_table_size(void)
{
  return sl_zigbee_key_table_size;
}

uint8_t sli_zigbee_stack_get_child_table_size(void)
{
  return sl_zigbee_child_table_size;
}

uint8_t sli_zigbee_stack_get_zc_and_zr_count(void)
{
  return sli_zigbee_zc_and_zr_count;
}

uint8_t sli_zigbee_stack_get_certificate_table_size(void)
{
  return sli_zigbee_certificate_table_size;
}

//for certain unit tests to link to
#ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
WEAK(uint8_t sli_zigbee_get_network_index_for_forked_global(void))
{
  return 0;
}
#endif
