/*****************************************************************************
 * @file
 * @brief Convenience stubs for little-used callbacks.
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
#include PLATFORM_HEADER
#include "sl_common.h"
#include "sl_code_classification.h"

#ifndef SL_ZIGBEE_AF_NCP
#include "stack/include/sl_zigbee.h"
#include "stack/include/message.h" // Required for packetHandlers
#include "stack/include/sl_zigbee_types_internal.h"
#include "stack/internal/inc/raw-message-internal-def.h"

#include  "mac-phy.h"
#include  "scan.h"

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_incoming_message_cb
SL_WEAK void sl_zigbee_incoming_message_handler(
  // Incoming message type
  sl_zigbee_incoming_message_type_t type,
  // The aps Frame
  sl_zigbee_aps_frame_t *apsFrame,
  // Rx packet information
  sl_zigbee_rx_packet_info_t *packetInfo,
  // Message length
  uint8_t messageLength,
  // Message pointer
  uint8_t *message)
{
  (void)type;
  (void)apsFrame;
  (void)packetInfo;
  (void)messageLength;
  (void)message;
}

// -----------------------------------------------------------------------------
// A callback invoked when a messageSend has been tried by the stack.
SL_WEAK void sl_zigbee_message_sent_handler(
  // Message delivery status
  sl_status_t status,
  // Outgoing message type
  sl_zigbee_outgoing_message_type_t type,
  // Index or destination of the message
  uint16_t indexOrDestination,
  // The aps Frame
  sl_zigbee_aps_frame_t *apsFrame,
  // Message tag
  uint16_t messageTag,
  // Message length
  uint8_t messageLength,
  // Pointer to the message
  uint8_t *message)
{
  (void)status;
  (void)type;
  (void)indexOrDestination;
  (void)apsFrame;
  (void)messageTag;
  (void)messageLength;
  (void)message;
}

sl_zigbee_join_decision_t sl_zigbee_default_trust_center_decision = SL_ZIGBEE_USE_PRECONFIGURED_KEY;

SL_WEAK sl_zigbee_join_decision_t sl_zigbee_internal_trust_center_join_handler(sl_802154_short_addr_t newNodeId,
                                                                               sl_802154_long_addr_t newNodeEui64,
                                                                               sl_zigbee_device_update_t status,
                                                                               sl_802154_short_addr_t parentOfNewNode)
{
  (void)newNodeId;
  (void)newNodeEui64;
  (void)status;
  (void)parentOfNewNode;

  //emzigbee-241-4
#if defined(SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR)
  if (sl_zigbee_default_trust_center_decision != SL_ZIGBEE_DENY_JOIN
      && parentOfNewNode != sli_zigbee_stack_get_node_id()) {
    // stodo: not sure about the condition above,
    // because we should get here only if the new child is not our
    // direct child and it is multiple hops away?
    bool deviceLeft = (status == SL_ZIGBEE_DEVICE_LEFT);
    sli_zigbee_change_source_route_entry(newNodeId, parentOfNewNode, false, deviceLeft);
  }
#endif

  if (status == SL_ZIGBEE_STANDARD_SECURITY_SECURED_REJOIN
      || status == SL_ZIGBEE_DEVICE_LEFT) {
    return SL_ZIGBEE_NO_ACTION;
  }

  return sl_zigbee_default_trust_center_decision;
}

// -----------------------------------------------------------------------------
//post-join handler
SL_WEAK void sl_zigbee_trust_center_post_join_handler(sl_802154_short_addr_t newNodeId,
                                                      sl_802154_long_addr_t newNodeEui64,
                                                      sl_zigbee_device_update_t status,
                                                      sl_zigbee_join_decision_t policyDecision,
                                                      sl_802154_short_addr_t parentOfNewNode)
{
  (void)newNodeId;
  (void)newNodeEui64;
  (void)status;
  (void)policyDecision;
  (void)parentOfNewNode;
}

// -----------------------------------------------------------------------------
// A callback that allows marking application buffers
SL_WEAK void sl_zigbee_mark_buffers_handler(void)
{
  // stub function
}

// -----------------------------------------------------------------------------
// A callback that allows the app to instrument the stack on what to do with an
// incoming packet
SL_WEAK sl_zigbee_packet_action_t sl_zigbee_internal_packet_handoff_incoming_handler(
  sl_zigbee_zigbee_packet_type_t packetType,
  sli_buffer_manager_buffer_t packetBuffer,
  uint8_t index,
  // Return:
  void *data,
  uint8_t data_len)
{
  (void)packetType;
  (void)packetBuffer;
  (void)index;
  (void)data;
  (void)data_len;
  return SL_ZIGBEE_ACCEPT_PACKET;
}

// -----------------------------------------------------------------------------
// A callback that allows the app to instrument the stack on what to do with an
// outgoing packet
SL_WEAK sl_zigbee_packet_action_t sl_zigbee_internal_packet_handoff_outgoing_handler(
  sl_zigbee_zigbee_packet_type_t packetType,
  sli_buffer_manager_buffer_t packetBuffer,
  uint8_t index,
  // Return:
  void *data,
  uint8_t data_len)
{
  (void)packetType;
  (void)packetBuffer;
  (void)index;
  (void)data;
  (void)data_len;
  return SL_ZIGBEE_ACCEPT_PACKET;
}

// -----------------------------------------------------------------------------
// sl_zigbee_incoming_mfg_test_message_handler
SL_WEAK void sl_zigbee_incoming_mfg_test_message_handler(
  uint8_t messageType,
  uint8_t dataLength,
  // Return:
  uint8_t *data)
{
  (void)messageType;
  (void)dataLength;
  (void)data;
  return;
}

// -----------------------------------------------------------------------------
// Allows the application to decide if it wants to handle a route record itself
SL_WEAK void sl_zigbee_override_incoming_route_record_handler(
  sl_zigbee_rx_packet_info_t *packetInfo,
  uint8_t relayCount,
  uint8_t *relayList,
  // Return:
  bool *consumed)
{
  (void)packetInfo;
  (void)relayCount;
  (void)relayList;
  (void)consumed;
}

// -----------------------------------------------------------------------------
// Allows the application to decide if it wants to append a source route to
// packet header itself
// Return: Added bytes
SL_WEAK uint8_t sl_zigbee_internal_override_append_source_route_handler(
  sl_802154_short_addr_t destination,
  // Return:
  sli_buffer_manager_buffer_t *header,
  // Return:
  bool *consumed)
{
  (void)destination;
  (void)header;
  (void)consumed;
  return 0;
}

// -----------------------------------------------------------------------------
// Allows the application to override the further index of an entry in source
// route table
SL_WEAK void sl_zigbee_internal_override_furthur_index_for_source_route_add_entry_handler(
  sl_802154_short_addr_t id,
  // Return:
  uint8_t *furtherIndex)
{
  (void)id;
  (void)furtherIndex;
}

// -----------------------------------------------------------------------------
// A callback invoked when the status of the stack changes. If the status
// parameter equals SL_STATUS_NETWORK_UP, then the getNetworkParameters command can
// be called to obtain the new network parameters. If any of the parameters are
// being stored in nonvolatile memory by the Host, the stored values should be
// updated.
SL_WEAK void sl_zigbee_stack_status_handler(
  // Stack status
  sl_status_t status)
{
  (void)status;
}

// -----------------------------------------------------------------------------
// A callback that allows the app to redirect the packet
SL_WEAK void sl_zigbee_redirect_outgoing_message_handler(
  // The mac index used to redirect messages
  uint8_t mac_index,
  // Packet length
  uint8_t packet_length,
  // Packet contents
  uint8_t* packet_contents,
  // Transmit priority
  uint8_t priority)
{
  (void)mac_index;
  (void)packet_length;
  (void)packet_contents;
  (void)priority;
}

// -----------------------------------------------------------------------------
// Reports the result of an energy scan for a single channel. The scan is not
// complete until the scanCompleteHandler callback is called.
SL_WEAK void sl_zigbee_energy_scan_result_handler(
  // The 802.15.4 channel number that was scanned.
  uint8_t channel,
  // The maximum RSSI value found on the channel.
  int8_t maxRssiValue)
{
  (void)channel;
  (void)maxRssiValue;
}

// -----------------------------------------------------------------------------
// Reports that a network was found as a result of a prior call to startScan.
// Gives the network parameters useful for deciding which network to join.
SL_WEAK void sl_zigbee_network_found_handler(
  // The parameters associated with the network found.
  sl_zigbee_zigbee_network_t *networkFound,
  // The link quality indication of the network found.
  uint8_t lqi,
  // The received signal strength indication of the network found.
  int8_t rssi)
{
  (void)networkFound;
  (void)lqi;
  (void)rssi;
}

// -----------------------------------------------------------------------------
// Returns the status of the current scan of type SL_ZIGBEE_ENERGY_SCAN or
// SL_ZIGBEE_ACTIVE_SCAN. SL_STATUS_OK signals that the scan has completed. Other
// error conditions signify a failure to scan on the channel specified.
SL_WEAK void sl_zigbee_scan_complete_handler(
  // The channel on which the current error occurred. Undefined for the case
  // of SL_STATUS_OK.
  uint8_t channel,
  // The error condition that occurred on the current channel. Value will be
  // SL_STATUS_OK when the scan has completed.
  sl_status_t status)
{
  (void)channel;
  (void)status;
}

// -----------------------------------------------------------------------------
// This function returns an unused panID and channel pair found via the find
// unused panId scan procedure.
SL_WEAK void sl_zigbee_unused_pan_id_found_handler(
  // The unused panID which has been found.
  sl_802154_pan_id_t panId,
  // The channel that the unused panID was found on.
  uint8_t channel)
{
  (void)panId;
  (void)channel;
}

// -----------------------------------------------------------------------------
// Indicates that a child has joined or left.
SL_WEAK void sl_zigbee_child_join_handler(
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
  (void)index;
  (void)joining;
  (void)childId;
  (void)childEui64;
  (void)childType;
}
// -----------------------------------------------------------------------------
// Callback fires when the duty cycle state has changed
SL_WEAK void sl_zigbee_duty_cycle_handler(
  // The channel page whose duty cycle state has changed.
  uint8_t channelPage,
  // The channel number whose duty cycle state has changed.
  uint8_t channel,
  // The current duty cycle state.
  sl_zigbee_duty_cycle_state_t state,
  uint8_t totalDevices,
  sl_zigbee_per_device_duty_cycle_t *arrayOfDeviceDutyCycles)
{
  (void)channelPage;
  (void)channel;
  (void)state;
  (void)totalDevices;
  (void)arrayOfDeviceDutyCycles;
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_remote_set_binding_handler
SL_WEAK void sl_zigbee_remote_set_binding_handler(
  // The contents of the binding entry.
  sl_zigbee_binding_table_entry_t *entry,
  // ZDO status.
  sl_zigbee_zdo_status_t status)
{
  (void)entry;
  (void)status;
  return;
}

// The NCP used the external binding modification policy to decide how to handle
// a remote set binding request. The Host cannot change the current decision,
// but it can change the policy for future decisions using the setPolicy
// command.
// Return: ZDO response status.
SL_WEAK sl_zigbee_zdo_status_t sl_zigbee_internal_remote_set_binding_handler(
  // Return: The contents of the binding entry.
  sl_zigbee_binding_table_entry_t *entry)
{
  (void)entry;
  return SL_ZIGBEE_ZDP_TABLE_FULL;
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_remote_delete_binding_handler
SL_WEAK void sl_zigbee_remote_delete_binding_handler(
  // The index of the binding whose deletion was requested.
  uint8_t index,
  // ZDO response status
  sl_zigbee_zdo_status_t status)
{
  (void)index;
  (void)status;
  return;
}

// The NCP used the external binding modification policy to decide how to handle
// a remote delete binding request. The Host cannot change the current decision,
// but it can change the policy for future decisions using the setPolicy
// command.
// Return: ZDO response status
SL_WEAK sl_zigbee_zdo_status_t sl_zigbee_internal_remote_delete_binding_handler(
  // The index of the binding whose deletion was requested.
  uint8_t index)
{
  (void)index;
  return SL_ZIGBEE_ZDP_TABLE_FULL;
}

// -----------------------------------------------------------------------------
// Indicates the result of a data poll to the parent of the local node.
SL_WEAK void sl_zigbee_poll_complete_handler(
  // An sl_status_t value: SL_STATUS_OK - Data was received in response to
  // the poll. SL_STATUS_MAC_NO_DATA - No data was pending.
  // SL_STATUS_ZIGBEE_DELIVERY_FAILED - The poll message could not be sent.
  // SL_STATUS_MAC_NO_ACK_RECEIVED - The poll message was sent but not
  // acknowledged by the parent.
  sl_status_t status)
{
  (void)status;
}

// -----------------------------------------------------------------------------
// Indicates that the local node received a data poll from a child.
SL_WEAK void sl_zigbee_poll_handler(
  // The node ID of the child that is requesting data.
  sl_802154_short_addr_t childId,
  // True if transmit expected, false otherwise.
  bool transmitExpected)
{
  (void)childId;
  (void)transmitExpected;
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_debug_handler
SL_WEAK void sl_zigbee_debug_handler(
  // debug message length
  uint8_t messageLength,
  // debug message
  uint8_t *messageContents)
{
  (void)messageLength;
  (void)messageContents;
}

// -----------------------------------------------------------------------------
// A callback indicating that a many-to-one route to the concentrator with the
// given short and long id is available for use.
SL_WEAK void sl_zigbee_incoming_many_to_one_route_request_handler(
  // The short id of the concentrator.
  sl_802154_short_addr_t source,
  // The EUI64 of the concentrator.
  sl_802154_long_addr_t longId,
  // The path cost to the concentrator. The cost may decrease as additional
  // route request packets for this discovery arrive, but the callback is
  // made only once.
  uint8_t cost)
{
  (void)source;
  (void)longId;
  (void)cost;
}

// -----------------------------------------------------------------------------
// A callback invoked when a route error message is received. The error
// indicates that a problem routing to or from the target node was encountered.
SL_WEAK void sl_zigbee_incoming_route_error_handler(
  // SL_STATUS_ZIGBEE_SOURCE_ROUTE_FAILURE or SL_STATUS_ZIGBEE_MANY_TO_ONE_ROUTE_FAILURE.
  sl_status_t status,
  // The short id of the remote node.
  sl_802154_short_addr_t target)
{
  (void)status;
  (void)target;
}

// -----------------------------------------------------------------------------
// A callback invoked when a network status/route error message is received. The
// error indicates that there was a problem sending/receiving messages from the
// target node
SL_WEAK void sl_zigbee_incoming_network_status_handler(
  // One byte over-the-air error code from network status message
  uint8_t errorCode,
  // The short ID of the remote node
  sl_802154_short_addr_t target)
{
  (void)errorCode;
  (void)target;
}

// -----------------------------------------------------------------------------
// Reports the arrival of a route record command frame.
SL_WEAK void sl_zigbee_incoming_route_record_handler(
  sl_zigbee_rx_packet_info_t *packetInfo,
  uint8_t relayCount,
  uint8_t *relayList)
{
  (void)packetInfo;
  (void)relayCount;
  (void)relayList;
}

// -----------------------------------------------------------------------------
// A callback invoked by the EmberZNet stack when an id conflict is discovered,
// that is, two different nodes in the network were found to be using the same
// short id. The stack automatically removes the conflicting short id from its
// internal tables (address, binding, route, neighbor, and child tables). The
// application should discontinue any other use of the id.
SL_WEAK void sl_zigbee_id_conflict_handler(
  // The short id for which a conflict was detected
  sl_802154_short_addr_t conflictingId)
{
  (void)conflictingId;
}

// -----------------------------------------------------------------------------
// A callback invoked by the EmberZNet stack when a MAC passthrough message is
// received.
SL_WEAK void sl_802154_passthrough_message_handler(
  // The type of MAC passthrough message received.
  sl_zigbee_mac_passthrough_type_t messageType,
  // RX packet info.
  sl_zigbee_rx_packet_info_t *packetInfo,
  // message length.
  uint8_t messageLength,
  // The raw message that was received.
  uint8_t *messageContents)
{
  (void)messageType;
  (void)packetInfo;
  (void)messageLength;
  (void)messageContents;
}

// -----------------------------------------------------------------------------
// A callback invoked to inform the application that a stack token has changed.
SL_WEAK void sl_zigbee_stack_token_changed_handler(
  // The address of the stack token that has changed.
  uint16_t tokenAddress)
{
  (void)tokenAddress;
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_timer_handler
SL_WEAK void sl_zigbee_timer_handler(
  // Which timer generated the callback (0 or 1).
  uint8_t timerId)
{
  (void)timerId;
}

// -----------------------------------------------------------------------------
// This call is fired when a counter exceeds its threshold
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_ZIGBEE_STACK, SL_CODE_CLASS_TIME_CRITICAL)
SL_WEAK void sl_zigbee_counter_rollover_handler(
  // Type of Counter
  sl_zigbee_counter_type_t type)
{
  (void)type;
}

// -----------------------------------------------------------------------------
// A callback invoked by the EmberZNet stack when the MAC has finished
// transmitting a raw message.
SL_WEAK void sl_zigbee_raw_transmit_complete_handler(
  // message
  uint8_t  messageLength,
  uint8_t* messageContents,
  // SL_STATUS_OK if the transmission was successful, or
  // SL_STATUS_ZIGBEE_DELIVERY_FAILED if not
  sl_status_t status,
  uint8_t messageTag)
{
  (void)messageLength;
  (void)messageContents;
  (void)status;
  (void)messageTag;
}

// -----------------------------------------------------------------------------
// A callback to inform the application that the Network Key has been updated
// and the node has been switched over to use the new key. The actual key being
// used is not passed up, but the sequence number is.
SL_WEAK void sl_zigbee_switch_network_key_handler(
  // The sequence number of the new network key.
  uint8_t sequenceNumber)
{
  (void)sequenceNumber;
}

// -----------------------------------------------------------------------------
// This is a callback that indicates the success or failure of an attempt to
// establish a key with a partner device.
SL_WEAK void sl_zigbee_zigbee_key_establishment_handler(
  // This is the IEEE address of the partner that the device successfully
  // established a key with. This value is all zeros on a failure.
  sl_802154_long_addr_t partner,
  // This is the status indicating what was established or why the key
  // establishment failed.
  sl_zigbee_key_status_t status)
{
  (void)partner;
  (void)status;
}

// -----------------------------------------------------------------------------
// A callback by the Crypto Engine indicating that a new ephemeral
// public/private key pair has been generated. The public/private key pair is
// stored on the NCP, but only the associated public key is returned to the
// host. The node's associated certificate is also returned.
SL_WEAK void sl_zigbee_generate_cbke_keys_handler(
  // The result of the CBKE operation.
  sl_status_t status,
  // Return: The generated ephemeral public key.
  sl_zigbee_public_key_data_t *ephemeralPublicKey)
{
  (void)status;
  (void)ephemeralPublicKey;
}

// -----------------------------------------------------------------------------
// A callback to indicate that the NCP has finished calculating the Secure
// Message Authentication Codes (SMAC) for both the initiator and responder. The
// associated link key is kept in temporary storage until the host tells the NCP
// to store or discard the key via sl_zigbee_clear_temporary_data_maybe_store_link_key().
SL_WEAK void sl_zigbee_calculate_smacs_handler(
  // The Result of the CBKE operation.
  sl_status_t status,
  // Return: The calculated value of the initiator's SMAC
  sl_zigbee_smac_data_t *initiatorSmac,
  // Return: The calculated value of the responder's SMAC
  sl_zigbee_smac_data_t *responderSmac)
{
  (void)status;
  (void)initiatorSmac;
  (void)responderSmac;
}

// -----------------------------------------------------------------------------
// The handler that returns the results of the signing operation. On success,
// the signature will be appended to the original message (including the
// signature type indicator that replaced the startIndex field for the signing)
// and both are returned via this callback.
SL_WEAK void sl_zigbee_dsa_sign_handler(
  // The result of the DSA signing operation.
  sl_status_t status,
  // The length of message.
  uint8_t messageLength,
  // The message and attached which includes the original message and the
  // appended signature.
  uint8_t *messageContents)
{
  (void)status;
  (void)messageLength;
  (void)messageContents;
}

// -----------------------------------------------------------------------------
// This callback is executed by the stack when the DSA verification has
// completed and has a result. If the result is SL_STATUS_OK, the signature is
// valid. If the result is SL_STATUS_ZIGBEE_SIGNATURE_VERIFY_FAILURE then the signature is
// invalid. If the result is anything else then the signature verify operation
// failed and the validity is unknown.
SL_WEAK void sl_zigbee_dsa_verify_handler(
  // The result of the DSA verification operation.
  sl_status_t status)
{
  (void)status;
}

// -----------------------------------------------------------------------------
// A callback invoked by the EmberZNet stack when a bootload message is
// received.
SL_WEAK void sl_zigbee_incoming_bootload_message_handler(
  // The EUI64 of the sending node.
  sl_802154_long_addr_t longId,
  // RX packet info.
  sl_zigbee_rx_packet_info_t *packetInfo,
  // message length.
  uint8_t messageLength,
  // The bootload message that was sent.
  uint8_t *messageContents)
{
  (void)longId;
  (void)packetInfo;
  (void)messageLength;
  (void)messageContents;
}

// -----------------------------------------------------------------------------
// A callback invoked by the EmberZNet stack when the MAC has finished
// transmitting a bootload message.
SL_WEAK void sl_zigbee_bootload_transmit_complete_handler(
  // An sl_status_t value of SL_STATUS_OK if an ACK was received from the
  // destination or SL_STATUS_ZIGBEE_DELIVERY_FAILED if no ACK was received.
  sl_status_t status,
  // message length.
  uint8_t messageLength,
  // The bootload message that was sent.
  uint8_t *messageContents)
{
  (void)status;
  (void)messageLength;
  (void)messageContents;
}

// -----------------------------------------------------------------------------
// This call is fired when a ZLL network scan finds a ZLL network.
SL_WEAK void sl_zigbee_zll_network_found_handler(
  sl_zigbee_zll_network_t* networkInfo,
  bool isDeviceInfoNull,
  sl_zigbee_zll_device_info_record_t* deviceInfo,
  sl_zigbee_rx_packet_info_t *packetInfo)
{
  (void)networkInfo;
  (void)isDeviceInfoNull;
  (void)deviceInfo;
  (void)packetInfo;
}

// -----------------------------------------------------------------------------
// This call is fired when a ZLL network scan is complete.
SL_WEAK void sl_zigbee_zll_scan_complete_handler(
  // Status of the operation.
  sl_status_t status)
{
  (void)status;
}

// -----------------------------------------------------------------------------
// This call is fired when network and group addresses are assigned to a remote
// mode in a network start or network join request.
SL_WEAK void sl_zigbee_zll_address_assignment_handler(
  // Return: Address assignment information.
  sl_zigbee_zll_address_assignment_t* addressInfo,
  sl_zigbee_rx_packet_info_t *packetInfo)
{
  (void)packetInfo;
  (void)addressInfo;
}

// -----------------------------------------------------------------------------
// This call is fired when the device is a target of a touch link.
SL_WEAK void sl_zigbee_zll_touch_link_target_handler(
  // Return: Information about the network.
  const sl_zigbee_zll_network_t *networkInfo)
{
  (void)networkInfo;
}

// -----------------------------------------------------------------------------
// A callback invoked by the EmberZNet stack when a raw MAC message that has
// matched one of the application's configured MAC filters.
SL_WEAK void sl_802154_filter_match_message_handler(
  // filter value match.
  sl_zigbee_mac_filter_match_data_t filterValueMatch,
  // message type.
  sl_zigbee_mac_passthrough_type_t messageType,
  // RX packet info.
  sl_zigbee_rx_packet_info_t *packetInfo,
  // message length.
  uint8_t messageLength,
  // message contents.
  uint8_t *messageContents)
{
  (void)filterValueMatch;
  (void)messageType;
  (void)packetInfo;
  (void)messageLength;
  (void)messageContents;
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_d_gp_sent_cb
SL_WEAK void sli_zigbee_stack_d_gp_sent_handler(
  // An sl_status_t value indicating success or the reason for failure.
  sl_status_t status,
  // The handle of the GPDF.
  uint8_t gpepHandle)
{
  (void)status;
  (void)gpepHandle;
}

// -----------------------------------------------------------------------------
// This handler is called by the stack to report the number of conflict reports
// exceeds SL_ZIGBEE_PAN_ID_CONFLICT_REPORT_THRESHOLD within a period of 1 minute )
// Return: An sl_status_t value indicating success or the reason for failure.
SL_WEAK sl_status_t sl_zigbee_internal_pan_id_conflict_handler(
  // Number of conflict reports
  int8_t conflictCount)
{
  (void)conflictCount;
  return SL_STATUS_NOT_AVAILABLE;
}

// -----------------------------------------------------------------------------
// Inform the application that an orphan notification has been received. This is
// generally not useful for applications. It could be useful in testing and is
// included for this purpose.
SL_WEAK void sl_zigbee_orphan_notification_handler(
  // The 8 byte EUI64 of the sender.
  sl_802154_long_addr_t longId)
{
  (void)longId;
}

// -----------------------------------------------------------------------------
// This handler is called for messages that need to be matched against mac
// passthrough filter
// Return: True if mac passthrough filter was matched. False otherwise.
SL_WEAK bool sl_802154_internal_passthrough_filter_handler(
  // Return: Mac Header of the matched messgae
  uint8_t *macHeader)
{
  (void)macHeader;
  return false;
}

// -----------------------------------------------------------------------------
// A callback by the Crypto Engine indicating that a new ephemeral
// public/private key pair has been generated. The public/private key pair is
// stored on the NCP, but only the associated public key is returned to the
// host. The node's associated certificate is also returned.
SL_WEAK void sl_zigbee_generate_cbke_keys_283k1_handler(
  // The result of the CBKE operation.
  sl_status_t status,
  // Return: The generated ephemeral public key.
  sl_zigbee_public_key_283k1_data_t *ephemeralPublicKey)
{
  (void)status;
  (void)ephemeralPublicKey;
}

// -----------------------------------------------------------------------------
// A callback to indicate that the NCP has finished calculating the Secure
// Message Authentication Codes (SMAC) for both the initiator and responder. The
// associated link key is kept in temporary storage until the host tells the NCP
// to store or discard the key via sl_zigbee_clear_temporary_data_maybe_store_link_key().
SL_WEAK void sl_zigbee_calculate_smacs_283k1_handler(
  // The Result of the CBKE operation.
  sl_status_t status,
  // Return: The calculated value of the initiator's SMAC
  sl_zigbee_smac_data_t *initiatorSmac,
  // Return: The calculated value of the responder's SMAC
  sl_zigbee_smac_data_t *responderSmac)
{
  (void)status;
  (void)initiatorSmac;
  (void)responderSmac;
}

// -----------------------------------------------------------------------------
// A callback invoked by the ZigBee GP stack when a GPDF is received.
SL_WEAK void sl_zigbee_gpep_incoming_message_handler(
  // GP parameters list represented as a macro for GP endpoint incoming message handler and callbacks prototypes.
  sl_zigbee_gp_params_t *params)
{
  (void)params;
}

// -----------------------------------------------------------------------------
// rtos Idle Handler
// Return: True or False.
SL_WEAK bool sl_zigbee_rtos_idle_handler(
  // Return: Idle time duration
  uint32_t *idleTimeMs)
{
  (void)idleTimeMs;
  return false;
}

// -----------------------------------------------------------------------------
// Rtos Stack Wakeup Isr Handler
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_ZIGBEE_STACK, SL_CODE_CLASS_TIME_CRITICAL)
SL_WEAK void sl_zigbee_rtos_stack_wakeup_isr_handler(void)
{
  // stub function
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_radio_needs_calibrating_handler
SL_WEAK void sl_zigbee_radio_needs_calibrating_handler(void)
{
  // stub function
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_scan_error_handler
SL_WEAK void sl_zigbee_scan_error_handler(
  // The error status of a scan
  sl_status_t status)
{
  (void)status;
}

#else //ncp

#include PLATFORM_HEADER
#include "stack/include/sl_zigbee.h"
#include "sl_common.h"
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

//for privileged callback handlers
#include "stack/internal/inc/internal-callbacks-patch.h"

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_incoming_message_cb
SL_WEAK void sl_zigbee_af_incoming_message_cb(
  // Incoming message type
  sl_zigbee_incoming_message_type_t type,
  // The aps Frame
  sl_zigbee_aps_frame_t *apsFrame,
  // Rx packet information
  sl_zigbee_rx_packet_info_t *packetInfo,
  // Message length
  uint8_t messageLength,
  // Message pointer
  uint8_t *message)
{
  (void)type;
  (void)apsFrame;
  (void)packetInfo;
  (void)messageLength;
  (void)message;
}

#if defined(SL_ZIGBEE_AF_NCP) && defined(SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT)
extern void sli_zigbee_af_incoming_message_handler(sl_zigbee_incoming_message_type_t type,
                                                   sl_zigbee_aps_frame_t *apsFrame,
                                                   sl_zigbee_rx_packet_info_t *packetInfo,
                                                   uint16_t messageLength,
                                                   uint8_t *messageContents);

SL_WEAK bool sl_zigbee_af_support_incoming_message_cb(sl_zigbee_aps_frame_t *apsFrame)
{
  (void)apsFrame;
  return false;
}

SL_WEAK bool sl_zigbee_af_support_incoming_message_handler(sl_zigbee_aps_frame_t *apsFrame)
{
  return ((sli_zb_af_support_incoming_message(apsFrame)) \
          || (sl_zigbee_af_support_incoming_message_cb(apsFrame)));
}

#endif // defined(SL_ZIGBEE_AF_NCP) && defined(SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT)

// A callback invoked when receiving a message.
SL_WEAK void sl_zigbee_incoming_message_handler(
  // Incoming message type
  sl_zigbee_incoming_message_type_t type,
  // The aps Frame
  sl_zigbee_aps_frame_t *apsFrame,
  // Packet information
  sl_zigbee_rx_packet_info_t *packetInfo,
  // Message length
  uint8_t messageLength,
  // Message pointer
  uint8_t *message)
{
  sl_zigbee_af_push_callback_network_index();
#if defined(SL_ZIGBEE_AF_NCP) && defined(SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT)
  // Attempt to handle incoming message on NCP. Messages for GP endpoint are handled here.
  // All other messages are passed up to the host.
  if (sl_zigbee_af_support_incoming_message_handler(apsFrame)) {
    sli_zigbee_af_incoming_message_handler(type,
                                           apsFrame,
                                           packetInfo,
                                           messageLength,
                                           message);
  } else
#endif // defined(SL_ZIGBEE_AF_NCP) && defined(SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT)
  {
    sli_zb_af_incoming_message(type, apsFrame, packetInfo, messageLength, message);
    sl_zigbee_af_incoming_message_cb(type, apsFrame, packetInfo, messageLength, message);
  }
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// A callback invoked when a messageSend has been tried by the stack.

SL_WEAK void sli_zigbee_af_message_sent_callback(sl_status_t status,
                                                 sl_zigbee_outgoing_message_type_t type,
                                                 uint16_t indexOrDestination,
                                                 sl_zigbee_aps_frame_t *apsFrame,
                                                 uint16_t messageTag,
                                                 uint8_t messageLength,
                                                 uint8_t *message)
{
  sli_zigbee_af_message_sent(status, type, indexOrDestination, apsFrame, messageTag, messageLength, message);
}

SL_WEAK void sl_zigbee_message_sent_handler(
  // Message delivery status
  sl_status_t status,
  // Outgoing message type
  sl_zigbee_outgoing_message_type_t type,
  // Index or destination of the message
  uint16_t indexOrDestination,
  // The aps Frame
  sl_zigbee_aps_frame_t *apsFrame,
  // Message tag
  uint16_t messageTag,
  // Message length
  uint8_t messageLength,
  // Pointer to the message
  uint8_t *message)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_message_sent_callback(status, type, indexOrDestination, apsFrame, messageTag, messageLength, message);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of sli_zigbee_af_trust_center_join_callback
SL_WEAK sl_zigbee_join_decision_t sli_zigbee_af_trust_center_pre_join_callback(
  // Joining node's id
  sl_802154_short_addr_t newNodeId,
  // Joining node's Eui64
  sl_802154_long_addr_t newNodeEui64,
  // Status
  sl_zigbee_device_update_t status,
  // Parent Id of the joining node
  sl_802154_short_addr_t parentOfNewNode)
{
  (void)newNodeId;
  (void)newNodeEui64;
  (void)status;
  (void)parentOfNewNode;
  return SL_ZIGBEE_NO_ACTION;
}

// A callback invoked when a node attempts to join a TC
SL_WEAK sl_zigbee_join_decision_t sl_zigbee_internal_trust_center_join_handler(
  // Joining node's id
  sl_802154_short_addr_t newNodeId,
  // Joining node's Eui64
  sl_802154_long_addr_t newNodeEui64,
  // Status
  sl_zigbee_device_update_t status,
  // Parent Id of the joining node
  sl_802154_short_addr_t parentOfNewNode)
{
  sl_zigbee_join_decision_t ret;
  sl_zigbee_af_push_callback_network_index();
  ret = sli_zigbee_af_trust_center_pre_join_callback(newNodeId, newNodeEui64, status, parentOfNewNode);
  sl_zigbee_af_pop_network_index();
  return ret;
}

// -----------------------------------------------------------------------------
// Weak implementation of sli_zigbee_af_trust_center_join_callback
SL_WEAK void sl_zigbee_af_trust_center_join_cb(
  // Joining node's id
  sl_802154_short_addr_t newNodeId,
  // Joining node's Eui64
  sl_802154_long_addr_t newNodeEui64,
  // Parent Id of the joining node
  sl_802154_short_addr_t parentOfNewNode,
  // Status
  sl_zigbee_device_update_t status,
  sl_zigbee_join_decision_t joinDecision)
{
  (void)newNodeId;
  (void)newNodeEui64;
  (void)status;
  (void)parentOfNewNode;
  (void)joinDecision;
}

//post-join handler
SL_WEAK void sl_zigbee_trust_center_post_join_handler(sl_802154_short_addr_t newNodeId,
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
  // Should rename this to post_join later (but that change probably needs to be scripted).
  // Also its signature should probably change to align with the handler.
  sl_zigbee_af_trust_center_join_cb(newNodeId,
                                    newNodeEui64,
                                    parentOfNewNode,
                                    status,
                                    policyDecision);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_mark_buffers_cb
SL_WEAK void sl_zigbee_af_mark_buffers_cb(void)
{
}

// A callback that allows marking application buffers
SL_WEAK void sl_zigbee_mark_buffers_handler(void)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_mark_buffers();
  sl_zigbee_af_mark_buffers_cb();
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of sli_zigbee_dispatch_packet_handoff_incoming_callback
SL_WEAK sl_zigbee_packet_action_t sli_zigbee_dispatch_packet_handoff_incoming_callback(
  sl_zigbee_zigbee_packet_type_t packetType,
  sli_buffer_manager_buffer_t packetBuffer,
  uint8_t index,
  // Return:
  void *data,
  uint8_t data_len)
{
  (void)packetType;
  (void)packetBuffer;
  (void)index;
  (void)data;
  (void)data_len;
  return SL_ZIGBEE_ACCEPT_PACKET;
}

// A callback that allows the app to instrument the stack on what to do with an
// incoming packet
SL_WEAK sl_zigbee_packet_action_t sl_zigbee_internal_packet_handoff_incoming_handler(
  sl_zigbee_zigbee_packet_type_t packetType,
  sli_buffer_manager_buffer_t packetBuffer,
  uint8_t index,
  // Return:
  void *data,
  uint8_t data_len)
{
  sl_zigbee_packet_action_t ret;
  sl_zigbee_af_push_callback_network_index();
  ret = sli_zigbee_dispatch_packet_handoff_incoming_callback(packetType, packetBuffer, index, data, data_len);
  sli_zigbee_dispatch_packet_handoff_incoming(packetType, packetBuffer, index, data, data_len);
  sl_zigbee_af_pop_network_index();
  return ret;
}

// -----------------------------------------------------------------------------
// Weak implementation of sli_zigbee_dispatch_packet_handoff_outgoing_callback
SL_WEAK sl_zigbee_packet_action_t sli_zigbee_dispatch_packet_handoff_outgoing_callback(
  sl_zigbee_zigbee_packet_type_t packetType,
  sli_buffer_manager_buffer_t packetBuffer,
  uint8_t index,
  // Return:
  void *data,
  uint8_t data_len)
{
  (void)packetType;
  (void)packetBuffer;
  (void)index;
  (void)data;
  (void)data_len;
  return SL_ZIGBEE_ACCEPT_PACKET;
}

// A callback that allows the app to instrument the stack on what to do with an
// outgoing packet
SL_WEAK sl_zigbee_packet_action_t sl_zigbee_internal_packet_handoff_outgoing_handler(
  sl_zigbee_zigbee_packet_type_t packetType,
  sli_buffer_manager_buffer_t packetBuffer,
  uint8_t index,
  // Return:
  void *data,
  uint8_t data_len)
{
  sl_zigbee_packet_action_t ret;
  sl_zigbee_af_push_callback_network_index();
  ret = sli_zigbee_dispatch_packet_handoff_outgoing_callback(packetType, packetBuffer, index, data, data_len);
  sli_zigbee_dispatch_packet_handoff_outgoing(packetType, packetBuffer, index, data, data_len);
  sl_zigbee_af_pop_network_index();
  return ret;
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_incoming_mfg_test_message_cb
SL_WEAK void sl_zigbee_af_incoming_mfg_test_message_cb(
  uint8_t messageType,
  uint8_t dataLength,
  // Return:
  uint8_t *data)
{
  (void)messageType;
  (void)dataLength;
  (void)data;
  return;
}

// incomingMfgTestMessageHandler
SL_WEAK void sl_zigbee_incoming_mfg_test_message_handler(
  uint8_t messageType,
  uint8_t dataLength,
  // Return:
  uint8_t *data)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_incoming_mfg_test_message(messageType, dataLength, data);
  sl_zigbee_af_incoming_mfg_test_message_cb(messageType, dataLength, data);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Allows the application to decide if it wants to handle a route record itself
SL_WEAK void sl_zigbee_override_incoming_route_record_handler(
  sl_zigbee_rx_packet_info_t *packetInfo,
  uint8_t relayCount,
  uint8_t *relayList,
  // Return:
  bool *consumed)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_override_incoming_route_record(packetInfo, relayCount, relayList, consumed);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of sl_zigbee_af_override_append_source_route_cb
SL_WEAK uint8_t sl_zigbee_af_override_append_source_route_cb(
  sl_802154_short_addr_t destination,
  // Return:
  sli_buffer_manager_buffer_t *header,
  // Return:
  bool *consumed)
{
  (void)destination;
  (void)header;
  (void)consumed;
  return 0;
}

// Allows the application to decide if it wants to append a source route to
// packet header itself
// Return: Added bytes
SL_WEAK uint8_t sl_zigbee_internal_override_append_source_route_handler(
  sl_802154_short_addr_t destination,
  // Return:
  sli_buffer_manager_buffer_t *header,
  // Return:
  bool *consumed)
{
  uint8_t ret;
  sl_zigbee_af_push_callback_network_index();
  ret = sl_zigbee_af_override_append_source_route_cb(destination, header, consumed);
  sli_zigbee_af_override_append_source_route(destination, header, consumed);
  sl_zigbee_af_pop_network_index();
  return ret;
}

// -----------------------------------------------------------------------------
// Allows the application to override the further index of an entry in source
// route table
SL_WEAK void sl_zigbee_internal_override_furthur_index_for_source_route_add_entry_handler(
  sl_802154_short_addr_t id,
  // Return:
  uint8_t *furtherIndex)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_override_furthur_index_for_source_route_add_entry(id, furtherIndex);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_stack_status_cb
SL_WEAK void sl_zigbee_af_stack_status_cb(
  // Stack status
  sl_status_t status)
{
  (void)status;
}

// A callback invoked when the status of the stack changes. If the status
// parameter equals SL_STATUS_NETWORK_UP, then the getNetworkParameters command can
// be called to obtain the new network parameters. If any of the parameters are
// being stored in nonvolatile memory by the Host, the stored values should be
// updated.
SL_WEAK void sl_zigbee_stack_status_handler(
  // Stack status
  sl_status_t status)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_stack_status(status);
  sl_zigbee_af_stack_status_cb(status);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_redirect_outgoing_message_cb
SL_WEAK void sl_zigbee_af_redirect_outgoing_message_cb(
  // The mac index used to redirect messages
  uint8_t mac_index,
  // Packet length
  uint8_t packet_length,
  // Packet contents
  uint8_t* packet_contents,
  // Transmit priority
  uint8_t priority)
{
  (void)mac_index;
  (void)packet_length;
  (void)packet_contents;
  (void)priority;
}

// A callback that allows the app to redirect the packet
SL_WEAK void sl_zigbee_redirect_outgoing_message_handler(
  // The mac index used to redirect messages
  uint8_t mac_index,
  // Packet length
  uint8_t packet_length,
  // Packet contents
  uint8_t* packet_contents,
  // Transmit priority
  uint8_t priority)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_redirect_outgoing_message(mac_index, packet_length, packet_contents, priority);
  sl_zigbee_af_redirect_outgoing_message_cb(mac_index, packet_length, packet_contents, priority);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_energy_scan_result_cb
SL_WEAK void sl_zigbee_af_energy_scan_result_cb(
  // The 802.15.4 channel number that was scanned.
  uint8_t channel,
  // The maximum RSSI value found on the channel.
  int8_t maxRssiValue)
{
  (void)channel;
  (void)maxRssiValue;
}

// Reports the result of an energy scan for a single channel. The scan is not
// complete until the scanCompleteHandler callback is called.
SL_WEAK void sl_zigbee_energy_scan_result_handler(
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
SL_WEAK void sl_zigbee_af_network_found_cb(
  // The parameters associated with the network found.
  sl_zigbee_zigbee_network_t *networkFound,
  // The link quality indication of the network found.
  uint8_t lqi,
  // The received signal strength indication of the network found.
  int8_t rssi)
{
  (void)networkFound;
  (void)lqi;
  (void)rssi;
}

// Reports that a network was found as a result of a prior call to startScan.
// Gives the network parameters useful for deciding which network to join.
SL_WEAK void sl_zigbee_network_found_handler(
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
SL_WEAK void sl_zigbee_af_scan_complete_cb(
  // The channel on which the current error occurred. Undefined for the case
  // of SL_STATUS_OK.
  uint8_t channel,
  // The error condition that occurred on the current channel. Value will be
  // SL_STATUS_OK when the scan has completed.
  sl_status_t status)
{
  (void)channel;
  (void)status;
}

// Returns the status of the current scan of type SL_ZIGBEE_ENERGY_SCAN or
// SL_ZIGBEE_ACTIVE_SCAN. SL_STATUS_OK signals that the scan has completed. Other
// error conditions signify a failure to scan on the channel specified.
SL_WEAK void sl_zigbee_scan_complete_handler(
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
SL_WEAK void sl_zigbee_af_unused_pan_id_found_cb(
  // The unused panID which has been found.
  sl_802154_pan_id_t panId,
  // The channel that the unused panID was found on.
  uint8_t channel)
{
  (void)panId;
  (void)channel;
}

// This function returns an unused panID and channel pair found via the find
// unused panId scan procedure.
SL_WEAK void sl_zigbee_unused_pan_id_found_handler(
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
// Weak implementation of public Callback sl_zigbee_af_child_join_cb
SL_WEAK void sl_zigbee_af_child_join_cb(
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
  (void)index;
  (void)joining;
  (void)childId;
  (void)childEui64;
  (void)childType;
}

// Indicates that a child has joined or left.
SL_WEAK void sl_zigbee_child_join_handler(
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
SL_WEAK void sl_zigbee_af_duty_cycle_cb(
  // The channel page whose duty cycle state has changed.
  uint8_t channelPage,
  // The channel number whose duty cycle state has changed.
  uint8_t channel,
  // The current duty cycle state.
  sl_zigbee_duty_cycle_state_t state,
  uint8_t totalDevices,
  sl_zigbee_per_device_duty_cycle_t *arrayOfDeviceDutyCycles)
{
  (void)channelPage;
  (void)channel;
  (void)state;
  (void)totalDevices;
  (void)arrayOfDeviceDutyCycles;
}

// Callback fires when the duty cycle state has changed
SL_WEAK void sl_zigbee_duty_cycle_handler(
  // The channel page whose duty cycle state has changed.
  uint8_t channelPage,
  // The channel number whose duty cycle state has changed.
  uint8_t channel,
  // The current duty cycle state.
  sl_zigbee_duty_cycle_state_t state,
  uint8_t totalDevices,
  sl_zigbee_per_device_duty_cycle_t *arrayOfDeviceDutyCycles)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_duty_cycle(channelPage, channel, state, totalDevices, arrayOfDeviceDutyCycles);
  sl_zigbee_af_duty_cycle_cb(channelPage, channel, state, totalDevices, arrayOfDeviceDutyCycles);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_remote_set_binding_cb
SL_WEAK void sl_zigbee_af_remote_set_binding_cb(
  // The contents of the binding entry.
  sl_zigbee_binding_table_entry_t *entry,
  // ZDO status.
  sl_zigbee_zdo_status_t status)
{
  (void)entry;
  (void)status;
  return;
}

// Weak implementation of sli_zigbee_af_remote_set_binding_callback
SL_WEAK sl_zigbee_zdo_status_t sli_zigbee_af_remote_set_binding_callback(
  // Return: The contents of the binding entry.
  sl_zigbee_binding_table_entry_t *entry)
{
  (void)entry;
  return SL_ZIGBEE_ZDP_TABLE_FULL;
}

// The NCP used the external binding modification policy to decide how to handle
// a remote set binding request. The Host cannot change the current decision,
// but it can change the policy for future decisions using the setPolicy
// command.
// Return: ZDO response status.
SL_WEAK sl_zigbee_zdo_status_t sl_zigbee_internal_remote_set_binding_handler(
  // Return: The contents of the binding entry.
  sl_zigbee_binding_table_entry_t *entry)
{
  sl_zigbee_zdo_status_t ret;
  sl_zigbee_af_push_callback_network_index();
  ret = sli_zigbee_af_remote_set_binding_callback(entry);
  sli_zigbee_stack_remote_set_binding_handler(entry, ret);
  sl_zigbee_af_pop_network_index();
  return ret;
}

SL_WEAK void sl_zigbee_remote_set_binding_handler(
  sl_zigbee_binding_table_entry_t *entry,
  sl_zigbee_zdo_status_t status)
{
  sli_zigbee_af_remote_set_binding(entry, status);
  sl_zigbee_af_remote_set_binding_cb(entry, status);
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_remote_delete_binding_cb
SL_WEAK void sl_zigbee_af_remote_delete_binding_cb(
  // The index of the binding whose deletion was requested.
  uint8_t index,
  // ZDO response status
  sl_zigbee_zdo_status_t status)
{
  (void)index;
  (void)status;
  return;
}

// Weak implementation of sli_zigbee_af_remote_delete_binding_callback
SL_WEAK sl_zigbee_zdo_status_t sli_zigbee_af_remote_delete_binding_callback(
  // The index of the binding whose deletion was requested.
  uint8_t index)
{
  (void)index;
  return SL_ZIGBEE_ZDP_TABLE_FULL;
}

// The NCP used the external binding modification policy to decide how to handle
// a remote delete binding request. The Host cannot change the current decision,
// but it can change the policy for future decisions using the setPolicy
// command.
// Return: ZDO response status
SL_WEAK sl_zigbee_zdo_status_t sl_zigbee_internal_remote_delete_binding_handler(
  // The index of the binding whose deletion was requested.
  uint8_t index)
{
  sl_zigbee_zdo_status_t ret;
  sl_zigbee_af_push_callback_network_index();
  ret = sli_zigbee_af_remote_delete_binding_callback(index);
  sli_zigbee_stack_remote_delete_binding_handler(index, ret);
  sl_zigbee_af_pop_network_index();
  return ret;
}

SL_WEAK void sl_zigbee_remote_delete_binding_handler(
  uint8_t index,
  sl_zigbee_zdo_status_t status)
{
  sli_zigbee_af_remote_delete_binding(index, status);
  sl_zigbee_af_remote_delete_binding_cb(index, status);
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_poll_complete_cb
SL_WEAK void sl_zigbee_af_poll_complete_cb(
  // An sl_status_t value: SL_STATUS_OK - Data was received in response to
  // the poll. SL_STATUS_MAC_NO_DATA - No data was pending.
  // SL_STATUS_ZIGBEE_DELIVERY_FAILED - The poll message could not be sent.
  // SL_STATUS_MAC_NO_ACK_RECEIVED - The poll message was sent but not
  // acknowledged by the parent.
  sl_status_t status)
{
  (void)status;
}

// Indicates the result of a data poll to the parent of the local node.
SL_WEAK void sl_zigbee_poll_complete_handler(
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
// Weak implementation of public Callback sl_zigbee_af_poll_cb
SL_WEAK void sl_zigbee_af_poll_cb(
  // The node ID of the child that is requesting data.
  sl_802154_short_addr_t childId,
  // True if transmit expected, false otherwise.
  bool transmitExpected)
{
  (void)childId;
  (void)transmitExpected;
}

// Indicates that the local node received a data poll from a child.
SL_WEAK void sl_zigbee_poll_handler(
  // The node ID of the child that is requesting data.
  sl_802154_short_addr_t childId,
  // True if transmit expected, false otherwise.
  bool transmitExpected)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_poll(childId, transmitExpected);
  sl_zigbee_af_poll_cb(childId, transmitExpected);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_debug_cb
SL_WEAK void sl_zigbee_af_debug_cb(
  // debug message length
  uint8_t messageLength,
  // debug message
  uint8_t *messageContents)
{
  (void)messageLength;
  (void)messageContents;
}

// debugHandler
SL_WEAK void sl_zigbee_debug_handler(
  // debug message length
  uint8_t messageLength,
  // debug message
  uint8_t *messageContents)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_debug(messageLength, messageContents);
  sl_zigbee_af_debug_cb(messageLength, messageContents);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback
//sl_zigbee_af_incoming_many_to_one_route_request_cb
SL_WEAK void sl_zigbee_af_incoming_many_to_one_route_request_cb(
  // The short id of the concentrator.
  sl_802154_short_addr_t source,
  // The EUI64 of the concentrator.
  sl_802154_long_addr_t longId,
  // The path cost to the concentrator. The cost may decrease as additional
  // route request packets for this discovery arrive, but the callback is
  // made only once.
  uint8_t cost)
{
  (void)source;
  (void)longId;
  (void)cost;
}

// A callback indicating that a many-to-one route to the concentrator with the
// given short and long id is available for use.
SL_WEAK void sl_zigbee_incoming_many_to_one_route_request_handler(
  // The short id of the concentrator.
  sl_802154_short_addr_t source,
  // The EUI64 of the concentrator.
  sl_802154_long_addr_t longId,
  // The path cost to the concentrator. The cost may decrease as additional
  // route request packets for this discovery arrive, but the callback is
  // made only once.
  uint8_t cost)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_incoming_many_to_one_route_request(source, longId, cost);
  sl_zigbee_af_incoming_many_to_one_route_request_cb(source, longId, cost);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_incoming_route_error_cb
SL_WEAK void sl_zigbee_af_incoming_route_error_cb(
  // SL_STATUS_ZIGBEE_SOURCE_ROUTE_FAILURE or SL_STATUS_ZIGBEE_MANY_TO_ONE_ROUTE_FAILURE.
  sl_status_t status,
  // The short id of the remote node.
  sl_802154_short_addr_t target)
{
  (void)status;
  (void)target;
}

// A callback invoked when a route error message is received. The error
// indicates that a problem routing to or from the target node was encountered.
SL_WEAK void sl_zigbee_incoming_route_error_handler(
  // SL_STATUS_ZIGBEE_SOURCE_ROUTE_FAILURE or SL_STATUS_ZIGBEE_MANY_TO_ONE_ROUTE_FAILURE.
  sl_status_t status,
  // The short id of the remote node.
  sl_802154_short_addr_t target)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_incoming_route_error(status, target);
  sl_zigbee_af_incoming_route_error_cb(status, target);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_incoming_network_status_cb
SL_WEAK void sl_zigbee_af_incoming_network_status_cb(
  // One byte over-the-air error code from network status message
  uint8_t errorCode,
  // The short ID of the remote node
  sl_802154_short_addr_t target)
{
  (void)errorCode;
  (void)target;
}

// A callback invoked when a network status/route error message is received. The
// error indicates that there was a problem sending/receiving messages from the
// target node
SL_WEAK void sl_zigbee_incoming_network_status_handler(
  // One byte over-the-air error code from network status message
  uint8_t errorCode,
  // The short ID of the remote node
  sl_802154_short_addr_t target)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_incoming_network_status(errorCode, target);
  sl_zigbee_af_incoming_network_status_cb(errorCode, target);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_incoming_route_record_cb
SL_WEAK void sl_zigbee_af_incoming_route_record_cb(
  sl_zigbee_rx_packet_info_t *packetInfo,
  uint8_t relayCount,
  uint8_t *relayList)
{
  (void)packetInfo;
  (void)relayCount;
  (void)relayList;
}

// Reports the arrival of a route record command frame.
SL_WEAK void sl_zigbee_incoming_route_record_handler(sl_zigbee_rx_packet_info_t *packetInfo,
                                                     uint8_t relayCount,
                                                     uint8_t *relayList)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_incoming_route_record(packetInfo,
                                      relayCount,
                                      relayList);

  sl_zigbee_af_incoming_route_record_cb(packetInfo,
                                        relayCount,
                                        relayList);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_id_conflict_cb
SL_WEAK void sl_zigbee_af_id_conflict_cb(
  // The short id for which a conflict was detected
  sl_802154_short_addr_t conflictingId)
{
  (void)conflictingId;
}

// A callback invoked by the EmberZNet stack when an id conflict is discovered,
// that is, two different nodes in the network were found to be using the same
// short id. The stack automatically removes the conflicting short id from its
// internal tables (address, binding, route, neighbor, and child tables). The
// application should discontinue any other use of the id.
SL_WEAK void sl_zigbee_id_conflict_handler(
  // The short id for which a conflict was detected
  sl_802154_short_addr_t conflictingId)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_id_conflict(conflictingId);
  sl_zigbee_af_id_conflict_cb(conflictingId);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_mac_passthrough_message_cb
SL_WEAK void sl_zigbee_af_mac_passthrough_message_cb(
  // The type of MAC passthrough message received.
  sl_zigbee_mac_passthrough_type_t messageType,
  // RX packet info.
  sl_zigbee_rx_packet_info_t *packetInfo,
  // message length.
  uint8_t messageLength,
  // The raw message that was received.
  uint8_t *messageContents)
{
  (void)messageType;
  (void)packetInfo;
  (void)messageLength;
  (void)messageContents;
}

// A callback invoked by the EmberZNet stack when a MAC passthrough message is
// received.
SL_WEAK void sl_802154_passthrough_message_handler(
  // The type of MAC passthrough message received.
  sl_zigbee_mac_passthrough_type_t messageType,
  sl_zigbee_rx_packet_info_t *packetInfo,
  uint8_t messageLength,
  // The raw message that was received.
  uint8_t *messageContents)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_mac_passthrough_message(messageType, packetInfo, messageLength, messageContents);
  sl_zigbee_af_mac_passthrough_message_cb(messageType, packetInfo, messageLength, messageContents);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_stack_token_changed_cb
SL_WEAK void sl_zigbee_af_stack_token_changed_cb(
  // The address of the stack token that has changed.
  uint16_t tokenAddress)
{
  (void)tokenAddress;
}

// A callback invoked to inform the application that a stack token has changed.
SL_WEAK void sl_zigbee_stack_token_changed_handler(
  // The address of the stack token that has changed.
  uint16_t tokenAddress)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_stack_token_changed(tokenAddress);
  sl_zigbee_af_stack_token_changed_cb(tokenAddress);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_timer_cb
SL_WEAK void sl_zigbee_af_timer_cb(
  // Which timer generated the callback (0 or 1).
  uint8_t timerId)
{
  (void)timerId;
}

// A callback from the timer.
SL_WEAK void sl_zigbee_timer_handler(
  // Which timer generated the callback (0 or 1).
  uint8_t timerId)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_timer(timerId);
  sl_zigbee_af_timer_cb(timerId);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_counter_rollover_cb
SL_WEAK void sl_zigbee_af_counter_rollover_cb(
  // Type of Counter
  sl_zigbee_counter_type_t type)
{
  (void)type;
}

// This call is fired when a counter exceeds its threshold
SL_WEAK void sl_zigbee_counter_rollover_handler(
  // Type of Counter
  sl_zigbee_counter_type_t type)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_counter_rollover(type);
  sl_zigbee_af_counter_rollover_cb(type);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_raw_transmit_complete_cb
SL_WEAK void sl_zigbee_af_raw_transmit_complete_cb(
  // message
  uint8_t  messageLength,
  uint8_t* messageContents,
  // SL_STATUS_OK if the transmission was successful, or
  // SL_STATUS_ZIGBEE_DELIVERY_FAILED if not
  sl_status_t status)
{
  (void)messageLength;
  (void)messageContents;
  (void)status;
}

// A callback invoked by the EmberZNet stack when the MAC has finished
// transmitting a raw message.
SL_WEAK void sl_zigbee_raw_transmit_complete_handler(
  // message
  uint8_t  messageLength,
  uint8_t* messageContents,
  // SL_STATUS_OK if the transmission was successful, or
  // SL_STATUS_ZIGBEE_DELIVERY_FAILED if not
  sl_status_t status,
  uint8_t messageTag)
{
  (void)messageTag;
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_raw_transmit_complete(messageLength, messageContents, status);
  sl_zigbee_af_raw_transmit_complete_cb(messageLength, messageContents, status);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_switch_network_key_cb
SL_WEAK void sl_zigbee_af_switch_network_key_cb(
  // The sequence number of the new network key.
  uint8_t sequenceNumber)
{
  (void)sequenceNumber;
}

// A callback to inform the application that the Network Key has been updated
// and the node has been switched over to use the new key. The actual key being
// used is not passed up, but the sequence number is.
SL_WEAK void sl_zigbee_switch_network_key_handler(
  // The sequence number of the new network key.
  uint8_t sequenceNumber)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_switch_network_key(sequenceNumber);
  sl_zigbee_af_switch_network_key_cb(sequenceNumber);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_zigbee_key_establishment_cb
SL_WEAK void sl_zigbee_af_zigbee_key_establishment_cb(
  // This is the IEEE address of the partner that the device successfully
  // established a key with. This value is all zeros on a failure.
  sl_802154_long_addr_t partner,
  // This is the status indicating what was established or why the key
  // establishment failed.
  sl_zigbee_key_status_t status)
{
  (void)partner;
  (void)status;
}

// This is a callback that indicates the success or failure of an attempt to
// establish a key with a partner device.
SL_WEAK void sl_zigbee_zigbee_key_establishment_handler(
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

// Weak implementation of public Callback sl_zigbee_af_mux_invalid_rx_cb
SL_WEAK void sl_zigbee_af_mux_invalid_rx_cb(uint8_t new_rx_channel, uint8_t old_rx_channel)
{
  (void)new_rx_channel;
  (void)old_rx_channel;
}

SL_WEAK void sl_zigbee_mux_invalid_rx_handler(uint8_t new_rx_channel, uint8_t old_rx_channel)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_mux_invalid_rx(new_rx_channel, old_rx_channel);
  sl_zigbee_af_mux_invalid_rx_cb(new_rx_channel, old_rx_channel);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_generate_cbke_keys_cb
SL_WEAK void sl_zigbee_af_generate_cbke_keys_cb(
  // The result of the CBKE operation.
  sl_status_t status,
  // Return: The generated ephemeral public key.
  sl_zigbee_public_key_data_t *ephemeralPublicKey)
{
  (void)status;
  (void)ephemeralPublicKey;
}

// A callback by the Crypto Engine indicating that a new ephemeral
// public/private key pair has been generated. The public/private key pair is
// stored on the NCP, but only the associated public key is returned to the
// host. The node's associated certificate is also returned.
SL_WEAK void sl_zigbee_generate_cbke_keys_handler(
  // The result of the CBKE operation.
  sl_status_t status,
  // Return: The generated ephemeral public key.
  sl_zigbee_public_key_data_t *ephemeralPublicKey)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_generate_cbke_keys(status, ephemeralPublicKey);
  sl_zigbee_af_generate_cbke_keys_cb(status, ephemeralPublicKey);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_calculate_smacs_cb
SL_WEAK void sl_zigbee_af_calculate_smacs_cb(
  // The Result of the CBKE operation.
  sl_status_t status,
  // Return: The calculated value of the initiator's SMAC
  sl_zigbee_smac_data_t *initiatorSmac,
  // Return: The calculated value of the responder's SMAC
  sl_zigbee_smac_data_t *responderSmac)
{
  (void)status;
  (void)initiatorSmac;
  (void)responderSmac;
}

// A callback to indicate that the NCP has finished calculating the Secure
// Message Authentication Codes (SMAC) for both the initiator and responder. The
// associated link key is kept in temporary storage until the host tells the NCP
// to store or discard the key via sl_zigbee_clear_temporary_data_maybe_store_link_key().
SL_WEAK void sl_zigbee_calculate_smacs_handler(
  // The Result of the CBKE operation.
  sl_status_t status,
  // Return: The calculated value of the initiator's SMAC
  sl_zigbee_smac_data_t *initiatorSmac,
  // Return: The calculated value of the responder's SMAC
  sl_zigbee_smac_data_t *responderSmac)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_calculate_smacs(status, initiatorSmac, responderSmac);
  sl_zigbee_af_calculate_smacs_cb(status, initiatorSmac, responderSmac);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_dsa_sign_cb
SL_WEAK void sl_zigbee_af_dsa_sign_cb(
  // The result of the DSA signing operation.
  sl_status_t status,
  // The length of message.
  uint8_t messageLength,
  // The message and attached which includes the original message and the
  // appended signature.
  uint8_t *messageContents)
{
  (void)status;
  (void)messageLength;
  (void)messageContents;
}

// The handler that returns the results of the signing operation. On success,
// the signature will be appended to the original message (including the
// signature type indicator that replaced the startIndex field for the signing)
// and both are returned via this callback.
SL_WEAK void sl_zigbee_dsa_sign_handler(
  // The result of the DSA signing operation.
  sl_status_t status,
  // The length of message
  uint8_t messageLength,
  // The message and attached which includes the original message and the
  // appended signature.
  uint8_t *messageContents)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_dsa_sign(status, messageLength, messageContents);
  sl_zigbee_af_dsa_sign_cb(status, messageLength, messageContents);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_dsa_verify_cb
SL_WEAK void sl_zigbee_af_dsa_verify_cb(
  // The result of the DSA verification operation.
  sl_status_t status)
{
  (void)status;
}

// This callback is executed by the stack when the DSA verification has
// completed and has a result. If the result is SL_STATUS_OK, the signature is
// valid. If the result is SL_STATUS_ZIGBEE_SIGNATURE_VERIFY_FAILURE then the signature is
// invalid. If the result is anything else then the signature verify operation
// failed and the validity is unknown.
SL_WEAK void sl_zigbee_dsa_verify_handler(
  // The result of the DSA verification operation.
  sl_status_t status)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_dsa_verify(status);
  sl_zigbee_af_dsa_verify_cb(status);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_incoming_bootload_message_cb
SL_WEAK void sl_zigbee_af_incoming_bootload_message_cb(
  // The EUI64 of the sending node.
  sl_802154_long_addr_t longId,
  // RX packet info.
  sl_zigbee_rx_packet_info_t *packetInfo,
  // message length.
  uint8_t messageLength,
  // The bootload message that was sent.
  uint8_t *messageContents)
{
  (void)longId;
  (void)packetInfo;
  (void)messageLength;
  (void)messageContents;
}

// A callback invoked by the EmberZNet stack when a bootload message is
// received.
SL_WEAK void sl_zigbee_incoming_bootload_message_handler(
  // The EUI64 of the sending node.
  sl_802154_long_addr_t longId,
  // RX packet info.
  sl_zigbee_rx_packet_info_t *packetInfo,
  // message length.
  uint8_t messageLength,
  // The bootload message that was sent.
  uint8_t *messageContents)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_incoming_bootload_message(longId, packetInfo, messageLength, messageContents);
  sl_zigbee_af_incoming_bootload_message_cb(longId, packetInfo, messageLength, messageContents);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback
//sl_zigbee_af_bootload_transmit_complete_cb
SL_WEAK void sl_zigbee_af_bootload_transmit_complete_cb(
  // An sl_status_t value of SL_STATUS_OK if an ACK was received from the
  // destination or SL_STATUS_ZIGBEE_DELIVERY_FAILED if no ACK was received.
  sl_status_t status,
  // message length.
  uint8_t messageLength,
  // The bootload message that was sent.
  uint8_t *messageContents)
{
  (void)status;
  (void)messageLength;
  (void)messageContents;
}

// A callback invoked by the EmberZNet stack when the MAC has finished
// transmitting a bootload message.
SL_WEAK void sl_zigbee_bootload_transmit_complete_handler(
  // An sl_status_t value of SL_STATUS_OK if an ACK was received from the
  // destination or SL_STATUS_ZIGBEE_DELIVERY_FAILED if no ACK was received.
  sl_status_t status,
  // message length.
  uint8_t messageLength,
  // The bootload message that was sent.
  uint8_t *messageContents)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_bootload_transmit_complete(status, messageLength, messageContents);
  sl_zigbee_af_bootload_transmit_complete_cb(status, messageLength, messageContents);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_zll_network_found_cb
SL_WEAK void sl_zigbee_af_zll_network_found_cb(
  // Used to interpret deviceInfo field.
  bool isDeviceInfoNull,
  // Information about the received packet.
  sl_zigbee_rx_packet_info_t *packetInfo,
  // Return: Information about the network.
  const sl_zigbee_zll_network_t *networkInfo,
  // Return: Device specific information.
  const sl_zigbee_zll_device_info_record_t *deviceInfo)
{
  (void)networkInfo;
  (void)isDeviceInfoNull;
  (void)deviceInfo;
  (void)packetInfo;
}

// This call is fired when a ZLL network scan finds a ZLL network.
SL_WEAK void sl_zigbee_zll_network_found_handler(
  // Return: Information about the network.
  sl_zigbee_zll_network_t *networkInfo,
  bool isDeviceInfoNull,
  // Return: Device specific information.
  sl_zigbee_zll_device_info_record_t *deviceInfo,
  // Information about the received packet.
  sl_zigbee_rx_packet_info_t *packetInfo)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_zll_network_found(networkInfo, isDeviceInfoNull, deviceInfo, packetInfo);
  sl_zigbee_af_zll_network_found_cb(isDeviceInfoNull, packetInfo, networkInfo, deviceInfo);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_zll_scan_complete_cb
SL_WEAK void sl_zigbee_af_zll_scan_complete_cb(
  // Status of the operation.
  sl_status_t status)
{
  (void)status;
}

// This call is fired when a ZLL network scan is complete.
SL_WEAK void sl_zigbee_zll_scan_complete_handler(
  // Status of the operation.
  sl_status_t status)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_zll_scan_complete(status);
  sl_zigbee_af_zll_scan_complete_cb(status);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_zll_address_assignment_cb
SL_WEAK void sl_zigbee_af_zll_address_assignment_cb(
  // Return: Address assignment information.
  const sl_zigbee_zll_address_assignment_t *addressInfo,
  // Information about the received packet.
  sl_zigbee_rx_packet_info_t *packetInfo)
{
  (void)packetInfo;
  (void)addressInfo;
}

// This call is fired when network and group addresses are assigned to a remote
// mode in a network start or network join request.
SL_WEAK void sl_zigbee_zll_address_assignment_handler(
  // Return: Address assignment information.
  sl_zigbee_zll_address_assignment_t *addressInfo,
  // Information about the received packet.
  sl_zigbee_rx_packet_info_t *packetInfo)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_zll_address_assignment(addressInfo, packetInfo);
  sl_zigbee_af_zll_address_assignment_cb(addressInfo, packetInfo);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_zll_touch_link_target_cb
SL_WEAK void sl_zigbee_af_zll_touch_link_target_cb(
  // Return: Information about the network.
  const sl_zigbee_zll_network_t *networkInfo)
{
  (void)networkInfo;
}

// This call is fired when the device is a target of a touch link.
SL_WEAK void sl_zigbee_zll_touch_link_target_handler(
  // Return: Information about the network.
  const sl_zigbee_zll_network_t *networkInfo)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_zll_touch_link_target(networkInfo);
  sl_zigbee_af_zll_touch_link_target_cb(networkInfo);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_mac_filter_match_message_cb
SL_WEAK void sl_zigbee_af_mac_filter_match_message_cb(
  // filter value match.
  sl_zigbee_mac_filter_match_data_t filterValueMatch,
  // message type.
  sl_zigbee_mac_passthrough_type_t messageType,
  // RX packet info.
  sl_zigbee_rx_packet_info_t *packetInfo,
  // message length.
  uint8_t messageLength,
  // message contents.
  uint8_t *messageContents)
{
  (void)filterValueMatch;
  (void)messageType;
  (void)packetInfo;
  (void)messageLength;
  (void)messageContents;
}

// A callback invoked by the EmberZNet stack when a raw MAC message that has
// matched one of the application's configured MAC filters.
SL_WEAK void sl_802154_filter_match_message_handler(
  // filter value match.
  sl_zigbee_mac_filter_match_data_t filterValueMatch,
  // message type.
  sl_zigbee_mac_passthrough_type_t messageType,
  // RX packet info.
  sl_zigbee_rx_packet_info_t *packetInfo,
  // message length.
  uint8_t messageLength,
  // message contents.
  uint8_t *messageContents)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_mac_filter_match_message(filterValueMatch,
                                         messageType,
                                         packetInfo,
                                         messageLength,
                                         messageContents);
  sl_zigbee_af_mac_filter_match_message_cb(filterValueMatch,
                                           messageType,
                                           packetInfo,
                                           messageLength,
                                           messageContents);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_d_gp_sent_cb
SL_WEAK void sl_zigbee_af_d_gp_sent_cb(
  // An sl_status_t value indicating success or the reason for failure.
  sl_status_t status,
  // The handle of the GPDF.
  uint8_t gpepHandle)
{
  (void)status;
  (void)gpepHandle;
}

// A callback to the GP endpoint to indicate the result of the GPDF
// transmission.
SL_WEAK void sli_zigbee_stack_d_gp_sent_handler(
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
// Weak implementation of public Callback sl_zigbee_af_pan_id_conflict_cb
SL_WEAK sl_status_t sl_zigbee_af_pan_id_conflict_cb(
  // Number of conflict reports
  int8_t conflictCount)
{
  (void)conflictCount;
  return SL_STATUS_NOT_AVAILABLE;
}

// Weak implementation of sli_zigbee_af_pan_id_conflict_callback
SL_WEAK sl_status_t sli_zigbee_af_pan_id_conflict_callback(
  // Number of conflict reports
  int8_t conflictCount)
{
  (void)conflictCount;
  return SL_STATUS_NOT_AVAILABLE;
}

// This handler is called by the stack to report the number of conflict reports
// exceeds SL_ZIGBEE_PAN_ID_CONFLICT_REPORT_THRESHOLD within a period of 1 minute )
// Return: An sl_status_t value indicating success or the reason for failure.
SL_WEAK sl_status_t sl_zigbee_internal_pan_id_conflict_handler(
  // Number of conflict reports
  int8_t conflictCount)
{
  sl_status_t ret;
  sl_zigbee_af_push_callback_network_index();
  ret = sli_zigbee_af_pan_id_conflict_callback(conflictCount);
  sli_zigbee_af_pan_id_conflict(conflictCount);
  sl_zigbee_af_pan_id_conflict_cb(conflictCount);
  sl_zigbee_af_pop_network_index();
  return ret;
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_orphan_notification_cb
SL_WEAK void sl_zigbee_af_orphan_notification_cb(
  // The 8 byte EUI64 of the sender.
  sl_802154_long_addr_t longId)
{
  (void)longId;
}

// Inform the application that an orphan notification has been received. This is
// generally not useful for applications. It could be useful in testing and is
// included for this purpose.
SL_WEAK void sl_zigbee_orphan_notification_handler(
  // The 8 byte EUI64 of the sender.
  sl_802154_long_addr_t longId)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_orphan_notification(longId);
  sl_zigbee_af_orphan_notification_cb(longId);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_mac_passthrough_filter_cb
SL_WEAK bool sl_zigbee_af_mac_passthrough_filter_cb(
  // Return: Mac Header of the matched messgae
  uint8_t *macHeader)
{
  (void)macHeader;
  return false;
}

// Weak implementation of sli_zigbee_af_mac_passthrough_filter_callback
SL_WEAK bool sli_zigbee_af_mac_passthrough_filter_callback(
  // Return: Mac Header of the matched messgae
  uint8_t *macHeader)
{
  (void)macHeader;
  return false;
}

// This handler is called for messages that need to be matched against mac
// passthrough filter
// Return: True if mac passthrough filter was matched. False otherwise.
SL_WEAK bool sl_802154_internal_passthrough_filter_handler(
  // Return: Mac Header of the matched messgae
  uint8_t *macHeader)
{
  bool ret;
  sl_zigbee_af_push_callback_network_index();
  ret = sli_zigbee_af_mac_passthrough_filter_callback(macHeader);
  sli_zigbee_af_mac_passthrough_filter(macHeader);
  sl_zigbee_af_mac_passthrough_filter_cb(macHeader);
  sl_zigbee_af_pop_network_index();
  return ret;
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback
//sl_zigbee_af_generate_cbke_keys_283k1_handler_cb
SL_WEAK void sl_zigbee_af_generate_cbke_keys_283k1_handler_cb(
  // The result of the CBKE operation.
  sl_status_t status,
  // Return: The generated ephemeral public key.
  sl_zigbee_public_key_283k1_data_t *ephemeralPublicKey)
{
  (void)status;
  (void)ephemeralPublicKey;
}

// A callback by the Crypto Engine indicating that a new ephemeral
// public/private key pair has been generated. The public/private key pair is
// stored on the NCP, but only the associated public key is returned to the
// host. The node's associated certificate is also returned.
SL_WEAK void sl_zigbee_generate_cbke_keys_283k1_handler(
  // The result of the CBKE operation.
  sl_status_t status,
  // Return: The generated ephemeral public key.
  sl_zigbee_public_key_283k1_data_t *ephemeralPublicKey)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_generate_cbke_keys_handler283k1(status, ephemeralPublicKey);
  sl_zigbee_af_generate_cbke_keys_283k1_handler_cb(status, ephemeralPublicKey);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback
//sl_zigbee_af_calculate_smacs_283k1_handler_cb
SL_WEAK void sl_zigbee_af_calculate_smacs_283k1_handler_cb(
  // The Result of the CBKE operation.
  sl_status_t status,
  // Return: The calculated value of the initiator's SMAC
  sl_zigbee_smac_data_t *initiatorSmac,
  // Return: The calculated value of the responder's SMAC
  sl_zigbee_smac_data_t *responderSmac)
{
  (void)status;
  (void)initiatorSmac;
  (void)responderSmac;
}

// A callback to indicate that the NCP has finished calculating the Secure
// Message Authentication Codes (SMAC) for both the initiator and responder. The
// associated link key is kept in temporary storage until the host tells the NCP
// to store or discard the key via sl_zigbee_clear_temporary_data_maybe_store_link_key().
SL_WEAK void sl_zigbee_calculate_smacs_283k1_handler(
  // The Result of the CBKE operation.
  sl_status_t status,
  // Return: The calculated value of the initiator's SMAC
  sl_zigbee_smac_data_t *initiatorSmac,
  // Return: The calculated value of the responder's SMAC
  sl_zigbee_smac_data_t *responderSmac)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_calculate_smacs_283k1_handler(status, initiatorSmac, responderSmac);
  sl_zigbee_af_calculate_smacs_283k1_handler_cb(status, initiatorSmac, responderSmac);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_gpep_incoming_message_cb
SL_WEAK void sl_zigbee_af_gpep_incoming_message_cb(
  // GP parameters list represented as a macro for GP endpoint incoming message handler and callbacks prototypes.
  sl_zigbee_gp_params_t *params)
{
  (void)params;
}

// A callback invoked by the ZigBee GP stack when a GPDF is received.
SL_WEAK void sl_zigbee_gpep_incoming_message_handler(
  // GP parameters list represented as a macro for GP endpoint incoming message handler and callbacks prototypes.
  sl_zigbee_gp_params_t *params)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_gpep_incoming_message(params);
  sl_zigbee_af_gpep_incoming_message_cb(params);
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of sli_zigbee_af_rtos_idle_callback
SL_WEAK bool sli_zigbee_af_rtos_idle_callback(
  // Return: Idle time duration
  uint32_t *idleTimeMs)
{
  (void)idleTimeMs;
  return false;
}

// rtos Idle Handler
// Return: True or False.
SL_WEAK bool sl_zigbee_rtos_idle_handler(
  // Return: Idle time duration
  uint32_t *idleTimeMs)
{
  bool ret;
  sl_zigbee_af_push_callback_network_index();
  ret = sli_zigbee_af_rtos_idle_callback(idleTimeMs);
  sli_zigbee_af_rtos_idle(idleTimeMs);
  sl_zigbee_af_pop_network_index();
  return ret;
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_rtos_stack_wakeup_isr_cb
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_ZIGBEE_STACK, SL_CODE_CLASS_TIME_CRITICAL)
SL_WEAK void sl_zigbee_af_rtos_stack_wakeup_isr_cb(void)
{
}

// Rtos Stack Wakeup Isr Handler
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_ZIGBEE_STACK, SL_CODE_CLASS_TIME_CRITICAL)
SL_WEAK void sl_zigbee_rtos_stack_wakeup_isr_handler(void)
{
  sl_zigbee_af_push_callback_network_index();
  sl_zigbee_af_rtos_stack_wakeup_isr_cb();
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_radio_needs_calibrating_cb
SL_WEAK void sl_zigbee_af_radio_needs_calibrating_cb(void)
{
}

// RadioNeedsCalibratingHandler
SL_WEAK void sl_zigbee_radio_needs_calibrating_handler(void)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_radio_needs_calibrating();
  sl_zigbee_af_radio_needs_calibrating_cb();
  sl_zigbee_af_pop_network_index();
}

// -----------------------------------------------------------------------------
// Weak implementation of public Callback sl_zigbee_af_scan_error_cb
SL_WEAK void sl_zigbee_af_scan_error_cb(
  // The error status of a scan
  sl_status_t status)
{
  (void)status;
}

// scanErrorHandler
SL_WEAK void sl_zigbee_scan_error_handler(
  // The error status of a scan
  sl_status_t status)
{
  sl_zigbee_af_push_callback_network_index();
  sli_zigbee_af_scan_error(status);
  sl_zigbee_af_scan_error_cb(status);
  sl_zigbee_af_pop_network_index();
}

#endif

// The following function is called for a Multi-MAC end device to allow the user configuration of
// the device to update channel mask for the scenario where a rejoin is initated due to end device
// timeout event. Based on the Multi-MAC end device configuration this call is expected to update
// the mask so that the rejoin interface will be selected. As per the SE 1.4 errata, the
// "Multi-MAC Selection" end device can change the rejoin interface (this is default) where as the
// Multi-MAC Joining end devices shall not, hence supply the channel mask based on the joined interface.
extern uint8_t sl_zigbee_get_logical_channel(void);
SL_WEAK uint32_t sl_zigbee_internal_update_multi_mac_rejoin_channel_mask_for_selection_or_joining_device_handler(uint32_t rejoinChannelMask)
{
#ifdef SL_CATALOG_ZIGBEE_PHY_2_4_SUBGHZ_JOINING_END_DEVICE_PRESENT
  (void) rejoinChannelMask;
  return PG_CH_BITMASK32(sl_zigbee_get_logical_channel());
#else
  return rejoinChannelMask;
#endif
}

SL_WEAK void sli_zigbee_stack_override_incoming_route_record_handler(sl_zigbee_rx_packet_info_t *packetInfo,
                                                                     uint8_t relayCount,
                                                                     uint8_t *relayList,
                                                                     bool *consumed)
{
  (void)packetInfo;
  (void)relayCount;
  (void)relayList;
  (void)consumed;
}

SL_WEAK void sl_zigbee_post_incoming_packet_filter_cb(sl_zigbee_zigbee_packet_type_t packetType,
                                                      uint8_t* packetData,
                                                      uint8_t size_p,
                                                      uint8_t *data,
                                                      uint8_t size_d,
                                                      sl_zigbee_packet_action_t action)
{
  UNUSED_VAR(packetType);
  UNUSED_VAR(packetData);
  UNUSED_VAR(size_p);
  UNUSED_VAR(data);
  UNUSED_VAR(size_d);
  UNUSED_VAR(action);
}

SL_WEAK void sl_zigbee_post_outgoing_packet_filter_cb(sl_zigbee_zigbee_packet_type_t packetType,
                                                      uint8_t* packetData,
                                                      uint8_t size_p,
                                                      uint8_t *data,
                                                      uint8_t size_d,
                                                      sl_zigbee_packet_action_t action)
{
  UNUSED_VAR(packetType);
  UNUSED_VAR(packetData);
  UNUSED_VAR(size_p);
  UNUSED_VAR(data);
  UNUSED_VAR(size_d);
  UNUSED_VAR(action);
}
