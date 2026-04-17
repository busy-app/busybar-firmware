/***************************************************************************//**
 * @file
 * @brief This file contains APIs and settings for storing, ordering, parsing,
 *  and constructing beacons.
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SILABS_BEACON_HANDLING_H
#define SILABS_BEACON_HANDLING_H

#include "mac/mac-info-element-parsing.h"

//-----------------------------------------------------------------------------
// Externs

#ifdef MAC_TEST_COMMANDS_SUPPORT
extern bool useEnhancedBeacon;
extern bool useZigbeeBeaconPayload;
#endif  // MAC_TEST_COMMANDS_SUPPORT

//-----------------------------------------------------------------------------
// Internal (em prefixed) APIs

/** @brief Initializes beacon data structures and state variables. */
void sli_zigbee_beacon_handling_init(void);

/** @brief Handles receiving and responding to beacon requests. */
void sli_zigbee_process_incoming_beacon_request(sli_zigbee_packet_header_t header,
                                                sli_802154mac_frame_info_element_parse_result result,
                                                sli_802154mac_info_element_field infoElementsArray[EM_MAC_FRAME_MAX_INFO_ELEMENT_COUNT]);

/** @brief Handles receiving and responding to beacons. */
void sli_zigbee_process_incoming_beacon(sli_zigbee_packet_header_t header);

/** @brief Gets the beacon payload in the received sli_zigbee_packet_header_t buffer. */
uint8_t sli_zigbee_get_beacon_payload(sli_zigbee_packet_header_t beacon, uint8_t *returnPayload);

/** @brief Validates the fields of a beacon and returns true if so; false
   otherwise */
bool sli_zigbee_is_beacon_valid(sl_zigbee_beacon_data_t beacon, uint8_t* payload);

/** @brief Compares the passed in beacon argument to beacons stored in cache.
   The beacon argument is stored if it is either more prioritized than another
   stored beacon or if there is room in the cache. The cache size is set by the
   caller before initiating a scan. See ::sli_zigbee_stack_set_num_beacons_to_store. */
void sli_util_store_if_better_beacon(sl_zigbee_beacon_data_t beacon);

/** @brief The event that is armed for when to send a beacon. Beacon
   transmissions are jittered to avoid collisions over the air. */
void sli_zigbee_beacon_event_handler(sli_zigbee_event_t *event);

/** @brief Removes a beacon collected from a scan. Typically called by a joining
   routine after a failed join. */
void sli_zigbee_clear_stored_beacon(sl_zigbee_beacon_data_t beacon);

/** @brief Constructs a packet buffer with beacon payload. */
sli_buffer_manager_buffer_t sli_zigbee_make_beacon_payload(uint8_t mac_index);

#if defined(SL_ZIGBEE_TEST) || defined(SL_ZIGBEE_STACK_TEST_HARNESS)
/** @brief A test function to suppress beacons. */
void sli_zigbee_test_harness_beacon_suppress_set(uint8_t count);

/** @brief A test function to get the state of suppressed beacons. */
uint8_t sli_zigbee_test_harness_beacon_suppress_get(void);
#endif // defined(SL_ZIGBEE_TEST) || defined(SL_ZIGBEE_STACK_TEST_HARNESS)

#endif // SILABS_BEACON_HANDLING_H
