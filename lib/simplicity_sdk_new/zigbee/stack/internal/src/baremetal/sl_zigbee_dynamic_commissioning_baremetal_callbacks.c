/***************************************************************************//**
 * @file sl_zigbee_dynamic_commissioning_baremetal_callbacks.c
 * @brief internal dispatch for 'sl_zigbee_dynamic_commissioning' callbacks as a thin-wrapper
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
// automatically generated from sl_zigbee_dynamic_commissioning.h.  Do not manually edit
#include "stack/include/sl_zigbee_dynamic_commissioning.h"
#include "stack/internal/inc/sl_zigbee_dynamic_commissioning_internal_def.h"

void sli_zigbee_stack_dynamic_commissioning_alert_callback(sl_zigbee_address_info *ids,
                                                           sl_zigbee_dynamic_commissioning_event_t event)
{
  sl_zigbee_dynamic_commissioning_alert_callback(ids,
                                                 event);
}
