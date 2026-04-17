/***************************************************************************//**
 * @file
 * @brief The in-core representation of packet headers.  The MAC
 * translates outgoing packets from this to the over-the-air packet format and
 * converts them back again on the way in.
 * Header layout:
 * Offset Size Description
 *  0      1    payload buffer
 *  1      2    mac info bytes
 *  3      6    queue storage
 *
 * ZigBee messages follow this with
 *  9      2    destination (for outgoing), source (for incoming)
 * 11      ?    NWK frame
 *
 * All other packets follow this with a standard 802.15.4 packet.
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

#ifndef SILABS_PACKET_HEADER_H
#define SILABS_PACKET_HEADER_H

#include "mac-header.h"

// The overhead is the payload byte, mac info field,
// and possibly queue storage and address field.
#define PACKET_HEADER_OVERHEAD        SL_802154_IN_MEMORY_OVERHEAD

// used in unit-test for parcel processing
#define PARCEL_MAC_INFO_INDEX        0

// This offset is relative to the network payload index to get the command frame.
// in the header (refer to mac-header.h) hence we need to subtract it when getting the command frame.
#define REJOIN_COMMAND_FRAME_INDEX_OFFSET (16)

//------------------------------------------------------------------------------
// Utilities for reading and writing from headers (these actually work
// on any buffer).  The values must be in the first 32 bytes.

uint8_t sli_zigbee_header_get_int8u(sli_zigbee_packet_header_t header, uint8_t index);
void sli_zigbee_header_set_int8u(sli_zigbee_packet_header_t header, uint8_t index, uint8_t data);
uint16_t sli_zigbee_header_get_int16u(sli_zigbee_packet_header_t header, uint8_t index);
void sli_zigbee_header_set_int16u(sli_zigbee_packet_header_t header, uint8_t index, uint16_t value);

// Returns the index of the NWK frame (for data packets) or the index of the PHY
// header (for all other packets).
#define sli_zigbee_packet_header_overhead(header)  SL_802154_IN_MEMORY_OVERHEAD

// Returns a pointer to the first byte of the the NWK frame (for data
// packets) or the PHY header (for all other packets).
#define sli_zigbee_packet_header_contents(header)  sli_mac_payload_pointer(header)

#define sli_zigbee_header_routing_frame(header)    sli_zigbee_packet_header_contents(header)

//------------------------------------------------------------------------------
// Payload
//
// Headers whose initial byte contains a payload buffer have 0x80 added
// to their reference counts.  The buffer code uses this to release the
// payload buffer when the header is freed.

#define HEADER_PAYLOAD_COUNT_MASK 0x80

#define sli_zigbee_header_has_payload(header) \
  (sli_legacy_packet_buffer_packet_header_payload(header) != NULL_BUFFER)

// Every packet header has a link to the buffer containing the payload.
//sli_buffer_manager_buffer_t sli_legacy_packet_buffer_packet_header_payload(sli_zigbee_packet_header_t header);

// This is here to allow the mac to create a header before having
// read in the payload.  No one else should use it.
//void sli_legacy_packet_buffer_set_packet_header_payload(sli_zigbee_packet_header_t header, sli_buffer_manager_buffer_t payload);

//------------------------------------------------------------------------------
// Mac info field

#define sli_zigbee_header_mac_info_field(header)          sli_mac_header_mac_info(header)
#define sli_zigbee_header_set_mac_info_field(header, info) sli_mac_header_set_mac_info(header, info)
#define sli_zigbee_header_set_stack_private(header)       sli_mac_header_set_mac_info_bit(header, SL_802154_INFO_STACK_PRIVATE_MASK, true)
// The low byte of the mac info field contains the older flags.
#define sli_802154mac_info_low_byte(header)              (uint8_t)(sli_mac_header_mac_info(header) & 0x00FF)

#if !defined(SL_ZIGBEE_MULTI_NETWORK_STRIPPED)
#define sli_zigbee_header_get_network_index(header) sli_mac_nwk_index(header)
#else
#define sli_zigbee_header_get_network_index(header) 0
#endif // !defined(SL_ZIGBEE_MULTI_NETWORK_STRIPPED)

#define sli_zigbee_header_is_stack_private(header) \
  (sli_802154mac_info_low_byte(header) & SL_802154_INFO_STACK_PRIVATE_MASK)

#define sli_zigbee_header_frame_type(header) sli_mac_header_mac_info_frame_type(header)

#define sli_zigbee_header_is_pass_through(header) \
  (sli_zigbee_header_frame_type(header) == SL_802154_INFO_TYPE_PASSTHROUGH)

#define sli_zigbee_header_is_data(header) \
  (sli_zigbee_header_frame_type(header) == SL_802154_INFO_TYPE_DATA)

// Only data packets need reformatting by the MAC.
#define sli_zigbee_header_is_mac_pass_through(header) (!sli_zigbee_header_is_data(header))

//------------------------------------------------------------------------------
// MAC storage

// Get and set the MAC source for incoming data packets and the MAC destination
// for outgoing data packets.
#define sli_zigbee_get_mac_data_address(header)      sli_mac_source(header)
#define sli_zigbee_set_mac_data_address(header, id)  sli_mac_set_destination(header, id)

// Access to the addresses of all headers, including beacons, MAC commands,
// and passthrough.  These can be applied to data packets as well, but the
// 'source' and 'destination' will both be the one address in the header.
// It is up to the caller to know if the packet is incoming or outgoing.
#define sli_802154mac_destination_mode(header)    sli_mac_destination_mode(header)
#define sli_802154mac_destination_pointer(header) sli_mac_destination_pointer(header)
#define sli_802154mac_short_destination(header)   sli_mac_destination(header)

#define sli_802154mac_source_mode(header)         sli_mac_source_mode(header)
#define sli_802154mac_source_pointer(header)      sli_mac_source_pointer(header)
#define sli_802154mac_short_source(header)        sli_mac_source(header)

//------------------------------------------------------------------------------
// Creating Packet Headers

#define sli_zigbee_make_raw_packet_header(macInfoFlags, payload) \
  sli_mac_make_raw_message((payload),                            \
                           (macInfoFlags),                       \
                           sli_zigbee_get_current_network_index())

#define sli_zigbee_make_data_packet_header(macInfoFlags, id, payload)                \
  sli_mac_make_data_message((id),                                                    \
                            sli_legacy_buffer_manager_get_buffer_length((payload)),  \
                            sli_legacy_buffer_manager_get_buffer_pointer((payload)), \
                            (macInfoFlags) | SL_802154_INFO_TYPE_DATA,               \
                            sli_zigbee_get_current_network_index())

#ifdef SL_ZIGBEE_TEST
// For building test headers.

sli_zigbee_packet_header_t makeMacHeader(uint16_t macFrameControl,
                                         uint8_t *destination,
                                         uint16_t destinationPanId,
                                         uint8_t *source,
                                         uint16_t sourcePanId,
                                         uint8_t *frame,
                                         uint8_t frameLength);

#define sli_zigbee_make_passthrough_header_parcel(macInfoFlags, frameControl) \
  (makeMessage("<2<2111111<4<2",                                              \
               ((macInfoFlags)                                                \
               ),                                                             \
               sli_zigbee_get_current_network_index(),                        \
               0,                                                             \
               0,                                                             \
               0,                                                             \
               0,                                                             \
               0,                                                             \
               0,                                                             \
               0,                                                             \
               (frameControl)))

#define sli_zigbee_make_raw_header_parcel(macInfoFlags, frameControl) \
  (makeMessage("<2<211111<4<2<2<2",                                   \
               ((macInfoFlags)                                        \
               ),                                                     \
               sli_zigbee_get_current_network_index(),                \
               0,                                                     \
               0,                                                     \
               0,                                                     \
               0,                                                     \
               0,                                                     \
               0,                                                     \
               0,                                                     \
               0,                                                     \
               (frameControl)))

#define sli_zigbee_make_raw_header_parcel_without_phy(macInfoFlags, frameControl) \
  (makeMessage("<2<2111<2<4<21<2",                                                \
               ((macInfoFlags)                                                    \
               ),                                                                 \
               sli_zigbee_get_current_network_index(),                            \
               0,                                                                 \
               0,                                                                 \
               0,                                                                 \
               0,                                                                 \
               0,                                                                 \
               0,                                                                 \
               0,                                                                 \
               (frameControl)))

#define sli_zigbee_make_umac_overhead_parcel(macInfoFlags) \
  (makeMessage("<2<2111<2<4<21",                           \
               ((macInfoFlags)                             \
               ),                                          \
               sli_zigbee_get_current_network_index(),     \
               0,                                          \
               0,                                          \
               0,                                          \
               0,                                          \
               0,                                          \
               0,                                          \
               0))

#define sli_zigbee_make_umac_overhead_parcel_with_lqi_rssi(macInfoFlags, lqi, rssi) \
  (makeMessage("<2<2111111111111",                                                  \
               ((macInfoFlags)                                                      \
               ),                                                                   \
               sli_zigbee_get_current_network_index(),                              \
               0x0,                                                                 \
               0x0,                                                                 \
               0x0,                                                                 \
               0x0,                                                                 \
               (lqi),                                                               \
               (rssi),                                                              \
               0x0,                                                                 \
               0x0,                                                                 \
               0x0,                                                                 \
               0x0,                                                                 \
               0x0,                                                                 \
               0x0))

#define sli_zigbee_make_data_header_parcel(address)                               \
  (makeMessage("<2<2<2111<41<2",                                                  \
               SL_802154_INFO_TYPE_DATA,                                          \
               sli_zigbee_get_current_network_index(),                            \
               (address),/*in_memory_packet->info.pkt_info.tx_data.destination */ \
               0,                                                                 \
               0,                                                                 \
               0,                                                                 \
               0,                                                                 \
               0,                                                                 \
               0))

#define sli_zigbee_make_beacon_header_parcel(shortId, panId, superframe)        \
  (makeMessage("p<21<2<2<211",                                                  \
               (sli_zigbee_make_umac_overhead_parcel(SL_802154_INFO_TYPE_BEACON \
                                                     )),                        \
               MAC_BEACON_FRAME_CONTROL,                                        \
               0,        /* sequence number */                                  \
               (panId),                                                         \
               (shortId),                                                       \
               ((superframe) | ZIGBEE_BEACON_SUPERFRAME),                       \
               0,        /* no GTS */                                           \
               0))       /* no pending addresses */

#define sli_zigbee_make_beacon_header_parcel_with_lqi_rssi(shortId, panId, superframe, lqi, rssi) \
  (makeMessage("p<21<2<2<211",                                                                    \
               (sli_zigbee_make_umac_overhead_parcel_with_lqi_rssi(SL_802154_INFO_TYPE_BEACON     \
                                                                   , lqi, rssi)),                 \
               MAC_BEACON_FRAME_CONTROL,                                                          \
               0,        /* sequence number */                                                    \
               (panId),                                                                           \
               (shortId),                                                                         \
               ((superframe) | ZIGBEE_BEACON_SUPERFRAME),                                         \
               0,        /* no GTS */                                                             \
               0))       /* no pending addresses */

// This is to test bug 11506.

#define sli_zigbee_make_beacon_header_parcel_with_pending_addresses(shortId, panId, superframe) \
  (makeMessage("p<21<2<2<211<4<4<2",                                                            \
               (sli_zigbee_make_umac_overhead_parcel(SL_802154_INFO_TYPE_BEACON                 \
                                                     )),                                        \
               MAC_BEACON_FRAME_CONTROL,                                                        \
               0,        /* sequence number */                                                  \
               (panId),                                                                         \
               (shortId),                                                                       \
               ((superframe) | ZIGBEE_BEACON_SUPERFRAME),                                       \
               0,        /* no GTS */                                                           \
               0x11, 0x1234, 0x5678, 0xABCD))

#endif // SL_ZIGBEE_TEST

#endif // SILABS_PACKET_HEADER_H
