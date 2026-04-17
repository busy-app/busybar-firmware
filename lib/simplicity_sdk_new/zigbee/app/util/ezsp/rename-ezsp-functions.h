/***************************************************************************//**
 * @file
 * @brief The file names certain functions that run on host to have
 * the ezsp- prefix for functions instead of the ember- prefix.
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

// TODO: we should update the EZSP generation script so that EZSP API wrappers are named just
// like the SoC API.

// Configuration Frames
#define sl_zigbee_set_passive_ack_config        sl_zigbee_ezsp_set_passive_ack_config

//Utility Frames
#define sl_zigbee_get_library_status               sl_zigbee_ezsp_get_library_status
#define sl_zigbee_get_phy_interface_count           sl_zigbee_ezsp_get_phy_interface_count
#define sl_zigbee_get_true_random_entropy_source     sl_zigbee_ezsp_get_true_random_entropy_source

//Networking Frames
#define sl_zigbee_set_manufacturer_code            sl_zigbee_ezsp_set_manufacturer_code
#define sl_zigbee_get_manufacturer_code            sl_zigbee_ezsp_get_manufacturer_code
#define sl_zigbee_set_power_descriptor             sl_zigbee_ezsp_set_power_descriptor
#define sl_zigbee_network_init                    sl_zigbee_ezsp_network_init
#define sl_zigbee_network_state                   sl_zigbee_ezsp_network_state
#define sl_zigbee_form_network                    sl_zigbee_ezsp_form_network
#define sl_zigbee_join_network                    sl_zigbee_ezsp_join_network
#define sl_zigbee_network_state                   sl_zigbee_ezsp_network_state
#define sl_zigbee_start_scan                      sl_zigbee_ezsp_start_scan
#define sl_zigbee_scan_for_unused_pan_id          sl_zigbee_ezsp_scan_for_unused_pan_id
#define sl_zigbee_stop_scan                       sl_zigbee_ezsp_stop_scan
#define sl_zigbee_form_network                    sl_zigbee_ezsp_form_network
#define sl_zigbee_join_network                    sl_zigbee_ezsp_join_network
#define sl_zigbee_leave_network                   sl_zigbee_ezsp_leave_network
#define sl_zigbee_permit_joining                  sl_zigbee_ezsp_permit_joining
#define sl_zigbee_energy_scan_request              sl_zigbee_ezsp_energy_scan_request
#define sl_zigbee_get_node_id                      sl_zigbee_ezsp_get_node_id
#define sl_zigbee_get_neighbor                    sl_zigbee_ezsp_get_neighbor
#define sl_zigbee_neighbor_count                  sl_zigbee_ezsp_neighbor_count
#define sl_zigbee_get_route_table_entry             sl_zigbee_ezsp_get_route_table_entry
#define sl_zigbee_set_radio_power                  sl_zigbee_ezsp_set_radio_power
#define sl_zigbee_set_radio_channel                sl_zigbee_ezsp_set_radio_channel
#define sl_zigbee_get_radio_channel                sl_zigbee_ezsp_get_radio_channel
#define sl_zigbee_set_radio_ieee802154_cca_mode      sl_zigbee_ezsp_set_radio_ieee802154_cca_mode
#define sl_zigbee_set_broken_route_error_code        sl_zigbee_ezsp_set_broken_route_error_code
#define sl_zigbee_get_neighbor_frame_counter        sl_zigbee_ezsp_get_neighbor_frame_counter
#define sl_zigbee_set_neighbor_frame_counter        sl_zigbee_ezsp_set_neighbor_frame_counter
#define sl_zigbee_get_child_data                   sl_zigbee_ezsp_get_child_data
#define sl_zigbee_set_child_data                   sl_zigbee_ezsp_set_child_data
#define sl_zigbee_set_initial_neighbor_outgoing_cost sl_zigbee_ezsp_set_initial_neighbor_outgoing_cost
#define sl_zigbee_get_initial_neighbor_outgoing_cost sl_zigbee_ezsp_get_initial_neighbor_outgoing_cost
#define sl_zigbee_reset_rejoining_neighbors_frame_counter sl_zigbee_ezsp_reset_rejoining_neighbors_frame_counter
#define sl_zigbee_is_reset_rejoining_neighbors_frame_counter_enabled sl_zigbee_ezsp_is_reset_rejoining_neighbors_frame_counter_enabled

#ifdef sl_zigbee_child_id
#undef sl_zigbee_child_id
#endif // sl_zigbee_child_id
#define sl_zigbee_child_id                        sl_zigbee_ezsp_child_id

#ifdef sl_zigbee_child_index
#undef sl_zigbee_child_index
#endif // sl_zigbee_child_index
#define sl_zigbee_child_index                     sl_zigbee_ezsp_child_index

#define sl_zigbee_set_concentrator                sl_zigbee_ezsp_set_concentrator
#define sl_zigbee_set_source_route_discovery_mode    sl_zigbee_ezsp_set_source_route_discovery_mode

#define sl_zigbee_concentrator_note_delivery_failure  ezspConcentratorNoteDeliveryFailure

#define sl_zigbee_multi_phy_start                  sl_zigbee_ezsp_multi_phy_start
#define sl_zigbee_multi_phy_stop                   sl_zigbee_ezsp_multi_phy_stop
#define sl_zigbee_multi_phy_set_radio_power          sl_zigbee_ezsp_multi_phy_set_radio_power
#define sl_zigbee_multi_phy_set_radio_channel        sl_zigbee_ezsp_multi_phy_set_radio_channel
#define sl_zigbee_get_duty_cycle_state              sl_zigbee_ezsp_get_duty_cycle_state
#define sl_zigbee_get_duty_cycle_limits             sl_zigbee_ezsp_get_duty_cycle_limits
#define sl_zigbee_get_radio_parameters             sl_zigbee_ezsp_get_radio_parameters
#define sl_zigbee_send_link_power_delta_request      sl_zigbee_ezsp_send_link_power_delta_request

#define sl_zigbee_write_node_data                  sl_zigbee_ezsp_write_node_data
#define sl_zigbee_get_routing_shortcut_threshold    sl_zigbee_ezsp_get_routing_shortcut_threshold
#define sl_zigbee_set_routing_shortcut_threshold    sl_zigbee_ezsp_set_routing_shortcut_threshold

#define sl_zigbee_clear_stored_beacons             sl_zigbee_ezsp_clear_stored_beacons
#define sl_zigbee_get_stored_beacon                 sl_zigbee_ezsp_get_stored_beacon
#define sl_zigbee_get_num_stored_beacons            sl_zigbee_ezsp_get_num_stored_beacons
#define sl_zigbee_join_network_directly            sl_zigbee_ezsp_join_network_directly
#define sl_zigbee_get_network_parameters           sl_zigbee_ezsp_get_network_parameters
#define sl_zigbee_get_extended_pan_id               sl_zigbee_ezsp_get_extended_pan_id

//Binding Frames
#define sl_zigbee_clear_binding_table              sl_zigbee_ezsp_clear_binding_table
#define sl_zigbee_set_binding                     sl_zigbee_ezsp_set_binding
#define sl_zigbee_get_binding                     sl_zigbee_ezsp_get_binding
#define sl_zigbee_delete_binding                  sl_zigbee_ezsp_delete_binding
#define sl_zigbee_binding_is_active                sl_zigbee_ezsp_binding_is_active
#define sl_zigbee_get_binding_remote_node_id         sl_zigbee_ezsp_get_binding_remote_node_id
#define sl_zigbee_set_binding_remote_node_id         sl_zigbee_ezsp_set_binding_remote_node_id

//Messaging Frames
#define sl_zigbee_send_unicast                       sl_zigbee_ezsp_send_unicast
#define sl_zigbee_send_broadcast                     sl_zigbee_ezsp_send_broadcast
#define sl_zigbee_send_multicast                     sl_zigbee_ezsp_send_multicast
#define sl_zigbee_send_many_to_one_route_request      sl_zigbee_ezsp_send_many_to_one_route_request
#define sl_zigbee_address_table_entry_is_active      sl_zigbee_ezsp_address_table_entry_is_active
#define sl_zigbee_set_address_table_info             sl_zigbee_ezsp_set_address_table_info
#define sl_zigbee_get_address_table_info             sl_zigbee_ezsp_get_address_table_info
#define sl_zigbee_set_extended_timeout             sl_zigbee_ezsp_set_extended_timeout
#define sl_zigbee_get_extended_timeout             sl_zigbee_ezsp_get_extended_timeout
#define sl_zigbee_lookup_node_id_by_eui64            sl_zigbee_ezsp_lookup_node_id_by_eui64
#define sl_zigbee_lookup_eui64_by_node_id            sl_zigbee_ezsp_lookup_eui64_by_node_id
#define sl_zigbee_set_mac_poll_failure_wait_time      sl_zigbee_ezsp_set_mac_poll_failure_wait_time
#define sl_zigbee_get_beacon_classification_params  sl_zigbee_ezsp_get_beacon_classification_params
#define sl_zigbee_set_beacon_classification_params  sl_zigbee_ezsp_set_beacon_classification_params
#define sl_zigbee_send_pan_id_update                sl_zigbee_ezsp_send_pan_id_update
#define sl_zigbee_proxy_next_broadcast_from_long    sl_zigbee_ezsp_proxy_next_broadcast_from_long
#define sl_zigbee_ezsp_message_sent_handler        sl_zigbee_message_sent_handler
#define sl_zigbee_ezsp_incoming_message_handler    sl_zigbee_incoming_message_handler
#define sl_zigbee_pending_acked_messages           sl_zigbee_ezsp_pending_acked_messages

//Token Interface Frames
#define sl_zigbee_get_token_count                  sl_zigbee_ezsp_get_token_count
#define sl_zigbee_get_token_info                   sl_zigbee_ezsp_get_token_info
#define sl_zigbee_get_token_data                   sl_zigbee_ezsp_get_token_data
#define sl_zigbee_set_token_data                   sl_zigbee_ezsp_set_token_data
#define sl_zigbee_reset_node                      sl_zigbee_ezsp_reset_node
#define sl_zigbee_token_factory_reset       sl_zigbee_ezsp_token_factory_reset

//Security Frames
#define sl_zigbee_set_initial_security_state        sl_zigbee_ezsp_set_initial_security_state
#define sl_zigbee_get_current_security_state        sl_zigbee_ezsp_get_current_security_state
#define sl_zigbee_find_key_table_entry              sl_zigbee_ezsp_find_key_table_entry
#define sl_zigbee_erase_key_table_entry             sl_zigbee_ezsp_erase_key_table_entry
#define sl_zigbee_clear_key_table                  sl_zigbee_ezsp_clear_key_table
#define sl_zigbee_request_link_key                 sl_zigbee_ezsp_request_link_key
#define sl_zigbee_clear_transient_link_keys         sl_zigbee_ezsp_clear_transient_link_keys
#define sl_zigbee_update_tc_link_key                sl_zigbee_ezsp_update_tc_link_key
#define sl_zigbee_send_trust_center_link_key         sl_zigbee_ezsp_send_trust_center_link_key
#define sl_zigbee_sec_man_get_network_key_info  sl_zigbee_ezsp_sec_man_get_network_key_info
#define sl_zigbee_sec_man_import_link_key       sl_zigbee_ezsp_sec_man_import_link_key
#define sl_zigbee_sec_man_check_key_context     sl_zigbee_ezsp_sec_man_check_key_context
#define sl_zigbee_sec_man_import_transient_key     sl_zigbee_ezsp_sec_man_import_transient_key
#define sl_zigbee_sec_man_export_transient_key_by_eui sl_zigbee_ezsp_sec_man_export_transient_key_by_eui
#define sl_zigbee_sec_man_export_transient_key_by_index sl_zigbee_ezsp_sec_man_export_transient_key_by_index
#define sl_zigbee_sec_man_export_link_key_by_index sl_zigbee_ezsp_sec_man_export_link_key_by_index
#define sl_zigbee_sec_man_get_aps_key_info         sl_zigbee_ezsp_sec_man_get_aps_key_info
#define sl_zigbee_set_incoming_tc_link_key_frame_counter sl_zigbee_ezsp_set_incoming_tc_link_key_frame_counter
#define sl_zigbee_aps_crypt_message                sl_zigbee_ezsp_aps_crypt_message

//ezspTrustCenter Frames
#define sl_zigbee_broadcast_next_network_key        sl_zigbee_ezsp_broadcast_next_network_key
#define sl_zigbee_broadcast_network_key_switch      sl_zigbee_ezsp_broadcast_network_key_switch

#define sl_zigbee_unicast_current_network_key       sl_zigbee_ezsp_unicast_current_network_key

// Certificate Based Key Exchange (CBKE)
#define sl_zigbee_generate_cbke_keys               sl_zigbee_ezsp_generate_cbke_keys
#define sl_zigbee_generate_cbke_keys_283k1          sl_zigbee_ezsp_generate_cbke_keys_283k1
#define sl_zigbee_calculate_smacs                 sl_zigbee_ezsp_calculate_smacs
#define sl_zigbee_calculate_smacs_283k1            sl_zigbee_ezsp_calculate_smacs_283k1
#define sl_zigbee_get_certificate                 sl_zigbee_ezsp_get_certificate
#define sl_zigbee_get_certificate_283k1            sl_zigbee_ezsp_get_certificate_283k1
#define sl_zigbee_dsa_verify                      sl_zigbee_ezsp_dsa_verify
#define sl_zigbee_set_preinstalled_cbke_data        sl_zigbee_ezsp_set_preinstalled_cbke_data
#define sl_zigbee_set_preinstalled_cbke_data_283k1   sl_zigbee_ezsp_set_preinstalled_cbke_data_283k1
#define sl_zigbee_clear_temporary_data_maybe_store_link_key      sl_zigbee_ezsp_clear_temporary_data_maybe_store_link_key
#define sl_zigbee_clear_temporary_data_maybe_store_link_key_283k1 sl_zigbee_ezsp_clear_temporary_data_maybe_store_link_key_283k1

#define sl_zigbee_d_gp_send sl_zigbee_ezsp_d_gp_send
#define sl_zigbee_gp_proxy_table_process_gp_pairing sl_zigbee_ezsp_gp_proxy_table_process_gp_pairing
#define sl_zigbee_gp_proxy_table_get_entry sl_zigbee_ezsp_gp_proxy_table_get_entry
#define sl_zigbee_gp_proxy_table_lookup sl_zigbee_ezsp_gp_proxy_table_lookup
#define sl_zigbee_gp_clear_proxy_table sl_zigbee_ezsp_gp_clear_proxy_table
#define sl_zigbee_gp_proxy_table_remove_entry sl_zigbee_ezsp_gp_proxy_table_remove_entry

#define sl_zigbee_gp_sink_table_get_entry sl_zigbee_ezsp_gp_sink_table_get_entry
#define sl_zigbee_gp_sink_table_lookup   sl_zigbee_ezsp_gp_sink_table_lookup
#define sl_zigbee_gp_sink_table_set_entry sl_zigbee_ezsp_gp_sink_table_set_entry
#define sl_zigbee_gp_sink_table_remove_entry sl_zigbee_ezsp_gp_sink_table_remove_entry
#define sl_zigbee_gp_sink_table_find_or_allocate_entry sl_zigbee_ezsp_gp_sink_table_find_or_allocate_entry
#define sl_zigbee_gp_sink_table_clear_all sl_zigbee_ezsp_gp_sink_table_clear_all
#define sl_zigbee_gp_sink_table_init        sl_zigbee_ezsp_gp_sink_table_init
#define sl_zigbee_gp_sink_table_set_security_frame_counter sl_zigbee_ezsp_gp_sink_table_set_security_frame_counter
#define sl_zigbee_gp_sink_table_get_number_of_active_entries sl_zigbee_ezsp_gp_sink_table_get_number_of_active_entries

// ZLL frames
#define sl_zigbee_zll_clear_tokens sl_zigbee_ezsp_zll_clear_tokens
#define sl_zigbee_zll_set_token_stack_zll_data sl_zigbee_ezsp_zll_set_data_token
#define sl_zigbee_zll_set_non_zll_network sl_zigbee_ezsp_zll_set_non_zll_network
#define sl_zigbee_is_zll_network sl_zigbee_ezsp_is_zll_network
#define sl_zigbee_zll_set_radio_idle_mode sl_zigbee_ezsp_zll_set_radio_idle_mode
#define sl_zigbee_zll_get_radio_idle_mode sl_zigbee_ezsp_zll_get_radio_idle_mode
#define sl_zigbee_set_zll_node_type sl_zigbee_ezsp_set_zll_node_type
#define sl_zigbee_set_zll_additional_state sl_zigbee_ezsp_set_zll_additional_state
#define sl_zigbee_zll_operation_in_progress sl_zigbee_ezsp_zll_operation_in_progress
#define sl_zigbee_zll_rx_on_when_idle_get_active sl_zigbee_ezsp_zll_rx_on_when_idle_get_active
#define sl_zigbee_get_zll_primary_channel_mask sl_zigbee_ezsp_get_zll_primary_channel_mask
#define sl_zigbee_get_zll_secondary_channel_mask sl_zigbee_ezsp_get_zll_secondary_channel_mask
#define sl_zigbee_set_zll_primary_channel_mask sl_zigbee_ezsp_set_zll_primary_channel_mask
#define sl_zigbee_set_zll_secondary_channel_mask sl_zigbee_ezsp_set_zll_secondary_channel_mask
#define sl_zigbee_set_logical_and_radio_channel sl_zigbee_ezsp_set_logical_and_radio_channel
#define sl_zigbee_zll_set_security_state_without_key sl_zigbee_ezsp_zll_set_security_state_without_key
#define sl_zigbee_zll_set_initial_security_state sl_zigbee_ezsp_zll_set_initial_security_state
#define sl_zigbee_zll_start_scan sl_zigbee_ezsp_zll_start_scan
#define sl_zigbee_zll_set_rx_on_when_idle sl_zigbee_ezsp_zll_set_rx_on_when_idle
#define zllGetTokens sl_zigbee_ezsp_zll_get_tokens
#define sl_zigbee_zll_set_rx_on_when_idle sl_zigbee_ezsp_zll_set_rx_on_when_idle
#define sl_zigbee_zll_scanning_complete sl_zigbee_ezsp_zll_scanning_complete

// Mirror SoC side macro in zll-api.h
#define sl_zigbee_zll_cancel_rx_on_when_idle() sl_zigbee_ezsp_zll_set_rx_on_when_idle(0)

//Other
#define sl_zigbee_get_pan_id sl_zigbee_af_get_pan_id
#define sl_zigbee_get_stack_profile sl_zigbee_af_get_stack_profile
#define sl_zigbee_get_security_level sl_zigbee_af_get_security_level
#define sl_zigbee_get_route_table_size sl_zigbee_af_get_route_table_size

#define sl_zigbee_get_max_mac_retries sl_zigbee_ezsp_get_max_mac_retries
#define sl_zigbee_set_pending_network_update_pan_id sl_zigbee_ezsp_set_pending_network_update_pan_id
#define sl_zigbee_set_pending_network_update_channel sl_zigbee_ezsp_set_pending_network_update_channel

#define sl_zigbee_set_stack_profile(stackProfile) sl_zigbee_ezsp_set_stack_profile(stackProfile)
#define sl_zigbee_set_security_level(securityLevel) sl_zigbee_ezsp_set_security_level(securityLevel)

//Have GP server use its own implementations of these for now
#ifndef SL_CATALOG_ZIGBEE_GREEN_POWER_SERVER_PRESENT
#define sl_zigbee_get_endpoint sl_zigbee_ezsp_get_endpoint
#define sl_zigbee_get_endpoint_count sl_zigbee_ezsp_get_endpoint_count
#endif

#define sl_zigbee_get_endpoint_description sl_zigbee_ezsp_get_endpoint_description
#define sl_zigbee_get_endpoint_cluster sl_zigbee_ezsp_get_endpoint_cluster

#define sl_zigbee_radio_get_scheduler_priorities sl_zigbee_ezsp_radio_get_scheduler_priorities
#define sl_zigbee_radio_get_scheduler_sliptime sl_zigbee_ezsp_radio_get_scheduler_sliptime
#define sl_zigbee_radio_set_scheduler_sliptime sl_zigbee_ezsp_radio_set_scheduler_sliptime
#define sl_zigbee_counter_requires_phy_index sl_zigbee_ezsp_counter_requires_phy_index
#define sl_zigbee_counter_requires_destination_node_id sl_zigbee_ezsp_counter_requires_destination_node_id

#define sl_zigbee_get_memory_usage_data sl_zigbee_ezsp_get_memory_usage_data

// Dynamic Hardware Configuration (DHC)
#define sl_zigbee_dhc_read_pa_metadata          sl_zigbee_ezsp_read_pa_metadata
#define sl_zigbee_dhc_write_pa_metadata         sl_zigbee_ezsp_write_pa_metadata
#define sl_zigbee_dhc_read_pa_descriptor        sl_zigbee_ezsp_read_pa_descriptor
#define sl_zigbee_dhc_write_pa_descriptor       sl_zigbee_ezsp_write_pa_descriptor
#define sl_zigbee_dhc_read_pa_curve             sl_zigbee_ezsp_read_pa_curve
#define sl_zigbee_dhc_write_pa_curve            sl_zigbee_ezsp_write_pa_curve
#define sl_zigbee_dhc_read_pa_curve_segment     sl_zigbee_ezsp_read_pa_curve_segment
#define sl_zigbee_dhc_write_pa_curve_segment    sl_zigbee_ezsp_write_pa_curve_segment
#define sl_zigbee_dhc_read_pa_table             sl_zigbee_ezsp_read_pa_table
#define sl_zigbee_dhc_write_pa_table            sl_zigbee_ezsp_write_pa_table
#define sl_zigbee_dhc_read_rssi_offset          sl_zigbee_ezsp_read_rssi_offset
#define sl_zigbee_dhc_write_rssi_offset         sl_zigbee_ezsp_write_rssi_offset
#define sl_zigbee_dhc_read_pa_voltage           sl_zigbee_ezsp_read_pa_voltage
#define sl_zigbee_dhc_write_pa_voltage          sl_zigbee_ezsp_write_pa_voltage
#define sl_zigbee_dhc_read_pa_mode              sl_zigbee_ezsp_read_pa_mode
#define sl_zigbee_dhc_write_pa_mode             sl_zigbee_ezsp_write_pa_mode
#define sl_zigbee_dhc_read_ctune                sl_zigbee_ezsp_read_ctune
#define sl_zigbee_dhc_write_ctune               sl_zigbee_ezsp_write_ctune
#define sl_zigbee_dhc_read_dhc_version          sl_zigbee_ezsp_read_dhc_version
#define sl_zigbee_dhc_write_dhc_version         sl_zigbee_ezsp_write_dhc_version
#define sl_zigbee_dhc_read_pa_version           sl_zigbee_ezsp_read_pa_version
#define sl_zigbee_dhc_read_pa_signature         sl_zigbee_ezsp_read_pa_signature
#define sl_zigbee_dhc_write_pa_signature        sl_zigbee_ezsp_write_pa_signature
