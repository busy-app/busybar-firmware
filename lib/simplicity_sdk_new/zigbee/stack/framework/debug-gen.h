// (c) Ember, 2008.
//
// This file is generated! Please do not modify manually.
// If you wish to change, check apitrace_header.py.

#ifndef SILABS_APITRACE_GEN
#define SILABS_APITRACE_GEN

enum {
  EM_DEBUG_SET_BINDING = 0x00,   // Set binding function call.
  EM_DEBUG_DELETE_BINDING = 0x01,   // Delete binding function call.
  EM_DEBUG_CLEAR_BINDING_TABLE = 0x02,   // Clear binding table function call.
  EM_DEBUG_SEND_LIMITED_MULTICAST = 0x06,   // Send limited multicast function call.
  EM_DEBUG_SEND_UNICAST = 0x08,   // Send unicast function call.
  EM_DEBUG_SEND_BROADCAST = 0x09,   // Send broadcast function call.
  EM_DEBUG_PROXY_BROADCAST = 0x0A,   // Proxy broadcast function call.
  EM_DEBUG_CANCEL_MESSAGE = 0x0B,   // Cancel message function call.
  EM_DEBUG_SEND_REPLY = 0x0C,   // Send reply function call.
  EM_DEBUG_SET_REPLY_BINDING = 0x0D,   // Set reply binding function call.
  EM_DEBUG_MESSAGE_SENT = 0x0E,   // Message sent function call.
  EM_DEBUG_INCOMING_MESSAGE_HANDLER = 0x11,   // Incoming message handler function call.
  EM_DEBUG_STACK_STATUS_HANDLER = 0x13,   // Stack status handler function call.
  EM_DEBUG_NETWORK_INIT = 0x14,   // Network init function call.
  EM_DEBUG_FORM_NETWORK = 0x15,   // Form network function call.
  EM_DEBUG_JOIN_NETWORK = 0x16,   // Join network function call.
  EM_DEBUG_LEAVE_NETWORK = 0x17,   // Leave network function call.
  EM_DEBUG_PERMIT_JOINING = 0x18,   // Permit joining function call.
  EM_DEBUG_POLL_FOR_DATA = 0x19,   // Poll for data function call.
  EM_DEBUG_POLL_HANDLER = 0x1A,   // Poll handler function call.
  EM_DEBUG_TRUST_CENTER_JOIN_HANDLER = 0x1E,   // Trust center join handler function call.
  EM_DEBUG_SET_MESSAGE_FLAG = 0x20,   // Set message flag function call.
  EM_DEBUG_CLEAR_MESSAGE_FLAG = 0x21,   // Clear message flag function call.
  EM_DEBUG_POLL_COMPLETE_HANDLER = 0x22,   // Poll complete handler function call.
  EM_DEBUG_CHILD_JOIN_HANDLER = 0x23,   // Child join handler function call.
  EM_DEBUG_START_SCAN = 0x24,   // Start scan function call.
  EM_DEBUG_STOP_SCAN = 0x25,   // Stop scan function call.
  EM_DEBUG_SCAN_COMPLETE_HANDLER = 0x26,   // Scan complete handler function call.
  EM_DEBUG_NETWORK_FOUND_HANDLER = 0x27,   // Network found handler function call.
  EM_DEBUG_ENERGY_SCAN_RESULT_HANDLER = 0x28,   // Energy scan result handler function call.
  EM_DEBUG_SET_INITIAL_SECURITY_STATE = 0x2B,   // Set initial security state function call.
  EM_DEBUG_REJOIN_NETWORK = 0x2C,   // Rejoin network function call.
  EM_DEBUG_STACK_POWER_DOWN = 0x2D,   // Stack power down function call.
  EM_DEBUG_STACK_POWER_UP = 0x2E,   // Stack power up function call.
  EM_DEBUG_SET_EXTENDED_SECURITY_BITMASK = 0x2F,   // Set extended security bitmask function call.
  EM_DEBUG_RF4CE_SET_PAIRING_TABLE_ENTRY = 0x30,   // RF4CE set pairing table entry function call.
  EM_DEBUG_RF4CE_GET_PAIRING_TABLE_ENTRY = 0x31,   // RF4CE get pairing table entry function call.
  EM_DEBUG_RF4CE_DELETE_PAIRING_TABLE_ENTRY = 0x32,   // RF4CE get pairing table entry function call.
  EM_DEBUG_RF4CE_SET_APPLICATION_INFO = 0x33,   // RF4CE set application info function call.
  EM_DEBUG_RF4CE_GET_APPLICATION_INFO = 0x34,   // RF4CE get application info function call.
  EM_DEBUG_RF4CE_KEY_UPDATE = 0x35,   // RF4CE key update function call.
  EM_DEBUG_RF4CE_SEND = 0x36,   // RF4CE send function call.
  EM_DEBUG_RF4CE_MESSAGE_SENT_HANDLER = 0x37,   // RF4CE message sent handler function call.
  EM_DEBUG_RF4CE_INCOMING_MESSAGE_HANDLER = 0x38,   // RF4CE incoming message handler function call.
  EM_DEBUG_RF4CE_START = 0x39,   // RF4CE start function call.
  EM_DEBUG_RF4CE_STOP = 0x3A,   // RF4CE stop function call.
  EM_DEBUG_RF4CE_DISCOVERY = 0x3B,   // RF4CE discovery function call.
  EM_DEBUG_RF4CE_DISCOVERY_COMPLETE_HANDLER = 0x3C,   // RF4CE discovery complete handler function call.
  EM_DEBUG_RF4CE_DISCOVERY_REQUEST_HANDLER = 0x3D,   // RF4CE discovery request handler function call.
  EM_DEBUG_RF4CE_DISCOVERY_RESPONSE_HANDLER = 0x3E,   // RF4CE discovery response handler function call.
  EM_DEBUG_RF4CE_ENABLE_AUTO_DISCOVERY_RESPONSE = 0x3F,   // RF4CE enable auto discovery response function call.
  EM_DEBUG_RF4CE_AUTO_DISCOVERY_RESPONSE_COMPLETE_HANDLER = 0x40,   // RF4CE auto discovery response complete handler function call.
  EM_DEBUG_RF4CE_PAIR = 0x41,   // RF4CE pair function call.
  EM_DEBUG_RF4CE_PAIR_COMPLETE_HANDLER = 0x42,   // RF4CE pair complete handler function call.
  EM_DEBUG_RF4CE_PAIR_REQUEST_HANDLER = 0x43,   // RF4CE pair request handler function call.
  EM_DEBUG_RF4CE_UNPAIR = 0x44,   // RF4CE unpair function call.
  EM_DEBUG_RF4CE_UNPAIR_HANDLER = 0x45,   // RF4CE unpair handler function call.
  EM_DEBUG_RF4CE_UNPAIR_COMPLETE_HANDLER = 0x46,   // RF4CE unpair complete handler function call.
  EM_DEBUG_RF4CE_SET_POWER_SAVING_PARAMETERS = 0x47,   // RF4CE set power saving parameters function call.
  EM_DEBUG_RF4CE_SET_FREQUENCY_AGILITY_PARAMETERS = 0x48,   // RF4CE set frequency agility parameters function call.
  EM_DEBUG_RF4CE_SET_DISCOVERY_LQI_THRESHOLD = 0x49,   // RF4CE set discovery LQI threshold function call.
};

#define API_TRACE_SET_BINDING(index)                        \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,   \
                                      "BB",                 \
                                      EM_DEBUG_SET_BINDING, \
                                      (index)               \
                                      ))
#define API_TRACE_DELETE_BINDING(index)                        \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,      \
                                      "BB",                    \
                                      EM_DEBUG_DELETE_BINDING, \
                                      (index)                  \
                                      ))
#define API_TRACE_CLEAR_BINDING_TABLE( )                           \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,          \
                                      "B",                         \
                                      EM_DEBUG_CLEAR_BINDING_TABLE \
                                      ))
#define API_TRACE_SEND_LIMITED_MULTICAST(groupId, profileId, clusterId, sourceEndpoint, destinationEndpoint, options, radius, broadcastAddr) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                                                                                    \
                                      "BWWBBBWBB",                                                                                           \
                                      EM_DEBUG_SEND_LIMITED_MULTICAST,                                                                       \
                                      (groupId),                                                                                             \
                                      (profileId),                                                                                           \
                                      (clusterId),                                                                                           \
                                      (sourceEndpoint),                                                                                      \
                                      (destinationEndpoint),                                                                                 \
                                      (options),                                                                                             \
                                      (radius),                                                                                              \
                                      (broadcastAddr)                                                                                        \
                                      ))
#define API_TRACE_SEND_UNICAST(indexOrDestination, profileId, clusterId, sourceEndpoint, destinationEndpoint, options) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                                                              \
                                      "BWWWBBW",                                                                       \
                                      EM_DEBUG_SEND_UNICAST,                                                           \
                                      (indexOrDestination),                                                            \
                                      (profileId),                                                                     \
                                      (clusterId),                                                                     \
                                      (sourceEndpoint),                                                                \
                                      (destinationEndpoint),                                                           \
                                      (options)                                                                        \
                                      ))
#define API_TRACE_SEND_BROADCAST(source, destination, sequence, profileId, clusterId, sourceEndpoint, destinationEndpoint, options, radius) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                                                                                   \
                                      "BWWBWWBBWB",                                                                                         \
                                      EM_DEBUG_PROXY_BROADCAST,                                                                             \
                                      (source),                                                                                             \
                                      (destination),                                                                                        \
                                      (sequence),                                                                                           \
                                      (profileId),                                                                                          \
                                      (clusterId),                                                                                          \
                                      (sourceEndpoint),                                                                                     \
                                      (destinationEndpoint),                                                                                \
                                      (options),                                                                                            \
                                      (radius)                                                                                              \
                                      ))
#define API_TRACE_CANCEL_MESSAGE(message)                      \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,      \
                                      "BB",                    \
                                      EM_DEBUG_CANCEL_MESSAGE, \
                                      (message)                \
                                      ))
#define API_TRACE_SEND_REPLY(clusterId)                    \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,  \
                                      "BW",                \
                                      EM_DEBUG_SEND_REPLY, \
                                      (clusterId)          \
                                      ))
#define API_TRACE_SET_REPLY_BINDING(index)                        \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,         \
                                      "BB",                       \
                                      EM_DEBUG_SET_REPLY_BINDING, \
                                      (index)                     \
                                      ))
#define API_TRACE_MESSAGE_SENT(mode, destination, profileId, clusterId, sourceEndpoint, destinationEndpoint, options, status) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                                                                     \
                                      "BBWWWBBWB",                                                                            \
                                      EM_DEBUG_MESSAGE_SENT,                                                                  \
                                      (mode),                                                                                 \
                                      (destination),                                                                          \
                                      (profileId),                                                                            \
                                      (clusterId),                                                                            \
                                      (sourceEndpoint),                                                                       \
                                      (destinationEndpoint),                                                                  \
                                      (options),                                                                              \
                                      (status)                                                                                \
                                      ))
#define API_TRACE_INCOMING_MESSAGE_HANDLER(type, profileId, clusterId, sourceEndpoint, destinationEndpoint, options) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                                                            \
                                      "BBWWBBW",                                                                     \
                                      EM_DEBUG_INCOMING_MESSAGE_HANDLER,                                             \
                                      (type),                                                                        \
                                      (profileId),                                                                   \
                                      (clusterId),                                                                   \
                                      (sourceEndpoint),                                                              \
                                      (destinationEndpoint),                                                         \
                                      (options)                                                                      \
                                      ))
#define API_TRACE_STACK_STATUS_HANDLER(stackStatus)                  \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,            \
                                      "BB",                          \
                                      EM_DEBUG_STACK_STATUS_HANDLER, \
                                      (stackStatus)                  \
                                      ))
#define API_TRACE_NETWORK_INIT(nodeType)                     \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,    \
                                      "BB",                  \
                                      EM_DEBUG_NETWORK_INIT, \
                                      (nodeType)             \
                                      ))
#define API_TRACE_FORM_NETWORK(extendedPanId, panId, radioTxPower, radioChannel) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                        \
                                      "B8pWBB",                                  \
                                      EM_DEBUG_FORM_NETWORK,                     \
                                      (extendedPanId),                           \
                                      (panId),                                   \
                                      (radioTxPower),                            \
                                      (radioChannel)                             \
                                      ))
#define API_TRACE_JOIN_NETWORK(nodeType, extendedPanId, panId, radioTxPower, radioChannel, joinMethod, nwkManagerId, nwkUpdateId, channels) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                                                                                   \
                                      "BB8pWBBBWBD",                                                                                        \
                                      EM_DEBUG_JOIN_NETWORK,                                                                                \
                                      (nodeType),                                                                                           \
                                      (extendedPanId),                                                                                      \
                                      (panId),                                                                                              \
                                      (radioTxPower),                                                                                       \
                                      (radioChannel),                                                                                       \
                                      (joinMethod),                                                                                         \
                                      (nwkManagerId),                                                                                       \
                                      (nwkUpdateId),                                                                                        \
                                      (channels)                                                                                            \
                                      ))
#define API_TRACE_LEAVE_NETWORK( )                           \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,    \
                                      "B",                   \
                                      EM_DEBUG_LEAVE_NETWORK \
                                      ))
#define API_TRACE_PERMIT_JOINING(duration)                     \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,      \
                                      "BB",                    \
                                      EM_DEBUG_PERMIT_JOINING, \
                                      (duration)               \
                                      ))
#define API_TRACE_POLL_FOR_DATA( )                           \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,    \
                                      "B",                   \
                                      EM_DEBUG_POLL_FOR_DATA \
                                      ))
#define API_TRACE_POLL_HANDLER(id, sendAppJitMessage)        \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,    \
                                      "BWB",                 \
                                      EM_DEBUG_POLL_HANDLER, \
                                      (id),                  \
                                      (sendAppJitMessage)    \
                                      ))
#define API_TRACE_TRUST_CENTER_JOIN_HANDLER(status, decision)             \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                 \
                                      "BBB",                              \
                                      EM_DEBUG_TRUST_CENTER_JOIN_HANDLER, \
                                      (status),                           \
                                      (decision)                          \
                                      ))
#define API_TRACE_SET_MESSAGE_FLAG(childId)                      \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,        \
                                      "BW",                      \
                                      EM_DEBUG_SET_MESSAGE_FLAG, \
                                      (childId)                  \
                                      ))
#define API_TRACE_CLEAR_MESSAGE_FLAG(childId)                      \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,          \
                                      "BW",                        \
                                      EM_DEBUG_CLEAR_MESSAGE_FLAG, \
                                      (childId)                    \
                                      ))
#define API_TRACE_POLL_COMPLETE_HANDLER(status)                       \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,             \
                                      "BB",                           \
                                      EM_DEBUG_POLL_COMPLETE_HANDLER, \
                                      (status)                        \
                                      ))
#define API_TRACE_CHILD_JOIN_HANDLER(childIndex, joining, childId, childEui64, childType) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                                 \
                                      "BBBW8pB",                                          \
                                      EM_DEBUG_CHILD_JOIN_HANDLER,                        \
                                      (childIndex),                                       \
                                      (joining),                                          \
                                      (childId),                                          \
                                      (childEui64),                                       \
                                      (childType)                                         \
                                      ))
#define API_TRACE_START_SCAN(scanType, channelMask, duration) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,     \
                                      "BBDB",                 \
                                      EM_DEBUG_START_SCAN,    \
                                      (scanType),             \
                                      (channelMask),          \
                                      (duration)              \
                                      ))
#define API_TRACE_STOP_SCAN( )                            \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE, \
                                      "B",                \
                                      EM_DEBUG_STOP_SCAN  \
                                      ))
#define API_TRACE_SCAN_COMPLETE_HANDLER(data, status)                 \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,             \
                                      "BBB",                          \
                                      EM_DEBUG_SCAN_COMPLETE_HANDLER, \
                                      (data),                         \
                                      (status)                        \
                                      ))
#define API_TRACE_NETWORK_FOUND_HANDLER(panId, permitJoin, stackProfile) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                \
                                      "BWBB",                            \
                                      EM_DEBUG_NETWORK_FOUND_HANDLER,    \
                                      (panId),                           \
                                      (permitJoin),                      \
                                      (stackProfile)                     \
                                      ))
#define API_TRACE_ENERGY_SCAN_RESULT_HANDLER(channel, rssi)                \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                  \
                                      "BBB",                               \
                                      EM_DEBUG_ENERGY_SCAN_RESULT_HANDLER, \
                                      (channel),                           \
                                      (rssi)                               \
                                      ))
#define API_TRACE_SET_INITIAL_SECURITY_STATE(mask, preconfiguredKey, networkKey, keySequence) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                                     \
                                      "BW16p16pB",                                            \
                                      EM_DEBUG_SET_INITIAL_SECURITY_STATE,                    \
                                      (mask),                                                 \
                                      (preconfiguredKey),                                     \
                                      (networkKey),                                           \
                                      (keySequence)                                           \
                                      ))
#define API_TRACE_REJOIN_NETWORK(haveKey, channelMask, status) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,      \
                                      "BBDB",                  \
                                      EM_DEBUG_REJOIN_NETWORK, \
                                      (haveKey),               \
                                      (channelMask),           \
                                      (status)                 \
                                      ))
#define API_TRACE_STACK_POWER_DOWN( )                           \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,       \
                                      "B",                      \
                                      EM_DEBUG_STACK_POWER_DOWN \
                                      ))
#define API_TRACE_STACK_POWER_UP( )                           \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,     \
                                      "B",                    \
                                      EM_DEBUG_STACK_POWER_UP \
                                      ))
#define API_TRACE_SET_EXTENDED_SECURITY_BITMASK(mask)                         \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                     \
                                      "BW",                                   \
                                      EM_DEBUG_SET_EXTENDED_SECURITY_BITMASK, \
                                      (mask)                                  \
                                      ))
#define API_TRACE_RF4CE_SET_PAIRING_TABLE_ENTRY(pairingIndex, securityLinkKey, destLongId, frameCounter, sourceNodeId, destPanId, destNodeId, destVendorId, destProfileIdListLength, info, channel, capabilities, lastSeqn) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                                                                                                                                                                   \
                                      "BB16p8pDWWWWBBBBB",                                                                                                                                                                  \
                                      EM_DEBUG_RF4CE_SET_PAIRING_TABLE_ENTRY,                                                                                                                                               \
                                      (pairingIndex),                                                                                                                                                                       \
                                      (securityLinkKey),                                                                                                                                                                    \
                                      (destLongId),                                                                                                                                                                         \
                                      (frameCounter),                                                                                                                                                                       \
                                      (sourceNodeId),                                                                                                                                                                       \
                                      (destPanId),                                                                                                                                                                          \
                                      (destNodeId),                                                                                                                                                                         \
                                      (destVendorId),                                                                                                                                                                       \
                                      (destProfileIdListLength),                                                                                                                                                            \
                                      (info),                                                                                                                                                                               \
                                      (channel),                                                                                                                                                                            \
                                      (capabilities),                                                                                                                                                                       \
                                      (lastSeqn)                                                                                                                                                                            \
                                      ))
#define API_TRACE_RF4CE_GET_PAIRING_TABLE_ENTRY(pairingIndex)                 \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                     \
                                      "BB",                                   \
                                      EM_DEBUG_RF4CE_GET_PAIRING_TABLE_ENTRY, \
                                      (pairingIndex)                          \
                                      ))
#define API_TRACE_RF4CE_DELETE_PAIRING_TABLE_ENTRY(pairingIndex)                 \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                        \
                                      "BB",                                      \
                                      EM_DEBUG_RF4CE_DELETE_PAIRING_TABLE_ENTRY, \
                                      (pairingIndex)                             \
                                      ))
#define API_TRACE_RF4CE_SET_APPLICATION_INFO(capabilities)                 \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                  \
                                      "BB",                                \
                                      EM_DEBUG_RF4CE_SET_APPLICATION_INFO, \
                                      (capabilities)                       \
                                      ))
#define API_TRACE_RF4CE_GET_APPLICATION_INFO( )                           \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                 \
                                      "B",                                \
                                      EM_DEBUG_RF4CE_GET_APPLICATION_INFO \
                                      ))
#define API_TRACE_RF4CE_KEY_UPDATE(pairingIndex, key)            \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,        \
                                      "BB16p",                   \
                                      EM_DEBUG_RF4CE_KEY_UPDATE, \
                                      (pairingIndex),            \
                                      (key)                      \
                                      ))
#define API_TRACE_RF4CE_SEND(pairingIndex, profileId, vendorId, txOptions, messageTag) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                              \
                                      "BBBWBB",                                        \
                                      EM_DEBUG_RF4CE_SEND,                             \
                                      (pairingIndex),                                  \
                                      (profileId),                                     \
                                      (vendorId),                                      \
                                      (txOptions),                                     \
                                      (messageTag)                                     \
                                      ))
#define API_TRACE_RF4CE_MESSAGE_SENT_HANDLER(status, pairingIndex, txOptions, profileId, vendorId, messageTag) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                                                      \
                                      "BBBBBWB",                                                               \
                                      EM_DEBUG_RF4CE_MESSAGE_SENT_HANDLER,                                     \
                                      (status),                                                                \
                                      (pairingIndex),                                                          \
                                      (txOptions),                                                             \
                                      (profileId),                                                             \
                                      (vendorId),                                                              \
                                      (messageTag)                                                             \
                                      ))
#define API_TRACE_RF4CE_INCOMING_MESSAGE_HANDLER(pairingIndex, profileId, vendorId, txOptions) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                                      \
                                      "BBBWB",                                                 \
                                      EM_DEBUG_RF4CE_INCOMING_MESSAGE_HANDLER,                 \
                                      (pairingIndex),                                          \
                                      (profileId),                                             \
                                      (vendorId),                                              \
                                      (txOptions)                                              \
                                      ))
#define API_TRACE_RF4CE_START(nodeCapabilities, vendorId, power) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,        \
                                      "BBWB",                    \
                                      EM_DEBUG_RF4CE_START,      \
                                      (nodeCapabilities),        \
                                      (vendorId),                \
                                      (power)                    \
                                      ))
#define API_TRACE_RF4CE_STOP( )                           \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE, \
                                      "B",                \
                                      EM_DEBUG_RF4CE_STOP \
                                      ))
#define API_TRACE_RF4CE_DISCOVERY(panId, nodeId, searchDevType, discDuration, maxDiscRepetitions) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                                         \
                                      "BWWBWB",                                                   \
                                      EM_DEBUG_RF4CE_DISCOVERY,                                   \
                                      (panId),                                                    \
                                      (nodeId),                                                   \
                                      (searchDevType),                                            \
                                      (discDuration),                                             \
                                      (maxDiscRepetitions)                                        \
                                      ))
#define API_TRACE_RF4CE_DISCOVERY_COMPLETE_HANDLER(status)                       \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                        \
                                      "BB",                                      \
                                      EM_DEBUG_RF4CE_DISCOVERY_COMPLETE_HANDLER, \
                                      (status)                                   \
                                      ))
#define API_TRACE_RF4CE_DISCOVERY_REQUEST_HANDLER(srcIeeeAddr, nodeCapabilities, vendorId, appCapabilities, searchDevType, rxLinkQuality) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                                                                                 \
                                      "B8pBWBBB",                                                                                         \
                                      EM_DEBUG_RF4CE_DISCOVERY_REQUEST_HANDLER,                                                           \
                                      (srcIeeeAddr),                                                                                      \
                                      (nodeCapabilities),                                                                                 \
                                      (vendorId),                                                                                         \
                                      (appCapabilities),                                                                                  \
                                      (searchDevType),                                                                                    \
                                      (rxLinkQuality)                                                                                     \
                                      ))
#define API_TRACE_RF4CE_DISCOVERY_RESPONSE_HANDLER(atCapacity, channel, panId, srcIeeeAddr, nodeCapabilities, vendorId, appCapabilities, rxLinkQuality, discRequestLqi) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                                                                                                               \
                                      "BBBW8pBWBBB",                                                                                                                    \
                                      EM_DEBUG_RF4CE_DISCOVERY_RESPONSE_HANDLER,                                                                                        \
                                      (atCapacity),                                                                                                                     \
                                      (channel),                                                                                                                        \
                                      (panId),                                                                                                                          \
                                      (srcIeeeAddr),                                                                                                                    \
                                      (nodeCapabilities),                                                                                                               \
                                      (vendorId),                                                                                                                       \
                                      (appCapabilities),                                                                                                                \
                                      (rxLinkQuality),                                                                                                                  \
                                      (discRequestLqi)                                                                                                                  \
                                      ))
#define API_TRACE_RF4CE_ENABLE_AUTO_DISCOVERY_RESPONSE(duration)                     \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                            \
                                      "BW",                                          \
                                      EM_DEBUG_RF4CE_ENABLE_AUTO_DISCOVERY_RESPONSE, \
                                      (duration)                                     \
                                      ))
#define API_TRACE_RF4CE_AUTO_DISCOVERY_RESPONSE_COMPLETE_HANDLER(status, srcIeeeAddr, nodeCapabilities, vendorId, appCapabilities, searchDevType) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                                                                                         \
                                      "BB8pBWBB",                                                                                                 \
                                      EM_DEBUG_RF4CE_AUTO_DISCOVERY_RESPONSE_COMPLETE_HANDLER,                                                    \
                                      (status),                                                                                                   \
                                      (srcIeeeAddr),                                                                                              \
                                      (nodeCapabilities),                                                                                         \
                                      (vendorId),                                                                                                 \
                                      (appCapabilities),                                                                                          \
                                      (searchDevType)                                                                                             \
                                      ))
#define API_TRACE_RF4CE_PAIR(channel, panId, ieeeAddr, keyExchangeTransferCount) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                        \
                                      "BBW8pB",                                  \
                                      EM_DEBUG_RF4CE_PAIR,                       \
                                      (channel),                                 \
                                      (panId),                                   \
                                      (ieeeAddr),                                \
                                      (keyExchangeTransferCount)                 \
                                      ))
#define API_TRACE_RF4CE_PAIR_COMPLETE_HANDLER(status, pairingIndex, vendorId, appCapabilities) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                                      \
                                      "BBBWB",                                                 \
                                      EM_DEBUG_RF4CE_PAIR_COMPLETE_HANDLER,                    \
                                      (status),                                                \
                                      (pairingIndex),                                          \
                                      (vendorId),                                              \
                                      (appCapabilities)                                        \
                                      ))
#define API_TRACE_RF4CE_PAIR_REQUEST_HANDLER(status, pairingIndex, srcIeeeAddr, nodeCapabilities, vendorId, appCapabilities, keyExchangeTransferCount) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                                                                                              \
                                      "BBB8pBWBB",                                                                                                     \
                                      EM_DEBUG_RF4CE_PAIR_REQUEST_HANDLER,                                                                             \
                                      (status),                                                                                                        \
                                      (pairingIndex),                                                                                                  \
                                      (srcIeeeAddr),                                                                                                   \
                                      (nodeCapabilities),                                                                                              \
                                      (vendorId),                                                                                                      \
                                      (appCapabilities),                                                                                               \
                                      (keyExchangeTransferCount)                                                                                       \
                                      ))
#define API_TRACE_RF4CE_UNPAIR(pairingIndex)                 \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,    \
                                      "BB",                  \
                                      EM_DEBUG_RF4CE_UNPAIR, \
                                      (pairingIndex)         \
                                      ))
#define API_TRACE_RF4CE_UNPAIR_HANDLER(pairingIndex)                 \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,            \
                                      "BB",                          \
                                      EM_DEBUG_RF4CE_UNPAIR_HANDLER, \
                                      (pairingIndex)                 \
                                      ))
#define API_TRACE_RF4CE_UNPAIR_COMPLETE_HANDLER(pairingIndex)                 \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                     \
                                      "BB",                                   \
                                      EM_DEBUG_RF4CE_UNPAIR_COMPLETE_HANDLER, \
                                      (pairingIndex)                          \
                                      ))
#define API_TRACE_RF4CE_SET_POWER_SAVING_PARAMETERS(dutyCycle, activePeriod)      \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                         \
                                      "BDD",                                      \
                                      EM_DEBUG_RF4CE_SET_POWER_SAVING_PARAMETERS, \
                                      (dutyCycle),                                \
                                      (activePeriod)                              \
                                      ))
#define API_TRACE_RF4CE_SET_FREQUENCY_AGILITY_PARAMETERS(rssiWindowSize, channelChangeReads, rssiThreshold, readInterval, readDuration) \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                                                                               \
                                      "BBBBWB",                                                                                         \
                                      EM_DEBUG_RF4CE_SET_FREQUENCY_AGILITY_PARAMETERS,                                                  \
                                      (rssiWindowSize),                                                                                 \
                                      (channelChangeReads),                                                                             \
                                      (rssiThreshold),                                                                                  \
                                      (readInterval),                                                                                   \
                                      (readDuration)                                                                                    \
                                      ))
#define API_TRACE_RF4CE_SET_DISCOVERY_LQI_THRESHOLD(threshold)                    \
  IF_DEBUG(sli_zigbee_debug_api_trace(EM_DEBUG_API_TRACE,                         \
                                      "BB",                                       \
                                      EM_DEBUG_RF4CE_SET_DISCOVERY_LQI_THRESHOLD, \
                                      (threshold)                                 \
                                      ))

#endif // SILABS_APITRACE_GEN
