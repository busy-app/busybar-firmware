/***************************************************************************//**
 * @file
 * @brief Prototypes for ZigBee PRO command handler functions.
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

//------------------------------------------------------------------------------
// Ezsp Command Handlers

sl_status_t sl_zigbee_af_ezsp_find_and_rejoin_network_command_cb(bool haveCurrentNetworkKey,
                                                                 uint32_t channelMask,
                                                                 sl_zigbee_rejoin_reason_t hostRejoinReason,
                                                                 sl_zigbee_node_type_t nodeType);

sl_status_t sl_zigbee_af_ezsp_unicast_current_network_key_command_cb(sl_802154_short_addr_t taretShort,
                                                                     sl_802154_long_addr_t targetLong,
                                                                     sl_802154_short_addr_t parentShortId);

uint32_t sl_zigbee_af_ezsp_set_source_route_discovery_mode_command_cb(uint8_t mode);

void sl_zigbee_af_ezsp_get_eui64_command_cb(sl_802154_long_addr_t eui64);

sl_status_t sl_zigbee_af_ezsp_get_network_parameters_command_cb(sl_zigbee_node_type_t* nodeType,
                                                                sl_zigbee_network_parameters_t* parameters);

uint8_t sl_zigbee_af_ezsp_get_parent_child_parameters_command_cb(sl_802154_long_addr_t parentEui64,
                                                                 sl_802154_short_addr_t* parentNodeId);

sl_status_t sl_zigbee_af_ezsp_send_unicast_command_cb(sl_zigbee_outgoing_message_type_t type,
                                                      sl_802154_short_addr_t indexOrDestination,
                                                      sl_zigbee_aps_frame_t* apsFrame,
                                                      uint8_t messageTag,
                                                      uint8_t messageLength,
                                                      uint8_t messageContents[],
                                                      uint8_t* sequence);

sl_status_t sl_zigbee_af_ezsp_send_broadcast_command_cb(sl_802154_short_addr_t alias,
                                                        sl_802154_short_addr_t destination,
                                                        uint8_t nwkSeq,
                                                        sl_zigbee_aps_frame_t* apsFrame,
                                                        uint8_t radius,
                                                        uint8_t messageTag,
                                                        uint8_t messageLength,
                                                        uint8_t messageContents[],
                                                        uint8_t* sequence);

sl_status_t sl_zigbee_af_ezsp_send_multicast_command_cb(sl_zigbee_aps_frame_t* apsFrame,
                                                        uint8_t hops,
                                                        uint16_t broadcastAddr,
                                                        sl_802154_short_addr_t alias,
                                                        uint8_t seq,
                                                        uint8_t messageTag,
                                                        uint8_t messageLength,
                                                        uint8_t messageContents[],
                                                        uint8_t* sequence);

sl_status_t sl_zigbee_af_ezsp_send_reply_command_cb(sl_802154_short_addr_t sender,
                                                    sl_zigbee_aps_frame_t* apsFrame,
                                                    uint8_t messageLength,
                                                    uint8_t messageContents[]);

sl_status_t sl_zigbee_af_ezsp_poll_for_data_command_cb(uint16_t interval,
                                                       sl_zigbee_event_units_t units,
                                                       uint8_t failureLimit);

sl_status_t sl_zigbee_af_ezsp_set_routing_shortcut_threshold_command_cb(uint8_t costThresh);
uint8_t sl_zigbee_af_ezsp_get_routing_shortcut_threshold_command_cb(void);

sl_status_t sl_zigbee_af_ezsp_get_source_route_table_entry_command_cb(uint8_t index,
                                                                      sl_802154_short_addr_t *destination,
                                                                      uint8_t *closerIndex);

uint8_t sl_zigbee_af_ezsp_get_source_route_table_total_size_command_cb(void);
uint8_t sl_zigbee_af_ezsp_get_source_route_table_filled_size_command_cb(void);

sl_status_t sl_zigbee_af_ezsp_replace_address_table_entry_command_cb(uint8_t addressTableIndex,
                                                                     sl_802154_long_addr_t newEui64,
                                                                     sl_802154_short_addr_t newId,
                                                                     bool newExtendedTimeout,
                                                                     sl_802154_long_addr_t oldEui64,
                                                                     sl_802154_short_addr_t* oldId,
                                                                     bool* oldExtendedTimeout);

sl_status_t sl_zigbee_af_ezsp_get_multicast_table_entry_command_cb(uint8_t index,
                                                                   sl_zigbee_multicast_table_entry_t* value);

sl_status_t sl_zigbee_af_ezsp_set_multicast_table_entry_command_cb(uint8_t index,
                                                                   sl_zigbee_multicast_table_entry_t* value);

sl_status_t sl_zigbee_af_ezsp_send_raw_message_command_cb(uint8_t messageLength,
                                                          uint8_t messageContents[],
                                                          uint8_t priority,
                                                          bool useCca);

sl_status_t sl_zigbee_af_ezsp_set_concentrator_command_cb(bool on,
                                                          uint16_t concentratorType,
                                                          uint16_t minTime,
                                                          uint16_t maxTime,
                                                          uint8_t routeErrorThreshold,
                                                          uint8_t deliveryFailureThreshold,
                                                          uint8_t maxHops);

sl_status_t sl_zigbee_af_ezsp_get_current_duty_cycle_command_cb(uint8_t maxDevices, uint8_t* perDeviceDutyCycles);

void sl_zigbee_xncp_zigbee_pro_get_value_command_cb(sl_zigbee_af_plugin_ezsp_value_command_context_t* context);
void sl_zigbee_xncp_zigbee_pro_get_extended_value_command_cb(sl_zigbee_af_plugin_ezsp_value_command_context_t* context);
void sl_zigbee_xncp_zigbee_pro_set_value_command_cb(sl_zigbee_af_plugin_ezsp_value_command_context_t* context);
void sl_zigbee_xncp_zigbee_pro_policy_command_cb(sl_zigbee_af_plugin_ezsp_policy_command_context_t* context);
void sl_zigbee_xncp_zigbee_pro_get_configuration_value_command_cb(sl_zigbee_af_plugin_ezsp_configuration_value_command_context_t* context);
void sl_zigbee_xncp_zigbee_pro_set_configuration_value_command_cb(sl_zigbee_af_plugin_ezsp_configuration_value_command_context_t* context);

sl_status_t sl_zigbee_af_ezsp_set_broken_route_error_code_command_cb(uint8_t errorCode);

//ezsp command processing picks the wrong name
uint8_t sli_zigbee_stack_maximum_aps_payload_length(void);
#define sli_zigbee_stack_maximum_payload_length sli_zigbee_stack_maximum_aps_payload_length
#define sli_zigbee_stack_unicast_nwk_key_update sli_zigbee_stack_send_unicast_network_key_update
#define sli_zigbee_stack_remove_device sli_zigbee_stack_send_remove_device
//--------------------------------------------------------------------------
// Utilities

void writeLqiAndRssi(uint8_t *loc);
void sl_zigbee_xncp_green_power_configuration_value_command_cb(sl_zigbee_af_plugin_ezsp_configuration_value_command_context_t* context);

sl_status_t sl_zigbee_af_ezsp_set_source_route_command_cb(sl_802154_short_addr_t destination,
                                                          uint8_t relayCount,
                                                          uint16_t* relayList);
