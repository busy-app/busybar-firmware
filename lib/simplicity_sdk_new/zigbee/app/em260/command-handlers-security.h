/***************************************************************************//**
 * @file
 * @brief Prototypes for ZigBee PRO Security command handler functions.
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

sl_status_t sl_zigbee_xncp_security_handle_key_command_cb(uint8_t index,
                                                          sl_zigbee_key_struct_t* keyStruct);

sl_status_t sl_zigbee_af_ezsp_aes_mmo_hash_command_cb(sl_zigbee_aes_mmo_hash_context_t* context,
                                                      bool finalize,
                                                      uint8_t length,
                                                      uint8_t* data,
                                                      sl_zigbee_aes_mmo_hash_context_t* returnContext);

sl_status_t sl_zigbee_xncp_security_handle_key_command_cb(uint8_t index,
                                                          sl_zigbee_key_struct_t* keyStruct);

sl_status_t sl_zigbee_af_ezsp_remove_device_command_cb(sl_802154_short_addr_t destShort,
                                                       sl_802154_long_addr_t destLong,
                                                       sl_802154_long_addr_t targetLong);

sl_status_t sl_zigbee_af_ezsp_unicast_nwk_key_update_command_cb(sl_802154_short_addr_t destShort,
                                                                sl_802154_long_addr_t destLong,
                                                                sl_zigbee_key_data_t* key);

sl_status_t sl_zigbee_af_ezsp_update_link_key_command_cb(void);

void sl_zigbee_xncp_security_get_value_command_cb(sl_zigbee_af_plugin_ezsp_value_command_context_t* context);
void sl_zigbee_xncp_security_set_value_command_cb(sl_zigbee_af_plugin_ezsp_value_command_context_t *context);
void sl_zigbee_xncp_security_set_value_command_cb(sl_zigbee_af_plugin_ezsp_value_command_context_t* context);
void sl_zigbee_xncp_security_get_configuration_value_command_cb(sl_zigbee_af_plugin_ezsp_configuration_value_command_context_t* context);
void sl_zigbee_xncp_security_set_configuration_value_command_cb(sl_zigbee_af_plugin_ezsp_configuration_value_command_context_t* context);
void sl_zigbee_xncp_security_policy_command_cb(sl_zigbee_af_plugin_ezsp_policy_command_context_t* context);

sl_zigbee_join_decision_t sl_zigbee_xncp_security_trust_center_join_cb(sl_802154_short_addr_t newNodeId,
                                                                       const sl_802154_long_addr_t newNodeEui64,
                                                                       sl_zigbee_device_update_t status,
                                                                       sl_802154_short_addr_t parentOfNewNode,
                                                                       sl_zigbee_ezsp_decision_id_t decisionId,
                                                                       sl_zigbee_join_decision_t joinDecision);
sl_status_t sl_zigbee_af_ezsp_aps_crypt_message_command_cb(bool encrypt,
                                                           uint8_t* length,
                                                           uint8_t* message,
                                                           uint8_t apsHeaderEndIndex,
                                                           uint8_t* remoteEui64);
