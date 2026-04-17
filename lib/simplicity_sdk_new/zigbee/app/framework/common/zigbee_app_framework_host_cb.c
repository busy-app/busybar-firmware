/*****************************************************************************/
/**
 * Copyright 2020 Silicon Laboratories, Inc.
 *
 *****************************************************************************/
//
// EMZIGBEE-7387: modify "gen_stack_callback_dispatchers.py" to generate this file
// as well as zigbee_stack_callback_dispatcher jinja templates to support UC host
// *** Generated file. Do not edit! ***
//

#include PLATFORM_HEADER
#include "zigbee_stack_callback_dispatcher.h"
#include "zigbee_app_framework_callback.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "app/framework/include/af.h"
#else
#define sl_zigbee_af_push_callback_network_index()
#define sl_zigbee_af_pop_network_index()
#endif

#if defined(EMBER_AF_PRINT_SECURITY)  || defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT)
// Corresponds to the sl_zigbee_join_decision_t status codes
static const char * joinDecisionText[] = {
  SL_ZIGBEE_JOIN_DECISION_STRINGS
};

// Corresponds to the sl_zigbee_device_update_t status codes
static const char * deviceUpdateText[] = {
  SL_ZIGBEE_DEVICE_UPDATE_STRINGS
};

#endif // SL_ZIGBEE_AF_PRINT_APP

// -----------------------------------------------------------------------------
// A callback invoked when receiving a message.
void sl_zigbee_incoming_message_handler(
  // Incoming message type
  sl_zigbee_incoming_message_type_t type,
  // The APS frame from the incoming message.
  sl_zigbee_aps_frame_t *apsFrame,
  // Packet information.
  sl_zigbee_rx_packet_info_t *packetInfo,
  // The length of the message in bytes.
  uint8_t messageLength,
  // The incoming message.
  uint8_t *message)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zb_af_incoming_message(type, apsFrame, packetInfo, messageLength, message);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// A callback invoked when a messageSend has been tried by the stack.

WEAK(void sli_zigbee_af_message_sent_callback(sl_status_t status,
                                              sl_zigbee_outgoing_message_type_t type,
                                              uint16_t indexOrDestination,
                                              sl_zigbee_aps_frame_t *apsFrame,
                                              uint16_t messageTag,
                                              uint8_t messageLength,
                                              uint8_t *messageContents))
{
  sli_zigbee_af_message_sent(status,
                             type,
                             indexOrDestination,
                             apsFrame,
                             messageTag,
                             messageLength,
                             messageContents);
}

void sl_zigbee_message_sent_handler(
  // An sl_status_t value of SL_STATUS_OK if an ACK was received from the
  // destination or SL_STATUS_ZIGBEE_DELIVERY_FAILED if no ACK was received.
  sl_status_t status,
  // The type of message sent.
  sl_zigbee_outgoing_message_type_t type,
  // The destination to which the message was sent, for direct unicasts, or
  // the address table or binding index for other unicasts. The value is
  // unspecified for multicasts and broadcasts.
  uint16_t indexOrDestination,
  // The APS frame for the message.
  sl_zigbee_aps_frame_t *apsFrame,
  // The value supplied by the Host in the sl_zigbee_send_unicast,
  // sl_zigbee_send_broadcast or sl_zigbee_send_multicast command.
  uint16_t messageTag,
  // The length of the messageContents parameter in bytes.
  uint8_t messageLength,
  // The unicast message supplied by the Host. The message contents are only
  // included here if the decision for the messageContentsInCallback policy
  // is messageTagAndContentsInCallback.
  uint8_t *messageContents)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_message_sent_callback(status, type, indexOrDestination, apsFrame, messageTag, messageLength, messageContents);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_zigbee_key_establishment_cb
WEAK(void sl_zigbee_af_zigbee_key_establishment_cb(
       // This is the IEEE address of the partner that the device successfully
       // established a key with. This value is all zeros on a failure.
       sl_802154_long_addr_t partner,
       // This is the status indicating what was established or why the key
       // establishment failed.
       sl_zigbee_key_status_t status))
{
  (void)partner;
  (void)status;
}

// This is a callback that indicates the success or failure of an attempt to
// establish a key with a partner device.
void sl_zigbee_ezsp_zigbee_key_establishment_handler(
  // This is the IEEE address of the partner that the device successfully
  // established a key with. This value is all zeros on a failure.
  sl_802154_long_addr_t partner,
  // This is the status indicating what was established or why the key
  // establishment failed.
  sl_zigbee_key_status_t status)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_zigbee_key_establishment(partner, status);
  sl_zigbee_af_zigbee_key_establishment_cb(partner, status);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_gpep_incoming_message_cb
WEAK(void sl_zigbee_af_gpep_incoming_message_cb(
       // GP parameters list represented as a macro for GP endpoint incoming message handler and callbacks prototypes.
       sl_zigbee_gp_params_t *params))
{
  (void)params;
}

// A callback invoked by the ZigBee GP stack when a GPDF is received.
void sl_zigbee_ezsp_gpep_incoming_message_handler(
  // GP parameters list represented as a macro for GP endpoint incoming message handler and callbacks prototypes.
  sl_zigbee_gp_params_t *params)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_gpep_incoming_message(params);
  sl_zigbee_af_gpep_incoming_message_cb(params);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_d_gp_sent_cb
WEAK(void sl_zigbee_af_d_gp_sent_cb(
       // An sl_status_t value indicating success or the reason for failure.
       sl_status_t status,
       // The handle of the GPDF.
       uint8_t gpepHandle))
{
  (void)status;
  (void)gpepHandle;
}

// A callback to the GP endpoint to indicate the result of the GPDF
// transmission.
void sl_zigbee_ezsp_d_gp_sent_handler(
  // An sl_status_t value indicating success or the reason for failure.
  sl_status_t status,
  // The handle of the GPDF.
  uint8_t gpepHandle)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_d_gp_sent(status, gpepHandle);
  sl_zigbee_af_d_gp_sent_cb(status, gpepHandle);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_stack_status_cb
WEAK(void sl_zigbee_af_stack_status_cb(
       // Stack status. One of the following: SL_STATUS_NETWORK_UP,
       // SL_STATUS_NETWORK_DOWN, SL_STATUS_NOT_JOINED, SL_STATUS_ZIGBEE_MOVE_FAILED
       sl_status_t status))
{
  (void)status;
}

extern void sli_zigbee_af_clear_network_cache(uint8_t network_index);
// A callback invoked when the status of the stack changes. If the status
// parameter equals SL_STATUS_NETWORK_UP, then the getNetworkParameters command can
// be called to obtain the new network parameters. If any of the parameters are
// being stored in nonvolatile memory by the Host, the stored values should be
// updated.
void sl_zigbee_ezsp_stack_status_handler(
  // Stack status. One of the following: SL_STATUS_NETWORK_UP,
  // SL_STATUS_NETWORK_DOWN, SL_STATUS_NOT_JOINED, SL_STATUS_ZIGBEE_MOVE_FAILED
  sl_status_t status)
{
  sl_zigbee_af_push_callback_network_index();
  // To be extra careful, we clear the network cache whenever a new status is
  // received.
  sli_zigbee_af_clear_network_cache(sl_zigbee_get_current_network());
  sli_zigbee_af_stack_status(status);
  sl_zigbee_af_stack_status_cb(status);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_energy_scan_result_cb
WEAK(void sl_zigbee_af_energy_scan_result_cb(
       // The 802.15.4 channel number that was scanned.
       uint8_t channel,
       // The maximum RSSI value found on the channel.
       int8_t maxRssiValue))
{
  (void)channel;
  (void)maxRssiValue;
}

// Reports the result of an energy scan for a single channel. The scan is not
// complete until the scanCompleteHandler callback is called.
void sl_zigbee_ezsp_energy_scan_result_handler(
  // The 802.15.4 channel number that was scanned.
  uint8_t channel,
  // The maximum RSSI value found on the channel.
  int8_t maxRssiValue)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_energy_scan_result(channel, maxRssiValue);
  sl_zigbee_af_energy_scan_result_cb(channel, maxRssiValue);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_network_found_cb
WEAK(void sl_zigbee_af_network_found_cb(
       // The parameters associated with the network found.
       sl_zigbee_zigbee_network_t *networkFound,
       // The link quality indication of the network found.
       uint8_t lqi,
       // The received signal strength indication of the network found.
       int8_t rssi))
{
  (void)networkFound;
  (void)lqi;
  (void)rssi;
}

// Reports that a network was found as a result of a prior call to startScan.
// Gives the network parameters useful for deciding which network to join.
void sl_zigbee_ezsp_network_found_handler(
  // The parameters associated with the network found.
  sl_zigbee_zigbee_network_t *networkFound,
  // The link quality indication of the network found.
  uint8_t lqi,
  // The received signal strength indication of the network found.
  int8_t rssi)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_network_found(networkFound, lqi, rssi);
  sl_zigbee_af_network_found_cb(networkFound, lqi, rssi);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_scan_complete_cb
WEAK(void sl_zigbee_af_scan_complete_cb(
       // The channel on which the current error occurred. Undefined for the case
       // of SL_STATUS_OK.
       uint8_t channel,
       // The error condition that occurred on the current channel. Value will be
       // SL_STATUS_OK when the scan has completed.
       sl_status_t status))
{
  (void)channel;
  (void)status;
}

// Returns the status of the current scan of type SL_ZIGBEE_ENERGY_SCAN or
// SL_ZIGBEE_ACTIVE_SCAN. SL_STATUS_OK signals that the scan has completed. Other
// error conditions signify a failure to scan on the channel specified.
void sl_zigbee_ezsp_scan_complete_handler(
  // The channel on which the current error occurred. Undefined for the case
  // of SL_STATUS_OK.
  uint8_t channel,
  // The error condition that occurred on the current channel. Value will be
  // SL_STATUS_OK when the scan has completed.
  sl_status_t status)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_scan_complete(channel, status);
  sl_zigbee_af_scan_complete_cb(channel, status);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_unused_pan_id_found_cb
WEAK(void sl_zigbee_af_unused_pan_id_found_cb(
       // The unused panID which has been found.
       sl_802154_pan_id_t panId,
       // The channel that the unused panID was found on.
       uint8_t channel))
{
  (void)panId;
  (void)channel;
}

// This function returns an unused panID and channel pair found via the find
// unused panId scan procedure.
void sl_zigbee_ezsp_unused_pan_id_found_handler(
  // The unused panID which has been found.
  sl_802154_pan_id_t panId,
  // The channel that the unused panID was found on.
  uint8_t channel)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_unused_pan_id_found(panId, channel);
  sl_zigbee_af_unused_pan_id_found_cb(panId, channel);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_scan_error_cb
WEAK(void sl_zigbee_af_scan_error_cb(
       // The error status of a scan
       sl_status_t status))
{
  (void)status;
}

// scanErrorHandler
void sl_zigbee_scan_error_handler(
  // The error status of a scan
  sl_status_t status)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_scan_error(status);
  sl_zigbee_af_scan_error_cb(status);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_switch_network_key_cb
WEAK(void sl_zigbee_af_switch_network_key_cb(
       // The sequence number of the new network key.
       uint8_t sequenceNumber))
{
  (void)sequenceNumber;
}

// A callback to inform the application that the Network Key has been updated
// and the node has been switched over to use the new key. The actual key being
// used is not passed up, but the sequence number is.
void sl_zigbee_ezsp_switch_network_key_handler(
  // The sequence number of the new network key.
  uint8_t sequenceNumber)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_switch_network_key(sequenceNumber);
  sl_zigbee_af_switch_network_key_cb(sequenceNumber);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_child_join_cb
WEAK(void sl_zigbee_af_child_join_cb(
       // The index of the child of interest.
       uint8_t index,
       // True if the child is joining. False the child is leaving.
       bool joining,
       sl_802154_short_addr_t childId,
       sl_802154_long_addr_t childEui64,
       sl_zigbee_node_type_t childType))
{
  (void)index;
  (void)joining;
  (void)childId;
  (void)childEui64;
  (void)childType;
}

// Indicates that a child has joined or left.
void sl_zigbee_ezsp_child_join_handler(
  // The index of the child of interest.
  uint8_t index,
  // True if the child is joining. False the child is leaving.
  bool joining,
  // The node ID of the child.
  sl_802154_short_addr_t childId,
  // The EUI64 of the child.
  sl_802154_long_addr_t childEui64,
  // The node type of the child.
  sl_zigbee_node_type_t childType)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_child_join(index, joining, childId, childEui64, childType);
  sl_zigbee_af_child_join_cb(index, joining, childId, childEui64, childType);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_duty_cycle_cb
WEAK(void sl_zigbee_af_duty_cycle_cb(
       // The channel page whose duty cycle state has changed.
       uint8_t channelPage,
       // The channel number whose duty cycle state has changed.
       uint8_t channel,
       // The current duty cycle state.
       sl_zigbee_duty_cycle_state_t state,
       uint8_t totalDevices,
       sl_zigbee_per_device_duty_cycle_t *arrayOfDeviceDutyCycles))
{
  (void)channelPage;
  (void)channel;
  (void)state;
  (void)totalDevices;
  (void)arrayOfDeviceDutyCycles;
}

// Callback fires when the duty cycle state has changed
void sl_zigbee_ezsp_duty_cycle_handler(
  // The channel page whose duty cycle state has changed.
  uint8_t channelPage,
  // The channel number whose duty cycle state has changed.
  uint8_t channel,
  // The current duty cycle state.
  sl_zigbee_duty_cycle_state_t state,
  // The total number of connected end devices that are being monitored for
  // duty cycle.
  uint8_t totalDevices,
  // Consumed duty cycles of end devices that are being monitored. The first
  // entry always be the local stack's nodeId, and thus the total aggregate
  // duty cycle for the device.
  sl_zigbee_per_device_duty_cycle_t *arrayOfDeviceDutyCycles)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_duty_cycle(channelPage, channel, state, totalDevices, arrayOfDeviceDutyCycles);
  sl_zigbee_af_duty_cycle_cb(channelPage, channel, state, totalDevices, arrayOfDeviceDutyCycles);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_poll_complete_cb
WEAK(void sl_zigbee_af_poll_complete_cb(
       // An sl_status_t value: SL_STATUS_OK - Data was received in response to
       // the poll. SL_STATUS_MAC_NO_DATA - No data was pending.
       // SL_STATUS_ZIGBEE_DELIVERY_FAILED - The poll message could not be sent.
       // SL_STATUS_MAC_NO_ACK_RECEIVED - The poll message was sent but not
       // acknowledged by the parent.
       sl_status_t status))
{
  (void)status;
}

// Indicates the result of a data poll to the parent of the local node.
void sl_zigbee_ezsp_poll_complete_handler(
  // An sl_status_t value: SL_STATUS_OK - Data was received in response to
  // the poll. SL_STATUS_MAC_NO_DATA - No data was pending.
  // SL_STATUS_ZIGBEE_DELIVERY_FAILED - The poll message could not be sent.
  // SL_STATUS_MAC_NO_ACK_RECEIVED - The poll message was sent but not
  // acknowledged by the parent.
  sl_status_t status)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_poll_complete(status);
  sl_zigbee_af_poll_complete_cb(status);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public callback sl_zigbee_af_dsa_sign_cb
// The handler that returns the results of the signing operation. On success,
// the signature will be appended to the original message (including the
// signature type indicator that replaced the startIndex field for the signing)
// and both are returned via this callback.
WEAK(void sl_zigbee_af_dsa_sign_cb(
       // The result of the DSA signing operation.
       sl_status_t status,
       // The length of message.
       uint8_t messageLength,
       // The message and attached which includes the original message and the
       // appended signature.
       uint8_t *messageContents))
{
  (void)status;
  (void)messageLength;
  (void)messageContents;
}

// Indicates the completion of a DSA signature operation.
void sl_zigbee_ezsp_dsa_sign_handler(
  //Status of DSA sign operation.
  sl_status_t status,
  // length of the message
  uint8_t messageLength,
  // message content
  uint8_t *message)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_dsa_sign(status, messageLength, message);
  sl_zigbee_af_dsa_sign_cb(status, messageLength, message);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------

void sl_zigbee_ezsp_trust_center_post_join_handler(sl_802154_short_addr_t newNodeId,
                                                   sl_802154_long_addr_t newNodeEui64,
                                                   sl_zigbee_device_update_t status,
                                                   sl_zigbee_join_decision_t policyDecision,
                                                   sl_802154_short_addr_t parentOfNewNode)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_trust_center_post_join(newNodeId,
                                       newNodeEui64,
                                       status,
                                       policyDecision,
                                       parentOfNewNode);
  // Allow users of the framework an opportunity to see this notification.
  sl_zigbee_af_trust_center_join_cb(newNodeId,
                                    newNodeEui64,
                                    parentOfNewNode,
                                    status,
                                    policyDecision);
  sl_zigbee_af_security_println("Trust Center Join Handler: status = %s, decision = %s (%02X), shortid 0x%04X",
                                deviceUpdateText[status],
                                joinDecisionText[policyDecision],
                                policyDecision,
                                newNodeId);
  sl_zigbee_af_security_flush();
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_zll_touch_link_target_cb
WEAK(void sl_zigbee_af_zll_touch_link_target_cb(
       // Return: Information about the network.
       const sl_zigbee_zll_network_t *networkInfo))
{
  (void)networkInfo;
}

// This call is fired when the device is a target of a touch link.
void sl_zigbee_ezsp_zll_touch_link_target_handler(
  // Return: Information about the network.
  sl_zigbee_zll_network_t *networkInfo)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_zll_touch_link_target(networkInfo);
  sl_zigbee_af_zll_touch_link_target_cb(networkInfo);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_stack_token_changed_cb
WEAK(void sl_zigbee_af_stack_token_changed_cb(
       // The address of the stack token that has changed.
       uint16_t tokenAddress))
{
  (void)tokenAddress;
}

void sl_zigbee_ezsp_stack_token_changed_handler(uint16_t tokenAddress)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_stack_token_changed(tokenAddress);
  sl_zigbee_af_stack_token_changed_cb(tokenAddress);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_dsa_verify_cb
WEAK(void sl_zigbee_af_dsa_verify_cb(
       // The result of the DSA verification operation.
       sl_status_t status))
{
  (void)status;
}

void sl_zigbee_ezsp_dsa_verify_handler(sl_status_t status)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_dsa_verify(status);
  sl_zigbee_af_dsa_verify_cb(status);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_incoming_network_status_cb
WEAK(void sl_zigbee_af_incoming_network_status_cb(
       // One byte over-the-air error code from network status message
       uint8_t errorCode,
       // The short ID of the remote node
       sl_802154_short_addr_t target))
{
  (void)errorCode;
  (void)target;
}

void sl_zigbee_ezsp_incoming_network_status_handler(uint8_t errorCode, sl_802154_short_addr_t target)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_incoming_network_status(errorCode, target);
  sl_zigbee_af_incoming_network_status_cb(errorCode, target);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_incoming_route_error_cb
WEAK(void sl_zigbee_af_incoming_route_error_cb(
       // SL_STATUS_ZIGBEE_SOURCE_ROUTE_FAILURE or SL_STATUS_ZIGBEE_MANY_TO_ONE_ROUTE_FAILURE.
       sl_status_t status,
       // The short id of the remote node.
       sl_802154_short_addr_t target))
{
  (void)status;
  (void)target;
}

void sl_zigbee_ezsp_incoming_route_error_handler(sl_status_t status, sl_802154_short_addr_t target)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_incoming_route_error(status, target);
  sl_zigbee_af_incoming_route_error_cb(status, target);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
//sl_zigbee_af_incoming_many_to_one_route_request_cb
WEAK(void sl_zigbee_af_incoming_many_to_one_route_request_cb(
       // The short id of the concentrator.
       sl_802154_short_addr_t source,
       // The EUI64 of the concentrator.
       sl_802154_long_addr_t longId,
       // The path cost to the concentrator. The cost may decrease as additional
       // route request packets for this discovery arrive, but the callback is
       // made only once.
       uint8_t cost))
{
  (void)source;
  (void)longId;
  (void)cost;
}

void sl_zigbee_ezsp_incoming_many_to_one_route_request_handler(sl_802154_short_addr_t source,
                                                               sl_802154_long_addr_t longId,
                                                               uint8_t cost)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_incoming_many_to_one_route_request(source, longId, cost);
  sl_zigbee_af_incoming_many_to_one_route_request_cb(source, longId, cost);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_id_conflict_cb
WEAK(void sl_zigbee_af_id_conflict_cb(
       // The short id for which a conflict was detected
       sl_802154_short_addr_t conflictingId))
{
  (void)conflictingId;
}

void sl_zigbee_ezsp_id_conflict_handler(sl_802154_short_addr_t id)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_id_conflict(id);
  sl_zigbee_af_id_conflict_cb(id);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_timer_cb
WEAK(void sl_zigbee_af_timer_cb(
       // Which timer generated the callback (0 or 1).
       uint8_t timerId))
{
  (void)timerId;
}

void sl_zigbee_ezsp_timer_handler(uint8_t timerId)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_timer(timerId);
  sl_zigbee_af_timer_cb(timerId);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_counter_rollover_cb
WEAK(void sl_zigbee_af_counter_rollover_cb(
       // Type of Counter
       sl_zigbee_counter_type_t type))
{
  (void)type;
}

void sl_zigbee_ezsp_counter_rollover_handler(sl_zigbee_counter_type_t type)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_counter_rollover(type);
  sl_zigbee_af_counter_rollover_cb(type);
  sl_zigbee_af_pop_network_index();
}
//sli_zigbee_af_mux_invalid_rx_callback

// -----------------------------------------------------------------------------
/* Weak implementation of public Callback sl_zigbee_af_mux_invalid_rx_cb */
WEAK(void sl_zigbee_af_mux_invalid_rx_cb(uint8_t new_rx_channel, uint8_t old_rx_channel))
{
  (void)new_rx_channel;
  (void)old_rx_channel;
}

void sl_zigbee_ezsp_mux_invalid_rx_handler(uint8_t new_rx_channel, uint8_t old_rx_channel)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_mux_invalid_rx(new_rx_channel, old_rx_channel);
  sl_zigbee_af_mux_invalid_rx_cb(new_rx_channel, old_rx_channel);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_zll_address_assignment_cb
WEAK(void sl_zigbee_af_zll_address_assignment_cb(
       // Return: Address assignment information.
       const sl_zigbee_zll_address_assignment_t *addressInfo,
       // RX packet info.
       sl_zigbee_rx_packet_info_t *packetInfo))
{
  (void)packetInfo;
  (void)addressInfo;
}

// This call is fired when network and group addresses are assigned to a remote
// mode in a network start or network join request.
void sl_zigbee_ezsp_zll_address_assignment_handler(
  // Address assignment information.
  sl_zigbee_zll_address_assignment_t *addressInfo,
  // RX packet info.
  sl_zigbee_rx_packet_info_t *packetInfo)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_zll_address_assignment(addressInfo, packetInfo);
  sl_zigbee_af_zll_address_assignment_cb(addressInfo, packetInfo);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_zll_scan_complete_cb
WEAK(void sl_zigbee_af_zll_scan_complete_cb(
       // Status of the operation.
       sl_status_t status))
{
  (void)status;
}

// This call is fired when a ZLL network scan is complete.
void sl_zigbee_ezsp_zll_scan_complete_handler(
  // Status of the operation.
  sl_status_t status)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_zll_scan_complete(status);
  sl_zigbee_af_zll_scan_complete_cb(status);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_zll_network_found_cb
WEAK(void sl_zigbee_af_zll_network_found_cb(
       // Used to interpret deviceInfo field.
       bool isDeviceInfoNull,
       // Information about incoming RX packet.
       sl_zigbee_rx_packet_info_t *packetInfo,
       // Return: Information about the network.
       const sl_zigbee_zll_network_t *networkInfo,
       // Return: Device specific information.
       const sl_zigbee_zll_device_info_record_t *deviceInfo))
{
  (void)networkInfo;
  (void)isDeviceInfoNull;
  (void)deviceInfo;
  (void)packetInfo;
}

// This call is fired when a ZLL network scan finds a ZLL network.
void sl_zigbee_ezsp_zll_network_found_handler(
  // Information about the network.
  sl_zigbee_zll_network_t *networkInfo,
  // Used to interpret deviceInfo field.
  bool isDeviceInfoNull,
  // Device specific information.
  sl_zigbee_zll_device_info_record_t *deviceInfo,
  // Information about incoming RX packet.
  sl_zigbee_rx_packet_info_t *packetInfo)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_zll_network_found(networkInfo, isDeviceInfoNull, deviceInfo, packetInfo);
  sl_zigbee_af_zll_network_found_cb(isDeviceInfoNull, packetInfo, networkInfo, deviceInfo);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
WEAK(void sl_zigbee_af_debug_cb(
       // debug message length
       uint8_t messageLength,
       // debug message
       uint8_t *messageContents))
{
  (void)messageLength;
  (void)messageContents;
}

void ezspDebugHandler(uint8_t messageLength,
                      uint8_t *messageContents)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_debug(messageLength, messageContents);
  sl_zigbee_af_debug_cb(messageLength, messageContents);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_raw_transmit_complete_cb
WEAK(void sl_zigbee_af_raw_transmit_complete_cb(
       // message
       uint8_t  messageLength,
       uint8_t* messageContents,
       // SL_STATUS_OK if the transmission was successful, or
       // SL_STATUS_ZIGBEE_DELIVERY_FAILED if not
       sl_status_t status))
{
  (void)messageLength;
  (void)messageContents;
  (void)status;
}

void sl_zigbee_ezsp_raw_transmit_complete_handler(uint8_t  messageLength,
                                                  uint8_t* messageContents,
                                                  sl_status_t status)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_raw_transmit_complete(messageLength, messageContents, status);
  sl_zigbee_af_raw_transmit_complete_cb(messageLength, messageContents, status);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_mac_filter_match_message_cb
WEAK(void sl_zigbee_af_mac_filter_match_message_cb(
       // filter value match.
       sl_zigbee_mac_filter_match_data_t filterValueMatch,
       // message type.
       sl_zigbee_mac_passthrough_type_t messageType,
       // RX packet info.
       sl_zigbee_rx_packet_info_t *packetInfo,
       // message length.
       uint8_t messageLength,
       // message contents.
       uint8_t *messageContents))
{
  (void)filterValueMatch;
  (void)messageType;
  (void)packetInfo;
  (void)messageLength;
  (void)messageContents;
}

void sl_zigbee_ezsp_mac_filter_match_message_handler(sl_zigbee_mac_filter_match_data_t filterValueMatch,
                                                     uint8_t legacyPassthroughType,
                                                     sl_zigbee_rx_packet_info_t *packetInfo,
                                                     uint8_t messageLength,
                                                     uint8_t *messageContents)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_mac_filter_match_message(filterValueMatch, legacyPassthroughType, packetInfo, messageLength, messageContents);
  sl_zigbee_af_mac_filter_match_message_cb(filterValueMatch, legacyPassthroughType, packetInfo, messageLength, messageContents);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
WEAK(void sl_zigbee_af_mac_passthrough_message_cb(
       // The type of MAC passthrough message received.
       sl_zigbee_mac_passthrough_type_t messageType,
       // RX packet info.
       sl_zigbee_rx_packet_info_t *packetInfo,
       // message length.
       uint8_t messageLength,
       // The raw message that was received.
       uint8_t *messageContents))
{
  (void)messageType;
  (void)packetInfo;
  (void)messageLength;
  (void)messageContents;
}

void sl_zigbee_ezsp_mac_passthrough_message_handler(uint8_t messageType,
                                                    sl_zigbee_rx_packet_info_t *packetInfo,
                                                    uint8_t messageLength,
                                                    uint8_t *messageContents)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_mac_passthrough_message(messageType, packetInfo, messageLength, messageContents);
  sl_zigbee_af_mac_passthrough_message_cb(messageType, packetInfo, messageLength, messageContents);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
WEAK(void sl_zigbee_af_bootload_transmit_complete_cb(
       // An sl_status_t value of SL_STATUS_OK if an ACK was received from the
       // destination or SL_STATUS_ZIGBEE_DELIVERY_FAILED if no ACK was received.
       sl_status_t status,
       // message length.
       uint8_t messageLength,
       // The bootload message that was sent.
       uint8_t *messageContents))
{
  (void)status;
  (void)messageLength;
  (void)messageContents;
}

void sl_zigbee_ezsp_bootload_transmit_complete_handler(sl_status_t status,
                                                       uint8_t messageLength,
                                                       uint8_t *messageContents)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_bootload_transmit_complete(status, messageLength, messageContents);
  sl_zigbee_af_bootload_transmit_complete_cb(status, messageLength, messageContents);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_incoming_bootload_message_cb
WEAK(void sl_zigbee_af_incoming_bootload_message_cb(
       // The EUI64 of the sending node.
       sl_802154_long_addr_t longId,
       // RX packet info.
       sl_zigbee_rx_packet_info_t *packetInfo,
       // message length.
       uint8_t messageLength,
       // The bootload message that was sent.
       uint8_t *messageContents))
{
  (void)longId;
  (void)packetInfo;
  (void)messageLength;
  (void)messageContents;
}

void sl_zigbee_ezsp_incoming_bootload_message_handler(sl_802154_long_addr_t longId,
                                                      sl_zigbee_rx_packet_info_t *packetInfo,
                                                      uint8_t messageLength,
                                                      uint8_t *messageContents)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_incoming_bootload_message(longId, packetInfo, messageLength, messageContents);
  sl_zigbee_af_incoming_bootload_message_cb(longId, packetInfo, messageLength, messageContents);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_poll_cb
WEAK(void sl_zigbee_af_poll_cb(
       // The node ID of the child that is requesting data.
       sl_802154_short_addr_t childId,
       // True if transmit expected, false otherwise.
       bool transmitExpected))
{
  (void)childId;
  (void)transmitExpected;
}

void sl_zigbee_ezsp_poll_handler(sl_802154_short_addr_t childId, bool transmitExpected)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_poll(childId, transmitExpected);
  sl_zigbee_af_poll_cb(childId, transmitExpected);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_remote_delete_binding_cb
WEAK(void sl_zigbee_af_remote_delete_binding_cb(
       // The index of the binding whose deletion was requested.
       uint8_t index,
       // ZDO response status
       sl_zigbee_zdo_status_t status))
{
  (void)index;
  (void)status;
  return;
}

void sl_zigbee_ezsp_remote_delete_binding_handler(uint8_t index,
                                                  sl_status_t policyDecision)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_remote_delete_binding(index, policyDecision);
  sl_zigbee_af_remote_delete_binding_cb(index, policyDecision);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_remote_set_binding_cb
WEAK(void sl_zigbee_af_remote_set_binding_cb(
       // The contents of the binding entry.
       sl_zigbee_binding_table_entry_t *entry,
       // ZDO status.
       sl_zigbee_zdo_status_t status))
{
  (void)entry;
  (void)status;
  return;
}

void sl_zigbee_ezsp_remote_set_binding_handler(sl_zigbee_binding_table_entry_t *entry,
                                               uint8_t index,
                                               sl_status_t policyDecision)
{
  UNUSED_VAR(index);

  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_remote_set_binding(entry, policyDecision);
  sl_zigbee_af_remote_set_binding_cb(entry, policyDecision);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback
//sl_zigbee_af_calculate_smacs_283k1_handler_cb
WEAK(void sl_zigbee_af_calculate_smacs_283k1_handler_cb(
       // The Result of the CBKE operation.
       sl_status_t status,
       // Return: The calculated value of the initiator's SMAC
       sl_zigbee_smac_data_t *initiatorSmac,
       // Return: The calculated value of the responder's SMAC
       sl_zigbee_smac_data_t *responderSmac))
{
  (void)status;
  (void)initiatorSmac;
  (void)responderSmac;
}

void sl_zigbee_ezsp_calculate_smacs_283k1_handler(sl_status_t status,
                                                  sl_zigbee_smac_data_t* initiatorSmac,
                                                  sl_zigbee_smac_data_t* responderSmac)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_calculate_smacs_283k1_handler(status, initiatorSmac, responderSmac);
  sl_zigbee_af_calculate_smacs_283k1_handler_cb(status, initiatorSmac, responderSmac);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback
//sl_zigbee_af_generate_cbke_keys_283k1_handler_cb
WEAK(void sl_zigbee_af_generate_cbke_keys_283k1_handler_cb(
       // The result of the CBKE operation.
       sl_status_t status,
       // Return: The generated ephemeral public key.
       sl_zigbee_public_key_283k1_data_t *ephemeralPublicKey))
{
  (void)status;
  (void)ephemeralPublicKey;
}

void sl_zigbee_ezsp_generate_cbke_keys_283k1_handler(sl_status_t status,
                                                     sl_zigbee_public_key_283k1_data_t* ephemeralPublicKey)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_generate_cbke_keys_handler283k1(status, ephemeralPublicKey);
  sl_zigbee_af_generate_cbke_keys_283k1_handler_cb(status, ephemeralPublicKey);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_calculate_smacs_cb
WEAK(void sl_zigbee_af_calculate_smacs_cb(
       // The Result of the CBKE operation.
       sl_status_t status,
       // Return: The calculated value of the initiator's SMAC
       sl_zigbee_smac_data_t *initiatorSmac,
       // Return: The calculated value of the responder's SMAC
       sl_zigbee_smac_data_t *responderSmac))
{
  (void)status;
  (void)initiatorSmac;
  (void)responderSmac;
}

void sl_zigbee_ezsp_calculate_smacs_handler(sl_status_t status,
                                            sl_zigbee_smac_data_t* initiatorSmac,
                                            sl_zigbee_smac_data_t* responderSmac)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_calculate_smacs(status, initiatorSmac, responderSmac);
  sl_zigbee_af_calculate_smacs_cb(status, initiatorSmac, responderSmac);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_generate_cbke_keys_cb
WEAK(void sl_zigbee_af_generate_cbke_keys_cb(
       // The result of the CBKE operation.
       sl_status_t status,
       // Return: The generated ephemeral public key.
       sl_zigbee_public_key_data_t *ephemeralPublicKey))
{
  (void)status;
  (void)ephemeralPublicKey;
}

void sl_zigbee_ezsp_generate_cbke_keys_handler(sl_status_t status,
                                               sl_zigbee_public_key_data_t* ephemeralPublicKey)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_generate_cbke_keys(status, ephemeralPublicKey);
  sl_zigbee_af_generate_cbke_keys_cb(status, ephemeralPublicKey);
  sl_zigbee_af_pop_network_index();
}
