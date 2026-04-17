/******************************************************************************/
/**
 * @file
 * @brief Development Kit debugging utilities for stack on host apps
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 *
 * https://www.silabs.com/about-us/legal/master-software-license-agreement
 *
 * This software is distributed to you in Source Code format and is governed by
 * the sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef SILABS_ZIGBEE_DEBUG_STACK_ON_HOST_H
#define SILABS_ZIGBEE_DEBUG_STACK_ON_HOST_H

#include <stdio.h>
#include "common/logging.hpp"

// Allow for quick debugging instrumentation on host stack
#define HOST_DEBUG_LOG(...)  otLogInfoPlat(__VA_ARGS__)

#define API_TRACE_SET_BINDING(index)    \
  otLogInfoPlat("[%s %s] %s=%d",        \
                __func__,               \
                "EM_DEBUG_SET_BINDING", \
                "index", index)
#define API_TRACE_DELETE_BINDING(index)    \
  otLogInfoPlat("[%s %s] %s=%d",           \
                __func__,                  \
                "EM_DEBUG_DELETE_BINDING", \
                "index", index)

#define API_TRACE_CLEAR_BINDING_TABLE()         \
  otLogInfoPlat("[%s %s]",                      \
                "EM_DEBUG_CLEAR_BINDING_TABLE", \
                __func__)

#define API_TRACE_SEND_LIMITED_MULTICAST(groupId, profileId, clusterId, sourceEndpoint, destinationEndpoint, options, radius, broadcastAddr) \
  otLogInfoPlat("[%s %s] %s=0x%04X %s=0x%04X %s=0x%04X %s=%d %s=%d %s=%d %s=%d %s=%d",                                                       \
                __func__,                                                                                                                    \
                "EM_DEBUG_SEND_LIMITED_MULTICAST",                                                                                           \
                "groupId", groupId,                                                                                                          \
                "profileId", profileId,                                                                                                      \
                "clusterId", clusterId,                                                                                                      \
                "sourceEndpoint", sourceEndpoint,                                                                                            \
                "destinationEndpoint", destinationEndpoint,                                                                                  \
                "options", options,                                                                                                          \
                "radius", radius,                                                                                                            \
                "broadcastAddr", broadcastAddr)

#define API_TRACE_SEND_UNICAST(indexOrDestination, profileId, clusterId, sourceEndpoint, destinationEndpoint, options) \
  otLogInfoPlat("[%s %s] %s=0x%04X %s=0x%04X %s=0x%04X %s=%d %s=%d %s=%d",                                             \
                __func__,                                                                                              \
                "EM_DEBUG_SEND_UNICAST",                                                                               \
                "indexOrDestination", indexOrDestination,                                                              \
                "profileId", profileId,                                                                                \
                "clusterId", clusterId,                                                                                \
                "sourceEndpoint", sourceEndpoint,                                                                      \
                "destinationEndpoint", destinationEndpoint,                                                            \
                "options", options)

#define API_TRACE_SEND_BROADCAST(source, destination, sequence, profileId, clusterId, sourceEndpoint, destinationEndpoint, options, radius) \
  otLogInfoPlat("[%s %s] %s=0x%04X %s=0x%04X %s=%d %s=0x%04X %s=0x%04X %s=%d %s=%d %s=%d %s=%d",                                            \
                __func__,                                                                                                                   \
                "EM_DEBUG_PROXY_BROADCAST",                                                                                                 \
                "source", source,                                                                                                           \
                "destination", destination,                                                                                                 \
                "sequence", sequence,                                                                                                       \
                "profileId", profileId,                                                                                                     \
                "clusterId", clusterId,                                                                                                     \
                "sourceEndpoint", sourceEndpoint,                                                                                           \
                "destinationEndpoint", destinationEndpoint,                                                                                 \
                "options", options,                                                                                                         \
                "radius", radius)

#define API_TRACE_CANCEL_MESSAGE(message)  \
  otLogInfoPlat("[%s %s] %s=%d",           \
                __func__,                  \
                "EM_DEBUG_CANCEL_MESSAGE", \
                "message", message)

#define API_TRACE_SEND_REPLY(clusterId) \
  otLogInfoPlat("[%s %s] %s=0x%04X",    \
                __func__,               \
                "EM_DEBUG_SEND_REPLY",  \
                "clusterId", clusterId)

#define API_TRACE_SET_REPLY_BINDING(index)    \
  otLogInfoPlat("[%s %s] %s=%d",              \
                __func__,                     \
                "EM_DEBUG_SET_REPLY_BINDING", \
                "index", index)

#define API_TRACE_MESSAGE_SENT(mode, destination, profileId, clusterId, sourceEndpoint, destinationEndpoint, options, status) \
  otLogInfoPlat("[%s %s] %s=%d %s=0x%04X %s=0x%04X %s=0x%04X %s=%d %s=%d %s=%d %s=0x02%02X",                                  \
                __func__,                                                                                                     \
                "EM_DEBUG_MESSAGE_SENT",                                                                                      \
                "mode", mode,                                                                                                 \
                "destination", destination,                                                                                   \
                "profileId", profileId,                                                                                       \
                "clusterId", clusterId,                                                                                       \
                "sourceEndpoint", sourceEndpoint,                                                                             \
                "destinationEndpoint", destinationEndpoint,                                                                   \
                "options", options,                                                                                           \
                "status", status)

#define API_TRACE_INCOMING_MESSAGE_HANDLER(type, profileId, clusterId, sourceEndpoint, destinationEndpoint, options) \
  otLogInfoPlat("[%s %s] %s=%d %s=0x%04X %s=0x%04X %s=%d %s=%d %s=%d",                                               \
                __func__,                                                                                            \
                "EM_DEBUG_INCOMING_MESSAGE_HANDLER",                                                                 \
                "type", type,                                                                                        \
                "profileId", profileId,                                                                              \
                "clusterId", clusterId,                                                                              \
                "sourceEndpoint", sourceEndpoint,                                                                    \
                "destinationEndpoint", destinationEndpoint,                                                          \
                "options", options)

#define API_TRACE_STACK_STATUS_HANDLER(stackStatus) \
  otLogInfoPlat("[%s %s] %s=0x02%02X",              \
                __func__,                           \
                "EM_DEBUG_STACK_STATUS_HANDLER",    \
                "stackStatus", stackStatus)

#define API_TRACE_NETWORK_INIT(nodeType) \
  otLogInfoPlat("[%s %s] %s=%d",         \
                __func__,                \
                "EM_DEBUG_NETWORK_INIT", \
                "nodeType", nodeType)

// extendedPanId is an "uint8_t [EXTENDED_PAN_ID_SIZE]"" arrary
// so not making OT logger to output it at this moment.
#define API_TRACE_FORM_NETWORK(extendedPanId, panId, radioTxPower, radioChannel) \
  otLogInfoPlat("[%s %s] %s=0x%04X %s=%d %s=%d",                                 \
                __func__,                                                        \
                "EM_DEBUG_FORM_NETWORK",                                         \
                "panId", panId,                                                  \
                "radioTxPower", radioTxPower,                                    \
                "radioChannel", radioChannel)

// extendedPanId is an "uint8_t [EXTENDED_PAN_ID_SIZE]"" arrary
// so not making OT logger to output it at this moment.
#define API_TRACE_JOIN_NETWORK(nodeType, extendedPanId, panId, radioTxPower, radioChannel, joinMethod, nwkManagerId, nwkUpdateId, channels) \
  otLogInfoPlat("[%s %s] %s=%d %s=0x%04X %s=%d %s=%d %s=%d %s=0x%04X %s=%d %s=0x%08X",                                                      \
                __func__,                                                                                                                   \
                "EM_DEBUG_JOIN_NETWORK",                                                                                                    \
                "nodeType", nodeType,                                                                                                       \
                "panId", panId,                                                                                                             \
                "radioTxPower", radioTxPower,                                                                                               \
                "radioChannel", radioChannel,                                                                                               \
                "joinMethod", joinMethod,                                                                                                   \
                "nwkManagerId", nwkManagerId,                                                                                               \
                "nwkUpdateId", nwkUpdateId,                                                                                                 \
                "channels", channels)

#define API_TRACE_LEAVE_NETWORK( ) \
  otLogInfoPlat("[%s %s]",         \
                __func__,          \
                "EM_DEBUG_LEAVE_NETWORK")

#define API_TRACE_PERMIT_JOINING(duration) \
  otLogInfoPlat("[%s %s] %s=%d",           \
                __func__,                  \
                "EM_DEBUG_PERMIT_JOINING", \
                "duration", duration)

#define API_TRACE_POLL_FOR_DATA( ) \
  otLogInfoPlat("[%s %s]",         \
                __func__,          \
                "EM_DEBUG_POLL_FOR_DATA")

#define API_TRACE_POLL_HANDLER(id, sendAppJitMessage) \
  otLogInfoPlat("[%s %s] %s=0x%04X %s=%d",            \
                __func__,                             \
                "EM_DEBUG_POLL_HANDLER",              \
                "id", id,                             \
                "sendAppJitMessage", sendAppJitMessage)

#define API_TRACE_TRUST_CENTER_JOIN_HANDLER(status, decision) \
  otLogInfoPlat("[%s %s] %s=%d %s=%d",                        \
                __func__,                                     \
                "EM_DEBUG_TRUST_CENTER_JOIN_HANDLER",         \
                "status", status,                             \
                "decision", decision)

#define API_TRACE_SET_MESSAGE_FLAG(childId)  \
  otLogInfoPlat("[%s %s] %s=%04X",           \
                __func__,                    \
                "EM_DEBUG_SET_MESSAGE_FLAG", \
                "childId", childId)

#define API_TRACE_CLEAR_MESSAGE_FLAG(childId)  \
  otLogInfoPlat("[%s %s] %s=%04X",             \
                __func__,                      \
                "EM_DEBUG_CLEAR_MESSAGE_FLAG", \
                "childId", childId)

#define API_TRACE_POLL_COMPLETE_HANDLER(status)   \
  otLogInfoPlat("[%s %s] %s=%08X",                \
                __func__,                         \
                "EM_DEBUG_POLL_COMPLETE_HANDLER", \
                "status", status)

#define API_TRACE_CHILD_JOIN_HANDLER(childIndex, joining, childId, childEui64, childType) \
  otLogInfoPlat("[%s %s] %s=%d %s=%d",                                                    \
                __func__,                                                                 \
                "EM_DEBUG_CHILD_JOIN_HANDLER",                                            \
                "childIndex", childIndex,                                                 \
                "joining", joining)

#define API_TRACE_START_SCAN(scanType, channelMask, duration) \
  otLogInfoPlat("[%s %s] %s=%d %s=0x%08X %s=%d",              \
                __func__,                                     \
                "EM_DEBUG_START_SCAN",                        \
                "scanType", scanType,                         \
                "channelMask", channelMask,                   \
                "duration", duration)

#define API_TRACE_STOP_SCAN( ) \
  otLogInfoPlat("[%s %s]",     \
                __func__,      \
                "EM_DEBUG_STOP_SCAN")

#define API_TRACE_SCAN_COMPLETE_HANDLER(data, status) \
  otLogInfoPlat("[%s %s] %s=%d %s=0x%08X",            \
                __func__,                             \
                "EM_DEBUG_SCAN_COMPLETE_HANDLER",     \
                "data", data,                         \
                "status", status)

#define API_TRACE_NETWORK_FOUND_HANDLER(panId, permitJoin, stackProfile) \
  otLogInfoPlat("[%s %s] %s=0x%04X %s=%d %s=0x%02X",                     \
                __func__,                                                \
                "EM_DEBUG_NETWORK_FOUND_HANDLER",                        \
                "panId", panId,                                          \
                "permitJoin", permitJoin,                                \
                "stackProfile", stackProfile)

#define API_TRACE_ENERGY_SCAN_RESULT_HANDLER(channel, rssi) \
  otLogInfoPlat("[%s %s] %s=%d %s=0x%d",                    \
                __func__,                                   \
                "EM_DEBUG_ENERGY_SCAN_RESULT_HANDLER",      \
                "channel", channel,                         \
                "rssi", rssi)

// preconfiguredKey and networkKey are an "uint8_t [SL_ZIGBEE_ENCRYPTION_KEY_SIZE]"" arrary
// so not making OT logger to output them at this moment.
#define API_TRACE_SET_INITIAL_SECURITY_STATE(mask, preconfiguredKey, networkKey, keySequence) \
  otLogInfoPlat("[%s %s] %s=0x%04X %s=0x%d",                                                  \
                __func__,                                                                     \
                "EM_DEBUG_SET_INITIAL_SECURITY_STATE",                                        \
                "mask", mask,                                                                 \
                "keySequence", keySequence)

#define API_TRACE_REJOIN_NETWORK(haveKey, channelMask, status) \
  otLogInfoPlat("[%s %s] %s=%d, %s=0x%08X %s=0x%08X",          \
                __func__,                                      \
                "EM_DEBUG_REJOIN_NETWORK",                     \
                "haveKey", haveKey,                            \
                "channelMask", channelMask,                    \
                "status", status)

#define API_TRACE_STACK_POWER_DOWN( ) \
  otLogInfoPlat("[%s %s]",            \
                __func__,             \
                "EM_DEBUG_STACK_POWER_DOWN")

#define API_TRACE_STACK_POWER_UP( ) \
  otLogInfoPlat("[%s %s]",          \
                __func__,           \
                "EM_DEBUG_STACK_POWER_UP")

#define API_TRACE_SET_EXTENDED_SECURITY_BITMASK(mask)     \
  otLogInfoPlat("[%s %s] %s=0x%04X",                      \
                __func__,                                 \
                "EM_DEBUG_SET_EXTENDED_SECURITY_BITMASK", \
                "mask", mask)

#undef assert
#define assert(condition)                                                          \
  do { if (!(condition)) {                                                         \
         fprintf(stderr, "Assert in function '%s' in file %s at line #%d : %s \n", \
                 __func__,                                                         \
                 __FILE__,                                                         \
                 __LINE__,                                                         \
                 #condition);                                                      \
         abort(); } } while (0)

#endif // SILABS_ZIGBEE_DEBUG_STACK_ON_HOST_H
