/***************************************************************************//**
 * @file
 * @brief Stack API for transient key management.
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

#ifndef APS_KEYS_TRANSIENT_H
#define APS_KEYS_TRANSIENT_H

// -----------------------------------------------------------------------------
// Helpers
#define WILDCARD_EUI    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }

// These Macros are used to help filter searches of the Transient Key Table
// by limiting the search to specific bitmask fields
#define TRANSIENT_KEY_BITMASK_DEFAULT_FIELDS (SL_ZIGBEE_KEY_HAS_PARTNER_EUI64            \
                                              | SL_ZIGBEE_KEY_HAS_INCOMING_FRAME_COUNTER \
                                              | SL_ZIGBEE_KEY_HAS_OUTGOING_FRAME_COUNTER)

#define TRANSIENT_KEY_BITMASK_ALL_FIELDS 0xFFFF

// -----------------------------------------------------------------------------
// Storage

sl_status_t sli_zigbee_add_transient_link_key(const sl_zigbee_key_struct_t* keyStruct);
bool sli_zigbee_remove_transient_link_key(sl_802154_long_addr_t eui64ToFind,
                                          sl_zigbee_key_struct_bitmask_t* bitmask);

bool sli_zigbee_find_transient_link_key(sl_802154_long_addr_t eui64ToFind,
                                        sl_zigbee_key_struct_t* keyDataReturn,
                                        sl_zigbee_key_struct_bitmask_t* bitmask);
bool sli_zigbee_have_transient_link_keys(void);
// Returns number of transient keys in storage
uint16_t sli_zigbee_get_num_transient_keys(void);
// Searches for current wildcard key and duplicates the key, but tied to the
// specific EUI passed in. This is used when doing 'plugin network-creator-security
// open-network', which adds one (wildcard) key for all devices to join (using ZA09),
// but needs to replicate that key for each joining device's EUI
sl_status_t sli_zigbee_duplicate_wildcard_transient_key(sl_802154_long_addr_t eui);

/**
 * @brief refreshes the duration of a transient key by a number of seconds
 * @param device_long the EUI64 associated with the key
 * @param refresh_duration the amount of time in seconds to add to the current timeout
 */
void sli_zigbee_refresh_aps_transient_key_timeout_seconds(sl_802154_long_addr_t device_long, uint16_t refresh_duration);

#define SLI_TRANSIENT_DEVICE_REFRESH_AMOUNT sli_zigbee_stack_get_transient_key_timeout_s()
#define sli_zigbee_refresh_aps_transient_key_timout(device_id) \
  sli_zigbee_refresh_aps_transient_key_timeout_seconds((device_id), SLI_TRANSIENT_DEVICE_REFRESH_AMOUNT)

// -----------------------------------------------------------------------------
// State

// Used to timeout the transient link keys in RAM on the trust center.
void sli_zigbee_transient_link_key_event_handler(sli_zigbee_event_t* event);
extern sli_zigbee_event_t sli_zigbee_transient_link_key_events[];

// Note state.
extern bool sli_zigbee_request_link_key_update;
#define sli_zigbee_set_update_link_key_request(set) (sli_zigbee_request_link_key_update = (set))
#define sli_zigbee_update_link_key_request_sent()   (sli_zigbee_request_link_key_update)

// -----------------------------------------------------------------------------
// Handlers

void sli_zigbee_process_verify_key(sl_802154_short_addr_t source,
                                   uint8_t* frame,
                                   sli_zigbee_send_aps_command_options_t packetSecurity);

#endif // APS_KEYS_TRANSIENT_H
