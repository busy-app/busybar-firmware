/***************************************************************************//**
 * @file
 * @brief Initializing, aging, and searching route and discovery tables.
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

#ifndef ROUTE_TABLE_H
#define ROUTE_TABLE_H

//------------------------------------------------------------------------------
// Route table

extern const sl_zigbee_library_status_t sli_zigbee_pro_library_status;

// Unused entries in the route table have destination SL_ZIGBEE_NULL_NODE_ID.

// The route table is provided by the application via sl_zigbee_configuration.c.
extern sli_zigbee_route_table_entry_t sli_zigbee_route_table[];

// The status field of a sli_zigbee_route_table_entry_t is a bitfield.
#define ROUTE_STATUS_MASK           ((uint8_t)(BIT(0) | BIT(1)))
#define ROUTE_RECORD_STATE_MASK     ((uint8_t)(BIT(2) | BIT(3)))
#define ROUTE_RECORD_STATE_SHIFT         2
#define ROUTE_AGGREGATOR_TYPE_MASK  ((uint8_t)(BIT(4) | BIT(5)))
#define ROUTE_AGGREGATOR_TYPE_SHIFT      4
#define ROUTE_AGE_MASK              ((uint8_t)(BIT(6) | BIT(7)))
#define ROUTE_AGE_SHIFT                  6

#define sli_zigbee_route_status(entry) \
  ((entry)->status & ROUTE_STATUS_MASK)
#define sli_zigbee_route_aggregator_type(entry) \
  ((entry)->status & ROUTE_AGGREGATOR_TYPE_MASK)
#define sli_zigbee_route_record_state(entry) \
  ((entry)->status & ROUTE_RECORD_STATE_MASK)
#define sli_zigbee_route_age(entry) \
  ((entry)->status >> ROUTE_AGE_SHIFT)

#define clearRouteAge(entry) \
  ((entry)->status &= ~ROUTE_AGE_MASK)
#define setRouteAge(entry, newAge) \
  ((entry)->status = ((entry)->status & ~ROUTE_AGE_MASK) | ((newAge) << ROUTE_AGE_SHIFT))
#define setRouteStatus(entry, newStatus) \
  ((entry)->status = ((entry)->status & ~ROUTE_STATUS_MASK) | (newStatus))
#define setRouteAggregatorType(entry, newType) \
  ((entry)->status = ((entry)->status & ~ROUTE_AGGREGATOR_TYPE_MASK) | (newType))
#define setRouteRecordState(entry, newState) \
  ((entry)->status = ((entry)->status & ~ROUTE_RECORD_STATE_MASK) | (newState))
#define setRouteStatusBitfields(entry, aggType, record, newStatus) \
  ((entry)->status = ((aggType) | (record) | (newStatus)))

// Length of a route age tick.
// Powers of 2 are used for cheap computation.
#define LOG_ROUTE_AGE_INCREMENT_SEC 5 // 32 seconds
#define LOG_ROUTE_AGE_INCREMENT_QS (LOG_ROUTE_AGE_INCREMENT_SEC + 2)
#define ROUTE_AGE_INCREMENT_QS (BIT(LOG_ROUTE_AGE_INCREMENT_QS))
#define ROUTE_AGE_INCREMENT_QS_MASK (ROUTE_AGE_INCREMENT_QS - 1)

// The maximum age of a route table entry, in units of ROUTE_AGE_INCREMENT.
// This value is used to indicate a stale route entry.  It cannot exceed
// 3 since it is stored using 2 bits.
#define MAX_ROUTE_AGE 2           // 64 seconds (minus up to 32 seconds)

// The route table status values are as they appear in the Zigbee spec
// (not that it matters since they don't go over the air).
// We don't use values DISCOVERY_FAILED (2) and VALIDATING (4).
enum {
  ROUTE_ACTIVE           = 0,
  ROUTE_DISCOVERY        = 1,
  ROUTE_UNUSED           = 3
};
typedef uint8_t sli_route_table_status_t;

// NOTE:  These are *not* the same as the over-the-air bits.
#define NOT_AN_AGGREGATOR   0x00
#define HIGH_RAM_AGGREGATOR 0x20
#define LOW_RAM_AGGREGATOR  0x10

typedef uint8_t sli_route_aggregator_type_t;

// Upon receiving a many-to-one route request from a high ram
// concentrator, the route record field is set to ROUTE_RECORD_NEEDED.
// The route record is sent the next time the route is used, and
// the field is set to ROUTE_RECORD_SENT.  Then upon receipt
// of a source route from the concentrator, the field is set to
// ROUTE_RECORD_CLEAR.

#define ROUTE_RECORD_CLEAR  0x00
#define ROUTE_RECORD_SENT   0x04
#define ROUTE_RECORD_NEEDED 0x08

//------------------------------------------------------------------------------
// Discovery table

// Unused entries in the discovery table have SL_ZIGBEE_NULL_NODE_ID as the source
// and 0 as the id.
extern sli_zigbee_discovery_table_entry_t sli_zigbee_discovery_table[];
extern uint8_t sli_zigbee_discovery_table_size;

//------------------------------------------------------------------------------
// Public functions

void sli_zigbee_initialize_table_routing(uint8_t nwkIndex);

uint8_t sli_zigbee_get_route_table_entry_index(sli_zigbee_route_table_entry_t *entry);

sli_zigbee_route_table_entry_t *sli_zigbee_find_route_table_entry(sl_802154_short_addr_t destination);

void sli_zigbee_delete_route_table_entry(sl_802154_short_addr_t destination);

void sli_zigbee_initialize_route_table_entry(sli_zigbee_route_table_entry_t *entry);

void sli_zigbee_set_route_status_and_zero_age(sli_zigbee_route_table_entry_t *entry,
                                              sli_route_table_status_t status);

// searchType controls what is done with id:
// 0 - ignore it
// 1 - match with search ID
// 2 - match with destination ID
sli_zigbee_discovery_table_entry_t *sli_zigbee_search_discovery_table(sl_802154_short_addr_t source,
                                                                      uint16_t id,
                                                                      uint8_t searchType);

sli_zigbee_discovery_table_entry_t *sli_zigbee_find_empty_discovery_table_entry(void);

#define sli_zigbee_is_discovery_underway(destination, originator) \
  (sli_zigbee_search_discovery_table((originator), (destination), 2) != NULL)

#define sli_zigbee_find_discovery_table_entry(source, id) \
  (sli_zigbee_search_discovery_table((source), (id), 1))

sli_zigbee_route_table_entry_t *sli_zigbee_find_or_create_route_table_entry(sl_802154_short_addr_t dest,
                                                                            sli_route_aggregator_type_t aggType,
                                                                            uint8_t *returnIndex);

// If there is no usable next hop this returns EMBER_BROADCAST_ID
// if route discovery has been initiated and SL_ZIGBEE_NULL_NODE_ID if not.

sl_802154_short_addr_t sli_zigbee_route_table_next_hop(sl_802154_short_addr_t destination,
                                                       uint8_t *statusReturn,
                                                       sli_zigbee_route_table_entry_t **returnEntry);

void sli_zigbee_deactivate_or_delete_active_route(sl_802154_short_addr_t destination);

// Used in ID assignment to check if a randomly chosen ID is already in use.
bool sli_zigbee_is_node_in_routing_tables(sl_802154_short_addr_t node);

// Used to keep next hop nodes from being evicted from the neighbor table.
bool sli_zigbee_in_use_as_next_hop(sl_802154_short_addr_t node);

bool sli_zigbee_is_concentrator(sl_802154_short_addr_t id);

void sli_zigbee_age_route_tables(void);

#if defined(SL_ZIGBEE_TEST) || defined(PRO_COMPLIANCE_LITE)

// For testing it is useful to be able to inject entries into the route table.
bool sli_zigbee_add_route_entry(sl_802154_short_addr_t destination,
                                sl_802154_short_addr_t nextHop,
                                sli_route_table_status_t status,
                                uint8_t age);

bool sli_zigbee_update_route_entry(sl_802154_short_addr_t destination,
                                   sl_802154_short_addr_t nextHop,
                                   sli_route_table_status_t status,
                                   uint8_t age);

extern char *routeStatusNames[];

void sli_zigbee_print_route_table(void);

#endif // SL_ZIGBEE_TEST / PRO_COMPLIANCE_LITE

//------------------------------------------------------------------------------
// Route record table (added in EMSTACK-2022): stores for each child whether a
// parent should send a route record in its behalf.

// For each entry in the route table we maintain a bitmask of
// ((sl_zigbee_child_table_size + 7) / 8) bytes. We account for 7 extra bits to ensure
// the minimum required size, given the potential truncation of the division by
// 8. Each bitmask is defined as an "entry" in the route record table.

void sli_zigbee_route_record_table_clear_child_flags(uint8_t childIndex);
void sli_zigbee_route_record_table_set_or_clear_child_flag(uint8_t routeIndex,
                                                           uint8_t childIndex,
                                                           bool isSet);
bool sli_zigbee_route_record_table_get_child_flag(uint8_t routeIndex, uint8_t childIndex);

extern uint8_t sli_zigbee_route_record_table_data[];
extern uint8_t sli_zigbee_route_table_size;
extern uint8_t sli_zigbee_tree_depth;

#define sli_zigbee_route_record_table_entry_size() \
  ((sl_zigbee_child_table_size + 7) >> 3)

#define sli_zigbee_route_record_table_size() \
  (sli_zigbee_route_table_size)

#define sli_zigbee_get_route_record_table_entry(routeIndex) \
  (sli_zigbee_route_record_table + (routeIndex) * sli_zigbee_route_record_table_entry_size())

#define sli_zigbee_init_route_record_table_entry(routeIndex)    \
  do {                                                          \
    memset(sli_zigbee_get_route_record_table_entry(routeIndex), \
           0x00,                                                \
           sli_zigbee_route_record_table_entry_size());         \
  } while (0)

#define sli_zigbee_route_record_table_set_child_flag(routeIndex, childIndex)             \
  do {                                                                                   \
    sli_zigbee_route_record_table_set_or_clear_child_flag(routeIndex, childIndex, true); \
  } while (0)

#define sli_zigbee_route_record_table_clear_child_flag(routeIndex, childIndex)            \
  do {                                                                                    \
    sli_zigbee_route_record_table_set_or_clear_child_flag(routeIndex, childIndex, false); \
  } while (0)

  #endif // ROUTE_TABLE_H
