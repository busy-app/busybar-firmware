/***************************************************************************//**
 * @file sl_legacy_hal_wdog_stub.c
 * @brief Legacy HAL Watchdog Stub functions
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories, Inc, www.silabs.com</b>
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

#include "sl_common.h"

SL_WEAK void halInternalEnableWatchDog(void)
{
  // stub
}

SL_WEAK void halInternalDisableWatchDog(uint8_t magicKey)
{
  // stub
  (void) magicKey;
}

SL_WEAK bool halInternalWatchDogEnabled(void)
{
  // stub
  return false;
}

SL_WEAK void halResetWatchdog(void)
{
  // stub
}
