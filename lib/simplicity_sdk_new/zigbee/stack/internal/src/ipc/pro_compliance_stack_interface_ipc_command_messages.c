/***************************************************************************//**
 * @file pro_compliance_stack_interface_ipc_command_messages.c
 * @brief internal wrappers for 'pro_compliance_stack_interface' ipc commands
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
#include "stack/internal/src/ipc/pro_compliance_stack_interface_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_mac_stack_find_child_short_id_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.find_child_short_id.response.result = sli_mac_stack_find_child_short_id(msg->data.find_child_short_id.request.eui64);
}

void sli_mac_stack_get_child_info_flags_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_child_info_flags.response.result = sli_mac_stack_get_child_info_flags(msg->data.get_child_info_flags.request.childIndex);
}

void sli_mac_stack_indirect_purge_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_mac_stack_indirect_purge(msg->data.indirect_purge.request.nwk_index);
}

void sli_mac_stack_kickstart_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_mac_stack_kickstart(msg->data.kickstart.request.mac_index);
}

void sli_mac_stack_lower_mac_radio_is_on_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.lower_mac_radio_is_on.response.result = sli_mac_stack_lower_mac_radio_is_on(msg->data.lower_mac_radio_is_on.request.mac_index);
}

void sli_mac_stack_lower_mac_radio_sleep_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_mac_stack_lower_mac_radio_sleep();
}

void sli_mac_stack_lower_mac_radio_wakeup_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_mac_stack_lower_mac_radio_wakeup();
}

void sli_mac_stack_lower_mac_set_radio_channel_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.lower_mac_set_radio_channel.response.result = sli_mac_stack_lower_mac_set_radio_channel(msg->data.lower_mac_set_radio_channel.request.mac_index,
                                                                                                    msg->data.lower_mac_set_radio_channel.request.channel);
}

void sli_mac_stack_lower_mac_set_radio_idle_mode_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.lower_mac_set_radio_idle_mode.response.result = sli_mac_stack_lower_mac_set_radio_idle_mode(msg->data.lower_mac_set_radio_idle_mode.request.mac_index,
                                                                                                        msg->data.lower_mac_set_radio_idle_mode.request.mode);
}

void sli_mac_stack_lower_mac_set_radio_power_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.lower_mac_set_radio_power.response.result = sli_mac_stack_lower_mac_set_radio_power(msg->data.lower_mac_set_radio_power.request.mac_index,
                                                                                                msg->data.lower_mac_set_radio_power.request.power);
}

void sli_mac_stack_set_coordinator_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_mac_stack_set_coordinator(msg->data.set_coordinator.request.isCoordinator);
}

void sli_mac_stack_test_associate_command_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.test_associate_command.response.result = sli_mac_stack_test_associate_command(msg->data.test_associate_command.request.parentId,
                                                                                          msg->data.test_associate_command.request.panId);
}

void sli_mac_stack_test_send_mac_command_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_mac_stack_test_send_mac_command(msg->data.test_send_mac_command.request.macCommandLength,
                                      msg->data.test_send_mac_command.request.macCommand);
}

void sli_mac_stack_test_set_nwk_radio_params_channel_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.test_set_nwk_radio_params_channel.response.result = sli_mac_stack_test_set_nwk_radio_params_channel(msg->data.test_set_nwk_radio_params_channel.request.channel);
}

void sli_mac_stack_test_set_nwk_radio_params_eui_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.test_set_nwk_radio_params_eui.response.result = sli_mac_stack_test_set_nwk_radio_params_eui(msg->data.test_set_nwk_radio_params_eui.request.network_index,
                                                                                                        msg->data.test_set_nwk_radio_params_eui.request.eui);
}

void sli_mac_stack_test_set_tx_power_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.test_set_tx_power.response.result = sli_mac_stack_test_set_tx_power(msg->data.test_set_tx_power.request.power);
}

void sli_zigbee_stack_bdb_tclk_max_exchange_attempts_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.bdb_tclk_max_exchange_attempts.response.result = sli_zigbee_stack_bdb_tclk_max_exchange_attempts();
}

void sli_zigbee_stack_request_link_key_with_option_encrypt_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.request_link_key_with_option_encrypt.response.result = sli_zigbee_stack_request_link_key_with_option_encrypt(msg->data.request_link_key_with_option_encrypt.request.partner,
                                                                                                                         msg->data.request_link_key_with_option_encrypt.request.option);
}

void sli_zigbee_stack_send_aps_ack_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.send_aps_ack.response.result = sli_zigbee_stack_send_aps_ack(msg->data.send_aps_ack.request.apsStruct,
                                                                         msg->data.send_aps_ack.request.dest);
}

void sli_zigbee_stack_set_end_device_poll_timeout_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_end_device_poll_timeout(msg->data.set_end_device_poll_timeout.request.timeout);
}

void sli_zigbee_stack_set_eui64_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_eui64(msg->data.set_eui64.request.eui64);
}

void sli_zigbee_stack_set_ignore_aps_acks_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_ignore_aps_acks(msg->data.set_ignore_aps_acks.request.ignore);
}

void sli_zigbee_stack_set_packet_validate_library_state_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_packet_validate_library_state.response.result = sli_zigbee_stack_set_packet_validate_library_state(msg->data.set_packet_validate_library_state.request.state);
}

void sli_zigbee_stack_set_pan_id_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_pan_id(msg->data.set_pan_id.request.panId);
}

void sli_zigbee_stack_set_pan_id_conflict_report_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_pan_id_conflict_report(msg->data.set_pan_id_conflict_report.request.set_value,
                                              msg->data.set_pan_id_conflict_report.request.nwk_index);
}

void sli_zigbee_stack_set_stack_compliance_revision_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_stack_compliance_revision(msg->data.set_stack_compliance_revision.request.revision);
}

void sli_zigbee_stack_set_zdo_dlk_save_derived_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_zdo_dlk_save_derived_key(msg->data.set_zdo_dlk_save_derived_key.request.value);
}

void sli_zigbee_stack_test_aps_key_in_sync_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.test_aps_key_in_sync.response.result = sli_zigbee_stack_test_aps_key_in_sync(msg->data.test_aps_key_in_sync.request.eui64);
}

void sli_zigbee_stack_test_aps_key_set_sync_status_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.test_aps_key_set_sync_status.response.result = sli_zigbee_stack_test_aps_key_set_sync_status(msg->data.test_aps_key_set_sync_status.request.eui64,
                                                                                                         msg->data.test_aps_key_set_sync_status.request.setSync);
}

void sli_zigbee_stack_test_custom_send_security_challenge_request_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.test_custom_send_security_challenge_request.response.result = sli_zigbee_stack_test_custom_send_security_challenge_request(msg->data.test_custom_send_security_challenge_request.request.destShort,
                                                                                                                                       &msg->data.test_custom_send_security_challenge_request.request.context,
                                                                                                                                       msg->data.test_custom_send_security_challenge_request.request.cmdoptions);
}

void sli_zigbee_stack_test_ieee_address_request_to_target_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.test_ieee_address_request_to_target.response.result = sli_zigbee_stack_test_ieee_address_request_to_target(msg->data.test_ieee_address_request_to_target.request.discoveryNodeId,
                                                                                                                       msg->data.test_ieee_address_request_to_target.request.reportKids,
                                                                                                                       msg->data.test_ieee_address_request_to_target.request.childStartIndex,
                                                                                                                       msg->data.test_ieee_address_request_to_target.request.sourceEndpoint,
                                                                                                                       msg->data.test_ieee_address_request_to_target.request.options,
                                                                                                                       msg->data.test_ieee_address_request_to_target.request.targetNodeIdOfRequest);
}

void sli_zigbee_stack_test_join_list_add_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_test_join_list_add(msg->data.test_join_list_add.request.command,
                                      msg->data.test_join_list_add.request.eui64List,
                                      msg->data.test_join_list_add.request.counts);
}

void sli_zigbee_stack_test_join_list_request_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_test_join_list_request(msg->data.test_join_list_request.request.startIndex);
}

void sli_zigbee_stack_test_network_send_command_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.test_network_send_command.response.result = sli_zigbee_stack_test_network_send_command(msg->data.test_network_send_command.request.destination,
                                                                                                   &msg->data.test_network_send_command.request.commandFrame,
                                                                                                   msg->data.test_network_send_command.request.length,
                                                                                                   msg->data.test_network_send_command.request.tryToInsertLongDest,
                                                                                                   msg->data.test_network_send_command.request.destinationEui);
}

void sli_zigbee_stack_test_perform_raw_active_scan_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_test_perform_raw_active_scan(msg->data.test_perform_raw_active_scan.request.scanChannels,
                                                msg->data.test_perform_raw_active_scan.request.scanDuration);
}

void sli_zigbee_stack_test_reset_frame_counter_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_test_reset_frame_counter(msg->data.test_reset_frame_counter.request.mask);
}

void sli_zigbee_stack_test_send_device_update_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.test_send_device_update.response.result = sli_zigbee_stack_test_send_device_update(msg->data.test_send_device_update.request.newShortId,
                                                                                               msg->data.test_send_device_update.request.newLongId,
                                                                                               msg->data.test_send_device_update.request.apsEncryption,
                                                                                               msg->data.test_send_device_update.request.deviceStatus);
}

void sli_zigbee_stack_test_send_leave_request_command_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.test_send_leave_request_command.response.result = sli_zigbee_stack_test_send_leave_request_command(msg->data.test_send_leave_request_command.request.destId,
                                                                                                               msg->data.test_send_leave_request_command.request.destEui);
}

void sli_zigbee_stack_test_send_link_key_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.test_send_link_key.response.result = sli_zigbee_stack_test_send_link_key(msg->data.test_send_link_key.request.targetNodeId,
                                                                                     msg->data.test_send_link_key.request.targetEui64,
                                                                                     msg->data.test_send_link_key.request.keyType,
                                                                                     &msg->data.test_send_link_key.request.key,
                                                                                     msg->data.test_send_link_key.request.useApsEncryption);
}

void sli_zigbee_stack_test_send_network_rejoin_command_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_test_send_network_rejoin_command(msg->data.test_send_network_rejoin_command.request.cmd_id,
                                                    msg->data.test_send_network_rejoin_command.request.longId,
                                                    msg->data.test_send_network_rejoin_command.request.oldShortId,
                                                    msg->data.test_send_network_rejoin_command.request.newShortId,
                                                    msg->data.test_send_network_rejoin_command.request.useNwkSecurity,
                                                    msg->data.test_send_network_rejoin_command.request.status,
                                                    msg->data.test_send_network_rejoin_command.request.reallySend);
}

void sli_zigbee_stack_test_send_network_timeout_request_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_test_send_network_timeout_request(msg->data.test_send_network_timeout_request.request.requestedTimeoutValue);
}

void sli_zigbee_stack_test_send_our_end_device_announcement_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_zigbee_stack_test_send_our_end_device_announcement();
}

void sli_zigbee_stack_test_send_remove_device_command_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.test_send_remove_device_command.response.result = sli_zigbee_stack_test_send_remove_device_command(msg->data.test_send_remove_device_command.request.destId,
                                                                                                               msg->data.test_send_remove_device_command.request.destEui,
                                                                                                               msg->data.test_send_remove_device_command.request.deviceToRemoveEui,
                                                                                                               msg->data.test_send_remove_device_command.request.sendNonEncrypted);
}

void sli_zigbee_stack_test_send_report_or_update_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.test_send_report_or_update.response.result = sli_zigbee_stack_test_send_report_or_update(msg->data.test_send_report_or_update.request.command,
                                                                                                     msg->data.test_send_report_or_update.request.updateId,
                                                                                                     msg->data.test_send_report_or_update.request.panId);
}

void sli_zigbee_stack_test_send_route_error_payload_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_test_send_route_error_payload(msg->data.test_send_route_error_payload.request.destination,
                                                 msg->data.test_send_route_error_payload.request.target,
                                                 msg->data.test_send_route_error_payload.request.errorCode,
                                                 &msg->data.test_send_route_error_payload.request.payload,
                                                 msg->data.test_send_route_error_payload.request.payload_len);
}

void sli_zigbee_stack_test_send_route_error_payload_no_network_encryption_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_test_send_route_error_payload_no_network_encryption(msg->data.test_send_route_error_payload_no_network_encryption.request.destination,
                                                                       msg->data.test_send_route_error_payload_no_network_encryption.request.target,
                                                                       msg->data.test_send_route_error_payload_no_network_encryption.request.errorCode,
                                                                       &msg->data.test_send_route_error_payload_no_network_encryption.request.payload,
                                                                       msg->data.test_send_route_error_payload_no_network_encryption.request.payload_len);
}

void sli_zigbee_stack_test_send_route_request_with_tlv_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.test_send_route_request_with_tlv.response.result = sli_zigbee_stack_test_send_route_request_with_tlv(msg->data.test_send_route_request_with_tlv.request.target);
}

void sli_zigbee_stack_test_send_timeout_request_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_zigbee_stack_test_send_timeout_request();
}

void sli_zigbee_stack_test_set_network_tokens_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_test_set_network_tokens(msg->data.test_set_network_tokens.request.stackProfile,
                                           msg->data.test_set_network_tokens.request.nodeType,
                                           msg->data.test_set_network_tokens.request.channel,
                                           msg->data.test_set_network_tokens.request.power,
                                           msg->data.test_set_network_tokens.request.nodeId,
                                           msg->data.test_set_network_tokens.request.panId,
                                           msg->data.test_set_network_tokens.request.extendedPanId);
}

void sli_zigbee_stack_test_spoof_device_announcement_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_test_spoof_device_announcement(msg->data.test_spoof_device_announcement.request.shortId,
                                                  &msg->data.test_spoof_device_announcement.request.sourceEUI64,
                                                  msg->data.test_spoof_device_announcement.request.deviceAnnounceEui,
                                                  msg->data.test_spoof_device_announcement.request.capabilities);
}

void sli_zigbee_stack_test_zdo_generate_clear_all_bindings_req_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.test_zdo_generate_clear_all_bindings_req.response.result = sli_zigbee_stack_test_zdo_generate_clear_all_bindings_req(msg->data.test_zdo_generate_clear_all_bindings_req.request.destination,
                                                                                                                                 msg->data.test_zdo_generate_clear_all_bindings_req.request.encrypt,
                                                                                                                                 msg->data.test_zdo_generate_clear_all_bindings_req.request.eui64_list,
                                                                                                                                 msg->data.test_zdo_generate_clear_all_bindings_req.request.counts);
}

void sli_zigbee_stack_test_zdo_generate_get_authentication_level_req_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.test_zdo_generate_get_authentication_level_req.response.result = sli_zigbee_stack_test_zdo_generate_get_authentication_level_req(msg->data.test_zdo_generate_get_authentication_level_req.request.dest,
                                                                                                                                             msg->data.test_zdo_generate_get_authentication_level_req.request.aps_options,
                                                                                                                                             msg->data.test_zdo_generate_get_authentication_level_req.request.target);
}

void sli_zigbee_stack_test_zdo_generate_security_decommission_req_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.test_zdo_generate_security_decommission_req.response.result = sli_zigbee_stack_test_zdo_generate_security_decommission_req(msg->data.test_zdo_generate_security_decommission_req.request.destination,
                                                                                                                                       msg->data.test_zdo_generate_security_decommission_req.request.encrypt,
                                                                                                                                       msg->data.test_zdo_generate_security_decommission_req.request.eui64_list,
                                                                                                                                       msg->data.test_zdo_generate_security_decommission_req.request.counts);
}

void sli_zigbee_stack_zigbee_remove_child_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.zigbee_remove_child.response.result = sli_zigbee_stack_zigbee_remove_child(msg->data.zigbee_remove_child.request.childId,
                                                                                       msg->data.zigbee_remove_child.request.options);
}

void slxi_zigbee_stack_gu_zdo_toggle_dlk_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  slxi_zigbee_stack_gu_zdo_toggle_dlk(msg->data.gu_zdo_toggle_dlk.request.do_dlk,
                                      msg->data.gu_zdo_toggle_dlk.request.allow_anon_psk);
}

void slxi_zigbee_stack_ignore_incoming_aps_acks_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  slxi_zigbee_stack_ignore_incoming_aps_acks(msg->data.ignore_incoming_aps_acks.request.ignore);
}

// public entrypoints

sl_802154_short_addr_t sl_mac_find_child_short_id(sl_802154_long_addr_t eui64)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (eui64 != NULL) {
    memmove(msg.data.find_child_short_id.request.eui64, eui64, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_mac_stack_find_child_short_id_process_ipc_command, &msg);

  if (eui64 != NULL) {
    memmove(eui64, msg.data.find_child_short_id.request.eui64, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.find_child_short_id.response.result;
}

uint64_t sl_mac_get_child_info_flags(uint8_t childIndex)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_child_info_flags.request.childIndex = childIndex;
  sli_zigbee_send_ipc_cmd(sli_mac_stack_get_child_info_flags_process_ipc_command, &msg);

  return msg.data.get_child_info_flags.response.result;
}

void sl_mac_indirect_purge(uint8_t nwk_index)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.indirect_purge.request.nwk_index = nwk_index;
  sli_zigbee_send_ipc_cmd(sli_mac_stack_indirect_purge_process_ipc_command, &msg);
}

void sl_mac_kickstart(uint8_t mac_index)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.kickstart.request.mac_index = mac_index;
  sli_zigbee_send_ipc_cmd(sli_mac_stack_kickstart_process_ipc_command, &msg);
}

bool sl_mac_lower_mac_radio_is_on(uint8_t mac_index)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.lower_mac_radio_is_on.request.mac_index = mac_index;
  sli_zigbee_send_ipc_cmd(sli_mac_stack_lower_mac_radio_is_on_process_ipc_command, &msg);

  return msg.data.lower_mac_radio_is_on.response.result;
}

void sl_mac_lower_mac_radio_sleep(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_mac_stack_lower_mac_radio_sleep_process_ipc_command, &msg);
}

void sl_mac_lower_mac_radio_wakeup(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_mac_stack_lower_mac_radio_wakeup_process_ipc_command, &msg);
}

sl_status_t sl_mac_lower_mac_set_radio_channel(uint8_t mac_index,
                                               uint8_t channel)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.lower_mac_set_radio_channel.request.mac_index = mac_index;
  msg.data.lower_mac_set_radio_channel.request.channel = channel;
  sli_zigbee_send_ipc_cmd(sli_mac_stack_lower_mac_set_radio_channel_process_ipc_command, &msg);

  return msg.data.lower_mac_set_radio_channel.response.result;
}

sl_status_t sl_mac_lower_mac_set_radio_idle_mode(uint8_t mac_index,
                                                 uint8_t mode)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.lower_mac_set_radio_idle_mode.request.mac_index = mac_index;
  msg.data.lower_mac_set_radio_idle_mode.request.mode = mode;
  sli_zigbee_send_ipc_cmd(sli_mac_stack_lower_mac_set_radio_idle_mode_process_ipc_command, &msg);

  return msg.data.lower_mac_set_radio_idle_mode.response.result;
}

sl_status_t sl_mac_lower_mac_set_radio_power(uint8_t mac_index,
                                             int8_t power)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.lower_mac_set_radio_power.request.mac_index = mac_index;
  msg.data.lower_mac_set_radio_power.request.power = power;
  sli_zigbee_send_ipc_cmd(sli_mac_stack_lower_mac_set_radio_power_process_ipc_command, &msg);

  return msg.data.lower_mac_set_radio_power.response.result;
}

void sl_mac_set_coordinator(bool isCoordinator)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_coordinator.request.isCoordinator = isCoordinator;
  sli_zigbee_send_ipc_cmd(sli_mac_stack_set_coordinator_process_ipc_command, &msg);
}

sl_status_t sl_mac_test_associate_command(sl_802154_short_addr_t parentId,
                                          uint16_t panId)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_associate_command.request.parentId = parentId;
  msg.data.test_associate_command.request.panId = panId;
  sli_zigbee_send_ipc_cmd(sli_mac_stack_test_associate_command_process_ipc_command, &msg);

  return msg.data.test_associate_command.response.result;
}

void sl_mac_test_send_mac_command(uint8_t macCommandLength,
                                  uint8_t *macCommand)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_send_mac_command.request.macCommandLength = macCommandLength;

  if (macCommandLength > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector macCommand length exceeds expected maximum
  }

  memmove(msg.data.test_send_mac_command.request.macCommand, macCommand, sizeof(uint8_t) * macCommandLength);
  sli_zigbee_send_ipc_cmd(sli_mac_stack_test_send_mac_command_process_ipc_command, &msg);

  if (macCommandLength > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector macCommand length exceeds expected maximum
  }

  memmove(macCommand, msg.data.test_send_mac_command.request.macCommand, sizeof(uint8_t) * macCommandLength);
}

sl_status_t sl_mac_test_set_nwk_radio_params_channel(uint8_t channel)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_set_nwk_radio_params_channel.request.channel = channel;
  sli_zigbee_send_ipc_cmd(sli_mac_stack_test_set_nwk_radio_params_channel_process_ipc_command, &msg);

  return msg.data.test_set_nwk_radio_params_channel.response.result;
}

sl_status_t sl_mac_test_set_nwk_radio_params_eui(uint8_t network_index,
                                                 sl_802154_long_addr_t eui)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_set_nwk_radio_params_eui.request.network_index = network_index;

  if (eui != NULL) {
    memmove(msg.data.test_set_nwk_radio_params_eui.request.eui, eui, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_mac_stack_test_set_nwk_radio_params_eui_process_ipc_command, &msg);

  if (eui != NULL) {
    memmove(eui, msg.data.test_set_nwk_radio_params_eui.request.eui, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.test_set_nwk_radio_params_eui.response.result;
}

sl_status_t sl_mac_test_set_tx_power(int8_t power)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_set_tx_power.request.power = power;
  sli_zigbee_send_ipc_cmd(sli_mac_stack_test_set_tx_power_process_ipc_command, &msg);

  return msg.data.test_set_tx_power.response.result;
}

uint8_t sl_zigbee_bdb_tclk_max_exchange_attempts(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_bdb_tclk_max_exchange_attempts_process_ipc_command, &msg);

  return msg.data.bdb_tclk_max_exchange_attempts.response.result;
}

sl_status_t sl_zigbee_request_link_key_with_option_encrypt(sl_802154_long_addr_t partner,
                                                           uint8_t option)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (partner != NULL) {
    memmove(msg.data.request_link_key_with_option_encrypt.request.partner, partner, sizeof(sl_802154_long_addr_t));
  }

  msg.data.request_link_key_with_option_encrypt.request.option = option;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_request_link_key_with_option_encrypt_process_ipc_command, &msg);

  if (partner != NULL) {
    memmove(partner, msg.data.request_link_key_with_option_encrypt.request.partner, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.request_link_key_with_option_encrypt.response.result;
}

sl_status_t sl_zigbee_send_aps_ack(sl_zigbee_aps_frame_t apsStruct,
                                   sl_802154_short_addr_t dest)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.send_aps_ack.request.apsStruct = apsStruct;
  msg.data.send_aps_ack.request.dest = dest;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_send_aps_ack_process_ipc_command, &msg);

  return msg.data.send_aps_ack.response.result;
}

void sl_zigbee_set_end_device_poll_timeout(uint8_t timeout)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_end_device_poll_timeout.request.timeout = timeout;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_end_device_poll_timeout_process_ipc_command, &msg);
}

void sl_zigbee_set_eui64(sl_802154_long_addr_t eui64)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (eui64 != NULL) {
    memmove(msg.data.set_eui64.request.eui64, eui64, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_eui64_process_ipc_command, &msg);

  if (eui64 != NULL) {
    memmove(eui64, msg.data.set_eui64.request.eui64, sizeof(sl_802154_long_addr_t));
  }
}

void sl_zigbee_set_ignore_aps_acks(bool ignore)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_ignore_aps_acks.request.ignore = ignore;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_ignore_aps_acks_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_set_packet_validate_library_state(uint16_t state)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_packet_validate_library_state.request.state = state;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_packet_validate_library_state_process_ipc_command, &msg);

  return msg.data.set_packet_validate_library_state.response.result;
}

void sl_zigbee_set_pan_id(uint16_t panId)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_pan_id.request.panId = panId;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_pan_id_process_ipc_command, &msg);
}

void sl_zigbee_set_pan_id_conflict_report(boolean set_value,
                                          uint8_t nwk_index)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_pan_id_conflict_report.request.set_value = set_value;
  msg.data.set_pan_id_conflict_report.request.nwk_index = nwk_index;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_pan_id_conflict_report_process_ipc_command, &msg);
}

void sl_zigbee_set_stack_compliance_revision(uint8_t revision)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_stack_compliance_revision.request.revision = revision;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_stack_compliance_revision_process_ipc_command, &msg);
}

void sl_zigbee_set_zdo_dlk_save_derived_key(bool value)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_zdo_dlk_save_derived_key.request.value = value;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_zdo_dlk_save_derived_key_process_ipc_command, &msg);
}

bool sl_zigbee_test_aps_key_in_sync(sl_802154_long_addr_t eui64)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (eui64 != NULL) {
    memmove(msg.data.test_aps_key_in_sync.request.eui64, eui64, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_aps_key_in_sync_process_ipc_command, &msg);

  if (eui64 != NULL) {
    memmove(eui64, msg.data.test_aps_key_in_sync.request.eui64, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.test_aps_key_in_sync.response.result;
}

sl_status_t sl_zigbee_test_aps_key_set_sync_status(sl_802154_long_addr_t eui64,
                                                   bool setSync)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (eui64 != NULL) {
    memmove(msg.data.test_aps_key_set_sync_status.request.eui64, eui64, sizeof(sl_802154_long_addr_t));
  }

  msg.data.test_aps_key_set_sync_status.request.setSync = setSync;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_aps_key_set_sync_status_process_ipc_command, &msg);

  if (eui64 != NULL) {
    memmove(eui64, msg.data.test_aps_key_set_sync_status.request.eui64, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.test_aps_key_set_sync_status.response.result;
}

sl_status_t sl_zigbee_test_custom_send_security_challenge_request(sl_802154_short_addr_t destShort,
                                                                  sl_zigbee_sec_man_context_t *context,
                                                                  uint8_t cmdoptions)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_custom_send_security_challenge_request.request.destShort = destShort;

  if (context != NULL) {
    msg.data.test_custom_send_security_challenge_request.request.context = *context;
  }

  msg.data.test_custom_send_security_challenge_request.request.cmdoptions = cmdoptions;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_custom_send_security_challenge_request_process_ipc_command, &msg);

  if (context != NULL) {
    *context = msg.data.test_custom_send_security_challenge_request.request.context;
  }

  return msg.data.test_custom_send_security_challenge_request.response.result;
}

sl_status_t sl_zigbee_test_ieee_address_request_to_target(sl_802154_short_addr_t discoveryNodeId,
                                                          bool reportKids,
                                                          uint8_t childStartIndex,
                                                          uint8_t sourceEndpoint,
                                                          sl_zigbee_aps_option_t options,
                                                          sl_802154_short_addr_t targetNodeIdOfRequest)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_ieee_address_request_to_target.request.discoveryNodeId = discoveryNodeId;
  msg.data.test_ieee_address_request_to_target.request.reportKids = reportKids;
  msg.data.test_ieee_address_request_to_target.request.childStartIndex = childStartIndex;
  msg.data.test_ieee_address_request_to_target.request.sourceEndpoint = sourceEndpoint;
  msg.data.test_ieee_address_request_to_target.request.options = options;
  msg.data.test_ieee_address_request_to_target.request.targetNodeIdOfRequest = targetNodeIdOfRequest;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_ieee_address_request_to_target_process_ipc_command, &msg);

  return msg.data.test_ieee_address_request_to_target.response.result;
}

void sl_zigbee_test_join_list_add(uint8_t command,
                                  uint8_t *eui64List,
                                  uint8_t counts)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_join_list_add.request.command = command;

  if ((counts * EUI64_SIZE) > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector eui64List length exceeds expected maximum
  }

  memmove(msg.data.test_join_list_add.request.eui64List, eui64List, sizeof(uint8_t) * (counts * EUI64_SIZE));
  msg.data.test_join_list_add.request.counts = counts;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_join_list_add_process_ipc_command, &msg);

  if ((counts * EUI64_SIZE) > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector eui64List length exceeds expected maximum
  }

  memmove(eui64List, msg.data.test_join_list_add.request.eui64List, sizeof(uint8_t) * (counts * EUI64_SIZE));
}

void sl_zigbee_test_join_list_request(uint8_t startIndex)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_join_list_request.request.startIndex = startIndex;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_join_list_request_process_ipc_command, &msg);
}

bool sl_zigbee_test_network_send_command(sl_802154_short_addr_t destination,
                                         uint8_t *commandFrame,
                                         uint8_t length,
                                         bool tryToInsertLongDest,
                                         sl_802154_long_addr_t destinationEui)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_network_send_command.request.destination = destination;

  if (commandFrame != NULL) {
    msg.data.test_network_send_command.request.commandFrame = *commandFrame;
  }

  msg.data.test_network_send_command.request.length = length;
  msg.data.test_network_send_command.request.tryToInsertLongDest = tryToInsertLongDest;

  if (destinationEui != NULL) {
    memmove(msg.data.test_network_send_command.request.destinationEui, destinationEui, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_network_send_command_process_ipc_command, &msg);

  if (commandFrame != NULL) {
    *commandFrame = msg.data.test_network_send_command.request.commandFrame;
  }

  if (destinationEui != NULL) {
    memmove(destinationEui, msg.data.test_network_send_command.request.destinationEui, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.test_network_send_command.response.result;
}

void sl_zigbee_test_perform_raw_active_scan(uint32_t scanChannels,
                                            uint8_t scanDuration)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_perform_raw_active_scan.request.scanChannels = scanChannels;
  msg.data.test_perform_raw_active_scan.request.scanDuration = scanDuration;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_perform_raw_active_scan_process_ipc_command, &msg);
}

void sl_zigbee_test_reset_frame_counter(uint8_t mask)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_reset_frame_counter.request.mask = mask;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_reset_frame_counter_process_ipc_command, &msg);
}

bool sl_zigbee_test_send_device_update(uint16_t newShortId,
                                       sl_802154_long_addr_t newLongId,
                                       bool apsEncryption,
                                       uint8_t deviceStatus)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_send_device_update.request.newShortId = newShortId;

  if (newLongId != NULL) {
    memmove(msg.data.test_send_device_update.request.newLongId, newLongId, sizeof(sl_802154_long_addr_t));
  }

  msg.data.test_send_device_update.request.apsEncryption = apsEncryption;
  msg.data.test_send_device_update.request.deviceStatus = deviceStatus;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_send_device_update_process_ipc_command, &msg);

  if (newLongId != NULL) {
    memmove(newLongId, msg.data.test_send_device_update.request.newLongId, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.test_send_device_update.response.result;
}

sl_status_t sl_zigbee_test_send_leave_request_command(uint16_t destId,
                                                      sl_802154_long_addr_t destEui)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_send_leave_request_command.request.destId = destId;

  if (destEui != NULL) {
    memmove(msg.data.test_send_leave_request_command.request.destEui, destEui, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_send_leave_request_command_process_ipc_command, &msg);

  if (destEui != NULL) {
    memmove(destEui, msg.data.test_send_leave_request_command.request.destEui, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.test_send_leave_request_command.response.result;
}

bool sl_zigbee_test_send_link_key(sl_802154_short_addr_t targetNodeId,
                                  sl_802154_long_addr_t targetEui64,
                                  uint8_t keyType,
                                  sl_zigbee_key_data_t *key,
                                  bool useApsEncryption)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_send_link_key.request.targetNodeId = targetNodeId;

  if (targetEui64 != NULL) {
    memmove(msg.data.test_send_link_key.request.targetEui64, targetEui64, sizeof(sl_802154_long_addr_t));
  }

  msg.data.test_send_link_key.request.keyType = keyType;

  if (key != NULL) {
    msg.data.test_send_link_key.request.key = *key;
  }

  msg.data.test_send_link_key.request.useApsEncryption = useApsEncryption;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_send_link_key_process_ipc_command, &msg);

  if (targetEui64 != NULL) {
    memmove(targetEui64, msg.data.test_send_link_key.request.targetEui64, sizeof(sl_802154_long_addr_t));
  }

  if (key != NULL) {
    *key = msg.data.test_send_link_key.request.key;
  }

  return msg.data.test_send_link_key.response.result;
}

void sl_zigbee_test_send_network_rejoin_command(uint8_t cmd_id,
                                                sl_802154_long_addr_t longId,
                                                sl_802154_short_addr_t oldShortId,
                                                sl_802154_short_addr_t newShortId,
                                                bool useNwkSecurity,
                                                uint8_t status,
                                                bool reallySend)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_send_network_rejoin_command.request.cmd_id = cmd_id;

  if (longId != NULL) {
    memmove(msg.data.test_send_network_rejoin_command.request.longId, longId, sizeof(sl_802154_long_addr_t));
  }

  msg.data.test_send_network_rejoin_command.request.oldShortId = oldShortId;
  msg.data.test_send_network_rejoin_command.request.newShortId = newShortId;
  msg.data.test_send_network_rejoin_command.request.useNwkSecurity = useNwkSecurity;
  msg.data.test_send_network_rejoin_command.request.status = status;
  msg.data.test_send_network_rejoin_command.request.reallySend = reallySend;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_send_network_rejoin_command_process_ipc_command, &msg);

  if (longId != NULL) {
    memmove(longId, msg.data.test_send_network_rejoin_command.request.longId, sizeof(sl_802154_long_addr_t));
  }
}

void sl_zigbee_test_send_network_timeout_request(uint8_t requestedTimeoutValue)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_send_network_timeout_request.request.requestedTimeoutValue = requestedTimeoutValue;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_send_network_timeout_request_process_ipc_command, &msg);
}

void sl_zigbee_test_send_our_end_device_announcement(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_send_our_end_device_announcement_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_test_send_remove_device_command(uint16_t destId,
                                                      sl_802154_long_addr_t destEui,
                                                      sl_802154_long_addr_t deviceToRemoveEui,
                                                      bool sendNonEncrypted)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_send_remove_device_command.request.destId = destId;

  if (destEui != NULL) {
    memmove(msg.data.test_send_remove_device_command.request.destEui, destEui, sizeof(sl_802154_long_addr_t));
  }

  if (deviceToRemoveEui != NULL) {
    memmove(msg.data.test_send_remove_device_command.request.deviceToRemoveEui, deviceToRemoveEui, sizeof(sl_802154_long_addr_t));
  }

  msg.data.test_send_remove_device_command.request.sendNonEncrypted = sendNonEncrypted;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_send_remove_device_command_process_ipc_command, &msg);

  if (destEui != NULL) {
    memmove(destEui, msg.data.test_send_remove_device_command.request.destEui, sizeof(sl_802154_long_addr_t));
  }

  if (deviceToRemoveEui != NULL) {
    memmove(deviceToRemoveEui, msg.data.test_send_remove_device_command.request.deviceToRemoveEui, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.test_send_remove_device_command.response.result;
}

bool sl_zigbee_test_send_report_or_update(uint8_t command,
                                          uint8_t updateId,
                                          uint16_t panId)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_send_report_or_update.request.command = command;
  msg.data.test_send_report_or_update.request.updateId = updateId;
  msg.data.test_send_report_or_update.request.panId = panId;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_send_report_or_update_process_ipc_command, &msg);

  return msg.data.test_send_report_or_update.response.result;
}

void sl_zigbee_test_send_route_error_payload(sl_802154_short_addr_t destination,
                                             sl_802154_short_addr_t target,
                                             uint8_t errorCode,
                                             uint8_t *payload,
                                             uint8_t payload_len)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_send_route_error_payload.request.destination = destination;
  msg.data.test_send_route_error_payload.request.target = target;
  msg.data.test_send_route_error_payload.request.errorCode = errorCode;

  if (payload != NULL) {
    msg.data.test_send_route_error_payload.request.payload = *payload;
  }

  msg.data.test_send_route_error_payload.request.payload_len = payload_len;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_send_route_error_payload_process_ipc_command, &msg);

  if (payload != NULL) {
    *payload = msg.data.test_send_route_error_payload.request.payload;
  }
}

void sl_zigbee_test_send_route_error_payload_no_network_encryption(sl_802154_short_addr_t destination,
                                                                   sl_802154_short_addr_t target,
                                                                   uint8_t errorCode,
                                                                   uint8_t *payload,
                                                                   uint8_t payload_len)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_send_route_error_payload_no_network_encryption.request.destination = destination;
  msg.data.test_send_route_error_payload_no_network_encryption.request.target = target;
  msg.data.test_send_route_error_payload_no_network_encryption.request.errorCode = errorCode;

  if (payload != NULL) {
    msg.data.test_send_route_error_payload_no_network_encryption.request.payload = *payload;
  }

  msg.data.test_send_route_error_payload_no_network_encryption.request.payload_len = payload_len;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_send_route_error_payload_no_network_encryption_process_ipc_command, &msg);

  if (payload != NULL) {
    *payload = msg.data.test_send_route_error_payload_no_network_encryption.request.payload;
  }
}

sl_status_t sl_zigbee_test_send_route_request_with_tlv(sl_802154_short_addr_t target)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_send_route_request_with_tlv.request.target = target;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_send_route_request_with_tlv_process_ipc_command, &msg);

  return msg.data.test_send_route_request_with_tlv.response.result;
}

void sl_zigbee_test_send_timeout_request(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_send_timeout_request_process_ipc_command, &msg);
}

void sl_zigbee_test_set_network_tokens(uint8_t stackProfile,
                                       uint8_t nodeType,
                                       uint8_t channel,
                                       int8_t power,
                                       uint16_t nodeId,
                                       uint16_t panId,
                                       sl_802154_long_addr_t extendedPanId)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_set_network_tokens.request.stackProfile = stackProfile;
  msg.data.test_set_network_tokens.request.nodeType = nodeType;
  msg.data.test_set_network_tokens.request.channel = channel;
  msg.data.test_set_network_tokens.request.power = power;
  msg.data.test_set_network_tokens.request.nodeId = nodeId;
  msg.data.test_set_network_tokens.request.panId = panId;

  if (extendedPanId != NULL) {
    memmove(msg.data.test_set_network_tokens.request.extendedPanId, extendedPanId, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_set_network_tokens_process_ipc_command, &msg);

  if (extendedPanId != NULL) {
    memmove(extendedPanId, msg.data.test_set_network_tokens.request.extendedPanId, sizeof(sl_802154_long_addr_t));
  }
}

void sl_zigbee_test_spoof_device_announcement(uint16_t shortId,
                                              uint8_t *sourceEUI64,
                                              sl_802154_long_addr_t deviceAnnounceEui,
                                              uint8_t capabilities)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_spoof_device_announcement.request.shortId = shortId;

  if (sourceEUI64 != NULL) {
    msg.data.test_spoof_device_announcement.request.sourceEUI64 = *sourceEUI64;
  }

  if (deviceAnnounceEui != NULL) {
    memmove(msg.data.test_spoof_device_announcement.request.deviceAnnounceEui, deviceAnnounceEui, sizeof(sl_802154_long_addr_t));
  }

  msg.data.test_spoof_device_announcement.request.capabilities = capabilities;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_spoof_device_announcement_process_ipc_command, &msg);

  if (sourceEUI64 != NULL) {
    *sourceEUI64 = msg.data.test_spoof_device_announcement.request.sourceEUI64;
  }

  if (deviceAnnounceEui != NULL) {
    memmove(deviceAnnounceEui, msg.data.test_spoof_device_announcement.request.deviceAnnounceEui, sizeof(sl_802154_long_addr_t));
  }
}

sl_status_t sl_zigbee_test_zdo_generate_clear_all_bindings_req(sl_802154_short_addr_t destination,
                                                               bool encrypt,
                                                               uint8_t *eui64_list,
                                                               uint8_t counts)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_zdo_generate_clear_all_bindings_req.request.destination = destination;
  msg.data.test_zdo_generate_clear_all_bindings_req.request.encrypt = encrypt;

  if ((counts * EUI64_SIZE) > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector eui64_list length exceeds expected maximum
  }

  memmove(msg.data.test_zdo_generate_clear_all_bindings_req.request.eui64_list, eui64_list, sizeof(uint8_t) * (counts * EUI64_SIZE));
  msg.data.test_zdo_generate_clear_all_bindings_req.request.counts = counts;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_zdo_generate_clear_all_bindings_req_process_ipc_command, &msg);

  if ((counts * EUI64_SIZE) > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector eui64_list length exceeds expected maximum
  }

  memmove(eui64_list, msg.data.test_zdo_generate_clear_all_bindings_req.request.eui64_list, sizeof(uint8_t) * (counts * EUI64_SIZE));
  return msg.data.test_zdo_generate_clear_all_bindings_req.response.result;
}

sl_status_t sl_zigbee_test_zdo_generate_get_authentication_level_req(sl_802154_short_addr_t dest,
                                                                     sl_zigbee_aps_option_t aps_options,
                                                                     sl_802154_long_addr_t target)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_zdo_generate_get_authentication_level_req.request.dest = dest;
  msg.data.test_zdo_generate_get_authentication_level_req.request.aps_options = aps_options;

  if (target != NULL) {
    memmove(msg.data.test_zdo_generate_get_authentication_level_req.request.target, target, sizeof(sl_802154_long_addr_t));
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_zdo_generate_get_authentication_level_req_process_ipc_command, &msg);

  if (target != NULL) {
    memmove(target, msg.data.test_zdo_generate_get_authentication_level_req.request.target, sizeof(sl_802154_long_addr_t));
  }

  return msg.data.test_zdo_generate_get_authentication_level_req.response.result;
}

sl_status_t sl_zigbee_test_zdo_generate_security_decommission_req(sl_802154_short_addr_t destination,
                                                                  bool encrypt,
                                                                  uint8_t *eui64_list,
                                                                  uint8_t counts)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.test_zdo_generate_security_decommission_req.request.destination = destination;
  msg.data.test_zdo_generate_security_decommission_req.request.encrypt = encrypt;

  if ((counts * EUI64_SIZE) > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector eui64_list length exceeds expected maximum
  }

  memmove(msg.data.test_zdo_generate_security_decommission_req.request.eui64_list, eui64_list, sizeof(uint8_t) * (counts * EUI64_SIZE));
  msg.data.test_zdo_generate_security_decommission_req.request.counts = counts;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_test_zdo_generate_security_decommission_req_process_ipc_command, &msg);

  if ((counts * EUI64_SIZE) > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector eui64_list length exceeds expected maximum
  }

  memmove(eui64_list, msg.data.test_zdo_generate_security_decommission_req.request.eui64_list, sizeof(uint8_t) * (counts * EUI64_SIZE));
  return msg.data.test_zdo_generate_security_decommission_req.response.result;
}

sl_status_t sl_zigbee_zigbee_remove_child(sl_802154_short_addr_t childId,
                                          uint8_t options)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.zigbee_remove_child.request.childId = childId;
  msg.data.zigbee_remove_child.request.options = options;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_zigbee_remove_child_process_ipc_command, &msg);

  return msg.data.zigbee_remove_child.response.result;
}

void slx_zigbee_gu_zdo_toggle_dlk(bool do_dlk,
                                  bool allow_anon_psk)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.gu_zdo_toggle_dlk.request.do_dlk = do_dlk;
  msg.data.gu_zdo_toggle_dlk.request.allow_anon_psk = allow_anon_psk;
  sli_zigbee_send_ipc_cmd(slxi_zigbee_stack_gu_zdo_toggle_dlk_process_ipc_command, &msg);
}

void slx_zigbee_ignore_incoming_aps_acks(bool ignore)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.ignore_incoming_aps_acks.request.ignore = ignore;
  sli_zigbee_send_ipc_cmd(slxi_zigbee_stack_ignore_incoming_aps_acks_process_ipc_command, &msg);
}
