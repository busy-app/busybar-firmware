/***************************************************************************//**
 * @file
 * @brief ZigBee child table
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

#ifndef SILABS_ZIGBEE_CHILD_H
#define SILABS_ZIGBEE_CHILD_H

#include "mac/multi-mac.h"
#ifdef SL_ZIGBEE_NO_STACK
// For no stack application such as nodetest to get sli_zigbee_child_status.
#include "core/sl_zigbee_multi_network.h"
#endif

// Entries in the child table have three fields: the child's EUI64,
// the child's node ID, and a capabilities byte.
#define CHILD_ENTRY_EUI64_OFFSET         0
#define CHILD_ENTRY_NODE_ID_OFFSET       8
#define CHILD_ENTRY_CAPABILITIES_OFFSET 10

#define CHILD_ENTRY_SIZE 11

// ZigBee distinguishes between children that are routers and children that
// are leaves.
//
// All of these have been moved to sl_zigbee.h to make them visible to applications.
//
//extern uint8_t sli_zigbee_max_end_device_children;  // actual max for this network
//extern uint8_t sli_zigbee_end_device_child_count;   // how many we have
//extern uint8_t sli_zigbee_stack_profile;

// The child ID table is provided by the application via
// sl_zigbee_configuration.c.
// We pass a pointer to the child table to the unified mac layer which maintains
// child IDs and certain status flags.
#if defined(SL_ZIGBEE_LEAF_STACK) && !defined(SL_ZIGBEE_AF_NCP)
#define sl_zigbee_child_table_size (0)
#else
extern uint8_t sl_zigbee_child_table_size;
#endif
extern sl_mac_child_entry_t sli_zigbee_child_table_data[];

//Allocated in sl_zigbee_configuration.c
//There is a power value for each child which get updated during EBR and
//NETWORK_LINK_POWER_DELTA periodically
extern int8_t sli_zigbee_child_power[];

//----------------------------------------------------------------
// Child state (stack specific) information.
// We have 16 bits of per-child information in RAM.

#define CHILD_USING_SHORT_TIMEOUT       0x0008
#define CHILD_PENDING_APPLICATION_JIT   0x0010
#define CHILD_PENDING_SLEEPY_BROADCAST  0x0100
#define CHILD_PENDING_KEY_SWITCH        0x0200
#define CHILD_PENDING_NEW_ADDRESS       0x0400
#define CHILD_IS_USING_ACTIVE_KEY       0x1000
#define CHILD_IS_AUTHENTICATED          0x2000
#define CHILD_KEY_TO_BE_SENT            0x4000

// The commented out (deprecated) flags below are now maintained by unified mac
// #define CHILD_MAC_INDEX              0x0001
// #define CHILD_IS_END_DEVICE          0x0002
// #define CHILD_IS_SLEEPY              0x0004
// #define CHILD_PENDING_MAC_INDIRECT   0x0020
// #define CHILD_EXPECTING_JIT          0x0800

// Pick out all of the flags that indicate that a MAC indirect message is
// pending.
#define CHILD_HAS_PENDING_MESSAGE   \
  (CHILD_PENDING_APPLICATION_JIT    \
   | CHILD_PENDING_SLEEPY_BROADCAST \
   | CHILD_PENDING_NEW_ADDRESS      \
   | CHILD_PENDING_KEY_SWITCH)

// Allocated in sl_zigbee_configuration.c.
extern sli_child_status_flags_t sli_zigbee_child_status_data[];
extern uint16_t sli_zigbee_child_lqi_data[];

#define sli_zigbee_child_has_pending_message(childIndex)                    \
  ((sli_zigbee_child_status[(childIndex)] & CHILD_HAS_PENDING_MESSAGE) != 0 \
   || sli_mac_child_has_pending_message(childIndex))

#define sli_zigbee_child_status_flag(childIndex, mask) \
  (sli_zigbee_child_status[(childIndex)] & (mask))

#define sli_zigbee_set_child_status_flag(childIndex, mask) \
  (sli_zigbee_child_status[(childIndex)] |= (mask))

#define sli_zigbee_clear_child_status_flag(childIndex, mask) \
  (sli_zigbee_child_status[(childIndex)] &=                  \
     ((sli_child_status_flags_t) ~((sli_child_status_flags_t) mask)))

//update the lqi value in child table
void sli_zigbee_child_process_quality(sl_802154_short_addr_t childId);
// Set or clear 'mask' bits for all sleepy children.
bool sli_zigbee_set_all_sleepy_child_flags(sli_child_status_flags_t mask, bool set);

// True if any child as all of the given flags set.
bool sli_zigbee_any_child_has_flags_set(sli_child_status_flags_t mask);

// True if any sleepy child has this flag set.
#define sli_zigbee_is_sleepy_child_flag_set(mask) \
  (sli_zigbee_any_child_has_flags_set(mask)       \
   && !sli_mac_check_any_child_flags(SL_MAC_CHILD_IS_RX_ON_WHEN_IDLE))

// Allocated in sl_zigbee_configuration.c.
extern uint32_t sli_zigbee_child_timers_data[];

//----------------------------------------------------------------
// Reading and writing the security state.

#define sli_zigbee_child_is_using_active_network_key(childIndex) \
  (sli_zigbee_child_status_flag((childIndex), CHILD_IS_USING_ACTIVE_KEY))

#define sli_zigbee_child_set_using_active_network_key(childIndex) \
  (sli_zigbee_set_child_status_flag((childIndex), CHILD_IS_USING_ACTIVE_KEY))

#define sli_zigbee_child_set_using_alternate_network_key(childIndex) \
  (sli_zigbee_clear_child_status_flag((childIndex), CHILD_IS_USING_ACTIVE_KEY))

#define sli_zigbee_child_is_key_to_be_sent(childIndex) \
  (sli_zigbee_child_status_flag((childIndex), CHILD_KEY_TO_BE_SENT))

#define sli_zigbee_child_set_key_to_be_sent(childIndex) \
  (sli_zigbee_set_child_status_flag((childIndex), CHILD_KEY_TO_BE_SENT))

#define sli_zigbee_child_set_key_sent(childIndex) \
  (sli_zigbee_clear_child_status_flag((childIndex), CHILD_KEY_TO_BE_SENT))

#define sli_zigbee_child_is_authenticated(childIndex) \
  (sli_zigbee_child_status_flag((childIndex), CHILD_IS_AUTHENTICATED))

#define sli_zigbee_child_set_authenticated(childIndex) \
  (sli_zigbee_set_child_status_flag((childIndex), CHILD_IS_AUTHENTICATED))

#define sli_zigbee_child_set_unauthenticated(childIndex) \
  (sli_zigbee_clear_child_status_flag((childIndex), CHILD_IS_AUTHENTICATED))

// When a child associates to a parent, it will consume a child table entry
// regardless of whether it succeeds in decrypting the transport key. To prevent
// devices from consuming child entries for sli_zigbee_stack_end_device_poll_timeout time,
// we give the child a short timeout when associating, then we bump that timeout
// to sli_zigbee_stack_end_device_poll_timeout after we receive a valid NWK encrypted frame.
// This mitigates a failed joining device (or attacker device) from consuming a
// a child entry for a long time (had we just initialized the timeout to
// sli_zigbee_stack_end_device_poll_timeout to start with, which can be as long as 16384 mins)
#define sli_zigbee_child_is_using_short_timeout(childIndex) \
  (sli_zigbee_child_status_flag((childIndex), CHILD_USING_SHORT_TIMEOUT))

#define sli_zigbee_child_set_using_short_timeout(childIndex) \
  (sli_zigbee_set_child_status_flag((childIndex), CHILD_USING_SHORT_TIMEOUT))

#define sli_zigbee_child_clear_using_short_timeout(childIndex) \
  (sli_zigbee_clear_child_status_flag((childIndex), CHILD_USING_SHORT_TIMEOUT))

//----------------------------------------------------------------
// Adding and removing entries from the child table.
bool sli_zigbee_initialize_child(uint8_t mac_index,
                                 uint8_t index,
                                 sl_802154_short_addr_t id,
                                 sl_802154_long_addr_t eui64,
                                 uint8_t capabilities,
                                 uint8_t notifyOptions,
                                 uint8_t lqi);

sl_802154_short_addr_t sli_zigbee_add_rejoined_child(uint8_t mac_index,
                                                     sl_802154_long_addr_t eui64,
                                                     sl_802154_short_addr_t oldChildId,
                                                     uint8_t capabilities,
                                                     uint8_t notifyOptions,
                                                     uint8_t lqi);
#ifdef SL_ZIGBEE_TEST
// Handy utility for tests.

#define sli_zigbee_add_child(id, capabilities)                                         \
  (sli_zigbee_add_rejoined_child(0, (id), sli_zigbee_pick_random_id(), (capabilities), \
                                 CHILD_CHANGE_JOINING, 0))
#endif
void sli_zigbee_set_child_id_token(uint8_t childIndex, sl_802154_short_addr_t newId);
void sli_zigbee_erase_child(uint8_t childIndex);
void sli_zigbee_erase_child_table(void);
void sli_zigbee_erase_child_power_table(void);
void sli_zigbee_clear_child_frame_counter(uint8_t index);

#define CHILD_CHANGE_LEAVING            0x00
#define CHILD_CHANGE_JOINING            0x01
#define CHILD_CHANGE_NO_CALLBACK        0x02   // do not call sli_zigbee_stack_child_join_handler()
#define CHILD_CHANGE_USE_SHORT_TIMEOUT  0x04   // use short timeout until we receive nwk encrypted message

void sli_zigbee_note_child_change(uint8_t childIndex, uint8_t options);

// These are also defined in mac/command.h, but not everyone includes
// that and the router bit name defined there is not perspicuous.
#define ROUTER_CAPABILITY          0x02
#define RX_ON_WHEN_IDLE_CAPABILITY 0x08

uint8_t sli_zigbee_get_child_capabilities(uint8_t index);

// Restart the timeout clock for our child.
void sli_zigbee_note_successful_poll_received(uint8_t childIndex, uint8_t nwk_index);

// Restart the timeout clock for end devices.
void sli_zigbee_note_successful_poll(void);

// Just what it says.
uint32_t sli_zigbee_ms_since_last_successful_poll(void);

// Handy utility.
#define copyEui64(to, from)      memmove((to), (from), EUI64_SIZE)

void sli_zigbee_child_init(void);

// Used to disown ungrateful children who don't poll frequently enough.
void sli_zigbee_age_children(void);

// TODO: move this in sl_zigbee_multi_network.h
#if defined(SL_ZIGBEE_MULTI_NETWORK_STRIPPED)
#define sli_zigbee_age_children_all_networks() sli_zigbee_age_children()
#else
void sli_zigbee_age_children_all_networks(void);
#endif // defined(SL_ZIGBEE_MULTI_NETWORK_STRIPPED)

void sli_zigbee_set_all_children_to_alternate_key(void);

#ifdef SL_ZIGBEE_TEST
// Useful test routine.

sl_status_t sli_zigbee_remove_child(sl_802154_long_addr_t childEui64);
#endif

void checkAndRestoreChildTimeout(sl_802154_short_addr_t nodeId);

#endif // SILABS_ZIGBEE_CHILD_H
