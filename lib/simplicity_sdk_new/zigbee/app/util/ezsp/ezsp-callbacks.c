/***************************************************************************//**
 * @file
 * @brief Convenience stubs for little-used EZSP callbacks.
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

#include PLATFORM_HEADER
#include "stack/include/sl_zigbee_types.h"

// *****************************************
// Convenience Stubs
// *****************************************

#ifndef EZSP_APPLICATION_HAS_WAITING_FOR_RESPONSE
void sl_zigbee_ezsp_waiting_for_response(void)
{
}
#endif

#ifndef EZSP_APPLICATION_HAS_NO_CALLBACKS
void sl_zigbee_ezsp_no_callbacks(void)
{
}
#endif

#ifndef EZSP_APPLICATION_HAS_TIMER_HANDLER
WEAK(void sl_zigbee_ezsp_timer_handler(uint8_t timerId))
{
  UNUSED_VAR(timerId);
}
#endif

#ifndef EZSP_APPLICATION_HAS_DEBUG_HANDLER
WEAK(void ezspDebugHandler(uint8_t messageLength,
                           uint8_t *messageContents))
{
  UNUSED_VAR(messageLength);
  UNUSED_VAR(messageContents);
}
#endif

#ifndef EZSP_APPLICATION_HAS_ZIGBEE_KEY_ESTABLISHMENT_HANDLER
void sl_zigbee_ezsp_zigbee_key_establishment_handler(sl_802154_long_addr_t partner, sl_zigbee_key_status_t status)
{
  UNUSED_VAR(partner);
  UNUSED_VAR(status);
}
#endif

#ifndef EZSP_APPLICATION_HAS_REMOTE_BINDING_HANDLER
WEAK(void sl_zigbee_ezsp_remote_set_binding_handler(sl_zigbee_binding_table_entry_t *entry,
                                                    uint8_t index,
                                                    sl_status_t policyDecision))
{
  UNUSED_VAR(entry);
  UNUSED_VAR(index);
  UNUSED_VAR(policyDecision);
}

WEAK(void sl_zigbee_ezsp_remote_delete_binding_handler(uint8_t index,
                                                       sl_status_t policyDecision))
{
  UNUSED_VAR(index);
  UNUSED_VAR(policyDecision);
}
#endif

WEAK(void sl_zigbee_ezsp_poll_complete_handler(sl_status_t status))
{
  UNUSED_VAR(status);
}

#ifndef EZSP_APPLICATION_HAS_POLL_HANDLER
WEAK(void sl_zigbee_ezsp_poll_handler(sl_802154_short_addr_t childId, bool transmitExpected))
{
  UNUSED_VAR(childId);
  UNUSED_VAR(transmitExpected);
}
#endif

#ifndef EZSP_APPLICATION_HAS_ENERGY_SCAN_RESULT_HANDLER
void sl_zigbee_ezsp_energy_scan_result_handler(uint8_t channel, int8_t maxRssiValue)
{
  UNUSED_VAR(channel);
  UNUSED_VAR(maxRssiValue);
}
#endif

#ifndef EZSP_APPLICATION_HAS_BUTTON_HANDLER
void halButtonIsr(uint8_t button, uint8_t state)
{
  UNUSED_VAR(button);
  UNUSED_VAR(state);
}
#endif

#ifndef EZSP_APPLICATION_HAS_ID_CONFLICT_HANDLER
WEAK(void sl_zigbee_ezsp_id_conflict_handler(sl_802154_short_addr_t id))
{
  UNUSED_VAR(id);
}
#endif

#ifndef EZSP_APPLICATION_HAS_INCOMING_MANY_TO_ONE_ROUTE_REQUEST_HANDLER
WEAK(void sl_zigbee_ezsp_incoming_many_to_one_route_request_handler(sl_802154_short_addr_t source,
                                                                    sl_802154_long_addr_t longId,
                                                                    uint8_t cost))
{
  UNUSED_VAR(source);
  UNUSED_VAR(longId);
  UNUSED_VAR(cost);
}
#endif

WEAK(void sl_zigbee_ezsp_incoming_route_error_handler(sl_status_t status, sl_802154_short_addr_t target))
{
  UNUSED_VAR(status);
  UNUSED_VAR(target);
}

#ifndef EZSP_APPLICATION_HAS_INCOMING_NETWORK_STATUS_HANDLER

WEAK(void sl_zigbee_ezsp_incoming_network_status_handler(uint8_t errorCode, sl_802154_short_addr_t target))
{
  UNUSED_VAR(errorCode);
  UNUSED_VAR(target);
}

#endif
#ifndef EZSP_APPLICATION_HAS_ROUTE_RECORD_HANDLER

WEAK(void sl_zigbee_ezsp_incoming_route_record_handler(sl_802154_short_addr_t source,
                                                       sl_802154_long_addr_t sourceEui,
                                                       uint8_t lastHopLqi,
                                                       int8_t lastHopRssi,
                                                       uint8_t relayCount,
                                                       uint8_t *relayList))
{
  UNUSED_VAR(source);
  UNUSED_VAR(sourceEui);
  UNUSED_VAR(lastHopLqi);
  UNUSED_VAR(lastHopRssi);
  UNUSED_VAR(relayCount);
  UNUSED_VAR(relayList);
}
#endif
#ifndef EZSP_APPLICATION_HAS_BOOTLOADER_HANDLER
WEAK(void sl_zigbee_ezsp_incoming_bootload_message_handler(sl_802154_long_addr_t longId,
                                                           uint8_t lastHopLqi,
                                                           int8_t lastHopRssi,
                                                           uint8_t messageLength,
                                                           uint8_t *messageContents))
{
  UNUSED_VAR(longId);
  UNUSED_VAR(lastHopLqi);
  UNUSED_VAR(lastHopRssi);
  UNUSED_VAR(messageLength);
  UNUSED_VAR(messageContents);
}

WEAK(void sl_zigbee_ezsp_bootload_transmit_complete_handler(sl_status_t status,
                                                            uint8_t messageLength,
                                                            uint8_t *messageContents))
{
  UNUSED_VAR(status);
  UNUSED_VAR(messageLength);
  UNUSED_VAR(messageContents);
}
#endif

#ifndef EZSP_APPLICATION_HAS_MAC_PASSTHROUGH_HANDLER
WEAK(void sl_zigbee_ezsp_mac_passthrough_message_handler(uint8_t messageType,
                                                         uint8_t lastHopLqi,
                                                         int8_t lastHopRssi,
                                                         uint8_t messageLength,
                                                         uint8_t *messageContents))
{
  UNUSED_VAR(messageType);
  UNUSED_VAR(lastHopLqi);
  UNUSED_VAR(lastHopRssi);
  UNUSED_VAR(messageLength);
  UNUSED_VAR(messageContents);
}
#endif

WEAK(void sl_zigbee_ezsp_mac_filter_match_message_handler(sl_zigbee_mac_filter_match_data_t filterValueMatch,
                                                          uint8_t legacyPassthroughType,
                                                          uint8_t lastHopLqi,
                                                          int8_t lastHopRssi,
                                                          uint8_t messageLength,
                                                          uint8_t *messageContents))
{
  UNUSED_VAR(filterValueMatch);
  sl_zigbee_ezsp_mac_passthrough_message_handler(legacyPassthroughType,
                                                 lastHopLqi,
                                                 lastHopRssi,
                                                 messageLength,
                                                 messageContents);
}

#ifndef EZSP_APPLICATION_HAS_MFGLIB_HANDLER
void sl_zigbee_ezsp_mfglib_rx_handler(
  uint8_t linkQuality,
  int8_t rssi,
  uint8_t packetLength,
  uint8_t *packetContents)
{
  UNUSED_VAR(linkQuality);
  UNUSED_VAR(rssi);
  UNUSED_VAR(packetLength);
  UNUSED_VAR(packetContents);
}
#endif

#ifndef EZSP_APPLICATION_HAS_RAW_HANDLER
WEAK(void sl_zigbee_ezsp_raw_transmit_complete_handler(sl_status_t status))
{
  UNUSED_VAR(status);
}
#endif

// Certificate Based Key Exchange (CBKE)
WEAK(void sl_zigbee_ezsp_generate_cbke_keys_handler(sl_status_t status,
                                                    sl_zigbee_public_key_data_t* ephemeralPublicKey))
{
  UNUSED_VAR(status);
  UNUSED_VAR(ephemeralPublicKey);
}

WEAK(void sl_zigbee_ezsp_calculate_smacs_handler(sl_status_t status,
                                                 sl_zigbee_smac_data_t* initiatorSmac,
                                                 sl_zigbee_smac_data_t* responderSmac))
{
  UNUSED_VAR(status);
  UNUSED_VAR(initiatorSmac);
  UNUSED_VAR(responderSmac);
}

WEAK(void sl_zigbee_ezsp_generate_cbke_keys_283k1_handler(sl_status_t status,
                                                          sl_zigbee_public_key_283k1_data_t* ephemeralPublicKey))
{
  UNUSED_VAR(status);
  UNUSED_VAR(ephemeralPublicKey);
}

WEAK(void sl_zigbee_ezsp_calculate_smacs_283k1_handler(sl_status_t status,
                                                       sl_zigbee_smac_data_t* initiatorSmac,
                                                       sl_zigbee_smac_data_t* responderSmac))
{
  UNUSED_VAR(status);
  UNUSED_VAR(initiatorSmac);
  UNUSED_VAR(responderSmac);
}
// Elliptical Cryptography Digital Signature Verification
#ifndef EZSP_APPLICATION_HAS_DSA_VERIFY_HANDLER
WEAK(void sl_zigbee_ezsp_dsa_verify_handler(sl_status_t status))
{
  UNUSED_VAR(status);
}
#endif

#ifndef EZSP_APPLICATION_HAS_FRAGMENT_SOURCE_ROUTE_HANDLER
sl_status_t ezspFragmentSourceRouteHandler(void)
{
  return SL_STATUS_OK;
}
#endif

WEAK(void sl_zigbee_ezsp_custom_frame_handler(uint8_t payloadLength, uint8_t* payload))
{
  UNUSED_VAR(payloadLength);
  UNUSED_VAR(payload);
}

#ifndef EZSP_APPLICATION_HAS_STACK_TOKEN_CHANGED_HANDLER
WEAK(void sl_zigbee_ezsp_stack_token_changed_handler(uint16_t tokenAddress))
{
  UNUSED_VAR(tokenAddress);
}
#endif

// This call is fired when a ZLL network scan finds a ZLL network.
WEAK(void sl_zigbee_ezsp_zll_network_found_handler(
       // Information about the network.
       sl_zigbee_zll_network_t *networkInfo,
       // Used to interpret deviceInfo field.
       bool isDeviceInfoNull,
       // Device specific information.
       sl_zigbee_zll_device_info_record_t *deviceInfo,
       // The link quality from the node that last relayed the message.
       uint8_t lastHopLqi,
       // The energy level (in units of dBm) observed during reception.
       int8_t lastHopRssi))
{
  UNUSED_VAR(networkInfo);
  UNUSED_VAR(isDeviceInfoNull);
  UNUSED_VAR(deviceInfo);
  UNUSED_VAR(lastHopLqi);
  UNUSED_VAR(lastHopRssi);
}

WEAK(void sl_zigbee_ezsp_zll_scan_complete_handler(sl_status_t status))
{
  UNUSED_VAR(status);
}

WEAK(void sl_zigbee_ezsp_mux_invalid_rx_handler(uint8_t new_rx_channel, uint8_t old_rx_channel))
{
  UNUSED_VAR(new_rx_channel);
  UNUSED_VAR(old_rx_channel);
}

WEAK(void sl_zigbee_ezsp_zll_address_assignment_handler(
       // Address assignment information.
       sl_zigbee_zll_address_assignment_t *addressInfo,
       // The link quality from the node that last relayed the message.
       uint8_t lastHopLqi,
       // The energy level (in units of dBm) observed during reception.
       int8_t lastHopRssi))
{
  UNUSED_VAR(addressInfo);
  UNUSED_VAR(lastHopLqi);
  UNUSED_VAR(lastHopRssi);
}

WEAK(void sl_zigbee_ezsp_zll_touch_link_target_handler(sl_zigbee_zll_network_t* networkInfo))
{
  UNUSED_VAR(networkInfo);
}

WEAK(void sl_zigbee_ezsp_counter_rollover_handler(sl_zigbee_counter_type_t type))
{
  UNUSED_VAR(type);
}

WEAK(void sl_zigbee_ezsp_gpep_incoming_message_handler(
       // GP parameters list represented as a macro for GP endpoint incoming message handler and callbacks prototypes.
       sl_zigbee_gp_params_t *params))
{
  UNUSED_VAR(params);
}

WEAK(void sl_zigbee_ezsp_d_gp_sent_handler(sl_status_t status, uint8_t gpepHandle))
{
  UNUSED_VAR(status);
  UNUSED_VAR(gpepHandle);
}

WEAK(void sl_zigbee_ezsp_dsa_sign_handler(sl_status_t status,
                                          uint8_t messageLength,
                                          uint8_t* messageContents))
{
  UNUSED_VAR(status);
  UNUSED_VAR(messageLength);
  UNUSED_VAR(messageContents);
}
WEAK(void sl_zigbee_ezsp_child_join_handler(uint8_t index,
                                            bool joining,
                                            sl_802154_short_addr_t childId,
                                            sl_802154_long_addr_t childEui64,
                                            sl_zigbee_node_type_t childType))
{
  UNUSED_VAR(index);
  UNUSED_VAR(joining);
  UNUSED_VAR(childId);
  UNUSED_VAR(childEui64);
  UNUSED_VAR(childType);
}

WEAK(void sl_zigbee_ezsp_duty_cycle_handler(uint8_t channelPage,
                                            uint8_t channel,
                                            sl_zigbee_duty_cycle_state_t state,
                                            uint8_t totalDevices,
                                            sl_zigbee_per_device_duty_cycle_t *arrayOfDeviceDutyCycles))
{
  UNUSED_VAR(channelPage);
  UNUSED_VAR(channel);
  UNUSED_VAR(state);
  UNUSED_VAR(totalDevices);
  UNUSED_VAR(arrayOfDeviceDutyCycles);
}

WEAK(void sl_zigbee_ezsp_switch_network_key_handler(uint8_t sequenceNumber))
{
  UNUSED_VAR(sequenceNumber);
}

WEAK(void sl_zigbee_ezsp_trust_center_post_join_handler(sl_802154_short_addr_t newNodeId,
                                                        sl_802154_long_addr_t newNodeEui64,
                                                        sl_zigbee_device_update_t status,
                                                        sl_zigbee_join_decision_t policyDecision,
                                                        sl_802154_short_addr_t parentOfNewNode))
{
  UNUSED_VAR(newNodeId);
  UNUSED_VAR(newNodeEui64);
  UNUSED_VAR(status);
  UNUSED_VAR(policyDecision);
  UNUSED_VAR(parentOfNewNode);
}

WEAK(void sl_zigbee_ezsp_incoming_mfg_test_message_handler(uint8_t messageType,
                                                           uint8_t dataLength,
                                                           uint8_t *data))
{
  UNUSED_VAR(messageType);
  UNUSED_VAR(dataLength);
  UNUSED_VAR(data);
}
