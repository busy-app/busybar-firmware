/***************************************************************************//**
 * @file sl_zigbee_memory_usage_internal_def.h
 * @brief internal names for 'sl_zigbee_memory_usage' declarations
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
// automatically generated from sl_zigbee_memory_usage.h.  Do not manually edit
#ifndef SL_ZIGBEE_MEMORY_USAGE_INTERNAL_DEF_H
#define SL_ZIGBEE_MEMORY_USAGE_INTERNAL_DEF_H

#include "stack/include/sl_zigbee_memory_usage.h"

// Command Indirection

uint32_t sli_zigbee_stack_get_memory_usage_data(sl_zigbee_memory_usage_data_t type);

void sli_zigbee_stack_memory_usage_init(void);

#endif // SL_ZIGBEE_MEMORY_USAGE_INTERNAL_DEF_H
