/***************************************************************************//**
 * @file sl_zigbee_duty_cycle_ipc_command_messages.c
 * @brief internal wrappers for 'sl_zigbee_duty_cycle' ipc commands
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
// automatically generated from sl_zigbee_duty_cycle.h.  Do not manually edit
#include "stack/include/sl_zigbee_duty_cycle.h"
#include "stack/internal/inc/sl_zigbee_duty_cycle_internal_def.h"
#include "stack/internal/src/ipc/sl_zigbee_duty_cycle_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_get_current_duty_cycle_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_current_duty_cycle.response.result = sli_zigbee_stack_get_current_duty_cycle(msg->data.get_current_duty_cycle.request.maxDevices,
                                                                                             msg->data.get_current_duty_cycle.request.arrayOfDeviceDutyCycles);
}

void sli_zigbee_stack_get_duty_cycle_limits_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_duty_cycle_limits.response.result = sli_zigbee_stack_get_duty_cycle_limits(&msg->data.get_duty_cycle_limits.request.returnedLimits);
}

void sli_zigbee_stack_get_duty_cycle_state_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.get_duty_cycle_state.response.result = sli_zigbee_stack_get_duty_cycle_state(&msg->data.get_duty_cycle_state.request.returnedState);
}

void sli_zigbee_stack_set_duty_cycle_limits_in_stack_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.set_duty_cycle_limits_in_stack.response.result = sli_zigbee_stack_set_duty_cycle_limits_in_stack(&msg->data.set_duty_cycle_limits_in_stack.request.limits);
}

// public entrypoints

sl_status_t sl_zigbee_get_current_duty_cycle(uint8_t maxDevices,
                                             sl_zigbee_per_device_duty_cycle_t *arrayOfDeviceDutyCycles)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.get_current_duty_cycle.request.maxDevices = maxDevices;

  if (maxDevices > SL_ZIGBEE_MAX_CHILDREN_FOR_PER_DEVICE_DUTY_CYCLE_MONITOR) {
    assert(false); // "vector arrayOfDeviceDutyCycles length exceeds expected maximum
  }

  memmove(msg.data.get_current_duty_cycle.request.arrayOfDeviceDutyCycles, arrayOfDeviceDutyCycles, sizeof(sl_zigbee_per_device_duty_cycle_t) * maxDevices);
  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_current_duty_cycle_process_ipc_command, &msg);

  if (maxDevices > SL_ZIGBEE_MAX_CHILDREN_FOR_PER_DEVICE_DUTY_CYCLE_MONITOR) {
    assert(false); // "vector arrayOfDeviceDutyCycles length exceeds expected maximum
  }

  memmove(arrayOfDeviceDutyCycles, msg.data.get_current_duty_cycle.request.arrayOfDeviceDutyCycles, sizeof(sl_zigbee_per_device_duty_cycle_t) * maxDevices);
  return msg.data.get_current_duty_cycle.response.result;
}

sl_status_t sl_zigbee_get_duty_cycle_limits(sl_zigbee_duty_cycle_limits_t *returnedLimits)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (returnedLimits != NULL) {
    msg.data.get_duty_cycle_limits.request.returnedLimits = *returnedLimits;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_duty_cycle_limits_process_ipc_command, &msg);

  if (returnedLimits != NULL) {
    *returnedLimits = msg.data.get_duty_cycle_limits.request.returnedLimits;
  }

  return msg.data.get_duty_cycle_limits.response.result;
}

sl_status_t sl_zigbee_get_duty_cycle_state(sl_zigbee_duty_cycle_state_t *returnedState)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (returnedState != NULL) {
    msg.data.get_duty_cycle_state.request.returnedState = *returnedState;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_get_duty_cycle_state_process_ipc_command, &msg);

  if (returnedState != NULL) {
    *returnedState = msg.data.get_duty_cycle_state.request.returnedState;
  }

  return msg.data.get_duty_cycle_state.response.result;
}

sl_status_t sl_zigbee_set_duty_cycle_limits_in_stack(const sl_zigbee_duty_cycle_limits_t *limits)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (limits != NULL) {
    msg.data.set_duty_cycle_limits_in_stack.request.limits = *limits;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_set_duty_cycle_limits_in_stack_process_ipc_command, &msg);

  return msg.data.set_duty_cycle_limits_in_stack.response.result;
}
