/***************************************************************************//**
 * @file
 * @brief EZSP utility library. These functions are provided to
 * make it easier to port applications from the environment where the
 * Ember stack and application where on a single processor to an
 * environment where the stack is running on an Ember radio chip and
 * the application is running on a separte host processor and
 * utilizing the EZSP library.
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

#ifndef SILABS_EZSP_UTILS_H
#define SILABS_EZSP_UTILS_H

#include "sl_zigbee_dhc.h"


/** @description Defines config parameter incompatibilities between the
 *   host and node
 */
enum {
  EZSP_UTIL_INCOMPATIBLE_PROTOCOL_VERSION                   = 0x00000001,
  EZSP_UTIL_INCOMPATIBLE_STACK_ID                           = 0x00000002,
  EZSP_UTIL_INCOMPATIBLE_PACKET_BUFFER_HEAP_SIZE           = 0x00000008,
  EZSP_UTIL_INCOMPATIBLE_NEIGHBOR_TABLE_SIZE                = 0x00000010,
  EZSP_UTIL_INCOMPATIBLE_APS_UNICAST_MESSAGE_COUNT          = 0x00000020,
  EZSP_UTIL_INCOMPATIBLE_BINDING_TABLE_SIZE                 = 0x00000040,
  EZSP_UTIL_INCOMPATIBLE_ADDRESS_TABLE_SIZE                 = 0x00000080,
  EZSP_UTIL_INCOMPATIBLE_MULTICAST_TABLE_SIZE               = 0x00000100,
  EZSP_UTIL_INCOMPATIBLE_ROUTE_TABLE_SIZE                   = 0x00000200,
  EZSP_UTIL_INCOMPATIBLE_DISCOVERY_TABLE_SIZE               = 0x00000400,
  EZSP_UTIL_INCOMPATIBLE_BROADCAST_ALARM_DATA_SIZE          = 0x00000800,
  EZSP_UTIL_INCOMPATIBLE_UNICAST_ALARM_DATA_SIZE            = 0x00001000,
  EZSP_UTIL_INCOMPATIBLE_STACK_PROFILE                      = 0x00004000,
  EZSP_UTIL_INCOMPATIBLE_SECURITY_LEVEL                     = 0x00008000,
  EZSP_UTIL_INCOMPATIBLE_MAX_HOPS                           = 0x00040000,
  EZSP_UTIL_INCOMPATIBLE_MAX_END_DEVICE_CHILDREN            = 0x00080000,
  EZSP_UTIL_INCOMPATIBLE_INDIRECT_TRANSMISSION_TIMEOUT      = 0x00100000,
  EZSP_UTIL_INCOMPATIBLE_END_DEVICE_POLL_TIMEOUT            = 0x00200000,
  EZSP_UTIL_INCOMPATIBLE_FRAGMENT_WINDOW_SIZE               = 0x04000000,
  EZSP_UTIL_INCOMPATIBLE_FRAGMENT_DELAY_MS                  = 0x08000000,
  EZSP_UTIL_INCOMPATIBLE_KEY_TABLE_SIZE                     = 0x10000000,
  EZSP_UTIL_INCOMPATIBLE_NUMBER_OF_SUPPORTED_NETWORKS       = 0x20000000,
};

// Replacement for SOFTWARE_VERSION
extern uint16_t ezspUtilStackVersion;

// Replacement function for sl_zigbee_init - checks configuration, sets the
// stack profile, and registers endpoints.
sl_status_t sl_zigbee_ezsp_util_init(uint8_t serialPort);

void sl_zigbee_tick(void);

extern sl_802154_long_addr_t sli_802154mac_local_eui64;
// The ezsp util library keeps track of the local node's EUI64 and
// short id so that there isn't the need for communication with the
// Ember radio node every time these values are queried.
#define sl_zigbee_get_eui64() (sli_802154mac_local_eui64)
#define sl_zigbee_is_local_eui64(eui64) \
  (memcmp(eui64, sli_802154mac_local_eui64, EUI64_SIZE) == 0)

extern uint8_t sl_zigbee_temporary_binding_entries;

// For back-compatibility
#define sl_zigbee_ezsp_reset() sl_zigbee_ezsp_init()

/** @brief Broadcasts a request to set the identity of the network manager and
 * the active channel mask.  The mask is used when scanning
 *  for the network after missing a channel update.
 *
 * @param networkManager   The network address of the network manager.
 * @param activeChannels   The new active channel mask.
 *
 * @return An ::sl_status_t value.
 * - ::SL_STATUS_OK
 * - ::SL_STATUS_ALLOCATION_FAILED
 * - ::SL_STATUS_NETWORK_DOWN
 * - ::SL_STATUS_BUSY
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
sl_status_t sl_zigbee_set_network_manager_request(sl_802154_short_addr_t networkManager,
                                                  uint32_t activeChannels);
#else
#define sl_zigbee_set_network_manager_request(manager, channels)     \
  (sl_zigbee_energy_scan_request(SL_ZIGBEE_SLEEPY_BROADCAST_ADDRESS, \
                                 (channels),                         \
                                 0xFF,                               \
                                 (manager)))
#endif

/** @brief Broadcasts a request to change the channel.  This request may
 * only be sent by the current network manager.  There is a delay of
 * several seconds from receipt of the broadcast to changing the channel,
 * to allow time for the broadcast to propagate.
 *
 * @param channel  The channel to change to.
 *
 * @return An ::sl_status_t value.
 * - ::SL_STATUS_OK
 * - ::SL_STATUS_ALLOCATION_FAILED
 * - ::SL_STATUS_NETWORK_DOWN
 * - ::SL_STATUS_BUSY
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
sl_status_t sl_zigbee_channel_change_request(uint8_t channel)
#else
#define sl_zigbee_channel_change_request(channel)                    \
  (sl_zigbee_energy_scan_request(SL_ZIGBEE_SLEEPY_BROADCAST_ADDRESS, \
                                 BIT32(channel),                     \
                                 0xFE,                               \
                                 0))
#endif

/** @brief Function to set NWK layer outgoing frame counter
 *    (intended for device restoration purposes).  Caveats:
 *    - Can only be called before NetworkInit / FormNetwork / JoinNetwork,
 *      when sl_zigbee_network_state()==SL_ZIGBEE_NO_NETWORK.
 *    - This function should be called before ::sl_zigbee_set_initial_security_state,
 *      and the SL_ZIGBEE_NO_FRAME_COUNTER_RESET bitmask should be added to the
 *      initial security bitmask when ::emberSetInitialSecuritState is called.
 *    - If used in multi-network context, be sure to call
 *      ::sl_zigbee_set_current_network() prior to calling this function.
 *
 * @param desiredValue The desired outgoing NWK frame counter value.  This
 *   should needs to be less than MAX_INT32U_VALUE to ensure that rollover
 *   does not occur on the next encrypted transmission.
 *
 * @return ::SL_STATUS_OK if calling context is valid (sl_zigbee_network_state()
 *   == SL_ZIGBEE_NO_NETWORK) and desiredValue < MAX_INT32U_VALUE. Otherwise,
 *   ::SL_STATUS_INVALID_STATE.
 */
#if defined DOXYGEN_SHOULD_SKIP_THIS
sl_status_t sl_zigbee_set_outgoing_nwk_frame_counter(uint32_t* desiredValuePtr);
#else
  #define sl_zigbee_set_outgoing_nwk_frame_counter(desiredValuePtr) \
  sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_NWK_FRAME_COUNTER, sizeof(*desiredValuePtr), desiredValuePtr)
#endif

/** @brief Function to set APS layer outgoing frame counter for Trust Center
 *    Link Key (intended for device restoration purposes).  Caveats:
 *    - Can only be called before NetworkInit / FormNetwork / JoinNetwork,
 *      when sl_zigbee_network_state()==SL_ZIGBEE_NO_NETWORK.
 *    - This function should be called before ::sl_zigbee_set_initial_security_state,
 *      and the SL_ZIGBEE_NO_FRAME_COUNTER_RESET bitmask should be added to the
 *      initial security bitmask when ::emberSetInitialSecuritState is called.
 *    - If used in multi-network context, be sure to call
 *      ::sl_zigbee_set_current_network() prior to calling this function.
 *
 * @param desiredValue The desired outgoing APS frame counter value.  This
 *   should needs to be less than MAX_INT32U_VALUE to ensure that rollover
 *   does not occur on the next encrypted transmission.
 *
 * @return ::SL_STATUS_OK if calling context is valid (sl_zigbee_network_state()
 *   == SL_ZIGBEE_NO_NETWORK) and desiredValue < MAX_INT32U_VALUE. Otherwise,
 *   ::SL_STATUS_INVALID_STATE.
 */
#if defined DOXYGEN_SHOULD_SKIP_THIS
sl_status_t sl_zigbee_set_outgoing_aps_frame_counter(uint32_t* desiredValuePtr);
#else
  #define sl_zigbee_set_outgoing_aps_frame_counter(desiredValuePtr) \
  sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_APS_FRAME_COUNTER, sizeof(*desiredValuePtr), desiredValuePtr)
#endif

#endif // __EZSP_UTILS_H__
