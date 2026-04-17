/***************************************************************************//**
 * @file stack-info-baremetal-wrapper.c
 * @brief internal implementations for 'stack-info' as a thin-wrapper
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
// automatically generated from stack-info.h.  Do not manually edit
#include "stack/include/stack-info.h"
#include "stack/internal/inc/stack-info-internal-def.h"

uint16_t sl_802154_get_tx_power_mode(void)
{
  return sli_802154_stack_get_tx_power_mode();
}

sl_status_t sl_802154_radio_get_scheduler_priorities(sl_802154_radio_priorities_t *priorities)
{
  return sli_802154_stack_radio_get_scheduler_priorities(priorities);
}

sl_status_t sl_802154_radio_get_scheduler_sliptime(uint32_t *slipTime)
{
  return sli_802154_stack_radio_get_scheduler_sliptime(slipTime);
}

sl_status_t sl_802154_radio_set_scheduler_sliptime(uint32_t slipTime)
{
  return sli_802154_stack_radio_set_scheduler_sliptime(slipTime);
}

sl_status_t sl_802154_set_tx_power_mode(uint16_t txPowerMode)
{
  return sli_802154_stack_set_tx_power_mode(txPowerMode);
}

void sl_mac_calibrate_current_channel(void)
{
  sli_mac_stack_calibrate_current_channel();
}

void sl_mac_get_csma_params(sl_mac_csma_parameters_t *csma_params)
{
  sli_mac_stack_get_csma_params(csma_params);
}

int8_t sl_mac_get_ed_cca_threshold(void)
{
  return sli_mac_stack_get_ed_cca_threshold();
}

void sl_mac_set_cca_threshold(int8_t threshold)
{
  sli_mac_stack_set_cca_threshold(threshold);
}

void sl_mac_set_csma_params(sl_mac_csma_parameters_t *csma_params)
{
  sli_mac_stack_set_csma_params(csma_params);
}

sl_status_t sl_zigbee_clear_counters(void)
{
  return sli_zigbee_stack_clear_counters();
}

bool sl_zigbee_counter_requires_destination_node_id(sl_zigbee_counter_type_t type)
{
  return sli_zigbee_stack_counter_requires_destination_node_id(type);
}

bool sl_zigbee_counter_requires_phy_index(sl_zigbee_counter_type_t type)
{
  return sli_zigbee_stack_counter_requires_phy_index(type);
}

uint8_t sl_zigbee_get_certificate_table_size(void)
{
  return sli_zigbee_stack_get_certificate_table_size();
}

uint8_t sl_zigbee_get_child_table_size(void)
{
  return sli_zigbee_stack_get_child_table_size();
}

uint8_t sl_zigbee_get_endpoint(uint8_t index)
{
  return sli_zigbee_stack_get_endpoint(index);
}

uint16_t sl_zigbee_get_endpoint_cluster(uint8_t endpoint,
                                        sl_zigbee_cluster_list_id_t listId,
                                        uint8_t listIndex)
{
  return sli_zigbee_stack_get_endpoint_cluster(endpoint,
                                               listId,
                                               listIndex);
}

uint8_t sl_zigbee_get_endpoint_count(void)
{
  return sli_zigbee_stack_get_endpoint_count();
}

bool sl_zigbee_get_endpoint_description(uint8_t endpoint,
                                        sl_zigbee_endpoint_description_t *result)
{
  return sli_zigbee_stack_get_endpoint_description(endpoint,
                                                   result);
}

uint8_t * sl_zigbee_get_eui64(void)
{
  return sli_zigbee_stack_get_eui64();
}

void sl_zigbee_get_extended_pan_id(uint8_t *resultLocation)
{
  sli_zigbee_stack_get_extended_pan_id(resultLocation);
}

uint8_t sl_zigbee_get_initial_neighbor_outgoing_cost(void)
{
  return sli_zigbee_stack_get_initial_neighbor_outgoing_cost();
}

uint8_t sl_zigbee_get_key_table_size(void)
{
  return sli_zigbee_stack_get_key_table_size();
}

uint16_t sl_zigbee_get_manufacturer_code(void)
{
  return sli_zigbee_stack_get_manufacturer_code();
}

uint8_t sl_zigbee_get_max_nwk_retries(void)
{
  return sli_zigbee_stack_get_max_nwk_retries();
}

sl_status_t sl_zigbee_get_neighbor(uint8_t index,
                                   sl_zigbee_neighbor_table_entry_t *result)
{
  return sli_zigbee_stack_get_neighbor(index,
                                       result);
}

sl_status_t sl_zigbee_get_neighbor_frame_counter(sl_802154_long_addr_t nodeEui64,
                                                 uint32_t *returnFrameCounter)
{
  return sli_zigbee_stack_get_neighbor_frame_counter(nodeEui64,
                                                     returnFrameCounter);
}

sl_status_t sl_zigbee_get_network_parameters(sl_zigbee_node_type_t *node_type,
                                             sl_zigbee_network_parameters_t *parameters)
{
  return sli_zigbee_stack_get_network_parameters(node_type,
                                                 parameters);
}

sl_status_t sl_zigbee_get_node_descriptor(sl_zigbee_node_descriptor_info_t *descriptor)
{
  return sli_zigbee_stack_get_node_descriptor(descriptor);
}

sl_802154_short_addr_t sl_zigbee_get_node_id(void)
{
  return sli_zigbee_stack_get_node_id();
}

sl_802154_pan_id_t sl_zigbee_get_pan_id(void)
{
  return sli_zigbee_stack_get_pan_id();
}

uint8_t sl_zigbee_get_phy_interface_count(void)
{
  return sli_zigbee_stack_get_phy_interface_count();
}

uint8_t sl_zigbee_get_radio_channel(void)
{
  return sli_zigbee_stack_get_radio_channel();
}

sl_status_t sl_zigbee_get_radio_parameters(uint8_t phyIndex,
                                           sl_zigbee_multi_phy_radio_parameters_t *parameters)
{
  return sli_zigbee_stack_get_radio_parameters(phyIndex,
                                               parameters);
}

int8_t sl_zigbee_get_radio_power(void)
{
  return sli_zigbee_stack_get_radio_power();
}

void * sl_zigbee_get_rail_handle(void)
{
  return sli_zigbee_stack_get_rail_handle();
}

sl_status_t sl_zigbee_get_route_table_entry(uint8_t index,
                                            sl_zigbee_route_table_entry_t *result)
{
  return sli_zigbee_stack_get_route_table_entry(index,
                                                result);
}

uint8_t sl_zigbee_get_route_table_size(void)
{
  return sli_zigbee_stack_get_route_table_size();
}

uint8_t sl_zigbee_get_routing_shortcut_threshold(void)
{
  return sli_zigbee_stack_get_routing_shortcut_threshold();
}

uint8_t sl_zigbee_get_source_route_table_size(void)
{
  return sli_zigbee_stack_get_source_route_table_size();
}

uint8_t sl_zigbee_get_stack_compliance_revision(void)
{
  return sli_zigbee_stack_get_stack_compliance_revision();
}

uint8_t sl_zigbee_get_supported_networks(void)
{
  return sli_zigbee_stack_get_supported_networks();
}

uint8_t sl_zigbee_get_zc_and_zr_count(void)
{
  return sli_zigbee_stack_get_zc_and_zr_count();
}

bool sl_zigbee_is_local_eui64(sl_802154_long_addr_t eui64)
{
  return sli_zigbee_stack_is_local_eui64(eui64);
}

bool sl_zigbee_is_reset_rejoining_neighbors_frame_counter_enabled(void)
{
  return sli_zigbee_stack_is_reset_rejoining_neighbors_frame_counter_enabled();
}

sl_status_t sl_zigbee_lookup_eui64_by_node_id(sl_802154_short_addr_t nodeId,
                                              sl_802154_long_addr_t eui64Return)
{
  return sli_zigbee_stack_lookup_eui64_by_node_id(nodeId,
                                                  eui64Return);
}

sl_status_t sl_zigbee_lookup_node_id_by_eui64(sl_802154_long_addr_t eui64,
                                              sl_802154_short_addr_t *node_id)
{
  return sli_zigbee_stack_lookup_node_id_by_eui64(eui64,
                                                  node_id);
}

uint32_t sl_zigbee_ms_to_next_stack_event(void)
{
  return sli_zigbee_stack_ms_to_next_stack_event();
}

uint8_t sl_zigbee_neighbor_count(void)
{
  return sli_zigbee_stack_neighbor_count();
}

sl_zigbee_network_status_t sl_zigbee_network_state(void)
{
  return sli_zigbee_stack_network_state();
}

uint8_t sl_zigbee_next_zigbee_sequence_number(void)
{
  return sli_zigbee_stack_next_zigbee_sequence_number();
}

void sl_zigbee_read_and_clear_counters(uint16_t *counter_array,
                                       uint8_t len)
{
  sli_zigbee_stack_read_and_clear_counters(counter_array,
                                           len);
}

void sl_zigbee_read_counters(uint16_t *counter_array,
                             uint8_t len)
{
  sli_zigbee_stack_read_counters(counter_array,
                                 len);
}

sl_status_t sl_zigbee_reset_counters_thresholds(void)
{
  return sli_zigbee_stack_reset_counters_thresholds();
}

void sl_zigbee_reset_rejoining_neighbors_frame_counter(bool reset)
{
  sli_zigbee_stack_reset_rejoining_neighbors_frame_counter(reset);
}

uint8_t sl_zigbee_security_level(void)
{
  return sli_zigbee_stack_security_level();
}

sl_status_t sl_zigbee_set_broken_route_error_code(uint8_t errorCode)
{
  return sli_zigbee_stack_set_broken_route_error_code(errorCode);
}

sl_status_t sl_zigbee_set_counter_threshold(sl_zigbee_counter_type_t type,
                                            uint16_t threshold)
{
  return sli_zigbee_stack_set_counter_threshold(type,
                                                threshold);
}

sl_status_t sl_zigbee_set_initial_neighbor_outgoing_cost(uint8_t cost)
{
  return sli_zigbee_stack_set_initial_neighbor_outgoing_cost(cost);
}

sl_status_t sl_zigbee_set_logical_and_radio_channel(uint8_t channel)
{
  return sli_zigbee_stack_set_logical_and_radio_channel(channel);
}

sl_status_t sl_zigbee_set_neighbor_frame_counter(sl_802154_long_addr_t nodeEui64,
                                                 uint32_t frameCounter)
{
  return sli_zigbee_stack_set_neighbor_frame_counter(nodeEui64,
                                                     frameCounter);
}

sl_status_t sl_zigbee_set_node_descriptor(sl_zigbee_node_descriptor_info_t *descriptor)
{
  return sli_zigbee_stack_set_node_descriptor(descriptor);
}

sl_status_t sl_zigbee_set_node_id(sl_802154_short_addr_t nodeId)
{
  return sli_zigbee_stack_set_node_id(nodeId);
}

sl_status_t sl_zigbee_set_power_descriptor(uint16_t descriptor)
{
  return sli_zigbee_stack_set_power_descriptor(descriptor);
}

sl_status_t sl_zigbee_set_radio_channel(uint8_t channel)
{
  return sli_zigbee_stack_set_radio_channel(channel);
}

sl_status_t sl_zigbee_set_radio_ieee802154_cca_mode(uint8_t ccaMode)
{
  return sli_zigbee_stack_set_radio_ieee802154_cca_mode(ccaMode);
}

sl_status_t sl_zigbee_set_radio_power(int8_t power)
{
  return sli_zigbee_stack_set_radio_power(power);
}

void sl_zigbee_set_route_table_size(uint8_t size)
{
  sli_zigbee_stack_set_route_table_size(size);
}

sl_status_t sl_zigbee_set_routing_shortcut_threshold(uint8_t costThresh)
{
  return sli_zigbee_stack_set_routing_shortcut_threshold(costThresh);
}

void sl_zigbee_set_security_level(uint8_t securityLevel)
{
  sli_zigbee_stack_set_security_level(securityLevel);
}

void sl_zigbee_set_source_route_table_size(uint8_t size)
{
  sli_zigbee_stack_set_source_route_table_size(size);
}

void sl_zigbee_set_stack_profile(uint8_t stackProfile)
{
  sli_zigbee_stack_set_stack_profile(stackProfile);
}

bool sl_zigbee_stack_is_up(void)
{
  return sli_zigbee_stack_stack_is_up();
}

uint8_t sl_zigbee_stack_profile(void)
{
  return sli_zigbee_stack_stack_profile();
}

void sl_zigbee_token_factory_reset(bool exclude_outgoing_fc,
                                   bool exclude_boot_counter)
{
  sli_zigbee_stack_token_factory_reset(exclude_outgoing_fc,
                                       exclude_boot_counter);
}

sl_status_t sl_zigbee_write_node_data(bool erase)
{
  return sli_zigbee_stack_write_node_data(erase);
}

uint8_t slx_zigbee_routing_get_route_record_policy(void)
{
  return slxi_zigbee_stack_routing_get_route_record_policy();
}

sl_status_t slx_zigbee_routing_set_route_record_policy(sl_zigbee_route_record_policy_type_t policy)
{
  return slxi_zigbee_stack_routing_set_route_record_policy(policy);
}
