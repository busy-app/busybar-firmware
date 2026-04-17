/***************************************************************************//**
 * @brief Debugging helpers used internally by the Bluetooth stack
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <em_device.h>
#include "sli_bgcommon_debug.h"

// Enable the MCU cycle counter
void sli_bgcommon_debug_init_cycle_count(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

// Get the current MCU cycle count
uint32_t sli_bgcommon_debug_get_cycle_count(void)
{
  return DWT->CYCCNT;
}
