/***************************************************************************//**
 * @file
 * @brief ZigBee-specific header definitions and function declarations.
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

#ifndef SILABS_ZIGBEE_PACKET_HEADER_H
#define SILABS_ZIGBEE_PACKET_HEADER_H

// From table 3.51 of 053474r14
#define SL_ZIGBEE_MIN_BROADCAST_ADDRESS 0xFFF8u

//-------------------------------------------------------------
// Maximum header size.
//
// The normal maximum-length ZigBee header has:
//   4   Overhead & MAC (payload buffer, MAC info byte, source/dest address)
//   9   Multicast NWK frame
//  14   Network security auxilliary frame
//   8   Broadcast APS frame
// -> 34 bytes, which does not fit in one buffer
//
// The true maximum-length Zigbee header has:
//   4   Overhead & MAC (payload buffer, MAC info byte, source/dest address)
//   8   Unicast NWK Frame (we can only do APS Security with unicast)
//   ?   Source route
//  14   Network Level Security Auxiliary Header Frame
//   6   Unicast APS Frame
//  14   APS Level Security Auxiliary Header Frame
// -> 46 bytes, which does NOT fit in one buffer.  However, APS security is
//      rarely used.
//
//-------------------------------------------------------------
// Zigbee network frame format:
//
//  0-1 frame control field
//  2-3 destination address
//  4-5 source address
//  6   radius
//  7   sequence number
//  8   multicast control (multicasts only)

#define ZIGBEE_NETWORK_PROTOCOL_VERSION 0x02u

// Zigbee frame control field

#define ZIGBEE_FRAME_CONTROL_FRAME_TYPE_DATA           0u
#define ZIGBEE_FRAME_CONTROL_FRAME_TYPE_COMMAND        ((uint16_t)(BIT(0)))     // 0x0001
#define ZIGBEE_FRAME_CONTROL_FRAME_TYPE_MASK           ((uint16_t)(BIT(0) | BIT(1)))    //0x0003
#define ZIGBEE_FRAME_CONTROL_PROTOCOL_VERSION          ((uint16_t)(ZIGBEE_NETWORK_PROTOCOL_VERSION << 2))
#define ZIGBEE_FRAME_CONTROL_PROTOCOL_VERSION_MASK \
  ((uint16_t)(BIT(2) | BIT(3) | BIT(4) | BIT(5)))   // 0x003C
#define ZIGBEE_FRAME_CONTROL_PROTOCOL_VERSION_GREENPOWER ((uint16_t)(BIT(2) | BIT(3)))  // 0x000C
#define ZIGBEE_FRAME_CONTROL_SUPPRESS_ROUTE_DISCOVERY  0u
#define ZIGBEE_FRAME_CONTROL_ENABLE_ROUTE_DISCOVERY    ((uint16_t)(BIT(6)))   // 0x0040
#define ZIGBEE_FRAME_CONTROL_FORCE_ROUTE_DISCOVERY     ((uint16_t)(BIT(7)))   // 0x0080
#define ZIGBEE_FRAME_CONTROL_DISCOVER_ROUTE_MASK       ((uint16_t)(BIT(6) | BIT(7)))    // 0x00C0
#define ZIGBEE_FRAME_CONTROL_MULTICAST                 ((uint16_t)(BIT(8)))   // 0x0100
#define ZIGBEE_FRAME_CONTROL_SECURITY                  ((uint16_t)(BIT(9)))   // 0x0200
#define ZIGBEE_FRAME_CONTROL_SOURCE_ROUTE              ((uint16_t)(BIT(10)))  // 0x0400
#define ZIGBEE_FRAME_CONTROL_DESTINATION_IEEE_ADDRESS  ((uint16_t)(BIT(11)))  // 0x0800
#define ZIGBEE_FRAME_CONTROL_SOURCE_IEEE_ADDRESS       ((uint16_t)(BIT(12)))  // 0x1000
#define ZIGBEE_FRAME_CONTROL_END_DEVICE_INITIATOR      ((uint16_t)(BIT(13)))  // 0x2000
// Bits 14-15 are reserved

#define ZIGBEE_DATA_FRAME_MAC_HEADER_MAXIMUM_SIZE 9

#define ZIGBEE_NETWORK_FRAME_MINIMUM_SIZE 8
#define ZIGBEE_NETWORK_FRAME_MAXIMUM_SIZE 9

//used for handling calls to a NWK or APS header index or size on malformed packets
#define ZIGBEE_PACKET_HEADER_INVALID_INDEX 0
#define ZIGBEE_PACKET_HEADER_INVALID_SIZE 0

#define sli_zigbee_network_frame_is_multicast(frame) (((frame)[1]) & 0x01u)
#define sli_zigbee_network_frame_has_security(frame) (((frame)[1]) & 0x02u)
#define sli_zigbee_network_frame_has_source_route(frame) (((frame)[1]) & 0x04u)
#define sli_zigbee_network_frame_has_destination_ieee_address(frame) (((frame)[1]) & 0x08u)
#define sli_zigbee_network_frame_has_source_ieee_address(frame) (((frame)[1]) & 0x10u)
#define sli_zigbee_network_frame_message_from_end_device(frame) (((frame)[1]) & 0x20u)
#define sli_zigbee_clear_message_from_end_device(frame) (((frame)[1]) &= ~0x20u)
// Routes may be either unicast or multicast, depending on whether the
// destination is a node or a multicast group (in a unicast-mode multicast
// message).
//
// We do not currently support multicast routes.

#define ZIGBEE_DESTINATION_ADDRESS_INDEX 2
#define ZIGBEE_SOURCE_ADDRESS_INDEX      4
#define ZIGBEE_RADIUS_INDEX              6
#define ZIGBEE_SEQUENCE_INDEX            7
#define ZIGBEE_LONG_SOURCE_ADDRESS_INDEX 8
#define ZIGBEE_MULTICAST_CONTROL_INDEX   8

#define sli_zigbee_frame_control(frame) \
  sl_util_fetch_low_high_int16u((frame))
#define sli_zigbee_set_zigbee_frame_control(frame, control) \
  sl_util_store_low_high_int16u((frame), (control))
#define sli_zigbee_set_route_discovery_suppressed(frame) \
  ((frame)[0] &= ~ZIGBEE_FRAME_CONTROL_DISCOVER_ROUTE_MASK)
#define sli_zigbee_set_force_route_discovery_suppressed(frame) \
  ((frame)[0] &= ~ZIGBEE_FRAME_CONTROL_FORCE_ROUTE_DISCOVERY)
#define sli_zigbee_set_source_route_flag(frame) ((frame)[1] |= 0x04u)
#define sli_zigbee_clear_source_route_flag(frame) ((frame)[1] &= ~0x04u)

#define sli_zigbee_destination(frame) \
  sl_util_fetch_low_high_int16u((frame) + ZIGBEE_DESTINATION_ADDRESS_INDEX)
#define sli_zigbee_source(frame) \
  sl_util_fetch_low_high_int16u((frame) + ZIGBEE_SOURCE_ADDRESS_INDEX)

#define sli_zigbee_set_zigbee_destination(frame, id) \
  sl_util_store_low_high_int16u((frame) + ZIGBEE_DESTINATION_ADDRESS_INDEX, (id))
#define sli_zigbee_set_zigbee_source(frame, id) \
  sl_util_store_low_high_int16u((frame) + ZIGBEE_SOURCE_ADDRESS_INDEX, (id))
#define sli_zigbee_set_zigbee_long_source(frame, id) \
  memcpy((frame) + ZIGBEE_LONG_SOURCE_ADDRESS_INDEX, (id), EUI64_SIZE)

// Only checks if the destination is the broadcast address.
// Doesn't check if it's a command or data frame.
#define sli_zigbee_is_zigbee_broadcast(frame) \
  (sl_zigbee_is_zigbee_broadcast_address(sli_zigbee_destination((frame))))

#define sli_zigbee_radius(frame) \
  ((frame)[ZIGBEE_RADIUS_INDEX])

#define sli_zigbee_sequence(frame) \
  ((frame)[ZIGBEE_SEQUENCE_INDEX])

#define sli_zigbee_multicast_control(frame) \
  ((frame)[ZIGBEE_MULTICAST_CONTROL_INDEX])

uint8_t *sli_zigbee_destinationIeeeAddress(uint8_t *networkFrame);

///////////////////////////////////////////////////////////////////////////////////
// NOTE:  Getting a pointer to the contents in the network frame only works
// if the examined contents are within a single packet buffer.
// The source IEEE address could in theory have contents beyond the packet buffer
// boundary, and so are rendered inaccessible via direct pointer access.

// sli_zigbee_get_zigbee_source_ieee_address and sli_zigbee_get_zigbee_source_ieee_address are modified to
// take a sli_zigbee_packet_header_t argument so they can call linked buffer APIs such as
// sl_legacy_buffer_manager_copy_from_linked_buffers and sl_legacy_buffer_manager_copy_to_linked_buffers to retrieve the correct
// frame contents.
///////////////////////////////////////////////////////////////////////////////////

bool sli_zigbee_get_zigbee_source_ieee_address(sli_zigbee_packet_header_t header, sl_802154_long_addr_t *target);
bool sli_zigbee_set_zigbee_source_ieee_address(sli_zigbee_packet_header_t header, sl_802154_long_addr_t *source);

uint8_t sli_zigbee_source_route_subframe_index(sli_zigbee_packet_header_t header);

#define sli_zigbee_next_zigbee_sequence_number() (++sli_zigbee_sequence_number)

#define ZIGBEE_DEFAULT_BROADCAST_RADIUS 10u

// Zigbee commands
// All command frames begin with the one byte command frame identifier.

enum {
  ZIGBEE_ROUTE_REQUEST   = 0x01, // <command options (1)>
                                 // <route request id (1)>
                                 // <target address (2)>
                                 // <path cost (1)>
  ZIGBEE_ROUTE_REPLY     = 0x02, // <command options (1)>
                                 // <route request id (1)>
                                 // <originator address (2)>
                                 // <target address (2)>
                                 // <path cost (1)>
  ZIGBEE_ROUTE_ERROR     = 0x03, // <error code (1)>
                                 // <destination address (2)>
  ZIGBEE_LEAVE_COMMAND   = 0x04, // <command options (1)>
  ZIGBEE_ROUTE_RECORD    = 0x05, // <relay count (1)>
                                 // <relay list (2x)>
  ZIGBEE_REJOIN_REQUEST  = 0x06, // <capability (1)>
  ZIGBEE_REJOIN_RESPONSE = 0x07, // <short id (2)>
                                 // <status (1)>
  ZIGBEE_LINK_STATUS     = 0x08, // <options (1)>
                                 // [<neighbor ID (2)> <status (1)>]*
  ZIGBEE_NETWORK_REPORT  = 0x09, // <options (1)> <EPID (8)> [<pan id (2)]*
  ZIGBEE_NETWORK_UPDATE  = 0x0A,  // <options (1)> <EPID (8)> <update id (1)>
                                  // <new pan id (2)>
  ZIGBEE_NETWORK_TIMEOUT_REQUEST = 0x0B,
  ZIGBEE_NETWORK_TIMEOUT_RESPONSE = 0x0C,
  ZIGBEE_NETWORK_LINK_POWER_DELTA = 0x0D,
  ZIGBEE_NETWORK_COMMISSIONING_REQUEST = 0x0E,
  ZIGBEE_NETWORK_COMMISSIONING_RESPONSE = 0x0F,
  ZIGBEE_NETWORK_INVALID_NWK_COMMAND_START,
  ZIGBEE_R23_NWK_COMMAND_START = ZIGBEE_NETWORK_COMMISSIONING_REQUEST,
};

enum {
  ZIGBEE_TIMEOUT_REQUEST_SUCCESS = 0x00,
  ZIGBEE_TIMEOUT_REQUEST_INVALID_VALUE = 0x01,
  ZIGBEE_TIMEOUT_REQUEST_FAILURE = 0x02
};

// Option for route commands.
#define ZIGBEE_COMMAND_OPTION_MANY_TO_ONE_MASK      0x18u // requests only
#define ZIGBEE_COMMAND_OPTION_HAVE_ROUTE_TABLE      0x08u // requests only
#define ZIGBEE_COMMAND_OPTION_NO_ROUTE_TABLE        0x10u // requests only
#define ZIGBEE_COMMAND_OPTION_HAVE_ORIGINATOR_EUI64 0x10u // replies only
#define ZIGBEE_COMMAND_OPTION_HAVE_TARGET_EUI64     0x20u // requests and replies
#define ZIGBEE_COMMAND_OPTION_MULTICAST             0x40u // requests and replies

// Options for leave command.
#define ZIGBEE_COMMAND_OPTION_REJOIN            0x20u
#define ZIGBEE_COMMAND_OPTION_LEAVE_IS_REQUEST  0x40u
// CCB 2047
// - CCB makes the first step to depracate the 'leave and remove children' functionality.
// - We were proactive here and deprecated it right away.
// #define ZIGBEE_COMMAND_OPTION_REMOVE_CHILDREN   0x80

// Options for link status command.
#define ZIGBEE_COMMAND_OPTION_LINK_COUNT_MASK   0x1Fu
#define ZIGBEE_COMMAND_OPTION_FIRST_FRAME       0x20u
#define ZIGBEE_COMMAND_OPTION_LAST_FRAME        0x40u

// Options for links within the link status command.
#define ZIGBEE_COMMAND_OPTION_INCOMING_MASK     0x07u
// Reserved                                     0x08
#define ZIGBEE_COMMAND_OPTION_OUTGOING_MASK     0x70u
// Reserved                                     0x80

// Options for report and update commands.
#define ZIGBEE_COMMAND_OPTION_COUNT_MASK 0x1Fu
#define ZIGBEE_COMMAND_OPTION_TYPE_MASK 0xE0u
// For both report and update commands, the only valid type is 0.
#define ZIGBEE_COMMAND_OPTION_TYPE_VALUE 0x00u

// These indexes are from the start of the command frame, not the nwk frame.
// The first two fields are common to route requests and replies
#define ZIGBEE_ROUTE_COMMAND_OPTION_INDEX    1
#define ZIGBEE_ROUTE_COMMAND_ID_INDEX        2

#define ZIGBEE_ROUTE_REQUEST_BASE_FRAME_SIZE 6
#define ZIGBEE_ROUTE_REQUEST_MAX_FRAME_SIZE  14
// This field is called the 'destination' in the ZigBee spec, but there
// is already a destination field in the header.  'Target' is less ambiguous.
#define ZIGBEE_ROUTE_REQUEST_TARGET_INDEX    3
#define ZIGBEE_ROUTE_REQUEST_PATH_COST_INDEX 5
#define ZIGBEE_ROUTE_REQUEST_TARGET_EUI64_INDEX 6

#define ZIGBEE_ROUTE_REPLY_BASE_FRAME_SIZE   8
#define ZIGBEE_ROUTE_REPLY_MAX_FRAME_SIZE   24
#define ZIGBEE_ROUTE_REPLY_ORIGINATOR_INDEX  3
#define ZIGBEE_ROUTE_REPLY_TARGET_INDEX      5
#define ZIGBEE_ROUTE_REPLY_PATH_COST_INDEX   7
#define ZIGBEE_ROUTE_REPLY_ORIGINATOR_EUI64_INDEX 8
#define ZIGBEE_ROUTE_REPLY_TARGET_EUI64_INDEX  16

#define ZIGBEE_ROUTE_ERROR_FRAME_SIZE        4
#define ZIGBEE_ROUTE_ERROR_CODE_INDEX        1
#define ZIGBEE_ROUTE_ERROR_DESTINATION_INDEX 2

#define ZIGBEE_NETWORK_STATUS_UNKNOWN_COMMAND_FRAME_SIZE  5
#define ZIGBEE_NETWORK_STATUS_UNKNOWN_COMMAND_ID_INDEX    4

#define ZIGBEE_LEAVE_COMMAND_FRAME_SIZE      2
#define ZIGBEE_NETWORK_TIMEOUT_REQUEST_FRAME_SIZE 3
#define ZIGBEE_NETWORK_TIMEOUT_RESPONSE_FRAME_SIZE 3
#define ZIGBEE_ROUTE_RECORD_RELAY_COUNT_INDEX 1
#define ZIGBEE_ROUTE_RECORD_BASE_FRAME_SIZE   2

#define ZIGBEE_JOIN_REQUEST_FRAME_SIZE       2

#define ZIGBEE_JOIN_RESPONSE_FRAME_SIZE      4
#define ZIGBEE_JOIN_RESPONSE_STATUS_INDEX    3

#define ZIGBEE_LINK_STATUS_BASE_FRAME_SIZE   2

#define ZIGBEE_NETWORK_REPORT_BASE_FRAME_SIZE   10
#define ZIGBEE_NETWORK_UPDATE_BASE_FRAME_SIZE   11

#define ZIGBEE_NETWORK_LINK_POWER_DELTA_BASE_FRAME_SIZE   2

#define ZIGBEE_NETWORK_COMMISSIONING_REQUEST_BASE_FRAME_SIZE  3
#define ZIGBEE_NETWORK_COMMISSIONING_RESPONSE_BASE_FRAME_SIZE 4

#define sli_zigbee_frame_type(frame) \
  ((frame)[0] & ZIGBEE_FRAME_CONTROL_FRAME_TYPE_MASK)
#define sli_zigbee_is_zigbee_data(frame) \
  (sli_zigbee_frame_type((frame)) == ZIGBEE_FRAME_CONTROL_FRAME_TYPE_DATA)
#define sli_zigbee_is_zigbee_command(frame) \
  (sli_zigbee_frame_type((frame)) == ZIGBEE_FRAME_CONTROL_FRAME_TYPE_COMMAND)

#define ZIGBEE_MULTICAST_MODE_MASK         0x03u
#define ZIGBEE_MULTICAST_FUZZ_MASK         0x1Cu
#define ZIGBEE_MULTICAST_MAXIMUM_FUZZ_MASK 0xE0u

#define ZIGBEE_MULTICAST_UNICAST_MODE   0x00u
#define ZIGBEE_MULTICAST_BROADCAST_MODE 0x01u

// 'Fuzz' was the original name of what is now called 'broadcastAddr'.
#define ZIGBEE_MULTICAST_FUZZ_OFFSET            2u
#define ZIGBEE_MULTICAST_MAXIMUM_FUZZ_OFFSET    5u
#define ZIGBEE_MULTICAST_INFINITE_FUZZ  0x07u

// We only do broadcast mode.
#define sli_zigbee_multicast_control_func(broadcastAddr, maxbroadcastAddr) \
  (ZIGBEE_MULTICAST_BROADCAST_MODE                                         \
   | (uint8_t) (((uint8_t) broadcastAddr) << ZIGBEE_MULTICAST_FUZZ_OFFSET) \
   | (uint8_t) (((uint8_t) maxbroadcastAddr)                               \
                << ZIGBEE_MULTICAST_MAXIMUM_FUZZ_OFFSET))
//----------------
// Returns the index of whatever is after the network header.  For commands
// this is the command itself and for data messages it is an APS frame.
uint8_t sli_zigbee_header_network_payload_index(sli_zigbee_packet_header_t header);

// Move any frames after the network frame from the start of the
// payload to the end of the header.  With an encrypted packet we can't
// tell where the header/application-payload division occurs until after
// decryption.
void sli_zigbee_reconstruct_decrypted_header(sli_zigbee_packet_header_t header);

// It returns true if the buffer contains at least as many bytes as indicated in
// the minSize variable, otherwise it returns false.
bool sli_zigbee_is_buffer_size_ok(sli_zigbee_packet_header_t header, uint16_t minSize);

// It Returns the size of the APS header, based on the information in the frame
// control.
uint8_t sli_zigbee_aps_header_size(uint8_t frameControl);
// It Returns the size of the extended APS header based on the information in
// the frame control and the extended frame control.
uint8_t sli_zigbee_aps_extended_header_size(uint8_t frameControl, uint8_t extFrameControl);

//----------------------------------------------------------------
// Application Support Sub-Layer (APS) frames (called the APDU for some
// reason).

#define ZIGBEE_APS_FRAME_CONTROL_TYPE_MASK        0x03u
#define ZIGBEE_APS_FRAME_CONTROL_TYPE_DATA        0x00u
#define ZIGBEE_APS_FRAME_CONTROL_TYPE_COMMAND     0x01u
#define ZIGBEE_APS_FRAME_CONTROL_TYPE_ACK         0x02u
#define ZIGBEE_APS_FRAME_CONTROL_TYPE_COMMAND_ACK 0x03u
#define ZIGBEE_APS_FRAME_CONTROL_TYPE_RESERVED    0x04u

#define ZIGBEE_APS_FRAME_CONTROL_MODE_MASK      0x0Cu
#define ZIGBEE_APS_FRAME_CONTROL_MODE_DIRECT    0x00u

// Only ZigBee-2003 uses this mechanism, so we no longer support it.
#define ZIGBEE_APS_FRAME_CONTROL_MODE_INDIRECT  0x04u

#define ZIGBEE_APS_FRAME_CONTROL_MODE_BROADCAST 0x08u
#define ZIGBEE_APS_FRAME_CONTROL_MODE_MULTICAST 0x0Cu

// The HAVE_SOURCE bit applies only to indirect messages.  If it is set
// the message is travelling from the source to the coordinator and has
// a source but no destination.  If it is clear the message is travelling
// from the coordinator to the destination and has a destination but
// no source.
//
// Only ZigBee-2003 uses this mechanism, so we no longer support it.
#define ZIGBEE_APS_FRAME_CONTROL_HAVE_SOURCE     0x10u

#define ZIGBEE_APS_FRAME_CONTROL_ACK_FORMAT      0x10u
#define ZIGBEE_APS_FRAME_CONTROL_HAVE_SECURITY   0x20u
#define ZIGBEE_APS_FRAME_CONTROL_WANT_ACK        0x40u
#define ZIGBEE_APS_FRAME_CONTROL_EXTENDED_HEADER 0x80u

// This field is in the extended header.
#define ZIGBEE_EXTENDED_APS_FRAGMENTATION_MASK       0x03u
#define ZIGBEE_EXTENDED_APS_NOT_FRAGMENTED           0x00u
#define ZIGBEE_EXTENDED_APS_FIRST_FRAGMENT           0x01u
#define ZIGBEE_EXTENDED_APS_LATER_FRAGMENT           0x02u
#define ZIGBEE_EXTENDED_APS_FRAGMENTATION_RESERVED   0x03u

// Extensions on unicasts are two bytes, those on acks are three bytes.
#define ZIGBEE_APS_HEADER_UNICAST_EXTENSION_SIZE 2
#define ZIGBEE_APS_HEADER_ACK_EXTENSION_SIZE 3
#define ZIGBEE_APS_DEVICE_ANNOUNCE_FARME_SIZE 11

// For data and ACK messages the APS frame is laid out as follows:
//
//  FrameControl         (1)
//  DestinationEndpoint  (0/1)   - Not present in multicasts
//  GroupAddress         (0/2)   - Only in multicasts.
//  ClusterIdentifier    (2)
//  ProfileIdentifier    (2)
//  SourceEndpoint       (1)
//  Counter              (1)
//  Extension            (0/2/3) - Only in fragmented messages (2 bytes) and
//                                 their acks (3 bytes).  Only unicasts can be
//                                 fragmented.
//
// The following suffix is present if the EXTENDED_HEADER bit is set
// in the frame control.
//  ExtendedFrameControl (1)
//  BlockCounter         (0/1)   - If the fragment field is non-zero.
//  AckBitfield          (0/1)   - If this is an ACK and the fragment field is
//                                 non-zero.

// The only APS commands are those used by the APS security subsystem.
// See stack/zigbee/aps-security.c for a description of the commands.
//
// It isn't clear how an ACK for an indirect message would work or would
// be encoded.

#define sli_zigbee_header_network_payload_first_byte(header) \
  (sl_legacy_buffer_manager_get_linked_buffers_byte(header, sli_zigbee_header_network_payload_index(header)))

#define sli_zigbee_header_aps_frame_control(header) sli_zigbee_header_network_payload_first_byte(header)

#define APS_COMMAND_FRAME_SIZE          2   // control(1) + counter(1)
#define APS_DATA_ACK_BASE_FRAME_SIZE    8
#define MAX_APS_FRAME_SIZE              12

// The first of these ignores any following security subframe.  The second
// includes the size of the security subframe, if there is any.

uint8_t sli_zigbee_aps_base_frame_size(sli_zigbee_packet_header_t header, uint8_t apsFrameIndex);
uint8_t sli_zigbee_aps_frame_size(sli_zigbee_packet_header_t header, uint8_t apsFrameIndex);

uint8_t sli_zigbee_header_aps_payload_index(sli_zigbee_packet_header_t header);

// Converting between over-the-air and in-memory formats.
// This function does not perform any sanity check (anymore). All the sanity
// checks are performed in sli_zigbee_aps_packet_is_valid().
void sli_zigbee_aps_frame_to_aps_struct(sli_zigbee_packet_header_t header,
                                        sl_zigbee_aps_frame_t *apsStruct);

// We can get the apsFrame from the application, and we get the mode from
// the call used (emberSend[Uni|Multi|Broad]cast), so the mode has to be
// passed separately.
bool sli_zigbee_add_aps_frame(sli_zigbee_packet_header_t *header,
                              uint8_t mode,
                              sl_zigbee_aps_frame_t *apsStruct);

sl_status_t sli_zigbee_get_application_payload(sli_zigbee_packet_header_t header,
                                               uint8_t startIndex,
                                               sli_buffer_manager_buffer_t *payloadReturn);

#ifdef SL_ZIGBEE_TEST

// Set this to false to disable incrementing sequence numbers.
extern bool useApsSequenceNumbers;

uint8_t nextApsSequenceNumber(void);

#else

#define nextApsSequenceNumber() sli_zigbee_aps_sequence_number++

#endif

//----------------------------------------------------------------
// Auxiliary security frames
//
// These are suffixes for various other frames, depending on the layer which
// is applying encryption.  For network security they go on the end of the
// network frame.
//
// The frames are from five to fourteen bytes in length:
//  Security Control    (1)
//  Frame Counter       (4)
//  Source Address      (0/8)  // present in network-layer frames; I don't
//                             // know when they are absent
//  Key Sequence Number (0/1)  // only in network-layer frames
//
// These are used to form a thirteen-byte nonce:
//  [source address (8)] [frame counter (4)] [security control (1)]
//
// The Key Sequence Number indicates which of the various available
// network keys was used.

#define ZIGBEE_SECURITY_SUBFRAME_MIN_SIZE      5
#define ZIGBEE_APS_SECURITY_SUBFRAME_MAX_SIZE 13
#define ZIGBEE_NETWORK_SECURITY_SUBFRAME_SIZE 14
#define ZIGBEE_MIC_SIZE 4

// Warning: security subframes may straddle buffer boundaries (both for
// network and APS security).  Therefore if you wish to use these
// macros you must first copy the subframe to an array.

#define sli_zigbee_security_frame_control(frame)                 ((frame)[0])
#define sli_zigbee_security_frame_counter(frame)                 ((frame) + 1)
#define sli_zigbee_security_frame_source_address(frame)           ((frame) + 5)
#define sli_zigbee_security_frame_key_sequence(frame)             ((frame)[13])

//----------------
// Security Control bytes

// For the security levels, the high bit is set to enable data encryption.
// The low two bits give the size of the frame-integrity block (MIC).
enum {
  ZIGBEE_SECURITY_NONE        = 0,
  ZIGBEE_SECURITY_MIC_32,
  ZIGBEE_SECURITY_MIC_64,
  ZIGBEE_SECURITY_MIC_128,
  ZIGBEE_SECURITY_ENC,
  ZIGBEE_SECURITY_ENC_MIC_32,
  ZIGBEE_SECURITY_ENC_MIC_64,
  ZIGBEE_SECURITY_ENC_MIC_128
};

// True if the security level requires encryption.
#define sli_zigbee_security_level_does_encryption(level) (ZIGBEE_SECURITY_ENC <= (level))

// Bits 6 and 7 (0xC0) are reserved.

//----------------------------------------------------------------
// Makes a header and constructs the MAC and Zigbee frames.
//
// If ZIGBEE_FRAME_CONTROL_SECURITY is set the packet will be encrypted
// if NWK security is enabled.  If not, the packet will not be encrypted
// regardless.  This is to allow the APS code to send unsecured messages
// in secure networks.

sli_zigbee_packet_header_t sli_zigbee_make_zigbee_header(sli_buffer_manager_buffer_t message,
                                                         uint16_t frameControl,
                                                         sl_802154_short_addr_t destination,
                                                         sl_802154_long_addr_t destinationEui,
                                                         uint8_t radius);

// Source and destination EUIs are automatically included
// in the header for id conflict detection (mesh stack).
sli_zigbee_packet_header_t sli_zigbee_make_zigbee_command_header(sl_802154_short_addr_t destination,
                                                                 uint8_t radius,
                                                                 uint8_t *commandFrame,
                                                                 uint8_t length,
                                                                 bool tryToInsertLongDest,
                                                                 sl_802154_long_addr_t destinationEui);

// We always set the Security Bit for our messages.  If the node
// is not using security then sli_zigbee_make_zigbee_header() will mask it off.
#define sli_zigbee_make_zigbee_unicast_header(message, frameControl,            \
                                              destination, destinationEui)      \
  (sli_zigbee_make_zigbee_header((message),                                     \
                                 (frameControl) | ZIGBEE_FRAME_CONTROL_SECURITY \
                                 | ZIGBEE_FRAME_CONTROL_FRAME_TYPE_DATA         \
                                 | ZIGBEE_FRAME_CONTROL_PROTOCOL_VERSION,       \
                                 (destination),                                 \
                                 (destinationEui),                              \
                                 0))

#define sli_zigbee_make_zigbee_broadcast_header(message, frameControl, dest, radius) \
  (sli_zigbee_make_zigbee_header((message),                                          \
                                 (frameControl) | ZIGBEE_FRAME_CONTROL_SECURITY      \
                                 | ZIGBEE_FRAME_CONTROL_FRAME_TYPE_DATA              \
                                 | ZIGBEE_FRAME_CONTROL_PROTOCOL_VERSION,            \
                                 (dest),                                             \
                                 NULL,                                               \
                                 (radius)))

#define sli_zigbee_make_zigbee_multicast_header(message, frameControl, dest, radius) \
  (sli_zigbee_make_zigbee_header((message),                                          \
                                 (frameControl) | ZIGBEE_FRAME_CONTROL_SECURITY      \
                                 | ZIGBEE_FRAME_CONTROL_MULTICAST                    \
                                 | ZIGBEE_FRAME_CONTROL_FRAME_TYPE_DATA              \
                                 | ZIGBEE_FRAME_CONTROL_PROTOCOL_VERSION,            \
                                 (dest),                                             \
                                 NULL,                                               \
                                 (radius)))

// Set the message tag global to pass it around both upwards and downwards.
void sli_zigbee_set_current_tag(uint16_t tag);

// Retrieve the message tag global to both set it in the retry queue entry
// and to pass it into the message_sent handler.
uint16_t sli_zigbee_get_current_tag(void);

//Zigbee Company ID
//This is assigned by IEEE and used for Zigbee Specific Information Elements
//in 15.4-2012 frames
#define ZIGBEE_IEEE_COMPANY_ID 0x4A191Bu

//sli_parcel_t type is defined in sl_zigbee_stack.h
#if defined(SL_ZIGBEE_TEST) && defined(SL_ZIGBEE_STACK_H)
//----------------------------------------------------------------
// Handy debug functions.

sli_parcel_t *makeZigbeeNetworkFrameParcel(uint16_t frameControl,
                                           sl_802154_short_addr_t source,
                                           sl_802154_short_addr_t destination,
                                           uint8_t radius,
                                           uint8_t seqNum,
                                           sl_802154_long_addr_t sourceLong,
                                           sl_802154_long_addr_t destLong);
sli_parcel_t *makeZigbeeUnicastFrameParcelWithEui64s(uint16_t frameControl,
                                                     sl_802154_short_addr_t source,
                                                     sl_802154_short_addr_t destination,
                                                     uint8_t sequenceNumber,
                                                     sl_802154_long_addr_t sourceLong,
                                                     sl_802154_long_addr_t destLong);
#define makeZigbeeUnicastFrameParcel(frameControl, source,                \
                                     destination, sequenceNumber)         \
  makeZigbeeUnicastFrameParcelWithEui64s((frameControl), (source),        \
                                         (destination), (sequenceNumber), \
                                         NULL, NULL)

sli_parcel_t *makeZigbeeBroadcastFrameParcel(uint16_t frameControl,
                                             sl_802154_short_addr_t source,
                                             sl_802154_short_addr_t destination,
                                             uint8_t radius,
                                             uint8_t seqNum);
sli_parcel_t *makeZigbeeMulticastFrameParcel(uint16_t frameControl,
                                             sl_802154_short_addr_t source,
                                             sl_zigbee_multicast_id_t destination,
                                             uint8_t radius,
                                             uint8_t seqNum,
                                             uint8_t multicastControl);
sli_parcel_t *makeEmptyNetworkSecurityFrameParcel(void);
sli_parcel_t *makeNetworkSecurityFrameParcel(uint8_t securityLevel,
                                             sl_802154_long_addr_t eui64,
                                             uint32_t frameCounter,
                                             uint8_t keySequence);
sli_parcel_t *makeMyRouteRequestParcel(uint16_t dest,
                                       uint16_t macAddress, // incoming or outgoing
                                       uint8_t radius,
                                       uint8_t requestId,
                                       uint8_t pathCost);
sli_parcel_t *makeTimeoutRequestParcel(uint16_t frameControl,
                                       sl_802154_short_addr_t source,
                                       sl_zigbee_multicast_id_t destination,
                                       uint8_t radius,
                                       uint8_t seqNum,
                                       uint8_t multicastControl);
sli_parcel_t *makeRouteRequestParcel(uint16_t source,
                                     uint16_t dest,
                                     uint16_t macAddress, // incoming or outgoing
                                     uint8_t radius,
                                     uint8_t requestId,
                                     uint8_t pathCost,
                                     sl_802154_long_addr_t longSource,
                                     sl_802154_long_addr_t longDest);
sli_parcel_t *makeManyToOneRouteRequestParcel(uint16_t source,
                                              uint8_t flags,
                                              uint16_t macAddress, // incoming or outgoing
                                              uint8_t radius,
                                              uint8_t requestId,
                                              uint8_t pathCost,
                                              sl_802154_long_addr_t sourceLong);
sli_parcel_t *makeRouteReplyParcel(uint16_t source,
                                   sl_802154_long_addr_t sourceLong,
                                   uint16_t dest,
                                   sl_802154_long_addr_t destLong,
                                   uint16_t originator,
                                   sl_802154_long_addr_t originatorLong,
                                   uint16_t responder,
                                   sl_802154_long_addr_t responderLong,
                                   uint16_t macAddress,
                                   uint8_t sequence,
                                   uint8_t requestId,
                                   uint8_t pathCost);
sli_parcel_t *makeRouteErrorParcel(uint16_t macDest,
                                   uint16_t nwkSource,
                                   uint16_t nwkDest,
                                   uint16_t routeErrorDest,
                                   uint8_t  radius,
                                   uint8_t  routeError,
                                   uint8_t  sequence);
sli_parcel_t *makeRouteRecordParcel(uint16_t source,
                                    sl_802154_long_addr_t sourceLong,
                                    uint16_t dest,
                                    sl_802154_long_addr_t destLong,
                                    uint16_t macId,
                                    uint8_t radius,
                                    int relayCount,
                                    ...);

sli_parcel_t *makeSourceRouteParcel(uint16_t source,
                                    uint16_t dest,
                                    uint16_t macId,
                                    uint8_t radius,
                                    uint8_t relayCount,
                                    uint8_t relayIndex,
                                    uint8_t *relayList);

sli_parcel_t *makeLeaveCommandParcel(uint16_t macAddress,
                                     uint16_t source,
                                     uint16_t destination,
                                     uint8_t options,
                                     sl_802154_long_addr_t longSource,
                                     sl_802154_long_addr_t longDestination,
                                     uint8_t sequence);

sli_parcel_t *makeLinkStatusCommandParcel(uint16_t networkFlags,
                                          uint16_t source,
                                          sl_802154_long_addr_t sourceEui64,
                                          uint16_t macId,
                                          uint8_t sequence,
                                          uint8_t payloadFlags,
                                          bool encrypt,
                                          sli_parcel_t *list);

void clearRoutingFrameSequenceNumber(sli_parcel_t *header);

void printPacket(sli_parcel_t *header, sli_parcel_t *payload);
void printRoutingFrame(sli_parcel_t *header);
bool printDispatchFrameFromHeader(sli_parcel_t *header);
void printDispatchFrame(uint8_t *header);

sli_parcel_t *makeApsFrame(uint8_t controlBits,
                           uint8_t sourceEndpoint,
                           uint8_t destinationEndpoint,
                           uint16_t clusterId,
                           uint16_t profileId,
                           uint16_t groupId,
                           uint8_t sequence);

sli_parcel_t *reallyMakeApsCommandFrame(bool security);
sli_parcel_t *makeApsCommandFrame(void);
sli_parcel_t *makeApsCommandFrameWithSecurity(void);

// Convert from in-memory struct format (2 byte sl_zigbee_aps_option_t) to over-the-air
// frame format (1 byte APS frame control).
sli_parcel_t *apsStructToApsFrameParcel(sl_zigbee_aps_frame_t *apsStruct, sl_zigbee_outgoing_message_type_t type);

// Copy the in-memory struct format into a parcel without making any changes.
// The order of the fields in the parcel is different to both the over-the-air
// frame format (which is also a different length) and the EZSP format.
// Hopefully this will avoid things matching when they shouldn't.
sli_parcel_t *apsStructParcel(sl_zigbee_aps_frame_t *apsStruct);

#define makeUnicastApsFrame(src, dest, clusterId, profileId, wantAck) \
  makeApsFrame((ZIGBEE_APS_FRAME_CONTROL_TYPE_DATA                    \
                | ZIGBEE_APS_FRAME_CONTROL_MODE_DIRECT                \
                | ((wantAck)                                          \
                   ? ZIGBEE_APS_FRAME_CONTROL_WANT_ACK                \
                   : 0)),                                             \
               (src), (dest), (clusterId), (profileId), 0, 0)

#define makeBroadcastApsFrame(src, dest, clusterId, profileId) \
  makeApsFrame((ZIGBEE_APS_FRAME_CONTROL_TYPE_DATA             \
                | ZIGBEE_APS_FRAME_CONTROL_MODE_BROADCAST),    \
               (src), (dest), (clusterId), (profileId), 0, 0)

#define makeBroadcastApsFrameWithSequence(src, dest, clusterId, profileId, seq) \
  makeApsFrame((ZIGBEE_APS_FRAME_CONTROL_TYPE_DATA                              \
                | ZIGBEE_APS_FRAME_CONTROL_MODE_BROADCAST),                     \
               (src), (dest), (clusterId), (profileId), 0, (seq))

#define makeApsAckFrameWithSequence(src, dest, clusterId, profileId, seq) \
  makeApsFrame((ZIGBEE_APS_FRAME_CONTROL_TYPE_ACK                         \
                | ZIGBEE_APS_FRAME_CONTROL_MODE_DIRECT),                  \
               (src), (dest), (clusterId), (profileId), 0, (seq))

#define makeApsAckFrame(src, dest, clusterId, profileId) \
  (makeApsAckFrameWithSequence((src), (dest), (clusterId), (profileId), 0))

uint8_t sli_zigbee_parcel_aps_frame_size(sli_parcel_t *header);
uint8_t sli_zigbee_parcel_aps_payload_index(sli_parcel_t *header);

#endif // SL_ZIGBEE_TEST && SL_ZIGBEE_STACK_H

#endif // SILABS_ZIGBEE_PACKET_HEADER_H
