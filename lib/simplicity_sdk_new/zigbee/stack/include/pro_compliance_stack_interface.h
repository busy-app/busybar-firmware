/***************************************************************************//**
 * @file pro_compliance_stack_interface.h
 * @brief Header for zigbee_pro_compliance specific APIs that need to cross
 * the IPC layer.  Intended for internal purposes; the implementations of these
 * APIs are not packaged, while this is currently packaged only because the IPC
 * currently needs to include every header that contributes any command to it.
 *
 * The purpose of this header is to allow zigbee_pro_compliance to split up its code
 * into CLI/app and stack contexts; internal APIs that should run in stack
 * context are accessed via the interface in this header.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef PRO_COMPLIANCE_STACK_INTERFACE_H
#define PRO_COMPLIANCE_STACK_INTERFACE_H

#include PLATFORM_HEADER
#include "sl_status.h"
#include "sl_zigbee.h"

sl_status_t sl_zigbee_zigbee_remove_child(sl_802154_short_addr_t childId, uint8_t options);

sl_status_t sl_zigbee_set_packet_validate_library_state(uint16_t state);

sl_status_t sl_zigbee_test_send_remove_device_command(uint16_t destId,
                                                      sl_802154_long_addr_t destEui,
                                                      sl_802154_long_addr_t deviceToRemoveEui,
                                                      bool sendNonEncrypted);

sl_status_t sl_zigbee_test_send_leave_request_command(uint16_t destId,
                                                      sl_802154_long_addr_t destEui);

void sl_zigbee_test_send_network_rejoin_command(uint8_t cmd_id,
                                                sl_802154_long_addr_t longId,
                                                sl_802154_short_addr_t oldShortId,
                                                sl_802154_short_addr_t newShortId,
                                                bool useNwkSecurity,
                                                uint8_t status,
                                                bool reallySend);

void sl_zigbee_test_send_route_error_payload_no_network_encryption(sl_802154_short_addr_t destination,
                                                                   sl_802154_short_addr_t target,
                                                                   uint8_t errorCode,
                                                                   uint8_t *payload,
                                                                   uint8_t payload_len);

void sl_zigbee_test_send_timeout_request(void);

bool sl_zigbee_test_send_report_or_update(uint8_t command,
                                          uint8_t updateId,
                                          uint16_t panId);

void sl_zigbee_test_send_network_timeout_request(uint8_t requestedTimeoutValue);

void sl_zigbee_test_spoof_device_announcement(uint16_t shortId,
                                              uint8_t *sourceEUI64,
                                              sl_802154_long_addr_t deviceAnnounceEui,
                                              uint8_t capabilities);

bool sl_zigbee_test_network_send_command(sl_802154_short_addr_t destination,
                                         uint8_t *commandFrame,
                                         uint8_t length,
                                         bool tryToInsertLongDest,
                                         sl_802154_long_addr_t destinationEui);

sl_status_t sl_zigbee_test_ieee_address_request_to_target(sl_802154_short_addr_t discoveryNodeId,
                                                          bool reportKids,
                                                          uint8_t childStartIndex,
                                                          uint8_t sourceEndpoint,
                                                          sl_zigbee_aps_option_t options,
                                                          sl_802154_short_addr_t targetNodeIdOfRequest);

void sl_zigbee_test_send_our_end_device_announcement(void);

void sl_zigbee_test_send_route_error_payload(sl_802154_short_addr_t destination,
                                             sl_802154_short_addr_t target,
                                             uint8_t errorCode,
                                             uint8_t *payload,
                                             uint8_t payload_len);

// Sends a route request for target and appends fixed TLVs to RR
sl_status_t sl_zigbee_test_send_route_request_with_tlv(sl_802154_short_addr_t target);

void sl_zigbee_set_end_device_poll_timeout(uint8_t timeout);

void sl_zigbee_set_pan_id(uint16_t panId);

bool sl_zigbee_test_send_link_key(sl_802154_short_addr_t targetNodeId,
                                  sl_802154_long_addr_t targetEui64,
                                  uint8_t keyType,
                                  sl_zigbee_key_data_t* key,
                                  bool useApsEncryption);

void sl_zigbee_test_set_network_tokens(uint8_t stackProfile,
                                       uint8_t nodeType,
                                       uint8_t channel,
                                       int8_t power,
                                       uint16_t nodeId,
                                       uint16_t panId,
                                       sl_802154_long_addr_t extendedPanId);

void sl_zigbee_test_reset_frame_counter(uint8_t mask);

bool sl_zigbee_test_send_device_update(uint16_t newShortId,
                                       sl_802154_long_addr_t newLongId,
                                       bool apsEncryption,
                                       uint8_t deviceStatus);

void sl_zigbee_test_join_list_request(uint8_t startIndex);

/**
 * @internal SL_ZIGBEE_IPC_ARGS
 * {# eui64List | length: counts * EUI64_SIZE | max: MAX_IPC_VEC_ARG_CAPACITY #}
 */
void sl_zigbee_test_join_list_add(uint8_t command,
                                  uint8_t* eui64List,
                                  uint8_t counts);

//R23 functionality
void sl_zigbee_set_pan_id_conflict_report(boolean set_value,
                                          uint8_t nwk_index);

sl_status_t sl_zigbee_test_custom_send_security_challenge_request(sl_802154_short_addr_t destShort,
                                                                  sl_zigbee_sec_man_context_t *context,
                                                                  uint8_t cmdoptions);

bool sl_zigbee_test_aps_key_in_sync(sl_802154_long_addr_t eui64);

sl_status_t sl_zigbee_test_aps_key_set_sync_status(sl_802154_long_addr_t eui64,
                                                   bool setSync);

//keep this out of doxygen, but IPC generator needs this logic
/**
 * @internal SL_ZIGBEE_IPC_ARGS
 * {# eui64_list | length: counts * EUI64_SIZE | max: MAX_IPC_VEC_ARG_CAPACITY #}
 */
sl_status_t sl_zigbee_test_zdo_generate_clear_all_bindings_req(sl_802154_short_addr_t destination,
                                                               bool encrypt,
                                                               uint8_t* eui64_list,
                                                               uint8_t counts);

sl_status_t sl_zigbee_test_zdo_generate_get_authentication_level_req(sl_802154_short_addr_t dest,
                                                                     sl_zigbee_aps_option_t aps_options,
                                                                     sl_802154_long_addr_t target);

/**
 * @internal SL_ZIGBEE_IPC_ARGS
 * {# eui64_list | length: counts * EUI64_SIZE | max: MAX_IPC_VEC_ARG_CAPACITY #}
 */
sl_status_t sl_zigbee_test_zdo_generate_security_decommission_req(sl_802154_short_addr_t destination,
                                                                  bool encrypt,
                                                                  uint8_t* eui64_list,
                                                                  uint8_t counts);

void slx_zigbee_gu_zdo_toggle_dlk(bool do_dlk, bool allow_anon_psk);

void sl_zigbee_set_eui64(sl_802154_long_addr_t eui64);

void sl_zigbee_set_zdo_dlk_save_derived_key(bool value);

void sl_zigbee_set_ignore_aps_acks(bool ignore);

uint8_t sl_zigbee_bdb_tclk_max_exchange_attempts(void);

void sl_zigbee_set_stack_compliance_revision(uint8_t revision);

// MAC-related functionality
bool sl_mac_lower_mac_radio_is_on(uint8_t mac_index);

sl_status_t sl_mac_lower_mac_set_radio_channel(uint8_t mac_index, uint8_t channel);

sl_status_t sl_mac_lower_mac_set_radio_power(uint8_t mac_index, int8_t power);

//mode is RadioPowerMode enum (uint8_t), but IPC code doesn't generally deal well with
//internal type declarations
sl_status_t sl_mac_lower_mac_set_radio_idle_mode(uint8_t mac_index, uint8_t mode);

void sl_mac_lower_mac_radio_sleep(void);

void sl_mac_lower_mac_radio_wakeup(void);

sl_802154_short_addr_t sl_mac_find_child_short_id(sl_802154_long_addr_t eui64);

void sl_mac_set_coordinator(bool isCoordinator);

uint64_t sl_mac_get_child_info_flags(uint8_t childIndex);

/**
 * @internal SL_ZIGBEE_IPC_ARGS
 * {# macCommand | length: macCommandLength | max: MAX_IPC_VEC_ARG_CAPACITY #}
 */
void sl_mac_test_send_mac_command(uint8_t macCommandLength, uint8_t* macCommand);

void sl_mac_kickstart(uint8_t mac_index);

void sl_mac_indirect_purge(uint8_t nwk_index);

void sl_zigbee_test_perform_raw_active_scan(uint32_t scanChannels, uint8_t scanDuration);

sl_status_t sl_mac_test_set_tx_power(int8_t power);

sl_status_t sl_mac_test_set_nwk_radio_params_channel(uint8_t channel);

sl_status_t sl_mac_test_set_nwk_radio_params_eui(uint8_t network_index, sl_802154_long_addr_t eui);

sl_status_t sl_mac_test_associate_command(sl_802154_short_addr_t parentId, uint16_t panId);

sl_status_t sl_zigbee_request_link_key_with_option_encrypt(sl_802154_long_addr_t partner, uint8_t option);

sl_status_t sl_zigbee_send_aps_ack(sl_zigbee_aps_frame_t apsStruct, sl_802154_short_addr_t dest);

void slx_zigbee_ignore_incoming_aps_acks(bool ignore);
#endif //PRO_COMPLIANCE_STACK_INTERFACE_H
