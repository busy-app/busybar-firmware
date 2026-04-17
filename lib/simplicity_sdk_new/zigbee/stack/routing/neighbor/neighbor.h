/***************************************************************************//**
 * @file
 * @brief Maintain neighbor link state information.
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

#ifndef NEIGHBOR_H
#define NEIGHBOR_H

#include "stack/core/sl_zigbee_stack.h"

// Link cost is an integer between 1 and 7 which is used by the
// routing layer to choose optimal routes.  It is based on link reliability:
// 1 represents the most reliable link, and 7 the least.

typedef uint8_t sli_link_cost_t;

// An entry in the neighbor table.  Nodes directly measure the incoming
// link quality, but rely on the neighbors to tell them their outgoing link
// quality.
//
// We use an uint16_t rather than an uint8_t for the incoming link quality
// value in order to have the needed precision to perform rolling averages.
//
// The age field is incremented every neighbor exchange period, and set
// to zero upon reception of a neighbor exchange message which lists us.
// Thus it indicates the time since last obtaining a valid outgoing cost.
//
// The overlap count is used in the neighbor selection algorithm
// to favor neighborhoods that have low overlap with ours.
//
// The active network key bit is used to indicate whether our neighbors
// are using the active network key (my key) or the alternate
// one.  This is important for knowing when to reset their frame counter.
// For children's frame counters, see 'child.h'.

// The Neighbor Association bit indicates whether the device was added to the
// neighbor table because it associated to the local node (vs Link Status
// message).  It is used to help pick which neighbor to remove from the
// neighbor table.

#define SL_ZIGBEE_PRIMARY_MAC_INDEX 0x00 //PHY_INDEX_NATIVE
#define SL_ZIGBEE_SECONDARY_MAC_INDEX 0x01

#define NEIGHBOR_AGE_MASK 0xF0u
#define NEIGHBOR_AGE_SHIFT 4
#define NEIGHBOR_OUTGOING_COST_MASK 0x07u

#define NEIGHBOR_OVERLAP_MASK                             0x03u
#define NEIGHBOR_INCOST_MASK                              0x0Cu
#define NEIGHBOR_USING_ACTIVE_NETWORK_KEY                 0x10u

sl_zigbee_cost_t sli_zigbee_neighbor_entry_incoming_cost(sl_zigbee_neighbor_table_entry_info_t *entry);

// Applies the mask on connectivity and remaps the bits to 1,3,5,7
#define sli_zigbee_neighbor_entry_prev_incoming_cost(entry) \
  ((((entry)->connectivity & NEIGHBOR_INCOST_MASK) >> 1) + 1u)
#define sli_zigbee_neighbor_entry_outgoing_cost(entry) \
  ((entry)->exchange & NEIGHBOR_OUTGOING_COST_MASK)

void sli_zigbee_neighbor_entry_set_outgoing_cost(sl_zigbee_neighbor_table_entry_info_t *entry, uint8_t cost);

void sli_zigbee_neighbor_entry_set_incoming_cost(sl_zigbee_neighbor_table_entry_info_t *entry, uint8_t costBits);

sl_zigbee_cost_t sli_zigbee_neighbor_entry_two_way_cost(sl_zigbee_neighbor_table_entry_info_t *entry);

#define sli_zigbee_neighbor_entry_overlap_count(entry) \
  ((entry)->connectivity & NEIGHBOR_OVERLAP_MASK)

#define sli_zigbee_neighbor_entry_set_overlap_count(entry, overlap) \
  ((entry)->connectivity                                            \
     = ((entry)->connectivity & ~NEIGHBOR_OVERLAP_MASK) + (overlap))

// This macro uses less flash than a function.
#define sli_zigbee_neighbor_entry_age(entry) \
  (((entry)->exchange & NEIGHBOR_AGE_MASK) >> NEIGHBOR_AGE_SHIFT)

void sli_zigbee_neighbor_entry_set_age(sl_zigbee_neighbor_table_entry_info_t *entry, uint8_t age);

//----------------------------------------------------------------
// Security flags.

#define sli_zigbee_neighbor_index_connectivity(index) \
  (((sl_zigbee_neighbor_table_entry_info_t *) (sli_zigbee_neighbor_table + (index)))->connectivity)

#define sli_zigbee_neighbor_index_mac_interface_index(index) \
  (((sl_zigbee_neighbor_table_entry_info_t *) (sli_zigbee_neighbor_table + (index)))->mac_interface_table_index)

#define sli_zigbee_neighbor_index_using_active_network_key(index) \
  (sli_zigbee_neighbor_index_connectivity(index) & NEIGHBOR_USING_ACTIVE_NETWORK_KEY)

#define sli_zigbee_neighbor_index_set_to_active_network_key(index) \
  (sli_zigbee_neighbor_index_connectivity(index) |= NEIGHBOR_USING_ACTIVE_NETWORK_KEY)

#define sli_zigbee_neighbor_index_set_to_alternate_network_key(index) \
  (sli_zigbee_neighbor_index_connectivity(index) &= ~NEIGHBOR_USING_ACTIVE_NETWORK_KEY)

#define sli_zigbee_neighbor_index_authenticated(index) \
  (sli_zigbee_neighbor_index_connectivity(index) & NEIGHBOR_AUTHENTICATED)

#define sli_zigbee_neighbor_entry_authenticated(entry) \
  ((entry)->connectivity & NEIGHBOR_AUTHENTICATED)

#define sli_zigbee_neighbor_set_mac_interface_index(index, value) \
  (sli_zigbee_neighbor_index_mac_interface_index(index) = (value))

//----------------------------------------------------------------

#define sli_zigbee_neighbor_table_is_full() (sli_zigbee_neighbor_count >= sli_zigbee_neighbor_table_size)

// A value of NO_OUTGOING_COST for the outgoing field means that we havn't
// recently received a neighbor exchange message which from that neighbor
// which lists us.
#define NO_OUTGOING_COST 0u

// Time between neighbor aging events in milliseconds.  Used for both the tree
// stack and the mesh stack.
#define EM_NEIGHBOR_AGING_PERIOD 16000

// A neighbor is stale when the age field is greater than this value.
#define EM_STALE_NEIGHBOR 6u

// A neighbor used for routing is stale when the age field is greater than this value.
#define EM_STALE_ROUTING_NEIGHBOR 8u

#define EM_MAX_NEIGHBOR_AGE (EM_STALE_ROUTING_NEIGHBOR + 1u)

// A neighbor must stay in the neighbor table at least this long
// before being evicted to avoid thrashing in dense networks.
#define EM_MIN_NEIGHBOR_AGE 3u

// The neighbor table itself.  This is allocated by sl_zigbee_configuration.c
// using the dummy type EmNeighborData.
extern sl_zigbee_neighbor_table_entry_info_t sli_zigbee_neighbor_data[];

extern uint8_t sli_zigbee_router_neighbor_table_size;

// Neighbor and child frame counters used by MAC and network security.
// The neighbor values come first.
extern uint32_t sli_zigbee_frame_counters_table[];

extern sli_zigbee_event_t sli_zigbee_neighbor_event;

// Updates the rolling average link quality for the sending node,
// or creates a new entry if there is an unused or stale  entry.
void sli_zigbee_neighbor_process_quality(sl_802154_short_addr_t neighbor);

//------------------------------------------------------------------------------
// Neighbor exchange

// Called from network.c.
void sli_zigbee_process_neighbor_exchange(sli_zigbee_packet_header_t header);

void sli_zigbee_reschedule_neighbor_exchange(void);

uint8_t sli_zigbee_weakest_neighbor_index(void);

//------------------------------------------------------------------------------
// Utilities

sl_zigbee_neighbor_table_entry_info_t * sli_zigbee_neighbor_find_entry(sl_802154_short_addr_t id);

// Returns the index of the neighbor entry in the neighbor table,
// or 0xFF if not found.
uint8_t sli_zigbee_neighbor_entry_index(sl_802154_short_addr_t neighbor);

// Returns the short id of a neighbor with the given long id,
// or NULL_NODE_ID if it is not found.
sl_802154_short_addr_t sli_zigbee_find_neighbor_by_eui64(sl_802154_long_addr_t eui64);

// Returns the index of a neighbor with the given long id,
// or 0xFF if it is not found.
uint8_t sli_zigbee_find_neighbor_index_by_eui64(sl_802154_long_addr_t eui64);

// Finds the given neighbor in the table, or adds it if it is not there
// and there are empty entries, or stale entries that are not in use.
// Returns the index, or 0xFF if the entry was not added.
uint8_t sli_zigbee_find_or_add_neighbor(sl_802154_short_addr_t id);

// Used during association to temporarily store the long to short id mapping.
uint8_t sli_zigbee_add_neighbor_guaranteed(sl_802154_short_addr_t shortId, sl_802154_long_addr_t longId);

// Used during association to kickstart routing communication between
// a router child and its parent.
void sli_zigbee_assume_symmetric_link(uint8_t neighborIndex);

// Deletes the entry at the given index and creates a new entry for
// the given neighbor id.  Returns the index of the new neighbor entry.
uint8_t sli_zigbee_replace_neighbor(uint8_t index, sl_802154_short_addr_t id);

void sli_zigbee_remove_neighbor(uint8_t index);

// Remove any entries matching the longId and NOT matching avoidShortId.
// This is for removing entries whose short id has changed.  It can
// also be used to remove entries by longId simply by passing NULL_NODE_ID
// for the avoidShortid.
void sli_zigbee_remove_neighbor_eui(sl_802154_long_addr_t longId, sl_802154_short_addr_t avoidShortId);

// Remove any entries communicating over subgig.
// This is called after multi-phy is disabled
void sli_zigbee_remove_neighbor_subgig(void);

// Fill in the long id given a new long/short pair.
// This is called *after* conflicts have been detected
// so it assumes there are no conflicts.
void sli_zigbee_update_neighbor_table(sl_802154_long_addr_t longId, sl_802154_short_addr_t shortId);

// These return 0 if the cost is unknown or too old.
sl_zigbee_cost_t sli_zigbee_neighbor_cost(sl_802154_short_addr_t id, bool incomingOnly);
#define sli_zigbee_neighbor_incoming_cost(id) (sli_zigbee_neighbor_cost((id), true))
#define sli_zigbee_neighbor_two_way_cost(id)   (sli_zigbee_neighbor_cost((id), false))

sl_zigbee_cost_t sli_zigbee_link_quality_to_cost(sli_link_quality_t quality);

void sli_zigbee_set_neighbor_eui64(uint8_t index, sl_802154_long_addr_t eui64);

void sli_zigbee_neighbor_init(void);

// Called in broadcast.c to get an initial bitmask for the broadcast table
sli_neighbor_bitmask_t sli_zigbee_make_broadcast_bitmask(sl_802154_short_addr_t source);

// Sets the bit indicating that our neighbors are using the
void sli_zigbee_set_all_neighbors_using_alternate_key(void);

// Choose a random two-way neighbor.  Used for many-to-one route errors.
sl_802154_short_addr_t sli_zigbee_choose_random_neighbor(void);

#endif // NEIGHBOR_H
