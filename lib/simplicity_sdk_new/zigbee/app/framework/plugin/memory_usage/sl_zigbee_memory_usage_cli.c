/***************************************************************************//**
 * @file
 * @brief CLI commands for accessing SoC/NCP memory usage data.
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

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include "sl_common.h"
#include "app/framework/include/af.h"
#include "stack/include/sl_zigbee_memory_usage.h"

//------------------------------------------------------------------------------
// CLI Commands

void sli_zigbee_af_memory_usage_print_current_data(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_af_core_println("Total heap size: %zu bytes", sl_zigbee_get_memory_usage_data(TOTAL_HEAP_SIZE));
  sl_zigbee_af_core_println("Current used heap size: %zu bytes", sl_zigbee_get_memory_usage_data(CURRENT_USED_HEAP_SIZE));
  sl_zigbee_af_core_println("Current heap size high watermark: %zu bytes", sl_zigbee_get_memory_usage_data(CURRENT_HEAP_HIGH_WATERMARK));
}

void sli_zigbee_af_memory_usage_print_init_data(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_af_core_println("Total heap size: %zu bytes", sl_zigbee_get_memory_usage_data(TOTAL_HEAP_SIZE));
  sl_zigbee_af_core_println("Init used heap size: %zu bytes", sl_zigbee_get_memory_usage_data(INIT_USED_HEAP_SIZE));
  sl_zigbee_af_core_println("Init heap size high watermark: %zu bytes", sl_zigbee_get_memory_usage_data(INIT_HEAP_HIGH_WATERMARK));
}
