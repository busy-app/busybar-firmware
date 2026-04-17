/***************************************************************************//**
 * @file source-route-ipc-command-messages.c
 * @brief internal wrappers for 'source-route' ipc commands
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
// automatically generated from source-route.h.  Do not manually edit
#include "stack/include/source-route.h"
#include "stack/internal/inc/source-route-internal-def.h"
#include "stack/internal/src/ipc/source-route-ipc-command-messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_concentrator_note_delivery_failure_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_concentrator_note_delivery_failure(msg->data.concentrator_note_delivery_failure.request.type,
                                                      msg->data.concentrator_note_delivery_failure.request.status);
}

void sli_zigbee_stack_concentrator_note_route_error_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_concentrator_note_route_error(msg->data.concentrator_note_route_error.request.status,
                                                 msg->data.concentrator_note_route_error.request.nodeId);
}

void sli_zigbee_stack_concentrator_start_discovery_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_zigbee_stack_concentrator_start_discovery();
}

void sli_zigbee_stack_concentrator_stop_discovery_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  (void)msg;
  sli_zigbee_stack_concentrator_stop_discovery();
}

void sli_zigbee_stack_get_assumed_trust_center_concentrator_type_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_assumed_trust_center_concentrator_type.response.result = sli_zigbee_stack_get_assumed_trust_center_concentrator_type();
}

void sli_zigbee_stack_get_source_route_overhead_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_source_route_overhead.response.result = sli_zigbee_stack_get_source_route_overhead(msg->data.get_source_route_overhead.request.destination);
}

void sli_zigbee_stack_get_source_route_table_entry_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_source_route_table_entry.response.result = sli_zigbee_stack_get_source_route_table_entry(msg->data.get_source_route_table_entry.request.index,
                                                                                                         &msg->data.get_source_route_table_entry.request.destination,
                                                                                                         &msg->data.get_source_route_table_entry.request.closerIndex);
}

void sli_zigbee_stack_get_source_route_table_filled_size_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_source_route_table_filled_size.response.result = sli_zigbee_stack_get_source_route_table_filled_size();
}

void sli_zigbee_stack_get_source_route_table_total_size_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_source_route_table_total_size.response.result = sli_zigbee_stack_get_source_route_table_total_size();
}

void sli_zigbee_stack_send_many_to_one_route_request_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.send_many_to_one_route_request.response.result = sli_zigbee_stack_send_many_to_one_route_request(msg->data.send_many_to_one_route_request.request.concentratorType,
                                                                                                             msg->data.send_many_to_one_route_request.request.radius);
}

void sli_zigbee_stack_set_assumed_trust_center_concentrator_type_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_set_assumed_trust_center_concentrator_type(msg->data.set_assumed_trust_center_concentrator_type.request.type);
}

void sli_zigbee_stack_set_concentrator_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_concentrator.response.result = sli_zigbee_stack_set_concentrator(msg->data.set_concentrator.request.on,
                                                                                 msg->data.set_concentrator.request.concentratorTypeParam,
                                                                                 msg->data.set_concentrator.request.minTime,
                                                                                 msg->data.set_concentrator.request.maxTime,
                                                                                 msg->data.set_concentrator.request.routeErrorThresholdParam,
                                                                                 msg->data.set_concentrator.request.deliveryFailureThresholdParam,
                                                                                 msg->data.set_concentrator.request.maxHopsParam);
}

void sli_zigbee_stack_set_source_route_discovery_mode_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_source_route_discovery_mode.response.result = sli_zigbee_stack_set_source_route_discovery_mode(msg->data.set_source_route_discovery_mode.request.mode);
}

// public entrypoints

void sl_zigbee_concentrator_note_delivery_failure(sl_zigbee_outgoing_message_type_t type,
                                                  sl_status_t status)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.concentrator_note_delivery_failure.request.type = type;
  msg.data.concentrator_note_delivery_failure.request.status = status;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_concentrator_note_delivery_failure_process_ipc_command, &msg);
}

void sl_zigbee_concentrator_note_route_error(sl_status_t status,
                                             sl_802154_short_addr_t nodeId)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.concentrator_note_route_error.request.status = status;
  msg.data.concentrator_note_route_error.request.nodeId = nodeId;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_concentrator_note_route_error_process_ipc_command, &msg);
}

void sl_zigbee_concentrator_start_discovery(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_concentrator_start_discovery_process_ipc_command, &msg);
}

void sl_zigbee_concentrator_stop_discovery(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_concentrator_stop_discovery_process_ipc_command, &msg);
}

sl_zigbee_assume_trust_center_concentrator_type_t sl_zigbee_get_assumed_trust_center_concentrator_type(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_assumed_trust_center_concentrator_type_process_ipc_command, &msg);

  return msg.data.get_assumed_trust_center_concentrator_type.response.result;
}

uint8_t sl_zigbee_get_source_route_overhead(sl_802154_short_addr_t destination)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_source_route_overhead.request.destination = destination;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_source_route_overhead_process_ipc_command, &msg);

  return msg.data.get_source_route_overhead.response.result;
}

sl_status_t sl_zigbee_get_source_route_table_entry(uint8_t index,
                                                   sl_802154_short_addr_t *destination,
                                                   uint8_t *closerIndex)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_source_route_table_entry.request.index = index;

  if (destination != NULL) {
    msg.data.get_source_route_table_entry.request.destination = *destination;
  }

  if (closerIndex != NULL) {
    msg.data.get_source_route_table_entry.request.closerIndex = *closerIndex;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_source_route_table_entry_process_ipc_command, &msg);

  if (destination != NULL) {
    *destination = msg.data.get_source_route_table_entry.request.destination;
  }

  if (closerIndex != NULL) {
    *closerIndex = msg.data.get_source_route_table_entry.request.closerIndex;
  }

  return msg.data.get_source_route_table_entry.response.result;
}

uint8_t sl_zigbee_get_source_route_table_filled_size(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_source_route_table_filled_size_process_ipc_command, &msg);

  return msg.data.get_source_route_table_filled_size.response.result;
}

uint8_t sl_zigbee_get_source_route_table_total_size(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_source_route_table_total_size_process_ipc_command, &msg);

  return msg.data.get_source_route_table_total_size.response.result;
}

sl_status_t sl_zigbee_send_many_to_one_route_request(uint16_t concentratorType,
                                                     uint8_t radius)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.send_many_to_one_route_request.request.concentratorType = concentratorType;
  msg.data.send_many_to_one_route_request.request.radius = radius;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_send_many_to_one_route_request_process_ipc_command, &msg);

  return msg.data.send_many_to_one_route_request.response.result;
}

void sl_zigbee_set_assumed_trust_center_concentrator_type(sl_zigbee_assume_trust_center_concentrator_type_t type)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_assumed_trust_center_concentrator_type.request.type = type;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_assumed_trust_center_concentrator_type_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_set_concentrator(bool on,
                                       uint16_t concentratorTypeParam,
                                       uint16_t minTime,
                                       uint16_t maxTime,
                                       uint8_t routeErrorThresholdParam,
                                       uint8_t deliveryFailureThresholdParam,
                                       uint8_t maxHopsParam)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_concentrator.request.on = on;
  msg.data.set_concentrator.request.concentratorTypeParam = concentratorTypeParam;
  msg.data.set_concentrator.request.minTime = minTime;
  msg.data.set_concentrator.request.maxTime = maxTime;
  msg.data.set_concentrator.request.routeErrorThresholdParam = routeErrorThresholdParam;
  msg.data.set_concentrator.request.deliveryFailureThresholdParam = deliveryFailureThresholdParam;
  msg.data.set_concentrator.request.maxHopsParam = maxHopsParam;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_concentrator_process_ipc_command, &msg);

  return msg.data.set_concentrator.response.result;
}

uint32_t sl_zigbee_set_source_route_discovery_mode(sl_zigbee_source_route_discovery_mode_t mode)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.set_source_route_discovery_mode.request.mode = mode;
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_source_route_discovery_mode_process_ipc_command, &msg);

  return msg.data.set_source_route_discovery_mode.response.result;
}
