/***************************************************************************//**
 * @file sl_zigbee_dynamic_commissioning_internal_def.h
 * @brief internal names for 'sl_zigbee_dynamic_commissioning' declarations
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
#ifndef SL_ZIGBEE_DYNAMIC_COMMISSIONING_INTERNAL_DEF_H
#define SL_ZIGBEE_DYNAMIC_COMMISSIONING_INTERNAL_DEF_H

#include "stack/include/sl_zigbee_dynamic_commissioning.h"

// Command Indirection

void sli_zigbee_stack_device_interview_status_update(sl_zigbee_address_info *ids,
                                                     sl_zigbee_dynamic_commissioning_event_t event);

bool sli_zigbee_stack_dynamic_commissioning_is_open_for_interview(void);

void sli_zigbee_stack_dynamic_commissioning_set_open_for_interview(bool open_interview);

// Callback Indirection

void sli_zigbee_stack_dynamic_commissioning_alert_callback(sl_zigbee_address_info *ids,
                                                           sl_zigbee_dynamic_commissioning_event_t event);

#endif // SL_ZIGBEE_DYNAMIC_COMMISSIONING_INTERNAL_DEF_H
