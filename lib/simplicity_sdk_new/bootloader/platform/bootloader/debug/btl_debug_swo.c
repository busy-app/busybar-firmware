/***************************************************************************//**
 * @file
 * @brief SWO debug component for Silicon Labs Bootloader.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "btl_debug.h"
#include "em_device.h"
#if (_SILICON_LABS_32B_SERIES <= 2)
#include "em_cmu.h"
#elif (_SILICON_LABS_32B_SERIES == 3)
#include "sl_hal_gpio.h"
#endif
#include "btl_debug_cfg.h"

#if defined (SL_DEBUG_PRINT) && (SL_DEBUG_PRINT == 1)
#if defined(_SILICON_LABS_32B_SERIES_3)
static uint32_t util_get_trace_clock_freq(void)
{
  uint32_t clockFreq = 0;

#if defined(CMU_TRACECLKCTRL_CLKSEL_HFRCOEM23)
  clockFreq = SystemHFRCOEM23ClockGet() / (1U + ((CMU->TRACECLKCTRL & _CMU_TRACECLKCTRL_PRESC_MASK)
                                                 >> _CMU_TRACECLKCTRL_PRESC_SHIFT));
#elif defined(_CMU_TRACECLKCTRL_CLKSEL_SYSCLK)
  clockFreq = SystemSYSCLKGet() / (1U + ((CMU->TRACECLKCTRL & _CMU_TRACECLKCTRL_PRESC_MASK)
                                         >> _CMU_TRACECLKCTRL_PRESC_SHIFT));
#endif
  return clockFreq;
}
#endif

void btl_debugInit(void)
{
//Below variable is supported for Cortex-M4,M33
#if ((__CORTEX_M == 4) || (__CORTEX_M == 33))
  uint32_t tpiu_prescaler_val;
#endif

#if defined(_CMU_CLKEN0_MASK)
  CMU->CLKEN0_SET = CMU_CLKEN0_GPIO;
#endif

#if defined(GPIO_SWV_PORT)
  /* Enable output on pin */
  GPIO->P[GPIO_SWV_PORT].MODEL &= ~(_GPIO_P_MODEL_MODE0_MASK << (GPIO_SWV_PIN * 4));
  GPIO->P[GPIO_SWV_PORT].MODEL |= _GPIO_P_MODEL_MODE0_PUSHPULL << (GPIO_SWV_PIN * 4);
  GPIO->TRACEROUTEPEN |= GPIO_TRACEROUTEPEN_SWVPEN;
#else
  sl_hal_gpio_enable_debug_swo(true);
#endif

#if defined(_SILICON_LABS_32B_SERIES_2)
#if defined(_SILICON_LABS_GECKO_INTERNAL_SDID_215) || defined(_SILICON_LABS_GECKO_INTERNAL_SDID_220) \
  || defined(_SILICON_LABS_GECKO_INTERNAL_SDID_225)
#if defined(_CMU_TRACECLKCTRL_CLKSEL_MASK) && defined(_CMU_TRACECLKCTRL_CLKSEL_HFRCOEM23)
#if defined(CMU_CLKEN0_HFRCOEM23)
  CMU->CLKEN0_SET = CMU_CLKEN0_HFRCOEM23;
#endif
  // Select HFRCOEM23 as source for TRACECLK
  CMU_CLOCK_SELECT_SET(TRACECLK, HFRCOEM23);
#endif
#else
#if defined(_CMU_TRACECLKCTRL_CLKSEL_MASK)
#if defined(_SILICON_LABS_GECKO_INTERNAL_SDID_230) || defined(_SILICON_LABS_GECKO_INTERNAL_SDID_235) \
  || defined(_SILICON_LABS_GECKO_INTERNAL_SDID_240) || defined(_SILICON_LABS_GECKO_INTERNAL_SDID_260)
  /* Select SYSCLK as source for TRACECLK */
  CMU_CLOCK_SELECT_SET(TRACECLK, SYSCLK);
#else
  /* Select HFRCOEM23 as source for TRACECLK */
  CMU_CLOCK_SELECT_SET(TRACECLK, HFRCOEM23);
#endif
#endif
#endif

  /* Set TPIU prescaler to get a 863.63 kHz SWO speed */
  tpiu_prescaler_val = CMU_ClockFreqGet(cmuClock_TRACECLK) / 863630 - 1;

#elif defined(_SILICON_LABS_32B_SERIES_3)

  CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;

#if defined(_CMU_TRACECLKCTRL_CLKSEL_MASK)
#if defined(_CMU_TRACECLKCTRL_CLKSEL_HFRCOEM23)
#if defined(CMU_CLKEN0_HFRCOEM23)
  CMU->CLKEN0_SET = CMU_CLKEN0_HFRCOEM23;
#endif
  CMU->TRACECLKCTRL = (CMU->TRACECLKCTRL & ~_CMU_TRACECLKCTRL_CLKSEL_MASK) \
                      | CMU_TRACECLKCTRL_CLKSEL_HFRCOEM23;
#elif defined(_CMU_TRACECLKCTRL_CLKSEL_SYSCLK)
  CMU->TRACECLKCTRL = (CMU->TRACECLKCTRL & ~_CMU_TRACECLKCTRL_CLKSEL_MASK) \
                      | CMU_TRACECLKCTRL_CLKSEL_SYSCLK;
#endif
#endif
  /* Set TPIU prescaler to get a 863.63 kHz SWO speed */
  tpiu_prescaler_val = util_get_trace_clock_freq() / 863630 - 1;
#else
#error Unknown device family!
#endif

//Below registers are supported for Cortex-M3,M4,M33
#if ((__CORTEX_M == 4) || (__CORTEX_M == 33))
  // Enable trace in core debug
  CoreDebug->DHCSR |= 1UL;
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

  // Enable PC and IRQ sampling output
  DWT->CTRL = 0x400003FFUL;

  // Set TPIU Prescaler
  TPI->ACPR = tpiu_prescaler_val;

  // Set protocol to NRZ
  TPI->SPPR = 2;

  // Disable continuous formatting
  TPI->FFCR = 0x100;

  // Unlock ITM and output data
  ITM->LAR = 0xC5ACCE55UL;
  ITM->TCR = 0x10009UL;

  // ITM Channel 0 is used for UART output
  ITM->TER |= (1UL << 0);
#endif
}
#endif
void btl_debugWriteChar(char c)
{
  (void) ITM_SendChar((uint32_t)c);
}
