/***************************************************************************//**
 * @file child_baremetal_wrapper.c
 * @brief internal implementations for 'child' as a thin-wrapper
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
// automatically generated from child.h.  Do not manually edit
#include "stack/include/child.h"
#include "stack/internal/inc/child_internal_def.h"

sl_status_t sl_zigbee_add_child(sl_802154_short_addr_t shortId,
                                sl_802154_long_addr_t longId,
                                sl_zigbee_node_type_t nodeType)
{
  return sli_zigbee_stack_add_child(shortId,
                                    longId,
                                    nodeType);
}

uint8_t sl_zigbee_child_count(void)
{
  return sli_zigbee_stack_child_count();
}

sl_802154_short_addr_t sl_zigbee_child_id(uint8_t childIndex)
{
  return sli_zigbee_stack_child_id(childIndex);
}

uint8_t sl_zigbee_child_index(sl_802154_short_addr_t childId)
{
  return sli_zigbee_stack_child_index(childId);
}

int8_t sl_zigbee_child_power(uint8_t childIndex)
{
  return sli_zigbee_stack_child_power(childIndex);
}

sl_status_t sl_zigbee_clear_message_flag(sl_802154_short_addr_t childId)
{
  return sli_zigbee_stack_clear_message_flag(childId);
}

uint16_t sl_zigbee_current_stack_tasks(void)
{
  return sli_zigbee_stack_current_stack_tasks();
}

int8_t sl_zigbee_get_avg_parent_rssi(void)
{
  return sli_zigbee_stack_get_avg_parent_rssi();
}

sl_status_t sl_zigbee_get_child_data(uint8_t index,
                                     sl_zigbee_child_data_t *childData)
{
  return sli_zigbee_stack_get_child_data(index,
                                         childData);
}

sl_zigbee_keep_alive_mode_t sl_zigbee_get_keep_alive_mode(void)
{
  return sli_zigbee_stack_get_keep_alive_mode();
}

uint8_t sl_zigbee_get_max_end_device_children(void)
{
  return sli_zigbee_stack_get_max_end_device_children();
}

uint8_t * sl_zigbee_get_parent_eui64(void)
{
  return sli_zigbee_stack_get_parent_eui64();
}

sl_802154_short_addr_t sl_zigbee_get_parent_id(void)
{
  return sli_zigbee_stack_get_parent_id();
}

uint32_t sl_zigbee_get_parent_incoming_nwk_frame_counter(void)
{
  return sli_zigbee_stack_get_parent_incoming_nwk_frame_counter();
}

sl_802154_short_addr_t sl_zigbee_get_parent_node_id(void)
{
  return sli_zigbee_stack_get_parent_node_id();
}

bool sl_zigbee_parent_token_set(void)
{
  return sli_zigbee_stack_parent_token_set();
}

sl_status_t sl_zigbee_poll_for_data(void)
{
  return sli_zigbee_stack_poll_for_data();
}

sl_status_t sl_zigbee_remove_child(sl_802154_long_addr_t childEui64)
{
  return sli_zigbee_stack_remove_child(childEui64);
}

void sl_zigbee_remove_neighbor(sl_802154_short_addr_t shortId,
                               sl_802154_long_addr_t longId)
{
  sli_zigbee_stack_remove_neighbor(shortId,
                                   longId);
}

sl_status_t sl_zigbee_send_link_power_delta_request(void)
{
  return sli_zigbee_stack_send_link_power_delta_request();
}

sl_status_t sl_zigbee_set_child_data(uint8_t index,
                                     const sl_zigbee_child_data_t *childData)
{
  return sli_zigbee_stack_set_child_data(index,
                                         childData);
}

void sl_zigbee_set_child_power(uint8_t childIndex,
                               int8_t newPower)
{
  sli_zigbee_stack_set_child_power(childIndex,
                                   newPower);
}

sl_status_t sl_zigbee_set_child_timeout_option_mask(uint16_t mask)
{
  return sli_zigbee_stack_set_child_timeout_option_mask(mask);
}

sl_status_t sl_zigbee_set_keep_alive_mode(sl_zigbee_keep_alive_mode_t mode)
{
  return sli_zigbee_stack_set_keep_alive_mode(mode);
}

void sl_zigbee_set_mac_poll_failure_wait_time(uint32_t waitBeforeRetryIntervalMs)
{
  sli_zigbee_stack_set_mac_poll_failure_wait_time(waitBeforeRetryIntervalMs);
}

sl_status_t sl_zigbee_set_max_end_device_children(uint8_t max)
{
  return sli_zigbee_stack_set_max_end_device_children(max);
}

sl_status_t sl_zigbee_set_message_flag(sl_802154_short_addr_t childId)
{
  return sli_zigbee_stack_set_message_flag(childId);
}

sl_status_t sl_zigbee_set_parent_incoming_nwk_frame_counter(uint32_t value)
{
  return sli_zigbee_stack_set_parent_incoming_nwk_frame_counter(value);
}

void sl_zigbee_stack_power_down(void)
{
  sli_zigbee_stack_stack_power_down();
}

void sl_zigbee_stack_power_up(void)
{
  sli_zigbee_stack_stack_power_up();
}
