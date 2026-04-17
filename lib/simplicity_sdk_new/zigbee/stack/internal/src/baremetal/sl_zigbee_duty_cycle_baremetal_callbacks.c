/***************************************************************************//**
 * @file sl_zigbee_duty_cycle_baremetal_callbacks.c
 * @brief internal dispatch for 'sl_zigbee_duty_cycle' callbacks as a thin-wrapper
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

void sli_zigbee_stack_duty_cycle_handler(uint8_t channelPage,
                                         uint8_t channel,
                                         sl_zigbee_duty_cycle_state_t state,
                                         uint8_t totalDevices,
                                         sl_zigbee_per_device_duty_cycle_t *arrayOfDeviceDutyCycles)
{
  sl_zigbee_duty_cycle_handler(channelPage,
                               channel,
                               state,
                               totalDevices,
                               arrayOfDeviceDutyCycles);
}
