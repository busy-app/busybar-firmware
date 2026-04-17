/***************************************************************************//**
 * @file
 * @brief Keep track of who we've heard broadcasts from
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

#ifndef BROADCAST_H
#define BROADCAST_H

#define BROADCAST_COMPLETE_MASK 0xFFFFFFFFU

#define BROADCAST_WAIT_FOR_ALL_ACKS   0xFF

// The number of passive acknowledgements to record from neighbors before we
// stop re-transmitting broadcasts
// The default (0xFF) is to wait for all neighbors to re-broadcast the message
#ifndef SL_ZIGBEE_BROADCAST_MIN_ACKS_NEEDED
 #define SL_ZIGBEE_BROADCAST_MIN_ACKS_NEEDED  BROADCAST_WAIT_FOR_ALL_ACKS
#endif // SL_ZIGBEE_BROADCAST_MIN_ACKS_NEEDED

// Few folks want to create a larger broadcast table, so we save
// a bit of RAM and flash by normally having the table be a fixed
// size.  Larger tables are created by config/sl_zigbee_configuration.c;
// the usual small table is in broadcast.c.

#define BROADCAST_TABLE_SIZE sli_zigbee_broadcast_table_size
extern uint8_t sli_zigbee_broadcast_table_size;
extern sl_zigbee_broadcast_table_entry_t sli_zigbee_broadcast_table_data[];

// The table is aged in batches.  The ith cutoff starts at index
//    (sli_zigbee_broadcast_age_cutoff_indices
//      >> (BITS_PER_CUTOFF_INDEX * (AGE_CUTOFFS - 1 - i))) & 0xFF
// from sli_zigbee_broadcast_head.  The entries after
// cutoff i are at least (i * AGE_CUTOFF_INCREMENT) quarter seconds old.
// The current settings use four 5 second buckets, giving a broadcast
// entry timeout of 15 - 20 seconds, with 15 table entries.
// A new entry can only be added to the table if there are fewer than
// NEW_ENTRY_THRESHOLD broadcasts in the third bucket, ie, fewer than
// that many broadcasts have been added within the last 10 - 15 seconds.
//
// This requirement approximates the behavior of the pre-z3.1 stack,
// which used three 4.5 second buckets and an 8 entry table.
// In testing we observed that on a 100 node network with
// heavy broadcast traffic, broadcasts could easily last up to 20 seconds.
// The ZigBee Pro profile document 074855r05 states a value of 9 seconds
// for the entry timeout, which is too short.  We double the timeout and
// the table size in order to get the same bandwidth as the spec.
// The danger of making the timeout too short
// is rejuvenated broadcasts and false id conflict detections.
//
// sli_zigbee_broadcast_age_cutoff_indices used to be an array of bytes but the
// operations on it can be done more efficiently if it is an integer.
#define AGE_CUTOFFS 4
#define AGE_CUTOFF_INCREMENT 20  // units are quarter seconds
#define BROADCAST_ENTRY_TIMEOUT (AGE_CUTOFFS * AGE_CUTOFF_INCREMENT)
#define CUTOFF_INDEX_MASK ((1 << BITS_PER_CUTOFF_INDEX) - 1)

#if SL_ZIGBEE_BROADCAST_TABLE_TIMEOUT_QS != (AGE_CUTOFFS * AGE_CUTOFF_INCREMENT)
  #error "Public API of SL_ZIGBEE_BROADCAST_TABLE_TIMEOUT_QS not in sync with internal values!"
#endif

// This must be larger than log2(BROADCAST_TABLE_SIZE)
  #define BITS_PER_CUTOFF_INDEX 8
// One in each of the four one-byte buckets
  #define AGE_CUTOFFS_INCREMENT 0x01010101
  #define NEW_ENTRY_THRESHOLD (BROADCAST_TABLE_SIZE - 6)

// For the test app.
#define sli_zigbee_broadcast_age_cutoff_index(i)        \
  ((sli_zigbee_broadcast_age_cutoff_indices             \
    >> (BITS_PER_CUTOFF_INDEX * (AGE_CUTOFFS - 1 - i))) \
   & CUTOFF_INDEX_MASK)

sl_zigbee_broadcast_table_entry_t *sli_zigbee_find_broadcast_table_entry(sl_802154_short_addr_t source,
                                                                         uint8_t sequence);

// Used in ID assignment to check if a randomly chosen ID is already in use.
bool sli_zigbee_is_node_in_broadcast_table(sl_802154_short_addr_t node);

bool sli_zigbee_broadcast_table_is_full(void);

void sli_zigbee_age_broadcast_table(void);

// Returns false if there was no space.
bool sli_zigbee_add_broadcast_table_entry(sl_802154_short_addr_t source,
                                          uint8_t sequence,
                                          sl_802154_short_addr_t macSource);

// Updates the neighbor bitmask of an existing entry
// Returns false if the entry is not found.
bool sli_zigbee_update_broadcast_table(sl_802154_short_addr_t source,
                                       uint8_t sequence,
                                       sl_802154_short_addr_t macSource);

void sli_zigbee_initialize_broadcast(void);

// Gets the number of broadcast passive acks required before terminating a
// broadcast transmission
// AND the passive ack mode
bool sli_get_passive_ack_config(uint8_t *p_min_ack_needed,
                                sl_passive_ack_config_enum_t *p_passive_ack_mode);

#endif // BROADCAST_H
