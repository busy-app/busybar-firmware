/***************************************************************************//**
 * @file network-formation-ipc-command-messages.c
 * @brief internal wrappers for 'network-formation' ipc commands
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
// automatically generated from network-formation.h.  Do not manually edit
#include "stack/include/network-formation.h"
#include "stack/internal/inc/network-formation-internal-def.h"
#include "stack/internal/src/ipc/network-formation-ipc-command-messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_clear_stored_beacons_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.clear_stored_beacons.response.result = sli_zigbee_stack_clear_stored_beacons();
}

void sli_zigbee_stack_delayed_join_activate_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_delayed_join_activate(msg->data.delayed_join_activate.request.activated);
}

void sli_zigbee_stack_delayed_join_is_activated_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.delayed_join_is_activated.response.result = sli_zigbee_stack_delayed_join_is_activated();
}

void sli_zigbee_stack_find_and_rejoin_network_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.find_and_rejoin_network.response.result = sli_zigbee_stack_find_and_rejoin_network(msg->data.find_and_rejoin_network.request.haveCurrentNetworkKey,
                                                                                               msg->data.find_and_rejoin_network.request.channelMask,
                                                                                               msg->data.find_and_rejoin_network.request.reason,
                                                                                               msg->data.find_and_rejoin_network.request.nodeType);
}

void sli_zigbee_stack_find_unused_pan_id_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.find_unused_pan_id.response.result = sli_zigbee_stack_find_unused_pan_id(msg->data.find_unused_pan_id.request.channelMask,
                                                                                     msg->data.find_unused_pan_id.request.duration);
}

void sli_zigbee_stack_form_network_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.form_network.response.result = sli_zigbee_stack_form_network(&msg->data.form_network.request.parameters);
}

void sli_zigbee_stack_get_beacon_classification_params_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_beacon_classification_params.response.result = sli_zigbee_stack_get_beacon_classification_params(&msg->data.get_beacon_classification_params.request.param);
}

void sli_zigbee_stack_get_join_uses_install_code_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_join_uses_install_code.response.result = sli_zigbee_stack_get_join_uses_install_code();
}

void sli_zigbee_stack_get_last_leave_reason_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_last_leave_reason.response.result = sli_zigbee_stack_get_last_leave_reason(&msg->data.get_last_leave_reason.request.returnNodeIdThatSentLeave);
}

void sli_zigbee_stack_get_last_rejoin_reason_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_last_rejoin_reason.response.result = sli_zigbee_stack_get_last_rejoin_reason();
}

void sli_zigbee_stack_get_network_key_timeout_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_network_key_timeout.response.result = sli_zigbee_stack_get_network_key_timeout();
}

void sli_zigbee_stack_get_num_stored_beacons_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_num_stored_beacons.response.result = sli_zigbee_stack_get_num_stored_beacons();
}

void sli_zigbee_stack_get_permit_joining_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_permit_joining.response.result = sli_zigbee_stack_get_permit_joining();
}

void sli_zigbee_stack_get_stored_beacon_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_stored_beacon.response.result = sli_zigbee_stack_get_stored_beacon(msg->data.get_stored_beacon.request.beacon_number,
                                                                                   &msg->data.get_stored_beacon.request.beacon);
}

void sli_zigbee_stack_init_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.init.response.result = sli_zigbee_stack_init();
}

void sli_zigbee_stack_is_performing_rejoin_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.is_performing_rejoin.response.result = sli_zigbee_stack_is_performing_rejoin();
}

void sli_zigbee_stack_join_network_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.join_network.response.result = sli_zigbee_stack_join_network(msg->data.join_network.request.nodeType,
                                                                         &msg->data.join_network.request.parameters);
}

void sli_zigbee_stack_join_network_directly_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.join_network_directly.response.result = sli_zigbee_stack_join_network_directly(msg->data.join_network_directly.request.localNodeType,
                                                                                           &msg->data.join_network_directly.request.beacon,
                                                                                           msg->data.join_network_directly.request.radioTxPower,
                                                                                           msg->data.join_network_directly.request.clearBeaconsAfterNetworkUp);
}

void sli_zigbee_stack_leave_network_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.leave_network.response.result = sli_zigbee_stack_leave_network(msg->data.leave_network.request.options);
}

void sli_zigbee_stack_network_init_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.network_init.response.result = sli_zigbee_stack_network_init(&msg->data.network_init.request.networkInitStruct);
}

void sli_zigbee_stack_permit_joining_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.permit_joining.response.result = sli_zigbee_stack_permit_joining(msg->data.permit_joining.request.duration);
}

void sli_zigbee_stack_reschedule_link_status_msg_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.reschedule_link_status_msg.response.result = sli_zigbee_stack_reschedule_link_status_msg();
}

void sli_zigbee_stack_send_pan_id_update_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.send_pan_id_update.response.result = sli_zigbee_stack_send_pan_id_update(msg->data.send_pan_id_update.request.newPan);
}

void sli_zigbee_stack_send_zigbee_leave_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.send_zigbee_leave.response.result = sli_zigbee_stack_send_zigbee_leave(msg->data.send_zigbee_leave.request.destination,
                                                                                   msg->data.send_zigbee_leave.request.flags);
}

void sli_zigbee_stack_set_beacon_classification_params_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_beacon_classification_params.response.result = sli_zigbee_stack_set_beacon_classification_params(&msg->data.set_beacon_classification_params.request.param);
}

void sli_zigbee_stack_set_beacon_jitter_duration_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_beacon_jitter_duration.response.result = sli_zigbee_stack_set_beacon_jitter_duration(msg->data.set_beacon_jitter_duration.request.beaconJitterDuration);
}

void sli_zigbee_stack_set_join_uses_install_code_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_join_uses_install_code(msg->data.set_join_uses_install_code.request.enable);
}

void sli_zigbee_stack_set_num_beacons_to_store_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_num_beacons_to_store.response.result = sli_zigbee_stack_set_num_beacons_to_store(msg->data.set_num_beacons_to_store.request.numBeacons);
}

void sli_zigbee_stack_set_nwk_update_id_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_nwk_update_id.response.result = sli_zigbee_stack_set_nwk_update_id(msg->data.set_nwk_update_id.request.nwkUpdateId,
                                                                                   msg->data.set_nwk_update_id.request.set_when_on_network);
}

void sli_zigbee_stack_set_pending_network_update_pan_id_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_pending_network_update_pan_id(msg->data.set_pending_network_update_pan_id.request.panId);
}

void sli_zigbee_stack_set_tc_rejoins_using_well_known_key_allowed_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_tc_rejoins_using_well_known_key_allowed.response.result = sli_zigbee_stack_set_tc_rejoins_using_well_known_key_allowed(msg->data.set_tc_rejoins_using_well_known_key_allowed.request.allow);
}

void sli_zigbee_stack_set_tc_rejoins_using_well_known_key_timeout_sec_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_tc_rejoins_using_well_known_key_timeout_sec.response.result = sli_zigbee_stack_set_tc_rejoins_using_well_known_key_timeout_sec(msg->data.set_tc_rejoins_using_well_known_key_timeout_sec.request.timeout_sec);
}

void sli_zigbee_stack_setup_delayed_join_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.setup_delayed_join.response.result = sli_zigbee_stack_setup_delayed_join(msg->data.setup_delayed_join.request.networkKeyTimeoutS);
}

void sli_zigbee_stack_sleepy_to_sleepy_network_start_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sleepy_to_sleepy_network_start.response.result = sli_zigbee_stack_sleepy_to_sleepy_network_start(&msg->data.sleepy_to_sleepy_network_start.request.parameters,
                                                                                                             msg->data.sleepy_to_sleepy_network_start.request.initiator);
}

void sli_zigbee_stack_start_scan_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.start_scan.response.result = sli_zigbee_stack_start_scan(msg->data.start_scan.request.scanType,
                                                                     msg->data.start_scan.request.channelMask,
                                                                     msg->data.start_scan.request.duration);
}

void sli_zigbee_stack_stop_scan_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.stop_scan.response.result = sli_zigbee_stack_stop_scan();
}

void sli_zigbee_stack_survey_beacons_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.survey_beacons.response.result = sli_zigbee_stack_survey_beacons(msg->data.survey_beacons.request.useStandardBeacons,
                                                                             msg->data.survey_beacons.request.channel_mask,
                                                                             msg->data.survey_beacons.request.networkFoundCallback,
                                                                             msg->data.survey_beacons.request.scanCompleteCallback);
}

void sli_zigbee_stack_tc_rejoins_using_well_known_key_allowed_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.tc_rejoins_using_well_known_key_allowed.response.result = sli_zigbee_stack_tc_rejoins_using_well_known_key_allowed();
}

void slxi_zigbee_stack_change_pan_id_now_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  slxi_zigbee_stack_change_pan_id_now(msg->data.change_pan_id_now.request.panId);
}

// public entrypoints

sl_status_t sl_zigbee_clear_stored_beacons(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_clear_stored_beacons_process_ipc_command, &msg);

  return msg.data.clear_stored_beacons.response.result;
}

void sl_zigbee_delayed_join_activate(bool activated)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.delayed_join_activate.request.activated = activated;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_delayed_join_activate_process_ipc_command, &msg);
}

bool sl_zigbee_delayed_join_is_activated(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_delayed_join_is_activated_process_ipc_command, &msg);

  return msg.data.delayed_join_is_activated.response.result;
}

sl_status_t sl_zigbee_find_and_rejoin_network(bool haveCurrentNetworkKey,
                                              uint32_t channelMask,
                                              sl_zigbee_rejoin_reason_t reason,
                                              sl_zigbee_node_type_t nodeType)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.find_and_rejoin_network.request.haveCurrentNetworkKey = haveCurrentNetworkKey;
  msg.data.find_and_rejoin_network.request.channelMask = channelMask;
  msg.data.find_and_rejoin_network.request.reason = reason;
  msg.data.find_and_rejoin_network.request.nodeType = nodeType;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_find_and_rejoin_network_process_ipc_command, &msg);

  return msg.data.find_and_rejoin_network.response.result;
}

sl_status_t sl_zigbee_find_unused_pan_id(uint32_t channelMask,
                                         uint8_t duration)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.find_unused_pan_id.request.channelMask = channelMask;
  msg.data.find_unused_pan_id.request.duration = duration;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_find_unused_pan_id_process_ipc_command, &msg);

  return msg.data.find_unused_pan_id.response.result;
}

sl_status_t sl_zigbee_form_network(sl_zigbee_network_parameters_t *parameters)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (parameters != NULL) {
    msg.data.form_network.request.parameters = *parameters;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_form_network_process_ipc_command, &msg);

  if (parameters != NULL) {
    *parameters = msg.data.form_network.request.parameters;
  }

  return msg.data.form_network.response.result;
}

sl_status_t sl_zigbee_get_beacon_classification_params(sl_zigbee_beacon_classification_params_t *param)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (param != NULL) {
    msg.data.get_beacon_classification_params.request.param = *param;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_beacon_classification_params_process_ipc_command, &msg);

  if (param != NULL) {
    *param = msg.data.get_beacon_classification_params.request.param;
  }

  return msg.data.get_beacon_classification_params.response.result;
}

bool sl_zigbee_get_join_uses_install_code(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_join_uses_install_code_process_ipc_command, &msg);

  return msg.data.get_join_uses_install_code.response.result;
}

sl_zigbee_leave_reason_t sl_zigbee_get_last_leave_reason(sl_802154_short_addr_t *returnNodeIdThatSentLeave)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (returnNodeIdThatSentLeave != NULL) {
    msg.data.get_last_leave_reason.request.returnNodeIdThatSentLeave = *returnNodeIdThatSentLeave;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_last_leave_reason_process_ipc_command, &msg);

  if (returnNodeIdThatSentLeave != NULL) {
    *returnNodeIdThatSentLeave = msg.data.get_last_leave_reason.request.returnNodeIdThatSentLeave;
  }

  return msg.data.get_last_leave_reason.response.result;
}

sl_zigbee_rejoin_reason_t sl_zigbee_get_last_rejoin_reason(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_last_rejoin_reason_process_ipc_command, &msg);

  return msg.data.get_last_rejoin_reason.response.result;
}

uint8_t sl_zigbee_get_network_key_timeout(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_network_key_timeout_process_ipc_command, &msg);

  return msg.data.get_network_key_timeout.response.result;
}

uint8_t sl_zigbee_get_num_stored_beacons(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_num_stored_beacons_process_ipc_command, &msg);

  return msg.data.get_num_stored_beacons.response.result;
}

bool sl_zigbee_get_permit_joining(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_permit_joining_process_ipc_command, &msg);

  return msg.data.get_permit_joining.response.result;
}

sl_status_t sl_zigbee_get_stored_beacon(uint8_t beacon_number,
                                        sl_zigbee_beacon_data_t *beacon)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_stored_beacon.request.beacon_number = beacon_number;

  if (beacon != NULL) {
    msg.data.get_stored_beacon.request.beacon = *beacon;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_stored_beacon_process_ipc_command, &msg);

  if (beacon != NULL) {
    *beacon = msg.data.get_stored_beacon.request.beacon;
  }

  return msg.data.get_stored_beacon.response.result;
}

sl_status_t sl_zigbee_init(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_init_process_ipc_command, &msg);

  return msg.data.init.response.result;
}

bool sl_zigbee_is_performing_rejoin(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_is_performing_rejoin_process_ipc_command, &msg);

  return msg.data.is_performing_rejoin.response.result;
}

sl_status_t sl_zigbee_join_network(sl_zigbee_node_type_t nodeType,
                                   sl_zigbee_network_parameters_t *parameters)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.join_network.request.nodeType = nodeType;

  if (parameters != NULL) {
    msg.data.join_network.request.parameters = *parameters;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_join_network_process_ipc_command, &msg);

  if (parameters != NULL) {
    *parameters = msg.data.join_network.request.parameters;
  }

  return msg.data.join_network.response.result;
}

sl_status_t sl_zigbee_join_network_directly(sl_zigbee_node_type_t localNodeType,
                                            sl_zigbee_beacon_data_t *beacon,
                                            int8_t radioTxPower,
                                            bool clearBeaconsAfterNetworkUp)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.join_network_directly.request.localNodeType = localNodeType;

  if (beacon != NULL) {
    msg.data.join_network_directly.request.beacon = *beacon;
  }

  msg.data.join_network_directly.request.radioTxPower = radioTxPower;
  msg.data.join_network_directly.request.clearBeaconsAfterNetworkUp = clearBeaconsAfterNetworkUp;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_join_network_directly_process_ipc_command, &msg);

  if (beacon != NULL) {
    *beacon = msg.data.join_network_directly.request.beacon;
  }

  return msg.data.join_network_directly.response.result;
}

sl_status_t sl_zigbee_leave_network(sl_zigbee_leave_network_option_t options)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.leave_network.request.options = options;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_leave_network_process_ipc_command, &msg);

  return msg.data.leave_network.response.result;
}

sl_status_t sl_zigbee_network_init(sl_zigbee_network_init_struct_t *networkInitStruct)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (networkInitStruct != NULL) {
    msg.data.network_init.request.networkInitStruct = *networkInitStruct;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_network_init_process_ipc_command, &msg);

  if (networkInitStruct != NULL) {
    *networkInitStruct = msg.data.network_init.request.networkInitStruct;
  }

  return msg.data.network_init.response.result;
}

sl_status_t sl_zigbee_permit_joining(uint8_t duration)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.permit_joining.request.duration = duration;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_permit_joining_process_ipc_command, &msg);

  return msg.data.permit_joining.response.result;
}

sl_status_t sl_zigbee_reschedule_link_status_msg(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_reschedule_link_status_msg_process_ipc_command, &msg);

  return msg.data.reschedule_link_status_msg.response.result;
}

bool sl_zigbee_send_pan_id_update(sl_802154_pan_id_t newPan)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.send_pan_id_update.request.newPan = newPan;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_send_pan_id_update_process_ipc_command, &msg);

  return msg.data.send_pan_id_update.response.result;
}

sl_status_t sl_zigbee_send_zigbee_leave(sl_802154_short_addr_t destination,
                                        sl_zigbee_leave_request_flags_t flags)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.send_zigbee_leave.request.destination = destination;
  msg.data.send_zigbee_leave.request.flags = flags;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_send_zigbee_leave_process_ipc_command, &msg);

  return msg.data.send_zigbee_leave.response.result;
}

sl_status_t sl_zigbee_set_beacon_classification_params(sl_zigbee_beacon_classification_params_t *param)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (param != NULL) {
    msg.data.set_beacon_classification_params.request.param = *param;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_beacon_classification_params_process_ipc_command, &msg);

  if (param != NULL) {
    *param = msg.data.set_beacon_classification_params.request.param;
  }

  return msg.data.set_beacon_classification_params.response.result;
}

sl_status_t sl_zigbee_set_beacon_jitter_duration(uint8_t beaconJitterDuration)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_beacon_jitter_duration.request.beaconJitterDuration = beaconJitterDuration;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_beacon_jitter_duration_process_ipc_command, &msg);

  return msg.data.set_beacon_jitter_duration.response.result;
}

void sl_zigbee_set_join_uses_install_code(bool enable)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_join_uses_install_code.request.enable = enable;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_join_uses_install_code_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_set_num_beacons_to_store(uint8_t numBeacons)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_num_beacons_to_store.request.numBeacons = numBeacons;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_num_beacons_to_store_process_ipc_command, &msg);

  return msg.data.set_num_beacons_to_store.response.result;
}

sl_status_t sl_zigbee_set_nwk_update_id(uint8_t nwkUpdateId,
                                        bool set_when_on_network)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_nwk_update_id.request.nwkUpdateId = nwkUpdateId;
  msg.data.set_nwk_update_id.request.set_when_on_network = set_when_on_network;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_nwk_update_id_process_ipc_command, &msg);

  return msg.data.set_nwk_update_id.response.result;
}

void sl_zigbee_set_pending_network_update_pan_id(uint16_t panId)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_pending_network_update_pan_id.request.panId = panId;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_pending_network_update_pan_id_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_set_tc_rejoins_using_well_known_key_allowed(bool allow)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_tc_rejoins_using_well_known_key_allowed.request.allow = allow;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_tc_rejoins_using_well_known_key_allowed_process_ipc_command, &msg);

  return msg.data.set_tc_rejoins_using_well_known_key_allowed.response.result;
}

sl_status_t sl_zigbee_set_tc_rejoins_using_well_known_key_timeout_sec(uint16_t timeout_sec)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_tc_rejoins_using_well_known_key_timeout_sec.request.timeout_sec = timeout_sec;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_tc_rejoins_using_well_known_key_timeout_sec_process_ipc_command, &msg);

  return msg.data.set_tc_rejoins_using_well_known_key_timeout_sec.response.result;
}

sl_status_t sl_zigbee_setup_delayed_join(uint8_t networkKeyTimeoutS)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.setup_delayed_join.request.networkKeyTimeoutS = networkKeyTimeoutS;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_setup_delayed_join_process_ipc_command, &msg);

  return msg.data.setup_delayed_join.response.result;
}

sl_status_t sl_zigbee_sleepy_to_sleepy_network_start(sl_zigbee_network_parameters_t *parameters,
                                                     bool initiator)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (parameters != NULL) {
    msg.data.sleepy_to_sleepy_network_start.request.parameters = *parameters;
  }

  msg.data.sleepy_to_sleepy_network_start.request.initiator = initiator;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sleepy_to_sleepy_network_start_process_ipc_command, &msg);

  if (parameters != NULL) {
    *parameters = msg.data.sleepy_to_sleepy_network_start.request.parameters;
  }

  return msg.data.sleepy_to_sleepy_network_start.response.result;
}

sl_status_t sl_zigbee_start_scan(sl_zigbee_network_scan_type_t scanType,
                                 uint32_t channelMask,
                                 uint8_t duration)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.start_scan.request.scanType = scanType;
  msg.data.start_scan.request.channelMask = channelMask;
  msg.data.start_scan.request.duration = duration;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_start_scan_process_ipc_command, &msg);

  return msg.data.start_scan.response.result;
}

sl_status_t sl_zigbee_stop_scan(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_stop_scan_process_ipc_command, &msg);

  return msg.data.stop_scan.response.result;
}

sl_status_t sl_zigbee_survey_beacons(bool useStandardBeacons,
                                     uint32_t channel_mask,
                                     sl_zigbee_network_found_callback_t networkFoundCallback,
                                     sl_zigbee_scan_complete_callback_t scanCompleteCallback)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.survey_beacons.request.useStandardBeacons = useStandardBeacons;
  msg.data.survey_beacons.request.channel_mask = channel_mask;
  msg.data.survey_beacons.request.networkFoundCallback = networkFoundCallback;
  msg.data.survey_beacons.request.scanCompleteCallback = scanCompleteCallback;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_survey_beacons_process_ipc_command, &msg);

  return msg.data.survey_beacons.response.result;
}

bool sl_zigbee_tc_rejoins_using_well_known_key_allowed(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_tc_rejoins_using_well_known_key_allowed_process_ipc_command, &msg);

  return msg.data.tc_rejoins_using_well_known_key_allowed.response.result;
}

void slx_zigbee_change_pan_id_now(sl_802154_pan_id_t panId)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.change_pan_id_now.request.panId = panId;
  sli_zigbee_send_ipc_cmd(slxi_zigbee_stack_change_pan_id_now_process_ipc_command, &msg);
}
