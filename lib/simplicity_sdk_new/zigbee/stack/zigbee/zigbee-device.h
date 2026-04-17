/***************************************************************************//**
 * @file
 * @brief implementation of the 'ZigBee device', which is the
 * administrative device sitting on endpoint zero.  The code here
 * is based on 'ZigBee Device Profile' revision 4, draft version 0.90
 * and 'ZigBee Device Objects', revision 7, draft version 0.90.
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

#ifndef SILABS_ZIGBEE_DEVICE_H
#define SILABS_ZIGBEE_DEVICE_H

/**
 * @brief sets the currently supported stack revision
 */
void sli_zigbee_set_stack_compliance_revision(uint8_t revision);

// ZigBee device messages have a transaction ID as the first byte.

#define ZDO_MESSAGE_OVERHEAD 1

//----------------------------------------------------------------
// ZigBee device profile cluster IDs

// The current joint winners are BIND_REQUEST and UNBIND_REQUEST.  Some of
// the others have variable-length fields that can be longer.
#define MAX_ZDO_FIXED_REQUEST_LENGTH 21

// We need to handle the fixed-length part of NETWORK_ADDRESS_RESPONSE and
// IEEE_ADDRESS_RESPONSE
#define MAX_ZDO_FIXED_RESPONSE_LENGTH 11

// The enumeration is located in
// include/zigbee-device-stack.h

//----------------------------------------------------------------
// Node Descriptor:
//
// 0:     logical type (0..2), complex descriptor available (3),
//        user descriptor available (4), reserved (5..7)
// 1:     APS flags (0..2), frequency band (3..7)
// 2:     MAC capability flags
// 3-4:   manufacturer code
// 5:     maximum buffer size
// 6-7:   maximum incoming transfer size
// 8-9:   server mask
// 10-11: maximum outgoing transfer size
// 12:    descriptor capability field

enum {
  ZIGBEE_DESCRIPTOR_COORDINATOR = 0x00,
  ZIGBEE_DESCRIPTOR_ROUTER      = 0x01,
  ZIGBEE_DESCRIPTOR_END_DEVICE  = 0x02
                                  // 0x03 to 0x07 are reserved
};

enum {
  // bits 0..2 are the APS
  ZIGBEE_DESCRIPTOR_BAND_868_MHZ          = 0x08,
  // bit 4 is reserved
  ZIGBEE_DESCRIPTOR_BAND_915_MHZ          = 0x20,
  ZIGBEE_DESCRIPTOR_BAND_2400_MHZ         = 0x40,
  ZIGBEE_DESCRIPTOR_BAND_EUROPEAN_SUBGHZ  = 0x80
};

#define SERVER_MASK_STACK_COMPLIANCE_REVISION_MASK 0xFE00
//Bits 9 to 15 are set to stack compliance revision. Everything else is 0.
#define SERVER_MASK_STACK_COMPLIANCE_REVISION_BIT_SHIFT 9
#define SERVER_MASK_STACK_COMPLIANCE_REVISION_21 0x2A00
#define SERVER_MASK_STACK_COMPLIANCE_REVISION_22 0x2C00
// NOTE 0x2e00
#define SERVER_MASK_STACK_COMPLIANCE_REVISION_23 23 << SERVER_MASK_STACK_COMPLIANCE_REVISION_BIT_SHIFT

//----------------------------------------------------------------
// Power Descriptor:
//
// 0: current power mode (0..3), available power sources (4..7)
// 1: current power source (0..3), current power source level (4..7)

enum {
  ZIGBEE_DESCRIPTOR_POWER_MODE_SEE_NODE_DESCRIPTOR = 0x00,
  ZIGBEE_DESCRIPTOR_POWER_MODE_PERIODIC            = 0x01,
  ZIGBEE_DESCRIPTOR_POWER_MODE_TRIGGERED           = 0x02,
};

enum {
  ZIGBEE_DESCRIPTOR_POWER_SOURCE_MAINS        = 0x01,
  ZIGBEE_DESCRIPTOR_POWER_SOURCE_RECHARGEABLE = 0x02,
  ZIGBEE_DESCRIPTOR_POWER_SOURCE_DISPOSABLE   = 0x04,
  // bit 3 is reserved
};

enum {
  ZIGBEE_DESCRIPTOR_POWER_LEVEL_CRITICAL = 0x00,
  ZIGBEE_DESCRIPTOR_POWER_LEVEL_33       = 0x04,
  ZIGBEE_DESCRIPTOR_POWER_LEVEL_66       = 0x08,
  ZIGBEE_DESCRIPTOR_POWER_LEVEL_100      = 0x0C
};

//----------------------------------------------------------------
// Simple Descriptor:

// ZigbeeSimpleDescriptor and ZigbeeEndpoint typedefs moved to
// core/sl_zigbee_stack.h.

enum {
  ZIGBEE_DESCRIPTOR_COMPLEX = 0x01,
  ZIGBEE_DESCRIPTOR_USER    = 0x02
};

//----------------------------------------------------------------
// Frequency Agility Messages

#define REQUEST_CHANNEL_CHANGE 0xFE
#define CHANGE_CHANNEL_MASK    0xFF

#define CHANNEL_MASK_OFFSET    0
#define SCAN_DURATION_OFFSET   4
#define SCAN_COUNT_OFFSET      5  // present when scan duration is 0-5
#define NWK_UPDATEID_OFFSET    5  // present when scan duration is 0xFE or 0xFF
#define NWK_MANAGER_OFFSET     6
#define MAX_NWK_UPDATE_REQUEST_LENGTH 8

// ---------------------------------------------------------------
// APS Default Values
#define APS_PARENT_ANNOUNCE_BASE_TIMER 10
#define APS_PARENT_ANNOUNCE_JITTER_MAX 11

//----------------------------------------------------------------
// To reduce the size of C4's bootloading mini-app, we put the bulk
// of the ZDO request handlers in a library.  The library goes into
// the upper flash and is not called during bootloading.

void sli_zigbee_auxiliary_zdo_request_handler(sl_802154_short_addr_t source,
                                              sl_zigbee_aps_frame_t *requestApsFrame,
                                              sl_zigbee_aps_frame_t *responseApsFrame,
                                              sli_zigbee_packet_header_t header,
                                              bool broadcastRequest,
                                              sli_buffer_manager_buffer_t payloadBuffer,
                                              uint8_t payloadIndex,
                                              uint8_t *contents,
                                              uint8_t length,
                                              sl_zigbee_aps_option_t options,
                                              uint8_t responseSequence);

uint8_t sli_zigbee_find_zdo_payload(sli_zigbee_packet_header_t header,
                                    sli_buffer_manager_buffer_t *payloadBuffer);

//----------------------------------------------------------------
// Helpers

uint16_t getServerMask(uint16_t requestedMask);

void sli_zigbee_zig_dev_proxy_broadcast_message(sl_802154_short_addr_t sourceNodeId);

//----------------------------------------------------------------

#define sli_zigbee_make_zig_dev_message(format, ...) \
  sli_legacy_packet_buffer_make_message(ZDO_MESSAGE_OVERHEAD, format, __VA_ARGS__)

// To send a ZDO message you first supply and APS frame and then call one
// of the actual message sending functions.  The ZDO utility code keeps
// a pointer to the supplied APS frame for use in sending the message.
//
// *** Important Note ***
// The call to the send function must be nested within the block that
// declares the APS frame, because once you leave that block the pointer
// saved by sli_zigbee_zig_dev_prepare_zdo_message() will no longer be valid.
// *** Important Note ***

void sli_zigbee_zig_dev_prepare_zdo_message(sl_zigbee_aps_frame_t *apsFrame,
                                            uint16_t clusterId,
                                            sl_zigbee_aps_option_t options,
                                            uint8_t sequence);

#define sli_zigbee_zig_dev_prepare_zdo_request(frame, clusterId, options) \
  (sli_zigbee_zig_dev_prepare_zdo_message((frame), (clusterId), (options), 0))

#define sli_zigbee_zig_dev_prepare_zdo_response(frame, clusterId, options, sequence) \
  (sli_zigbee_zig_dev_prepare_zdo_message((frame), (clusterId), (options), (sequence)))

sl_status_t sli_zigbee_send_zig_dev_message(sl_802154_short_addr_t destination, sl_zigbee_aps_frame_t *apsFrame, const char * format, ...);

#define sli_zigbee_send_unicast_zig_dev_message(destination, apsFrame, message) \
  (sli_zigbee_send_zig_dev_message((destination), apsFrame, NULL, (message)))

// Return true if a sleepy request was handled.
enum {
  AM_ZDO_TARGET,                // The request is for the local device.
  CHECK_IF_ZDO_CHILD_PROXY,     // The request is for a sleepy child, return
                                //  true if the local device can handle it.
  ACT_AS_ZDO_CHILD_PROXY        // The request is for a sleepy child, handle it
                                // if possible.
};

bool sli_zigbee_device_request_handler(sl_802154_short_addr_t source,
                                       sl_zigbee_aps_frame_t *apsFrame,
                                       sli_zigbee_packet_header_t header,
                                       uint8_t actAsChildProxy,
                                       bool broadcastRequest);
void sli_zigbee_device_response_handler(sl_802154_short_addr_t source,
                                        sl_zigbee_aps_frame_t *apsFrame,
                                        sli_zigbee_packet_header_t header,
                                        uint16_t clusterId);

extern uint8_t sli_zigbee_route_table_size;
extern uint8_t sli_zigbee_tree_depth;

// Used when deciding whether to pass the ZDO request on up to the app.
extern uint8_t sli_zigbee_current_zdo_request_status;

extern uint8_t sli_zigbee_app_zdo_configuration_flags;

#define sli_zigbee_app_handles_zdo_endpoint_requests() \
  ((sli_zigbee_app_zdo_configuration_flags & SL_ZIGBEE_APP_HANDLES_ZDO_ENDPOINT_REQUESTS) != 0)

#define sli_zigbee_app_receives_supported_zdo_requests() \
  ((sli_zigbee_app_zdo_configuration_flags & SL_ZIGBEE_APP_RECEIVES_SUPPORTED_ZDO_REQUESTS) != 0)

#define sli_zigbee_app_handles_unsupported_zdo_requests() \
  ((sli_zigbee_app_zdo_configuration_flags & SL_ZIGBEE_APP_HANDLES_UNSUPPORTED_ZDO_REQUESTS) != 0)

#define sli_zigbee_app_handles_zdo_binding_requests() \
  ((sli_zigbee_app_zdo_configuration_flags & SL_ZIGBEE_APP_HANDLES_ZDO_BINDING_REQUESTS) != 0)

enum {
  ZDO_REQUEST_INVALID,
  ZDO_REQUEST_UNSUPPORTED,
  ZDO_REQUEST_SUPPORTED,
};

extern sli_zigbee_event_t sli_zigbee_send_parent_announce_events[];

// Handles both the BIND_REQUEST and the UNBIND_REQUEST.  Like the
// end-device-bind the binding-table code is in a separate file
// because it is not always needed. Returns true if the request is
// supported.
bool sli_zigbee_handle_bind_unbind_requests(sl_802154_short_addr_t source,
                                            sl_zigbee_aps_frame_t *responseApsFrame,
                                            uint8_t *contents,
                                            uint16_t clusterId);

// Handles the BINDING_TABLE_REQUEST. Returns true if the request is
// supported. See comments above regarding binding-table support.
bool sli_zigbee_handle_binding_table_request(sl_zigbee_aps_option_t options,
                                             sl_zigbee_aps_frame_t *responseApsFrame,
                                             sl_802154_short_addr_t source,
                                             uint8_t *contents);

// ZDO channel management. (a.k.a. Frequency Agility)
#define CHANNEL_CHANGE_TIMER 36  // quarter seconds.  9 seconds, as per the
// PRO stack profile (this is the value of
// nwkNetworkBroadcastDeliveryTime).

extern uint8_t pendingNwkUpdateChannel;

void sli_zigbee_handle_nwk_enhanced_update_request(bool isEnhance,
                                                   sl_802154_short_addr_t source,
                                                   sl_zigbee_aps_frame_t *apsFrame,
                                                   sli_zigbee_packet_header_t header,
                                                   bool broadcastRequest,
                                                   uint8_t *contents);

// Called by sl_zigbee_tick() to see if we need to file an interference report.
void sli_zigbee_check_channel_status(void);

void sli_zigbee_handle_ieee_joining_list_request(sl_802154_short_addr_t source,
                                                 sl_zigbee_aps_frame_t *apsFrame,
                                                 sli_zigbee_packet_header_t header,
                                                 bool broadcastRequest,
                                                 uint8_t *contents);

void sli_zigbee_process_ieee_list_response(uint8_t *contents,
                                           sli_buffer_manager_buffer_t header,
                                           uint8_t payloadIndex);

//----------------------------------------------------------------
// Generating requests

void sli_zigbee_send_our_end_device_announcement(void);
void sli_zigbee_send_parent_announcement(void);
void sli_zigbee_spoof_device_announcement(uint16_t shortId,
                                          uint8_t *sourceEUI64,
                                          sl_802154_long_addr_t deviceAnnounceEui,
                                          uint8_t capabilities);

// In order to avoid confusing the application with unexpected calls to
// sli_zigbee_stack_message_sent_handler(), calls from the stack must use SL_ZIGBEE_ZDO_ENDPOINT
// as the source endpoint.

sl_status_t sli_zigbee_node_descriptor_request(sl_802154_short_addr_t dest,
                                               sl_zigbee_aps_option_t options,
                                               sl_802154_short_addr_t target);

sl_status_t sli_zigbee_network_address_request(sl_802154_long_addr_t target,
                                               bool reportKids,
                                               uint8_t childStartIndex,
                                               uint8_t sourceEndpoint);

sl_status_t sli_zigbee_ieee_address_request(sl_802154_short_addr_t target,
                                            bool reportKids,
                                            uint8_t childStartIndex,
                                            uint8_t sourceEndpoint,
                                            sl_zigbee_aps_option_t options);

sl_status_t sli_zigbee_ieee_address_request_to_target(sl_802154_short_addr_t discoveryNodeId,
                                                      bool reportKids,
                                                      uint8_t childStartIndex,
                                                      uint8_t sourceEndpoint,
                                                      sl_zigbee_aps_option_t options,
                                                      sl_802154_short_addr_t targetNodeIdOfRequest);

uint8_t sli_zigbee_next_stack_zdo_sequence_number(void);
#endif // SILABS_ZIGBEE_DEVICE_H
