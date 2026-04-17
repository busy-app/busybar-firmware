/***************************************************************************//**
 * @file sl_zigbee_duty_cycle_baremetal_wrapper.c
 * @brief internal implementations for 'sl_zigbee_duty_cycle' as a thin-wrapper
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

sl_status_t sl_zigbee_get_current_duty_cycle(uint8_t maxDevices,
                                             sl_zigbee_per_device_duty_cycle_t *arrayOfDeviceDutyCycles)
{
  return sli_zigbee_stack_get_current_duty_cycle(maxDevices,
                                                 arrayOfDeviceDutyCycles);
}

sl_status_t sl_zigbee_get_duty_cycle_limits(sl_zigbee_duty_cycle_limits_t *returnedLimits)
{
  return sli_zigbee_stack_get_duty_cycle_limits(returnedLimits);
}

sl_status_t sl_zigbee_get_duty_cycle_state(sl_zigbee_duty_cycle_state_t *returnedState)
{
  return sli_zigbee_stack_get_duty_cycle_state(returnedState);
}

sl_status_t sl_zigbee_set_duty_cycle_limits_in_stack(const sl_zigbee_duty_cycle_limits_t *limits)
{
  return sli_zigbee_stack_set_duty_cycle_limits_in_stack(limits);
}
