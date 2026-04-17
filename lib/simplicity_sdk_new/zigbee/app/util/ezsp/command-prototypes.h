/*****************************************************************************/
/**
 * Copyright 2017 Silicon Laboratories, Inc.
 *
 *****************************************************************************/
//
// *** Generated file. Do not edit! ***
//
// Description: Function prototypes for sending every EM260 frame and returning
// the result to the Host.

#include "stack/include/library.h"
// RAIL tests define COMPONENT_CATALOG, but don't actually generate zap, so we
// key off of RAIL-defined SIMULATION_DEVICE to avoid pulling in zap-type.h
#if defined(SL_COMPONENT_CATALOG_PRESENT) && !defined(SIMULATION_DEVICE)
 #include "zap-type.h"
#else // SL_COMPONENT_CATALOG_PRESENT
 #include "app/framework/test/headers/zap-type.h"
#endif // SL_COMPONENT_CATALOG_PRESENT
#include "stack/include/sl_zigbee_dhc.h"


//------------------------------------------------------------------------------
// Configuration Frames
//------------------------------------------------------------------------------

// The command allows the Host to specify the desired EZSP version and must be
// sent before any other command. The response provides information about the
// firmware running on the NCP.
// Return: The EZSP version the NCP is using.
uint8_t sl_zigbee_ezsp_version(
      // The EZSP version the Host wishes to use. To successfully set the
      // version and allow other commands, this must be same as
      // EZSP_PROTOCOL_VERSION.
      uint8_t desiredProtocolVersion,
      // Return: The type of stack running on the NCP (2).
      uint8_t *stackType,
      // Return: The version number of the stack.
      uint16_t *stackVersion);

// Reads a configuration value from the NCP.
// Return: SL_STATUS_OK if the value was read successfully,
// SL_STATUS_ZIGBEE_EZSP_ERROR if send command failed.
// SL_STATUS_INVALID_PARAMETER is returned if the NCP does not recognize
// configId.
sl_status_t sl_zigbee_ezsp_get_configuration_value(
      // Identifies which configuration value to read.
      sl_zigbee_ezsp_config_id_t configId,
      // Return: The configuration value.
      uint16_t *value);

// Writes a configuration value to the NCP. Configuration values can be modified
// by the Host after the NCP has reset. Once the status of the stack changes to
// SL_STATUS_NETWORK_UP, configuration values can no longer be modified and this
// command will respond with SL_ZIGBEE_EZSP_ERROR_INVALID_CALL.
// Return: SL_STATUS_OK if the configuration value was changed,
// SL_STATUS_ZIGBEE_EZSP_ERROR if send command failed. Possible error codes
// include SL_STATUS_INVALID_STATE if the stack is not in a state where
// configuration values can be changed, and SL_STATUS_INVALID_PARAMETER if the
// new value is not valid.
sl_status_t sl_zigbee_ezsp_set_configuration_value(
      // Identifies which configuration value to change.
      sl_zigbee_ezsp_config_id_t configId,
      // The new configuration value.
      uint16_t value);

// Read attribute data on NCP endpoints.
// Return: An sl_zigbee_af_status_t value indicating success or the reason for
// failure, handled by the EZSP layer as a uint8_t. 255 indicates an
// EZSP-specific error.
sl_zigbee_af_status_t sl_zigbee_ezsp_read_attribute(
      // Endpoint
      uint8_t endpoint,
      // Cluster.
      uint16_t cluster,
      // Attribute ID.
      uint16_t attributeId,
      // Mask.
      uint8_t mask,
      // Manufacturer code.
      uint16_t manufacturerCode,
      // Return: Attribute data type.
      uint8_t *dataType,
      // Return: Length of attribute data.
      uint8_t *readLength,
      // Return: Attribute data.
      uint8_t *dataPtr);

// Write attribute data on NCP endpoints.
// Return: An sl_zigbee_af_status_t value indicating success or the reason for
// failure.
sl_zigbee_af_status_t sl_zigbee_ezsp_write_attribute(
      // Endpoint
      uint8_t endpoint,
      // Cluster.
      uint16_t cluster,
      // Attribute ID.
      uint16_t attributeId,
      // Mask.
      uint8_t mask,
      // Manufacturer code.
      uint16_t manufacturerCode,
      // Override read only and data type.
      bool overrideReadOnlyAndDataType,
      // Override read only and data type.
      bool justTest,
      // Attribute data type.
      uint8_t dataType,
      // Attribute data length.
      uint8_t dataLength,
      // Attribute data.
      uint8_t *data);

// Configures endpoint information on the NCP. The NCP does not remember these
// settings after a reset. Endpoints can be added by the Host after the NCP has
// reset. Once the status of the stack changes to SL_STATUS_NETWORK_UP,
// endpoints can no longer be added and this command will respond with
// SL_ZIGBEE_EZSP_ERROR_INVALID_CALL.
// Return: SL_STATUS_OK if the endpoint was added, SL_STATUS_ZIGBEE_EZSP_ERROR
// if send command failed. SL_STATUS_ALLOCATION_FAILED is returned if there is
// not enough memory available to add the endpoint, SL_STATUS_INVALID_PARAMETER
// if the endpoint already exists.
sl_status_t sl_zigbee_ezsp_add_endpoint(
      // The application endpoint to be added.
      uint8_t endpoint,
      // The endpoint's application profile.
      uint16_t profileId,
      // The endpoint's device ID within the application profile.
      uint16_t deviceId,
      // The endpoint's device version.
      uint8_t deviceVersion,
      // The number of cluster IDs in inputClusterList.
      uint8_t inputClusterCount,
      // The number of cluster IDs in outputClusterList.
      uint8_t outputClusterCount,
      // Input cluster IDs the endpoint will accept.
      uint16_t *inputClusterList,
      // Output cluster IDs the endpoint may send.
      uint16_t *outputClusterList);

// Allows the Host to change the policies used by the NCP to make fast
// decisions.
// Return: SL_STATUS_OK if the policy was changed, SL_STATUS_ZIGBEE_EZSP_ERROR
// if send command failed. SL_STATUS_INVALID_PARAMETER is returned if the NCP
// does not recognize policyId.
sl_status_t sl_zigbee_ezsp_set_policy(
      // Identifies which policy to modify.
      sl_zigbee_ezsp_policy_id_t policyId,
      // The new decision for the specified policy.
      sl_zigbee_ezsp_decision_id_t decisionId);

// Allows the Host to read the policies used by the NCP to make fast decisions.
// Return: SL_STATUS_OK if the policy was read successfully,
// SL_STATUS_ZIGBEE_EZSP_ERROR if send command failed.
// SL_STATUS_INVALID_PARAMETER is returned if the NCP does not recognize
// policyId.
sl_status_t sl_zigbee_ezsp_get_policy(
      // Identifies which policy to read.
      sl_zigbee_ezsp_policy_id_t policyId,
      // Return: The current decision for the specified policy.
      sl_zigbee_ezsp_decision_id_t *decisionId);

// Triggers a pan id update message.
// Return: true if the request was successfully handed to the stack, false
// otherwise
bool sl_zigbee_ezsp_send_pan_id_update(
      // The new Pan Id
      sl_802154_pan_id_t newPan);

// Reads a value from the NCP.
// Return: SL_STATUS_OK if the value was read successfully,
// SL_STATUS_ZIGBEE_EZSP_ERROR if send command failed.
// SL_STATUS_INVALID_PARAMETER is returned if the NCP does not recognize
// valueId.
sl_status_t sl_zigbee_ezsp_get_value(
      // Identifies which value to read.
      sl_zigbee_ezsp_value_id_t valueId,
      // Return: Both a command and response parameter. On command, the maximum
      // size in bytes of local storage allocated to receive the returned value.
      // On response, the actual length in bytes of the returned value.
      uint8_t *valueLength,
      // Return: The value.
      uint8_t *value);

// Reads a value from the NCP but passes an extra argument specific to the value
// being retrieved.
// Return: SL_STATUS_OK if the value was read successfully,
// SL_STATUS_ZIGBEE_EZSP_ERROR if send command failed.
// SL_STATUS_INVALID_PARAMETER is returned if the NCP does not recognize
// valueId.
sl_status_t sl_zigbee_ezsp_get_extended_value(
      // Identifies which extended value ID to read.
      sl_zigbee_ezsp_extended_value_id_t valueId,
      // Identifies which characteristics of the extended value ID to read.
      // These are specific to the value being read.
      uint32_t characteristics,
      // Return: Both a command and response parameter. On command, the maximum
      // size in bytes of local storage allocated to receive the returned value.
      // On response, the actual length in bytes of the returned value.
      uint8_t *valueLength,
      // Return: The value.
      uint8_t *value);

// Writes a value to the NCP.
// Return: SL_STATUS_OK if the value was changed, SL_STATUS_ZIGBEE_EZSP_ERROR if
// send command failed. SL_STATUS_INVALID_PARAMETER is returned if the NCP does
// not recognize valueId.
sl_status_t sl_zigbee_ezsp_set_value(
      // Identifies which value to change.
      sl_zigbee_ezsp_value_id_t valueId,
      // The length of the value parameter in bytes.
      uint8_t valueLength,
      // The new value.
      uint8_t *value);

// Allows the Host to control the broadcast behaviour of a routing device used
// by the NCP.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_set_passive_ack_config(
      // Passive ack config enum.
      uint8_t config,
      // The minimum number of acknowledgments (re-broadcasts) to wait for until
      // deeming the broadcast transmission complete.
      uint8_t minAcksNeeded);

// Set the PAN ID to be accepted by the device in a NLME Network Update command.
//  If this is set to a different value than its default 0xFFFF, NLME network
// update messages will be ignored if they do not match this PAN ID.
void sl_zigbee_ezsp_set_pending_network_update_pan_id(
      // PAN ID to be accepted in a network update.
      uint16_t panId);

// Set the channel number the device will accept in ZDO Mgmt Network Update
// command to change channel. If a ZDO Mgmt Network Update command is received
// by the device specifying a channel that does not match with the given
// channel, the ZDO Mgmt Network Update command will be ignored by the device. A
// value of 0xFF indicates that any channel received in a ZDO Mgmt Network
// Update command will be accepted which is also the default value set by the
// stack.
void sl_zigbee_ezsp_set_pending_network_update_channel(
      // A pending network update channel number.
      uint8_t channel);

// Retrieve the endpoint number located at the specified index.
// Return: Endpoint number at the index.
uint8_t sl_zigbee_ezsp_get_endpoint(
      // Index to retrieve the endpoint number for.
      uint8_t index);

// Get the number of configured endpoints.
// Return: Number of configured endpoints.
uint8_t sl_zigbee_ezsp_get_endpoint_count(void);

// Retrieve the endpoint description for the given endpoint number.
// Return: true if the endpoint description was retrieved successfully, false
// otherwise.
bool sl_zigbee_ezsp_get_endpoint_description(
      // Endpoint number to get the description of.
      uint8_t endpoint,
      // Return: Description of this endpoint.
      sl_zigbee_endpoint_description_t *result);

// Retrieve one of the cluster IDs associated with the given endpoint.
// Return: ID of the requested cluster.
uint16_t sl_zigbee_ezsp_get_endpoint_cluster(
      // Endpoint number to get a cluster ID for.
      uint8_t endpoint,
      // Which list to get the cluster ID from.  (0 for input, 1 for output).
      uint8_t listId,
      // Index from requested list to look at the cluster ID of.
      uint8_t listIndex);

//------------------------------------------------------------------------------
// Utilities Frames
//------------------------------------------------------------------------------

// A command which does nothing. The Host can use this to set the sleep mode or
// to check the status of the NCP.
void sl_zigbee_ezsp_nop(void);

// Variable length data from the Host is echoed back by the NCP. This command
// has no other effects and is designed for testing the link between the Host
// and NCP.
// Return: The length of the echo parameter in bytes.
uint8_t sl_zigbee_ezsp_echo(
      // The length of the data parameter in bytes.
      uint8_t dataLength,
      // The data to be echoed back.
      uint8_t *data,
      // Return: The echo of the data.
      uint8_t *echo);

// Allows the NCP to respond with a pending callback.
void sl_zigbee_ezsp_callback(void);

// Callback
// Indicates that there are currently no pending callbacks.
void sl_zigbee_ezsp_no_callbacks(void);

// Sets a token (8 bytes of non-volatile storage) in the Simulated EEPROM of the
// NCP.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_set_token(
      // Which token to set
      uint8_t tokenId,
      // The data to write to the token.
      uint8_t *tokenData);

// Retrieves a token (8 bytes of non-volatile storage) from the Simulated EEPROM
// of the NCP.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_get_token(
      // Which token to read
      uint8_t tokenId,
      // Return: The contents of the token.
      uint8_t *tokenData);

// Retrieves a manufacturing token from the Flash Information Area of the NCP
// (except for SL_ZIGBEE_EZSP_STACK_CAL_DATA which is managed by the stack).
// Return: The length of the tokenData parameter in bytes.
uint8_t sl_zigbee_ezsp_get_mfg_token(
      // Which manufacturing token to read.
      sl_zigbee_ezsp_mfg_token_id_t tokenId,
      // Return: The manufacturing token data.
      uint8_t *tokenData);

// Sets a manufacturing token in the Customer Information Block (CIB) area of
// the NCP if that token currently unset (fully erased). Cannot be used with
// SL_ZIGBEE_EZSP_STACK_CAL_DATA, SL_ZIGBEE_EZSP_STACK_CAL_FILTER,
// SL_ZIGBEE_EZSP_MFG_ASH_CONFIG, or SL_ZIGBEE_EZSP_MFG_CBKE_DATA token.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_set_mfg_token(
      // Which manufacturing token to set.
      sl_zigbee_ezsp_mfg_token_id_t tokenId,
      // The length of the tokenData parameter in bytes.
      uint8_t tokenDataLength,
      // The manufacturing token data.
      uint8_t *tokenData);

// Callback
// A callback invoked to inform the application that a stack token has changed.
void sl_zigbee_ezsp_stack_token_changed_handler(
      // The address of the stack token that has changed.
      uint16_t tokenAddress);

// Returns a pseudorandom number.
// Return: Always returns SL_STATUS_OK.
sl_status_t sl_zigbee_ezsp_get_random_number(
      // Return: A pseudorandom number.
      uint16_t *value);

// Sets a timer on the NCP. There are 2 independent timers available for use by
// the Host. A timer can be cancelled by setting time to 0 or units to
// SL_ZIGBEE_EVENT_INACTIVE.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_set_timer(
      // Which timer to set (0 or 1).
      uint8_t timerId,
      // The delay before the timerHandler callback will be generated. Note that
      // the timer clock is free running and is not synchronized with this
      // command. This means that the actual delay will be between time and
      // (time - 1). The maximum delay is 32767.
      uint16_t time,
      // The units for time.
      sl_zigbee_event_units_t units,
      // If true, a timerHandler callback will be generated repeatedly. If
      // false, only a single timerHandler callback will be generated.
      bool repeat);

// Gets information about a timer. The Host can use this command to find out how
// much longer it will be before a previously set timer will generate a
// callback.
// Return: The delay before the timerHandler callback will be generated.
uint16_t sl_zigbee_ezsp_get_timer(
      // Which timer to get information about (0 or 1).
      uint8_t timerId,
      // Return: The units for time.
      sl_zigbee_event_units_t *units,
      // Return: True if a timerHandler callback will be generated repeatedly.
      // False if only a single timerHandler callback will be generated.
      bool *repeat);

// Callback
// A callback from the timer.
void sl_zigbee_ezsp_timer_handler(
      // Which timer generated the callback (0 or 1).
      uint8_t timerId);

// Sends a debug message from the Host to the Network Analyzer utility via the
// NCP.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_debug_write(
      // true if the message should be interpreted as binary data, false if the
      // message should be interpreted as ASCII text.
      bool binaryMessage,
      // The length of the messageContents parameter in bytes.
      uint8_t messageLength,
      // The binary message.
      uint8_t *messageContents);

// Retrieves and clears Ember counters. See the sl_zigbee_counter_type_t
// enumeration for the counter types.
void sl_zigbee_ezsp_read_and_clear_counters(
      // Return: A list of all counter values ordered according to the
      // sl_zigbee_counter_type_t enumeration.
      uint16_t *values);

// Retrieves Ember counters. See the sl_zigbee_counter_type_t enumeration for
// the counter types.
void sl_zigbee_ezsp_read_counters(
      // Return: A list of all counter values ordered according to the
      // sl_zigbee_counter_type_t enumeration.
      uint16_t *values);

// Callback
// This call is fired when a counter exceeds its threshold
void sl_zigbee_ezsp_counter_rollover_handler(
      // Type of Counter
      sl_zigbee_counter_type_t type);

// Callback
// This call is fired when mux detects an invalid rx case, which would be
// different rx channels for different protocol contexts, when fast cahnnel
// switching is not enabled
void sl_zigbee_ezsp_mux_invalid_rx_handler(
      uint8_t new_rx_channel,
      uint8_t old_rx_channel);

// Used to test that UART flow control is working correctly.
void sl_zigbee_ezsp_delay_test(
      // Data will not be read from the host for this many milliseconds.
      uint16_t delay);

// This retrieves the status of the passed library ID to determine if it is
// compiled into the stack.
// Return: The status of the library being queried.
sl_zigbee_library_status_t sl_zigbee_ezsp_get_library_status(
      // The ID of the library being queried.
      sl_zigbee_library_id_t libraryId);

// Allows the HOST to know whether the NCP is running the XNCP library. If so,
// the response contains also the manufacturer ID and the version number of the
// XNCP application that is running on the NCP.
// Return: SL_STATUS_OK if the NCP is running the XNCP library.
// SL_STATUS_INVALID_STATE otherwise.
sl_status_t sl_zigbee_ezsp_get_xncp_info(
      // Return: The manufactured ID the user has defined in the XNCP
      // application.
      uint16_t *manufacturerId,
      // Return: The version number of the XNCP application.
      uint16_t *versionNumber);

// Provides the customer a custom EZSP frame. On the NCP, these frames are only
// handled if the XNCP library is included. On the NCP side these frames are
// handled in the sl_zigbee_xncp_incoming_custom_ezsp_message_cb() callback
// function.
// Return: The status returned by the custom command.
sl_status_t sl_zigbee_ezsp_custom_frame(
      // The length of the custom frame payload (maximum 119 bytes).
      uint8_t payloadLength,
      // The payload of the custom frame.
      uint8_t *payload,
      // Return: The length of the response.
      uint8_t *replyLength,
      // Return: The response.
      uint8_t *reply);

// Callback
// A callback indicating a custom EZSP message has been received.
void sl_zigbee_ezsp_custom_frame_handler(
      // The length of the custom frame payload.
      uint8_t payloadLength,
      // The payload of the custom frame.
      uint8_t *payload);

// Returns the EUI64 ID of the local node.
void sl_zigbee_ezsp_get_eui64(
      // Return: The 64-bit ID.
      sl_802154_long_addr_t eui64);

// Returns the 16-bit node ID of the local node.
// Return: The 16-bit ID.
sl_802154_short_addr_t sl_zigbee_ezsp_get_node_id(void);

// Returns number of phy interfaces present.
// Return: Value indicate how many phy interfaces present.
uint8_t sl_zigbee_ezsp_get_phy_interface_count(void);

// Returns the entropy source used for true random number generation.
// Return: Value indicates the used entropy source.
sl_zigbee_entropy_source_t sl_zigbee_ezsp_get_true_random_entropy_source(void);

// Extend a joiner's timeout to wait for the network key on the joiner default
// key timeout is 3 sec, and only values greater equal to 3 sec are accepted.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_setup_delayed_join(
      // Network key timeout
      uint8_t networkKeyTimeoutS);

// Get the current scheduler priorities for radio operations
void sl_zigbee_ezsp_radio_get_scheduler_priorities(
      // Return: The current priorities.
      sl_802154_radio_priorities_t *priorities);

// Get the current multiprotocol sliptime
void sl_zigbee_ezsp_radio_get_scheduler_sliptime(
      // Return: Value of the current slip time.
      uint32_t *slipTime);

// Set the current multiprotocol sliptime
void sl_zigbee_ezsp_radio_set_scheduler_sliptime(
      // Value of the current slip time.
      uint32_t slipTime);

// Check if a particular counter is one that could report from either a 2.4GHz
// or sub-GHz interface.
// Return: Whether this counter requires a PHY index when operating on a
// dual-PHY system.
bool sl_zigbee_ezsp_counter_requires_phy_index(
      // The counter to be checked.
      sl_zigbee_counter_type_t counter);

// Check if a particular counter can report on the destination node ID they have
// been triggered from.
// Return: Whether this counter requires the destination node ID.
bool sl_zigbee_ezsp_counter_requires_destination_node_id(
      // The counter to be checked.
      sl_zigbee_counter_type_t counter);

//------------------------------------------------------------------------------
// Networking Frames
//------------------------------------------------------------------------------

// Sets the manufacturer code to the specified value. The manufacturer code is
// one of the fields of the node descriptor.
sl_status_t sl_zigbee_ezsp_set_manufacturer_code(
      // The manufacturer code for the local node.
      uint16_t code);

// Gets the manufacturer code to the specified value. The manufacturer code is
// one of the fields of the node descriptor.
// Return: The manufacturer code for the local node.
uint16_t sl_zigbee_ezsp_get_manufacturer_code(void);

// Sets the power descriptor to the specified value. The power descriptor is a
// dynamic value. Therefore, you should call this function whenever the value
// changes.
sl_status_t sl_zigbee_ezsp_set_power_descriptor(
      // The new power descriptor for the local node.
      uint16_t descriptor);

// Resume network operation after a reboot. The node retains its original type.
// This should be called on startup whether or not the node was previously part
// of a network. SL_STATUS_NOT_JOINED is returned if the node is not part of a
// network. This command accepts options to control the network initialization.
// Return: An sl_status_t value that indicates one of the following: successful
// initialization, SL_STATUS_NOT_JOINED if the node is not part of a network, or
// the reason for failure.
sl_status_t sl_zigbee_ezsp_network_init(
      // An sl_zigbee_network_init_struct_t containing the options for
      // initialization.
      sl_zigbee_network_init_struct_t *networkInitStruct);

// Returns a value indicating whether the node is joining, joined to, or leaving
// a network.
// Return: An sl_zigbee_network_status_t value indicating the current join
// status.
sl_zigbee_network_status_t sl_zigbee_ezsp_network_state(void);

// Callback
// A callback invoked when the status of the stack changes. If the status
// parameter equals SL_STATUS_NETWORK_UP, then the getNetworkParameters command
// can be called to obtain the new network parameters. If any of the parameters
// are being stored in nonvolatile memory by the Host, the stored values should
// be updated.
void sl_zigbee_ezsp_stack_status_handler(
      // Stack status
      sl_status_t status);

// This function will start a scan.
// Return: SL_STATUS_OK signals that the scan successfully started. Possible
// error responses and their meanings: SL_STATUS_MAC_SCANNING, we are already
// scanning; SL_STATUS_BAD_SCAN_DURATION, we have set a duration value that is
// not 0..14 inclusive; SL_STATUS_MAC_INCORRECT_SCAN_TYPE, we have requested an
// undefined scanning type; SL_STATUS_INVALID_CHANNEL_MASK, our channel mask did
// not specify any valid channels.
sl_status_t sl_zigbee_ezsp_start_scan(
      // Indicates the type of scan to be performed. Possible values are:
      // SL_ZIGBEE_EZSP_ENERGY_SCAN and SL_ZIGBEE_EZSP_ACTIVE_SCAN. For each
      // type, the respective callback for reporting results is:
      // energyScanResultHandler and networkFoundHandler. The energy scan and
      // active scan report errors and completion via the scanCompleteHandler.
      sl_zigbee_ezsp_network_scan_type_t scanType,
      // Bits set as 1 indicate that this particular channel should be scanned.
      // Bits set to 0 indicate that this particular channel should not be
      // scanned. For example, a channelMask value of 0x00000001 would indicate
      // that only channel 0 should be scanned. Valid channels range from 11 to
      // 26 inclusive. This translates to a channel mask value of 0x07FFF800. As
      // a convenience, a value of 0 is reinterpreted as the mask for the
      // current channel.
      uint32_t channelMask,
      // Sets the exponent of the number of scan periods, where a scan period is
      // 960 symbols. The scan will occur for ((2^duration) + 1) scan periods.
      uint8_t duration);

// Callback
// Reports the result of an energy scan for a single channel. The scan is not
// complete until the scanCompleteHandler callback is called.
void sl_zigbee_ezsp_energy_scan_result_handler(
      // The 802.15.4 channel number that was scanned.
      uint8_t channel,
      // The maximum RSSI value found on the channel.
      int8_t maxRssiValue);

// Callback
// Reports that a network was found as a result of a prior call to startScan.
// Gives the network parameters useful for deciding which network to join.
void sl_zigbee_ezsp_network_found_handler(
      // The parameters associated with the network found.
      sl_zigbee_zigbee_network_t *networkFound,
      // Link quality of incoming packet from network.
      uint8_t lastHopLqi,
      // Power (in dBm) of incoming packet.
      int8_t lastHopRssi);

// Callback
// Returns the status of the current scan of type SL_ZIGBEE_EZSP_ENERGY_SCAN or
// SL_ZIGBEE_EZSP_ACTIVE_SCAN. SL_STATUS_OK signals that the scan has completed.
// Other error conditions signify a failure to scan on the channel specified.
void sl_zigbee_ezsp_scan_complete_handler(
      // The channel on which the current error occurred. Undefined for the case
      // of SL_STATUS_OK.
      uint8_t channel,
      // The error condition that occurred on the current channel. Value will be
      // SL_STATUS_OK when the scan has completed.
      sl_status_t status);

// Callback
// This function returns an unused panID and channel pair found via the find
// unused panId scan procedure.
void sl_zigbee_ezsp_unused_pan_id_found_handler(
      // The unused panID which has been found.
      sl_802154_pan_id_t panId,
      // The channel that the unused panID was found on.
      uint8_t channel);

// This function starts a series of scans which will return an available panId.
// Return: The error condition that occurred during the scan. Value will be
// SL_STATUS_OK if there are no errors.
sl_status_t sl_zigbee_ezsp_find_unused_pan_id(
      // The channels that will be scanned for available panIds.
      uint32_t channelMask,
      // The duration of the procedure.
      uint8_t duration);

// Terminates a scan in progress.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_stop_scan(void);

// Forms a new network by becoming the coordinator.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_form_network(
      // Specification of the new network.
      sl_zigbee_network_parameters_t *parameters);

// Causes the stack to associate with the network using the specified network
// parameters. It can take several seconds for the stack to associate with the
// local network. Do not send messages until the stackStatusHandler callback
// informs you that the stack is up.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_join_network(
      // Specification of the role that this node will have in the network. This
      // role must not be SL_ZIGBEE_COORDINATOR. To be a coordinator, use the
      // formNetwork command.
      sl_zigbee_node_type_t nodeType,
      // Specification of the network with which the node should associate.
      sl_zigbee_network_parameters_t *parameters);

// Causes the stack to associate with the network using the specified network
// parameters in the beacon parameter. It can take several seconds for the stack
// to associate with the local network. Do not send messages until the
// stackStatusHandler callback informs you that the stack is up. Unlike
// ::sli_zigbee_stack_join_network(), this function does not issue an active
// scan before joining. Instead, it will cause the local node to issue a MAC
// Association Request directly to the specified target node. It is assumed that
// the beacon parameter is an artifact after issuing an active scan. (For more
// information, see sli_zigbee_stack_get_stored_beacon.)
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_join_network_directly(
      // Specifies the role that this node will have in the network. This role
      // must not be SL_ZIGBEE_COORDINATOR. To be a coordinator, use the
      // formNetwork command.
      sl_zigbee_node_type_t localNodeType,
      // Specifies the network with which the node should associate.
      sl_zigbee_beacon_data_t *beacon,
      // The radio transmit power to use, specified in dBm.
      int8_t radioTxPower,
      // If true, clear beacons in cache upon join success. If join fail, do
      // nothing.
      bool clearBeaconsAfterNetworkUp);

// Causes the stack to leave the current network. This generates a
// stackStatusHandler callback to indicate that the network is down. The radio
// will not be used until after sending a formNetwork or joinNetwork command.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_leave_network(
      // This parameter gives options when leave network
      sl_zigbee_leave_network_option_t options);

// The application may call this function when contact with the network has been
// lost. The most common usage case is when an end device can no longer
// communicate with its parent and wishes to find a new one. Another case is
// when a device has missed a Network Key update and no longer has the current
// Network Key.  The stack will call sl_zigbee_ezsp_stack_status_handler to
// indicate that the network is down, then try to re-establish contact with the
// network by performing an active scan, choosing a network with matching
// extended pan id, and sending a ZigBee network rejoin request. A second call
// to the sl_zigbee_ezsp_stack_status_handler callback indicates either the
// success or the failure of the attempt. The process takes approximately 150
// milliseconds per channel to complete. 
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_find_and_rejoin_network(
      // This parameter tells the stack whether to try to use the current
      // network key. If it has the current network key it will perform a secure
      // rejoin (encrypted). If this fails the device should try an unsecure
      // rejoin. If the Trust Center allows the rejoin then the current Network
      // Key will be sent encrypted using the device's Link Key.
      bool haveCurrentNetworkKey,
      // A mask indicating the channels to be scanned. See
      // sli_zigbee_stack_start_scan for format details. A value of 0 is
      // reinterpreted as the mask for the current channel.
      uint32_t channelMask,
      // A sl_zigbee_rejoin_reason_t variable which could be passed in if there
      // is actually a reason for rejoin, or could be left at 0xFF
      uint8_t reason,
      // The rejoin could be triggered with a different nodeType. This value
      // could be set to 0 or SL_ZIGBEE_DEVICE_TYPE_UNCHANGED if not needed.
      uint8_t nodeType);

// Tells the stack to allow other nodes to join the network with this node as
// their parent. Joining is initially disabled by default.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_permit_joining(
      // A value of 0x00 disables joining. A value of 0xFF enables joining. Any
      // other value enables joining for that number of seconds.
      uint8_t duration);

// Callback
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
      sl_zigbee_node_type_t childType);

// Sends a ZDO energy scan request. This request may only be sent by the current
// network manager and must be unicast, not broadcast. See ezsp-utils.h for
// related macros sli_zigbee_stack_set_network_manager_request() and
// sl_zigbee_change_channel_request().
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_energy_scan_request(
      // The network address of the node to perform the scan.
      sl_802154_short_addr_t target,
      // A mask of the channels to be scanned
      uint32_t scanChannels,
      // How long to scan on each channel. Allowed values are 0..5, with the
      // scan times as specified by 802.15.4 (0 = 31ms, 1 = 46ms, 2 = 77ms, 3 =
      // 138ms, 4 = 261ms, 5 = 507ms).
      uint8_t scanDuration,
      // The number of scans to be performed on each channel (1..8).
      uint16_t scanCount);

// Returns the current network parameters.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_get_network_parameters(
      // Return: An sl_zigbee_node_type_t value indicating the current node
      // type.
      sl_zigbee_node_type_t *nodeType,
      // Return: The current network parameters.
      sl_zigbee_network_parameters_t *parameters);

// Returns the current radio parameters based on phy index.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_get_radio_parameters(
      // Desired index of phy interface for radio parameters.
      uint8_t phyIndex,
      // Return: The current radio parameters based on provided phy index.
      sl_zigbee_multi_phy_radio_parameters_t *parameters);

// Returns information about the children of the local node and the parent of
// the local node.
// Return: The number of children the node currently has.
uint8_t sl_zigbee_ezsp_get_parent_child_parameters(
      // Return: The parent's EUI64. The value is undefined for nodes without
      // parents (coordinators and nodes that are not joined to a network).
      sl_802154_long_addr_t parentEui64,
      // Return: The parent's node ID. The value is undefined for nodes without
      // parents (coordinators and nodes that are not joined to a network).
      sl_802154_short_addr_t *parentNodeId);

// Return the number of router children that the node currently has.
// Return: The number of router children.
uint8_t sl_zigbee_ezsp_router_child_count(void);

// Return the maximum number of children for this node. The return value is
// undefined for nodes that are not joined to a network.
// Return: The maximum number of children.
uint8_t sl_zigbee_ezsp_max_child_count(void);

// Return the maximum number of router children for this node. The return value
// is undefined for nodes that are not joined to a network.
// Return: The maximum number of router children.
uint8_t sl_zigbee_ezsp_max_router_child_count(void);

uint32_t sl_zigbee_ezsp_get_parent_incoming_nwk_frame_counter(void);

sl_status_t sl_zigbee_ezsp_set_parent_incoming_nwk_frame_counter(
      uint32_t value);

// Return a bitmask indicating the stack's current tasks. The mask
// ::SL_ZIGBEE_HIGH_PRIORITY_TASKS defines which tasks are high priority. 
// Devices should not sleep if any high priority tasks are active. Active tasks
// that are not high priority are waiting for messages to arrive from other
// devices.  If there are active tasks, but no high priority ones, the device
// may sleep but should periodically wake up and call ::emberPollForData() in
// order to receive messages.  Parents will hold messages for
// ::SL_ZIGBEE_INDIRECT_TRANSMISSION_TIMEOUT milliseconds before discarding
// them.
// Return: A bitmask of the stack's active tasks.
uint16_t sl_zigbee_ezsp_current_stack_tasks(void);

// Indicate whether the stack is currently in a state where there are no
// high-priority tasks, allowing the device to sleep.
// There may be tasks expecting incoming messages, in which case the device
// should periodically wake up and call ::emberPollForData() in order to receive
// messages. This function can only be called when the node type is
// ::SL_ZIGBEE_SLEEPY_END_DEVICE
// Return: True if the application may sleep but the stack may be expecting
// incoming messages.
bool sl_zigbee_ezsp_ok_to_nap(void);

// Indicate whether the parent token has been set by association.
// Return: True if the parent token has been set.
bool sl_zigbee_ezsp_parent_token_set(void);

// Indicate whether the stack currently has any tasks pending. If no tasks are
// pending, ::emberTick() does not need to be called until the next time a stack
// API function is called. This function can only be called when the node type
// is ::SL_ZIGBEE_SLEEPY_END_DEVICE.
// Return: True if the application may sleep for as long as it wishes.
bool sl_zigbee_ezsp_ok_to_hibernate(void);

// Indicate whether the stack is currently in a state that does not require the
// application to periodically poll.
// Return: True if the device may poll less frequently.
bool sl_zigbee_ezsp_ok_to_long_poll(void);

// Calling this function will render all other stack functions except
// sli_zigbee_stack_stack_power_up() non-functional until the radio is powered
// back on.
void sl_zigbee_ezsp_stack_power_down(void);

// Initialize the radio.  Typically called coming out of deep sleep. For
// non-sleepy devices, also turns the radio on and leaves it in RX mode.
void sl_zigbee_ezsp_stack_power_up(void);

// Returns information about a child of the local node.
// Return: SL_STATUS_OK if there is a child at index. SL_STATUS_NOT_JOINED if
// there is no child at index.
sl_status_t sl_zigbee_ezsp_get_child_data(
      // The index of the child of interest in the child table. Possible indexes
      // range from zero to SL_ZIGBEE_CHILD_TABLE_SIZE.
      uint8_t index,
      // Return: The data of the child.
      sl_zigbee_child_data_t *childData);

// Sets child data to the child table token.
// Return: SL_STATUS_OK if the child data is set successfully at index.
// SL_STATUS_INVALID_INDEX if provided index is out of range.
sl_status_t sl_zigbee_ezsp_set_child_data(
      // The index of the child of interest in the child table. Possible indexes
      // range from zero to (SL_ZIGBEE_CHILD_TABLE_SIZE - 1).
      uint8_t index,
      // The data of the child.
      sl_zigbee_child_data_t *childData);

// Convert a child index to a node ID
// Return: The node ID of the child or SL_ZIGBEE_NULL_NODE_ID if there isn't a
// child at the childIndex specified
sl_802154_short_addr_t sl_zigbee_ezsp_child_id(
      // The index of the child of interest in the child table. Possible indexes
      // range from zero to SL_ZIGBEE_CHILD_TABLE_SIZE.
      uint8_t childIndex);

// Return radio power value of the child from the given childIndex
// Return: The power of the child or maximum radio power, which is the power
// value provided by the user while forming/joining a network if there isn't a
// child at the childIndex specified
int8_t sl_zigbee_ezsp_child_power(
      // The index of the child of interest in the child table. Possible indexes
      // range from zero to SL_ZIGBEE_CHILD_TABLE_SIZE.
      uint8_t childIndex);

// Set the radio power value for a given child index.
void sl_zigbee_ezsp_set_child_power(
      // The index.
      uint8_t childIndex,
      // The new power value.
      int8_t newPower);

// Convert a node ID to a child index
// Return: The child index or 0xFF if the node ID doesn't belong to a child
uint8_t sl_zigbee_ezsp_child_index(
      // The node ID of the child
      sl_802154_short_addr_t childId);

// Returns the source route table total size.
// Return: Total size of source route table.
uint8_t sl_zigbee_ezsp_get_source_route_table_total_size(void);

// Returns the number of filled entries in source route table.
// Return: The number of filled entries in source route table.
uint8_t sl_zigbee_ezsp_get_source_route_table_filled_size(void);

// Returns information about a source route table entry
// Return: SL_STATUS_OK if there is source route entry at
// index. SL_STATUS_NOT_FOUND if there is no
// source route at index.
sl_status_t sl_zigbee_ezsp_get_source_route_table_entry(
      // The index of the entry of interest in the
      // source route table. Possible indexes range from zero to
      // SOURCE_ROUTE_TABLE_FILLED_SIZE.
      uint8_t index,
      // Return: The node ID of the destination in that entry.
      sl_802154_short_addr_t *destination,
      // Return: The closer node index for this source route table entry
      uint8_t *closerIndex);

// Returns the neighbor table entry at the given index. The number of active
// neighbors can be obtained using the neighborCount command.
// Return: SL_STATUS_FAIL if the index is greater or equal to the number of
// active neighbors, or if the device is an end device. Returns SL_STATUS_OK
// otherwise.
sl_status_t sl_zigbee_ezsp_get_neighbor(
      // The index of the neighbor of interest. Neighbors are stored in
      // ascending order by node id, with all unused entries at the end of the
      // table.
      uint8_t index,
      // Return: The contents of the neighbor table entry.
      sl_zigbee_neighbor_table_entry_t *value);

// Return sl_status_t depending on whether the frame counter of the node is
// found in the neighbor or child table. This function gets the last received
// frame counter as found in the Network Auxiliary header for the specified
// neighbor or child
// Return: Return SL_STATUS_NOT_FOUND if the node is not found in the neighbor
// or child table. Returns SL_STATUS_OK otherwise
sl_status_t sl_zigbee_ezsp_get_neighbor_frame_counter(
      // eui64 of the node
      sl_802154_long_addr_t eui64,
      // Return: Return the frame counter of the node from the neighbor or child
      // table
      uint32_t *returnFrameCounter);

// Sets the frame counter for the neighbour or child.
// Return: Return SL_STATUS_NOT_FOUND if the node is not found in the neighbor
// or child table. Returns SL_STATUS_OK otherwise
sl_status_t sl_zigbee_ezsp_set_neighbor_frame_counter(
      // eui64 of the node
      sl_802154_long_addr_t eui64,
      // Return the frame counter of the node from the neighbor or child table
      uint32_t frameCounter);

// Sets the routing shortcut threshold to directly use a neighbor instead of
// performing routing.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_set_routing_shortcut_threshold(
      // The routing shortcut threshold to configure.
      uint8_t costThresh);

// Gets the routing shortcut threshold used to differentiate between directly
// using a neighbor vs. performing routing.
// Return: The routing shortcut threshold
uint8_t sl_zigbee_ezsp_get_routing_shortcut_threshold(void);

// Returns the number of active entries in the neighbor table.
// Return: The number of active entries in the neighbor table.
uint8_t sl_zigbee_ezsp_neighbor_count(void);

// Returns the route table entry at the given index. The route table size can be
// obtained using the getConfigurationValue command.
// Return: SL_STATUS_FAIL if the index is out of range or the device is an end
// device, and SL_STATUS_OK otherwise.
sl_status_t sl_zigbee_ezsp_get_route_table_entry(
      // The index of the route table entry of interest.
      uint8_t index,
      // Return: The contents of the route table entry.
      sl_zigbee_route_table_entry_t *value);

// Sets the radio output power at which a node is operating. Ember radios have
// discrete power settings. For a list of available power settings, see the
// technical specification for the RF communication module in your Developer
// Kit. Note: Care should be taken when using this API on a running network, as
// it will directly impact the established link qualities neighboring nodes have
// with the node on which it is called. This can lead to disruption of existing
// routes and erratic network behavior.
// Return: An sl_status_t value indicating the success or failure of the
// command.
sl_status_t sl_zigbee_ezsp_set_radio_power(
      // Desired radio output power, in dBm.
      int8_t power);

// Sets the channel to use for sending and receiving messages. For a list of
// available radio channels, see the technical specification for the RF
// communication module in your Developer Kit. Note: Care should be taken when
// using this API, as all devices on a network must use the same channel.
// Return: An sl_status_t value indicating the success or failure of the
// command.
sl_status_t sl_zigbee_ezsp_set_radio_channel(
      // Desired radio channel.
      uint8_t channel);

// Gets the channel in use for sending and receiving messages.
// Return: Current radio channel.
uint8_t sl_zigbee_ezsp_get_radio_channel(void);

// Set the configured 802.15.4 CCA mode in the radio.
// Return: An sl_status_t value indicating the success or failure of the
// command.
sl_status_t sl_zigbee_ezsp_set_radio_ieee802154_cca_mode(
      // A RAIL_IEEE802154_CcaMode_t value.
      uint8_t ccaMode);

// Enable/disable concentrator support.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_set_concentrator(
      // If this bool is true the concentrator support is enabled. Otherwise is
      // disabled. If this bool is false all the other arguments are ignored.
      bool on,
      // Must be either SL_ZIGBEE_HIGH_RAM_CONCENTRATOR or
      // SL_ZIGBEE_LOW_RAM_CONCENTRATOR. The former is used when the caller has
      // enough memory to store source routes for the whole network. In that
      // case, remote nodes stop sending route records once the concentrator has
      // successfully received one. The latter is used when the concentrator has
      // insufficient RAM to store all outbound source routes. In that case,
      // route records are sent to the concentrator prior to every inbound APS
      // unicast.
      uint16_t concentratorType,
      // The minimum amount of time that must pass between MTORR broadcasts.
      uint16_t minTime,
      // The maximum amount of time that can pass between MTORR broadcasts.
      uint16_t maxTime,
      // The number of route errors that will trigger a re-broadcast of the
      // MTORR.
      uint8_t routeErrorThreshold,
      // The number of APS delivery failures that will trigger a re-broadcast of
      // the MTORR.
      uint8_t deliveryFailureThreshold,
      // The maximum number of hops that the MTORR broadcast will be allowed to
      // have. A value of 0 will be converted to the SL_ZIGBEE_MAX_HOPS value
      // set by the stack.
      uint8_t maxHops);

// Starts periodic many-to-one route discovery. Periodic discovery is started by
// default on bootup, but this function may be used if discovery has been
// stopped by a call to ::emberConcentratorStopDiscovery().
void sl_zigbee_ezsp_concentrator_start_discovery(void);

// Stops periodic many-to-one route discovery.
void sl_zigbee_ezsp_concentrator_stop_discovery(void);

// Notes when a route error has occurred.
void sl_zigbee_ezsp_concentrator_note_route_error(
      sl_status_t status,
      sl_802154_short_addr_t nodeId);

// Sets the error code that is sent back from a router with a broken route. 
// Return: An sl_status_t value indicating the success or failure of the
// command.
sl_status_t sl_zigbee_ezsp_set_broken_route_error_code(
      // Desired error code.
      uint8_t errorCode);

// This causes to initialize the desired radio interface other than native and
// form a new network by becoming the coordinator with same panId as native
// radio network.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_multi_phy_start(
      // Index of phy interface. The native phy index would be always zero hence
      // valid phy index starts from one.
      uint8_t phyIndex,
      // Desired radio channel page.
      uint8_t page,
      // Desired radio channel.
      uint8_t channel,
      // Desired radio output power, in dBm.
      int8_t power,
      // Network configuration bitmask.
      sl_zigbee_multi_phy_nwk_config_t bitmask);

// This causes to bring down the radio interface other than native.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_multi_phy_stop(
      // Index of phy interface. The native phy index would be always zero hence
      // valid phy index starts from one.
      uint8_t phyIndex);

// Sets the radio output power for desired phy interface at which a node is
// operating. Ember radios have discrete power settings. For a list of available
// power settings, see the technical specification for the RF communication
// module in your Developer Kit. Note: Care should be taken when using this api
// on a running network, as it will directly impact the established link
// qualities neighboring nodes have with the node on which it is called. This
// can lead to disruption of existing routes and erratic network behavior.
// Return: An sl_status_t value indicating the success or failure of the
// command.
sl_status_t sl_zigbee_ezsp_multi_phy_set_radio_power(
      // Index of phy interface. The native phy index would be always zero hence
      // valid phy index starts from one.
      uint8_t phyIndex,
      // Desired radio output power, in dBm.
      int8_t power);

// Send Link Power Delta Request from a child to its parent
// Return: An sl_status_t value indicating the success or failure of sending the
// request.
sl_status_t sl_zigbee_ezsp_send_link_power_delta_request(void);

// Sets the channel for desired phy interface to use for sending and receiving
// messages. For a list of available radio pages and channels, see the technical
// specification for the RF communication module in your Developer Kit. Note:
// Care should be taken when using this API, as all devices on a network must
// use the same page and channel.
// Return: An sl_status_t value indicating the success or failure of the
// command.
sl_status_t sl_zigbee_ezsp_multi_phy_set_radio_channel(
      // Index of phy interface. The native phy index would be always zero hence
      // valid phy index starts from one.
      uint8_t phyIndex,
      // Desired radio channel page.
      uint8_t page,
      // Desired radio channel.
      uint8_t channel);

// Obtains the current duty cycle state.
// Return: An sl_status_t value indicating the success or failure of the
// command.
sl_status_t sl_zigbee_ezsp_get_duty_cycle_state(
      // Return: The current duty cycle state in effect.
      sl_zigbee_duty_cycle_state_t *returnedState);

// Set the current duty cycle limits configuration. The Default limits set by
// stack if this call is not made.
// Return: SL_STATUS_OK  if the duty cycle limit configurations set
// successfully, SL_STATUS_INVALID_PARAMETER if set illegal value such as
// setting only one of the limits to default or violates constraints Susp > Crit
// > Limi, SL_STATUS_INVALID_STATE if device is operating on 2.4Ghz
sl_status_t sl_zigbee_ezsp_set_duty_cycle_limits_in_stack(
      // The duty cycle limits configuration to utilize.
      sl_zigbee_duty_cycle_limits_t *limits);

// Obtains the current duty cycle limits that were previously set by a call to
// sli_zigbee_stack_set_duty_cycle_limits_in_stack(), or the defaults set by the
// stack if no set call was made.
// Return: An sl_status_t value indicating the success or failure of the
// command.
sl_status_t sl_zigbee_ezsp_get_duty_cycle_limits(
      // Return: Return current duty cycle limits if returnedLimits is not NULL
      sl_zigbee_duty_cycle_limits_t *returnedLimits);

// Returns the duty cycle of the stack's connected children that are being
// monitored, up to maxDevices. It indicates the amount of overall duty cycle
// they have consumed (up to the suspend limit). The first entry is always the
// local stack's nodeId, and thus the total aggregate duty cycle for the device.
// The passed pointer arrayOfDeviceDutyCycles MUST have space for maxDevices.
// Return: SL_STATUS_OK  if the duty cycles were read successfully,
// SL_STATUS_INVALID_PARAMETER maxDevices is greater than
// SL_ZIGBEE_MAX_END_DEVICE_CHILDREN + 1.
sl_status_t sl_zigbee_ezsp_get_current_duty_cycle(
      // Number of devices to retrieve consumed duty cycle.
      uint8_t maxDevices,
      // Return: Consumed duty cycles up to maxDevices. When the number of
      // children that are being monitored is less than maxDevices, the
      // sl_802154_short_addr_t element in the sl_zigbee_per_device_duty_cycle_t
      // will be 0xFFFF.
      uint8_t *arrayOfDeviceDutyCycles);

// Callback
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
      sl_zigbee_per_device_duty_cycle_t *arrayOfDeviceDutyCycles);

// Configure the number of beacons to store when issuing active scans for
// networks.
// Return: SL_STATUS_INVALID_PARAMETER if numBeacons is greater than
// SL_ZIGBEE_MAX_BEACONS_TO_STORE, otherwise SL_STATUS_OK
sl_status_t sl_zigbee_ezsp_set_num_beacons_to_store(
      // The number of beacons to cache when scanning.
      uint8_t numBeacons);

// Fetches the specified beacon in the cache. Beacons are stored in cache after
// issuing an active scan.
// Return: An appropriate sl_status_t status code.
sl_status_t sl_zigbee_ezsp_get_stored_beacon(
      // The beacon index to fetch. Valid values range from 0 to
      // sli_zigbee_stack_get_num_stored_beacons-1.
      uint8_t beacon_number,
      // Return: The beacon to populate upon success.
      sl_zigbee_beacon_data_t *beacon);

// Returns the number of cached beacons that have been collected from a scan.
// Return: The number of cached beacons that have been collected from a scan.
uint8_t sl_zigbee_ezsp_get_num_stored_beacons(void);

// Clears all cached beacons that have been collected from a scan.
sl_status_t sl_zigbee_ezsp_clear_stored_beacons(void);

// This call sets the radio channel in the stack and propagates the information
// to the hardware.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_set_logical_and_radio_channel(
      // The radio channel to be set.
      uint8_t radioChannel);

// Form a new sleepy-to-sleepy network.  If the network is using security, the
// device must call sli_zigbee_stack_set_initial_security_state() first.
// Return: An sl_status_t value indicating success or a reason for failure.
sl_status_t sl_zigbee_ezsp_sleepy_to_sleepy_network_start(
      // Specification of the new network.
      sl_zigbee_network_parameters_t *parameters,
      // Whether this device is initiating or joining the network.
      bool initiator);

// Send a Zigbee NWK Leave command to the destination.
// Return: Status indicating success or a reason for failure. Call is invalid if
// destination is on network or is the local node.
sl_status_t sl_zigbee_ezsp_send_zigbee_leave(
      // Node ID of the device being told to leave.
      sl_802154_pan_id_t destination,
      // Bitmask indicating additional considerations for the leave request.
      sl_zigbee_leave_request_flags_t flags);

// Indicate the state of permit joining in MAC.
// Return: Whether the current network permits joining.
bool sl_zigbee_ezsp_get_permit_joining(void);

// Get the 8-byte extended PAN ID of this node.
void sl_zigbee_ezsp_get_extended_pan_id(
      // Return: Extended PAN ID of this node.  Valid only if it is currently on
      // a network.
      uint8_t *extendedPanId);

// Get the current network.
// Return: Return the current network index.
uint8_t sl_zigbee_ezsp_get_current_network(void);

// Set initial outgoing link cost for neighbor.
// Return: Whether or not initial cost was successfully set.
sl_status_t sl_zigbee_ezsp_set_initial_neighbor_outgoing_cost(
      // The new default cost. Valid values are 0, 1, 3, 5, and 7.
      uint8_t cost);

// Get initial outgoing link cost for neighbor.
// Return: The default cost associated with new neighbor's outgoing links.
uint8_t sl_zigbee_ezsp_get_initial_neighbor_outgoing_cost(void);

// Indicate whether a rejoining neighbor should have its incoming frame counter
// reset.
void sl_zigbee_ezsp_reset_rejoining_neighbors_frame_counter(
      // Whether or not a neighbor's incoming FC should be reset upon rejoining
      // (true or false).
      bool reset);

// Check whether a rejoining neighbor will have its incoming frame counter reset
// based on the currently set policy.
// Return: Whether or not a rejoining neighbor's incoming FC gets reset (true or
// false).
bool sl_zigbee_ezsp_is_reset_rejoining_neighbors_frame_counter_enabled(void);

//------------------------------------------------------------------------------
// Binding Frames
//------------------------------------------------------------------------------

// Deletes all binding table entries.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_clear_binding_table(void);

// Sets an entry in the binding table.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_set_binding(
      // The index of a binding table entry.
      uint8_t index,
      // The contents of the binding entry.
      sl_zigbee_binding_table_entry_t *value);

// Gets an entry from the binding table.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_get_binding(
      // The index of a binding table entry.
      uint8_t index,
      // Return: The contents of the binding entry.
      sl_zigbee_binding_table_entry_t *value);

// Deletes a binding table entry.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_delete_binding(
      // The index of a binding table entry.
      uint8_t index);

// Indicates whether any messages are currently being sent using this binding
// table entry. Note that this command does not indicate whether a binding is
// clear. To determine whether a binding is clear, check whether the type field
// of the sl_zigbee_binding_table_entry_t has the value
// SL_ZIGBEE_UNUSED_BINDING.
// Return: True if the binding table entry is active, false otherwise.
bool sl_zigbee_ezsp_binding_is_active(
      // The index of a binding table entry.
      uint8_t index);

// Returns the node ID for the binding's destination, if the ID is known. If a
// message is sent using the binding and the destination's ID is not known, the
// stack will discover the ID by broadcasting a ZDO address request. The
// application can avoid the need for this discovery by using
// setBindingRemoteNodeId when it knows the correct ID via some other means. The
// destination's node ID is forgotten when the binding is changed, when the
// local node reboots or, much more rarely, when the destination node changes
// its ID in response to an ID conflict.
// Return: The short ID of the destination node or SL_ZIGBEE_NULL_NODE_ID if no
// destination is known.
sl_802154_short_addr_t sl_zigbee_ezsp_get_binding_remote_node_id(
      // The index of a binding table entry.
      uint8_t index);

// Set the node ID for the binding's destination. See getBindingRemoteNodeId for
// a description.
void sl_zigbee_ezsp_set_binding_remote_node_id(
      // The index of a binding table entry.
      uint8_t index,
      // The short ID of the destination node.
      sl_802154_short_addr_t nodeId);

// Callback
// The NCP used the external binding modification policy to decide how to handle
// a remote set binding request. The Host cannot change the current decision,
// but it can change the policy for future decisions using the setPolicy
// command.
void sl_zigbee_ezsp_remote_set_binding_handler(
      // The requested binding.
      sl_zigbee_binding_table_entry_t *entry,
      // The index at which the binding was added.
      uint8_t index,
      // SL_STATUS_OK if the binding was added to the table and any other status
      // if not.
      sl_status_t policyDecision);

// Callback
// The NCP used the external binding modification policy to decide how to handle
// a remote delete binding request. The Host cannot change the current decision,
// but it can change the policy for future decisions using the setPolicy
// command.
void sl_zigbee_ezsp_remote_delete_binding_handler(
      // The index of the binding whose deletion was requested.
      uint8_t index,
      // SL_STATUS_OK if the binding was removed from the table and any other
      // status if not.
      sl_status_t policyDecision);

//------------------------------------------------------------------------------
// Messaging Frames
//------------------------------------------------------------------------------

// Returns the maximum size of the payload. The size depends on the security
// level in use.
// Return: The maximum APS payload length.
uint8_t sl_zigbee_ezsp_maximum_payload_length(void);

// Sends a unicast message as per the ZigBee specification. The message will
// arrive at its destination only if there is a known route to the destination
// node. Setting the ENABLE_ROUTE_DISCOVERY option will cause a route to be
// discovered if none is known. Setting the FORCE_ROUTE_DISCOVERY option will
// force route discovery. Routes to end-device children of the local node are
// always known. Setting the APS_RETRY option will cause the message to be
// retransmitted until either a matching acknowledgement is received or three
// transmissions have been made. Note: Using the FORCE_ROUTE_DISCOVERY option
// will cause the first transmission to be consumed by a route request as part
// of discovery, so the application payload of this packet will not reach its
// destination on the first attempt. If you want the packet to reach its
// destination, the APS_RETRY option must be set so that another attempt is made
// to transmit the message with its application payload after the route has been
// constructed. Note: When sending fragmented messages, the stack will only
// assign a new APS sequence number for the first fragment of the message (i.e.,
// SL_ZIGBEE_APS_OPTION_FRAGMENT is set and the low-order byte of the groupId
// field in the APS frame is zero). For all subsequent fragments of the same
// message, the application must set the sequence number field in the APS frame
// to the sequence number assigned by the stack to the first fragment.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_send_unicast(
      // Specifies the outgoing message type. Must be one of
      // SL_ZIGBEE_OUTGOING_DIRECT, SL_ZIGBEE_OUTGOING_VIA_ADDRESS_TABLE, or
      // SL_ZIGBEE_OUTGOING_VIA_BINDING.
      sl_zigbee_outgoing_message_type_t type,
      // Depending on the type of addressing used, this is either the
      // sl_802154_short_addr_t of the destination, an index into the address
      // table, or an index into the binding table.
      sl_802154_short_addr_t indexOrDestination,
      // The APS frame which is to be added to the message.
      sl_zigbee_aps_frame_t *apsFrame,
      // A value chosen by the Host. This value is used in the
      // sl_zigbee_ezsp_message_sent_handler response to refer to this message.
      uint16_t messageTag,
      // The length of the messageContents parameter in bytes.
      uint8_t messageLength,
      // Content of the message.
      uint8_t *messageContents,
      // Return: The sequence number that will be used when this message is
      // transmitted.
      uint8_t *sequence);

// Sends a broadcast message as per the ZigBee specification.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_send_broadcast(
      // The aliased source from which we send the broadcast. This must be
      // SL_ZIGBEE_NULL_NODE_ID if we do not need an aliased source
      sl_802154_short_addr_t alias,
      // The destination to which to send the broadcast. This must be one of the
      // three ZigBee broadcast addresses.
      sl_802154_short_addr_t destination,
      // The alias nwk sequence number. this won't be used if there is no
      // aliased source.
      uint8_t nwkSequence,
      // The APS frame for the message.
      sl_zigbee_aps_frame_t *apsFrame,
      // The message will be delivered to all nodes within radius hops of the
      // sender. A radius of zero is converted to SL_ZIGBEE_MAX_HOPS.
      uint8_t radius,
      // A value chosen by the Host. This value is used in the
      // sl_zigbee_ezsp_message_sent_handler response to refer to this message.
      uint16_t messageTag,
      // The length of the messageContents parameter in bytes.
      uint8_t messageLength,
      // The broadcast message.
      uint8_t *messageContents,
      // Return: The APS sequence number that will be used when this message is
      // transmitted.
      uint8_t *apsSequence);

// Sends proxied broadcast message for another node in conjunction with
// sl_zigbee_proxy_broadcast where a long source is also specified in the NWK
// frame control.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_proxy_next_broadcast_from_long(
      // The long source from which to send the broadcast
      uint8_t *euiSource);

// Sends a multicast message to all endpoints that share a specific multicast ID
// and are within a specified number of hops of the sender.
// Return: An sl_status_t value. For any result other than SL_STATUS_OK, the
// message will not be sent. SL_STATUS_OK - The message has been submitted for
// transmission. SL_STATUS_INVALID_INDEX - The bindingTableIndex refers to a
// non-multicast binding. SL_STATUS_NETWORK_DOWN - The node is not part of a
// network. SL_STATUS_MESSAGE_TOO_LONG - The message is too large to fit in a
// MAC layer frame. SL_STATUS_ALLOCATION_FAILED - The free packet buffer pool is
// empty. SL_STATUS_BUSY - Insufficient resources available in Network or MAC
// layers to send message.
sl_status_t sl_zigbee_ezsp_send_multicast(
      // The APS frame for the message. The multicast will be sent to the
      // groupId in this frame.
      sl_zigbee_aps_frame_t *apsFrame,
      // The message will be delivered to all nodes within this number of hops
      // of the sender. A value of zero is converted to SL_ZIGBEE_MAX_HOPS.
      uint8_t hops,
      // The number of hops that the message will be forwarded by devices that
      // are not members of the group. A value of 7 or greater is treated as
      // infinite.
      uint16_t broadcastAddr,
      // The alias source address
      uint16_t alias,
      // the alias sequence number
      uint8_t nwkSequence,
      // A value chosen by the Host. This value is used in the
      // sl_zigbee_ezsp_message_sent_handler response to refer to this message.
      uint16_t messageTag,
      // The length of the messageContents parameter in bytes.
      uint8_t messageLength,
      // The multicast message.
      uint8_t *messageContents,
      // Return: The sequence number that will be used when this message is
      // transmitted.
      uint8_t *sequence);

// Sends a reply to a received unicast message. The incomingMessageHandler
// callback for the unicast being replied to supplies the values for all the
// parameters except the reply itself.
// Return: An sl_status_t value. SL_STATUS_INVALID_STATE - The
// SL_ZIGBEE_EZSP_UNICAST_REPLIES_POLICY is set to
// SL_ZIGBEE_EZSP_HOST_WILL_NOT_SUPPLY_REPLY. This means the NCP will
// automatically send an empty reply. The Host must change the policy to
// SL_ZIGBEE_EZSP_HOST_WILL_SUPPLY_REPLY before it can supply the reply. There
// is one exception to this rule: In the case of responses to message fragments,
// the host must call sendReply when a message fragment is received. In this
// case, the policy set on the NCP does not matter. The NCP expects a sendReply
// call from the Host for message fragments regardless of the current policy
// settings. SL_STATUS_ALLOCATION_FAILED - Not enough memory was available to
// send the reply. SL_STATUS_BUSY - Either no route or insufficient resources
// available. SL_STATUS_OK - The reply was successfully queued for transmission.
sl_status_t sl_zigbee_ezsp_send_reply(
      // Value supplied by incoming unicast.
      sl_802154_short_addr_t sender,
      // Value supplied by incoming unicast.
      sl_zigbee_aps_frame_t *apsFrame,
      // The length of the messageContents parameter in bytes.
      uint8_t messageLength,
      // The reply message.
      uint8_t *messageContents);

// Callback
// A callback indicating the stack has completed sending a message.
void sl_zigbee_ezsp_message_sent_handler(
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
      // The value supplied by the Host in the sl_zigbee_ezsp_send_unicast,
      // sl_zigbee_ezsp_send_broadcast or sl_zigbee_ezsp_send_multicast command.
      uint16_t messageTag,
      // The length of the messageContents parameter in bytes.
      uint8_t messageLength,
      // The unicast message supplied by the Host. The message contents are only
      // included here if the decision for the messageContentsInCallback policy
      // is messageTagAndContentsInCallback.
      uint8_t *messageContents);

// Sends a route request packet that creates routes from every node in the
// network back to this node. This function should be called by an application
// that wishes to communicate with many nodes, for example, a gateway, central
// monitor, or controller. A device using this function was referred to as an
// 'aggregator' in EmberZNet 2.x and earlier, and is referred to as a
// 'concentrator' in the ZigBee specification and EmberZNet 3.  This function
// enables large scale networks, because the other devices do not have to
// individually perform bandwidth-intensive route discoveries. Instead, when a
// remote node sends an APS unicast to a concentrator, its network layer
// automatically delivers a special route record packet first, which lists the
// network ids of all the intermediate relays. The concentrator can then use
// source routing to send outbound APS unicasts. (A source routed message is one
// in which the entire route is listed in the network layer header.) This allows
// the concentrator to communicate with thousands of devices without requiring
// large route tables on neighboring nodes.  This function is only available in
// ZigBee Pro (stack profile 2), and cannot be called on end devices. Any router
// can be a concentrator (not just the coordinator), and there can be multiple
// concentrators on a network.  Note that a concentrator does not automatically
// obtain routes to all network nodes after calling this function. Remote
// applications must first initiate an inbound APS unicast.  Many-to-one routes
// are not repaired automatically. Instead, the concentrator application must
// call this function to rediscover the routes as necessary, for example, upon
// failure of a retried APS message. The reason for this is that there is no
// scalable one-size-fits-all route repair strategy. A common and recommended
// strategy is for the concentrator application to refresh the routes by calling
// this function periodically.
// Return: SL_STATUS_OK if the route request was successfully submitted to the
// transmit queue, and SL_STATUS_FAIL otherwise.
sl_status_t sl_zigbee_ezsp_send_many_to_one_route_request(
      // Must be either SL_ZIGBEE_HIGH_RAM_CONCENTRATOR or
      // SL_ZIGBEE_LOW_RAM_CONCENTRATOR. The former is used when the caller has
      // enough memory to store source routes for the whole network. In that
      // case, remote nodes stop sending route records once the concentrator has
      // successfully received one. The latter is used when the concentrator has
      // insufficient RAM to store all outbound source routes. In that case,
      // route records are sent to the concentrator prior to every inbound APS
      // unicast.
      uint16_t concentratorType,
      // The maximum number of hops the route request will be relayed. A radius
      // of zero is converted to SL_ZIGBEE_MAX_HOPS
      uint8_t radius);

// Periodically request any pending data from our parent. Setting interval to 0
// or units to SL_ZIGBEE_EVENT_INACTIVE will generate a single poll.
// Return: The result of sending the first poll.
sl_status_t sl_zigbee_ezsp_poll_for_data(
      // The time between polls. Note that the timer clock is free running and
      // is not synchronized with this command. This means that the time will be
      // between interval and (interval - 1). The maximum interval is 32767.
      uint16_t interval,
      // The units for interval.
      sl_zigbee_event_units_t units,
      // The number of poll failures that will be tolerated before a
      // pollCompleteHandler callback is generated. A value of zero will result
      // in a callback for every poll. Any status value apart from SL_STATUS_OK
      // and SL_STATUS_MAC_NO_DATA is counted as a failure.
      uint8_t failureLimit);

// Callback
// Indicates the result of a data poll to the parent of the local node.
void sl_zigbee_ezsp_poll_complete_handler(
      // An sl_status_t value: SL_STATUS_OK - Data was received in response to
      // the poll. SL_STATUS_MAC_NO_DATA - No data was pending.
      // SL_STATUS_ZIGBEE_DELIVERY_FAILED - The poll message could not be sent.
      // SL_STATUS_MAC_NO_ACK_RECEIVED - The poll message was sent but not
      // acknowledged by the parent.
      sl_status_t status);

// Set a flag to indicate that a message is pending for a child. The next time
// that the child polls, it will be informed that it has a pending message. The
// message is sent from emberPollHandler, which is called when the child
// requests data.
// Return: SL_STATUS_OK - The next time that the child polls, it will be
// informed that it has pending data. SL_STATUS_NOT_JOINED - The child
// identified by childId is not our child.
sl_status_t sl_zigbee_ezsp_set_message_flag(
      // The ID of the child that just polled for data.
      sl_802154_short_addr_t childId);

// Clear a flag to indicate that there are no more messages for a child. The
// next time the child polls, it will be informed that it does not have any
// pending messages.
// Return: SL_STATUS_OK - The next time that the child polls, it will be
// informed that it does not have any pending messages. SL_STATUS_NOT_JOINED -
// The child identified by childId is not our child.
sl_status_t sl_zigbee_ezsp_clear_message_flag(
      // The ID of the child that no longer has pending messages.
      sl_802154_short_addr_t childId);

// Callback
// Indicates that the local node received a data poll from a child.
void sl_zigbee_ezsp_poll_handler(
      // The node ID of the child that is requesting data.
      sl_802154_short_addr_t childId,
      // True if transmit is expected, false otherwise.
      bool transmitExpected);

// Add a child to the child/neighbor table only on SoC, allowing direct
// manipulation of these tables by the application. This can affect the network
// functionality, and needs to be used wisely. If used appropriately, the
// application can maintain more than the maximum of children provided by the
// stack.
// Return: SL_STATUS_OK - This node has been successfully added. SL_STATUS_FAIL
// - The child was not added to the child/neighbor table.
sl_status_t sl_zigbee_ezsp_add_child(
      // The preferred short ID of the node.
      sl_802154_short_addr_t shortId,
      // The long ID of the node.
      sl_802154_long_addr_t longId,
      // The nodetype e.g., SL_ZIGBEE_ROUTER defining, if this would be added to
      // the child table or neighbor table.
      sl_zigbee_node_type_t nodeType);

// Remove a node from child/neighbor table only on SoC, allowing direct
// manipulation of these tables by the application. This can affect the network
// functionality, and needs to be used wisely.
// Return: SL_STATUS_OK - This node has been successfully removed.
// SL_STATUS_FAIL - The node was not found in either of the child or neighbor
// tables.
sl_status_t sl_zigbee_ezsp_remove_child(
      // The long ID of the node.
      sl_802154_long_addr_t childEui64);

// Remove a neighbor from neighbor table only on SoC, allowing direct
// manipulation of neighbor table by the application. This can affect the
// network functionality, and needs to be used wisely.
void sl_zigbee_ezsp_remove_neighbor(
      // The short ID of the neighbor.
      sl_802154_short_addr_t shortId,
      // The long ID of the neighbor.
      sl_802154_long_addr_t longId);

// Callback
// A callback indicating a message has been received.
void sl_zigbee_ezsp_incoming_message_handler(
      // The type of the incoming message. One of the following:
      // SL_ZIGBEE_INCOMING_UNICAST, SL_ZIGBEE_INCOMING_UNICAST_REPLY,
      // SL_ZIGBEE_INCOMING_MULTICAST, SL_ZIGBEE_INCOMING_MULTICAST_LOOPBACK,
      // SL_ZIGBEE_INCOMING_BROADCAST, SL_ZIGBEE_INCOMING_BROADCAST_LOOPBACK
      sl_zigbee_incoming_message_type_t type,
      // The APS frame from the incoming message.
      sl_zigbee_aps_frame_t *apsFrame,
      // Miscellanous message information.
      sl_zigbee_rx_packet_info_t *packetInfo,
      // The length of the message parameter in bytes.
      uint8_t messageLength,
      // The incoming message.
      uint8_t *message);

// Sets source route discovery(MTORR) mode to on, off, reschedule
// Return: Remaining time(ms) until next MTORR broadcast if the mode is on,
// MAX_INT32U_VALUE if the mode is off
uint32_t sl_zigbee_ezsp_set_source_route_discovery_mode(
      // Source route discovery mode: off:0, on:1, reschedule:2
      uint8_t mode);

// Callback
// A callback indicating that a many-to-one route to the concentrator with the
// given short and long id is available for use.
void sl_zigbee_ezsp_incoming_many_to_one_route_request_handler(
      // The short id of the concentrator.
      sl_802154_short_addr_t source,
      // The EUI64 of the concentrator.
      sl_802154_long_addr_t longId,
      // The path cost to the concentrator. The cost may decrease as additional
      // route request packets for this discovery arrive, but the callback is
      // made only once.
      uint8_t cost);

// Callback
// A callback invoked when a route error message is received. The error
// indicates that a problem routing to or from the target node was encountered.
void sl_zigbee_ezsp_incoming_route_error_handler(
      // SL_STATUS_ZIGBEE_SOURCE_ROUTE_FAILURE or
      // SL_STATUS_ZIGBEE_MANY_TO_ONE_ROUTE_FAILURE.
      sl_status_t status,
      // The short id of the remote node.
      sl_802154_short_addr_t target);

// Callback
// A callback invoked when a network status/route error message is received. The
// error indicates that there was a problem sending/receiving messages from the
// target node
void sl_zigbee_ezsp_incoming_network_status_handler(
      // One byte over-the-air error code from network status message
      uint8_t errorCode,
      // The short ID of the remote node
      sl_802154_short_addr_t target);

// Callback
// Reports the arrival of a route record command frame.
void sl_zigbee_ezsp_incoming_route_record_handler(
      // The source of the route record.
      sl_802154_short_addr_t source,
      // The EUI64 of the source.
      sl_802154_long_addr_t sourceEui,
      // The link quality from the node that last relayed the route record.
      uint8_t lastHopLqi,
      // The energy level (in units of dBm) observed during the reception.
      int8_t lastHopRssi,
      // The number of relays in relayList.
      uint8_t relayCount,
      // The route record. Each relay in the list is an uint16_t node ID. The
      // list is passed as uint8_t * to avoid alignment problems.
      uint8_t *relayList);

// Supply a source route for the next outgoing message.
// Return: SL_STATUS_OK if the source route was successfully stored, and
// SL_STATUS_ALLOCATION_FAILED otherwise.
sl_status_t sl_zigbee_ezsp_set_source_route(
      // The destination of the source route.
      sl_802154_short_addr_t destination,
      // The number of relays in relayList.
      uint8_t relayCount,
      // The source route.
      uint16_t *relayList);

// Send the network key to a destination.
// Return: SL_STATUS_OK if send was successful
sl_status_t sl_zigbee_ezsp_unicast_current_network_key(
      // The destination node of the key.
      sl_802154_short_addr_t targetShort,
      // The long address of the destination node.
      sl_802154_long_addr_t targetLong,
      // The parent node of the destination node.
      sl_802154_short_addr_t parentShortId);

// Indicates whether any messages are currently being sent using this address
// table entry. Note that this function does not indicate whether the address
// table entry is unused. To determine whether an address table entry is unused,
// check the remote node ID. The remote node ID will have the value
// SL_ZIGBEE_TABLE_ENTRY_UNUSED_NODE_ID when the address table entry is not in
// use.
// Return: True if the address table entry is active, false otherwise.
bool sl_zigbee_ezsp_address_table_entry_is_active(
      // The index of an address table entry.
      uint8_t addressTableIndex);

// Sets the EUI64 and short ID of an address table entry. Usually the
// application will not need to set the short ID in the address table. Once the
// remote EUI64 is set the stack is capable of figuring out the short ID on its
// own. However, in cases where the application does set the short ID, the
// application must set the remote EUI64 prior to setting the short ID. This
// function will also check other address table entries, the child table and the
// neighbor table to see if the node ID for the given EUI64 is already known. If
// known then this function will set node ID. If not known it will set the node
// ID to SL_ZIGBEE_UNKNOWN_NODE_ID.
// Return: SL_STATUS_OK if the information was successfully set, and
// SL_STATUS_ZIGBEE_ADDRESS_TABLE_ENTRY_IS_ACTIVE otherwise.
sl_status_t sl_zigbee_ezsp_set_address_table_info(
      // The index of an address table entry.
      uint8_t addressTableIndex,
      // The EUI64 to use for the address table entry.
      sl_802154_long_addr_t eui64,
      // The short ID corresponding to the remote node whose EUI64 is stored in
      // the address table at the given index or
      // SL_ZIGBEE_TABLE_ENTRY_UNUSED_NODE_ID which indicates that the entry
      // stored in the address table at the given index is not in use.
      sl_802154_short_addr_t id);

// Gets the EUI64 and short ID of an address table entry.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_get_address_table_info(
      // The index of an address table entry.
      uint8_t addressTableIndex,
      // Return: One of the following: The short ID corresponding to the remote
      // node whose EUI64 is stored in the address table at the given index.
      // SL_ZIGBEE_UNKNOWN_NODE_ID - Indicates that the EUI64 stored in the
      // address table at the given index is valid but the short ID is currently
      // unknown. SL_ZIGBEE_DISCOVERY_ACTIVE_NODE_ID - Indicates that the EUI64
      // stored in the address table at the given location is valid and network
      // address discovery is underway. SL_ZIGBEE_TABLE_ENTRY_UNUSED_NODE_ID -
      // Indicates that the entry stored in the address table at the given index
      // is not in use.
      sl_802154_short_addr_t *nodeId,
      // Return: The EUI64 of the address table entry is copied to this
      // location.
      sl_802154_long_addr_t eui64);

// Tells the stack whether or not the normal interval between retransmissions of
// a retried unicast message should be increased by
// SL_ZIGBEE_INDIRECT_TRANSMISSION_TIMEOUT. The interval needs to be increased
// when sending to a sleepy node so that the message is not retransmitted until
// the destination has had time to wake up and poll its parent. The stack will
// automatically extend the timeout: - For our own sleepy children. - When an
// address response is received from a parent on behalf of its child. - When an
// indirect transaction expiry route error is received. - When an end device
// announcement is received from a sleepy node.
// Return: An sl_status_t value indicating success or the reason for failure  
sl_status_t sl_zigbee_ezsp_set_extended_timeout(
      // The address of the node for which the timeout is to be set.
      sl_802154_long_addr_t remoteEui64,
      // true if the retry interval should be increased by
      // SL_ZIGBEE_INDIRECT_TRANSMISSION_TIMEOUT. false if the normal retry
      // interval should be used.
      bool extendedTimeout);

// Indicates whether or not the stack will extend the normal interval between
// retransmissions of a retried unicast message by
// SL_ZIGBEE_INDIRECT_TRANSMISSION_TIMEOUT.
// Return: SL_STATUS_OK if the retry interval will be increased by
// SL_ZIGBEE_INDIRECT_TRANSMISSION_TIMEOUT and SL_STATUS_FAIL if the normal
// retry interval will be used.
sl_status_t sl_zigbee_ezsp_get_extended_timeout(
      // The address of the node for which the timeout is to be returned.
      sl_802154_long_addr_t remoteEui64);

// Replaces the EUI64, short ID and extended timeout setting of an address table
// entry. The previous EUI64, short ID and extended timeout setting are
// returned.
// Return: SL_STATUS_OK if the EUI64, short ID and extended timeout setting were
// successfully modified, and SL_STATUS_ZIGBEE_ADDRESS_TABLE_ENTRY_IS_ACTIVE
// otherwise.
sl_status_t sl_zigbee_ezsp_replace_address_table_entry(
      // The index of the address table entry that will be modified.
      uint8_t addressTableIndex,
      // The EUI64 to be written to the address table entry.
      sl_802154_long_addr_t newEui64,
      // One of the following: The short ID corresponding to the new EUI64.
      // SL_ZIGBEE_UNKNOWN_NODE_ID if the new EUI64 is valid but the short ID is
      // unknown and should be discovered by the stack.
      // SL_ZIGBEE_TABLE_ENTRY_UNUSED_NODE_ID if the address table entry is now
      // unused.
      sl_802154_short_addr_t newId,
      // true if the retry interval should be increased by
      // SL_ZIGBEE_INDIRECT_TRANSMISSION_TIMEOUT. false if the normal retry
      // interval should be used.
      bool newExtendedTimeout,
      // Return: The EUI64 of the address table entry before it was modified.
      sl_802154_long_addr_t oldEui64,
      // Return: One of the following: The short ID corresponding to the EUI64
      // before it was modified. SL_ZIGBEE_UNKNOWN_NODE_ID if the short ID was
      // unknown. SL_ZIGBEE_DISCOVERY_ACTIVE_NODE_ID if discovery of the short
      // ID was underway. SL_ZIGBEE_TABLE_ENTRY_UNUSED_NODE_ID if the address
      // table entry was unused.
      sl_802154_short_addr_t *oldId,
      // Return: true if the retry interval was being increased by
      // SL_ZIGBEE_INDIRECT_TRANSMISSION_TIMEOUT. false if the normal retry
      // interval was being used.
      bool *oldExtendedTimeout);

// Returns the node ID that corresponds to the specified EUI64. The node ID is
// found by searching through all stack tables for the specified EUI64.
// Return: SL_STATUS_OK if the short ID was found, SL_STATUS_FAIL if the short
// ID is not known.
sl_status_t sl_zigbee_ezsp_lookup_node_id_by_eui64(
      // The EUI64 of the node to look up.
      sl_802154_long_addr_t eui64,
      // Return: The short ID of the node or SL_ZIGBEE_NULL_NODE_ID if the short
      // ID is not known.
      sl_802154_short_addr_t *nodeId);

// Returns the EUI64 that corresponds to the specified node ID. The EUI64 is
// found by searching through all stack tables for the specified node ID.
// Return: SL_STATUS_OK if the EUI64 was found, SL_STATUS_FAIL if the EUI64 is
// not known.
sl_status_t sl_zigbee_ezsp_lookup_eui64_by_node_id(
      // The short ID of the node to look up.
      sl_802154_short_addr_t nodeId,
      // Return: The EUI64 of the node.
      sl_802154_long_addr_t eui64);

// Gets an entry from the multicast table.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_get_multicast_table_entry(
      // The index of a multicast table entry.
      uint8_t index,
      // Return: The contents of the multicast entry.
      sl_zigbee_multicast_table_entry_t *value);

// Sets an entry in the multicast table.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_set_multicast_table_entry(
      // The index of a multicast table entry
      uint8_t index,
      // The contents of the multicast entry.
      sl_zigbee_multicast_table_entry_t *value);

// Callback
// A callback invoked by the EmberZNet stack when an id conflict is discovered,
// that is, two different nodes in the network were found to be using the same
// short id. The stack automatically removes the conflicting short id from its
// internal tables (address, binding, route, neighbor, and child tables). The
// application should discontinue any other use of the id.
void sl_zigbee_ezsp_id_conflict_handler(
      // The short id for which a conflict was detected
      sl_802154_short_addr_t id);

// Write the current node Id, PAN ID, or Node type to the tokens
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_write_node_data(
      // Erase the node type or not
      bool erase);

// Transmits the given message without modification. The MAC header is assumed
// to be configured in the message at the time this function is called.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_send_raw_message(
      // The length of the messageContents parameter in bytes.
      uint8_t messageLength,
      // The raw message.
      uint8_t *messageContents,
      // transmit priority.
      uint8_t priority,
      // Should we enable CCA or not.
      bool useCca);

// Callback
// A callback invoked by the EmberZNet stack when a MAC passthrough message is
// received.
void sl_zigbee_ezsp_mac_passthrough_message_handler(
      // The type of MAC passthrough message received.
      sl_zigbee_mac_passthrough_type_t messageType,
      // Information about the incoming packet.
      sl_zigbee_rx_packet_info_t *packetInfo,
      // The length of the messageContents parameter in bytes.
      uint8_t messageLength,
      // The raw message that was received.
      uint8_t *messageContents);

// Callback
// A callback invoked by the EmberZNet stack when a raw MAC message that has
// matched one of the application's configured MAC filters.
void sl_zigbee_ezsp_mac_filter_match_message_handler(
      // The value of the filter that was matched.
      sl_zigbee_mac_filter_match_data_t filterValueMatch,
      // The type of MAC passthrough message received.
      sl_zigbee_mac_passthrough_type_t legacyPassthroughType,
      // Information about the incoming packet.
      sl_zigbee_rx_packet_info_t *packetInfo,
      // The length of the messageContents parameter in bytes.
      uint8_t messageLength,
      // The raw message that was received.
      uint8_t *messageContents);

// Callback
// A callback invoked by the EmberZNet stack when the MAC has finished
// transmitting a raw message.
void sl_zigbee_ezsp_raw_transmit_complete_handler(
      // Length of the message that was transmitted.
      uint8_t messageLength,
      // The message that was transmitted.
      uint8_t *messageContents,
      // SL_STATUS_OK if the transmission was successful, or
      // SL_STATUS_ZIGBEE_DELIVERY_FAILED if not
      sl_status_t status);

// This function is useful to sleepy end devices. This function will set the
// retry interval (in milliseconds) for mac data poll. This interval is the time
// in milliseconds the device waits before retrying a data poll when a MAC level
// data poll fails for any reason.
void sl_zigbee_ezsp_set_mac_poll_failure_wait_time(
      // Time in milliseconds the device waits before retrying a data poll when
      // a MAC level data poll fails for any reason.
      uint32_t waitBeforeRetryIntervalMs);

// Returns the maximum number of no-ack retries that will be attempted
// Return: Max MAC retries
uint8_t sl_zigbee_ezsp_get_max_mac_retries(void);

// Sets the priority masks and related variables for choosing the best beacon.
// Return: The attempt to set the pramaters returns SL_STATUS_OK
sl_status_t sl_zigbee_ezsp_set_beacon_classification_params(
      // Return: Gets the beacon prioritization related variable
      sl_zigbee_beacon_classification_params_t *param);

// Gets the priority masks and related variables for choosing the best beacon.
// Return: The attempt to get the pramaters returns SL_STATUS_OK
sl_status_t sl_zigbee_ezsp_get_beacon_classification_params(
      // Return: Gets the beacon prioritization related variable
      sl_zigbee_beacon_classification_params_t *param);

// Indicate whether there are pending messages in the APS retry queue.
// Return: True if there is a pending message for this network in the APS retry
// queue, false if not.
bool sl_zigbee_ezsp_pending_acked_messages(void);

// Reschedule sending link status message, with first one being sent
// immediately.
sl_status_t sl_zigbee_ezsp_reschedule_link_status_msg(void);

// Set the network update ID to the desired value. Must be called before joining
// or forming the network.
// Return: Status of set operation for the network update ID.
sl_status_t sl_zigbee_ezsp_set_nwk_update_id(
      // Desired value of the network update ID.
      uint8_t nwkUpdateId,
      // Set to true in case change should also apply when on network.
      bool set_when_on_network);

//------------------------------------------------------------------------------
// Security Frames
//------------------------------------------------------------------------------

// Sets the security state that will be used by the device when it forms or
// joins the network. This call should not be used when restoring saved network
// state via networkInit as this will result in a loss of security data and will
// cause communication problems when the device re-enters the network.
// Return: The success or failure code of the operation.
sl_status_t sl_zigbee_ezsp_set_initial_security_state(
      // The security configuration to be set.
      sl_zigbee_initial_security_state_t *state);

// Gets the current security state that is being used by a device that is joined
// in the network.
// Return: The success or failure code of the operation.
sl_status_t sl_zigbee_ezsp_get_current_security_state(
      // Return: The security configuration in use by the stack.
      sl_zigbee_current_security_state_t *state);

// Exports a key from security manager based on passed context.
// Return: The success or failure code of the operation.
sl_status_t sl_zigbee_ezsp_sec_man_export_key(
      // Metadata to identify the requested key.
      sl_zigbee_sec_man_context_t *context,
      // Return: Data to store the exported key in.
      sl_zigbee_sec_man_key_t *key);

// Imports a key into security manager based on passed context.
// Return: The success or failure code of the operation.
sl_status_t sl_zigbee_ezsp_sec_man_import_key(
      // Metadata to identify where the imported key should be stored.
      sl_zigbee_sec_man_context_t *context,
      // The key to be imported.
      sl_zigbee_sec_man_key_t *key);

// Callback
// A callback to inform the application that the Network Key has been updated
// and the node has been switched over to use the new key. The actual key being
// used is not passed up, but the sequence number is.
void sl_zigbee_ezsp_switch_network_key_handler(
      // The sequence number of the new network key.
      uint8_t sequenceNumber);

// This function searches through the Key Table and tries to find the entry that
// matches the passed search criteria.
// Return: This indicates the index of the entry that matches the search
// criteria. A value of 0xFF is returned if not matching entry is found.
uint8_t sl_zigbee_ezsp_find_key_table_entry(
      // The address to search for. Alternatively, all zeros may be passed in to
      // search for the first empty entry.
      sl_802154_long_addr_t address,
      // This indicates whether to search for an entry that contains a link key
      // or a master key. true means to search for an entry with a Link Key.
      bool linkKey);

// This function sends an APS TransportKey command containing the current trust
// center link key. The node to which the command is sent is specified via the
// short and long address arguments.
// Return: An sl_status_t value indicating success of failure of the operation
sl_status_t sl_zigbee_ezsp_send_trust_center_link_key(
      // The short address of the node to which this command will be sent
      sl_802154_short_addr_t destinationNodeId,
      // The long address of the node to which this command will be sent
      sl_802154_long_addr_t destinationEui64);

// This function erases the data in the key table entry at the specified index.
// If the index is invalid, false is returned.
// Return: The success or failure of the operation.
sl_status_t sl_zigbee_ezsp_erase_key_table_entry(
      // This indicates the index of entry to erase.
      uint8_t index);

// This function clears the key table of the current network.
// Return: The success or failure of the operation.
sl_status_t sl_zigbee_ezsp_clear_key_table(void);

// A function to request a Link Key from the Trust Center with another device on
// the Network (which could be the Trust Center). A Link Key with the Trust
// Center is possible but the requesting device cannot be the Trust Center. Link
// Keys are optional in ZigBee Standard Security and thus the stack cannot know
// whether the other device supports them. If SL_ZIGBEE_REQUEST_KEY_TIMEOUT is
// non-zero on the Trust Center and the partner device is not the Trust Center,
// both devices must request keys with their partner device within the time
// period. The Trust Center only supports one outstanding key request at a time
// and therefore will ignore other requests. If the timeout is zero then the
// Trust Center will immediately respond and not wait for the second request.
// The Trust Center will always immediately respond to requests for a Link Key
// with it. Sleepy devices should poll at a higher rate until a response is
// received or the request times out. The success or failure of the request is
// returned via sl_zigbee_ezsp_zigbee_key_establishment_handler(...)
// Return: The success or failure of sending the request. This is not the final
// result of the attempt. sl_zigbee_ezsp_zigbee_key_establishment_handler(...)
// will return that.
sl_status_t sl_zigbee_ezsp_request_link_key(
      // This is the IEEE address of the partner device that will share the link
      // key.
      sl_802154_long_addr_t partner);

// Requests a new link key from the Trust Center. This function starts by
// sending a Node Descriptor request to the Trust Center to verify its R21+
// stack version compliance. A Request Key message will then be sent, followed
// by a Verify Key Confirm message.
// Return: The success or failure of sending the request. If the Node Descriptor
// is successfully transmitted,
// sl_zigbee_ezsp_zigbee_key_establishment_handler(...) will be called at a
// later time with a final status result.
sl_status_t sl_zigbee_ezsp_update_tc_link_key(
      // The maximum number of attempts a node should make when sending the Node
      // Descriptor, Request Key, and Verify Key Confirm messages. The number of
      // attempts resets for each message type sent (e.g., if maxAttempts is 3,
      // up to 3 Node Descriptors are sent, up to 3 Request Keys, and up to 3
      // Verify Key Confirm messages are sent).
      uint8_t maxAttempts);

// Callback
// This is a callback that indicates the success or failure of an attempt to
// establish a key with a partner device.
void sl_zigbee_ezsp_zigbee_key_establishment_handler(
      // This is the IEEE address of the partner that the device successfully
      // established a key with. This value is all zeros on a failure.
      sl_802154_long_addr_t partner,
      // This is the status indicating what was established or why the key
      // establishment failed.
      sl_zigbee_key_status_t status);

// Clear all of the transient link keys from RAM.
void sl_zigbee_ezsp_clear_transient_link_keys(void);

// Retrieve information about the current and alternate network key, excluding
// their contents.
// Return: Success or failure of retrieving network key info.
sl_status_t sl_zigbee_ezsp_sec_man_get_network_key_info(
      // Return: Information about current and alternate network keys.
      sl_zigbee_sec_man_network_key_info_t *network_key_info);

// Retrieve metadata about an APS link key.  Does not retrieve contents.
// Return: Status of metadata retrieval operation.
sl_status_t sl_zigbee_ezsp_sec_man_get_aps_key_info(
      // Context used to input information about key.
      sl_zigbee_sec_man_context_t *context,
      // Return: Metadata about the referenced key.
      sl_zigbee_sec_man_aps_key_metadata_t *key_data);

// Import an application link key into the key table.
// Return: Status of key import operation.
sl_status_t sl_zigbee_ezsp_sec_man_import_link_key(
      // Index where this key is to be imported to.
      uint8_t index,
      // EUI64 this key is associated with.
      sl_802154_long_addr_t address,
      // The key data to be imported.
      sl_zigbee_sec_man_key_t *plaintext_key);

// Export the link key at given index from the key table.
// Return: Status of key export operation.
sl_status_t sl_zigbee_ezsp_sec_man_export_link_key_by_index(
      // Index of key to export.
      uint8_t index,
      // Return: Context referencing the exported key.  Contains information
      // like the EUI64 address it is associated with.
      sl_zigbee_sec_man_context_t *context,
      // Return: The exported key.
      sl_zigbee_sec_man_key_t *plaintext_key,
      // Return: Metadata about the key.
      sl_zigbee_sec_man_aps_key_metadata_t *key_data);

// Export the link key associated with the given EUI from the key table.
// Return: Status of key export operation.
sl_status_t sl_zigbee_ezsp_sec_man_export_link_key_by_eui(
      // EUI64 associated with the key to export.
      sl_802154_long_addr_t eui,
      // Return: Context referring to the exported key, containing the table
      // index that this key is located in.
      sl_zigbee_sec_man_context_t *context,
      // Return: The exported key.
      sl_zigbee_sec_man_key_t *plaintext_key,
      // Return: Metadata about the key.
      sl_zigbee_sec_man_aps_key_metadata_t *key_data);

// Check whether a key context can be used to load a valid key.
// Return: Validity of the checked context.
sl_status_t sl_zigbee_ezsp_sec_man_check_key_context(
      // Context struct to check the validity of.
      sl_zigbee_sec_man_context_t *context);

// Import a transient link key.
// Return: Status of key import operation.
sl_status_t sl_zigbee_ezsp_sec_man_import_transient_key(
      // EUI64 associated with this transient key.
      sl_802154_long_addr_t eui64,
      // The key to import.
      sl_zigbee_sec_man_key_t *plaintext_key);

// Export a transient link key from a given table index.
// Return: Status of key export operation.
sl_status_t sl_zigbee_ezsp_sec_man_export_transient_key_by_index(
      // Index to export from.
      uint8_t index,
      // Return: Context struct for export operation.
      sl_zigbee_sec_man_context_t *context,
      // Return: The exported key.
      sl_zigbee_sec_man_key_t *plaintext_key,
      // Return: Metadata about the key.
      sl_zigbee_sec_man_aps_key_metadata_t *key_data);

// Export a transient link key associated with a given EUI64
// Return: Status of key export operation.
sl_status_t sl_zigbee_ezsp_sec_man_export_transient_key_by_eui(
      // Index to export from.
      sl_802154_long_addr_t eui,
      // Return: Context struct for export operation.
      sl_zigbee_sec_man_context_t *context,
      // Return: The exported key.
      sl_zigbee_sec_man_key_t *plaintext_key,
      // Return: Metadata about the key.
      sl_zigbee_sec_man_aps_key_metadata_t *key_data);

// Set the incoming TC link key frame counter to desired value.
void sl_zigbee_ezsp_set_incoming_tc_link_key_frame_counter(
      // Value to set the frame counter to.
      uint32_t frameCounter);

// Encrypt/decrypt a message in-place using APS.
// Return: Status of the encryption/decryption call.
sl_status_t sl_zigbee_ezsp_aps_crypt_message(
      // Encrypt (true) or decrypt (false) the message.
      bool encrypt,
      // Length of the array containing message, needs to be long enough to
      // include the auxiliary header and MIC.
      uint8_t length_combined_arg,
      // The message to be en/de-crypted.
      uint8_t *message,
      // Index just past the APS frame.
      uint8_t apsHeaderEndIndex,
      // IEEE address of the device this message is associated with.
      sl_802154_long_addr_t remoteEui64);

//------------------------------------------------------------------------------
// Trust Center Frames
//------------------------------------------------------------------------------

// Callback
// The NCP uses the trust center behavior policy to decide whether to allow a
// new node to join the network (part of the trust center pre-join handler). The
// Host cannot change the current decision in this post-join callback, but it
// can change the policy for future decisions using the setPolicy command.
void sl_zigbee_ezsp_trust_center_post_join_handler(
      // The Node Id of the node whose status changed
      sl_802154_short_addr_t newNodeId,
      // The EUI64 of the node whose status changed.
      sl_802154_long_addr_t newNodeEui64,
      // The status of the node: Secure Join/Rejoin, Unsecure Join/Rejoin,
      // Device left.
      sl_zigbee_device_update_t status,
      // An sl_zigbee_join_decision_t reflecting the decision made.
      sl_zigbee_join_decision_t policyDecision,
      // The parent of the node whose status has changed.
      sl_802154_short_addr_t parentOfNewNodeId);

// This function broadcasts a new encryption key, but does not tell the nodes in
// the network to start using it. To tell nodes to switch to the new key, use
// sl_zigbee_send_network_key_switch(). This is only valid for the Trust
// Center/Coordinator. It is up to the application to determine how quickly to
// send the Switch Key after sending the alternate encryption key.
// Return: sl_status_t value that indicates the success or failure of the
// command.
sl_status_t sl_zigbee_ezsp_broadcast_next_network_key(
      // An optional pointer to a 16-byte encryption key
      // (SL_ZIGBEE_ENCRYPTION_KEY_SIZE). An all zero key may be passed in,
      // which will cause the stack to randomly generate a new key.
      sl_zigbee_key_data_t *key);

// This function broadcasts a switch key message to tell all nodes to change to
// the sequence number of the previously sent Alternate Encryption Key.
// Return: sl_status_t value that indicates the success or failure of the
// command.
sl_status_t sl_zigbee_ezsp_broadcast_network_key_switch(void);

// This routine processes the passed chunk of data and updates the hash context
// based on it. If the 'finalize' parameter is not set, then the length of the
// data passed in must be a multiple of 16. If the 'finalize' parameter is set
// then the length can be any value up 1-16, and the final hash value will be
// calculated.
// Return: The result of the operation
sl_status_t sl_zigbee_ezsp_aes_mmo_hash(
      // The hash context to update.
      sl_zigbee_aes_mmo_hash_context_t *context,
      // This indicates whether the final hash value should be calculated
      bool finalize,
      // The length of the data to hash.
      uint8_t length,
      // The data to hash.
      uint8_t *data,
      // Return: The updated hash context.
      sl_zigbee_aes_mmo_hash_context_t *returnContext);

// This command sends an APS remove device using APS encryption to the
// destination indicating either to remove itself from the network, or one of
// its children.
// Return: An sl_status_t value indicating success, or the reason for failure
sl_status_t sl_zigbee_ezsp_remove_device(
      // The node ID of the device that will receive the message
      sl_802154_short_addr_t destShort,
      // The long address (EUI64) of the device that will receive the message.
      sl_802154_long_addr_t destLong,
      // The long address (EUI64) of the device to be removed.
      sl_802154_long_addr_t targetLong);

// This command will send a unicast transport key message with a new NWK key to
// the specified device. APS encryption using the device's existing link key
// will be used.
// Return: An sl_status_t value indicating success, or the reason for failure
sl_status_t sl_zigbee_ezsp_unicast_nwk_key_update(
      // The node ID of the device that will receive the message
      sl_802154_short_addr_t destShort,
      // The long address (EUI64) of the device that will receive the message.
      sl_802154_long_addr_t destLong,
      // The NWK key to send to the new device.
      sl_zigbee_key_data_t *key);

//------------------------------------------------------------------------------
// Certificate Based Key Exchange (CBKE) Frames
//------------------------------------------------------------------------------

// This call starts the generation of the ECC Ephemeral Public/Private key pair.
// When complete it stores the private key. The results are returned via
// sl_zigbee_ezsp_generate_cbke_keys_handler().
sl_status_t sl_zigbee_ezsp_generate_cbke_keys(void);

// Callback
// A callback by the Crypto Engine indicating that a new ephemeral
// public/private key pair has been generated. The public/private key pair is
// stored on the NCP, but only the associated public key is returned to the
// host. The node's associated certificate is also returned.
void sl_zigbee_ezsp_generate_cbke_keys_handler(
      // The result of the CBKE operation.
      sl_status_t status,
      // The generated ephemeral public key.
      sl_zigbee_public_key_data_t *ephemeralPublicKey);

// Calculates the SMAC verification keys for both the initiator and responder
// roles of CBKE using the passed parameters and the stored public/private key
// pair previously generated with ezspGenerateKeysRetrieveCert(). It also stores
// the unverified link key data in temporary storage on the NCP until the key
// establishment is complete.
sl_status_t sl_zigbee_ezsp_calculate_smacs(
      // The role of this device in the Key Establishment protocol.
      bool amInitiator,
      // The key establishment partner's implicit certificate.
      sl_zigbee_certificate_data_t *partnerCertificate,
      // The key establishment partner's ephemeral public key
      sl_zigbee_public_key_data_t *partnerEphemeralPublicKey);

// Callback
// A callback to indicate that the NCP has finished calculating the Secure
// Message Authentication Codes (SMAC) for both the initiator and responder. The
// associated link key is kept in temporary storage until the host tells the NCP
// to store or discard the key via
// sli_zigbee_stack_clear_temporary_data_maybe_store_link_key().
void sl_zigbee_ezsp_calculate_smacs_handler(
      // The Result of the CBKE operation.
      sl_status_t status,
      // The calculated value of the initiator's SMAC
      sl_zigbee_smac_data_t *initiatorSmac,
      // The calculated value of the responder's SMAC
      sl_zigbee_smac_data_t *responderSmac);

// This call starts the generation of the ECC 283k1 curve Ephemeral
// Public/Private key pair. When complete it stores the private key. The results
// are returned via sl_zigbee_ezsp_generate_cbke_keys_283k1_handler().
sl_status_t sl_zigbee_ezsp_generate_cbke_keys_283k1(void);

// Callback
// A callback by the Crypto Engine indicating that a new 283k1 ephemeral
// public/private key pair has been generated. The public/private key pair is
// stored on the NCP, but only the associated public key is returned to the
// host. The node's associated certificate is also returned.
void sl_zigbee_ezsp_generate_cbke_keys_283k1_handler(
      // The result of the CBKE operation.
      sl_status_t status,
      // The generated ephemeral public key.
      sl_zigbee_public_key_283k1_data_t *ephemeralPublicKey);

// Calculates the SMAC verification keys for both the initiator and responder
// roles of CBKE for the 283k1 ECC curve using the passed parameters and the
// stored public/private key pair previously generated with
// sl_zigbee_ezsp_generate_keys_retrieve_cert_283k1(). It also stores the
// unverified link key data in temporary storage on the NCP until the key
// establishment is complete.
sl_status_t sl_zigbee_ezsp_calculate_smacs_283k1(
      // The role of this device in the Key Establishment protocol.
      bool amInitiator,
      // The key establishment partner's implicit certificate.
      sl_zigbee_certificate_283k1_data_t *partnerCertificate,
      // The key establishment partner's ephemeral public key
      sl_zigbee_public_key_283k1_data_t *partnerEphemeralPublicKey);

// Callback
// A callback to indicate that the NCP has finished calculating the Secure
// Message Authentication Codes (SMAC) for both the initiator and responder for
// the CBKE 283k1 Library. The associated link key is kept in temporary storage
// until the host tells the NCP to store or discard the key via
// sli_zigbee_stack_clear_temporary_data_maybe_store_link_key().
void sl_zigbee_ezsp_calculate_smacs_283k1_handler(
      // The Result of the CBKE operation.
      sl_status_t status,
      // The calculated value of the initiator's SMAC
      sl_zigbee_smac_data_t *initiatorSmac,
      // The calculated value of the responder's SMAC
      sl_zigbee_smac_data_t *responderSmac);

// Clears the temporary data associated with CBKE and the key establishment,
// most notably the ephemeral public/private key pair. If storeLinKey is true it
// moves the unverified link key stored in temporary storage into the link key
// table. Otherwise it discards the key.
sl_status_t sl_zigbee_ezsp_clear_temporary_data_maybe_store_link_key(
      // A bool indicating whether to store (true) or discard (false) the
      // unverified link key derived when sl_zigbee_ezsp_calculate_smacs() was
      // previously called.
      bool storeLinkKey);

// Clears the temporary data associated with CBKE and the key establishment,
// most notably the ephemeral public/private key pair. If storeLinKey is true it
// moves the unverified link key stored in temporary storage into the link key
// table. Otherwise it discards the key.
sl_status_t sl_zigbee_ezsp_clear_temporary_data_maybe_store_link_key_283k1(
      // A bool indicating whether to store (true) or discard (false) the
      // unverified link key derived when sl_zigbee_ezsp_calculate_smacs() was
      // previously called.
      bool storeLinkKey);

// Retrieves the certificate installed on the NCP.
sl_status_t sl_zigbee_ezsp_get_certificate(
      // Return: The locally installed certificate.
      sl_zigbee_certificate_data_t *localCert);

// Retrieves the 283k certificate installed on the NCP.
sl_status_t sl_zigbee_ezsp_get_certificate_283k1(
      // Return: The locally installed certificate.
      sl_zigbee_certificate_283k1_data_t *localCert);

// LEGACY FUNCTION: This functionality has been replaced by a single bit in the
// sl_zigbee_aps_frame_t, SL_ZIGBEE_APS_OPTION_DSA_SIGN. Devices wishing to send
// signed messages should use that as it requires fewer function calls and
// message buffering. The dsaSignHandler response is still called when
// SL_ZIGBEE_APS_OPTION_DSA_SIGN is used. However, this function is still
// supported. This function begins the process of signing the passed message
// contained within the messageContents array. If no other ECC operation is
// going on, it will immediately return with SL_STATUS_IN_PROGRESS to indicate
// the start of ECC operation. It will delay a period of time to let APS retries
// take place, but then it will shut down the radio and consume the CPU
// processing until the signing is complete. This may take up to 1 second. The
// signed message will be returned in the dsaSignHandler response. Note that the
// last byte of the messageContents passed to this function has special
// significance. As the typical use case for DSA signing is to sign the ZCL
// payload of a DRLC Report Event Status message in SE 1.0, there is often both
// a signed portion (ZCL payload) and an unsigned portion (ZCL header). The last
// byte in the content of messageToSign is therefore used as a special indicator
// to signify how many bytes of leading data in the array should be excluded
// from consideration during the signing process. If the signature needs to
// cover the entire array (all bytes except last one), the caller should ensure
// that the last byte of messageContents is 0x00. When the signature operation
// is complete, this final byte will be replaced by the signature type indicator
// (0x01 for ECDSA signatures), and the actual signature will be appended to the
// original contents after this byte.
// Return: SL_STATUS_IN_PROGRESS if the stack has queued up the operation for
// execution. SL_STATUS_INVALID_STATE if the operation can't be performed in
// this context, possibly because another ECC operation is pending.
sl_status_t sl_zigbee_ezsp_dsa_sign(
      // The length of the messageContents parameter in bytes.
      uint8_t messageLength,
      // The message contents for which to create a signature. Per above notes,
      // this may include a leading portion of data not included in the
      // signature, in which case the last byte of this array should be set to
      // the index of the first byte to be considered for signing. Otherwise,
      // the last byte of messageContents should be 0x00 to indicate that a
      // signature should occur across the entire contents.
      uint8_t *messageContents);

// Callback
// The handler that returns the results of the signing operation. On success,
// the signature will be appended to the original message (including the
// signature type indicator that replaced the startIndex field for the signing)
// and both are returned via this callback.
void sl_zigbee_ezsp_dsa_sign_handler(
      // The result of the DSA signing operation.
      sl_status_t status,
      // The length of the messageContents parameter in bytes.
      uint8_t messageLength,
      // The message and attached which includes the original message and the
      // appended signature.
      uint8_t *messageContents);

// Verify that signature of the associated message digest was signed by the
// private key of the associated certificate.
sl_status_t sl_zigbee_ezsp_dsa_verify(
      // The AES-MMO message digest of the signed data. If dsaSign command was
      // used to generate the signature for this data, the final byte (replaced
      // by signature type of 0x01) in the messageContents array passed to
      // dsaSign is included in the hash context used for the digest
      // calculation.
      sl_zigbee_message_digest_t *digest,
      // The certificate of the signer. Note that the signer's certificate and
      // the verifier's certificate must both be issued by the same Certificate
      // Authority, so they should share the same CA Public Key.
      sl_zigbee_certificate_data_t *signerCertificate,
      // The signature of the signed data.
      sl_zigbee_signature_data_t *receivedSig);

// Callback
// This callback is executed by the stack when the DSA verification has
// completed and has a result. If the result is SL_STATUS_OK, the signature is
// valid. If the result is SL_STATUS_ZIGBEE_SIGNATURE_VERIFY_FAILURE then the
// signature is invalid. If the result is anything else then the signature
// verify operation failed and the validity is unknown.
void sl_zigbee_ezsp_dsa_verify_handler(
      // The result of the DSA verification operation.
      sl_status_t status);

// Verify that signature of the associated message digest was signed by the
// private key of the associated certificate.
sl_status_t sl_zigbee_ezsp_dsa_verify_283k1(
      // The AES-MMO message digest of the signed data. If dsaSign command was
      // used to generate the signature for this data, the final byte (replaced
      // by signature type of 0x01) in the messageContents array passed to
      // dsaSign is included in the hash context used for the digest
      // calculation.
      sl_zigbee_message_digest_t *digest,
      // The certificate of the signer. Note that the signer's certificate and
      // the verifier's certificate must both be issued by the same Certificate
      // Authority, so they should share the same CA Public Key.
      sl_zigbee_certificate_283k1_data_t *signerCertificate,
      // The signature of the signed data.
      sl_zigbee_signature_283k1_data_t *receivedSig);

// Sets the device's CA public key, local certificate, and static private key on
// the NCP associated with this node.
sl_status_t sl_zigbee_ezsp_set_preinstalled_cbke_data(
      // The Certificate Authority's public key.
      sl_zigbee_public_key_data_t *caPublic,
      // The node's new certificate signed by the CA.
      sl_zigbee_certificate_data_t *myCert,
      // The node's new static private key.
      sl_zigbee_private_key_data_t *myKey);

// Sets the device's 283k1 curve CA public key, local certificate, and static
// private key on the NCP associated with this node.
sl_status_t sl_zigbee_ezsp_save_preinstalled_cbke_data_283k1(void);

//------------------------------------------------------------------------------
// Mfglib Frames
//------------------------------------------------------------------------------

// Activate use of mfglib test routines and enables the radio receiver to report
// packets it receives to the mfgLibRxHandler() callback. These packets will not
// be passed up with a CRC failure. All other mfglib functions will return an
// error until the mfglibInternalStart() has been called
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t mfglibInternalStart(
      // true to generate a mfglibRxHandler callback when a packet is received.
      bool rxCallback);

// Deactivate use of mfglib test routines; restores the hardware to the state it
// was in prior to mfglibInternalStart() and stops receiving packets started by
// mfglibInternalStart() at the same time.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t mfglibInternalEnd(void);

// Starts transmitting an unmodulated tone on the currently set channel and
// power level. Upon successful return, the tone will be transmitting. To stop
// transmitting tone, application must call mfglibInternalStopTone(), allowing
// it the flexibility to determine its own criteria for tone duration (time,
// event, etc.)
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t mfglibInternalStartTone(void);

// Stops transmitting tone started by mfglibInternalStartTone().
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t mfglibInternalStopTone(void);

// Starts transmitting a random stream of characters. This is so that the radio
// modulation can be measured.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t mfglibInternalStartStream(void);

// Stops transmitting a random stream of characters started by
// mfglibInternalStartStream().
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t mfglibInternalStopStream(void);

// Sends a single packet consisting of the following bytes: packetLength,
// packetContents[0], ... , packetContents[packetLength - 3], CRC[0], CRC[1].
// The total number of bytes sent is packetLength + 1. The radio replaces the
// last two bytes of packetContents[] with the 16-bit CRC for the packet.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t mfglibInternalSendPacket(
      // The length of the packetContents parameter in bytes. Must be greater
      // than 3 and less than 123.
      uint8_t packetLength,
      // The packet to send. The last two bytes will be replaced with the 16-bit
      // CRC.
      uint8_t *packetContents);

// Sets the radio channel. Calibration occurs if this is the first time the
// channel has been used.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t mfglibInternalSetChannel(
      // The channel to switch to. Valid values are 11 to 26.
      uint8_t channel);

// Returns the current radio channel, as previously set via
// mfglibInternalSetChannel().
// Return: The current channel.
uint8_t mfglibInternalGetChannel(void);

// First select the transmit power mode, and then include a method for selecting
// the radio transmit power. The valid power settings depend upon the specific
// radio in use. Ember radios have discrete power settings, and then requested
// power is rounded to a valid power setting; the actual power output is
// available to the caller via mfglibInternalGetPower().
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t mfglibInternalSetPower(
      // Power mode. Refer to txPowerModes in stack/include/sl_zigbee_types.h
      // for possible values.
      uint16_t txPowerMode,
      // Power in units of dBm. Refer to radio data sheet for valid range.
      int8_t power);

// Returns the current radio power setting, as previously set via
// mfglibInternalSetPower().
// Return: Power in units of dBm. Refer to radio data sheet for valid range.
int8_t mfglibInternalGetPower(void);

// Callback
// A callback indicating a packet with a valid CRC has been received.
void sl_zigbee_ezsp_mfglib_rx_handler(
      // The link quality observed during the reception
      uint8_t linkQuality,
      // The energy level (in units of dBm) observed during the reception.
      int8_t rssi,
      // The length of the packetContents parameter in bytes. Will be greater
      // than 3 and less than 123.
      uint8_t packetLength,
      // The received packet (last 2 bytes are not FCS / CRC and may be
      // discarded)
      uint8_t *packetContents);

//------------------------------------------------------------------------------
// Bootloader Frames
//------------------------------------------------------------------------------

// Quits the current application and launches the standalone bootloader (if
// installed) The function returns an error if the standalone bootloader is not
// present
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_launch_standalone_bootloader(
      // If true, launch the standalone bootloader. If false, do nothing.
      bool enabled);

// Transmits the given bootload message to a neighboring node using a specific
// 802.15.4 header that allows the EmberZNet stack as well as the bootloader to
// recognize the message, but will not interfere with other ZigBee stacks.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_send_bootload_message(
      // If true, the destination address and pan id are both set to the
      // broadcast address.
      bool broadcast,
      // The EUI64 of the target node. Ignored if the broadcast field is set to
      // true.
      sl_802154_long_addr_t destEui64,
      // The length of the messageContents parameter in bytes.
      uint8_t messageLength,
      // The multicast message.
      uint8_t *messageContents);

// Detects if the standalone bootloader is installed, and if so returns the
// installed version. If not return 0xffff. A returned version of 0x1234 would
// indicate version 1.2 build 34. Also return the node's version of PLAT, MICRO
// and PHY.
// Return: BOOTLOADER_INVALID_VERSION if the standalone bootloader is not
// present, or the version of the installed standalone bootloader.
uint16_t sl_zigbee_ezsp_get_standalone_bootloader_version_plat_micro_phy(
      // Return: The value of PLAT on the node
      uint8_t *nodePlat,
      // Return: The value of MICRO on the node
      uint8_t *nodeMicro,
      // Return: The value of PHY on the node
      uint8_t *nodePhy);

// Callback
// A callback invoked by the EmberZNet stack when a bootload message is
// received.
void sl_zigbee_ezsp_incoming_bootload_message_handler(
      // The EUI64 of the sending node.
      sl_802154_long_addr_t longId,
      // Information about the incoming packet.
      sl_zigbee_rx_packet_info_t *packetInfo,
      // The length of the messageContents parameter in bytes.
      uint8_t messageLength,
      // The bootload message that was sent.
      uint8_t *messageContents);

// Callback
// A callback invoked by the EmberZNet stack when the MAC has finished
// transmitting a bootload message.
void sl_zigbee_ezsp_bootload_transmit_complete_handler(
      // An sl_status_t value of SL_STATUS_OK if an ACK was received from the
      // destination or SL_STATUS_ZIGBEE_DELIVERY_FAILED if no ACK was received.
      sl_status_t status,
      // The length of the messageContents parameter in bytes.
      uint8_t messageLength,
      // The message that was sent.
      uint8_t *messageContents);

// Perform AES encryption on plaintext using key.
void sl_zigbee_ezsp_aes_encrypt(
      // 16 bytes of plaintext.
      uint8_t *plaintext,
      // The 16-byte encryption key to use.
      uint8_t *key,
      // Return: 16 bytes of ciphertext.
      uint8_t *ciphertext);

// Callback
// A callback to be implemented on the Golden Node to process acknowledgements.
// If you supply a custom version of this handler, you must define
// SL_ZIGBEE_APPLICATION_HAS_INCOMING_MFG_TEST_MESSAGE_HANDLER in your
// application's CONFIGURATION_HEADER
void sl_zigbee_ezsp_incoming_mfg_test_message_handler(
      // The type of the incoming message. Currently, the only possibility is
      // MFG_TEST_TYPE_ACK.
      uint8_t messageType,
      // The length of the incoming message.
      uint8_t dataLength,
      // A pointer to the data received in the current message.
      uint8_t *data);

// A function used on the Golden Node to switch between normal network operation
// (for testing) and manufacturing configuration. Like emberSleep(), it may not
// be possible to execute this command due to pending network activity. For the
// transition from normal network operation to manufacturing configuration, it
// is customary to loop, calling this function alternately with emberTick()
// until the mode change succeeds.
// Return: An sl_status_t value indicating success or failure of the command.
sl_status_t sl_zigbee_ezsp_mfg_test_set_packet_mode(
      // Determines the new mode of operation. true causes the node to enter
      // manufacturing configuration. false causes the node to return to normal
      // network operation.
      bool beginConfiguration);

// A function used during manufacturing configuration on the Golden Node to send
// the DUT a reboot command. The usual practice is to execute this command at
// the end of manufacturing configuration, to place the DUT into normal network
// operation for testing. This function executes only during manufacturing
// configuration mode and returns an error otherwise. If successful, the DUT
// acknowledges the reboot command within 20 milliseconds and then reboots.
// Return: An sl_status_t value indicating success or failure of the command.
sl_status_t sl_zigbee_ezsp_mfg_test_send_reboot_command(void);

// A function used during manufacturing configuration on the Golden Node to set
// the DUT's 8-byte EUI ID. This function executes only during manufacturing
// configuration mode and returns an error otherwise. If successful, the DUT
// acknowledges the new EUI ID within 150 milliseconds.
// Return: An sl_status_t value indicating success or failure of the command.
sl_status_t sl_zigbee_ezsp_mfg_test_send_eui64(
      // The 8-byte EUID for the DUT.
      sl_802154_long_addr_t newId);

// A function used during manufacturing configuration on the Golden Node to set
// the DUT's 16-byte configuration string. This function executes only during
// manufacturing configuration mode and will return an error otherwise. If
// successful, the DUT will acknowledge the new string within 150 milliseconds.
// Return: An sl_status_t value indicating success or failure of the command.
sl_status_t sl_zigbee_ezsp_mfg_test_send_manufacturing_string(
      // The 16-byte manufacturing string.
      sl_zigbee_manufacturing_string_t newString);

// A function used during manufacturing configuration on the Golden Node to set
// the DUT's radio parameters. This function executes only during manufacturing
// configuration mode and returns an error otherwise. If successful, the DUT
// acknowledges the new parameters within 25 milliseconds.
// Return: An sl_status_t value indicating success or failure of the command.
sl_status_t sl_zigbee_ezsp_mfg_test_send_radio_parameters(
      // Sets the radio band for the DUT. See ember-common.h for possible
      // values.
      uint8_t supportedBands,
      // Sets the CC1020 crystal offset. This parameter has no effect on the
      // EM2420, and it may safely be set to 0 for this RFIC.
      int8_t crystalOffset);

// A function used in each of the manufacturing configuration API calls. Most
// implementations will not need to call this function directly. See mfg-test.c
// for more detail. This function executes only during manufacturing
// configuration mode and returns an error otherwise.
// Return: An sl_status_t value indicating success or failure of the command.
sl_status_t sl_zigbee_ezsp_mfg_test_send_command(
      // A pointer to the outgoing command string.
      uint8_t *command);

//------------------------------------------------------------------------------
// ZLL Frames
//------------------------------------------------------------------------------

// A consolidation of ZLL network operations with similar signatures;
// specifically, forming and joining networks or touch-linking.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_zll_network_ops(
      // Information about the network.
      sl_zigbee_zll_network_t *networkInfo,
      // Operation indicator.
      sl_zigbee_ezsp_zll_network_operation_t op,
      // Radio transmission power.
      int8_t radioTxPower);

// This call will cause the device to setup the security information used in its
// network. It must be called prior to forming, starting, or joining a network.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_zll_set_initial_security_state(
      // ZLL Network key.
      sl_zigbee_key_data_t *networkKey,
      // Initial security state of the network.
      sl_zigbee_zll_initial_security_state_t *securityState);

// This call will update ZLL security token information. Unlike
// sli_zigbee_stack_zll_set_initial_security_state, this can be called while a
// network is already established.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_zll_set_security_state_without_key(
      // Security state of the network.
      sl_zigbee_zll_initial_security_state_t *securityState);

// This call will initiate a ZLL network scan on all the specified channels.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_zll_start_scan(
      // The range of channels to scan.
      uint32_t channelMask,
      // The radio output power used for the scan requests.
      int8_t radioPowerForScan,
      // The node type of the local device.
      sl_zigbee_node_type_t nodeType);

// This call will change the mode of the radio so that the receiver is on for a
// specified amount of time when the device is idle.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_zll_set_rx_on_when_idle(
      // The duration in milliseconds to leave the radio on.
      uint32_t durationMs);

// Callback
// This call is fired when a ZLL network scan finds a ZLL network.
void sl_zigbee_ezsp_zll_network_found_handler(
      // Information about the network.
      sl_zigbee_zll_network_t *networkInfo,
      // Used to interpret deviceInfo field.
      bool isDeviceInfoNull,
      // Device specific information.
      sl_zigbee_zll_device_info_record_t *deviceInfo,
      // Information about the incoming packet received from this network.
      sl_zigbee_rx_packet_info_t *packetInfo);

// Callback
// This call is fired when a ZLL network scan is complete.
void sl_zigbee_ezsp_zll_scan_complete_handler(
      // Status of the operation.
      sl_status_t status);

// Callback
// This call is fired when network and group addresses are assigned to a remote
// mode in a network start or network join request.
void sl_zigbee_ezsp_zll_address_assignment_handler(
      // Address assignment information.
      sl_zigbee_zll_address_assignment_t *addressInfo,
      // Information about the incoming packet.
      sl_zigbee_rx_packet_info_t *packetInfo);

// Callback
// This call is fired when the device is a target of a touch link.
void sl_zigbee_ezsp_zll_touch_link_target_handler(
      // Information about the network.
      sl_zigbee_zll_network_t *networkInfo);

// Get the ZLL tokens.
void sl_zigbee_ezsp_zll_get_tokens(
      // Return: Data token return value.
      sl_zigbee_tok_type_stack_zll_data_t *data,
      // Return: Security token return value.
      sl_zigbee_tok_type_stack_zll_security_t *security);

// Set the ZLL data token.
void sl_zigbee_ezsp_zll_set_data_token(
      // Data token to be set.
      sl_zigbee_tok_type_stack_zll_data_t *data);

// Set the ZLL data token bitmask to reflect the ZLL network state.
void sl_zigbee_ezsp_zll_set_non_zll_network(void);

// Is this a ZLL network?
// Return: ZLL network?
bool sl_zigbee_ezsp_is_zll_network(void);

// This call sets the radio's default idle power mode.
void sl_zigbee_ezsp_zll_set_radio_idle_mode(
      // The power mode to be set.
      sl_zigbee_radio_power_mode_t mode);

// This call gets the radio's default idle power mode.
// Return: The current power mode.
uint8_t sl_zigbee_ezsp_zll_get_radio_idle_mode(void);

// This call sets the default node type for a factory new ZLL device.
void sl_zigbee_ezsp_set_zll_node_type(
      // The node type to be set.
      sl_zigbee_node_type_t nodeType);

// This call sets additional capability bits in the ZLL state.
void sl_zigbee_ezsp_set_zll_additional_state(
      // A mask with the bits to be set or cleared.
      uint16_t state);

// Is there a ZLL (Touchlink) operation in progress?
// Return: ZLL operation in progress?
bool sl_zigbee_ezsp_zll_operation_in_progress(void);

// Is the ZLL radio on when idle mode is active?
// Return: ZLL radio on when idle mode is active?
bool sl_zigbee_ezsp_zll_rx_on_when_idle_get_active(void);

// Informs the ZLL API that application scanning is complete
void sl_zigbee_ezsp_zll_scanning_complete(void);

// Get the primary ZLL (touchlink) channel mask.
// Return: The primary ZLL channel mask
uint32_t sl_zigbee_ezsp_get_zll_primary_channel_mask(void);

// Get the secondary ZLL (touchlink) channel mask.
// Return: The secondary ZLL channel mask
uint32_t sl_zigbee_ezsp_get_zll_secondary_channel_mask(void);

// Set the primary ZLL (touchlink) channel mask
void sl_zigbee_ezsp_set_zll_primary_channel_mask(
      // The primary ZLL channel mask
      uint32_t zllPrimaryChannelMask);

// Set the secondary ZLL (touchlink) channel mask.
void sl_zigbee_ezsp_set_zll_secondary_channel_mask(
      // The secondary ZLL channel mask
      uint32_t zllSecondaryChannelMask);

// Clear ZLL stack tokens.
void sl_zigbee_ezsp_zll_clear_tokens(void);

//------------------------------------------------------------------------------
// Green Power Frames
//------------------------------------------------------------------------------

// Update the GP Proxy table based on a GP pairing.
// Return: Whether a GP Pairing has been created or not.
bool sl_zigbee_ezsp_gp_proxy_table_process_gp_pairing(
      // The options field of the GP Pairing command.
      uint32_t options,
      // The target GPD.
      sl_zigbee_gp_address_t *addr,
      // The communication mode of the GP Sink.
      uint8_t commMode,
      // The network address of the GP Sink.
      uint16_t sinkNetworkAddress,
      // The group ID of the GP Sink.
      uint16_t sinkGroupId,
      // The alias assigned to the GPD.
      uint16_t assignedAlias,
      // The IEEE address of the GP Sink.
      uint8_t *sinkIeeeAddress,
      // The key to use for the target GPD.
      sl_zigbee_key_data_t *gpdKey,
      // The GPD security frame counter.
      uint32_t gpdSecurityFrameCounter,
      // The forwarding radius.
      uint8_t forwardingRadius);

// Adds/removes an entry from the GP Tx Queue.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_d_gp_send(
      // The action to perform on the GP TX queue (true to add, false to
      // remove).
      bool action,
      // Whether to use ClearChannelAssessment when transmitting the GPDF.
      bool useCca,
      // The Address of the destination GPD.
      sl_zigbee_gp_address_t *addr,
      // The GPD command ID to send.
      uint8_t gpdCommandId,
      // The length of the GP command payload.
      uint8_t gpdAsduLength,
      // The GP command payload.
      uint8_t *gpdAsdu,
      // The handle to refer to the GPDF.
      uint8_t gpepHandle,
      // How long to keep the GPDF in the TX Queue.
      uint16_t gpTxQueueEntryLifetimeMs);

// Callback
// A callback to the GP endpoint to indicate the result of the GPDF
// transmission.
void sl_zigbee_ezsp_d_gp_sent_handler(
      // An sl_status_t value indicating success or the reason for failure.
      sl_status_t status,
      // The handle of the GPDF.
      uint8_t gpepHandle);

// Callback
// A callback invoked by the ZigBee GP stack when a GPDF is received.
void sl_zigbee_ezsp_gpep_incoming_message_handler(
      // GP parameters list represented as a macro for GP endpoint incoming
      // message handler and callbacks prototypes.
      sl_zigbee_gp_params_t *param);

// Retrieves the proxy table entry stored at the passed index.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_gp_proxy_table_get_entry(
      // The index of the requested proxy table entry.
      uint8_t proxyIndex,
      // Return: An sl_zigbee_gp_proxy_table_entry_t struct containing a copy of
      // the requested proxy entry.
      sl_zigbee_gp_proxy_table_entry_t *entry);

// Finds the index of the passed address in the gp table.
// Return: The index, or 0xFF for not found
uint8_t sl_zigbee_ezsp_gp_proxy_table_lookup(
      // The address to search for
      sl_zigbee_gp_address_t *addr);

// Removes the proxy table entry stored at the passed index.
void sl_zigbee_ezsp_gp_proxy_table_remove_entry(
      // The index of the requested proxy table entry.
      uint8_t proxyIndex);

// Clear the entire proxy table
void sl_zigbee_ezsp_gp_clear_proxy_table(void);

// Retrieves the sink table entry stored at the passed index.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_gp_sink_table_get_entry(
      // The index of the requested sink table entry.
      uint8_t sinkIndex,
      // Return: An sl_zigbee_gp_sink_table_entry_t struct containing a copy of
      // the requested sink entry.
      sl_zigbee_gp_sink_table_entry_t *entry);

// Finds the index of the passed address in the gp table.
// Return: The index, or 0xFF for not found
uint8_t sl_zigbee_ezsp_gp_sink_table_lookup(
      // The address to search for.
      sl_zigbee_gp_address_t *addr);

// Retrieves the sink table entry stored at the passed index.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_gp_sink_table_set_entry(
      // The index of the requested sink table entry.
      uint8_t sinkIndex,
      // An sl_zigbee_gp_sink_table_entry_t struct containing a copy of the sink
      // entry to be updated.
      sl_zigbee_gp_sink_table_entry_t *entry);

// Removes the sink table entry stored at the passed index.
void sl_zigbee_ezsp_gp_sink_table_remove_entry(
      // The index of the requested sink table entry.
      uint8_t sinkIndex);

// Finds or allocates a sink entry
// Return: An index of found or allocated sink or 0xFF if failed.
uint8_t sl_zigbee_ezsp_gp_sink_table_find_or_allocate_entry(
      // An sl_zigbee_gp_address_t struct containing a copy of the gpd address
      // to be found.
      sl_zigbee_gp_address_t *addr);

// Clear the entire sink table
void sl_zigbee_ezsp_gp_sink_table_clear_all(void);

// Iniitializes Sink Table
void sl_zigbee_ezsp_gp_sink_table_init(void);

// Sets security framecounter in the sink table
void sl_zigbee_ezsp_gp_sink_table_set_security_frame_counter(
      // Index to the Sink table
      uint8_t index,
      // Security Frame Counter
      uint32_t sfc);

// Puts the GPS in commissioning mode.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_gp_sink_commission(
      // commissioning options
      uint8_t options,
      // gpm address for security.
      uint16_t gpmAddrForSecurity,
      // gpm address for pairing.
      uint16_t gpmAddrForPairing,
      // sink endpoint.
      uint8_t sinkEndpoint);

// Clears all entries within the translation table.
void sl_zigbee_ezsp_gp_translation_table_clear(void);

// Return number of active entries in sink table.
// Return: Number of active entries in sink table.
uint8_t sl_zigbee_ezsp_gp_sink_table_get_number_of_active_entries(void);

//------------------------------------------------------------------------------
// Token Interface Frames
//------------------------------------------------------------------------------

// Gets the total number of tokens.
// Return: Total number of tokens.
uint32_t sl_zigbee_ezsp_get_token_count(void);

// Gets the token information for a single token at provided index
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_get_token_info(
      // Index of the token in the token table for which information is needed.
      uint8_t index,
      // Return: Token information.
      sl_zigbee_token_info_t *tokenInfo);

// Gets the token data for a single token with provided key
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_get_token_data(
      // Key of the token in the token table for which data is needed.
      uint32_t token,
      // Index in case of the indexed token.
      uint32_t index,
      // Return: Token Data
      sl_zigbee_token_data_t *tokenData);

// Sets the token data for a single token with provided key
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_set_token_data(
      // Key of the token in the token table for which data is to be set.
      uint32_t token,
      // Index in case of the indexed token.
      uint32_t index,
      // Token Data
      sl_zigbee_token_data_t *tokenData);

// Reset the node by calling halReboot.
void sl_zigbee_ezsp_reset_node(void);

// Run GP security test vectors.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_gp_security_test_vectors(void);

// Factory reset all configured zigbee tokens
void sl_zigbee_ezsp_token_factory_reset(
      // Exclude network and APS outgoing frame counter tokens.
      bool excludeOutgoingFC,
      // Exclude stack boot counter token.
      bool excludeBootCounter);

//------------------------------------------------------------------------------
// Dynamic Hardware Configuration Frames
//------------------------------------------------------------------------------

// Read descriptor at index.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_read_pa_descriptor(
      // The PA index.
      uint8_t index,
      // Return: The PA descriptor.
      sl_zigbee_dhc_pa_descriptor_t *descriptor);

// Write descriptor at index.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_write_pa_descriptor(
      // The PA index.
      uint8_t index,
      // The PA descriptor.
      sl_zigbee_dhc_pa_descriptor_t *descriptor);

// Read one curve segment (algorithm=0).
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_read_pa_curve_segment(
      // The PA index.
      uint8_t index,
      // The segment index.
      uint8_t segment_index,
      // Return: Read PA Curve segment.
      sl_zigbee_dhc_pa_curve_segment_t *segment);

// Write one curve segment (algorithm=0).
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_write_pa_curve_segment(
      // The PA index.
      uint8_t index,
      // The segment index.
      uint8_t segment_index,
      // Write PA Curve segment.
      sl_zigbee_dhc_pa_curve_segment_t *segment);

// Read full curve for descriptor index (algorithm=0).
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_read_pa_curve(
      // The PA index.
      uint8_t index,
      // Return: Read PA Curve.
      sl_zigbee_dhc_pa_curve_t *curve);

// Write full curve (algorithm=0).
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_write_pa_curve(
      // The PA index.
      uint8_t index,
      // Write PA Curve.
      sl_zigbee_dhc_pa_curve_t *curve);

// Read table for descriptor index (algorithm=1).
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_read_pa_table(
      // The PA index.
      uint8_t index,
      // Return: Read PA Table.
      sl_zigbee_dhc_pa_table_t *table);

// Write table (algorithm=1).
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_write_pa_table(
      // The PA index.
      uint8_t index,
      // Write PA Table.
      sl_zigbee_dhc_pa_table_t *table);

// Read RSSI offset.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_read_rssi_offset(
      // Return: Read RSSI offset.
      sl_zigbee_dhc_rssi_offset_t *rssi_offset);

// Write RSSI offset.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_write_rssi_offset(
      // Write RSSI offset.
      sl_zigbee_dhc_rssi_offset_t *rssi_offset);

// Read PA voltage.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_read_pa_voltage(
      // Return: Read PA voltage.
      uint16_t *pa_voltage);

// Write PA voltage (keep consistent with metadata).
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_write_pa_voltage(
      // Write PA voltage.
      uint16_t pa_voltage);

// Read PA mode.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_read_pa_mode(
      // Return: Read PA mode.
      sl_zigbee_dhc_pa_mode_t *pa_mode);

// Write PA mode.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_write_pa_mode(
      // PA mode value to write.
      sl_zigbee_dhc_pa_mode_t *pa_mode);

// Read CTUNE value.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_read_ctune(
      // Return: Read CTUNE value.
      sl_zigbee_dhc_ctune_t *ctune);

// Write CTUNE value.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_write_ctune(
      // Write CTUNE value.
      sl_zigbee_dhc_ctune_t *ctune);

// Read DHC top-level version.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_read_dhc_version(
      // Return: Read DHC top-level version.
      uint8_t *dhc_version);

// Write DHC top-level version.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_write_dhc_version(
      // Write DHC top-level version.
      uint8_t dhc_version);

// Read PA dataset version.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_read_pa_version(
      // Return: Read PA dataset version.
      sl_zigbee_dhc_pa_version_t *pa_version);

// Read PA signature (metadata.signature mirror).
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_read_pa_signature(
      // Return: Read PA signature (metadata.signature mirror).
      sl_zigbee_dhc_pa_signature_t *pa_signature);

// Write PA signature (updates metadata signature).
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_write_pa_signature(
      // Write PA signature (updates metadata signature).
      sl_zigbee_dhc_pa_signature_t *pa_signature);

// Read PA calibration metadata.
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_read_pa_metadata(
      // Return: Read PA calibration metadata block.
      sl_zigbee_dhc_pa_metadata_t *metadata);

// Write PA calibration metadata (must precede descriptor/data writes).
// Return: An sl_status_t value indicating success or the reason for failure.
sl_status_t sl_zigbee_ezsp_write_pa_metadata(
      // The PA metadata.
      sl_zigbee_dhc_pa_metadata_t *metadata);
