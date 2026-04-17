/***************************************************************************//**
 * @file pro_compliance_stack_interface_baremetal_wrapper.c
 * @brief internal implementations for 'pro_compliance_stack_interface' as a thin-wrapper
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
// automatically generated from pro_compliance_stack_interface.h.  Do not manually edit
#include "stack/include/pro_compliance_stack_interface.h"
#include "stack/internal/inc/pro_compliance_stack_interface_internal_def.h"

sl_802154_short_addr_t sl_mac_find_child_short_id(sl_802154_long_addr_t eui64)
{
  return sli_mac_stack_find_child_short_id(eui64);
}

uint64_t sl_mac_get_child_info_flags(uint8_t childIndex)
{
  return sli_mac_stack_get_child_info_flags(childIndex);
}

void sl_mac_indirect_purge(uint8_t nwk_index)
{
  sli_mac_stack_indirect_purge(nwk_index);
}

void sl_mac_kickstart(uint8_t mac_index)
{
  sli_mac_stack_kickstart(mac_index);
}

bool sl_mac_lower_mac_radio_is_on(uint8_t mac_index)
{
  return sli_mac_stack_lower_mac_radio_is_on(mac_index);
}

void sl_mac_lower_mac_radio_sleep(void)
{
  sli_mac_stack_lower_mac_radio_sleep();
}

void sl_mac_lower_mac_radio_wakeup(void)
{
  sli_mac_stack_lower_mac_radio_wakeup();
}

sl_status_t sl_mac_lower_mac_set_radio_channel(uint8_t mac_index,
                                               uint8_t channel)
{
  return sli_mac_stack_lower_mac_set_radio_channel(mac_index,
                                                   channel);
}

sl_status_t sl_mac_lower_mac_set_radio_idle_mode(uint8_t mac_index,
                                                 uint8_t mode)
{
  return sli_mac_stack_lower_mac_set_radio_idle_mode(mac_index,
                                                     mode);
}

sl_status_t sl_mac_lower_mac_set_radio_power(uint8_t mac_index,
                                             int8_t power)
{
  return sli_mac_stack_lower_mac_set_radio_power(mac_index,
                                                 power);
}

void sl_mac_set_coordinator(bool isCoordinator)
{
  sli_mac_stack_set_coordinator(isCoordinator);
}

sl_status_t sl_mac_test_associate_command(sl_802154_short_addr_t parentId,
                                          uint16_t panId)
{
  return sli_mac_stack_test_associate_command(parentId,
                                              panId);
}

void sl_mac_test_send_mac_command(uint8_t macCommandLength,
                                  uint8_t *macCommand)
{
  sli_mac_stack_test_send_mac_command(macCommandLength,
                                      macCommand);
}

sl_status_t sl_mac_test_set_nwk_radio_params_channel(uint8_t channel)
{
  return sli_mac_stack_test_set_nwk_radio_params_channel(channel);
}

sl_status_t sl_mac_test_set_nwk_radio_params_eui(uint8_t network_index,
                                                 sl_802154_long_addr_t eui)
{
  return sli_mac_stack_test_set_nwk_radio_params_eui(network_index,
                                                     eui);
}

sl_status_t sl_mac_test_set_tx_power(int8_t power)
{
  return sli_mac_stack_test_set_tx_power(power);
}

uint8_t sl_zigbee_bdb_tclk_max_exchange_attempts(void)
{
  return sli_zigbee_stack_bdb_tclk_max_exchange_attempts();
}

sl_status_t sl_zigbee_request_link_key_with_option_encrypt(sl_802154_long_addr_t partner,
                                                           uint8_t option)
{
  return sli_zigbee_stack_request_link_key_with_option_encrypt(partner,
                                                               option);
}

sl_status_t sl_zigbee_send_aps_ack(sl_zigbee_aps_frame_t apsStruct,
                                   sl_802154_short_addr_t dest)
{
  return sli_zigbee_stack_send_aps_ack(apsStruct,
                                       dest);
}

void sl_zigbee_set_end_device_poll_timeout(uint8_t timeout)
{
  sli_zigbee_stack_set_end_device_poll_timeout(timeout);
}

void sl_zigbee_set_eui64(sl_802154_long_addr_t eui64)
{
  sli_zigbee_stack_set_eui64(eui64);
}

void sl_zigbee_set_ignore_aps_acks(bool ignore)
{
  sli_zigbee_stack_set_ignore_aps_acks(ignore);
}

sl_status_t sl_zigbee_set_packet_validate_library_state(uint16_t state)
{
  return sli_zigbee_stack_set_packet_validate_library_state(state);
}

void sl_zigbee_set_pan_id(uint16_t panId)
{
  sli_zigbee_stack_set_pan_id(panId);
}

void sl_zigbee_set_pan_id_conflict_report(boolean set_value,
                                          uint8_t nwk_index)
{
  sli_zigbee_stack_set_pan_id_conflict_report(set_value,
                                              nwk_index);
}

void sl_zigbee_set_stack_compliance_revision(uint8_t revision)
{
  sli_zigbee_stack_set_stack_compliance_revision(revision);
}

void sl_zigbee_set_zdo_dlk_save_derived_key(bool value)
{
  sli_zigbee_stack_set_zdo_dlk_save_derived_key(value);
}

bool sl_zigbee_test_aps_key_in_sync(sl_802154_long_addr_t eui64)
{
  return sli_zigbee_stack_test_aps_key_in_sync(eui64);
}

sl_status_t sl_zigbee_test_aps_key_set_sync_status(sl_802154_long_addr_t eui64,
                                                   bool setSync)
{
  return sli_zigbee_stack_test_aps_key_set_sync_status(eui64,
                                                       setSync);
}

sl_status_t sl_zigbee_test_custom_send_security_challenge_request(sl_802154_short_addr_t destShort,
                                                                  sl_zigbee_sec_man_context_t *context,
                                                                  uint8_t cmdoptions)
{
  return sli_zigbee_stack_test_custom_send_security_challenge_request(destShort,
                                                                      context,
                                                                      cmdoptions);
}

sl_status_t sl_zigbee_test_ieee_address_request_to_target(sl_802154_short_addr_t discoveryNodeId,
                                                          bool reportKids,
                                                          uint8_t childStartIndex,
                                                          uint8_t sourceEndpoint,
                                                          sl_zigbee_aps_option_t options,
                                                          sl_802154_short_addr_t targetNodeIdOfRequest)
{
  return sli_zigbee_stack_test_ieee_address_request_to_target(discoveryNodeId,
                                                              reportKids,
                                                              childStartIndex,
                                                              sourceEndpoint,
                                                              options,
                                                              targetNodeIdOfRequest);
}

void sl_zigbee_test_join_list_add(uint8_t command,
                                  uint8_t *eui64List,
                                  uint8_t counts)
{
  sli_zigbee_stack_test_join_list_add(command,
                                      eui64List,
                                      counts);
}

void sl_zigbee_test_join_list_request(uint8_t startIndex)
{
  sli_zigbee_stack_test_join_list_request(startIndex);
}

bool sl_zigbee_test_network_send_command(sl_802154_short_addr_t destination,
                                         uint8_t *commandFrame,
                                         uint8_t length,
                                         bool tryToInsertLongDest,
                                         sl_802154_long_addr_t destinationEui)
{
  return sli_zigbee_stack_test_network_send_command(destination,
                                                    commandFrame,
                                                    length,
                                                    tryToInsertLongDest,
                                                    destinationEui);
}

void sl_zigbee_test_perform_raw_active_scan(uint32_t scanChannels,
                                            uint8_t scanDuration)
{
  sli_zigbee_stack_test_perform_raw_active_scan(scanChannels,
                                                scanDuration);
}

void sl_zigbee_test_reset_frame_counter(uint8_t mask)
{
  sli_zigbee_stack_test_reset_frame_counter(mask);
}

bool sl_zigbee_test_send_device_update(uint16_t newShortId,
                                       sl_802154_long_addr_t newLongId,
                                       bool apsEncryption,
                                       uint8_t deviceStatus)
{
  return sli_zigbee_stack_test_send_device_update(newShortId,
                                                  newLongId,
                                                  apsEncryption,
                                                  deviceStatus);
}

sl_status_t sl_zigbee_test_send_leave_request_command(uint16_t destId,
                                                      sl_802154_long_addr_t destEui)
{
  return sli_zigbee_stack_test_send_leave_request_command(destId,
                                                          destEui);
}

bool sl_zigbee_test_send_link_key(sl_802154_short_addr_t targetNodeId,
                                  sl_802154_long_addr_t targetEui64,
                                  uint8_t keyType,
                                  sl_zigbee_key_data_t *key,
                                  bool useApsEncryption)
{
  return sli_zigbee_stack_test_send_link_key(targetNodeId,
                                             targetEui64,
                                             keyType,
                                             key,
                                             useApsEncryption);
}

void sl_zigbee_test_send_network_rejoin_command(uint8_t cmd_id,
                                                sl_802154_long_addr_t longId,
                                                sl_802154_short_addr_t oldShortId,
                                                sl_802154_short_addr_t newShortId,
                                                bool useNwkSecurity,
                                                uint8_t status,
                                                bool reallySend)
{
  sli_zigbee_stack_test_send_network_rejoin_command(cmd_id,
                                                    longId,
                                                    oldShortId,
                                                    newShortId,
                                                    useNwkSecurity,
                                                    status,
                                                    reallySend);
}

void sl_zigbee_test_send_network_timeout_request(uint8_t requestedTimeoutValue)
{
  sli_zigbee_stack_test_send_network_timeout_request(requestedTimeoutValue);
}

void sl_zigbee_test_send_our_end_device_announcement(void)
{
  sli_zigbee_stack_test_send_our_end_device_announcement();
}

sl_status_t sl_zigbee_test_send_remove_device_command(uint16_t destId,
                                                      sl_802154_long_addr_t destEui,
                                                      sl_802154_long_addr_t deviceToRemoveEui,
                                                      bool sendNonEncrypted)
{
  return sli_zigbee_stack_test_send_remove_device_command(destId,
                                                          destEui,
                                                          deviceToRemoveEui,
                                                          sendNonEncrypted);
}

bool sl_zigbee_test_send_report_or_update(uint8_t command,
                                          uint8_t updateId,
                                          uint16_t panId)
{
  return sli_zigbee_stack_test_send_report_or_update(command,
                                                     updateId,
                                                     panId);
}

void sl_zigbee_test_send_route_error_payload(sl_802154_short_addr_t destination,
                                             sl_802154_short_addr_t target,
                                             uint8_t errorCode,
                                             uint8_t *payload,
                                             uint8_t payload_len)
{
  sli_zigbee_stack_test_send_route_error_payload(destination,
                                                 target,
                                                 errorCode,
                                                 payload,
                                                 payload_len);
}

void sl_zigbee_test_send_route_error_payload_no_network_encryption(sl_802154_short_addr_t destination,
                                                                   sl_802154_short_addr_t target,
                                                                   uint8_t errorCode,
                                                                   uint8_t *payload,
                                                                   uint8_t payload_len)
{
  sli_zigbee_stack_test_send_route_error_payload_no_network_encryption(destination,
                                                                       target,
                                                                       errorCode,
                                                                       payload,
                                                                       payload_len);
}

sl_status_t sl_zigbee_test_send_route_request_with_tlv(sl_802154_short_addr_t target)
{
  return sli_zigbee_stack_test_send_route_request_with_tlv(target);
}

void sl_zigbee_test_send_timeout_request(void)
{
  sli_zigbee_stack_test_send_timeout_request();
}

void sl_zigbee_test_set_network_tokens(uint8_t stackProfile,
                                       uint8_t nodeType,
                                       uint8_t channel,
                                       int8_t power,
                                       uint16_t nodeId,
                                       uint16_t panId,
                                       sl_802154_long_addr_t extendedPanId)
{
  sli_zigbee_stack_test_set_network_tokens(stackProfile,
                                           nodeType,
                                           channel,
                                           power,
                                           nodeId,
                                           panId,
                                           extendedPanId);
}

void sl_zigbee_test_spoof_device_announcement(uint16_t shortId,
                                              uint8_t *sourceEUI64,
                                              sl_802154_long_addr_t deviceAnnounceEui,
                                              uint8_t capabilities)
{
  sli_zigbee_stack_test_spoof_device_announcement(shortId,
                                                  sourceEUI64,
                                                  deviceAnnounceEui,
                                                  capabilities);
}

sl_status_t sl_zigbee_test_zdo_generate_clear_all_bindings_req(sl_802154_short_addr_t destination,
                                                               bool encrypt,
                                                               uint8_t *eui64_list,
                                                               uint8_t counts)
{
  return sli_zigbee_stack_test_zdo_generate_clear_all_bindings_req(destination,
                                                                   encrypt,
                                                                   eui64_list,
                                                                   counts);
}

sl_status_t sl_zigbee_test_zdo_generate_get_authentication_level_req(sl_802154_short_addr_t dest,
                                                                     sl_zigbee_aps_option_t aps_options,
                                                                     sl_802154_long_addr_t target)
{
  return sli_zigbee_stack_test_zdo_generate_get_authentication_level_req(dest,
                                                                         aps_options,
                                                                         target);
}

sl_status_t sl_zigbee_test_zdo_generate_security_decommission_req(sl_802154_short_addr_t destination,
                                                                  bool encrypt,
                                                                  uint8_t *eui64_list,
                                                                  uint8_t counts)
{
  return sli_zigbee_stack_test_zdo_generate_security_decommission_req(destination,
                                                                      encrypt,
                                                                      eui64_list,
                                                                      counts);
}

sl_status_t sl_zigbee_zigbee_remove_child(sl_802154_short_addr_t childId,
                                          uint8_t options)
{
  return sli_zigbee_stack_zigbee_remove_child(childId,
                                              options);
}

void slx_zigbee_gu_zdo_toggle_dlk(bool do_dlk,
                                  bool allow_anon_psk)
{
  slxi_zigbee_stack_gu_zdo_toggle_dlk(do_dlk,
                                      allow_anon_psk);
}

void slx_zigbee_ignore_incoming_aps_acks(bool ignore)
{
  slxi_zigbee_stack_ignore_incoming_aps_acks(ignore);
}
