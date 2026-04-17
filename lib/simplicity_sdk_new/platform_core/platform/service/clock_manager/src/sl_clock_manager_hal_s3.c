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

#include "em_device.h"

#include <stddef.h>
#include "sl_clock_manager.h"
#include "sli_clock_manager.h"
#include "sli_clock_manager_hal.h"
#include "sl_common.h"
#include "sl_core.h"
#include "sl_gpio.h"
#include "sl_hal_bus.h"
#include "sl_se_manager_extmem.h"
#include "sl_se_manager_util.h"
#include "sl_se_manager.h"
#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif
#if defined(SL_CATALOG_CLOCK_MANAGER_PRESENT)
#include "sl_clock_manager_oscillator_config.h"
#endif
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif
#if defined(CLOCK_MANAGER_INIT_HAL_INTERNAL_PRESENT)
#include "sli_clock_manager_init_hal_internal.h"
#endif

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#define CLOCK_MANAGER_SOCPLL_FREQ_PRECISION_PPM   41U
#define CLOCK_MANAGER_PERPLL_FREQ_PRECISION_PPM   3U

#define HFXO_IRQ_NUMBER  HFXO0_IRQn
#define HFXO_IRQ_HANDLER_FUNCTION  HFXO0_IRQHandler

#if defined(_HFXO_BUFOUTCTRL_MASK)
#define HFXO_CTUNE_DELTA 40
#else
#define HFXO_CTUNE_DELTA 0
#endif

/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/

// LFXO precision.
uint16_t clock_manager_lfxo_precision = 0;
// HFXO precision.
uint16_t clock_manager_hfxo_precision = 0;

#if defined (_SILICON_LABS_32B_SERIES_3_CONFIG_301)
// FLPLL configurations.
sli_se_qspi_flpll_config_t clock_manager_flpll_cfg = {
  .ref_clock = SLI_SE_QSPI_REF_CLOCK_HFRCODPLL,
};
sl_se_command_context_t clock_manager_cmd_ctx;
// Device SE FW version
uint32_t clock_manager_se_version = 0;
// Current QSPI reference clock.
sl_oscillator_t current_qspi_reference_clock = SL_OSCILLATOR_INVALID;
#endif

// HFXO startup time.
#if defined(SL_CLOCK_MANAGER_HFXO_STARTUP_TIME_MEASUREMENT_EN) && SL_CLOCK_MANAGER_HFXO_STARTUP_TIME_MEASUREMENT_EN
uint16_t clock_manager_hfxo_startup_time = 0;
#endif

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

/*******************************************************************************
 **********************   GLOBAL INTERNAL FUNCTIONS   **************************
 ******************************************************************************/

/***************************************************************************//**
 * Performs Clock Manager runtime initialization.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_runtime_init(void)
{
  // Enable HFXO bus clock.
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_HFXO0);

  // Set the external FLASH reference clock to FLPLL with HFXO.
#if defined (_SILICON_LABS_32B_SERIES_3_CONFIG_301)
  sli_clock_manager_hal_set_ext_flash_clk(SL_OSCILLATOR_FLPLL);
#endif

  // Enable HFXO RDY Interrupt.
  HFXO0->IEN_CLR = HFXO_IEN_RDY;
  HFXO0->IF_CLR = HFXO_IF_RDY;

  NVIC_ClearPendingIRQ(HFXO_IRQ_NUMBER);
  NVIC_EnableIRQ(HFXO_IRQ_NUMBER);

  HFXO0->IEN_SET = HFXO_IEN_RDY;

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

    case SL_OSCILLATOR_HFRCOEM23:
      *frequency = SystemHFRCOEM23ClockGet();
      break;

    case SL_OSCILLATOR_LFXO:
      *frequency = SystemLFXOClockGet();
      break;

    case SL_OSCILLATOR_LFRCO:
      *frequency = SystemLFRCOClockGet();
      break;

    case SL_OSCILLATOR_ULFRCO:
      *frequency = SystemULFRCOClockGet();
      break;

#if defined(_SOCPLL_SOCCLK0_MASK)
    case SL_OSCILLATOR_SOCPLL0_OUT0:
      *frequency = SystemSOCPLLClockGet(0, 0);
      break;

    case SL_OSCILLATOR_SOCPLL0_OUT1:
      *frequency = SystemSOCPLLClockGet(0, 1);
      break;

    case SL_OSCILLATOR_SOCPLL0_OUT2:
      *frequency = SystemSOCPLLClockGet(0, 2);
      break;
#else
    case SL_OSCILLATOR_SOCPLL0:
      *frequency = SystemSOCPLLClockGet(0, 0);
      break;
#endif

#if defined(SOCPLL1)
    case SL_OSCILLATOR_SOCPLL1_OUT0:
      *frequency = SystemSOCPLLClockGet(1, 0);
      break;

    case SL_OSCILLATOR_SOCPLL1_OUT1:
      *frequency = SystemSOCPLLClockGet(1, 1);
      break;

    case SL_OSCILLATOR_SOCPLL1_OUT2:
      *frequency = SystemSOCPLLClockGet(1, 2);
      break;
#endif

#if defined(SOCPLL2)
    case SL_OSCILLATOR_SOCPLL2_OUT0:
      *frequency = SystemSOCPLLClockGet(2, 0);
      break;

    case SL_OSCILLATOR_SOCPLL2_OUT1:
      *frequency = SystemSOCPLLClockGet(2, 1);
      break;

    case SL_OSCILLATOR_SOCPLL2_OUT2:
      *frequency = SystemSOCPLLClockGet(2, 2);
      break;
#endif

#if defined(PERPLL_PRESENT)
    case SL_OSCILLATOR_PERPLL0:
      *frequency = SystemPERPLLClockGet(0);
      break;
#if (PERPLL_COUNT > 1)
    case SL_OSCILLATOR_PERPLL1:
      *frequency = SystemPERPLLClockGet(1);
      break;
#endif
#endif

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
    case SL_OSCILLATOR_LFXO:
      // If precision is not set, LFXO has not been initialized
      if (clock_manager_lfxo_precision == 0) {
        return SL_STATUS_FAIL;
      }
      *precision = clock_manager_lfxo_precision;
      break;

    case SL_OSCILLATOR_HFXO:
      // If precision is not set, HFXO has not been initialized
      if (clock_manager_hfxo_precision == 0) {
        return SL_STATUS_FAIL;
      }
      *precision = clock_manager_hfxo_precision;
      break;

    case SL_OSCILLATOR_LFRCO:
      if ((LFRCO->CFG & _LFRCO_CFG_HIGHPRECEN_MASK) != 0) {
        *precision = 500;
      } else {
        *precision = 0xFFFF;
        return SL_STATUS_NOT_AVAILABLE;
      }
      break;

#if defined(_SOCPLL_SOCCLK0_MASK)
    case SL_OSCILLATOR_SOCPLL0_OUT0:
    case SL_OSCILLATOR_SOCPLL0_OUT1:
    case SL_OSCILLATOR_SOCPLL0_OUT2:
#else
    case SL_OSCILLATOR_SOCPLL0:
#endif
#if defined(SOCPLL1)
    case SL_OSCILLATOR_SOCPLL1_OUT0:
    case SL_OSCILLATOR_SOCPLL1_OUT1:
    case SL_OSCILLATOR_SOCPLL1_OUT2:
#endif
#if defined(SOCPLL2)
    case SL_OSCILLATOR_SOCPLL2_OUT0:
    case SL_OSCILLATOR_SOCPLL2_OUT1:
    case SL_OSCILLATOR_SOCPLL2_OUT2:
#endif
      *precision = CLOCK_MANAGER_SOCPLL_FREQ_PRECISION_PPM;
      break;

#if defined(PERPLL_PRESENT)
    case SL_OSCILLATOR_PERPLL0:
#if (PERPLL_COUNT > 1)
    case SL_OSCILLATOR_PERPLL1:
#endif
      *precision = CLOCK_MANAGER_PERPLL_FREQ_PRECISION_PPM;
      break;
#endif

    case SL_OSCILLATOR_FSRCO:
    case SL_OSCILLATOR_HFRCODPLL:
    case SL_OSCILLATOR_ULFRCO:
    case SL_OSCILLATOR_HFRCOEM23:
      *precision = 0xFFFF;
      return SL_STATUS_NOT_AVAILABLE;

    default:
      *precision = 0U;
      return SL_STATUS_INVALID_PARAMETER;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Enables/Disables the given module's bus clock..
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_enable_bus_clock(sl_bus_clock_t module_bus_clock, bool enable)
{
  volatile uint32_t *reg = NULL;
  uint32_t bit;

#if defined (_SILICON_LABS_32B_SERIES_3_CONFIG_301)
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
#if defined(_CMU_CLKENHV_MASK)
  } else if (clken_index == BUS_CLOCK_CLKENHV) {
    reg = &CMU->CLKENHV;
#endif
  } else {
    return SL_STATUS_NOT_AVAILABLE;
  }
#else
  reg = (uint32_t *)(*module_bus_clock);
  if (reg == SL_BUS_CLOCK_INVALID || reg == (uint32_t *)0xFFFFFFFF) {
    return SL_STATUS_NOT_AVAILABLE;
  }

  // The CLKEN field is always the first bit of the register
  bit = 0;
#endif

  // Enable/disable bus clock.
  sl_hal_bus_reg_write_bit(reg, bit, (uint32_t)enable);

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
  uint32_t tmp = 0;
  uint32_t mask;
  CORE_DECLARE_IRQ_STATE;

  EFM_ASSERT(pin <= (_GPIO_CMU_CLKOUT0ROUTE_PIN_MASK >> _GPIO_CMU_CLKOUT0ROUTE_PIN_SHIFT));

  sl_gpio_t gpio;
  gpio.port = (sl_gpio_port_t)port;
  gpio.pin = (uint8_t)pin;

  switch (output_select) {
    case SL_CLOCK_MANAGER_EXPORT_CLOCK_OUTPUT_SELECT_0:
    case SL_CLOCK_MANAGER_EXPORT_CLOCK_OUTPUT_SELECT_1:
      EFM_ASSERT((port == 2U) || (port == 3U));
      break;
    case SL_CLOCK_MANAGER_EXPORT_CLOCK_OUTPUT_SELECT_2:
      EFM_ASSERT((port == 0U) || (port == 1U));
      break;
    default:
      EFM_ASSERT(false);
      break;
  }

  switch (export_clock_source) {
    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_DISABLED:
      tmp = CMU_EXPORTCLKCTRL_CLKOUTSEL0_DISABLED;
      break;

    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_FSRCO:
      tmp = CMU_EXPORTCLKCTRL_CLKOUTSEL0_FSRCO;
      break;

    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_HFXO:
      tmp = CMU_EXPORTCLKCTRL_CLKOUTSEL0_HFXO;
      break;

    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_HFRCODPLL:
      tmp = CMU_EXPORTCLKCTRL_CLKOUTSEL0_HFRCODPLL;
      break;

    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_HFRCOEM23:
      tmp = CMU_EXPORTCLKCTRL_CLKOUTSEL0_HFRCOEM23;
      break;

    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_HFEXPCLK:
      EFM_ASSERT((divider > 0U) && (divider <= 32U));
      tmp  = CMU_EXPORTCLKCTRL_CLKOUTSEL0_HFEXPCLK;
      break;

    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_LFXO:
      tmp = CMU_EXPORTCLKCTRL_CLKOUTSEL0_LFXO;
      break;

    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_LFRCO:
      tmp = CMU_EXPORTCLKCTRL_CLKOUTSEL0_LFRCO;
      break;

    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_ULFRCO:
      tmp = CMU_EXPORTCLKCTRL_CLKOUTSEL0_ULFRCO;
      break;

    case SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_HCLK:
      tmp = CMU_EXPORTCLKCTRL_CLKOUTSEL0_HCLK;
      break;

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }

  mask  = _CMU_EXPORTCLKCTRL_CLKOUTSEL0_MASK << (output_select * _CMU_EXPORTCLKCTRL_CLKOUTSEL1_SHIFT);
  tmp <<= output_select * _CMU_EXPORTCLKCTRL_CLKOUTSEL1_SHIFT;

  if (export_clock_source == SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_HFEXPCLK) {
    tmp  |= (divider - 1U) << _CMU_EXPORTCLKCTRL_PRESC_SHIFT;
    mask |= _CMU_EXPORTCLKCTRL_PRESC_MASK;
  }

  CORE_ENTER_ATOMIC();

  CMU->EXPORTCLKCTRL = (CMU->EXPORTCLKCTRL & ~mask) | tmp;

  if (export_clock_source == SL_CLOCK_MANAGER_EXPORT_CLOCK_SOURCE_DISABLED) {
    GPIO->CMUROUTE_CLR.ROUTEEN = GPIO_CMU_ROUTEEN_CLKOUT0PEN << output_select;
    sl_gpio_set_pin_mode(&gpio, SL_GPIO_MODE_DISABLED, false);
  } else {
    GPIO->CMUROUTE_SET.ROUTEEN = GPIO_CMU_ROUTEEN_CLKOUT0PEN << output_select;
    if (output_select == SL_CLOCK_MANAGER_EXPORT_CLOCK_OUTPUT_SELECT_0) {
      GPIO->CMUROUTE.CLKOUT0ROUTE = (port << _GPIO_CMU_CLKOUT0ROUTE_PORT_SHIFT)
                                    | (pin << _GPIO_CMU_CLKOUT0ROUTE_PIN_SHIFT);
    } else if (output_select == SL_CLOCK_MANAGER_EXPORT_CLOCK_OUTPUT_SELECT_1) {
      GPIO->CMUROUTE.CLKOUT1ROUTE = (port << _GPIO_CMU_CLKOUT1ROUTE_PORT_SHIFT)
                                    | (pin << _GPIO_CMU_CLKOUT1ROUTE_PIN_SHIFT);
    } else {
      GPIO->CMUROUTE.CLKOUT2ROUTE = (port << _GPIO_CMU_CLKOUT2ROUTE_PORT_SHIFT)
                                    | (pin << _GPIO_CMU_CLKOUT2ROUTE_PIN_SHIFT);
    }
    sl_gpio_set_pin_mode(&gpio, SL_GPIO_MODE_PUSH_PULL, false);
  }

  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Gets frequency of given clock branch.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_clock_branch_frequency(sl_clock_branch_t clock_branch,
                                                             uint32_t *frequency)
{
  uint8_t pclk_divider;
  uint8_t per_divider;
  sl_status_t return_status = SL_STATUS_OK;
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();

  switch (clock_branch) {
    case SL_CLOCK_BRANCH_SYSCLK:
      *frequency = SystemSYSCLKGet();
      break;

    case SL_CLOCK_BRANCH_HCLK:
      *frequency = SystemHCLKGet();
      break;

    case SL_CLOCK_BRANCH_PCLK:
      pclk_divider = ((CMU->SYSCLKCTRL & _CMU_SYSCLKCTRL_PCLKPRESC_MASK) >> _CMU_SYSCLKCTRL_PCLKPRESC_SHIFT) + 1;
      *frequency = SystemHCLKGet() / pclk_divider;
      break;

    case SL_CLOCK_BRANCH_LSPCLK:
      pclk_divider = ((CMU->SYSCLKCTRL & _CMU_SYSCLKCTRL_PCLKPRESC_MASK) >> _CMU_SYSCLKCTRL_PCLKPRESC_SHIFT) + 1;
      *frequency = SystemHCLKGet() / pclk_divider / 2U;
      break;

    case SL_CLOCK_BRANCH_TRACECLK:
      per_divider = ((CMU->TRACECLKCTRL & _CMU_TRACECLKCTRL_PRESC_MASK) >> _CMU_TRACECLKCTRL_PRESC_SHIFT) + 1;
      switch (CMU->TRACECLKCTRL & _CMU_TRACECLKCTRL_CLKSEL_MASK) {
        case CMU_TRACECLKCTRL_CLKSEL_SYSCLK:
          *frequency = SystemSYSCLKGet() / per_divider;
          break;

        case CMU_TRACECLKCTRL_CLKSEL_HFRCOEM23:
          *frequency = SystemHFRCOEM23ClockGet() / per_divider;
          break;

        case CMU_TRACECLKCTRL_CLKSEL_HFRCODPLLRT:
          *frequency = SystemHFRCODPLLClockGet() / per_divider;
          break;

        default:
          *frequency = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_EXPORTCLK:
      *frequency = SystemSYSCLKGet() / (((CMU->EXPORTCLKCTRL & _CMU_EXPORTCLKCTRL_PRESC_MASK) >> _CMU_EXPORTCLKCTRL_PRESC_SHIFT) + 1);
      break;

    case SL_CLOCK_BRANCH_EM01GRPACLK:
      switch (CMU->EM01GRPACLKCTRL & _CMU_EM01GRPACLKCTRL_CLKSEL_MASK) {
        case CMU_EM01GRPACLKCTRL_CLKSEL_HFRCODPLL:
        case CMU_EM01GRPACLKCTRL_CLKSEL_HFRCODPLLRT:
          *frequency = SystemHFRCODPLLClockGet();
          break;

        case CMU_EM01GRPACLKCTRL_CLKSEL_HFXO:
        case CMU_EM01GRPACLKCTRL_CLKSEL_HFXORT:
          *frequency = SystemHFXOClockGet();
          break;

        case CMU_EM01GRPACLKCTRL_CLKSEL_HFRCOEM23:
          *frequency = SystemHFRCOEM23ClockGet();
          break;

        case CMU_EM01GRPACLKCTRL_CLKSEL_FSRCO:
          *frequency = SystemFSRCOClockGet();
          break;

        default:
          *frequency = 0U;
          return_status = SL_STATUS_INVALID_STATE;
      }
      break;

    case SL_CLOCK_BRANCH_EM01GRPCCLK:
      switch (CMU->EM01GRPCCLKCTRL & _CMU_EM01GRPCCLKCTRL_CLKSEL_MASK) {
        case CMU_EM01GRPCCLKCTRL_CLKSEL_HFRCODPLL:
        case CMU_EM01GRPCCLKCTRL_CLKSEL_HFRCODPLLRT:
          *frequency = SystemHFRCODPLLClockGet();
          break;

        case CMU_EM01GRPCCLKCTRL_CLKSEL_HFRCOEM23:
          *frequency = SystemHFRCOEM23ClockGet();
          break;

        case CMU_EM01GRPCCLKCTRL_CLKSEL_HFXO:
        case CMU_EM01GRPCCLKCTRL_CLKSEL_HFXORT:
          *frequency = SystemHFXOClockGet();
          break;

        case CMU_EM01GRPCCLKCTRL_CLKSEL_FSRCO:
          *frequency = SystemFSRCOClockGet();
          break;

        default:
          *frequency = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_EM23GRPACLK:
      switch (CMU->EM23GRPACLKCTRL & _CMU_EM23GRPACLKCTRL_CLKSEL_MASK) {
        case CMU_EM23GRPACLKCTRL_CLKSEL_LFRCO:
          *frequency = SystemLFRCOClockGet();
          break;

        case CMU_EM23GRPACLKCTRL_CLKSEL_LFXO:
          *frequency = SystemLFXOClockGet();
          break;

        case CMU_EM23GRPACLKCTRL_CLKSEL_ULFRCO:
          *frequency = SystemULFRCOClockGet();
          break;

        default:
          *frequency = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_EM4GRPACLK:
      switch (CMU->EM4GRPACLKCTRL & _CMU_EM4GRPACLKCTRL_CLKSEL_MASK) {
        case CMU_EM4GRPACLKCTRL_CLKSEL_LFRCO:
          *frequency = SystemLFRCOClockGet();
          break;

        case CMU_EM4GRPACLKCTRL_CLKSEL_LFXO:
          *frequency = SystemLFXOClockGet();
          break;

        case CMU_EM4GRPACLKCTRL_CLKSEL_ULFRCO:
          *frequency = SystemULFRCOClockGet();
          break;

        default:
          *frequency = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_WDOG0CLK:
      switch (CMU->WDOG0CLKCTRL & _CMU_WDOG0CLKCTRL_CLKSEL_MASK) {
        case CMU_WDOG0CLKCTRL_CLKSEL_LFRCO:
          *frequency = SystemLFRCOClockGet();
          break;

        case CMU_WDOG0CLKCTRL_CLKSEL_LFXO:
          *frequency = SystemLFXOClockGet();
          break;

        case CMU_WDOG0CLKCTRL_CLKSEL_ULFRCO:
          *frequency = SystemULFRCOClockGet();
          break;

        case CMU_WDOG0CLKCTRL_CLKSEL_HCLKDIV1024:
          *frequency = SystemHCLKGet() / 1024U;
          break;

        default:
          *frequency = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_WDOG1CLK:
      *frequency = SystemHCLKGet() / 1024U;
      break;

    case SL_CLOCK_BRANCH_SYSRTCCLK:
      switch (CMU->SYSRTC0CLKCTRL & _CMU_SYSRTC0CLKCTRL_CLKSEL_MASK) {
        case CMU_SYSRTC0CLKCTRL_CLKSEL_LFRCO:
          *frequency = SystemLFRCOClockGet();
          break;

        case CMU_SYSRTC0CLKCTRL_CLKSEL_LFXO:
          *frequency = SystemLFXOClockGet();
          break;

        case CMU_SYSRTC0CLKCTRL_CLKSEL_ULFRCO:
          *frequency = SystemULFRCOClockGet();
          break;

        default:
          *frequency = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_EUSART0CLK:
      switch (CMU->EUSART0CLKCTRL & _CMU_EUSART0CLKCTRL_CLKSEL_MASK) {
        case CMU_EUSART0CLKCTRL_CLKSEL_EM01GRPCCLK:
          return_status =  sli_clock_manager_hal_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM01GRPCCLK, frequency);
          break;

        case CMU_EUSART0CLKCTRL_CLKSEL_HFRCOEM23:
          *frequency = SystemHFRCOEM23ClockGet();
          break;

        case CMU_EUSART0CLKCTRL_CLKSEL_LFRCO:
          *frequency = SystemLFRCOClockGet();
          break;

        case CMU_EUSART0CLKCTRL_CLKSEL_LFXO:
          *frequency = SystemLFXOClockGet();
          break;

        default:
          *frequency = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_PCNT0CLK:
      switch (CMU->PCNT0CLKCTRL & _CMU_PCNT0CLKCTRL_CLKSEL_MASK) {
        case CMU_PCNT0CLKCTRL_CLKSEL_EM23GRPACLK:
          return_status = sli_clock_manager_hal_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM23GRPACLK, frequency);
          break;

        case CMU_PCNT0CLKCTRL_CLKSEL_PCNTS0:
          *frequency = 0U;
          return_status = SL_STATUS_NOT_SUPPORTED;
          break;

        default:
          *frequency = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_EM01GRPDCLK:
      switch (CMU->EM01GRPDCLKCTRL & _CMU_EM01GRPDCLKCTRL_CLKSEL_MASK) {
        case CMU_EM01GRPDCLKCTRL_CLKSEL_HFRCODPLL:
          *frequency = SystemHFRCODPLLClockGet();
          break;

        case CMU_EM01GRPDCLKCTRL_CLKSEL_HFXO:
          *frequency = SystemHFXOClockGet();
          break;

        case CMU_EM01GRPDCLKCTRL_CLKSEL_FSRCO:
          *frequency = SystemFSRCOClockGet();
          break;

        case CMU_EM01GRPDCLKCTRL_CLKSEL_HFRCOEM23:
          *frequency = SystemHFRCOEM23ClockGet();
          break;

        default:
          *frequency = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_I2C0CLK:
      switch (CMU->I2C0CLKCTRL & _CMU_I2C0CLKCTRL_CLKSEL_MASK) {
        case CMU_I2C0CLKCTRL_CLKSEL_EM01GRPDCLK:
          return_status = sli_clock_manager_hal_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM01GRPDCLK, frequency);
          break;

        case CMU_I2C0CLKCTRL_CLKSEL_HFRCOEM23:
          *frequency = SystemHFRCOEM23ClockGet();
          break;

        case CMU_I2C0CLKCTRL_CLKSEL_LFRCO:
          *frequency = SystemLFRCOClockGet();
          break;

        case CMU_I2C0CLKCTRL_CLKSEL_LFXO:
          *frequency = SystemLFXOClockGet();
          break;

        default:
          *frequency = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

#if defined(_CMU_PIXELRZCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_PIXELRZCLK:
      per_divider = ((CMU->PIXELRZCLKCTRL & _CMU_PIXELRZCLKCTRL_PRESC_MASK) >> _CMU_PIXELRZCLKCTRL_PRESC_SHIFT) + 1;
      switch (CMU->PIXELRZCLKCTRL & _CMU_PIXELRZCLKCTRL_CLKSEL_MASK) {
        case CMU_PIXELRZCLKCTRL_CLKSEL_HFRCODPLL:
          *frequency = SystemHFRCODPLLClockGet() / per_divider;
          break;

        case CMU_PIXELRZCLKCTRL_CLKSEL_HFXO:
          *frequency = SystemHFXOClockGet() / per_divider;
          break;

        case CMU_PIXELRZCLKCTRL_CLKSEL_FSRCO:
          *frequency = SystemFSRCOClockGet() / per_divider;
          break;

        case CMU_PIXELRZCLKCTRL_CLKSEL_HFRCOEM23:
          *frequency = SystemHFRCOEM23ClockGet() / per_divider;
          break;

        default:
          *frequency = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;
#endif

    case SL_CLOCK_BRANCH_ADCCLK:
      switch (CMU->ADCCLKCTRL & _CMU_ADCCLKCTRL_CLKSEL_MASK) {
        case CMU_ADCCLKCTRL_CLKSEL_EM01GRPACLK:
          return_status =  sli_clock_manager_hal_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM01GRPACLK, frequency);
          break;

        case CMU_ADCCLKCTRL_CLKSEL_FSRCO:
          *frequency = SystemFSRCOClockGet();
          break;

        case CMU_ADCCLKCTRL_CLKSEL_HFRCOEM23:
          *frequency = SystemHFRCOEM23ClockGet();
          break;

        default:
          *frequency = 0U;
          return_status =  SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_SYSTICKCLK:
    {
      if (SysTick->CTRL & SysTick_CTRL_CLKSOURCE_Msk) {
        // Systick connected to processor clock when CLKSOURCE bit is set: HCLK
        return_status = sli_clock_manager_hal_get_clock_branch_frequency(SL_CLOCK_BRANCH_HCLK, frequency);
      } else {
        // Systick connected to external clock when CLKSOURCE bit is clear: EM23GRPACLK
        return_status = sli_clock_manager_hal_get_clock_branch_frequency(SL_CLOCK_BRANCH_EM23GRPACLK, frequency);
      }
      break;
    }

    case SL_CLOCK_BRANCH_DPLLREFCLK:
      switch (CMU->DPLLREFCLKCTRL & _CMU_DPLLREFCLKCTRL_CLKSEL_MASK) {
        case CMU_DPLLREFCLKCTRL_CLKSEL_HFXO:
          *frequency = SystemHFXOClockGet();
          break;

        case CMU_DPLLREFCLKCTRL_CLKSEL_LFXO:
          *frequency = SystemLFXOClockGet();
          break;

        case CMU_DPLLREFCLKCTRL_CLKSEL_CLKIN0:
          *frequency = SystemCLKIN0Get();
          break;

        default:
          *frequency = 0U;
          return_status =  SL_STATUS_INVALID_STATE;
          break;
      }
      break;

#if defined(_CMU_QSPISYSCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_QSPISYSCLK:
      per_divider = ((CMU->QSPISYSCLKCTRL & _CMU_QSPISYSCLKCTRL_PRESC_MASK) >> _CMU_QSPISYSCLKCTRL_PRESC_SHIFT) + 1;
      switch (CMU->QSPISYSCLKCTRL & _CMU_QSPISYSCLKCTRL_CLKSEL_MASK) {
        case CMU_QSPISYSCLKCTRL_CLKSEL_HFRCODPLL:
          *frequency = SystemHFRCODPLLClockGet() / per_divider;
          break;

        case CMU_QSPISYSCLKCTRL_CLKSEL_HFXO:
          *frequency = SystemHFXOClockGet() / per_divider;
          break;

        case CMU_QSPISYSCLKCTRL_CLKSEL_SOCPLL:
          *frequency = SystemSOCPLLClockGet(0, 0) / per_divider;
          break;

        default:
          *frequency = 0U;
          (void)per_divider;
          return_status =  SL_STATUS_INVALID_STATE;
          break;
      }
      break;
#endif

#if defined(_CMU_FLPLLREFCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_FLPLLREFCLK:
      per_divider = ((CMU->FLPLLREFCLKCTRL & _CMU_FLPLLREFCLKCTRL_PRESC_MASK) >> _CMU_FLPLLREFCLKCTRL_PRESC_SHIFT) + 1;
      switch (CMU->FLPLLREFCLKCTRL & _CMU_FLPLLREFCLKCTRL_CLKSEL_MASK) {
        case CMU_FLPLLREFCLKCTRL_CLKSEL_HFRCODPLLRT:
          *frequency = SystemHFRCODPLLClockGet() / per_divider;
          break;

        case CMU_FLPLLREFCLKCTRL_CLKSEL_CLKIN0:
          *frequency = SystemCLKIN0Get() / per_divider;
          break;

        default:
          *frequency = 0U;
          (void)per_divider;
          return_status =  SL_STATUS_INVALID_STATE;
          break;
      }
      break;
#endif

    default:
#if defined(CLOCK_MANAGER_RUNTIME_HAL_INTERNAL_PRESENT)
      return_status = sli_clock_manager_hal_get_clock_branch_frequency_internal(clock_branch, frequency);
#else
      *frequency = 0U;
      return_status = SL_STATUS_INVALID_PARAMETER;
#endif
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
  sl_status_t return_status = SL_STATUS_FAIL;
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();

  switch (clock_branch) {
    case SL_CLOCK_BRANCH_SYSCLK:
    case SL_CLOCK_BRANCH_HCLK:
    case SL_CLOCK_BRANCH_PCLK:
    case SL_CLOCK_BRANCH_LSPCLK:
    case SL_CLOCK_BRANCH_EXPORTCLK:
      switch (CMU->SYSCLKCTRL & _CMU_SYSCLKCTRL_CLKSEL_MASK) {
        case CMU_SYSCLKCTRL_CLKSEL_HFXO:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_HFXO, precision);
          break;

        case CMU_SYSCLKCTRL_CLKSEL_HFRCODPLL:
        case CMU_SYSCLKCTRL_CLKSEL_FSRCO:
          *precision = 0xFFFF;
          return_status = SL_STATUS_NOT_AVAILABLE;
          break;

#if defined(_SOCPLL_SOCCLK0_MASK)
        case CMU_SYSCLKCTRL_CLKSEL_SOCPLL:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_SOCPLL0_OUT1, precision);
          break;
#else
        case CMU_SYSCLKCTRL_CLKSEL_SOCPLL:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_SOCPLL0, precision);
          break;
#endif

#if defined(CMU_SYSCLKCTRL_CLKSEL_SOCPLL1)
        case CMU_SYSCLKCTRL_CLKSEL_SOCPLL1:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_SOCPLL1_OUT1, precision);
          break;
#endif

        default:
          *precision = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_TRACECLK:
      switch (CMU->TRACECLKCTRL & _CMU_TRACECLKCTRL_CLKSEL_MASK) {
        case CMU_TRACECLKCTRL_CLKSEL_SYSCLK:
          return_status = sli_clock_manager_hal_get_clock_branch_precision(SL_CLOCK_BRANCH_SYSCLK, precision);
          break;

        case CMU_TRACECLKCTRL_CLKSEL_HFRCOEM23:
        case CMU_TRACECLKCTRL_CLKSEL_HFRCODPLLRT:
        case CMU_TRACECLKCTRL_CLKSEL_DISABLE:
          *precision = 0xFFFF;
          return_status = SL_STATUS_NOT_AVAILABLE;
          break;

        default:
          *precision = 0U;
          return_status =  SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_EM01GRPACLK:
      switch (CMU->EM01GRPACLKCTRL & _CMU_EM01GRPACLKCTRL_CLKSEL_MASK) {
        case CMU_EM01GRPACLKCTRL_CLKSEL_HFXO:
        case CMU_EM01GRPACLKCTRL_CLKSEL_HFXORT:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_HFXO, precision);
          break;

        case CMU_EM01GRPACLKCTRL_CLKSEL_HFRCODPLL:
        case CMU_EM01GRPACLKCTRL_CLKSEL_HFRCODPLLRT:
        case CMU_EM01GRPACLKCTRL_CLKSEL_HFRCOEM23:
        case CMU_EM01GRPACLKCTRL_CLKSEL_FSRCO:
          *precision = 0xFFFF;
          return_status = SL_STATUS_NOT_AVAILABLE;
          break;

        default:
          *precision = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_EM01GRPCCLK:
      switch (CMU->EM01GRPCCLKCTRL & _CMU_EM01GRPCCLKCTRL_CLKSEL_MASK) {
        case CMU_EM01GRPCCLKCTRL_CLKSEL_HFXO:
        case CMU_EM01GRPCCLKCTRL_CLKSEL_HFXORT:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_HFXO, precision);
          break;

        case CMU_EM01GRPCCLKCTRL_CLKSEL_HFRCODPLL:
        case CMU_EM01GRPCCLKCTRL_CLKSEL_HFRCODPLLRT:
        case CMU_EM01GRPCCLKCTRL_CLKSEL_HFRCOEM23:
        case CMU_EM01GRPCCLKCTRL_CLKSEL_FSRCO:
          *precision = 0xFFFF;
          return_status = SL_STATUS_NOT_AVAILABLE;
          break;

        default:
          *precision = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_EM23GRPACLK:
      switch (CMU->EM23GRPACLKCTRL & _CMU_EM23GRPACLKCTRL_CLKSEL_MASK) {
        case CMU_EM23GRPACLKCTRL_CLKSEL_LFXO:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_LFXO, precision);
          break;

        case CMU_EM23GRPACLKCTRL_CLKSEL_LFRCO:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_LFRCO, precision);
          break;

        case CMU_EM23GRPACLKCTRL_CLKSEL_ULFRCO:
          *precision = 0xFFFF;
          return_status = SL_STATUS_NOT_AVAILABLE;
          break;

        default:
          *precision = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_EM4GRPACLK:
      switch (CMU->EM4GRPACLKCTRL & _CMU_EM4GRPACLKCTRL_CLKSEL_MASK) {
        case CMU_EM4GRPACLKCTRL_CLKSEL_LFXO:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_LFXO, precision);
          break;

        case CMU_EM4GRPACLKCTRL_CLKSEL_LFRCO:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_LFRCO, precision);
          break;

        case CMU_EM4GRPACLKCTRL_CLKSEL_ULFRCO:
          *precision = 0xFFFF;
          return_status = SL_STATUS_NOT_AVAILABLE;
          break;

        default:
          *precision = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_WDOG0CLK:
      switch (CMU->WDOG0CLKCTRL & _CMU_WDOG0CLKCTRL_CLKSEL_MASK) {
        case CMU_WDOG0CLKCTRL_CLKSEL_LFXO:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_LFXO, precision);
          break;

        case CMU_WDOG0CLKCTRL_CLKSEL_LFRCO:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_LFRCO, precision);
          break;

        case CMU_WDOG0CLKCTRL_CLKSEL_HCLKDIV1024:
          return_status = sli_clock_manager_hal_get_clock_branch_precision(SL_CLOCK_BRANCH_HCLK, precision);
          break;

        case CMU_WDOG0CLKCTRL_CLKSEL_ULFRCO:
          *precision = 0xFFFF;
          return_status = SL_STATUS_NOT_AVAILABLE;
          break;

        default:
          *precision = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_WDOG1CLK:
      return_status = sli_clock_manager_hal_get_clock_branch_precision(SL_CLOCK_BRANCH_HCLK, precision);
      break;

    case SL_CLOCK_BRANCH_SYSRTCCLK:
      switch (CMU->SYSRTC0CLKCTRL & _CMU_SYSRTC0CLKCTRL_CLKSEL_MASK) {
        case CMU_SYSRTC0CLKCTRL_CLKSEL_LFXO:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_LFXO, precision);
          break;

        case CMU_SYSRTC0CLKCTRL_CLKSEL_LFRCO:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_LFRCO, precision);
          break;

        case CMU_SYSRTC0CLKCTRL_CLKSEL_ULFRCO:
          *precision = 0xFFFF;
          return_status = SL_STATUS_NOT_AVAILABLE;
          break;

        default:
          *precision = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_EUSART0CLK:
      switch (CMU->EUSART0CLKCTRL & _CMU_EUSART0CLKCTRL_CLKSEL_MASK) {
        case CMU_EUSART0CLKCTRL_CLKSEL_EM01GRPCCLK:
          return_status = sli_clock_manager_hal_get_clock_branch_precision(SL_CLOCK_BRANCH_EM01GRPCCLK, precision);
          break;

        case CMU_EUSART0CLKCTRL_CLKSEL_HFRCOEM23:
          *precision = 0xFFFF;
          return_status = SL_STATUS_NOT_AVAILABLE;
          break;

        case CMU_EUSART0CLKCTRL_CLKSEL_LFRCO:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_LFRCO, precision);
          break;

        case CMU_EUSART0CLKCTRL_CLKSEL_LFXO:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_LFXO, precision);
          break;

        default:
          *precision = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_PCNT0CLK:
      switch (CMU->PCNT0CLKCTRL & _CMU_PCNT0CLKCTRL_CLKSEL_MASK) {
        case CMU_PCNT0CLKCTRL_CLKSEL_EM23GRPACLK:
          return_status = sli_clock_manager_hal_get_clock_branch_precision(SL_CLOCK_BRANCH_EM23GRPACLK, precision);
          break;

        case CMU_PCNT0CLKCTRL_CLKSEL_PCNTS0:
          *precision = 0;
          return_status = SL_STATUS_NOT_SUPPORTED;
          break;

        default:
          *precision = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

    case SL_CLOCK_BRANCH_EM01GRPDCLK:
      switch (CMU->EM01GRPDCLKCTRL & _CMU_EM01GRPDCLKCTRL_CLKSEL_MASK) {
        case CMU_EM01GRPDCLKCTRL_CLKSEL_HFXO:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_HFXO, precision);
          break;

        case CMU_EM01GRPDCLKCTRL_CLKSEL_HFRCODPLL:
        case CMU_EM01GRPDCLKCTRL_CLKSEL_HFRCOEM23:
        case CMU_EM01GRPDCLKCTRL_CLKSEL_FSRCO:
          *precision = 0xFFFF;
          return_status = SL_STATUS_NOT_AVAILABLE;
          break;

        default:
          *precision = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

#if defined(_CMU_QSPISYSCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_QSPISYSCLK:
      switch (CMU->QSPISYSCLKCTRL & _CMU_QSPISYSCLKCTRL_CLKSEL_MASK) {
        case CMU_QSPISYSCLKCTRL_CLKSEL_HFXO:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_HFXO, precision);
          break;

        case CMU_QSPISYSCLKCTRL_CLKSEL_HFRCODPLL:
          *precision = 0xFFFF;
          return_status = SL_STATUS_NOT_AVAILABLE;
          break;

        case CMU_QSPISYSCLKCTRL_CLKSEL_SOCPLL:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_SOCPLL0, precision);
          break;

        default:
          *precision = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;
#endif

    case SL_CLOCK_BRANCH_I2C0CLK:
      switch (CMU->I2C0CLKCTRL & _CMU_I2C0CLKCTRL_CLKSEL_MASK) {
        case CMU_I2C0CLKCTRL_CLKSEL_EM01GRPDCLK:
          return_status = sli_clock_manager_hal_get_clock_branch_precision(SL_CLOCK_BRANCH_EM01GRPDCLK, precision);
          break;

        case CMU_I2C0CLKCTRL_CLKSEL_HFRCOEM23:
          *precision = 0xFFFF;
          return_status = SL_STATUS_NOT_AVAILABLE;
          break;

        case CMU_I2C0CLKCTRL_CLKSEL_LFRCO:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_LFRCO, precision);
          break;

        case CMU_I2C0CLKCTRL_CLKSEL_LFXO:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_LFXO, precision);
          break;

        default:
          *precision = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;

#if defined(_CMU_PIXELRZCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_PIXELRZCLK:
      switch (CMU->PIXELRZCLKCTRL & _CMU_PIXELRZCLKCTRL_CLKSEL_MASK) {
        case CMU_PIXELRZCLKCTRL_CLKSEL_HFXO:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_HFXO, precision);
          break;

        case CMU_PIXELRZCLKCTRL_CLKSEL_HFRCODPLL:
        case CMU_PIXELRZCLKCTRL_CLKSEL_FSRCO:
        case CMU_PIXELRZCLKCTRL_CLKSEL_HFRCOEM23:
          *precision = 0xFFFF;
          return_status = SL_STATUS_NOT_AVAILABLE;
          break;

        default:
          *precision = 0U;
          return_status = SL_STATUS_INVALID_STATE;
          break;
      }
      break;
#endif

    case SL_CLOCK_BRANCH_SYSTICKCLK:
      if (SysTick->CTRL & SysTick_CTRL_CLKSOURCE_Msk) {
        // Systick connected to processor clock when CLKSOURCE bit is set: HCLK
        return_status = sli_clock_manager_hal_get_clock_branch_precision(SL_CLOCK_BRANCH_HCLK, precision);
      } else {
        // Systick connected to external clock when CLKSOURCE bit is clear: EM23GRPACLK
        return_status = sli_clock_manager_hal_get_clock_branch_precision(SL_CLOCK_BRANCH_EM23GRPACLK, precision);
      }
      break;

    case SL_CLOCK_BRANCH_DPLLREFCLK:
      switch (CMU->DPLLREFCLKCTRL & _CMU_DPLLREFCLKCTRL_CLKSEL_MASK) {
        case CMU_DPLLREFCLKCTRL_CLKSEL_HFXO:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_HFXO, precision);
          break;

        case CMU_DPLLREFCLKCTRL_CLKSEL_LFXO:
          return_status = sli_clock_manager_hal_get_oscillator_precision(SL_OSCILLATOR_LFXO, precision);
          break;

        case CMU_DPLLREFCLKCTRL_CLKSEL_CLKIN0:
          *precision = 0xFFFF;
          return_status = SL_STATUS_NOT_AVAILABLE;
          break;

        default:
          *precision = 0U;
          return_status =  SL_STATUS_INVALID_STATE;
          break;
      }
      break;

#if defined(_CMU_FLPLLREFCLKCTRL_MASK)
    case SL_CLOCK_BRANCH_FLPLLREFCLK:
      switch (CMU->FLPLLREFCLKCTRL & _CMU_FLPLLREFCLKCTRL_CLKSEL_MASK) {
        case CMU_FLPLLREFCLKCTRL_CLKSEL_HFRCODPLLRT:
          *precision = 0xFFFF;
          return_status = SL_STATUS_NOT_AVAILABLE;
          break;

        case CMU_FLPLLREFCLKCTRL_CLKSEL_CLKIN0:
          *precision = 0XFFFF;
          return_status = SL_STATUS_NOT_AVAILABLE;
          break;

        default:
          *precision = 0U;
          return_status =  SL_STATUS_INVALID_STATE;
          break;
      }
      break;
#endif

    default:
#if defined(CLOCK_MANAGER_RUNTIME_HAL_INTERNAL_PRESENT)
      return_status = sli_clock_manager_hal_get_clock_branch_precision_internal(clock_branch, precision);
#else
      *precision = 0U;
      return_status = SL_STATUS_INVALID_PARAMETER;
#endif
      break;
  }

  CORE_EXIT_ATOMIC();

  return return_status;
}

/***************************************************************************//**
 * Gets the RC oscillator frequency tuning setting.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_rc_oscillator_calibration(sl_oscillator_t oscillator,
                                                                uint32_t *val)
{
  switch (oscillator) {
    case SL_OSCILLATOR_HFRCODPLL:
      sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_HFRCO0);
      *val = (HFRCO0->CAL & _HFRCO_CAL_TUNING_MASK) >> _HFRCO_CAL_TUNING_SHIFT;
      break;

    case SL_OSCILLATOR_HFRCOEM23:
      *val = (HFRCOEM23->CAL & _HFRCO_CAL_TUNING_MASK) >> _HFRCO_CAL_TUNING_SHIFT;
      break;

#if defined(_LFRCO_CAL_FREQTRIM_MASK)
    case SL_OSCILLATOR_LFRCO:
      CMU->CLKEN0_SET = CMU_CLKEN0_LFRCO;
      *val = (LFRCO->CAL & _LFRCO_CAL_FREQTRIM_MASK) >> _LFRCO_CAL_FREQTRIM_SHIFT;
      break;
#endif

    default:
      *val = 0;
      return SL_STATUS_INVALID_PARAMETER;
  }

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
      sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_HFRCO0);
      EFM_ASSERT(val <= (_HFRCO_CAL_TUNING_MASK >> _HFRCO_CAL_TUNING_SHIFT));
      val &= _HFRCO_CAL_TUNING_MASK >> _HFRCO_CAL_TUNING_SHIFT;
      while ((HFRCO0->STATUS & HFRCO_STATUS_SYNCBUSY) != 0UL) {
        // Wait for any ongoing synchronization to finish.
      }
      HFRCO0->CAL = (HFRCO0->CAL & ~_HFRCO_CAL_TUNING_MASK)
                    | (val << _HFRCO_CAL_TUNING_SHIFT);
      break;

    case SL_OSCILLATOR_HFRCOEM23:
      EFM_ASSERT(val <= (_HFRCO_CAL_TUNING_MASK >> _HFRCO_CAL_TUNING_SHIFT));
      val &= _HFRCO_CAL_TUNING_MASK >> _HFRCO_CAL_TUNING_SHIFT;
      while ((HFRCOEM23->STATUS & HFRCO_STATUS_SYNCBUSY) != 0UL) {
        // Wait for any ongoing synchronization to finish.
      }
      HFRCOEM23->CAL = (HFRCOEM23->CAL & ~_HFRCO_CAL_TUNING_MASK)
                       | (val << _HFRCO_CAL_TUNING_SHIFT);
      break;

#if defined(_LFRCO_CAL_FREQTRIM_MASK)
    case SL_OSCILLATOR_LFRCO:
      CMU->CLKEN0_SET = CMU_CLKEN0_LFRCO;
      EFM_ASSERT(val <= (_LFRCO_CAL_FREQTRIM_MASK
                         >> _LFRCO_CAL_FREQTRIM_SHIFT));
      val &= _LFRCO_CAL_FREQTRIM_MASK >> _LFRCO_CAL_FREQTRIM_SHIFT;
      while (LFRCO->SYNCBUSY != 0U) {
        // Wait for any ongoing synchronization to finish.
      }
      LFRCO->CAL = (LFRCO->CAL & ~_LFRCO_CAL_FREQTRIM_MASK)
                   | (val << _LFRCO_CAL_FREQTRIM_SHIFT);
      break;
#endif

    default:
      return SL_STATUS_INVALID_PARAMETER;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Sets HFXO calibration value.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_set_hfxo_calibration(uint32_t val)
{
  CORE_DECLARE_IRQ_STATE;
  bool disondemand = false;
  sl_status_t status = SL_STATUS_OK;

#if defined (_SILICON_LABS_32B_SERIES_3_CONFIG_301)
  sl_oscillator_t initial_qspi_reference_clock;

  // Get current QSPI reference clock.
  status = sli_clock_manager_hal_get_ext_flash_clk(&initial_qspi_reference_clock);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Switch QSPI clock to FSRCO.
  status = sli_clock_manager_hal_set_ext_flash_clk(SL_OSCILLATOR_FSRCO);
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif

  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_HFXO0);
  EFM_ASSERT(val <= (_HFXO_XTALCTRL_COREBIASANA_MASK >> _HFXO_XTALCTRL_COREBIASANA_SHIFT));

  // Make sure HFXO is disabled.
  EFM_ASSERT((HFXO0->STATUS & HFXO_STATUS_ENS) == 0);

  CORE_ENTER_ATOMIC();

  // Set DISONDEMAND if not already set and wait for FSMLOCK to be clear so that
  // software can write to register
  disondemand = (HFXO0->CTRL & _HFXO_CTRL_DISONDEMAND_MASK) >> _HFXO_CTRL_DISONDEMAND_SHIFT;
  if (disondemand == false) {
    HFXO0->CTRL_SET = HFXO_CTRL_DISONDEMAND;
  }

  // Update Core Bias Ana setting and enable Optimization skip
  HFXO0->XTALCTRL = (HFXO0->XTALCTRL & ~_HFXO_XTALCTRL_COREBIASANA_MASK)
                    | (val << _HFXO_XTALCTRL_COREBIASANA_SHIFT)
                    | HFXO_XTALCTRL_SKIPCOREBIASOPT;
  // Clear back DISONDEMAND if needed
  if (disondemand == false) {
    HFXO0->CTRL_CLR = HFXO_CTRL_DISONDEMAND;
  }

#if defined (_SILICON_LABS_32B_SERIES_3_CONFIG_301)
  // Switch QSPI clock to initial reference clock.
  status = sli_clock_manager_hal_set_ext_flash_clk(initial_qspi_reference_clock);
#endif

  CORE_EXIT_ATOMIC();

  return status;
}

/***************************************************************************//**
 * Gets the HFXO calibration value.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_hfxo_calibration(uint32_t *val)
{
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_HFXO0);
  *val = (HFXO0->XTALCTRL & _HFXO_XTALCTRL_COREBIASANA_MASK) >> _HFXO_XTALCTRL_COREBIASANA_SHIFT;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Sets the HFXO CTUNE setting.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_hfxo_set_ctune(uint32_t ctune)
{
  sl_status_t status = SL_STATUS_OK;

  CORE_DECLARE_IRQ_STATE;

#if defined (_SILICON_LABS_32B_SERIES_3_CONFIG_301)
  sl_oscillator_t initial_qspi_reference_clock;

  // Get current QSPI reference clock.
  status = sli_clock_manager_hal_get_ext_flash_clk(&initial_qspi_reference_clock);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Switch QSPI clock to FSRCO.
  status = sli_clock_manager_hal_set_ext_flash_clk(SL_OSCILLATOR_FSRCO);
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif

  // Make sure the given CTUNE value is within the allowable range.
  if (ctune > (_HFXO_XTALCTRL_CTUNEXIANA_MASK >> _HFXO_XTALCTRL_CTUNEXIANA_SHIFT)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  uint32_t ctuneXoana = ctune + HFXO_CTUNE_DELTA;
  if (ctuneXoana > (_HFXO_XTALCTRL_CTUNEXOANA_MASK >> _HFXO_XTALCTRL_CTUNEXOANA_SHIFT)) {
    ctuneXoana = _HFXO_XTALCTRL_CTUNEXOANA_MASK >> _HFXO_XTALCTRL_CTUNEXOANA_SHIFT; // Max value
  }

  CORE_ENTER_ATOMIC();

  bool was_locked = ((HFXO0->STATUS & HFXO_STATUS_LOCK_LOCKED) != 0) ? true : false;

  // Unlock register interface.
  HFXO0->LOCK = HFXO_LOCK_LOCKKEY_UNLOCK;

  // Write new CTune value.
  HFXO0->XTALCTRL = ((HFXO0->XTALCTRL & ~(_HFXO_XTALCTRL_CTUNEXOANA_MASK | _HFXO_XTALCTRL_CTUNEXIANA_MASK))
                     | ((ctuneXoana << _HFXO_XTALCTRL_CTUNEXOANA_SHIFT) & _HFXO_XTALCTRL_CTUNEXOANA_MASK)
                     | ((ctune << _HFXO_XTALCTRL_CTUNEXIANA_SHIFT) & _HFXO_XTALCTRL_CTUNEXIANA_MASK));

  // Relock register if it was locked in the first place.
  if (was_locked) {
    HFXO0->LOCK = ~HFXO_LOCK_LOCKKEY_UNLOCK;
  }

#if defined (_SILICON_LABS_32B_SERIES_3_CONFIG_301)
  // Switch QSPI clock to initial reference clock.
  status = sli_clock_manager_hal_set_ext_flash_clk(initial_qspi_reference_clock);
#endif

  CORE_EXIT_ATOMIC();

  return status;
}

/***************************************************************************//**
 * Gets the HFXO CTUNE setting.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_hfxo_get_ctune(uint32_t *ctune)
{
  if (ctune == NULL) {
    EFM_ASSERT(ctune != NULL);
    return SL_STATUS_NULL_POINTER;
  }

  *ctune = ((HFXO0->XTALCTRL & _HFXO_XTALCTRL_CTUNEXIANA_MASK)
            >> _HFXO_XTALCTRL_CTUNEXIANA_SHIFT);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Updates the tuning capacitances and calibrate the Core Bias Current.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_hfxo_calibrate_ctune(uint32_t ctune)
{
  uint32_t hfxo_ctrl_backup = HFXO0->CTRL;
  sl_status_t status = SL_STATUS_OK;

#if defined (_SILICON_LABS_32B_SERIES_3_CONFIG_301)
  sl_oscillator_t initial_qspi_reference_clock;

  // Get current QSPI reference clock.
  status = sli_clock_manager_hal_get_ext_flash_clk(&initial_qspi_reference_clock);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Switch QSPI clock to FSRCO.
  status = sli_clock_manager_hal_set_ext_flash_clk(SL_OSCILLATOR_FSRCO);
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif

  // The FORCEEN and DISONDEMAND bits need to be set to launch the calibration.
  HFXO0->CTRL_SET = HFXO_CTRL_FORCEEN;
  while ((HFXO0->STATUS & (HFXO_STATUS_COREBIASOPTRDY | HFXO_STATUS_RDY)) != (HFXO_STATUS_COREBIASOPTRDY | HFXO_STATUS_RDY)) {
    // Wait for crystal to startup.
  }
  HFXO0->CTRL_SET = HFXO_CTRL_DISONDEMAND;

  while ((HFXO0->STATUS & (HFXO_STATUS_COREBIASOPTRDY | HFXO_STATUS_RDY | HFXO_STATUS_ENS))
         != (HFXO_STATUS_COREBIASOPTRDY | HFXO_STATUS_RDY | HFXO_STATUS_ENS)) {
    // Making sure HFXO is in steady state.
  }

  status = sli_clock_manager_hal_hfxo_set_ctune(ctune);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Start core bias optimization.
  HFXO0->CMD_SET = HFXO_CMD_COREBIASOPT;

  while ((HFXO0->STATUS & HFXO_STATUS_COREBIASOPTRDY) == HFXO_STATUS_COREBIASOPTRDY) {
    // Wait for core bias optimization to start.
  }
  while ((HFXO0->STATUS & HFXO_STATUS_COREBIASOPTRDY) == 0) {
    // Wait for core bias optimization to finish.
  }

  // Retrieve initial state of the CTRL register.
  sl_hal_bus_reg_write_mask(&HFXO0->CTRL, (_HFXO_CTRL_DISONDEMAND_MASK | _HFXO_CTRL_FORCEEN_MASK), hfxo_ctrl_backup);

#if defined (_SILICON_LABS_32B_SERIES_3_CONFIG_301)
  // Switch QSPI clock to initial reference clock.
  status = sli_clock_manager_hal_set_ext_flash_clk(initial_qspi_reference_clock);
#endif

  return status;
}

/***************************************************************************//**
 * Gets the HFXO's average startup time.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_hfxo_average_startup_time(uint32_t *val)
{
#if defined(SL_CLOCK_MANAGER_HFXO_STARTUP_TIME_MEASUREMENT_EN) && SL_CLOCK_MANAGER_HFXO_STARTUP_TIME_MEASUREMENT_EN
  if (clock_manager_hfxo_startup_time != 0) {
    // We got a value, return ok.
    *val = clock_manager_hfxo_startup_time;
    return SL_STATUS_OK;
  } else {
    // Still waiting for a measure, return not ready.
    *val = SL_CLOCK_MANAGER_HFXO_STARTUP_TIME_DEFAULT_VALUE;
    return SL_STATUS_NOT_READY;
  }

#else
  (void) val;
  return SL_STATUS_NOT_SUPPORTED;
#endif
}

/***************************************************************************//**
 * Sets LFXO frequency tuning control.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_set_lfxo_calibration(uint32_t val)
{
  CORE_DECLARE_IRQ_STATE;
  bool lfxo_lock_status = false;
  uint8_t ctune = 0;

  CORE_ENTER_ATOMIC();

  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LFXO);
  lfxo_lock_status = (LFXO->STATUS & _LFXO_STATUS_LOCK_MASK) >> _LFXO_STATUS_LOCK_SHIFT;
  // Unlock register interface if register is locked before
  if (lfxo_lock_status == _LFXO_STATUS_LOCK_LOCKED) {
    LFXO->LOCK = LFXO_LOCK_LOCKKEY_UNLOCK;
  }

  EFM_ASSERT(val <= (_LFXO_CAL_CAPTUNE_MASK >> _LFXO_CAL_CAPTUNE_SHIFT));
  // Max internal capacitance tuning value is 0x59 (29.1 pF)
  ctune = (uint8_t) SL_MIN(0x59U, (uint8_t)val);

  // Wait for CALBSY bit to clear before writing the tuning value to CAL register
  while (((LFXO->SYNCBUSY & _LFXO_SYNCBUSY_CAL_MASK) >> _LFXO_SYNCBUSY_CAL_SHIFT) != 0U) ;
  LFXO->CAL = (LFXO->CAL & ~_LFXO_CAL_CAPTUNE_MASK)
              | ((uint32_t)ctune << _LFXO_CAL_CAPTUNE_SHIFT);

  // Lock register interface again
  if (lfxo_lock_status == _LFXO_STATUS_LOCK_LOCKED) {
    LFXO->LOCK = ~LFXO_LOCK_LOCKKEY_UNLOCK;
  }

  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Gets the LFXO frequency tuning setting.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_lfxo_calibration(uint32_t *val)
{
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LFXO);
  *val = (LFXO->CAL & _LFXO_CAL_CAPTUNE_MASK) >> _LFXO_CAL_CAPTUNE_SHIFT;

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
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();

  // Keep untouched configuration settings
  uint32_t calCtrl = CMU->CALCTRL
                     & ~(_CMU_CALCTRL_UPSEL_MASK
                         | _CMU_CALCTRL_DOWNSEL_MASK);
  // Check for cycle count overflow
  EFM_ASSERT(cycles <= (_CMU_CALTOP_CALTOP_MASK >> _CMU_CALTOP_CALTOP_SHIFT));
  CMU->CALTOP = cycles << _CMU_CALTOP_CALTOP_SHIFT;

  // Set down counting clock source selector
  switch (down_counter_selection) {
    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_HCLK:
      calCtrl |= CMU_CALCTRL_DOWNSEL_HCLK;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_PRS:
      calCtrl |= CMU_CALCTRL_DOWNSEL_PRS;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_HFXO:
      calCtrl |= CMU_CALCTRL_DOWNSEL_HFXO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_LFXO:
      calCtrl |= CMU_CALCTRL_DOWNSEL_LFXO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_HFRCODPLL:
      calCtrl |= CMU_CALCTRL_DOWNSEL_HFRCODPLL;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_HFRCOEM23:
      calCtrl |= CMU_CALCTRL_DOWNSEL_HFRCOEM23;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_FSRCO:
      calCtrl |= CMU_CALCTRL_DOWNSEL_FSRCO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_LFRCO:
      calCtrl |= CMU_CALCTRL_DOWNSEL_LFRCO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_ULFRCO:
      calCtrl |= CMU_CALCTRL_DOWNSEL_ULFRCO;
      break;

    default:
      CORE_EXIT_ATOMIC();
      return SL_STATUS_NOT_AVAILABLE;
  }

  // Set up counting clock source selector
  switch (up_counter_selection) {
    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_PRS:
      calCtrl |= CMU_CALCTRL_UPSEL_PRS;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_HFXO:
      calCtrl |= CMU_CALCTRL_UPSEL_HFXO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_LFXO:
      calCtrl |= CMU_CALCTRL_UPSEL_LFXO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_HFRCODPLL:
      calCtrl |= CMU_CALCTRL_UPSEL_HFRCODPLL;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_HFRCOEM23:
      calCtrl |= CMU_CALCTRL_UPSEL_HFRCOEM23;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_FSRCO:
      calCtrl |= CMU_CALCTRL_UPSEL_FSRCO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_LFRCO:
      calCtrl |= CMU_CALCTRL_UPSEL_LFRCO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_ULFRCO:
      calCtrl |= CMU_CALCTRL_UPSEL_ULFRCO;
      break;

    case SL_CLOCK_MANAGER_CLOCK_CALIBRATION_HCLK:
      CORE_EXIT_ATOMIC();
      return SL_STATUS_NOT_SUPPORTED;

    default:
      CORE_EXIT_ATOMIC();
      return SL_STATUS_NOT_AVAILABLE;
  }

  CMU->CALCTRL = calCtrl;
  sl_hal_bus_reg_write_bit(&CMU->CALCTRL, _CMU_CALCTRL_CONT_SHIFT, (uint32_t)continuous_calibration);

  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Starts the RCO calibration.
 ******************************************************************************/
void sli_clock_manager_hal_start_rco_calibration(void)
{
  CMU->CALCMD = CMU_CALCMD_CALSTART;
}

/***************************************************************************//**
 * Stops the RCO calibration.
 ******************************************************************************/
void sli_clock_manager_hal_stop_rco_calibration(void)
{
  CMU->CALCMD = CMU_CALCMD_CALSTOP;
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
 * Sets SYSCLK clock source clock.
 *
 * @note On series 3, we actively wait for the oscillator to be ready before
 *       switching the SYSCLK branch. This is to not gate the SYSCLK branch
 *       during switching for other HW modules connected to SYSCLK branch.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_set_sysclk_source(sl_oscillator_t source)
{
  sl_status_t return_status = SL_STATUS_OK;
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();

  switch (source) {
    case SL_OSCILLATOR_FSRCO:
      CMU->SYSCLKCTRL = (CMU->SYSCLKCTRL & ~_CMU_SYSCLKCTRL_CLKSEL_MASK) | CMU_SYSCLKCTRL_CLKSEL_FSRCO;
      break;

    case SL_OSCILLATOR_HFXO:
      if ((HFXO0->STATUS & HFXO_STATUS_RDY) == 0) {
        HFXO0->CTRL_SET = HFXO_CTRL_FORCEEN;
        while ((HFXO0->STATUS & HFXO_STATUS_RDY) == 0) ;
      }
      CMU->SYSCLKCTRL = (CMU->SYSCLKCTRL & ~_CMU_SYSCLKCTRL_CLKSEL_MASK) | CMU_SYSCLKCTRL_CLKSEL_HFXO;
      HFXO0->CTRL_CLR = HFXO_CTRL_FORCEEN;
      break;

    case SL_OSCILLATOR_HFRCODPLL:
      if ((HFRCO0->STATUS & HFRCO_STATUS_RDY) == 0) {
        HFRCO0->CTRL_SET = HFRCO_CTRL_FORCEEN;
        while ((HFRCO0->STATUS & HFRCO_STATUS_RDY) == 0) ;
      }
      CMU->SYSCLKCTRL = (CMU->SYSCLKCTRL & ~_CMU_SYSCLKCTRL_CLKSEL_MASK) | CMU_SYSCLKCTRL_CLKSEL_HFRCODPLL;
      HFRCO0->CTRL_CLR = HFRCO_CTRL_FORCEEN;
      break;

    case SL_OSCILLATOR_CLKIN0:
      CMU->SYSCLKCTRL = (CMU->SYSCLKCTRL & ~_CMU_SYSCLKCTRL_CLKSEL_MASK) | CMU_SYSCLKCTRL_CLKSEL_CLKIN0;
      break;

#if defined(_SOCPLL_SOCCLK0_MASK)
    case SL_OSCILLATOR_SOCPLL0_OUT1:
#else
    case SL_OSCILLATOR_SOCPLL0:
#endif
      if ((SOCPLL0->STATUS & SOCPLL_STATUS_RDY) == 0) {
        SOCPLL0->CTRL_SET = SOCPLL_CTRL_FORCEEN;
        while ((SOCPLL0->STATUS & SOCPLL_STATUS_RDY) == 0) ;
      }
      CMU->SYSCLKCTRL = (CMU->SYSCLKCTRL & ~_CMU_SYSCLKCTRL_CLKSEL_MASK) | CMU_SYSCLKCTRL_CLKSEL_SOCPLL;
      SOCPLL0->CTRL_CLR = SOCPLL_CTRL_FORCEEN;
      break;

#if defined(CMU_SYSCLKCTRL_CLKSEL_SOCPLL1)
    case SL_OSCILLATOR_SOCPLL1_OUT1:
      if ((SOCPLL1->STATUS & SOCPLL_STATUS_RDY) == 0) {
        SOCPLL1->CTRL_SET = SOCPLL_CTRL_FORCEEN;
        while ((SOCPLL1->STATUS & SOCPLL_STATUS_RDY) == 0) ;
      }
      CMU->SYSCLKCTRL = (CMU->SYSCLKCTRL & ~_CMU_SYSCLKCTRL_CLKSEL_MASK) | CMU_SYSCLKCTRL_CLKSEL_SOCPLL1;
      SOCPLL1->CTRL_CLR = SOCPLL_CTRL_FORCEEN;
      break;
#endif

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
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();

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
#if defined(_SOCPLL_SOCCLK0_MASK)
    case  CMU_SYSCLKCTRL_CLKSEL_SOCPLL:
      *source = SL_OSCILLATOR_SOCPLL0_OUT1;
      break;
#else
    case  CMU_SYSCLKCTRL_CLKSEL_SOCPLL:
      *source = SL_OSCILLATOR_SOCPLL0;
      break;
#endif
#if defined(CMU_SYSCLKCTRL_CLKSEL_SOCPLL1)
    case  CMU_SYSCLKCTRL_CLKSEL_SOCPLL1:
      *source = SL_OSCILLATOR_SOCPLL1_OUT1;
      break;
#endif
    default:
      EFM_ASSERT(false);
      break;
  }

  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/*******************************************************************************
 * HFXO interrupt handler.
 ******************************************************************************/
void HFXO_IRQ_HANDLER_FUNCTION(void)
{
  uint32_t irq_flag = HFXO0->IF;

#if defined(SL_CLOCK_MANAGER_HFXO_SLEEPY_CRYSTAL_SUPPORT) && (SL_CLOCK_MANAGER_HFXO_SLEEPY_CRYSTAL_SUPPORT == 1)
  // SLEEPYXTAL Interrupt Flag Handling.
  if (irq_flag & HFXO_IF_SLEEPYXTAL) {
    // Clear error flag
    HFXO0->IF_CLR = irq_flag & HFXO_IF_SLEEPYXTAL;

    // Notification callback function
    sl_clock_manager_hfxo_notify_consecutive_failed_startups();
  }
#endif

  // Ready Interrupt Flag Handling.
  if (irq_flag & HFXO_IF_RDY) {
    // Clear Ready flag.
    HFXO0->IF_CLR = irq_flag & HFXO_IF_RDY;

    // Callback fuction to notify HFXO is ready.
    sli_clock_manager_notify_hfxo_ready();
  }

#if defined(SL_CLOCK_MANAGER_HFXO_STARTUP_TIME_MEASUREMENT_EN) && SL_CLOCK_MANAGER_HFXO_STARTUP_TIME_MEASUREMENT_EN
  // Startup time Measure Done Interrupt Flag Handling.
  if ((irq_flag & HFXO_IF_STUPMEASDONE) && (HFXO0->IEN & HFXO_IEN_STUPMEASDONE)) {
    // Clear Ready flag and disable interrupt.
    HFXO0->IF_CLR = irq_flag & HFXO_IF_STUPMEASDONE;
    HFXO0->IEN_CLR = irq_flag & HFXO_IEN_STUPMEASDONE;

    // Update startup time
    clock_manager_hfxo_startup_time = (HFXO0->AVGSTARTUPTIME & _HFXO_AVGSTARTUPTIME_AVGSTUP_MASK)
                                      >> _HFXO_AVGSTARTUPTIME_AVGSTUP_SHIFT;

    // Disable startup time measure by removing clock since we cannot change the cfg while it is running.
    CMU->HFXO0LFCLKCTRL = (CMU->HFXO0LFCLKCTRL & ~_CMU_HFXO0LFCLKCTRL_CLKSEL_MASK) \
                          | _CMU_HFXO0LFCLKCTRL_CLKSEL_DISABLED;
  }
#endif
}

/***************************************************************************//**
 * Waits for USBPLL clock to be ready.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_wait_usbpll(void)
{
  return SL_STATUS_NOT_AVAILABLE;
}

/***************************************************************************//**
 * Sets the external FLASH reference clock.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_set_ext_flash_clk(sl_oscillator_t oscillator)
{
#if defined (_SILICON_LABS_32B_SERIES_3_CONFIG_301)
  sl_status_t status = SL_STATUS_OK;
  CORE_DECLARE_IRQ_STATE;

  // Check if the QSPI is already clocked by target oscillator.
  if (current_qspi_reference_clock == oscillator) {
    return status;
  }

  if (clock_manager_se_version >= 0x00202L) {
    switch (oscillator) {
      case SL_OSCILLATOR_FLPLL:
        if (clock_manager_flpll_cfg.ref_clock == SLI_SE_QSPI_REF_CLOCK_HFXO) {
          // Send the SE command to switch the QSPI clock to FLPLL with HFXO reference clock.
          CORE_ENTER_ATOMIC();
          status = sli_se_qspi_configure_clock_flpll(&clock_manager_cmd_ctx, &clock_manager_flpll_cfg);
          CORE_EXIT_ATOMIC();
        } else {
          status = SL_STATUS_NOT_AVAILABLE;
        }
        break;

      case SL_OSCILLATOR_FSRCO:
        // Send the SE command to switch QSPI clock to FSRCO
        CORE_ENTER_ATOMIC();
        status = sli_se_qspi_configure_clock_fsrco(&clock_manager_cmd_ctx);
        CORE_EXIT_ATOMIC();
        break;

      default:
        status = SL_STATUS_INVALID_PARAMETER;
        break;
    }

    if (status != SL_STATUS_OK) {
      return status;
    }
    // Update global QSPI reference clock variable.
    current_qspi_reference_clock = oscillator;
  } else {
    (void)oscillator;
  }

  return status;
#else
  (void)oscillator;
  return SL_STATUS_NOT_SUPPORTED;
#endif
}
/***************************************************************************//**
 * Gets the external FLASH clock source.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_ext_flash_clk(sl_oscillator_t *oscillator)
{
  if (oscillator == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

#if defined (_SILICON_LABS_32B_SERIES_3_CONFIG_301)
  if (current_qspi_reference_clock == SL_OSCILLATOR_INVALID) {
    return SL_STATUS_NOT_AVAILABLE;
  }

  *oscillator = current_qspi_reference_clock;
  return SL_STATUS_OK;
#else
  (void)oscillator;
  return SL_STATUS_NOT_SUPPORTED;
#endif
}

/***************************************************************************//**
 * Retrieves the FREQPLAN NWP SOCPLL config.
 ******************************************************************************/
sl_status_t sli_clock_manager_hal_get_nwp_socpll_freqplan_config(const uint16_t **socpll_freqplan_config,
                                                                 uint8_t *target_frequency_index)
{
#if defined(SLI_CLOCK_MANAGER_NWPSOCPLL_FREQPLAN_DATA)
  *socpll_freqplan_config = SLI_CLOCK_MANAGER_NWPSOCPLL_FREQPLAN_DATA->socpll_config;
  *target_frequency_index = SLI_CLOCK_MANAGER_NWPSOCPLL_FREQPLAN_DATA->default_freqsel;
  return SL_STATUS_OK;
#else
  (void)socpll_freqplan_config;
  (void)target_frequency_index;
  return SL_STATUS_NOT_SUPPORTED;
#endif
}
