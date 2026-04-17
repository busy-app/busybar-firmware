// File: ezsp-enum.h
//
// *** Generated file. Do not edit! ***
// 
// Description: Enumerations for EZSP.
// 
// Copyright 2007 by Ember Corporation. All rights reserved.                *80*

#ifndef __EZSP_ENUM_H__
#define __EZSP_ENUM_H__

//------------------------------------------------------------------------------
// Identifies a configuration value.
typedef uint8_t sl_zigbee_ezsp_config_id_t;

enum {
  // The NCP no longer supports configuration of packet buffer heap at runtime
  // using this parameter. Packet buffer heap space must be configured using the
  // SL_ZIGBEE_PACKET_BUFFER_HEAP_SIZE macro when building the NCP project.
  SL_ZIGBEE_EZSP_CONFIG_PACKET_BUFFER_HEAP_SIZE = 0x01,
  // The maximum number of router neighbors the stack can keep track of. A
  // neighbor is a node within radio range.
  SL_ZIGBEE_EZSP_CONFIG_NEIGHBOR_TABLE_SIZE     = 0x02,
  // The maximum number of APS retried messages the stack can be transmitting at
  // any time.
  SL_ZIGBEE_EZSP_CONFIG_APS_UNICAST_MESSAGE_COUNT = 0x03,
  // The maximum number of non-volatile bindings supported by the stack.
  SL_ZIGBEE_EZSP_CONFIG_BINDING_TABLE_SIZE      = 0x04,
  // The maximum number of EUI64 to network address associations that the stack
  // can maintain for the application. (Note, the total number of such address
  // associations maintained by the NCP is the sum of the value of this setting
  // and the value of ::SL_ZIGBEE_EZSP_CONFIG_TRUST_CENTER_ADDRESS_CACHE_SIZE.
  SL_ZIGBEE_EZSP_CONFIG_ADDRESS_TABLE_SIZE      = 0x05,
  // The maximum number of multicast groups that the device may be a member of.
  SL_ZIGBEE_EZSP_CONFIG_MULTICAST_TABLE_SIZE    = 0x06,
  // The maximum number of destinations to which a node can route messages. This
  // includes both messages originating at this node and those relayed for
  // others.
  SL_ZIGBEE_EZSP_CONFIG_ROUTE_TABLE_SIZE        = 0x07,
  // The number of simultaneous route discoveries that a node will support.
  SL_ZIGBEE_EZSP_CONFIG_DISCOVERY_TABLE_SIZE    = 0x08,
  // Specifies the stack profile.
  SL_ZIGBEE_EZSP_CONFIG_STACK_PROFILE           = 0x0C,
  // The security level used for security at the MAC and network layers. The
  // supported values are 0 (no security) and 5 (payload is encrypted and a
  // four-byte MIC is used for authentication).
  SL_ZIGBEE_EZSP_CONFIG_SECURITY_LEVEL          = 0x0D,
  // The maximum number of hops for a message.
  SL_ZIGBEE_EZSP_CONFIG_MAX_HOPS                = 0x10,
  // The maximum number of end device children that a router will support.
  SL_ZIGBEE_EZSP_CONFIG_MAX_END_DEVICE_CHILDREN = 0x11,
  // The maximum amount of time that the MAC will hold a message for indirect
  // transmission to a child.
  SL_ZIGBEE_EZSP_CONFIG_INDIRECT_TRANSMISSION_TIMEOUT = 0x12,
  // The maximum amount of time that an end device child can wait between polls.
  // If no poll is heard within this timeout, then the parent removes the end
  // device from its tables. Value range 0-14. The timeout corresponding to a
  // value of zero is 10 seconds. The timeout corresponding to a nonzero value N
  // is 2^N minutes, ranging from 2^1 = 2 minutes to 2^14 = 16384 minutes.
  SL_ZIGBEE_EZSP_CONFIG_END_DEVICE_POLL_TIMEOUT = 0x13,
  // Enables boost power mode and/or the alternate transmitter output.
  SL_ZIGBEE_EZSP_CONFIG_TX_POWER_MODE           = 0x17,
  // 0: Allow this node to relay messages. 1: Prevent this node from relaying
  // messages.
  SL_ZIGBEE_EZSP_CONFIG_DISABLE_RELAY           = 0x18,
  // The maximum number of EUI64 to network address associations that the Trust
  // Center can maintain. These address cache entries are reserved for and
  // reused by the Trust Center when processing device join/rejoin
  // authentications. This cache size limits the number of overlapping joins the
  // Trust Center can process within a narrow time window (e.g. two seconds),
  // and thus should be set to the maximum number of near simultaneous joins the
  // Trust Center is expected to accommodate. (Note, the total number of such
  // address associations maintained by the NCP is the sum of the value of this
  // setting and the value of ::SL_ZIGBEE_EZSP_CONFIG_ADDRESS_TABLE_SIZE.)
  SL_ZIGBEE_EZSP_CONFIG_TRUST_CENTER_ADDRESS_CACHE_SIZE = 0x19,
  // The size of the source route table.
  SL_ZIGBEE_EZSP_CONFIG_SOURCE_ROUTE_TABLE_SIZE = 0x1A,
  // The number of blocks of a fragmented message that can be sent in a single
  // window.
  SL_ZIGBEE_EZSP_CONFIG_FRAGMENT_WINDOW_SIZE    = 0x1C,
  // The time the stack will wait (in milliseconds) between sending blocks of a
  // fragmented message.
  SL_ZIGBEE_EZSP_CONFIG_FRAGMENT_DELAY_MS       = 0x1D,
  // The size of the Key Table used for storing individual link keys (if the
  // device is a Trust Center) or Application Link Keys (if the device is a
  // normal node).
  SL_ZIGBEE_EZSP_CONFIG_KEY_TABLE_SIZE          = 0x1E,
  // The APS ACK timeout value. The stack waits this amount of time between
  // resends of APS retried messages.
  SL_ZIGBEE_EZSP_CONFIG_APS_ACK_TIMEOUT         = 0x1F,
  // The duration of a beacon jitter, in the units used by the 15.4 scan
  // parameter (((1 << duration) + 1) * 15ms), when responding to a beacon
  // request.
  SL_ZIGBEE_EZSP_CONFIG_BEACON_JITTER_DURATION  = 0x20,
  // The number of PAN id conflict reports that must be received by the network
  // manager within one minute to trigger a PAN id change.
  SL_ZIGBEE_EZSP_CONFIG_PAN_ID_CONFLICT_REPORT_THRESHOLD = 0x22,
  // The timeout value in minutes for how long the Trust Center or a normal node
  // waits for the ZigBee Request Key to complete. On the Trust Center this
  // controls whether or not the device buffers the request, waiting for a
  // matching pair of ZigBee Request Key. If the value is non-zero, the Trust
  // Center buffers and waits for that amount of time. If the value is zero, the
  // Trust Center does not buffer the request and immediately responds to the
  // request. Zero is the most compliant behavior.
  SL_ZIGBEE_EZSP_CONFIG_REQUEST_KEY_TIMEOUT     = 0x24,
  // This value indicates the size of the runtime modifiable certificate table.
  // Normally certificates are stored in MFG tokens but this table can be used
  // to field upgrade devices with new Smart Energy certificates. This value
  // cannot be set, it can only be queried.
  SL_ZIGBEE_EZSP_CONFIG_CERTIFICATE_TABLE_SIZE  = 0x29,
  // This is a bitmask that controls which incoming ZDO request messages are
  // passed to the application. The bits are defined in the
  // sl_zigbee_zdo_configuration_flags_t enumeration. To see if the application
  // is required to send a ZDO response in reply to an incoming message, the
  // application must check the APS options bitfield within the
  // incomingMessageHandler callback to see if the
  // SL_ZIGBEE_APS_OPTION_ZDO_RESPONSE_REQUIRED flag is set.
  SL_ZIGBEE_EZSP_CONFIG_APPLICATION_ZDO_FLAGS   = 0x2A,
  // The maximum number of broadcasts during a single broadcast timeout period.
  SL_ZIGBEE_EZSP_CONFIG_BROADCAST_TABLE_SIZE    = 0x2B,
  // The size of the MAC filter list table.
  SL_ZIGBEE_EZSP_CONFIG_MAC_FILTER_TABLE_SIZE   = 0x2C,
  // The number of supported networks.
  SL_ZIGBEE_EZSP_CONFIG_SUPPORTED_NETWORKS      = 0x2D,
  // Whether multicasts are sent to the RxOnWhenIdle=true address (0xFFFD) or
  // the sleepy broadcast address (0xFFFF). The RxOnWhenIdle=true address is the
  // ZigBee compliant destination for multicasts.
  SL_ZIGBEE_EZSP_CONFIG_SEND_MULTICASTS_TO_SLEEPY_ADDRESS = 0x2E,
  // ZLL group address initial configuration.
  SL_ZIGBEE_EZSP_CONFIG_ZLL_GROUP_ADDRESSES     = 0x2F,
  // ZLL rssi threshold initial configuration.
  SL_ZIGBEE_EZSP_CONFIG_ZLL_RSSI_THRESHOLD      = 0x30,
  // Toggles the MTORR flow control in the stack.
  SL_ZIGBEE_EZSP_CONFIG_MTORR_FLOW_CONTROL      = 0x33,
  // Setting the retry queue size. Applies to all queues. Default value in the
  // sample applications is 16.
  SL_ZIGBEE_EZSP_CONFIG_RETRY_QUEUE_SIZE        = 0x34,
  // Setting the new broadcast entry threshold. The number (BROADCAST_TABLE_SIZE
  // - NEW_BROADCAST_ENTRY_THRESHOLD) of broadcast table entries are reserved
  // for relaying the broadcast messages originated on other devices. The local
  // device will fail to originate a broadcast message after this threshold is
  // reached. Setting this value to BROADCAST_TABLE_SIZE and greater will
  // effectively kill this limitation.
  SL_ZIGBEE_EZSP_CONFIG_NEW_BROADCAST_ENTRY_THRESHOLD = 0x35,
  //  The length of time, in seconds, that a trust center will store a transient
  // link key that a device can use to join its network. A transient key is
  // added with a call to emberAddTransientLinkKey. After the transient key is
  // added, it will be removed once this amount of time has passed. A joining
  // device will not be able to use that key to join until it is added again on
  // the trust center. The default value is 300 seconds, i.e., 5 minutes.
  SL_ZIGBEE_EZSP_CONFIG_TRANSIENT_KEY_TIMEOUT_S = 0x36,
  // The number of passive acknowledgements to record from neighbors before we
  // stop re-transmitting broadcasts
  SL_ZIGBEE_EZSP_CONFIG_BROADCAST_MIN_ACKS_NEEDED = 0x37,
  // The length of time, in seconds, that a trust center will allow a Trust
  // Center (insecure) rejoin for a device that is using the well-known link
  // key. This timeout takes effect once rejoins using the well-known key has
  // been allowed. This command updates the
  // sli_zigbee_allow_tc_rejoins_using_well_known_key_timeout_sec value.
  SL_ZIGBEE_EZSP_CONFIG_TC_REJOINS_USING_WELL_KNOWN_KEY_TIMEOUT_S = 0x38,
  // Valid range of a CTUNE value is 0x0000-0x01FF. Higher order bits (0xFE00)
  // of the 16-bit value are ignored.
  SL_ZIGBEE_EZSP_CONFIG_CTUNE_VALUE             = 0x39,
  // To configure non trust center node to assume a concentrator type of the
  // trust center it join to, until it receive many-to-one route request from
  // the trust center. For the trust center node, concentrator type is
  // configured from the concentrator plugin. The stack by default assumes trust
  // center be a low RAM concentrator that make other devices send route record
  // to the trust center even without receiving a many-to-one route request. The
  // default concentrator type can be changed by setting appropriate
  // sl_zigbee_assume_trust_center_concentrator_type_t config value.
  SL_ZIGBEE_EZSP_CONFIG_ASSUME_TC_CONCENTRATOR_TYPE = 0x40,
  // This is green power proxy table size. This value is read-only and cannot be
  // set at runtime
  SL_ZIGBEE_EZSP_CONFIG_GP_PROXY_TABLE_SIZE     = 0x41,
  // This is green power sink table size. This value is read-only and cannot be
  // set at runtime
  SL_ZIGBEE_EZSP_CONFIG_GP_SINK_TABLE_SIZE      = 0x42,
  // This is The configuration advertised by the end device to the parent when
  // joining/rejoining, either SL_ZIGBEE_END_DEVICE_CONFIG_NONE or
  // SL_ZIGBEE_END_DEVICE_CONFIG_PERSIST_DATA_ON_PARENT.
  SL_ZIGBEE_EZSP_CONFIG_END_DEVICE_CONFIGURATION = 0x43
};
#define SL_ZIGBEE_EZSP_CONFIG_ID_MAX  0x43

//------------------------------------------------------------------------------
// Identifies a value.
typedef uint8_t sl_zigbee_ezsp_value_id_t;

enum {
  // The contents of the node data stack token.
  SL_ZIGBEE_EZSP_VALUE_TOKEN_STACK_NODE_DATA    = 0x00,
  // The types of MAC passthrough messages that the host wishes to receive.
  SL_ZIGBEE_EZSP_VALUE_MAC_PASSTHROUGH_FLAGS    = 0x01,
  // The source address used to filter legacy EmberNet messages when the
  // SL_802154_PASSTHROUGH_EMBERNET_SOURCE flag is set in
  // SL_ZIGBEE_EZSP_VALUE_MAC_PASSTHROUGH_FLAGS.
  SL_ZIGBEE_EZSP_VALUE_EMBERNET_PASSTHROUGH_SOURCE_ADDRESS = 0x02,
  // The amount in bytes (max 2^16) of available general purpose heap memory
  SL_ZIGBEE_EZSP_VALUE_BUFFER_HEAP_FREE_SIZE    = 0x03,
  // Selects sending synchronous callbacks in ezsp-uart.
  SL_ZIGBEE_EZSP_VALUE_UART_SYNCH_CALLBACKS     = 0x04,
  // The maximum incoming transfer size for the local node. Default value is set
  // to 82 and does not use fragmentation. Sets the value in Node Descriptor. To
  // set, this takes the input of a uint8 array of length 2 where you pass the
  // lower byte at index 0 and upper byte at index 1.
  SL_ZIGBEE_EZSP_VALUE_MAXIMUM_INCOMING_TRANSFER_SIZE = 0x05,
  // The maximum outgoing transfer size for the local node. Default value is set
  // to 82 and does not use fragmentation. Sets the value in Node Descriptor. To
  // set, this takes the input of a uint8 array of length 2 where you pass the
  // lower byte at index 0 and upper byte at index 1.
  SL_ZIGBEE_EZSP_VALUE_MAXIMUM_OUTGOING_TRANSFER_SIZE = 0x06,
  // A bool indicating whether stack tokens are written to persistent storage as
  // they change.
  SL_ZIGBEE_EZSP_VALUE_STACK_TOKEN_WRITING      = 0x07,
  // A read-only value indicating whether the stack is currently performing a
  // rejoin.
  SL_ZIGBEE_EZSP_VALUE_STACK_IS_PERFORMING_REJOIN = 0x08,
  // A list of sl_zigbee_mac_filter_match_data_t values.
  SL_ZIGBEE_EZSP_VALUE_MAC_FILTER_LIST          = 0x09,
  // The Ember Extended Security Bitmask.
  SL_ZIGBEE_EZSP_VALUE_EXTENDED_SECURITY_BITMASK = 0x0A,
  // The node short ID.
  SL_ZIGBEE_EZSP_VALUE_NODE_SHORT_ID            = 0x0B,
  // The descriptor capability of the local node. Write only.
  SL_ZIGBEE_EZSP_VALUE_DESCRIPTOR_CAPABILITY    = 0x0C,
  // The stack device request sequence number of the local node.
  SL_ZIGBEE_EZSP_VALUE_STACK_DEVICE_REQUEST_SEQUENCE_NUMBER = 0x0D,
  // Enable or disable radio hold-off.
  SL_ZIGBEE_EZSP_VALUE_RADIO_HOLD_OFF           = 0x0E,
  // The flags field associated with the endpoint data.
  SL_ZIGBEE_EZSP_VALUE_ENDPOINT_FLAGS           = 0x0F,
  // Enable/disable the Mfg security config key settings.
  SL_ZIGBEE_EZSP_VALUE_MFG_SECURITY_CONFIG      = 0x10,
  // Retrieves the version information from the stack on the NCP.
  SL_ZIGBEE_EZSP_VALUE_VERSION_INFO             = 0x11,
  // This is the reason that the last rejoin took place. This value may only be
  // retrieved, not set. The rejoin may have been initiated by the stack (NCP)
  // or the application (host). If a host initiated a rejoin the reason will be
  // set by default to SL_ZIGBEE_REJOIN_DUE_TO_APP_EVENT_1. If the application
  // wishes to denote its own rejoin reasons it can do so by calling
  // sl_zigbee_ezsp_set_value(EMBER_VALUE_HOST_REJOIN_REASON,
  // SL_ZIGBEE_REJOIN_DUE_TO_APP_EVENT_X). X is a number corresponding to one of
  // the app events defined. If the NCP initiated a rejoin it will record this
  // value internally for retrieval by
  // sl_zigbee_ezsp_get_value(EZSP_VALUE_REAL_REJOIN_REASON).
  SL_ZIGBEE_EZSP_VALUE_LAST_REJOIN_REASON       = 0x13,
  // The next ZigBee sequence number.
  SL_ZIGBEE_EZSP_VALUE_NEXT_ZIGBEE_SEQUENCE_NUMBER = 0x14,
  // CCA energy detect threshold for radio.
  SL_ZIGBEE_EZSP_VALUE_CCA_THRESHOLD            = 0x15,
  // The threshold value for a counter
  SL_ZIGBEE_EZSP_VALUE_SET_COUNTER_THRESHOLD    = 0x17,
  // Resets all counters thresholds to 0xFF
  SL_ZIGBEE_EZSP_VALUE_RESET_COUNTER_THRESHOLDS = 0x18,
  // Clears all the counters
  SL_ZIGBEE_EZSP_VALUE_CLEAR_COUNTERS           = 0x19,
  // The node's new certificate signed by the CA.
  SL_ZIGBEE_EZSP_VALUE_CERTIFICATE_283K1        = 0x1A,
  // The Certificate Authority's public key.
  SL_ZIGBEE_EZSP_VALUE_PUBLIC_KEY_283K1         = 0x1B,
  // The node's new static private key.
  SL_ZIGBEE_EZSP_VALUE_PRIVATE_KEY_283K1        = 0x1C,
  // The NWK layer security frame counter value
  SL_ZIGBEE_EZSP_VALUE_NWK_FRAME_COUNTER        = 0x23,
  // The APS layer security frame counter value. Managed by the stack. Users
  // should not set these unless doing backup and restore.
  SL_ZIGBEE_EZSP_VALUE_APS_FRAME_COUNTER        = 0x24,
  // Sets the device type to use on the next rejoin using device type
  SL_ZIGBEE_EZSP_VALUE_RETRY_DEVICE_TYPE        = 0x25,
  // Setting this byte enables R21 behavior on the NCP.
  SL_ZIGBEE_EZSP_VALUE_ENABLE_R21_BEHAVIOR      = 0x29,
  // Configure the antenna mode(0-don't switch,1-primary,2-secondary,3-TX
  // antenna diversity).
  SL_ZIGBEE_EZSP_VALUE_ANTENNA_MODE             = 0x30,
  // Enable or disable packet traffic arbitration.
  SL_ZIGBEE_EZSP_VALUE_ENABLE_PTA               = 0x31,
  // Set packet traffic arbitration configuration options.
  SL_ZIGBEE_EZSP_VALUE_PTA_OPTIONS              = 0x32,
  // Configure manufacturing library options (0-non-CSMA transmits,1-CSMA
  // transmits). To be used with Manufacturing Library.
  SL_ZIGBEE_EZSP_VALUE_MFGLIB_OPTIONS           = 0x33,
  // Sets the flag to use either negotiated power by link power delta (LPD) or
  // fixed power value provided by user while forming/joining a network for
  // packet transmissions on sub-ghz interface. This is mainly for testing
  // purposes.
  SL_ZIGBEE_EZSP_VALUE_USE_NEGOTIATED_POWER_BY_LPD = 0x34,
  // Set packet traffic arbitration PWM options.
  SL_ZIGBEE_EZSP_VALUE_PTA_PWM_OPTIONS          = 0x35,
  // Set packet traffic arbitration directional priority pulse width in
  // microseconds.
  SL_ZIGBEE_EZSP_VALUE_PTA_DIRECTIONAL_PRIORITY_PULSE_WIDTH = 0x36,
  // Set packet traffic arbitration phy select timeout(ms).
  SL_ZIGBEE_EZSP_VALUE_PTA_PHY_SELECT_TIMEOUT   = 0x37,
  // Configure the RX antenna mode: (0-do not switch; 1-primary; 2-secondary;
  // 3-RX antenna diversity).
  SL_ZIGBEE_EZSP_VALUE_ANTENNA_RX_MODE          = 0x38,
  // Configure the timeout to wait for the network key before failing a join.
  // Acceptable timeout range [3,255]. Value is in seconds.
  SL_ZIGBEE_EZSP_VALUE_NWK_KEY_TIMEOUT          = 0x39,
  // The number of failed CSMA attempts due to failed CCA made by the MAC before
  // continuing transmission with CCA disabled.  This is the same as calling the
  // sli_zigbee_stack_force_tx_after_failed_cca(uint8_t csmaAttempts) API. A
  // value of 0 disables the feature.
  SL_ZIGBEE_EZSP_VALUE_FORCE_TX_AFTER_FAILED_CCA_ATTEMPTS = 0x3A,
  // The length of time, in seconds, that a trust center will store a transient
  // link key that a device can use to join its network. A transient key is
  // added with a call to sli_zigbee_stack_sec_man_import_transient_key. After
  // the transient key is added, it will be removed once this amount of time has
  // passed. A joining device will not be able to use that key to join until it
  // is added again on the trust center. The default value is 300 seconds (5
  // minutes).
  SL_ZIGBEE_EZSP_VALUE_TRANSIENT_KEY_TIMEOUT_S  = 0x3B,
  // Cumulative energy usage metric since the last value reset of the coulomb
  // counter plugin. Setting this value will reset the coulomb counter.
  SL_ZIGBEE_EZSP_VALUE_COULOMB_COUNTER_USAGE    = 0x3C,
  // When scanning, configure the maximum number of beacons to store in cache.
  // Each beacon consumes on average 32-bytes (+ buffer overhead) in RAM.
  SL_ZIGBEE_EZSP_VALUE_MAX_BEACONS_TO_STORE     = 0x3D,
  // Set the mask to filter out unacceptable child timeout options on a router.
  SL_ZIGBEE_EZSP_VALUE_END_DEVICE_TIMEOUT_OPTIONS_MASK = 0x3E,
  // The end device keep-alive mode supported by the parent.
  SL_ZIGBEE_EZSP_VALUE_END_DEVICE_KEEP_ALIVE_SUPPORT_MODE = 0x3F,
  // Return the active radio config. Read only. Values are 0: Default, 1:
  // Antenna Diversity, 2: Co-Existence, 3: Antenna Diversity and Co-Existence.
  SL_ZIGBEE_EZSP_VALUE_ACTIVE_RADIO_CONFIG      = 0x41,
  // Return the number of seconds the network will remain open. A return value
  // of 0 indicates that the network is closed. Read only.
  SL_ZIGBEE_EZSP_VALUE_NWK_OPEN_DURATION        = 0x42,
  // Timeout in milliseconds to store entries in the transient device table. If
  // the devices are not authenticated before the timeout, the entry shall be
  // purged
  SL_ZIGBEE_EZSP_VALUE_TRANSIENT_DEVICE_TIMEOUT = 0x43,
  // Return information about the key storage on an NCP.  Returns 0 if keys are
  // in classic key storage, and 1 if they are located in PSA key storage. Read
  // only.
  SL_ZIGBEE_EZSP_VALUE_KEY_STORAGE_VERSION      = 0x44,
  // Return activation state about TC Delayed Join on an NCP.  A return value of
  // 0 indicates that the feature is not activated.
  SL_ZIGBEE_EZSP_VALUE_DELAYED_JOIN_ACTIVATION  = 0x45,
  // The maximum number of NWK retries that will be attempted.
  SL_ZIGBEE_EZSP_VALUE_MAX_NWK_RETRIES          = 0x46,
  // Policies for allowing/disallowing rejoins.
  SL_ZIGBEE_EZSP_VALUE_REJOIN_MODE              = 0x47,
  // Controls whether devices must use an install code when joining.
  SL_ZIGBEE_EZSP_VALUE_JOIN_USE_INSTALL_CODE_ENABLE = 0x48,
  // The stack revision.
  SL_ZIGBEE_EZSP_VALUE_STACK_REVISION           = 0x49
};
#define SL_ZIGBEE_EZSP_VALUE_ID_MAX  0x49

//------------------------------------------------------------------------------
// Identifies a value based on specified characteristics. Each set of
// characteristics is unique to that value and is specified during the call to
// get the extended value.
typedef uint8_t sl_zigbee_ezsp_extended_value_id_t;

enum {
  // The flags field associated with the specified endpoint.
  SL_ZIGBEE_EZSP_EXTENDED_VALUE_ENDPOINT_FLAGS  = 0x00,
  // This is the reason for the node to leave the network as well as the device
  // that told it to leave. The leave reason is the 1st byte of the value while
  // the node ID is the 2nd and 3rd byte. If the leave was caused due to an API
  // call rather than an over the air message, the node ID will be
  // SL_ZIGBEE_UNKNOWN_NODE_ID (0xFFFD).
  SL_ZIGBEE_EZSP_EXTENDED_VALUE_LAST_LEAVE_REASON = 0x01,
  // This number of bytes of overhead required in the network frame for source
  // routing to a particular destination.
  SL_ZIGBEE_EZSP_EXTENDED_VALUE_GET_SOURCE_ROUTE_OVERHEAD = 0x02,
  // These values are current or boot-time metrics gathered by the memory
  // manager/buffer manager.
  SL_ZIGBEE_EZSP_EXTENDED_VALUE_MEMORY_USAGE_DATA = 0x03
};
#define SL_ZIGBEE_EZSP_EXTENDED_VALUE_ID_MAX  0x03

//------------------------------------------------------------------------------
// Flags associated with the endpoint data configured on the NCP.
typedef uint16_t sl_zigbee_ezsp_endpoint_flags_t;

enum {
  // Indicates that the endpoint is disabled and NOT discoverable via ZDO.
  SL_ZIGBEE_EZSP_ENDPOINT_DISABLED              = 0x00,
  // Indicates that the endpoint is enabled and discoverable via ZDO.
  SL_ZIGBEE_EZSP_ENDPOINT_ENABLED               = 0x01
};
#define SL_ZIGBEE_EZSP_ENDPOINT_FLAGS_MAX  0x01

//------------------------------------------------------------------------------
// Identifies a policy.
typedef uint8_t sl_zigbee_ezsp_policy_id_t;

enum {
  // Controls trust center behavior.
  SL_ZIGBEE_EZSP_TRUST_CENTER_POLICY            = 0x00,
  // Controls how external binding modification requests are handled.
  SL_ZIGBEE_EZSP_BINDING_MODIFICATION_POLICY    = 0x01,
  // Controls whether the Host supplies unicast replies.
  SL_ZIGBEE_EZSP_UNICAST_REPLIES_POLICY         = 0x02,
  // Controls whether pollHandler callbacks are generated.
  SL_ZIGBEE_EZSP_POLL_HANDLER_POLICY            = 0x03,
  // Controls whether the message contents are included in the
  // messageSentHandler callback.
  SL_ZIGBEE_EZSP_MESSAGE_CONTENTS_IN_CALLBACK_POLICY = 0x04,
  // Controls whether the Trust Center will respond to Trust Center link key
  // requests.
  SL_ZIGBEE_EZSP_TC_KEY_REQUEST_POLICY          = 0x05,
  // Controls whether the Trust Center will respond to application link key
  // requests.
  SL_ZIGBEE_EZSP_APP_KEY_REQUEST_POLICY         = 0x06,
  // Controls whether ZigBee packets that appear invalid are automatically
  // dropped by the stack. A counter will be incremented when this occurs.
  SL_ZIGBEE_EZSP_PACKET_VALIDATE_LIBRARY_POLICY = 0x07,
  // Controls whether the stack will process ZLL messages.
  SL_ZIGBEE_EZSP_ZLL_POLICY                     = 0x08,
  // Controls whether Trust Center (insecure) rejoins for devices using the
  // well-known link key are accepted. If rejoining using the well-known key is
  // allowed, it is disabled again after
  // sli_zigbee_allow_tc_rejoins_using_well_known_key_timeout_sec seconds.
  SL_ZIGBEE_EZSP_TC_REJOINS_USING_WELL_KNOWN_KEY_POLICY = 0x09
};
#define SL_ZIGBEE_EZSP_POLICY_ID_MAX  0x09

//------------------------------------------------------------------------------
// This is the policy decision bitmask that controls the trust center decision
// strategies. The bitmask is modified and extracted from the
// sl_zigbee_ezsp_decision_id_t for supporting bitmask operations.
typedef uint16_t sl_zigbee_ezsp_decision_bitmask_t;

enum {
  // Disallow joins and rejoins.
  SL_ZIGBEE_EZSP_DECISION_BITMASK_DEFAULT_CONFIGURATION = 0x0000,
  // Send the network key to all joining devices.
  SL_ZIGBEE_EZSP_DECISION_ALLOW_JOINS           = 0x0001,
  // Send the network key to all rejoining devices.
  SL_ZIGBEE_EZSP_DECISION_ALLOW_UNSECURED_REJOINS = 0x0002,
  // Send the network key in the clear.
  SL_ZIGBEE_EZSP_DECISION_SEND_KEY_IN_CLEAR     = 0x0004,
  // Do nothing for unsecured rejoins.
  SL_ZIGBEE_EZSP_DECISION_IGNORE_UNSECURED_REJOINS = 0x0008,
  // Allow joins if there is an entry in the transient key table.
  SL_ZIGBEE_EZSP_DECISION_JOINS_USE_INSTALL_CODE_KEY = 0x0010,
  // Delay sending the network key to a new joining device.
  SL_ZIGBEE_EZSP_DECISION_DEFER_JOINS           = 0x0020
};
#define SL_ZIGBEE_EZSP_DECISION_BITMASK_MAX  0x0020

//------------------------------------------------------------------------------
// Identifies a policy decision.
typedef uint8_t sl_zigbee_ezsp_decision_id_t;

enum {
  // SL_ZIGBEE_EZSP_BINDING_MODIFICATION_POLICY default decision. Do not allow
  // the local binding table to be changed by remote nodes.
  SL_ZIGBEE_EZSP_DISALLOW_BINDING_MODIFICATION  = 0x10,
  // SL_ZIGBEE_EZSP_BINDING_MODIFICATION_POLICY decision. Allow remote nodes to
  // change the local binding table.
  SL_ZIGBEE_EZSP_ALLOW_BINDING_MODIFICATION     = 0x11,
  // SL_ZIGBEE_EZSP_BINDING_MODIFICATION_POLICY decision. Allows remote nodes to
  // set local binding entries only if the entries correspond to endpoints
  // defined on the device, and for output clusters bound to those endpoints.
  SL_ZIGBEE_EZSP_CHECK_BINDING_MODIFICATIONS_ARE_VALID_ENDPOINT_CLUSTERS = 0x12,
  // SL_ZIGBEE_EZSP_UNICAST_REPLIES_POLICY default decision. The NCP will
  // automatically send an empty reply (containing no payload) for every unicast
  // received.
  SL_ZIGBEE_EZSP_HOST_WILL_NOT_SUPPLY_REPLY     = 0x20,
  // SL_ZIGBEE_EZSP_UNICAST_REPLIES_POLICY decision. The NCP will only send a
  // reply if it receives a sendReply command from the Host.
  SL_ZIGBEE_EZSP_HOST_WILL_SUPPLY_REPLY         = 0x21,
  // SL_ZIGBEE_EZSP_POLL_HANDLER_POLICY default decision. Do not inform the Host
  // when a child polls.
  SL_ZIGBEE_EZSP_POLL_HANDLER_IGNORE            = 0x30,
  // SL_ZIGBEE_EZSP_POLL_HANDLER_POLICY decision. Generate a pollHandler
  // callback when a child polls.
  SL_ZIGBEE_EZSP_POLL_HANDLER_CALLBACK          = 0x31,
  // SL_ZIGBEE_EZSP_MESSAGE_CONTENTS_IN_CALLBACK_POLICY default decision.
  // Include only the message tag in the messageSentHandler callback.
  SL_ZIGBEE_EZSP_MESSAGE_TAG_ONLY_IN_CALLBACK   = 0x40,
  // SL_ZIGBEE_EZSP_MESSAGE_CONTENTS_IN_CALLBACK_POLICY decision. Include both
  // the message tag and the message contents in the messageSentHandler
  // callback.
  SL_ZIGBEE_EZSP_MESSAGE_TAG_AND_CONTENTS_IN_CALLBACK = 0x41,
  // SL_ZIGBEE_EZSP_TC_KEY_REQUEST_POLICY decision. When the Trust Center
  // receives a request for a Trust Center link key, it will be ignored.
  SL_ZIGBEE_EZSP_DENY_TC_KEY_REQUESTS           = 0x50,
  // SL_ZIGBEE_EZSP_TC_KEY_REQUEST_POLICY decision. When the Trust Center
  // receives a request for a Trust Center link key, it will reply to it with
  // the corresponding key.
  SL_ZIGBEE_EZSP_ALLOW_TC_KEY_REQUESTS_AND_SEND_CURRENT_KEY = 0x51,
  // SL_ZIGBEE_EZSP_TC_KEY_REQUEST_POLICY decision. When the Trust Center
  // receives a request for a Trust Center link key, it will generate a key to
  // send to the joiner. After generation, the key will be added to the
  // transient key tabe and After verification, this key will be added into the
  // link key table
  SL_ZIGBEE_EZSP_ALLOW_TC_KEY_REQUEST_AND_GENERATE_NEW_KEY = 0x52,
  // SL_ZIGBEE_EZSP_APP_KEY_REQUEST_POLICY decision. When the Trust Center
  // receives a request for an application link key, it will be ignored.
  SL_ZIGBEE_EZSP_DENY_APP_KEY_REQUESTS          = 0x60,
  // SL_ZIGBEE_EZSP_APP_KEY_REQUEST_POLICY decision. When the Trust Center
  // receives a request for an application link key, it will randomly generate a
  // key and send it to both partners.
  SL_ZIGBEE_EZSP_ALLOW_APP_KEY_REQUESTS         = 0x61,
  // Indicates that packet validate library checks are enabled on the NCP.
  SL_ZIGBEE_EZSP_PACKET_VALIDATE_LIBRARY_CHECKS_ENABLED = 0x62,
  // Indicates that packet validate library checks are NOT enabled on the NCP.
  SL_ZIGBEE_EZSP_PACKET_VALIDATE_LIBRARY_CHECKS_DISABLED = 0x63
};
#define SL_ZIGBEE_EZSP_DECISION_ID_MAX  0x63

//------------------------------------------------------------------------------
// Manufacturing token IDs used by sl_zigbee_ezsp_get_mfg_token().
typedef uint8_t sl_zigbee_ezsp_mfg_token_id_t;

enum {
  // Custom version (2 bytes).
  SL_ZIGBEE_EZSP_MFG_CUSTOM_VERSION             = 0x00,
  // Manufacturing string (16 bytes).
  SL_ZIGBEE_EZSP_MFG_STRING                     = 0x01,
  // Board name (16 bytes).
  SL_ZIGBEE_EZSP_MFG_BOARD_NAME                 = 0x02,
  // Manufacturing ID (2 bytes).
  SL_ZIGBEE_EZSP_MFG_MANUF_ID                   = 0x03,
  // Radio configuration (2 bytes).
  SL_ZIGBEE_EZSP_MFG_PHY_CONFIG                 = 0x04,
  // Bootload AES key (16 bytes).
  SL_ZIGBEE_EZSP_MFG_BOOTLOAD_AES_KEY           = 0x05,
  // ASH configuration (40 bytes).
  SL_ZIGBEE_EZSP_MFG_ASH_CONFIG                 = 0x06,
  // EZSP storage (8 bytes).
  SL_ZIGBEE_EZSP_MFG_SL_ZIGBEE_EZSP_STORAGE     = 0x07,
  // Radio calibration data (64 bytes). 4 bytes are stored for each of the 16
  // channels. This token is not stored in the Flash Information Area. It is
  // updated by the stack each time a calibration is performed.
  SL_ZIGBEE_EZSP_STACK_CAL_DATA                 = 0x08,
  // Certificate Based Key Exchange (CBKE) data (92 bytes).
  SL_ZIGBEE_EZSP_MFG_CBKE_DATA                  = 0x09,
  // Installation code (20 bytes).
  SL_ZIGBEE_EZSP_MFG_INSTALLATION_CODE          = 0x0A,
  // Radio channel filter calibration data (1 byte). This token is not stored in
  // the Flash Information Area. It is updated by the stack each time a
  // calibration is performed.
  SL_ZIGBEE_EZSP_STACK_CAL_FILTER               = 0x0B,
  // Custom EUI64 MAC address (8 bytes).
  SL_ZIGBEE_EZSP_MFG_CUSTOM_EUI_64              = 0x0C,
  // CTUNE value (2 byte).
  SL_ZIGBEE_EZSP_MFG_CTUNE                      = 0x0D
};
#define SL_ZIGBEE_EZSP_MFG_TOKEN_ID_MAX  0x0D

//------------------------------------------------------------------------------
// Status values used by EZSP.
typedef uint8_t sl_zigbee_ezsp_status_t;

enum {
  // Success.
  SL_ZIGBEE_EZSP_SUCCESS                        = 0x00,
  // Fatal error.
  SL_ZIGBEE_EZSP_SPI_ERR_FATAL                  = 0x10,
  // The Response frame of the current transaction indicates the NCP has reset.
  SL_ZIGBEE_EZSP_SPI_ERR_NCP_RESET              = 0x11,
  // The NCP is reporting that the Command frame of the current transaction is
  // oversized (the length byte is too large).
  SL_ZIGBEE_EZSP_SPI_ERR_OVERSIZED_SL_ZIGBEE_EZSP_FRAME = 0x12,
  // The Response frame of the current transaction indicates the previous
  // transaction was aborted (nSSEL deasserted too soon).
  SL_ZIGBEE_EZSP_SPI_ERR_ABORTED_TRANSACTION    = 0x13,
  // The Response frame of the current transaction indicates the frame
  // terminator is missing from the Command frame.
  SL_ZIGBEE_EZSP_SPI_ERR_MISSING_FRAME_TERMINATOR = 0x14,
  // The NCP has not provided a Response within the time limit defined by
  // WAIT_SECTION_TIMEOUT.
  SL_ZIGBEE_EZSP_SPI_ERR_WAIT_SECTION_TIMEOUT   = 0x15,
  // The Response frame from the NCP is missing the frame terminator.
  SL_ZIGBEE_EZSP_SPI_ERR_NO_FRAME_TERMINATOR    = 0x16,
  // The Host attempted to send an oversized Command (the length byte is too
  // large) and the AVR's spi-protocol.c blocked the transmission.
  SL_ZIGBEE_EZSP_SPI_ERR_SL_ZIGBEE_EZSP_COMMAND_OVERSIZED = 0x17,
  // The NCP attempted to send an oversized Response (the length byte is too
  // large) and the AVR's spi-protocol.c blocked the reception.
  SL_ZIGBEE_EZSP_SPI_ERR_SL_ZIGBEE_EZSP_RESPONSE_OVERSIZED = 0x18,
  // The Host has sent the Command and is still waiting for the NCP to send a
  // Response.
  SL_ZIGBEE_EZSP_SPI_WAITING_FOR_RESPONSE       = 0x19,
  // The NCP has not asserted nHOST_INT within the time limit defined by
  // WAKE_HANDSHAKE_TIMEOUT.
  SL_ZIGBEE_EZSP_SPI_ERR_HANDSHAKE_TIMEOUT      = 0x1A,
  // The NCP has not asserted nHOST_INT after an NCP reset within the time limit
  // defined by STARTUP_TIMEOUT.
  SL_ZIGBEE_EZSP_SPI_ERR_STARTUP_TIMEOUT        = 0x1B,
  // The Host attempted to verify the SPI Protocol activity and version number,
  // and the verification failed.
  SL_ZIGBEE_EZSP_SPI_ERR_STARTUP_FAIL           = 0x1C,
  // The Host has sent a command with a SPI Byte that is unsupported by the
  // current mode the NCP is operating in.
  SL_ZIGBEE_EZSP_SPI_ERR_UNSUPPORTED_SPI_COMMAND = 0x1D,
  // Operation not yet complete.
  SL_ZIGBEE_EZSP_ASH_IN_PROGRESS                = 0x20,
  // Fatal error detected by host.
  SL_ZIGBEE_EZSP_HOST_FATAL_ERROR               = 0x21,
  // Fatal error detected by NCP.
  SL_ZIGBEE_EZSP_ASH_NCP_FATAL_ERROR            = 0x22,
  // Tried to send DATA frame too long.
  SL_ZIGBEE_EZSP_DATA_FRAME_TOO_LONG            = 0x23,
  // Tried to send DATA frame too short.
  SL_ZIGBEE_EZSP_DATA_FRAME_TOO_SHORT           = 0x24,
  // No space for tx'ed DATA frame.
  SL_ZIGBEE_EZSP_NO_TX_SPACE                    = 0x25,
  // No space for rec'd DATA frame.
  SL_ZIGBEE_EZSP_NO_RX_SPACE                    = 0x26,
  // No receive data available.
  SL_ZIGBEE_EZSP_NO_RX_DATA                     = 0x27,
  // Not in Connected state.
  SL_ZIGBEE_EZSP_NOT_CONNECTED                  = 0x28,
  // The NCP received a command before the EZSP version had been set.
  SL_ZIGBEE_EZSP_ERROR_VERSION_NOT_SET          = 0x30,
  // The NCP received a command containing an unsupported frame ID.
  SL_ZIGBEE_EZSP_ERROR_INVALID_FRAME_ID         = 0x31,
  // The direction flag in the frame control field was incorrect.
  SL_ZIGBEE_EZSP_ERROR_WRONG_DIRECTION          = 0x32,
  // The truncated flag in the frame control field was set, indicating there was
  // not enough memory available to complete the response or that the response
  // would have exceeded the maximum EZSP frame length.
  SL_ZIGBEE_EZSP_ERROR_TRUNCATED                = 0x33,
  // The overflow flag in the frame control field was set, indicating one or
  // more callbacks occurred since the previous response and there was not
  // enough memory available to report them to the Host.
  SL_ZIGBEE_EZSP_ERROR_OVERFLOW                 = 0x34,
  // Insufficient memory was available.
  SL_ZIGBEE_EZSP_ERROR_OUT_OF_MEMORY            = 0x35,
  // The value was out of bounds.
  SL_ZIGBEE_EZSP_ERROR_INVALID_VALUE            = 0x36,
  // The configuration id was not recognized.
  SL_ZIGBEE_EZSP_ERROR_INVALID_ID               = 0x37,
  // Configuration values can no longer be modified.
  SL_ZIGBEE_EZSP_ERROR_INVALID_CALL             = 0x38,
  // The NCP failed to respond to a command.
  SL_ZIGBEE_EZSP_ERROR_NO_RESPONSE              = 0x39,
  // The length of the command exceeded the maximum EZSP frame length.
  SL_ZIGBEE_EZSP_ERROR_COMMAND_TOO_LONG         = 0x40,
  // The UART receive queue was full causing a callback response to be dropped.
  SL_ZIGBEE_EZSP_ERROR_QUEUE_FULL               = 0x41,
  // The command has been filtered out by NCP.
  SL_ZIGBEE_EZSP_ERROR_COMMAND_FILTERED         = 0x42,
  // EZSP Security Key is already set
  SL_ZIGBEE_EZSP_ERROR_SECURITY_KEY_ALREADY_SET = 0x43,
  // EZSP Security Type is invalid
  SL_ZIGBEE_EZSP_ERROR_SECURITY_TYPE_INVALID    = 0x44,
  // EZSP Security Parameters are invalid
  SL_ZIGBEE_EZSP_ERROR_SECURITY_PARAMETERS_INVALID = 0x45,
  // EZSP Security Parameters are already set
  SL_ZIGBEE_EZSP_ERROR_SECURITY_PARAMETERS_ALREADY_SET = 0x46,
  // EZSP Security Key is not set
  SL_ZIGBEE_EZSP_ERROR_SECURITY_KEY_NOT_SET     = 0x47,
  // EZSP Security Parameters are not set
  SL_ZIGBEE_EZSP_ERROR_SECURITY_PARAMETERS_NOT_SET = 0x48,
  // Received frame with unsupported control byte
  SL_ZIGBEE_EZSP_ERROR_UNSUPPORTED_CONTROL      = 0x49,
  // Received frame is unsecure, when security is established
  SL_ZIGBEE_EZSP_ERROR_UNSECURE_FRAME           = 0x4A,
  // Incompatible ASH version
  SL_ZIGBEE_EZSP_ASH_ERROR_VERSION              = 0x50,
  // Exceeded max ACK timeouts
  SL_ZIGBEE_EZSP_ASH_ERROR_TIMEOUTS             = 0x51,
  // Timed out waiting for RSTACK
  SL_ZIGBEE_EZSP_ASH_ERROR_RESET_FAIL           = 0x52,
  // Unexpected ncp reset
  SL_ZIGBEE_EZSP_ASH_ERROR_NCP_RESET            = 0x53,
  // Serial port initialization failed
  SL_ZIGBEE_EZSP_ERROR_SERIAL_INIT              = 0x54,
  // Invalid ncp processor type
  SL_ZIGBEE_EZSP_ASH_ERROR_NCP_TYPE             = 0x55,
  // Invalid ncp reset method
  SL_ZIGBEE_EZSP_ASH_ERROR_RESET_METHOD         = 0x56,
  // XON/XOFF not supported by host driver
  SL_ZIGBEE_EZSP_ASH_ERROR_XON_XOFF             = 0x57,
  // ASH protocol started
  SL_ZIGBEE_EZSP_ASH_STARTED                    = 0x70,
  // ASH protocol connected
  SL_ZIGBEE_EZSP_ASH_CONNECTED                  = 0x71,
  // ASH protocol disconnected
  SL_ZIGBEE_EZSP_ASH_DISCONNECTED               = 0x72,
  // Timer expired waiting for ack
  SL_ZIGBEE_EZSP_ASH_ACK_TIMEOUT                = 0x73,
  // Frame in progress cancelled
  SL_ZIGBEE_EZSP_ASH_CANCELLED                  = 0x74,
  // Received frame out of sequence
  SL_ZIGBEE_EZSP_ASH_OUT_OF_SEQUENCE            = 0x75,
  // Received frame with CRC error
  SL_ZIGBEE_EZSP_ASH_BAD_CRC                    = 0x76,
  // Received frame with comm error
  SL_ZIGBEE_EZSP_ASH_COMM_ERROR                 = 0x77,
  // Received frame with bad ackNum
  SL_ZIGBEE_EZSP_ASH_BAD_ACKNUM                 = 0x78,
  // Received frame shorter than minimum
  SL_ZIGBEE_EZSP_ASH_TOO_SHORT                  = 0x79,
  // Received frame longer than maximum
  SL_ZIGBEE_EZSP_ASH_TOO_LONG                   = 0x7A,
  // Received frame with illegal control byte
  SL_ZIGBEE_EZSP_ASH_BAD_CONTROL                = 0x7B,
  // Received frame with illegal length for its type
  SL_ZIGBEE_EZSP_ASH_BAD_LENGTH                 = 0x7C,
  // Received ASH Ack
  SL_ZIGBEE_EZSP_ASH_ACK_RECEIVED               = 0x7D,
  // Sent ASH Ack
  SL_ZIGBEE_EZSP_ASH_ACK_SENT                   = 0x7E,
  // Received ASH Nak
  SL_ZIGBEE_EZSP_ASH_NAK_RECEIVED               = 0x7F,
  // Sent ASH Nak
  SL_ZIGBEE_EZSP_ASH_NAK_SENT                   = 0x80,
  // Received ASH RST
  SL_ZIGBEE_EZSP_ASH_RST_RECEIVED               = 0x81,
  // Sent ASH RST
  SL_ZIGBEE_EZSP_ASH_RST_SENT                   = 0x82,
  // ASH Status
  SL_ZIGBEE_EZSP_ASH_STATUS                     = 0x83,
  // ASH TX
  SL_ZIGBEE_EZSP_ASH_TX                         = 0x84,
  // ASH RX
  SL_ZIGBEE_EZSP_ASH_RX                         = 0x85,
  // Failed to connect to CPC daemon or failed to open CPC endpoint
  SL_ZIGBEE_EZSP_CPC_ERROR_INIT                 = 0x86,
  // No reset or error
  SL_ZIGBEE_EZSP_NO_ERROR                       = 0xFF
};
#define SL_ZIGBEE_EZSP_STATUS_MAX  0xFF

//------------------------------------------------------------------------------
// Network scan types.
typedef uint8_t sl_zigbee_ezsp_network_scan_type_t;

enum {
  // An energy scan scans each channel for its RSSI value.
  SL_ZIGBEE_EZSP_ENERGY_SCAN                    = 0x00,
  // An active scan scans each channel for available networks.
  SL_ZIGBEE_EZSP_ACTIVE_SCAN                    = 0x01
};
#define SL_ZIGBEE_EZSP_NETWORK_SCAN_TYPE_MAX  0x01

//------------------------------------------------------------------------------
// Differentiates among ZLL network operations.
typedef uint8_t sl_zigbee_ezsp_zll_network_operation_t;

enum {
  // ZLL form network command.
  SL_ZIGBEE_EZSP_ZLL_FORM_NETWORK               = 0x00,
  // ZLL join target command.
  SL_ZIGBEE_EZSP_ZLL_JOIN_TARGET                = 0x01
};
#define SL_ZIGBEE_EZSP_ZLL_NETWORK_OPERATION_MAX  0x01

//------------------------------------------------------------------------------
// Frame IDs

enum {

// Configuration Frames
  SL_ZIGBEE_EZSP_VERSION                        = 0x0000,
  SL_ZIGBEE_EZSP_GET_CONFIGURATION_VALUE        = 0x0052,
  SL_ZIGBEE_EZSP_SET_CONFIGURATION_VALUE        = 0x0053,
  SL_ZIGBEE_EZSP_READ_ATTRIBUTE                 = 0x0108,
  SL_ZIGBEE_EZSP_WRITE_ATTRIBUTE                = 0x0109,
  SL_ZIGBEE_EZSP_ADD_ENDPOINT                   = 0x0002,
  SL_ZIGBEE_EZSP_SET_POLICY                     = 0x0055,
  SL_ZIGBEE_EZSP_GET_POLICY                     = 0x0056,
  SL_ZIGBEE_EZSP_SEND_PAN_ID_UPDATE             = 0x0057,
  SL_ZIGBEE_EZSP_GET_VALUE                      = 0x00AA,
  SL_ZIGBEE_EZSP_GET_EXTENDED_VALUE             = 0x0003,
  SL_ZIGBEE_EZSP_SET_VALUE                      = 0x00AB,
  SL_ZIGBEE_EZSP_SET_PASSIVE_ACK_CONFIG         = 0x0105,
  SL_ZIGBEE_EZSP_SET_PENDING_NETWORK_UPDATE_PAN_ID = 0x011E,
  SL_ZIGBEE_EZSP_SET_PENDING_NETWORK_UPDATE_CHANNEL = 0x003D,
  SL_ZIGBEE_EZSP_GET_ENDPOINT                   = 0x012E,
  SL_ZIGBEE_EZSP_GET_ENDPOINT_COUNT             = 0x012F,
  SL_ZIGBEE_EZSP_GET_ENDPOINT_DESCRIPTION       = 0x0130,
  SL_ZIGBEE_EZSP_GET_ENDPOINT_CLUSTER           = 0x0131,

// Utilities Frames
  SL_ZIGBEE_EZSP_NOP                            = 0x0005,
  SL_ZIGBEE_EZSP_ECHO                           = 0x0081,
  SL_ZIGBEE_EZSP_INVALID_COMMAND                = 0x0058,
  SL_ZIGBEE_EZSP_CALLBACK                       = 0x0006,
  SL_ZIGBEE_EZSP_NO_CALLBACKS                   = 0x0007,
  SL_ZIGBEE_EZSP_SET_TOKEN                      = 0x0009,
  SL_ZIGBEE_EZSP_GET_TOKEN                      = 0x000A,
  SL_ZIGBEE_EZSP_GET_MFG_TOKEN                  = 0x000B,
  SL_ZIGBEE_EZSP_SET_MFG_TOKEN                  = 0x000C,
  SL_ZIGBEE_EZSP_STACK_TOKEN_CHANGED_HANDLER    = 0x000D,
  SL_ZIGBEE_EZSP_GET_RANDOM_NUMBER              = 0x0049,
  SL_ZIGBEE_EZSP_SET_TIMER                      = 0x000E,
  SL_ZIGBEE_EZSP_GET_TIMER                      = 0x004E,
  SL_ZIGBEE_EZSP_TIMER_HANDLER                  = 0x000F,
  SL_ZIGBEE_EZSP_DEBUG_WRITE                    = 0x0012,
  SL_ZIGBEE_EZSP_READ_AND_CLEAR_COUNTERS        = 0x0065,
  SL_ZIGBEE_EZSP_READ_COUNTERS                  = 0x00F1,
  SL_ZIGBEE_EZSP_COUNTER_ROLLOVER_HANDLER       = 0x00F2,
  SL_ZIGBEE_EZSP_MUX_INVALID_RX_HANDLER         = 0x0062,
  SL_ZIGBEE_EZSP_DELAY_TEST                     = 0x009D,
  SL_ZIGBEE_EZSP_GET_LIBRARY_STATUS             = 0x0001,
  SL_ZIGBEE_EZSP_GET_XNCP_INFO                  = 0x0013,
  SL_ZIGBEE_EZSP_CUSTOM_FRAME                   = 0x0047,
  SL_ZIGBEE_EZSP_CUSTOM_FRAME_HANDLER           = 0x0054,
  SL_ZIGBEE_EZSP_GET_EUI64                      = 0x0026,
  SL_ZIGBEE_EZSP_GET_NODE_ID                    = 0x0027,
  SL_ZIGBEE_EZSP_GET_PHY_INTERFACE_COUNT        = 0x00FC,
  SL_ZIGBEE_EZSP_GET_TRUE_RANDOM_ENTROPY_SOURCE = 0x004F,
  SL_ZIGBEE_EZSP_SETUP_DELAYED_JOIN             = 0x003A,
  SL_ZIGBEE_EZSP_RADIO_GET_SCHEDULER_PRIORITIES = 0x012A,
  SL_ZIGBEE_EZSP_RADIO_GET_SCHEDULER_SLIPTIME   = 0x012C,
  SL_ZIGBEE_EZSP_RADIO_SET_SCHEDULER_SLIPTIME   = 0x012D,
  SL_ZIGBEE_EZSP_COUNTER_REQUIRES_PHY_INDEX     = 0x0132,
  SL_ZIGBEE_EZSP_COUNTER_REQUIRES_DESTINATION_NODE_ID = 0x0133,

// Networking Frames
  SL_ZIGBEE_EZSP_SET_MANUFACTURER_CODE          = 0x0015,
  SL_ZIGBEE_EZSP_GET_MANUFACTURER_CODE          = 0x00CA,
  SL_ZIGBEE_EZSP_SET_POWER_DESCRIPTOR           = 0x0016,
  SL_ZIGBEE_EZSP_NETWORK_INIT                   = 0x0017,
  SL_ZIGBEE_EZSP_NETWORK_STATE                  = 0x0018,
  SL_ZIGBEE_EZSP_STACK_STATUS_HANDLER           = 0x0019,
  SL_ZIGBEE_EZSP_START_SCAN                     = 0x001A,
  SL_ZIGBEE_EZSP_ENERGY_SCAN_RESULT_HANDLER     = 0x0048,
  SL_ZIGBEE_EZSP_NETWORK_FOUND_HANDLER          = 0x001B,
  SL_ZIGBEE_EZSP_SCAN_COMPLETE_HANDLER          = 0x001C,
  SL_ZIGBEE_EZSP_UNUSED_PAN_ID_FOUND_HANDLER    = 0x00D2,
  SL_ZIGBEE_EZSP_FIND_UNUSED_PAN_ID             = 0x00D3,
  SL_ZIGBEE_EZSP_STOP_SCAN                      = 0x001D,
  SL_ZIGBEE_EZSP_FORM_NETWORK                   = 0x001E,
  SL_ZIGBEE_EZSP_JOIN_NETWORK                   = 0x001F,
  SL_ZIGBEE_EZSP_JOIN_NETWORK_DIRECTLY          = 0x003B,
  SL_ZIGBEE_EZSP_LEAVE_NETWORK                  = 0x0020,
  SL_ZIGBEE_EZSP_FIND_AND_REJOIN_NETWORK        = 0x0021,
  SL_ZIGBEE_EZSP_PERMIT_JOINING                 = 0x0022,
  SL_ZIGBEE_EZSP_CHILD_JOIN_HANDLER             = 0x0023,
  SL_ZIGBEE_EZSP_ENERGY_SCAN_REQUEST            = 0x009C,
  SL_ZIGBEE_EZSP_GET_NETWORK_PARAMETERS         = 0x0028,
  SL_ZIGBEE_EZSP_GET_RADIO_PARAMETERS           = 0x00FD,
  SL_ZIGBEE_EZSP_GET_PARENT_CHILD_PARAMETERS    = 0x0029,
  SL_ZIGBEE_EZSP_ROUTER_CHILD_COUNT             = 0x013B,
  SL_ZIGBEE_EZSP_MAX_CHILD_COUNT                = 0x013C,
  SL_ZIGBEE_EZSP_MAX_ROUTER_CHILD_COUNT         = 0x013D,
  SL_ZIGBEE_EZSP_GET_PARENT_INCOMING_NWK_FRAME_COUNTER = 0x013E,
  SL_ZIGBEE_EZSP_SET_PARENT_INCOMING_NWK_FRAME_COUNTER = 0x013F,
  SL_ZIGBEE_EZSP_CURRENT_STACK_TASKS            = 0x0145,
  SL_ZIGBEE_EZSP_OK_TO_NAP                      = 0x0146,
  SL_ZIGBEE_EZSP_PARENT_TOKEN_SET               = 0x0140,
  SL_ZIGBEE_EZSP_OK_TO_HIBERNATE                = 0x0141,
  SL_ZIGBEE_EZSP_OK_TO_LONG_POLL                = 0x0142,
  SL_ZIGBEE_EZSP_STACK_POWER_DOWN               = 0x0143,
  SL_ZIGBEE_EZSP_STACK_POWER_UP                 = 0x0144,
  SL_ZIGBEE_EZSP_GET_CHILD_DATA                 = 0x004A,
  SL_ZIGBEE_EZSP_SET_CHILD_DATA                 = 0x00AC,
  SL_ZIGBEE_EZSP_CHILD_ID                       = 0x0106,
  SL_ZIGBEE_EZSP_CHILD_POWER                    = 0x0134,
  SL_ZIGBEE_EZSP_SET_CHILD_POWER                = 0x0135,
  SL_ZIGBEE_EZSP_CHILD_INDEX                    = 0x0107,
  SL_ZIGBEE_EZSP_GET_SOURCE_ROUTE_TABLE_TOTAL_SIZE = 0x00C3,
  SL_ZIGBEE_EZSP_GET_SOURCE_ROUTE_TABLE_FILLED_SIZE = 0x00C2,
  SL_ZIGBEE_EZSP_GET_SOURCE_ROUTE_TABLE_ENTRY   = 0x00C1,
  SL_ZIGBEE_EZSP_GET_NEIGHBOR                   = 0x0079,
  SL_ZIGBEE_EZSP_GET_NEIGHBOR_FRAME_COUNTER     = 0x003E,
  SL_ZIGBEE_EZSP_SET_NEIGHBOR_FRAME_COUNTER     = 0x00AD,
  SL_ZIGBEE_EZSP_SET_ROUTING_SHORTCUT_THRESHOLD = 0x00D0,
  SL_ZIGBEE_EZSP_GET_ROUTING_SHORTCUT_THRESHOLD = 0x00D1,
  SL_ZIGBEE_EZSP_NEIGHBOR_COUNT                 = 0x007A,
  SL_ZIGBEE_EZSP_GET_ROUTE_TABLE_ENTRY          = 0x007B,
  SL_ZIGBEE_EZSP_SET_RADIO_POWER                = 0x0099,
  SL_ZIGBEE_EZSP_SET_RADIO_CHANNEL              = 0x009A,
  SL_ZIGBEE_EZSP_GET_RADIO_CHANNEL              = 0x00FF,
  SL_ZIGBEE_EZSP_SET_RADIO_IEEE802154_CCA_MODE  = 0x0095,
  SL_ZIGBEE_EZSP_SET_CONCENTRATOR               = 0x0010,
  SL_ZIGBEE_EZSP_CONCENTRATOR_START_DISCOVERY   = 0x014F,
  SL_ZIGBEE_EZSP_CONCENTRATOR_STOP_DISCOVERY    = 0x0150,
  SL_ZIGBEE_EZSP_CONCENTRATOR_NOTE_ROUTE_ERROR  = 0x0151,
  SL_ZIGBEE_EZSP_SET_BROKEN_ROUTE_ERROR_CODE    = 0x0011,
  SL_ZIGBEE_EZSP_MULTI_PHY_START                = 0x00F8,
  SL_ZIGBEE_EZSP_MULTI_PHY_STOP                 = 0x00F9,
  SL_ZIGBEE_EZSP_MULTI_PHY_SET_RADIO_POWER      = 0x00FA,
  SL_ZIGBEE_EZSP_SEND_LINK_POWER_DELTA_REQUEST  = 0x00F7,
  SL_ZIGBEE_EZSP_MULTI_PHY_SET_RADIO_CHANNEL    = 0x00FB,
  SL_ZIGBEE_EZSP_GET_DUTY_CYCLE_STATE           = 0x0035,
  SL_ZIGBEE_EZSP_SET_DUTY_CYCLE_LIMITS_IN_STACK = 0x0040,
  SL_ZIGBEE_EZSP_GET_DUTY_CYCLE_LIMITS          = 0x004B,
  SL_ZIGBEE_EZSP_GET_CURRENT_DUTY_CYCLE         = 0x004C,
  SL_ZIGBEE_EZSP_DUTY_CYCLE_HANDLER             = 0x004D,
  SL_ZIGBEE_EZSP_SET_NUM_BEACONS_TO_STORE       = 0x0037,
  SL_ZIGBEE_EZSP_GET_STORED_BEACON              = 0x0004,
  SL_ZIGBEE_EZSP_GET_NUM_STORED_BEACONS         = 0x0008,
  SL_ZIGBEE_EZSP_CLEAR_STORED_BEACONS           = 0x003C,
  SL_ZIGBEE_EZSP_SET_LOGICAL_AND_RADIO_CHANNEL  = 0x00B9,
  SL_ZIGBEE_EZSP_SLEEPY_TO_SLEEPY_NETWORK_START = 0x0119,
  SL_ZIGBEE_EZSP_SEND_ZIGBEE_LEAVE              = 0x011A,
  SL_ZIGBEE_EZSP_GET_PERMIT_JOINING             = 0x011F,
  SL_ZIGBEE_EZSP_GET_EXTENDED_PAN_ID            = 0x0127,
  SL_ZIGBEE_EZSP_GET_CURRENT_NETWORK            = 0x014E,
  SL_ZIGBEE_EZSP_SET_INITIAL_NEIGHBOR_OUTGOING_COST = 0x0122,
  SL_ZIGBEE_EZSP_GET_INITIAL_NEIGHBOR_OUTGOING_COST = 0x0123,
  SL_ZIGBEE_EZSP_RESET_REJOINING_NEIGHBORS_FRAME_COUNTER = 0x0124,
  SL_ZIGBEE_EZSP_IS_RESET_REJOINING_NEIGHBORS_FRAME_COUNTER_ENABLED = 0x0125,

// Binding Frames
  SL_ZIGBEE_EZSP_CLEAR_BINDING_TABLE            = 0x002A,
  SL_ZIGBEE_EZSP_SET_BINDING                    = 0x002B,
  SL_ZIGBEE_EZSP_GET_BINDING                    = 0x002C,
  SL_ZIGBEE_EZSP_DELETE_BINDING                 = 0x002D,
  SL_ZIGBEE_EZSP_BINDING_IS_ACTIVE              = 0x002E,
  SL_ZIGBEE_EZSP_GET_BINDING_REMOTE_NODE_ID     = 0x002F,
  SL_ZIGBEE_EZSP_SET_BINDING_REMOTE_NODE_ID     = 0x0030,
  SL_ZIGBEE_EZSP_REMOTE_SET_BINDING_HANDLER     = 0x0031,
  SL_ZIGBEE_EZSP_REMOTE_DELETE_BINDING_HANDLER  = 0x0032,

// Messaging Frames
  SL_ZIGBEE_EZSP_MAXIMUM_PAYLOAD_LENGTH         = 0x0033,
  SL_ZIGBEE_EZSP_SEND_UNICAST                   = 0x0034,
  SL_ZIGBEE_EZSP_SEND_BROADCAST                 = 0x0036,
  SL_ZIGBEE_EZSP_PROXY_NEXT_BROADCAST_FROM_LONG = 0x0066,
  SL_ZIGBEE_EZSP_SEND_MULTICAST                 = 0x0038,
  SL_ZIGBEE_EZSP_SEND_REPLY                     = 0x0039,
  SL_ZIGBEE_EZSP_MESSAGE_SENT_HANDLER           = 0x003F,
  SL_ZIGBEE_EZSP_SEND_MANY_TO_ONE_ROUTE_REQUEST = 0x0041,
  SL_ZIGBEE_EZSP_POLL_FOR_DATA                  = 0x0042,
  SL_ZIGBEE_EZSP_POLL_COMPLETE_HANDLER          = 0x0043,
  SL_ZIGBEE_EZSP_SET_MESSAGE_FLAG               = 0x0136,
  SL_ZIGBEE_EZSP_CLEAR_MESSAGE_FLAG             = 0x0137,
  SL_ZIGBEE_EZSP_POLL_HANDLER                   = 0x0044,
  SL_ZIGBEE_EZSP_ADD_CHILD                      = 0x0138,
  SL_ZIGBEE_EZSP_REMOVE_CHILD                   = 0x0139,
  SL_ZIGBEE_EZSP_REMOVE_NEIGHBOR                = 0x013A,
  SL_ZIGBEE_EZSP_INCOMING_MESSAGE_HANDLER       = 0x0045,
  SL_ZIGBEE_EZSP_SET_SOURCE_ROUTE_DISCOVERY_MODE = 0x005A,
  SL_ZIGBEE_EZSP_INCOMING_MANY_TO_ONE_ROUTE_REQUEST_HANDLER = 0x007D,
  SL_ZIGBEE_EZSP_INCOMING_ROUTE_ERROR_HANDLER   = 0x0080,
  SL_ZIGBEE_EZSP_INCOMING_NETWORK_STATUS_HANDLER = 0x00C4,
  SL_ZIGBEE_EZSP_INCOMING_ROUTE_RECORD_HANDLER  = 0x0059,
  SL_ZIGBEE_EZSP_SET_SOURCE_ROUTE               = 0x00AE,
  SL_ZIGBEE_EZSP_UNICAST_CURRENT_NETWORK_KEY    = 0x0050,
  SL_ZIGBEE_EZSP_ADDRESS_TABLE_ENTRY_IS_ACTIVE  = 0x005B,
  SL_ZIGBEE_EZSP_SET_ADDRESS_TABLE_INFO         = 0x005C,
  SL_ZIGBEE_EZSP_GET_ADDRESS_TABLE_INFO         = 0x005E,
  SL_ZIGBEE_EZSP_SET_EXTENDED_TIMEOUT           = 0x007E,
  SL_ZIGBEE_EZSP_GET_EXTENDED_TIMEOUT           = 0x007F,
  SL_ZIGBEE_EZSP_REPLACE_ADDRESS_TABLE_ENTRY    = 0x0082,
  SL_ZIGBEE_EZSP_LOOKUP_NODE_ID_BY_EUI64        = 0x0060,
  SL_ZIGBEE_EZSP_LOOKUP_EUI64_BY_NODE_ID        = 0x0061,
  SL_ZIGBEE_EZSP_GET_MULTICAST_TABLE_ENTRY      = 0x0063,
  SL_ZIGBEE_EZSP_SET_MULTICAST_TABLE_ENTRY      = 0x0064,
  SL_ZIGBEE_EZSP_ID_CONFLICT_HANDLER            = 0x007C,
  SL_ZIGBEE_EZSP_WRITE_NODE_DATA                = 0x00FE,
  SL_ZIGBEE_EZSP_SEND_RAW_MESSAGE               = 0x0051,
  SL_ZIGBEE_EZSP_MAC_PASSTHROUGH_MESSAGE_HANDLER = 0x0097,
  SL_ZIGBEE_EZSP_MAC_FILTER_MATCH_MESSAGE_HANDLER = 0x0046,
  SL_ZIGBEE_EZSP_RAW_TRANSMIT_COMPLETE_HANDLER  = 0x0098,
  SL_ZIGBEE_EZSP_SET_MAC_POLL_FAILURE_WAIT_TIME = 0x00F4,
  SL_ZIGBEE_EZSP_GET_MAX_MAC_RETRIES            = 0x006A,
  SL_ZIGBEE_EZSP_SET_BEACON_CLASSIFICATION_PARAMS = 0x00EF,
  SL_ZIGBEE_EZSP_GET_BEACON_CLASSIFICATION_PARAMS = 0x00F3,
  SL_ZIGBEE_EZSP_PENDING_ACKED_MESSAGES         = 0x0121,
  SL_ZIGBEE_EZSP_RESCHEDULE_LINK_STATUS_MSG     = 0x011B,
  SL_ZIGBEE_EZSP_SET_NWK_UPDATE_ID              = 0x011D,

// Security Frames
  SL_ZIGBEE_EZSP_SET_INITIAL_SECURITY_STATE     = 0x0068,
  SL_ZIGBEE_EZSP_GET_CURRENT_SECURITY_STATE     = 0x0069,
  SL_ZIGBEE_EZSP_SEC_MAN_EXPORT_KEY             = 0x0114,
  SL_ZIGBEE_EZSP_SEC_MAN_IMPORT_KEY             = 0x0115,
  SL_ZIGBEE_EZSP_SWITCH_NETWORK_KEY_HANDLER     = 0x006e,
  SL_ZIGBEE_EZSP_FIND_KEY_TABLE_ENTRY           = 0x0075,
  SL_ZIGBEE_EZSP_SEND_TRUST_CENTER_LINK_KEY     = 0x0067,
  SL_ZIGBEE_EZSP_ERASE_KEY_TABLE_ENTRY          = 0x0076,
  SL_ZIGBEE_EZSP_CLEAR_KEY_TABLE                = 0x00B1,
  SL_ZIGBEE_EZSP_REQUEST_LINK_KEY               = 0x0014,
  SL_ZIGBEE_EZSP_UPDATE_TC_LINK_KEY             = 0x006C,
  SL_ZIGBEE_EZSP_ZIGBEE_KEY_ESTABLISHMENT_HANDLER = 0x009B,
  SL_ZIGBEE_EZSP_CLEAR_TRANSIENT_LINK_KEYS      = 0x006B,
  SL_ZIGBEE_EZSP_SEC_MAN_GET_NETWORK_KEY_INFO   = 0x0116,
  SL_ZIGBEE_EZSP_SEC_MAN_GET_APS_KEY_INFO       = 0x010C,
  SL_ZIGBEE_EZSP_SEC_MAN_IMPORT_LINK_KEY        = 0x010E,
  SL_ZIGBEE_EZSP_SEC_MAN_EXPORT_LINK_KEY_BY_INDEX = 0x010F,
  SL_ZIGBEE_EZSP_SEC_MAN_EXPORT_LINK_KEY_BY_EUI = 0x010D,
  SL_ZIGBEE_EZSP_SEC_MAN_CHECK_KEY_CONTEXT      = 0x0110,
  SL_ZIGBEE_EZSP_SEC_MAN_IMPORT_TRANSIENT_KEY   = 0x0111,
  SL_ZIGBEE_EZSP_SEC_MAN_EXPORT_TRANSIENT_KEY_BY_INDEX = 0x0112,
  SL_ZIGBEE_EZSP_SEC_MAN_EXPORT_TRANSIENT_KEY_BY_EUI = 0x0113,
  SL_ZIGBEE_EZSP_SET_INCOMING_TC_LINK_KEY_FRAME_COUNTER = 0x0128,
  SL_ZIGBEE_EZSP_APS_CRYPT_MESSAGE              = 0x0129,

// Trust Center Frames
  SL_ZIGBEE_EZSP_TRUST_CENTER_POST_JOIN_HANDLER = 0x0024,
  SL_ZIGBEE_EZSP_BROADCAST_NEXT_NETWORK_KEY     = 0x0073,
  SL_ZIGBEE_EZSP_BROADCAST_NETWORK_KEY_SWITCH   = 0x0074,
  SL_ZIGBEE_EZSP_AES_MMO_HASH                   = 0x006F,
  SL_ZIGBEE_EZSP_REMOVE_DEVICE                  = 0x00A8,
  SL_ZIGBEE_EZSP_UNICAST_NWK_KEY_UPDATE         = 0x00A9,

// Certificate Based Key Exchange (CBKE) Frames
  SL_ZIGBEE_EZSP_GENERATE_CBKE_KEYS             = 0x00A4,
  SL_ZIGBEE_EZSP_GENERATE_CBKE_KEYS_HANDLER     = 0x009E,
  SL_ZIGBEE_EZSP_CALCULATE_SMACS                = 0x009F,
  SL_ZIGBEE_EZSP_CALCULATE_SMACS_HANDLER        = 0x00A0,
  SL_ZIGBEE_EZSP_GENERATE_CBKE_KEYS_283K1       = 0x00E8,
  SL_ZIGBEE_EZSP_GENERATE_CBKE_KEYS_283K1_HANDLER = 0x00E9,
  SL_ZIGBEE_EZSP_CALCULATE_SMACS_283K1          = 0x00EA,
  SL_ZIGBEE_EZSP_CALCULATE_SMACS_283K1_HANDLER  = 0x00EB,
  SL_ZIGBEE_EZSP_CLEAR_TEMPORARY_DATA_MAYBE_STORE_LINK_KEY = 0x00A1,
  SL_ZIGBEE_EZSP_CLEAR_TEMPORARY_DATA_MAYBE_STORE_LINK_KEY_283K1 = 0x00EE,
  SL_ZIGBEE_EZSP_GET_CERTIFICATE                = 0x00A5,
  SL_ZIGBEE_EZSP_GET_CERTIFICATE_283K1          = 0x00EC,
  SL_ZIGBEE_EZSP_DSA_SIGN                       = 0x00A6,
  SL_ZIGBEE_EZSP_DSA_SIGN_HANDLER               = 0x00A7,
  SL_ZIGBEE_EZSP_DSA_VERIFY                     = 0x00A3,
  SL_ZIGBEE_EZSP_DSA_VERIFY_HANDLER             = 0x0078,
  SL_ZIGBEE_EZSP_DSA_VERIFY_283K1               = 0x00B0,
  SL_ZIGBEE_EZSP_SET_PREINSTALLED_CBKE_DATA     = 0x00A2,
  SL_ZIGBEE_EZSP_SAVE_PREINSTALLED_CBKE_DATA_283K1 = 0x00ED,

// Mfglib Frames
  SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_START          = 0x0083,
  SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_END            = 0x0084,
  SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_START_TONE     = 0x0085,
  SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_STOP_TONE      = 0x0086,
  SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_START_STREAM   = 0x0087,
  SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_STOP_STREAM    = 0x0088,
  SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_SEND_PACKET    = 0x0089,
  SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_SET_CHANNEL    = 0x008a,
  SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_GET_CHANNEL    = 0x008b,
  SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_SET_POWER      = 0x008c,
  SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_GET_POWER      = 0x008d,
  SL_ZIGBEE_EZSP_MFGLIB_RX_HANDLER              = 0x008e,

// Bootloader Frames
  SL_ZIGBEE_EZSP_LAUNCH_STANDALONE_BOOTLOADER   = 0x008f,
  SL_ZIGBEE_EZSP_SEND_BOOTLOAD_MESSAGE          = 0x0090,
  SL_ZIGBEE_EZSP_GET_STANDALONE_BOOTLOADER_VERSION_PLAT_MICRO_PHY = 0x0091,
  SL_ZIGBEE_EZSP_INCOMING_BOOTLOAD_MESSAGE_HANDLER = 0x0092,
  SL_ZIGBEE_EZSP_BOOTLOAD_TRANSMIT_COMPLETE_HANDLER = 0x0093,
  SL_ZIGBEE_EZSP_AES_ENCRYPT                    = 0x0094,
  SL_ZIGBEE_EZSP_INCOMING_MFG_TEST_MESSAGE_HANDLER = 0x0147,
  SL_ZIGBEE_EZSP_MFG_TEST_SET_PACKET_MODE       = 0x0148,
  SL_ZIGBEE_EZSP_MFG_TEST_SEND_REBOOT_COMMAND   = 0x0149,
  SL_ZIGBEE_EZSP_MFG_TEST_SEND_EUI64            = 0x014A,
  SL_ZIGBEE_EZSP_MFG_TEST_SEND_MANUFACTURING_STRING = 0x014B,
  SL_ZIGBEE_EZSP_MFG_TEST_SEND_RADIO_PARAMETERS = 0x014C,
  SL_ZIGBEE_EZSP_MFG_TEST_SEND_COMMAND          = 0x014D,

// ZLL Frames
  SL_ZIGBEE_EZSP_ZLL_NETWORK_OPS                = 0x00B2,
  SL_ZIGBEE_EZSP_ZLL_SET_INITIAL_SECURITY_STATE = 0x00B3,
  SL_ZIGBEE_EZSP_ZLL_SET_SECURITY_STATE_WITHOUT_KEY = 0x00CF,
  SL_ZIGBEE_EZSP_ZLL_START_SCAN                 = 0x00B4,
  SL_ZIGBEE_EZSP_ZLL_SET_RX_ON_WHEN_IDLE        = 0x00B5,
  SL_ZIGBEE_EZSP_ZLL_NETWORK_FOUND_HANDLER      = 0x00B6,
  SL_ZIGBEE_EZSP_ZLL_SCAN_COMPLETE_HANDLER      = 0x00B7,
  SL_ZIGBEE_EZSP_ZLL_ADDRESS_ASSIGNMENT_HANDLER = 0x00B8,
  SL_ZIGBEE_EZSP_ZLL_TOUCH_LINK_TARGET_HANDLER  = 0x00BB,
  SL_ZIGBEE_EZSP_ZLL_GET_TOKENS                 = 0x00BC,
  SL_ZIGBEE_EZSP_ZLL_SET_DATA_TOKEN             = 0x00BD,
  SL_ZIGBEE_EZSP_ZLL_SET_NON_ZLL_NETWORK        = 0x00BF,
  SL_ZIGBEE_EZSP_IS_ZLL_NETWORK                 = 0x00BE,
  SL_ZIGBEE_EZSP_ZLL_SET_RADIO_IDLE_MODE        = 0x00D4,
  SL_ZIGBEE_EZSP_ZLL_GET_RADIO_IDLE_MODE        = 0x00BA,
  SL_ZIGBEE_EZSP_SET_ZLL_NODE_TYPE              = 0x00D5,
  SL_ZIGBEE_EZSP_SET_ZLL_ADDITIONAL_STATE       = 0x00D6,
  SL_ZIGBEE_EZSP_ZLL_OPERATION_IN_PROGRESS      = 0x00D7,
  SL_ZIGBEE_EZSP_ZLL_RX_ON_WHEN_IDLE_GET_ACTIVE = 0x00D8,
  SL_ZIGBEE_EZSP_ZLL_SCANNING_COMPLETE          = 0x00F6,
  SL_ZIGBEE_EZSP_GET_ZLL_PRIMARY_CHANNEL_MASK   = 0x00D9,
  SL_ZIGBEE_EZSP_GET_ZLL_SECONDARY_CHANNEL_MASK = 0x00DA,
  SL_ZIGBEE_EZSP_SET_ZLL_PRIMARY_CHANNEL_MASK   = 0x00DB,
  SL_ZIGBEE_EZSP_SET_ZLL_SECONDARY_CHANNEL_MASK = 0x00DC,
  SL_ZIGBEE_EZSP_ZLL_CLEAR_TOKENS               = 0x0025,

// Green Power Frames
  SL_ZIGBEE_EZSP_GP_PROXY_TABLE_PROCESS_GP_PAIRING = 0x00C9,
  SL_ZIGBEE_EZSP_D_GP_SEND                      = 0x00C6,
  SL_ZIGBEE_EZSP_D_GP_SENT_HANDLER              = 0x00C7,
  SL_ZIGBEE_EZSP_GPEP_INCOMING_MESSAGE_HANDLER  = 0x00C5,
  SL_ZIGBEE_EZSP_GP_PROXY_TABLE_GET_ENTRY       = 0x00C8,
  SL_ZIGBEE_EZSP_GP_PROXY_TABLE_LOOKUP          = 0x00C0,
  SL_ZIGBEE_EZSP_GP_PROXY_TABLE_REMOVE_ENTRY    = 0x005D,
  SL_ZIGBEE_EZSP_GP_CLEAR_PROXY_TABLE           = 0x005F,
  SL_ZIGBEE_EZSP_GP_SINK_TABLE_GET_ENTRY        = 0x00DD,
  SL_ZIGBEE_EZSP_GP_SINK_TABLE_LOOKUP           = 0x00DE,
  SL_ZIGBEE_EZSP_GP_SINK_TABLE_SET_ENTRY        = 0x00DF,
  SL_ZIGBEE_EZSP_GP_SINK_TABLE_REMOVE_ENTRY     = 0x00E0,
  SL_ZIGBEE_EZSP_GP_SINK_TABLE_FIND_OR_ALLOCATE_ENTRY = 0x00E1,
  SL_ZIGBEE_EZSP_GP_SINK_TABLE_CLEAR_ALL        = 0x00E2,
  SL_ZIGBEE_EZSP_GP_SINK_TABLE_INIT             = 0x0070,
  SL_ZIGBEE_EZSP_GP_SINK_TABLE_SET_SECURITY_FRAME_COUNTER = 0x00F5,
  SL_ZIGBEE_EZSP_GP_SINK_COMMISSION             = 0x010A,
  SL_ZIGBEE_EZSP_GP_TRANSLATION_TABLE_CLEAR     = 0x010B,
  SL_ZIGBEE_EZSP_GP_SINK_TABLE_GET_NUMBER_OF_ACTIVE_ENTRIES = 0x0118,

// Token Interface Frames
  SL_ZIGBEE_EZSP_GET_TOKEN_COUNT                = 0x0100,
  SL_ZIGBEE_EZSP_GET_TOKEN_INFO                 = 0x0101,
  SL_ZIGBEE_EZSP_GET_TOKEN_DATA                 = 0x0102,
  SL_ZIGBEE_EZSP_SET_TOKEN_DATA                 = 0x0103,
  SL_ZIGBEE_EZSP_RESET_NODE                     = 0x0104,
  SL_ZIGBEE_EZSP_GP_SECURITY_TEST_VECTORS       = 0x0117,
  SL_ZIGBEE_EZSP_TOKEN_FACTORY_RESET            = 0x0077,

// Dynamic Hardware Configuration Frames
  SL_ZIGBEE_EZSP_READ_PA_DESCRIPTOR             = 0x0152,
  SL_ZIGBEE_EZSP_WRITE_PA_DESCRIPTOR            = 0x0153,
  SL_ZIGBEE_EZSP_READ_PA_CURVE_SEGMENT          = 0x0154,
  SL_ZIGBEE_EZSP_WRITE_PA_CURVE_SEGMENT         = 0x0155,
  SL_ZIGBEE_EZSP_READ_PA_CURVE                  = 0x0156,
  SL_ZIGBEE_EZSP_WRITE_PA_CURVE                 = 0x0157,
  SL_ZIGBEE_EZSP_READ_PA_TABLE                  = 0x0158,
  SL_ZIGBEE_EZSP_WRITE_PA_TABLE                 = 0x0159,
  SL_ZIGBEE_EZSP_READ_RSSI_OFFSET               = 0x015A,
  SL_ZIGBEE_EZSP_WRITE_RSSI_OFFSET              = 0x015F,
  SL_ZIGBEE_EZSP_READ_PA_VOLTAGE                = 0x015B,
  SL_ZIGBEE_EZSP_WRITE_PA_VOLTAGE               = 0x015C,
  SL_ZIGBEE_EZSP_READ_PA_MODE                   = 0x015D,
  SL_ZIGBEE_EZSP_WRITE_PA_MODE                  = 0x015E,
  SL_ZIGBEE_EZSP_READ_CTUNE                     = 0x0160,
  SL_ZIGBEE_EZSP_WRITE_CTUNE                    = 0x0161,
  SL_ZIGBEE_EZSP_READ_DHC_VERSION               = 0x0162,
  SL_ZIGBEE_EZSP_WRITE_DHC_VERSION              = 0x0163,
  SL_ZIGBEE_EZSP_READ_PA_VERSION                = 0x0164,
  SL_ZIGBEE_EZSP_READ_PA_SIGNATURE              = 0x0166,
  SL_ZIGBEE_EZSP_WRITE_PA_SIGNATURE             = 0x0167,
  SL_ZIGBEE_EZSP_READ_PA_METADATA               = 0x0168,
  SL_ZIGBEE_EZSP_WRITE_PA_METADATA              = 0x0169
};

#endif // __EZSP_ENUM_H__

