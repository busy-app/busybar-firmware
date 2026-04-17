/***************************************************************************//**
 * @file sl_zigbee_memory_usage_baremetal_wrapper.c
 * @brief internal implementations for 'sl_zigbee_memory_usage' as a thin-wrapper
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
#include "stack/include/sl_zigbee_memory_usage.h"
#include "stack/internal/inc/sl_zigbee_memory_usage_internal_def.h"

uint32_t sl_zigbee_get_memory_usage_data(sl_zigbee_memory_usage_data_t type)
{
  return sli_zigbee_stack_get_memory_usage_data(type);
}

void sl_zigbee_memory_usage_init(void)
{
  sli_zigbee_stack_memory_usage_init();
}
