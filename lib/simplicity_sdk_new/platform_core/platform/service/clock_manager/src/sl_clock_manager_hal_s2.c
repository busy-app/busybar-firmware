/***************************************************************************//**
 * @file
 * @brief Clock Manager HAL API implementations.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include <stddef.h>
#include "sl_core.h"
#include "sl_clock_manager.h"
#include "sli_clock_manager.h"
#include "sli_clock_manager_hal.h"
#include "em_cmu.h"
#include "em_bus.h"
#include "em_device.h"
#include "em_gpio.h"
#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif

/***************************************************************************//**
 * Performs Clock Manager runtime initialization.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_runtime_init(void)
{
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Gets frequency of given oscillator.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_oscillator_frequency(sl_oscillator_t oscillator,
                                                           uint32_t *frequency)
{
  switch (oscillator) {
    case SL_OSCILLATOR_FSRCO:
      *frequency = SystemFSRCOClockGet();
      break;

    case SL_OSCILLATOR_HFXO:
      *frequency = SystemHFXOClockGet();
      break;

    case SL_OSCILLATOR_HFRCODPLL:
      *frequency = SystemHFRCODPLLClockGet();
      break;

#if defined(HFRCOEM23_PRESENT)
    case SL_OSCILLATOR_HFRCOEM23:
      *frequency = SystemHFRCOEM23ClockGet();
      break;
#endif

#if defined(RFFPLL_PRESENT)
    case SL_OSCILLATOR_RFFPLL:
      *frequency = SystemRFFPLLClockGet();
      break;
#endif

#if defined(USBPLL_PRESENT)
    case SL_OSCILLATOR_USBPLL:
      *frequency = CMU_ClockFreqGet(cmuClock_USB);
      break;
#endif

    case SL_OSCILLATOR_LFXO:
      *frequency = SystemLFXOClockGet();
      break;

    case SL_OSCILLATOR_LFRCO:
      *frequency = SystemLFRCOClockGet();
      break;

    case SL_OSCILLATOR_ULFRCO:
      *frequency = SystemULFRCOClockGet();
      break;

    default:
      *frequency = 0U;
      return SL_STATUS_INVALID_PARAMETER;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Gets precision of given oscillator.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_oscillator_precision(sl_oscillator_t oscillator,
                                                           uint16_t *precision)
{
  switch (oscillator) {
    case SL_OSCILLATOR_HFXO:
      *precision = CMU_HFXOPrecisionGet();
      break;

    case SL_OSCILLATOR_LFXO:
      *precision = CMU_LFXOPrecisionGet();
      break;

    case SL_OSCILLATOR_LFRCO:
#if defined(LFRCO_CFG_HIGHPRECEN)
      if (LFRCO->CFG & _LFRCO_CFG_HIGHPRECEN_MASK) {
        *precision = 500;
      } else {
        *precision = 0xFFFF;
        return SL_STATUS_NOT_AVAILABLE;
      }
#else
      *precision = 0xFFFF;
      return SL_STATUS_NOT_AVAILABLE;
#endif
      break;

    case SL_OSCILLATOR_FSRCO:
    case SL_OSCILLATOR_HFRCODPLL:
    case SL_OSCILLATOR_ULFRCO:
#if defined(HFRCOEM23_PRESENT)
    case SL_OSCILLATOR_HFRCOEM23:
#endif
      *precision = 0xFFFF;
      return SL_STATUS_NOT_AVAILABLE;

#if defined(RFFPLL_PRESENT)
    case SL_OSCILLATOR_RFFPLL:
      *precision = 0xFFFF;
      return SL_STATUS_NOT_AVAILABLE;
#endif

#if defined(USBPLL_PRESENT)
    case SL_OSCILLATOR_USBPLL:
      *precision = 0xFFFF;
      return SL_STATUS_NOT_AVAILABLE;
#endif

    default:
      *precision = 0;
      return SL_STATUS_INVALID_PARAMETER;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Gets frequency of given clock branch.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_clock_branch_frequency(sl_clock_branch_t clock_branch,
                                                             uint32_t *frequency)
{
  CORE_DECLARE_IRQ_STATE;

  sl_status_t return_status = SL_STATUS_OK;

  CORE_ENTER_ATOMIC();

  switch (clock_branch) {
    case SL_CLOCK_BRANCH_SYSCLK:
      *frequency = SystemSYSCLKGet();
      break;

    case SL_CLOCK_BRANCH_HCLK:
      *frequency = CMU_ClockFreqGet(cmuClock_HCLK);
      break;

    case SL_CLOCK_BRANCH_PCLK:
      *frequency = CMU_ClockFreqGet(cmuClock_PCLK);
      break;

    case SL_CLOCK_BRANCH_LSPCLK:
      *frequency = CMU_ClockFreqGet(cmuClock_LSPCLK);
      break;

#if defined(_CMU_TRACECLKCTRL_MASK)
    case SL_CLOCK_BRANCH_TRACECLK:
      *frequency = CMU_ClockFreqGet(cmuClock_TRACECLK);
      break;
#endif
#if defined(_CMU_EXPORTCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_EXPORTCLK:
      *frequency = CMU_ClockFreqGet(cmuClock_EXPCLK);
      break;
#endif
#if defined(_CMU_EM01GRPACLKCTRL_MASK)
    case SL_CLOCK_BRANCH_EM01GRPACLK:
      *frequency = CMU_ClockFreqGet(cmuClock_EM01GRPACLK);
      break;
#endif
#if defined(_CMU_EM01GRPBCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_EM01GRPBCLK:
      *frequency = CMU_ClockFreqGet(cmuClock_EM01GRPBCLK);
      break;
#endif
#if defined(_CMU_EM01GRPCCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_EM01GRPCCLK:
      *frequency = CMU_ClockFreqGet(cmuClock_EM01GRPCCLK);
      break;
#endif
#if defined(_CMU_EM23GRPACLKCTRL_MASK)
    case SL_CLOCK_BRANCH_EM23GRPACLK:
      *frequency = CMU_ClockFreqGet(cmuClock_EM23GRPACLK);
      break;
#endif
#if defined(_CMU_EM4GRPACLKCTRL_MASK)
    case SL_CLOCK_BRANCH_EM4GRPACLK:
      *frequency = CMU_ClockFreqGet(cmuClock_EM4GRPACLK);
      break;
#endif
#if defined(_CMU_IADCCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_IADCCLK:
      *frequency = CMU_ClockFreqGet(cmuClock_IADCCLK);
      break;
#endif
#if defined(_CMU_WDOG0CLKCTRL_MASK)
    case SL_CLOCK_BRANCH_WDOG0CLK:
      *frequency = CMU_ClockFreqGet(cmuClock_WDOG0CLK);
      break;
#endif
#if defined(_CMU_WDOG1CLKCTRL_MASK)
    case SL_CLOCK_BRANCH_WDOG1CLK:
      *frequency = CMU_ClockFreqGet(cmuClock_WDOG1CLK);
      break;
#endif
#if defined(_CMU_RTCCCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_RTCCCLK:
      *frequency = CMU_ClockFreqGet(cmuClock_RTCCCLK);
      break;
#endif
#if defined(_CMU_SYSRTC0CLKCTRL_MASK)
    case SL_CLOCK_BRANCH_SYSRTCCLK:
      *frequency = CMU_ClockFreqGet(cmuClock_SYSRTCCLK);
      break;
#endif
#if defined(_CMU_EUART0CLKCTRL_MASK)
    case SL_CLOCK_BRANCH_EUART0CLK:
      *frequency = CMU_ClockFreqGet(cmuClock_EUART0CLK);
      break;
#endif
#if defined(_CMU_EUSART0CLKCTRL_MASK)
    case SL_CLOCK_BRANCH_EUSART0CLK:
      *frequency = CMU_ClockFreqGet(cmuClock_EUSART0CLK);
      break;
#endif
#if defined(LCD_PRESENT) && defined(_CMU_LCDCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_LCDCLK:
      *frequency = CMU_ClockFreqGet(cmuClock_LCDCLK);
      break;
#endif
#if defined(_CMU_PCNT0CLKCTRL_MASK)
    case SL_CLOCK_BRANCH_PCNT0CLK:
      *frequency = CMU_ClockFreqGet(cmuClock_PCNT0CLK);
      break;
#endif

    case SL_CLOCK_BRANCH_SYSTICKCLK:
      *frequency = CMU_ClockFreqGet(cmuClock_SYSTICK);
      break;

#if defined(_CMU_LESENSEHFCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_LESENSEHFCLK:
      *frequency = CMU_ClockFreqGet(cmuClock_LESENSEHFCLK);
      break;
#endif
#if defined(_CMU_VDAC0CLKCTRL_MASK)
    case SL_CLOCK_BRANCH_VDAC0CLK:
      *frequency = CMU_ClockFreqGet(cmuClock_VDAC0CLK);
      break;
#endif
#if defined(_CMU_VDAC1CLKCTRL_MASK)
    case SL_CLOCK_BRANCH_VDAC1CLK:
      *frequency = CMU_ClockFreqGet(cmuClock_VDAC1CLK);
      break;
#endif
#if defined(_CMU_USB0CLKCTRL_MASK)
    case SL_CLOCK_BRANCH_USB0CLK:
      *frequency = CMU_ClockFreqGet(cmuClock_USB);
      break;
#endif
#if defined(_CMU_DPLLREFCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_DPLLREFCLK:
      *frequency = CMU_ClockFreqGet(cmuClock_DPLLREFCLK);
      break;
#endif
    default:
      *frequency = 0U;
      return_status = SL_STATUS_INVALID_PARAMETER;
      break;
  }

  CORE_EXIT_ATOMIC();

  return return_status;
}

/***************************************************************************//**
 * Gets precision of given clock branch.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_clock_branch_precision(sl_clock_branch_t clock_branch,
                                                             uint16_t *precision)
{
  CORE_DECLARE_IRQ_STATE;
  sl_status_t return_status = SL_STATUS_OK;

  CORE_ENTER_ATOMIC();

  switch (clock_branch) {
    case SL_CLOCK_BRANCH_SYSCLK:
    case SL_CLOCK_BRANCH_HCLK:
    case SL_CLOCK_BRANCH_PCLK:
    case SL_CLOCK_BRANCH_LSPCLK:
    case SL_CLOCK_BRANCH_EXPORTCLK:
      *precision = CMU_HF_ClockPrecisionGet(cmuClock_SYSCLK);
      if (*precision == 0xFFFF) {
        return_status = SL_STATUS_NOT_AVAILABLE;
      }
      break;

#if defined(_CMU_TRACECLKCTRL_MASK)
    case SL_CLOCK_BRANCH_TRACECLK:
#if defined(_CMU_TRACECLKCTRL_CLKSEL_MASK)
      *precision = 0U;
      return_status = SL_STATUS_NOT_SUPPORTED;
#else
      *precision = CMU_HF_ClockPrecisionGet(cmuClock_SYSCLK);
      if (*precision == 0xFFFF) {
        return_status = SL_STATUS_NOT_AVAILABLE;
      }
#endif
      break;

#endif
#if defined(_CMU_EM01GRPACLKCTRL_MASK)
    case SL_CLOCK_BRANCH_EM01GRPACLK:
      *precision = CMU_HF_ClockPrecisionGet(cmuClock_EM01GRPACLK);
      if (*precision == 0xFFFF) {
        return_status = SL_STATUS_NOT_AVAILABLE;
      }
      break;
#endif
#if defined(_CMU_EM01GRPBCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_EM01GRPBCLK:
      *precision = CMU_HF_ClockPrecisionGet(cmuClock_EM01GRPBCLK);
      if (*precision == 0xFFFF) {
        return_status =  SL_STATUS_NOT_AVAILABLE;
      }
      break;
#endif
#if defined(_CMU_EM01GRPCCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_EM01GRPCCLK:
      *precision = CMU_HF_ClockPrecisionGet(cmuClock_EM01GRPCCLK);
      if (*precision == 0xFFFF) {
        return_status =  SL_STATUS_NOT_AVAILABLE;
      }
      break;
#endif
#if defined(_CMU_EM23GRPACLKCTRL_MASK)
    case SL_CLOCK_BRANCH_EM23GRPACLK:
      *precision = CMU_LF_ClockPrecisionGet(cmuClock_EM23GRPACLK);
      if (*precision == 0xFFFF) {
        return_status =  SL_STATUS_NOT_AVAILABLE;
      }
      break;
#endif
#if defined(_CMU_EM4GRPACLKCTRL_MASK)
    case SL_CLOCK_BRANCH_EM4GRPACLK:
      *precision = CMU_LF_ClockPrecisionGet(cmuClock_EM4GRPACLK);
      if (*precision == 0xFFFF) {
        return_status =  SL_STATUS_NOT_AVAILABLE;
      }
      break;
#endif
#if defined(_CMU_IADCCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_IADCCLK:
      *precision = 0U;
      return_status =  SL_STATUS_NOT_SUPPORTED;
      break;
#endif
#if defined(_CMU_WDOG0CLKCTRL_MASK)
    case SL_CLOCK_BRANCH_WDOG0CLK:
      *precision = 0U;
      return_status =  SL_STATUS_NOT_SUPPORTED;
      break;
#endif
#if defined(_CMU_WDOG1CLKCTRL_MASK)
    case SL_CLOCK_BRANCH_WDOG1CLK:
      *precision = 0U;
      return_status =  SL_STATUS_NOT_SUPPORTED;
      break;
#endif
#if defined(_CMU_RTCCCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_RTCCCLK:
      *precision = CMU_LF_ClockPrecisionGet(cmuClock_RTCCCLK);
      if (*precision == 0xFFFF) {
        return_status =  SL_STATUS_NOT_AVAILABLE;
      }
      break;
#endif
#if defined(_CMU_SYSRTC0CLKCTRL_MASK)
    case SL_CLOCK_BRANCH_SYSRTCCLK:
      *precision = CMU_LF_ClockPrecisionGet(cmuClock_SYSRTCCLK);
      if (*precision == 0xFFFF) {
        return_status =  SL_STATUS_NOT_AVAILABLE;
      }
      break;
#endif
#if defined(_CMU_EUART0CLKCTRL_MASK)
    case SL_CLOCK_BRANCH_EUART0CLK:
      *precision = 0U;
      return_status =  SL_STATUS_NOT_SUPPORTED;
      break;
#endif
#if defined(_CMU_EUSART0CLKCTRL_MASK)
    case SL_CLOCK_BRANCH_EUSART0CLK:
      *precision = 0U;
      return_status =  SL_STATUS_NOT_SUPPORTED;
      break;
#endif
#if defined(LCD_PRESENT) && defined(_CMU_LCDCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_LCDCLK:
      *precision = CMU_LF_ClockPrecisionGet(cmuClock_LCDCLK);
      if (*precision == 0xFFFF) {
        return_status =  SL_STATUS_NOT_AVAILABLE;
      }
      break;
#endif
#if defined(_CMU_PCNT0CLKCTRL_MASK)
    case SL_CLOCK_BRANCH_PCNT0CLK:
      *precision = 0U;
      return_status =  SL_STATUS_NOT_SUPPORTED;
      break;
#endif

    case SL_CLOCK_BRANCH_SYSTICKCLK:
      *precision = 0U;
      return_status =  SL_STATUS_NOT_SUPPORTED;
      break;

#if defined(_CMU_LESENSEHFCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_LESENSEHFCLK:
      *precision = CMU_HF_ClockPrecisionGet(cmuClock_LESENSEHFCLK);
      if (*precision == 0xFFFF) {
        return_status =  SL_STATUS_NOT_AVAILABLE;
      }
      break;
#endif
#if defined(_CMU_VDAC0CLKCTRL_MASK)
    case SL_CLOCK_BRANCH_VDAC0CLK:
      *precision = 0U;
      return_status =  SL_STATUS_NOT_SUPPORTED;
      break;
#endif
#if defined(_CMU_VDAC1CLKCTRL_MASK)
    case SL_CLOCK_BRANCH_VDAC1CLK:
      *precision = 0U;
      return_status =  SL_STATUS_NOT_SUPPORTED;
      break;
#endif
#if defined(_CMU_USB0CLKCTRL_MASK)
    case SL_CLOCK_BRANCH_USB0CLK:
      *precision = 0U;
      return_status =  SL_STATUS_NOT_SUPPORTED;
      break;
#endif

#if defined(_CMU_DPLLREFCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_DPLLREFCLK:
      *precision = 0U;
      return_status =  SL_STATUS_NOT_SUPPORTED;
      break;
#endif

    default:
      *precision = 0U;
      return_status =  SL_STATUS_INVALID_PARAMETER;
      break;
  }

  CORE_EXIT_ATOMIC();

  return return_status;
}

/***************************************************************************//**
 * Enables/Disables the given module's bus clock.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_enable_bus_clock(sl_bus_clock_t module_bus_clock, bool enable)
{
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG) && (_SILICON_LABS_32B_SERIES_2_CONFIG > 1)
  volatile uint32_t *reg = NULL;
  uint32_t bit;
  uint32_t clken_index;

  if (module_bus_clock == SL_BUS_CLOCK_INVALID) {
    return SL_STATUS_NOT_AVAILABLE;
  }

  bit = (*module_bus_clock & _BUS_CLOCK_CLKEN_BIT_MASK) >> _BUS_CLOCK_CLKEN_BIT_SHIFT;
  clken_index = (*module_bus_clock & _BUS_CLOCK_CLKENX_MASK) >> _BUS_CLOCK_CLKENX_SHIFT;

  if (clken_index == BUS_CLOCK_CLKEN0) {
    reg = &CMU->CLKEN0;
  } else if (clken_index == BUS_CLOCK_CLKEN1) {
    reg = &CMU->CLKEN1;
#if defined(_CMU_CLKEN2_MASK)
  } else if (clken_index == BUS_CLOCK_CLKEN2) {
    reg = &CMU->CLKEN2;
#endif
  } else {
    return SL_STATUS_NOT_AVAILABLE;
  }

  // Enable/disable bus clock.
  BUS_RegBitWrite(reg, bit, (uint32_t)enable);
#else // defined(_SILICON_LABS_32B_SERIES_2_CONFIG) && (_SILICON_LABS_32B_SERIES_2_CONFIG > 1)
  (void)enable;
  (void)module_bus_clock;
#endif

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Configures the exported clock feature on CMU to output user selected
 * clock source specified GPIO pin.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_set_gpio_clock_output(sl_clock_manager_export_clock_source_t export_clock_source,
                                                        sl_clock_manager_export_clock_output_select_t output_select,
                                                        uint16_t divider,
                                                        uint32_t port,
                                                        uint32_t pin)
{
  CMU_Select_TypeDef cmu_clock_select;
  CORE_DECLARE_IRQ_STATE;

  switch (export_clock_source) {
    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_DISABLED:
      cmu_clock_select = cmuSelect_Disabled;
      break;
#if defined(FSRCO_PRESENT)
    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_FSRCO:
      cmu_clock_select = cmuSelect_FSRCO;
      break;
#endif
#if defined(HFXO_PRESENT)
    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_HFXO:
      cmu_clock_select = cmuSelect_HFXO;
      break;
#endif
#if defined(DPLL_PRESENT)
    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_HFRCODPLL:
      cmu_clock_select = cmuSelect_HFRCODPLL;
      break;
#endif
#if defined(HFRCOEM23_PRESENT)
    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_HFRCOEM23:
      cmu_clock_select = cmuSelect_HFRCOEM23;
      break;
#endif
    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_HFEXPCLK:
      cmu_clock_select  = cmuSelect_EXPCLK;
      break;
#if defined(LFXO_PRESENT)
    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_LFXO:
      cmu_clock_select = cmuSelect_LFXO;
      break;
#endif
#if defined(PLFRCO_PRESENT)
    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_PLFRCO:
      cmu_clock_select = cmuSelect_PLFRCO;
      break;
#endif
#if defined(LFRCO)
    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_LFRCO:
      cmu_clock_select = cmuSelect_LFRCO;
      break;
#endif
#if defined(ULFRCO)
    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_ULFRCO:
      cmu_clock_select = cmuSelect_ULFRCO;
      break;
#endif
    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_HCLK:
      cmu_clock_select = cmuSelect_HCLK;
      break;

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }

  CORE_ENTER_ATOMIC();

  CMU_ClkOutPinConfig((uint32_t)output_select, cmu_clock_select, (CMU_ClkDiv_TypeDef)divider, port, pin);

  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Sets the RC oscillator frequency tuning control.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_set_rc_oscillator_calibration(sl_oscillator_t oscillator,
                                                                uint32_t val)
{
  switch (oscillator) {
    case SL_OSCILLATOR_HFRCODPLL:
      CMU_OscillatorTuningSet(cmuOsc_HFRCODPLL, val);
      break;

#if defined(HFRCOEM23_PRESENT)
    case SL_OSCILLATOR_HFRCOEM23:
      CMU_OscillatorTuningSet(cmuOsc_HFRCOEM23, val);
      break;
#endif

    case SL_OSCILLATOR_LFRCO:
      CMU_OscillatorTuningSet(cmuOsc_LFRCO, val);
      break;

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Gets the RC oscillator frequency tuning setting.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_rc_oscillator_calibration(sl_oscillator_t oscillator,
                                                                uint32_t *val)
{
  switch (oscillator) {
    case SL_OSCILLATOR_HFRCODPLL:
      *val = CMU_OscillatorTuningGet(cmuOsc_HFRCODPLL);
      break;

#if defined(HFRCOEM23_PRESENT)
    case SL_OSCILLATOR_HFRCOEM23:
      *val = CMU_OscillatorTuningGet(cmuOsc_HFRCOEM23);
      break;
#endif

    case SL_OSCILLATOR_LFRCO:
      *val = CMU_OscillatorTuningGet(cmuOsc_LFRCO);
      break;

    default:
      *val = 0;
      return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Sets HFXO calibration value.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_set_hfxo_calibration(uint32_t val)
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();

  CMU_OscillatorTuningSet(cmuOsc_HFXO, val);

  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Gets the HFXO calibration value.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_hfxo_calibration(uint32_t *val)
{
  *val = CMU_OscillatorTuningGet(cmuOsc_HFXO);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Sets the HFXO CTUNE setting.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_hfxo_set_ctune(uint32_t ctune)
{
  CORE_DECLARE_IRQ_STATE;
  sl_status_t return_status = SL_STATUS_OK;

  CORE_ENTER_ATOMIC();
  return_status = CMU_HFXOCTuneSet(ctune);
  CORE_EXIT_ATOMIC();

  return return_status;
}

/***************************************************************************//**
 * Gets the HFXO CTUNE setting.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_hfxo_get_ctune(uint32_t *ctune)
{
  *ctune = CMU_HFXOCTuneGet();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Updates the tuning capacitances and calibrate the Core Bias Current.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_hfxo_calibrate_ctune(uint32_t ctune)
{
  uint32_t hfxo_ctrl_backup = HFXO0->CTRL;

  // Having the FORCEEN here prevents the enabling and disabling of HFXO in
  // each function calls.
  HFXO0->CTRL_SET = HFXO_CTRL_FORCEEN;
  while ((HFXO0->STATUS & (HFXO_STATUS_COREBIASOPTRDY | HFXO_STATUS_RDY | HFXO_STATUS_ENS))
         != (HFXO_STATUS_COREBIASOPTRDY | HFXO_STATUS_RDY | HFXO_STATUS_ENS)) {
    // Wait for crystal to startup
  }

  sl_status_t status = CMU_HFXOCTuneSet(ctune);

  if (status == SL_STATUS_OK) {
    CMU_HFXOCoreBiasCurrentCalibrate();
  }

  BUS_RegMaskedWrite(&HFXO0->CTRL, _HFXO_CTRL_FORCEEN_MASK, hfxo_ctrl_backup);

  return status;
}

/***************************************************************************//**
 * Gets the HFXO's average startup time.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_hfxo_average_startup_time(uint32_t *val)
{
  (void) val;
  return SL_STATUS_NOT_SUPPORTED; // Not supported on Series 2
}

/***************************************************************************//**
 * Sets LFXO frequency tuning control.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_set_lfxo_calibration(uint32_t val)
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();

  CMU_OscillatorTuningSet(cmuOsc_LFXO, val);

  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Gets the LFXO frequency tuning setting.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_lfxo_calibration(uint32_t *val)
{
  *val = CMU_OscillatorTuningGet(cmuOsc_LFXO);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Configures the RCO calibration.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_configure_rco_calibration(uint32_t cycles,
                                                            sl_clock_manager_clock_calibration_t down_counter_selection,
                                                            sl_clock_manager_clock_calibration_t up_counter_selection,
                                                            bool continuous_calibration)
{
  CMU_Select_TypeDef down_selection;
  CMU_Select_TypeDef up_selection;

  switch (down_counter_selection) {
    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_HFXO:
      down_selection = cmuSelect_HFXO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_LFXO:
      down_selection = cmuSelect_LFXO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_HFRCODPLL:
      down_selection = cmuSelect_HFRCODPLL;
      break;

#if defined(HFRCOEM23_PRESENT)
    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_HFRCOEM23:
      down_selection = cmuSelect_HFRCOEM23;
      break;
#endif

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_FSRCO:
      down_selection = cmuSelect_FSRCO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_LFRCO:
      down_selection = cmuSelect_LFRCO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_ULFRCO:
      down_selection = cmuSelect_ULFRCO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_PRS:
      down_selection = cmuSelect_PRS;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_HCLK:
      down_selection = cmuSelect_HCLK;
      break;

    default:
      return SL_STATUS_NOT_AVAILABLE;
  }

  switch (up_counter_selection) {
    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_HFXO:
      up_selection = cmuSelect_HFXO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_LFXO:
      up_selection = cmuSelect_LFXO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_HFRCODPLL:
      up_selection = cmuSelect_HFRCODPLL;
      break;

#if defined(HFRCOEM23_PRESENT)
    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_HFRCOEM23:
      up_selection = cmuSelect_HFRCOEM23;
      break;
#endif

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_FSRCO:
      up_selection =  cmuSelect_FSRCO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_LFRCO:
      up_selection = cmuSelect_LFRCO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_ULFRCO:
      up_selection = cmuSelect_ULFRCO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_PRS:
      up_selection = cmuSelect_PRS;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_HCLK:
      return SL_STATUS_NOT_SUPPORTED;

    default:
      return SL_STATUS_NOT_AVAILABLE;
  }

  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();

  CMU_CalibrateConfig(cycles, down_selection, up_selection);
  CMU_CalibrateCont(continuous_calibration);

  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Starts the RCO calibration.
 ******************************************************************************/
void sli_clock_manager_hal_start_rco_calibration(void)
{
  CMU_CalibrateStart();
}

/***************************************************************************//**
 * Stops the RCO calibration.
 ******************************************************************************/
void sli_clock_manager_hal_stop_rco_calibration(void)
{
  CMU_CalibrateStop();
}

/***************************************************************************//**
 * Waits for the RCO calibration to finish.
 ******************************************************************************/
void sli_clock_manager_hal_wait_rco_calibration(void)
{
  // Wait until calibration completes, UNLESS continuous calibration mode is on
  if ((CMU->CALCTRL & CMU_CALCTRL_CONT) == 0UL) {
    // Wait until calibration completes
    while ((CMU->STATUS & CMU_STATUS_CALRDY) == 0UL) ;
  }
}

/***************************************************************************//**
 * Gets calibration count value.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_rco_calibration_count(uint32_t *count)
{
  *count = CMU->CALCNT;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Updates SYSCLK clock source clock.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_set_sysclk_source(sl_oscillator_t source)
{
  CORE_DECLARE_IRQ_STATE;
  sl_status_t return_status = SL_STATUS_OK;

  CORE_ENTER_ATOMIC();

  switch (source) {
    case SL_OSCILLATOR_FSRCO:
      sli_em_cmu_SYSCLKInitPreClockSelect();
      CMU->SYSCLKCTRL = (CMU->SYSCLKCTRL & ~_CMU_SYSCLKCTRL_CLKSEL_MASK) | CMU_SYSCLKCTRL_CLKSEL_FSRCO;
      sli_em_cmu_SYSCLKInitPostClockSelect(false);
      break;
    case SL_OSCILLATOR_HFXO:
      sli_em_cmu_SYSCLKInitPreClockSelect();
      CMU->SYSCLKCTRL = (CMU->SYSCLKCTRL & ~_CMU_SYSCLKCTRL_CLKSEL_MASK) | CMU_SYSCLKCTRL_CLKSEL_HFXO;
      sli_em_cmu_SYSCLKInitPostClockSelect(false);
      break;
    case SL_OSCILLATOR_HFRCODPLL:
      sli_em_cmu_SYSCLKInitPreClockSelect();
      CMU->SYSCLKCTRL = (CMU->SYSCLKCTRL & ~_CMU_SYSCLKCTRL_CLKSEL_MASK) | CMU_SYSCLKCTRL_CLKSEL_HFRCODPLL;
      sli_em_cmu_SYSCLKInitPostClockSelect(false);
      break;
#if defined(CMU_SYSCLKCTRL_CLKSEL_RFFPLL0SYS)
    case SL_OSCILLATOR_RFFPLL:
      sli_em_cmu_SYSCLKInitPreClockSelect();
      CMU->SYSCLKCTRL = (CMU->SYSCLKCTRL & ~_CMU_SYSCLKCTRL_CLKSEL_MASK) | CMU_SYSCLKCTRL_CLKSEL_RFFPLL0SYS;
      sli_em_cmu_SYSCLKInitPostClockSelect(false);
      break;
#endif
    case SL_OSCILLATOR_CLKIN0:
      sli_em_cmu_SYSCLKInitPreClockSelect();
      CMU->SYSCLKCTRL = (CMU->SYSCLKCTRL & ~_CMU_SYSCLKCTRL_CLKSEL_MASK) | CMU_SYSCLKCTRL_CLKSEL_CLKIN0;
      sli_em_cmu_SYSCLKInitPostClockSelect(false);
      break;
    default:
      return_status = SL_STATUS_INVALID_PARAMETER;
      break;
  }

  SystemCoreClockUpdate();
  CORE_EXIT_ATOMIC();

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  slx_power_manager_update_clock_info();
#endif

  return return_status;
}

/***************************************************************************//**
 * Gets SYSCLK clock source clock.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_sysclk_source(sl_oscillator_t *source)
{
  switch (CMU->SYSCLKCTRL & _CMU_SYSCLKCTRL_CLKSEL_MASK) {
    case  CMU_SYSCLKCTRL_CLKSEL_FSRCO:
      *source = SL_OSCILLATOR_FSRCO;
      break;
    case  CMU_SYSCLKCTRL_CLKSEL_HFXO:
      *source = SL_OSCILLATOR_HFXO;
      break;
    case  CMU_SYSCLKCTRL_CLKSEL_HFRCODPLL:
      *source = SL_OSCILLATOR_HFRCODPLL;
      break;
    case  CMU_SYSCLKCTRL_CLKSEL_CLKIN0:
      *source = SL_OSCILLATOR_CLKIN0;
      break;
#if defined(CMU_SYSCLKCTRL_CLKSEL_RFFPLL0SYS)
    case CMU_SYSCLKCTRL_CLKSEL_RFFPLL0SYS:
      *source = SL_OSCILLATOR_RFFPLL;
      break;
#endif
    default:
      EFM_ASSERT(false);
      break;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Waits for USBPLL clock to be ready.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_wait_usbpll(void)
{
#if defined(USBPLL_PRESENT)
  while ((USBPLL0->STATUS & (USBPLL_STATUS_PLLRDY | USBPLL_STATUS_PLLLOCK))
         != (USBPLL_STATUS_PLLRDY | USBPLL_STATUS_PLLLOCK)) {
    // Wait for USB PLL lock and ready
  }
  return SL_STATUS_OK;
#else
  return SL_STATUS_NOT_AVAILABLE;
#endif
}

/***************************************************************************//**
 * Sets the external FLASH reference clock.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_set_ext_flash_clk(sl_oscillator_t oscillator)
{
  (void)oscillator;
  return SL_STATUS_NOT_SUPPORTED;
}

/***************************************************************************//**
 * Gets the external FLASH clock source.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_ext_flash_clk(sl_oscillator_t *oscillator)
{
  if (oscillator == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  (void)oscillator;
  return SL_STATUS_NOT_SUPPORTED;
}

/***************************************************************************//**
 * Retrieves the FREQPLAN NWP SOCPLL config.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_nwp_socpll_freqplan_config(const uint16_t **socpll_freqplan_config,
                                                                 uint8_t *target_frequency_index)
{
  (void) socpll_freqplan_config;
  (void) target_frequency_index;
  return SL_STATUS_NOT_SUPPORTED;
}
