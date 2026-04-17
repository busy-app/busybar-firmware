/***************************************************************************//**
 * @file stack-info-ipc-command-messages.c
 * @brief internal wrappers for 'stack-info' ipc commands
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
#include "stack/internal/src/ipc/stack-info-ipc-command-messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_802154_stack_get_tx_power_mode_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_tx_power_mode.response.result = sli_802154_stack_get_tx_power_mode();
}

void sli_802154_stack_radio_get_scheduler_priorities_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.radio_get_scheduler_priorities.response.result = sli_802154_stack_radio_get_scheduler_priorities(&msg->data.radio_get_scheduler_priorities.request.priorities);
}

void sli_802154_stack_radio_get_scheduler_sliptime_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.radio_get_scheduler_sliptime.response.result = sli_802154_stack_radio_get_scheduler_sliptime(&msg->data.radio_get_scheduler_sliptime.request.slipTime);
}

void sli_802154_stack_radio_set_scheduler_sliptime_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.radio_set_scheduler_sliptime.response.result = sli_802154_stack_radio_set_scheduler_sliptime(msg->data.radio_set_scheduler_sliptime.request.slipTime);
}

void sli_802154_stack_set_tx_power_mode_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_tx_power_mode.response.result = sli_802154_stack_set_tx_power_mode(msg->data.set_tx_power_mode.request.txPowerMode);
}

void sli_mac_stack_calibrate_current_channel_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_mac_stack_calibrate_current_channel();
}

void sli_mac_stack_get_csma_params_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_mac_stack_get_csma_params(&msg->data.get_csma_params.request.csma_params);
}

void sli_mac_stack_get_ed_cca_threshold_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_ed_cca_threshold.response.result = sli_mac_stack_get_ed_cca_threshold();
}

void sli_mac_stack_set_cca_threshold_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_mac_stack_set_cca_threshold(msg->data.set_cca_threshold.request.threshold);
}

void sli_mac_stack_set_csma_params_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_mac_stack_set_csma_params(&msg->data.set_csma_params.request.csma_params);
}

void sli_zigbee_stack_clear_counters_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.clear_counters.response.result = sli_zigbee_stack_clear_counters();
}

void sli_zigbee_stack_counter_requires_destination_node_id_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.counter_requires_destination_node_id.response.result = sli_zigbee_stack_counter_requires_destination_node_id(msg->data.counter_requires_destination_node_id.request.type);
}

void sli_zigbee_stack_counter_requires_phy_index_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.counter_requires_phy_index.response.result = sli_zigbee_stack_counter_requires_phy_index(msg->data.counter_requires_phy_index.request.type);
}

void sli_zigbee_stack_get_certificate_table_size_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_certificate_table_size.response.result = sli_zigbee_stack_get_certificate_table_size();
}

void sli_zigbee_stack_get_child_table_size_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_child_table_size.response.result = sli_zigbee_stack_get_child_table_size();
}

void sli_zigbee_stack_get_endpoint_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_endpoint.response.result = sli_zigbee_stack_get_endpoint(msg->data.get_endpoint.request.index);
}

void sli_zigbee_stack_get_endpoint_cluster_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_endpoint_cluster.response.result = sli_zigbee_stack_get_endpoint_cluster(msg->data.get_endpoint_cluster.request.endpoint,
                                                                                         msg->data.get_endpoint_cluster.request.listId,
                                                                                         msg->data.get_endpoint_cluster.request.listIndex);
}

void sli_zigbee_stack_get_endpoint_count_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_endpoint_count.response.result = sli_zigbee_stack_get_endpoint_count();
}

void sli_zigbee_stack_get_endpoint_description_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_endpoint_description.response.result = sli_zigbee_stack_get_endpoint_description(msg->data.get_endpoint_description.request.endpoint,
                                                                                                 &msg->data.get_endpoint_description.request.result);
}

void sli_zigbee_stack_get_eui64_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_eui64.response.result = sli_zigbee_stack_get_eui64();
}

void sli_zigbee_stack_get_extended_pan_id_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_get_extended_pan_id(msg->data.get_extended_pan_id.request.resultLocation);
}

void sli_zigbee_stack_get_initial_neighbor_outgoing_cost_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_initial_neighbor_outgoing_cost.response.result = sli_zigbee_stack_get_initial_neighbor_outgoing_cost();
}

void sli_zigbee_stack_get_key_table_size_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_key_table_size.response.result = sli_zigbee_stack_get_key_table_size();
}

void sli_zigbee_stack_get_manufacturer_code_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_manufacturer_code.response.result = sli_zigbee_stack_get_manufacturer_code();
}

void sli_zigbee_stack_get_max_nwk_retries_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_max_nwk_retries.response.result = sli_zigbee_stack_get_max_nwk_retries();
}

void sli_zigbee_stack_get_neighbor_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_neighbor.response.result = sli_zigbee_stack_get_neighbor(msg->data.get_neighbor.request.index,
                                                                         &msg->data.get_neighbor.request.result);
}

void sli_zigbee_stack_get_neighbor_frame_counter_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_neighbor_frame_counter.response.result = sli_zigbee_stack_get_neighbor_frame_counter(msg->data.get_neighbor_frame_counter.request.nodeEui64,
                                                                                                     &msg->data.get_neighbor_frame_counter.request.returnFrameCounter);
}

void sli_zigbee_stack_get_network_parameters_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_network_parameters.response.result = sli_zigbee_stack_get_network_parameters(&msg->data.get_network_parameters.request.node_type,
                                                                                             &msg->data.get_network_parameters.request.parameters);
}

void sli_zigbee_stack_get_node_descriptor_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_node_descriptor.response.result = sli_zigbee_stack_get_node_descriptor(&msg->data.get_node_descriptor.request.descriptor);
}

void sli_zigbee_stack_get_node_id_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_node_id.response.result = sli_zigbee_stack_get_node_id();
}

void sli_zigbee_stack_get_pan_id_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_pan_id.response.result = sli_zigbee_stack_get_pan_id();
}

void sli_zigbee_stack_get_phy_interface_count_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_phy_interface_count.response.result = sli_zigbee_stack_get_phy_interface_count();
}

void sli_zigbee_stack_get_radio_channel_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_radio_channel.response.result = sli_zigbee_stack_get_radio_channel();
}

void sli_zigbee_stack_get_radio_parameters_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_radio_parameters.response.result = sli_zigbee_stack_get_radio_parameters(msg->data.get_radio_parameters.request.phyIndex,
                                                                                         &msg->data.get_radio_parameters.request.parameters);
}

void sli_zigbee_stack_get_radio_power_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_radio_power.response.result = sli_zigbee_stack_get_radio_power();
}

void sli_zigbee_stack_get_rail_handle_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_rail_handle.response.result = sli_zigbee_stack_get_rail_handle();
}

void sli_zigbee_stack_get_route_table_entry_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_route_table_entry.response.result = sli_zigbee_stack_get_route_table_entry(msg->data.get_route_table_entry.request.index,
                                                                                           &msg->data.get_route_table_entry.request.result);
}

void sli_zigbee_stack_get_route_table_size_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_route_table_size.response.result = sli_zigbee_stack_get_route_table_size();
}

void sli_zigbee_stack_get_routing_shortcut_threshold_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_routing_shortcut_threshold.response.result = sli_zigbee_stack_get_routing_shortcut_threshold();
}

void sli_zigbee_stack_get_source_route_table_size_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_source_route_table_size.response.result = sli_zigbee_stack_get_source_route_table_size();
}

void sli_zigbee_stack_get_stack_compliance_revision_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_stack_compliance_revision.response.result = sli_zigbee_stack_get_stack_compliance_revision();
}

void sli_zigbee_stack_get_supported_networks_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_supported_networks.response.result = sli_zigbee_stack_get_supported_networks();
}

void sli_zigbee_stack_get_zc_and_zr_count_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_zc_and_zr_count.response.result = sli_zigbee_stack_get_zc_and_zr_count();
}

void sli_zigbee_stack_is_local_eui64_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.is_local_eui64.response.result = sli_zigbee_stack_is_local_eui64(msg->data.is_local_eui64.request.eui64);
}

void sli_zigbee_stack_is_reset_rejoining_neighbors_frame_counter_enabled_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.is_reset_rejoining_neighbors_frame_counter_enabled.response.result = sli_zigbee_stack_is_reset_rejoining_neighbors_frame_counter_enabled();
}

void sli_zigbee_stack_lookup_eui64_by_node_id_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.lookup_eui64_by_node_id.response.result = sli_zigbee_stack_lookup_eui64_by_node_id(msg->data.lookup_eui64_by_node_id.request.nodeId,
                                                                                               msg->data.lookup_eui64_by_node_id.request.eui64Return);
}

void sli_zigbee_stack_lookup_node_id_by_eui64_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.lookup_node_id_by_eui64.response.result = sli_zigbee_stack_lookup_node_id_by_eui64(msg->data.lookup_node_id_by_eui64.request.eui64,
                                                                                               &msg->data.lookup_node_id_by_eui64.request.node_id);
}

void sli_zigbee_stack_ms_to_next_stack_event_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.ms_to_next_stack_event.response.result = sli_zigbee_stack_ms_to_next_stack_event();
}

void sli_zigbee_stack_neighbor_count_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.neighbor_count.response.result = sli_zigbee_stack_neighbor_count();
}

void sli_zigbee_stack_network_state_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.network_state.response.result = sli_zigbee_stack_network_state();
}

void sli_zigbee_stack_next_zigbee_sequence_number_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.next_zigbee_sequence_number.response.result = sli_zigbee_stack_next_zigbee_sequence_number();
}

void sli_zigbee_stack_read_and_clear_counters_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_read_and_clear_counters(msg->data.read_and_clear_counters.request.counter_array,
                                           msg->data.read_and_clear_counters.request.len);
}

void sli_zigbee_stack_read_counters_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_read_counters(msg->data.read_counters.request.counter_array,
                                 msg->data.read_counters.request.len);
}

void sli_zigbee_stack_reset_counters_thresholds_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.reset_counters_thresholds.response.result = sli_zigbee_stack_reset_counters_thresholds();
}

void sli_zigbee_stack_reset_rejoining_neighbors_frame_counter_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_reset_rejoining_neighbors_frame_counter(msg->data.reset_rejoining_neighbors_frame_counter.request.reset);
}

void sli_zigbee_stack_security_level_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.security_level.response.result = sli_zigbee_stack_security_level();
}

void sli_zigbee_stack_set_broken_route_error_code_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_broken_route_error_code.response.result = sli_zigbee_stack_set_broken_route_error_code(msg->data.set_broken_route_error_code.request.errorCode);
}

void sli_zigbee_stack_set_counter_threshold_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_counter_threshold.response.result = sli_zigbee_stack_set_counter_threshold(msg->data.set_counter_threshold.request.type,
                                                                                           msg->data.set_counter_threshold.request.threshold);
}

void sli_zigbee_stack_set_initial_neighbor_outgoing_cost_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_initial_neighbor_outgoing_cost.response.result = sli_zigbee_stack_set_initial_neighbor_outgoing_cost(msg->data.set_initial_neighbor_outgoing_cost.request.cost);
}

void sli_zigbee_stack_set_logical_and_radio_channel_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_logical_and_radio_channel.response.result = sli_zigbee_stack_set_logical_and_radio_channel(msg->data.set_logical_and_radio_channel.request.channel);
}

void sli_zigbee_stack_set_neighbor_frame_counter_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_neighbor_frame_counter.response.result = sli_zigbee_stack_set_neighbor_frame_counter(msg->data.set_neighbor_frame_counter.request.nodeEui64,
                                                                                                     msg->data.set_neighbor_frame_counter.request.frameCounter);
}

void sli_zigbee_stack_set_node_descriptor_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_node_descriptor.response.result = sli_zigbee_stack_set_node_descriptor(&msg->data.set_node_descriptor.request.descriptor);
}

void sli_zigbee_stack_set_node_id_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_node_id.response.result = sli_zigbee_stack_set_node_id(msg->data.set_node_id.request.nodeId);
}

void sli_zigbee_stack_set_power_descriptor_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_power_descriptor.response.result = sli_zigbee_stack_set_power_descriptor(msg->data.set_power_descriptor.request.descriptor);
}

void sli_zigbee_stack_set_radio_channel_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_radio_channel.response.result = sli_zigbee_stack_set_radio_channel(msg->data.set_radio_channel.request.channel);
}

void sli_zigbee_stack_set_radio_ieee802154_cca_mode_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_radio_ieee802154_cca_mode.response.result = sli_zigbee_stack_set_radio_ieee802154_cca_mode(msg->data.set_radio_ieee802154_cca_mode.request.ccaMode);
}

void sli_zigbee_stack_set_radio_power_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_radio_power.response.result = sli_zigbee_stack_set_radio_power(msg->data.set_radio_power.request.power);
}

void sli_zigbee_stack_set_route_table_size_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_route_table_size(msg->data.set_route_table_size.request.size);
}

void sli_zigbee_stack_set_routing_shortcut_threshold_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_routing_shortcut_threshold.response.result = sli_zigbee_stack_set_routing_shortcut_threshold(msg->data.set_routing_shortcut_threshold.request.costThresh);
}

void sli_zigbee_stack_set_security_level_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_security_level(msg->data.set_security_level.request.securityLevel);
}

void sli_zigbee_stack_set_source_route_table_size_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_source_route_table_size(msg->data.set_source_route_table_size.request.size);
}

void sli_zigbee_stack_set_stack_profile_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_stack_profile(msg->data.set_stack_profile.request.stackProfile);
}

void sli_zigbee_stack_stack_is_up_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.stack_is_up.response.result = sli_zigbee_stack_stack_is_up();
}

void sli_zigbee_stack_stack_profile_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.stack_profile.response.result = sli_zigbee_stack_stack_profile();
}

void sli_zigbee_stack_token_factory_reset_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_token_factory_reset(msg->data.token_factory_reset.request.exclude_outgoing_fc,
                                       msg->data.token_factory_reset.request.exclude_boot_counter);
}

void sli_zigbee_stack_write_node_data_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.write_node_data.response.result = sli_zigbee_stack_write_node_data(msg->data.write_node_data.request.erase);
}

void slxi_zigbee_stack_routing_get_route_record_policy_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.routing_get_route_record_policy.response.result = slxi_zigbee_stack_routing_get_route_record_policy();
}

void slxi_zigbee_stack_routing_set_route_record_policy_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.routing_set_route_record_policy.response.result = slxi_zigbee_stack_routing_set_route_record_policy(msg->data.routing_set_route_record_policy.request.policy);
}

// public entrypoints

uint16_t sl_802154_get_tx_power_mode(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_802154_stack_get_tx_power_mode_process_ipc_command, &msg);

  return msg.data.get_tx_power_mode.response.result;
}

sl_status_t sl_802154_radio_get_scheduler_priorities(sl_802154_radio_priorities_t *priorities)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (priorities != NULL) {
    msg.data.radio_get_scheduler_priorities.request.priorities = *priorities;
  }

  sli_zigbee_send_ipc_cmd(sli_802154_stack_radio_get_scheduler_priorities_process_ipc_command, &msg);

  if (priorities != NULL) {
    *priorities = msg.data.radio_get_scheduler_priorities.request.priorities;
  }

  return msg.data.radio_get_scheduler_priorities.response.result;
}

sl_status_t sl_802154_radio_get_scheduler_sliptime(uint32_t *slipTime)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (slipTime != NULL) {
    msg.data.radio_get_scheduler_sliptime.request.slipTime = *slipTime;
  }

  sli_zigbee_send_ipc_cmd(sli_802154_stack_radio_get_scheduler_sliptime_process_ipc_command, &msg);

  if (slipTime != NULL) {
    *slipTime = msg.data.radio_get_scheduler_sliptime.request.slipTime;
  }

  return msg.data.radio_get_scheduler_sliptime.response.result;
}

sl_status_t sl_802154_radio_set_scheduler_sliptime(uint32_t slipTime)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.radio_set_scheduler_sliptime.request.slipTime = slipTime;
  sli_zigbee_send_ipc_cmd(sli_802154_stack_radio_set_scheduler_sliptime_process_ipc_command, &msg);

  return msg.data.radio_set_scheduler_sliptime.response.result;
}

sl_status_t sl_802154_set_tx_power_mode(uint16_t txPowerMode)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_tx_power_mode.request.txPowerMode = txPowerMode;
  sli_zigbee_send_ipc_cmd(sli_802154_stack_set_tx_power_mode_process_ipc_command, &msg);

  return msg.data.set_tx_power_mode.response.result;
}

void sl_mac_calibrate_current_channel(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_mac_stack_calibrate_current_channel_process_ipc_command, &msg);
}

void sl_mac_get_csma_params(sl_mac_csma_parameters_t *csma_params)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (csma_params != NULL) {
    msg.data.get_csma_params.request.csma_params = *csma_params;
  }

  sli_zigbee_send_ipc_cmd(sli_mac_stack_get_csma_params_process_ipc_command, &msg);

  if (csma_params != NULL) {
    *csma_params = msg.data.get_csma_params.request.csma_params;
  }
}

int8_t sl_mac_get_ed_cca_threshold(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_mac_stack_get_ed_cca_threshold_process_ipc_command, &msg);

  return msg.data.get_ed_cca_threshold.response.result;
}

void sl_mac_set_cca_threshold(int8_t threshold)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_cca_threshold.request.threshold = threshold;
  sli_zigbee_send_ipc_cmd(sli_mac_stack_set_cca_threshold_process_ipc_command, &msg);
}

void sl_mac_set_csma_params(sl_mac_csma_parameters_t *csma_params)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (csma_params != NULL) {
    msg.data.set_csma_params.request.csma_params = *csma_params;
  }

  sli_zigbee_send_ipc_cmd(sli_mac_stack_set_csma_params_process_ipc_command, &msg);

  if (csma_params != NULL) {
    *csma_params = msg.data.set_csma_params.request.csma_params;
  }
}

sl_status_t sl_zigbee_clear_counters(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_clear_counters_process_ipc_command, &msg);

  return msg.data.clear_counters.response.result;
}

bool sl_zigbee_counter_requires_destination_node_id(sl_zigbee_counter_type_t type)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.counter_requires_destination_node_id.request.type = type;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_counter_requires_destination_node_id_process_ipc_command, &msg);

  return msg.data.counter_requires_destination_node_id.response.result;
}

bool sl_zigbee_counter_requires_phy_index(sl_zigbee_counter_type_t type)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.counter_requires_phy_index.request.type = type;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_counter_requires_phy_index_process_ipc_command, &msg);

  return msg.data.counter_requires_phy_index.response.result;
}

uint8_t sl_zigbee_get_certificate_table_size(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_certificate_table_size_process_ipc_command, &msg);

  return msg.data.get_certificate_table_size.response.result;
}

uint8_t sl_zigbee_get_child_table_size(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_child_table_size_process_ipc_command, &msg);

  return msg.data.get_child_table_size.response.result;
}

uint8_t sl_zigbee_get_endpoint(uint8_t index)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_endpoint.request.index = index;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_endpoint_process_ipc_command, &msg);

  return msg.data.get_endpoint.response.result;
}

uint16_t sl_zigbee_get_endpoint_cluster(uint8_t endpoint,
                                        sl_zigbee_cluster_list_id_t listId,
                                        uint8_t listIndex)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_endpoint_cluster.request.endpoint = endpoint;
  msg.data.get_endpoint_cluster.request.listId = listId;
  msg.data.get_endpoint_cluster.request.listIndex = listIndex;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_endpoint_cluster_process_ipc_command, &msg);

  return msg.data.get_endpoint_cluster.response.result;
}

uint8_t sl_zigbee_get_endpoint_count(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_endpoint_count_process_ipc_command, &msg);

  return msg.data.get_endpoint_count.response.result;
}

bool sl_zigbee_get_endpoint_description(uint8_t endpoint,
                                        sl_zigbee_endpoint_description_t *result)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_endpoint_description.request.endpoint = endpoint;

  if (result != NULL) {
    msg.data.get_endpoint_description.request.result = *result;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_endpoint_description_process_ipc_command, &msg);

  if (result != NULL) {
    *result = msg.data.get_endpoint_description.request.result;
  }

  return msg.data.get_endpoint_description.response.result;
}

uint8_t * sl_zigbee_get_eui64(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_eui64_process_ipc_command, &msg);

  return msg.data.get_eui64.response.result;
}

void sl_zigbee_get_extended_pan_id(uint8_t *resultLocation)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (resultLocation != NULL) {
    memmove(msg.data.get_extended_pan_id.request.resultLocation, resultLocation, sizeof(uint8_t) * 16);
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_extended_pan_id_process_ipc_command, &msg);

  if (resultLocation != NULL) {
    memmove(resultLocation, msg.data.get_extended_pan_id.request.resultLocation, sizeof(uint8_t) * 16);
  }
}

uint8_t sl_zigbee_get_initial_neighbor_outgoing_cost(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_initial_neighbor_outgoing_cost_process_ipc_command, &msg);

  return msg.data.get_initial_neighbor_outgoing_cost.response.result;
}

uint8_t sl_zigbee_get_key_table_size(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_key_table_size_process_ipc_command, &msg);

  return msg.data.get_key_table_size.response.result;
}

uint16_t sl_zigbee_get_manufacturer_code(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_manufacturer_code_process_ipc_command, &msg);

  return msg.data.get_manufacturer_code.response.result;
}

uint8_t sl_zigbee_get_max_nwk_retries(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_max_nwk_retries_process_ipc_command, &msg);

  return msg.data.get_max_nwk_retries.response.result;
}

sl_status_t sl_zigbee_get_neighbor(uint8_t index,
                                   sl_zigbee_neighbor_table_entry_t *result)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_neighbor.request.index = index;

  if (result != NULL) {
    msg.data.get_neighbor.request.result = *result;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_neighbor_process_ipc_command, &msg);

  if (result != NULL) {
    *result = msg.data.get_neighbor.request.result;
  }

  return msg.data.get_neighbor.response.result;
}

sl_status_t sl_zigbee_get_neighbor_frame_counter(sl_802154_long_addr_t nodeEui64,
                                                 uint32_t *returnFrameCounter)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (nodeEui64 != NULL) {
    memmove(msg.data.get_neighbor_frame_counter.request.nodeEui64, nodeEui64, sizeof(sl_802154_long_addr_t));
  }

  if (returnFrameCounter != NULL) {
    msg.data.get_neighbor_frame_counter.request.returnFrameCounter = *returnFrameCounter;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_neighbor_frame_counter_process_ipc_command, &msg);

  if (nodeEui64 != NULL) {
    memmove(nodeEui64, msg.data.get_neighbor_frame_counter.request.nodeEui64, sizeof(sl_802154_long_addr_t));
  }

  if (returnFrameCounter != NULL) {
    *returnFrameCounter = msg.data.get_neighbor_frame_counter.request.returnFrameCounter;
  }

  return msg.data.get_neighbor_frame_counter.response.result;
}

sl_status_t sl_zigbee_get_network_parameters(sl_zigbee_node_type_t *node_type,
                                             sl_zigbee_network_parameters_t *parameters)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (node_type != NULL) {
    msg.data.get_network_parameters.request.node_type = *node_type;
  }

  if (parameters != NULL) {
    msg.data.get_network_parameters.request.parameters = *parameters;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_network_parameters_process_ipc_command, &msg);

  if (node_type != NULL) {
    *node_type = msg.data.get_network_parameters.request.node_type;
  }

  if (parameters != NULL) {
    *parameters = msg.data.get_network_parameters.request.parameters;
  }

  return msg.data.get_network_parameters.response.result;
}

sl_status_t sl_zigbee_get_node_descriptor(sl_zigbee_node_descriptor_info_t *descriptor)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (descriptor != NULL) {
    msg.data.get_node_descriptor.request.descriptor = *descriptor;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_node_descriptor_process_ipc_command, &msg);

  if (descriptor != NULL) {
    *descriptor = msg.data.get_node_descriptor.request.descriptor;
  }

  return msg.data.get_node_descriptor.response.result;
}

sl_802154_short_addr_t sl_zigbee_get_node_id(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_node_id_process_ipc_command, &msg);

  return msg.data.get_node_id.response.result;
}

sl_802154_pan_id_t sl_zigbee_get_pan_id(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_pan_id_process_ipc_command, &msg);

  return msg.data.get_pan_id.response.result;
}

uint8_t sl_zigbee_get_phy_interface_count(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_phy_interface_count_process_ipc_command, &msg);

  return msg.data.get_phy_interface_count.response.result;
}

uint8_t sl_zigbee_get_radio_channel(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_radio_channel_process_ipc_command, &msg);

  return msg.data.get_radio_channel.response.result;
}

sl_status_t sl_zigbee_get_radio_parameters(uint8_t phyIndex,
                                           sl_zigbee_multi_phy_radio_parameters_t *parameters)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_radio_parameters.request.phyIndex = phyIndex;

  if (parameters != NULL) {
    msg.data.get_radio_parameters.request.parameters = *parameters;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_radio_parameters_process_ipc_command, &msg);

  if (parameters != NULL) {
    *parameters = msg.data.get_radio_parameters.request.parameters;
  }

  return msg.data.get_radio_parameters.response.result;
}

int8_t sl_zigbee_get_radio_power(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_radio_power_process_ipc_command, &msg);

  return msg.data.get_radio_power.response.result;
}

void * sl_zigbee_get_rail_handle(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_rail_handle_process_ipc_command, &msg);

  return msg.data.get_rail_handle.response.result;
}

sl_status_t sl_zigbee_get_route_table_entry(uint8_t index,
                                            sl_zigbee_route_table_entry_t *result)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_route_table_entry.request.index = index;

  if (result != NULL) {
    msg.data.get_route_table_entry.request.result = *result;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_route_table_entry_process_ipc_command, &msg);

  if (result != NULL) {
    *result = msg.data.get_route_table_entry.request.result;
  }

  return msg.data.get_route_table_entry.response.result;
}

uint8_t sl_zigbee_get_route_table_size(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_route_table_size_process_ipc_command, &msg);

  return msg.data.get_route_table_size.response.result;
}

uint8_t sl_zigbee_get_routing_shortcut_threshold(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_routing_shortcut_threshold_process_ipc_command, &msg);

  return msg.data.get_routing_shortcut_threshold.response.result;
}

uint8_t sl_zigbee_get_source_route_table_size(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_source_route_table_size_process_ipc_command, &msg);

  return msg.data.get_source_route_table_size.response.result;
}

uint8_t sl_zigbee_get_stack_compliance_revision(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_stack_compliance_revision_process_ipc_command, &msg);

  return msg.data.get_stack_compliance_revision.response.result;
}

uint8_t sl_zigbee_get_supported_networks(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_supported_networks_process_ipc_command, &msg);

  return msg.data.get_supported_networks.response.result;
}

uint8_t sl_zigbee_get_zc_and_zr_count(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_zc_and_zr_count_process_ipc_command, &msg);

  return msg.data.get_zc_and_zr_count.response.result;
}

bool sl_zigbee_is_local_eui64(sl_802154_long_addr_t eui64)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (eui64 != NULL) {
    memmove(msg.data.is_local_eui64.request.eui64, eui64, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_is_local_eui64_process_ipc_command, &msg);

  if (eui64 != NULL) {
    memmove(eui64, msg.data.is_local_eui64.request.eui64, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.is_local_eui64.response.result;
}

bool sl_zigbee_is_reset_rejoining_neighbors_frame_counter_enabled(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_is_reset_rejoining_neighbors_frame_counter_enabled_process_ipc_command, &msg);

  return msg.data.is_reset_rejoining_neighbors_frame_counter_enabled.response.result;
}

sl_status_t sl_zigbee_lookup_eui64_by_node_id(sl_802154_short_addr_t nodeId,
                                              sl_802154_long_addr_t eui64Return)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.lookup_eui64_by_node_id.request.nodeId = nodeId;

  if (eui64Return != NULL) {
    memmove(msg.data.lookup_eui64_by_node_id.request.eui64Return, eui64Return, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_lookup_eui64_by_node_id_process_ipc_command, &msg);

  if (eui64Return != NULL) {
    memmove(eui64Return, msg.data.lookup_eui64_by_node_id.request.eui64Return, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.lookup_eui64_by_node_id.response.result;
}

sl_status_t sl_zigbee_lookup_node_id_by_eui64(sl_802154_long_addr_t eui64,
                                              sl_802154_short_addr_t *node_id)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (eui64 != NULL) {
    memmove(msg.data.lookup_node_id_by_eui64.request.eui64, eui64, sizeof(sl_802154_long_addr_t));
  }

  if (node_id != NULL) {
    msg.data.lookup_node_id_by_eui64.request.node_id = *node_id;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_lookup_node_id_by_eui64_process_ipc_command, &msg);

  if (eui64 != NULL) {
    memmove(eui64, msg.data.lookup_node_id_by_eui64.request.eui64, sizeof(sl_802154_long_addr_t));
  }

  if (node_id != NULL) {
    *node_id = msg.data.lookup_node_id_by_eui64.request.node_id;
  }

  return msg.data.lookup_node_id_by_eui64.response.result;
}

uint32_t sl_zigbee_ms_to_next_stack_event(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_ms_to_next_stack_event_process_ipc_command, &msg);

  return msg.data.ms_to_next_stack_event.response.result;
}

uint8_t sl_zigbee_neighbor_count(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_neighbor_count_process_ipc_command, &msg);

  return msg.data.neighbor_count.response.result;
}

sl_zigbee_network_status_t sl_zigbee_network_state(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_network_state_process_ipc_command, &msg);

  return msg.data.network_state.response.result;
}

uint8_t sl_zigbee_next_zigbee_sequence_number(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_next_zigbee_sequence_number_process_ipc_command, &msg);

  return msg.data.next_zigbee_sequence_number.response.result;
}

void sl_zigbee_read_and_clear_counters(uint16_t *counter_array,
                                       uint8_t len)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (len > SL_ZIGBEE_COUNTER_TYPE_COUNT) {
    assert(false); // "vector counter_array length exceeds expected maximum
  }

  memmove(msg.data.read_and_clear_counters.request.counter_array, counter_array, sizeof(uint16_t) * len);
  msg.data.read_and_clear_counters.request.len = len;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_read_and_clear_counters_process_ipc_command, &msg);

  if (len > SL_ZIGBEE_COUNTER_TYPE_COUNT) {
    assert(false); // "vector counter_array length exceeds expected maximum
  }

  memmove(counter_array, msg.data.read_and_clear_counters.request.counter_array, sizeof(uint16_t) * len);
}

void sl_zigbee_read_counters(uint16_t *counter_array,
                             uint8_t len)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (len > SL_ZIGBEE_COUNTER_TYPE_COUNT) {
    assert(false); // "vector counter_array length exceeds expected maximum
  }

  memmove(msg.data.read_counters.request.counter_array, counter_array, sizeof(uint16_t) * len);
  msg.data.read_counters.request.len = len;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_read_counters_process_ipc_command, &msg);

  if (len > SL_ZIGBEE_COUNTER_TYPE_COUNT) {
    assert(false); // "vector counter_array length exceeds expected maximum
  }

  memmove(counter_array, msg.data.read_counters.request.counter_array, sizeof(uint16_t) * len);
}

sl_status_t sl_zigbee_reset_counters_thresholds(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_reset_counters_thresholds_process_ipc_command, &msg);

  return msg.data.reset_counters_thresholds.response.result;
}

void sl_zigbee_reset_rejoining_neighbors_frame_counter(bool reset)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.reset_rejoining_neighbors_frame_counter.request.reset = reset;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_reset_rejoining_neighbors_frame_counter_process_ipc_command, &msg);
}

uint8_t sl_zigbee_security_level(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_security_level_process_ipc_command, &msg);

  return msg.data.security_level.response.result;
}

sl_status_t sl_zigbee_set_broken_route_error_code(uint8_t errorCode)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_broken_route_error_code.request.errorCode = errorCode;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_broken_route_error_code_process_ipc_command, &msg);

  return msg.data.set_broken_route_error_code.response.result;
}

sl_status_t sl_zigbee_set_counter_threshold(sl_zigbee_counter_type_t type,
                                            uint16_t threshold)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_counter_threshold.request.type = type;
  msg.data.set_counter_threshold.request.threshold = threshold;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_counter_threshold_process_ipc_command, &msg);

  return msg.data.set_counter_threshold.response.result;
}

sl_status_t sl_zigbee_set_initial_neighbor_outgoing_cost(uint8_t cost)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_initial_neighbor_outgoing_cost.request.cost = cost;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_initial_neighbor_outgoing_cost_process_ipc_command, &msg);

  return msg.data.set_initial_neighbor_outgoing_cost.response.result;
}

sl_status_t sl_zigbee_set_logical_and_radio_channel(uint8_t channel)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_logical_and_radio_channel.request.channel = channel;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_logical_and_radio_channel_process_ipc_command, &msg);

  return msg.data.set_logical_and_radio_channel.response.result;
}

sl_status_t sl_zigbee_set_neighbor_frame_counter(sl_802154_long_addr_t nodeEui64,
                                                 uint32_t frameCounter)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (nodeEui64 != NULL) {
    memmove(msg.data.set_neighbor_frame_counter.request.nodeEui64, nodeEui64, sizeof(sl_802154_long_addr_t));
  }

  msg.data.set_neighbor_frame_counter.request.frameCounter = frameCounter;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_neighbor_frame_counter_process_ipc_command, &msg);

  if (nodeEui64 != NULL) {
    memmove(nodeEui64, msg.data.set_neighbor_frame_counter.request.nodeEui64, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.set_neighbor_frame_counter.response.result;
}

sl_status_t sl_zigbee_set_node_descriptor(sl_zigbee_node_descriptor_info_t *descriptor)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (descriptor != NULL) {
    msg.data.set_node_descriptor.request.descriptor = *descriptor;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_node_descriptor_process_ipc_command, &msg);

  if (descriptor != NULL) {
    *descriptor = msg.data.set_node_descriptor.request.descriptor;
  }

  return msg.data.set_node_descriptor.response.result;
}

sl_status_t sl_zigbee_set_node_id(sl_802154_short_addr_t nodeId)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_node_id.request.nodeId = nodeId;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_node_id_process_ipc_command, &msg);

  return msg.data.set_node_id.response.result;
}

sl_status_t sl_zigbee_set_power_descriptor(uint16_t descriptor)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_power_descriptor.request.descriptor = descriptor;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_power_descriptor_process_ipc_command, &msg);

  return msg.data.set_power_descriptor.response.result;
}

sl_status_t sl_zigbee_set_radio_channel(uint8_t channel)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_radio_channel.request.channel = channel;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_radio_channel_process_ipc_command, &msg);

  return msg.data.set_radio_channel.response.result;
}

sl_status_t sl_zigbee_set_radio_ieee802154_cca_mode(uint8_t ccaMode)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_radio_ieee802154_cca_mode.request.ccaMode = ccaMode;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_radio_ieee802154_cca_mode_process_ipc_command, &msg);

  return msg.data.set_radio_ieee802154_cca_mode.response.result;
}

sl_status_t sl_zigbee_set_radio_power(int8_t power)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_radio_power.request.power = power;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_radio_power_process_ipc_command, &msg);

  return msg.data.set_radio_power.response.result;
}

void sl_zigbee_set_route_table_size(uint8_t size)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_route_table_size.request.size = size;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_route_table_size_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_set_routing_shortcut_threshold(uint8_t costThresh)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_routing_shortcut_threshold.request.costThresh = costThresh;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_routing_shortcut_threshold_process_ipc_command, &msg);

  return msg.data.set_routing_shortcut_threshold.response.result;
}

void sl_zigbee_set_security_level(uint8_t securityLevel)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_security_level.request.securityLevel = securityLevel;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_security_level_process_ipc_command, &msg);
}

void sl_zigbee_set_source_route_table_size(uint8_t size)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_source_route_table_size.request.size = size;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_source_route_table_size_process_ipc_command, &msg);
}

void sl_zigbee_set_stack_profile(uint8_t stackProfile)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_stack_profile.request.stackProfile = stackProfile;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_stack_profile_process_ipc_command, &msg);
}

bool sl_zigbee_stack_is_up(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_stack_is_up_process_ipc_command, &msg);

  return msg.data.stack_is_up.response.result;
}

uint8_t sl_zigbee_stack_profile(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_stack_profile_process_ipc_command, &msg);

  return msg.data.stack_profile.response.result;
}

void sl_zigbee_token_factory_reset(bool exclude_outgoing_fc,
                                   bool exclude_boot_counter)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.token_factory_reset.request.exclude_outgoing_fc = exclude_outgoing_fc;
  msg.data.token_factory_reset.request.exclude_boot_counter = exclude_boot_counter;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_token_factory_reset_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_write_node_data(bool erase)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.write_node_data.request.erase = erase;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_write_node_data_process_ipc_command, &msg);

  return msg.data.write_node_data.response.result;
}

uint8_t slx_zigbee_routing_get_route_record_policy(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(slxi_zigbee_stack_routing_get_route_record_policy_process_ipc_command, &msg);

  return msg.data.routing_get_route_record_policy.response.result;
}

sl_status_t slx_zigbee_routing_set_route_record_policy(sl_zigbee_route_record_policy_type_t policy)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.routing_set_route_record_policy.request.policy = policy;
  sli_zigbee_send_ipc_cmd(slxi_zigbee_stack_routing_set_route_record_policy_process_ipc_command, &msg);

  return msg.data.routing_set_route_record_policy.response.result;
}
