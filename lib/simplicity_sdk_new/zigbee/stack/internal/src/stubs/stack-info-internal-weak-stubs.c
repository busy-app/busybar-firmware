/***************************************************************************//**
 * @file stack-info-internal-weak-stubs.c
 * @brief stubbed definitions of internal implementations for stack-info
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
#include "stack/internal/inc/stack-info-internal-def.h"
#include "sl_common.h"

// Command Indirection

SL_WEAK uint16_t sli_802154_stack_get_tx_power_mode(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK sl_status_t sli_802154_stack_radio_get_scheduler_priorities(sl_802154_radio_priorities_t *priorities)
{
  // NOTE stub definition
  (void) priorities;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_802154_stack_radio_get_scheduler_sliptime(uint32_t *slipTime)
{
  // NOTE stub definition
  (void) slipTime;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_802154_stack_radio_set_scheduler_sliptime(uint32_t slipTime)
{
  // NOTE stub definition
  (void) slipTime;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_802154_stack_set_tx_power_mode(uint16_t txPowerMode)
{
  // NOTE stub definition
  (void) txPowerMode;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK void sli_mac_stack_calibrate_current_channel(void)
{
  // NOTE stub definition
}

SL_WEAK void sli_mac_stack_get_csma_params(sl_mac_csma_parameters_t *csma_params)
{
  // NOTE stub definition
  (void) csma_params;
}

SL_WEAK int8_t sli_mac_stack_get_ed_cca_threshold(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK void sli_mac_stack_set_cca_threshold(int8_t threshold)
{
  // NOTE stub definition
  (void) threshold;
}

SL_WEAK void sli_mac_stack_set_csma_params(sl_mac_csma_parameters_t *csma_params)
{
  // NOTE stub definition
  (void) csma_params;
}

SL_WEAK sl_status_t sli_zigbee_stack_clear_counters(void)
{
  // NOTE stub definition

  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK bool sli_zigbee_stack_counter_requires_destination_node_id(sl_zigbee_counter_type_t type)
{
  // NOTE stub definition
  (void) type;
  return false;
}

SL_WEAK bool sli_zigbee_stack_counter_requires_phy_index(sl_zigbee_counter_type_t type)
{
  // NOTE stub definition
  (void) type;
  return false;
}

SL_WEAK uint8_t sli_zigbee_stack_get_certificate_table_size(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK uint8_t sli_zigbee_stack_get_child_table_size(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK uint8_t sli_zigbee_stack_get_endpoint(uint8_t index)
{
  // NOTE stub definition
  (void) index;
  return 0;
}

SL_WEAK uint16_t sli_zigbee_stack_get_endpoint_cluster(uint8_t endpoint,
                                                       sl_zigbee_cluster_list_id_t listId,
                                                       uint8_t listIndex)
{
  // NOTE stub definition
  (void) endpoint;
  (void) listId;
  (void) listIndex;
  return 0;
}

SL_WEAK uint8_t sli_zigbee_stack_get_endpoint_count(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK bool sli_zigbee_stack_get_endpoint_description(uint8_t endpoint,
                                                       sl_zigbee_endpoint_description_t *result)
{
  // NOTE stub definition
  (void) endpoint;
  (void) result;
  return false;
}

SL_WEAK uint8_t * sli_zigbee_stack_get_eui64(void)
{
  // NOTE stub definition

  return NULL;
}

SL_WEAK void sli_zigbee_stack_get_extended_pan_id(uint8_t *resultLocation)
{
  // NOTE stub definition
  (void) resultLocation;
}

SL_WEAK uint8_t sli_zigbee_stack_get_initial_neighbor_outgoing_cost(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK uint8_t sli_zigbee_stack_get_key_table_size(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK uint16_t sli_zigbee_stack_get_manufacturer_code(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK uint8_t sli_zigbee_stack_get_max_nwk_retries(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK sl_status_t sli_zigbee_stack_get_neighbor(uint8_t index,
                                                  sl_zigbee_neighbor_table_entry_t *result)
{
  // NOTE stub definition
  (void) index;
  (void) result;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_zigbee_stack_get_neighbor_frame_counter(sl_802154_long_addr_t nodeEui64,
                                                                uint32_t *returnFrameCounter)
{
  // NOTE stub definition
  (void) nodeEui64;
  (void) returnFrameCounter;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_zigbee_stack_get_network_parameters(sl_zigbee_node_type_t *node_type,
                                                            sl_zigbee_network_parameters_t *parameters)
{
  // NOTE stub definition
  (void) node_type;
  (void) parameters;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_zigbee_stack_get_node_descriptor(sl_zigbee_node_descriptor_info_t *descriptor)
{
  // NOTE stub definition
  (void) descriptor;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_802154_short_addr_t sli_zigbee_stack_get_node_id(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK sl_802154_pan_id_t sli_zigbee_stack_get_pan_id(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK uint8_t sli_zigbee_stack_get_phy_interface_count(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK uint8_t sli_zigbee_stack_get_radio_channel(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK sl_status_t sli_zigbee_stack_get_radio_parameters(uint8_t phyIndex,
                                                          sl_zigbee_multi_phy_radio_parameters_t *parameters)
{
  // NOTE stub definition
  (void) phyIndex;
  (void) parameters;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK int8_t sli_zigbee_stack_get_radio_power(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK void * sli_zigbee_stack_get_rail_handle(void)
{
  // NOTE stub definition

  return NULL;
}

SL_WEAK sl_status_t sli_zigbee_stack_get_route_table_entry(uint8_t index,
                                                           sl_zigbee_route_table_entry_t *result)
{
  // NOTE stub definition
  (void) index;
  (void) result;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK uint8_t sli_zigbee_stack_get_route_table_size(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK uint8_t sli_zigbee_stack_get_routing_shortcut_threshold(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK uint8_t sli_zigbee_stack_get_source_route_table_size(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK uint8_t sli_zigbee_stack_get_stack_compliance_revision(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK uint8_t sli_zigbee_stack_get_supported_networks(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK uint8_t sli_zigbee_stack_get_zc_and_zr_count(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK bool sli_zigbee_stack_is_local_eui64(sl_802154_long_addr_t eui64)
{
  // NOTE stub definition
  (void) eui64;
  return false;
}

SL_WEAK bool sli_zigbee_stack_is_reset_rejoining_neighbors_frame_counter_enabled(void)
{
  // NOTE stub definition

  return false;
}

SL_WEAK sl_status_t sli_zigbee_stack_lookup_eui64_by_node_id(sl_802154_short_addr_t nodeId,
                                                             sl_802154_long_addr_t eui64Return)
{
  // NOTE stub definition
  (void) nodeId;
  (void) eui64Return;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_zigbee_stack_lookup_node_id_by_eui64(sl_802154_long_addr_t eui64,
                                                             sl_802154_short_addr_t *node_id)
{
  // NOTE stub definition
  (void) eui64;
  (void) node_id;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK uint32_t sli_zigbee_stack_ms_to_next_stack_event(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK uint8_t sli_zigbee_stack_neighbor_count(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK sl_zigbee_network_status_t sli_zigbee_stack_network_state(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK uint8_t sli_zigbee_stack_next_zigbee_sequence_number(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK void sli_zigbee_stack_read_and_clear_counters(uint16_t *counter_array,
                                                      uint8_t len)
{
  // NOTE stub definition
  (void) counter_array;
  (void) len;
}

SL_WEAK void sli_zigbee_stack_read_counters(uint16_t *counter_array,
                                            uint8_t len)
{
  // NOTE stub definition
  (void) counter_array;
  (void) len;
}

SL_WEAK sl_status_t sli_zigbee_stack_reset_counters_thresholds(void)
{
  // NOTE stub definition

  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK void sli_zigbee_stack_reset_rejoining_neighbors_frame_counter(bool reset)
{
  // NOTE stub definition
  (void) reset;
}

SL_WEAK uint8_t sli_zigbee_stack_security_level(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK sl_status_t sli_zigbee_stack_set_broken_route_error_code(uint8_t errorCode)
{
  // NOTE stub definition
  (void) errorCode;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_zigbee_stack_set_counter_threshold(sl_zigbee_counter_type_t type,
                                                           uint16_t threshold)
{
  // NOTE stub definition
  (void) type;
  (void) threshold;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_zigbee_stack_set_initial_neighbor_outgoing_cost(uint8_t cost)
{
  // NOTE stub definition
  (void) cost;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_zigbee_stack_set_logical_and_radio_channel(uint8_t channel)
{
  // NOTE stub definition
  (void) channel;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_zigbee_stack_set_neighbor_frame_counter(sl_802154_long_addr_t nodeEui64,
                                                                uint32_t frameCounter)
{
  // NOTE stub definition
  (void) nodeEui64;
  (void) frameCounter;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_zigbee_stack_set_node_descriptor(sl_zigbee_node_descriptor_info_t *descriptor)
{
  // NOTE stub definition
  (void) descriptor;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_zigbee_stack_set_node_id(sl_802154_short_addr_t nodeId)
{
  // NOTE stub definition
  (void) nodeId;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_zigbee_stack_set_power_descriptor(uint16_t descriptor)
{
  // NOTE stub definition
  (void) descriptor;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_zigbee_stack_set_radio_channel(uint8_t channel)
{
  // NOTE stub definition
  (void) channel;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_zigbee_stack_set_radio_ieee802154_cca_mode(uint8_t ccaMode)
{
  // NOTE stub definition
  (void) ccaMode;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_zigbee_stack_set_radio_power(int8_t power)
{
  // NOTE stub definition
  (void) power;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK void sli_zigbee_stack_set_route_table_size(uint8_t size)
{
  // NOTE stub definition
  (void) size;
}

SL_WEAK sl_status_t sli_zigbee_stack_set_routing_shortcut_threshold(uint8_t costThresh)
{
  // NOTE stub definition
  (void) costThresh;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK void sli_zigbee_stack_set_security_level(uint8_t securityLevel)
{
  // NOTE stub definition
  (void) securityLevel;
}

SL_WEAK void sli_zigbee_stack_set_source_route_table_size(uint8_t size)
{
  // NOTE stub definition
  (void) size;
}

SL_WEAK void sli_zigbee_stack_set_stack_profile(uint8_t stackProfile)
{
  // NOTE stub definition
  (void) stackProfile;
}

SL_WEAK bool sli_zigbee_stack_stack_is_up(void)
{
  // NOTE stub definition

  return false;
}

SL_WEAK uint8_t sli_zigbee_stack_stack_profile(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK void sli_zigbee_stack_token_factory_reset(bool exclude_outgoing_fc,
                                                  bool exclude_boot_counter)
{
  // NOTE stub definition
  (void) exclude_outgoing_fc;
  (void) exclude_boot_counter;
}

SL_WEAK sl_status_t sli_zigbee_stack_write_node_data(bool erase)
{
  // NOTE stub definition
  (void) erase;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK uint8_t slxi_zigbee_stack_routing_get_route_record_policy(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK sl_status_t slxi_zigbee_stack_routing_set_route_record_policy(sl_zigbee_route_record_policy_type_t policy)
{
  // NOTE stub definition
  (void) policy;
  return SL_STATUS_NOT_AVAILABLE;
}
