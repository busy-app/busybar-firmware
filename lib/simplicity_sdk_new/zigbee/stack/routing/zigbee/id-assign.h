/***************************************************************************//**
 * @file
 * @brief ZigBee id assignment interface
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

#ifndef ID_ASSIGN_H
#define ID_ASSIGN_H

// Moved to sl_zigbee.h
//extern sl_802154_short_addr_t sli_zigbee_parent_id; // only useful for end devices in general

bool sli_zigbee_initialize_id_assignment(void);

// Compares the given long/short id pair to our own and to the
// address table.  Takes action and returns true if a conflict
// is detected.  Updates neighbor/address/binding tables if
// no conflict is detected and the update argument is true.
bool sli_zigbee_detect_id_conflict(sl_802154_short_addr_t shortId,
                                   uint8_t *longId,
                                   bool update);

// The random ID version of this picks a new ID for the local node.  The
// tree version does nothing.
void sli_zigbee_pick_new_id(bool sendRouteError);

// Called by the neighbor code when it notices that two neighbors share
// addresses.
void sli_zigbee_neighbor_id_conflict(sl_802154_short_addr_t neighborAddress, uint8_t *longId);

// Check if the conflicting id belongs to our child and handle it if so.
// Random ID only.
void sli_zigbee_handle_child_id_conflict(sl_802154_short_addr_t target, uint8_t *longId);

//----------------------------------------------------------------
// The random ID code uses broadcast and route request sequence numbers
// to detect conflicts.

// Called when a broadcast arrives that has our or one of our end-device
// children's node ID as source but was not in the history table.  This
// could be a harmless consequence of a reboot.

void sli_zigbee_unrecognized_broadcast_sequence(void);

// This is for the random id code.  It is needed by the
// route error code for picking a new id for a conflicted child.
sl_802154_short_addr_t sli_zigbee_pick_random_id(void);

//----------------------------------------------------------------
// The two stacks use different criteria to choose their parent.

bool sli_zigbee_new_beacon_is_better(sli_zigbee_packet_header_t oldBeacon,
                                     uint8_t oldLqi,
                                     int8_t oldRssi,
                                     uint8_t oldDepth,
                                     uint8_t newDepth);

//----------------------------------------------------------------
// A list of IDs that have recently been in conflict.

void sli_zigbee_add_blacklisted_id(sl_802154_short_addr_t badId);
bool sli_zigbee_is_blacklisted_id(sl_802154_short_addr_t id);
void sli_zigbee_age_blacklisted_ids(void);

#endif // ID_ASSIGN_H
