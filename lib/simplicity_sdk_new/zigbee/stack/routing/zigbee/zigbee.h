/***************************************************************************//**
 * @file
 * @brief Constants and whatnot from the ZigBee specification.
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

#ifndef SILABS_ZIGBEE_H
#define SILABS_ZIGBEE_H

#include "stack/mac/multi-mac.h"  // for MAC_DUAL_PRESENT

//----------------------------------------------------------------
// MAC constants, some of which I can't find in the MAC anywhere.
#define aMaxPhyPacketSize PHY_MAX_SIZE
#define aMaxFrameOverhead  25
#define aMaxMacFrameSize (aMaxPhyPacketSize - aMaxFrameOverhead)

//----------------------------------------------------------------
// NWK constants, typed in from table 36 of NWK spec version 0.92
#define ZIGBEE_COORDINATOR_CAPABLE      true
#define ZIGBEE_DEFAULT_NETWORK_SECURITY_LEVEL   ZIGBEE_SECURITY_ENC_MIC_64
#define ZIGBEE_DISCOVERY_RETRY_LIMIT    0x03
#define ZIGBEE_MAX_DEPTH                0x0F
#define ZIGBEE_MAX_CHILDREN             0xFF
#define ZIGBEE_MAX_FRAME_OVERHEAD       0x08 // Changed from 0x0D by CCB #366
#define ZIGBEE_MAX_NETWORK_ADDRESS      0xFFF7

// The 15.4 spec says that the maximum MAC payload is the maximum PHY message
// minus the maximum MAC header.  Other than during association, all ZigBee
// messages have a MAC header that is 14 bytes less than the maximum MAC header.
// At some point, it is believed the Zigbee spec will allow devices to
// use that 14 bytes of extra payload.  For now it is not allowed.
// This behavior can be turned on/off in application-support.c

#define ZIGBEE_NETWORK_MAX_PAYLOAD_SIZE \
  (aMaxMacFrameSize - ZIGBEE_MAX_FRAME_OVERHEAD)

#define ZIGBEE_PROTOCOL_ID              0x00
// #define ZIGBEE_NETWORK_PROTOCOL_VERSION   // moved to zigbee-packet-header.h
#define ZIGBEE_REPAIR_THRESHOLD         0x03
#define ZIGBEE_ROUTE_DISCOVERY_TIME     0x14    // in quarter seconds (5 secs)
#define ZIGBEE_INITIAL_ROUTE_REQUEST_RETRIES 0x03
#define ZIGBEE_ROUTE_REQUEST_RETRIES    0x02
#define ZIGBEE_ROUTE_REQUEST_RETRY_INTERVAL 0xfe // in milliseconds (= 254)
#define ZIGBEE_MIN_ROUTE_REQUEST_JITTER 0x02     // in milliseconds
#define ZIGBEE_MAX_ROUTE_REQUEST_JITTER 0x80     // in milliseconds (= 128)

// Retrying route replies is not in the ZigBee spec, it's an Ember thing.
#define ZIGBEE_ROUTE_REPLY_RETRIES      0x02
#define ZIGBEE_ROUTE_REPLY_RETRY_INTERVAL 400  // in milliseconds
#define ZIGBEE_ID_CONFLICT_JITTER_MASK  0xFF     // 255 milliseconds

// This is called "nwkPassiveAckTimeout" in the spec.
#define ZIGBEE_BROADCAST_RETRY_MAX_INTERVAL 500    // in milliseconds
#define ZIGBEE_MAX_BROADCAST_RETRIES 2
#define ZIGBEE_BROADCAST_JITTER_MASK    0x3F   // in milliseconds (= 63)
#define ZIGBEE_BROADCAST_DELIVERY_TIME 3000        // in milliseconds

// Network retries are not specified in ZigBee.
// A more descriptive name would be "extended MAC retries".
extern uint8_t sli_zigbee_max_network_retries;

#define ZIGBEE_NETWORK_RETRY_MIN_INTERVAL 16    // in milliseconds
#define ZIGBEE_NETWORK_RETRY_JITTER_MASK  0x1F  // jitter 0-32 msec
#define ZIGBEE_NETWORK_RETRY_TIMEOUT      750   // in milliseconds
// Need to increase timeout value for subghz supported phy due to LBT + 100ms between
// packet transmission constraints.
#define ZIGBEE_SUBGHZ_NETWORK_RETRY_TIMEOUT \
  (ZIGBEE_NETWORK_RETRY_TIMEOUT + (sli_zigbee_retry_queue_size * 100)) // in milliseconds
#define ZIGBEE_NETWORK_RETRY_WAIT_FOR_STACK_UP_INTERVAL 40  // milliseconds

// Messages that trigger a route record are delayed by this much.
// This reduces the chance of the message arriving before the
// route record.  We make it large enough to cover the worst case
// for a single network retry (50 ms).
#define POST_ROUTE_RECORD_DELAY_MS \
  (ZIGBEE_NETWORK_RETRY_MIN_INTERVAL + ZIGBEE_NETWORK_RETRY_JITTER_MASK + 1)

// Application support constants
#define ZIGBEE_APPLICATION_FRAME_OVERHEAD                0x08
#define ZIGBEE_APPLICATION_AND_SECURITY_FRAME_OVERHEAD   0x14
#define ZIGBEE_APSC_MAX_TRANSMIT_RETRIES                 3

// We currently have the application-defined profile (zero),
// the home-lighting-control profile (one), the Zigbee Pro Profile (two), and
// the deprecated Zigbee RF4CE Profile (three).
#define ZIGBEE_HOME_CONTROL_STACK_PROFILE     1
#define ZIGBEE_PRO_STACK_PROFILE              2
#define ZIGBEE_RF4CE_STACK_PROFILE            3

// ZigBee used to reserve PAN IDs higher than 0x3FFF, but that was
// dropped somewhere before r14 of the spec.  0xFFFF is the broadcast
// PAN ID and cannot be used.
#define ZIGBEE_MAX_PAN_ID 0xFFFE

//extern uint8_t sli_zigbee_network_security_level;            // moved to sl_zigbee.h
//extern uint8_t sli_zigbee_max_hops;                               // moved to sl_zigbee.h

//----------------------------------------------------------------
// The following is a zigbee beacon packet format. This is what we parse when we
// receive a packet from the MAC and the in memory packet format is raw
// (basically, not MAC data messages, which would be the case for beacons).
//
// PHY header
//   PHY length          (1 byte)
// MAC header
//   frame control       (2 bytes)
//   sequence number     (1 byte)
//   Pan ID              (2 bytes)
//   Sender ID           (2 bytes)
//
// MAC Beacon Payload
//   MAC superframe      (2 bytes)
//   GTS                 (1 byte)
//   pending addresses   (1 byte)
//
// Beacon Payload
//   NWK Protocol ID     (1 byte)
//
//   Stack Profile       (4 bits)
//   Protocol Version    (4 bits)
//
//   Reserved            (2 bits)
//   Router Capacity     (1 bit)   // Bit 18
//   Depth               (4 bits)
//   End Device Capacity (1 bit)
//
//   Extended PAN Id     (8 bytes) // Bits 24 - 87
//   Tx Offset           (3 bytes) // Bits 88 - 111
//   NWK Update ID       (1 byte)

// The size of a 802.15.4 MAC beacon header in bytes.
#define MAC_BEACON_HEADER_SIZE (7)

// The size of a 802.15.4 MAC enhanced beacon header in bytes.
#define MAC_ENHANCED_BEACON_HEADER_SIZE (13)

// The size of a 802.15.4 MAC beacon payload in bytes.
#define MAC_BEACON_SIZE (4)

// For raw packets, the beacon starts after the raw header plus the PHY byte
#define IEEE_802_15_4_BEACON_OFFSET   (0)

#define BEACON_FRAME_CONTROL_OFFSET    (IEEE_802_15_4_BEACON_OFFSET +  0)
#define BEACON_PAN_ID_OFFSET           (IEEE_802_15_4_BEACON_OFFSET +  3)
#define BEACON_SENDER_ID_OFFSET        (IEEE_802_15_4_BEACON_OFFSET +  5)
#define BEACON_SUPERFRAME_OFFSET       (IEEE_802_15_4_BEACON_OFFSET +  7)
#define BEACON_GTS_OFFSET              (IEEE_802_15_4_BEACON_OFFSET +  9)
#define BEACON_PENDING_ADDRESS_OFFSET  (IEEE_802_15_4_BEACON_OFFSET + 10)
#define BEACON_PAYLOAD_STANDARD_OFFSET (IEEE_802_15_4_BEACON_OFFSET + 11)

// These offsets are from the start of the beacon payload
#define BEACON_NWK_PROTOCOL_ID_OFFSET  0
#define BEACON_STACK_PROFILE_OFFSET    1
#define BEACON_DEVICE_CAPACITY_OFFSET  2
#define BEACON_EXTENDED_PAN_ID_OFFSET  3
#define BEACON_TX_OFFSET_OFFSET        11
#define BEACON_NWK_UPDATE_ID_OFFSET    14
#define BEACON_PAYLOAD_SIZE            15

#define ZIGBEE_PRO_BEACON_LENGTH      (IEEE_802_15_4_BEACON_OFFSET + 26)

// Avoid messing with beacons that don't look exactly as expected.
#define MAC_BEACON_FRAME_CONTROL \
  (MAC_FRAME_TYPE_BEACON         \
   | MAC_FRAME_SOURCE_MODE_SHORT \
   | MAC_FRAME_DESTINATION_MODE_NONE)

#define MAC_BEACON_LONG_FRAME_CONTROL \
  (MAC_FRAME_TYPE_BEACON              \
   | MAC_FRAME_SOURCE_MODE_LONG       \
   | MAC_FRAME_DESTINATION_MODE_NONE)

#define MAC_ENHANCED_BEACON_FRAME_CONTROL \
  (MAC_FRAME_TYPE_BEACON                  \
   | MAC_FRAME_SOURCE_MODE_LONG           \
   | MAC_FRAME_DESTINATION_MODE_NONE      \
   | MAC_FRAME_FLAG_IE_LIST_PRESENT       \
   | MAC_FRAME_VERSION_2012)

// No beacon order, no superframe order, and the CAP is whole frame.
// The MAC_SUPERFRAME_PAN_COORD and MAC_SUPERFRAME_ASSOC_PERMIT may
// be set.
#define ZIGBEE_BEACON_SUPERFRAME    \
  (MAC_SUPERFRAME_BEACON_ORDER_MASK \
   | MAC_SUPERFRAME_ORDER_MASK      \
   | MAC_SUPERFRAME_FINAL_CAP_MASK)

#define ZIGBEE_BEACON_SUPERFRAME_FLAGS \
  (MAC_SUPERFRAME_PAN_COORD | MAC_SUPERFRAME_ASSOC_PERMIT)

#define BEACON_TX_OFFSET_SIZE 3

// Breaking up the stack profile byte
#define sli_zigbee_beacon_stack_profile(beacon) \
  ((beacon)[BEACON_STACK_PROFILE_OFFSET] & 0x0F)
#define sli_zigbee_beacon_protocol_version(beacon) \
  ((beacon)[BEACON_STACK_PROFILE_OFFSET] >> 4)

// Breaking up the capacity byte
#define BEACON_ROUTER_CAPACITY_MASK     0x04
#define BEACON_DEPT_MASK                0x78
#define BEACON_END_DEVICE_CAPACITY_MASK 0x80
#define BEACON_HUB_CONNECTIVITY_MASK    0x02
#define BEACON_LONG_UPTIME_MASK         0x01
#define BEACON_PARENT_PRIORITY_MASK     0x03

#define sli_zigbee_beacon_depth(beacon) \
  (((beacon)[BEACON_DEVICE_CAPACITY_OFFSET] & BEACON_DEPT_MASK) >> 3)
#define sli_zigbee_beacon_router_capacity(beacon) \
  ((beacon)[BEACON_DEVICE_CAPACITY_OFFSET] & BEACON_ROUTER_CAPACITY_MASK)
#define sli_zigbee_beacon_child_capacity(beacon) \
  ( (beacon)[BEACON_DEVICE_CAPACITY_OFFSET] & BEACON_END_DEVICE_CAPACITY_MASK)
#define sli_zigbee_beacon_parent_priority(beacon) \
  ( (beacon)[BEACON_DEVICE_CAPACITY_OFFSET] & BEACON_PARENT_PRIORITY_MASK)

#endif // SILABS_ZIGBEE_H
