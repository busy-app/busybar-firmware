/***************************************************************************//**
 * @file sl_legacy_hal_wdog.c
 * @brief Legacy HAL Watchdog
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

#include PLATFORM_HEADER
#include "sl_legacy_hal_wdog.h"
#include "sl_code_classification.h"

/***************************************************************************//**
 * Watchdog Interrupt Handler.
 ******************************************************************************/
#if (_SILICON_LABS_32B_SERIES >= 1)
SL_WEAK void SL_LEGACY_HAL_WDOG_IRQHandler(void)
{
  uint32_t interrupts;

  interrupts = WDOGn_IntGet(SL_LEGACY_HAL_WDOG);
  WDOGn_IntClear(SL_LEGACY_HAL_WDOG, interrupts);
}
#endif

void halInternalEnableWatchDog(void)
{
  sl_clock_manager_enable_bus_clock(SL_LEGACY_HAL_WDOG_CMUCLOCK);

  // Make sure FULL reset is used on WDOG timeout
#if defined(_RMU_CTRL_WDOGRMODE_MASK)
  RMU_ResetControl(rmuResetWdog, rmuResetModeFull);
#endif

  /* Note: WDOG_INIT_DEFAULT comes from platform/emlib/inc/em_wdog.h */
  WDOG_Init_TypeDef init = WDOG_INIT_DEFAULT;

  // The default timeout of wdogPeriod_64k will trigger
  // watchdog reset after 2 seconds (64k / 32k) and
  // warning interrupt is triggered after 1.5 seconds (75% of timeout).
  init.perSel = SL_LEGACY_HAL_WDOG_PERIOD;
  init.warnSel = SL_LEGACY_HAL_WDOG_WARNING;

  // Counter keeps running during debug halt
  init.debugRun = SL_LEGACY_HAL_WDOG_DEBUG_RUN;

#if defined(_WDOG_CTRL_CLKSEL_MASK)
  init.clkSel = wdogClkSelLFRCO;
#endif

  WDOGn_Init(SL_LEGACY_HAL_WDOG, &init);

  /* Enable WARN interrupt. */
#if defined(WDOG_IF_WARN) && !defined(BOOTLOADER)
  NVIC_ClearPendingIRQ(SL_LEGACY_HAL_WDOG_IRQn);
  WDOGn_IntClear(SL_LEGACY_HAL_WDOG, WDOG_IF_WARN);
  NVIC_EnableIRQ(SL_LEGACY_HAL_WDOG_IRQn);
  WDOGn_IntEnable(SL_LEGACY_HAL_WDOG, WDOG_IEN_WARN);
#endif
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_LEGACY_HAL, SL_CODE_CLASS_TIME_CRITICAL)
void halResetWatchdog(void)
{
#if (SL_LEGACY_HAL_DISABLE_WATCHDOG == 0)
  WDOGn_Feed(SL_LEGACY_HAL_WDOG);
#endif // (SL_LEGACY_HAL_DISABLE_WATCHDOG == 0)
}

/** @brief The value no longer matters.
 */
void halInternalDisableWatchDog(uint8_t magicKey)
{
  (void) magicKey;

#if (SL_LEGACY_HAL_DISABLE_WATCHDOG == 0)
  WDOGn_SyncWait(DEFAULT_WDOG);
  WDOGn_Enable(SL_LEGACY_HAL_WDOG, false);
#endif // (SL_LEGACY_HAL_DISABLE_WATCHDOG == 0)
}

bool halInternalWatchDogEnabled(void)
{
#if (SL_LEGACY_HAL_DISABLE_WATCHDOG == 0)
  return WDOGn_IsEnabled(SL_LEGACY_HAL_WDOG);
#else // (SL_LEGACY_HAL_DISABLE_WATCHDOG == 0)
  return false;
#endif // (SL_LEGACY_HAL_DISABLE_WATCHDOG == 0)
}
