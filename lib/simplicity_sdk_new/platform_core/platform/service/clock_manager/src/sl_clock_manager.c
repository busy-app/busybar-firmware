/***************************************************************************//**
 * @file
 * @brief Clock Manager API implementations.
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

#include "sl_clock_manager.h"
#include "sli_clock_manager.h"
#include "sli_clock_manager_hal.h"
#include "sl_assert.h"
#include "cmsis_compiler.h"

/***************************************************************************//**
 * Performs Clock Manager runtime initialization.
 ******************************************************************************/
sl_status_t sl_clock_manager_runtime_init(void)
{
  return sli_clock_manager_hal_runtime_init();
}

/***************************************************************************//**
 * Gets frequency of given oscillator.
 ******************************************************************************/
sl_status_t sl_clock_manager_get_oscillator_frequency(sl_oscillator_t oscillator,
                                                      uint32_t        *frequency)
{
  if (frequency == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  return sli_clock_manager_hal_get_oscillator_frequency(oscillator, frequency);
}

/***************************************************************************//**
 * Gets precision of given oscillator.
 ******************************************************************************/
sl_status_t sl_clock_manager_get_oscillator_precision(sl_oscillator_t oscillator,
                                                      uint16_t        *precision)
{
  if (precision == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  return sli_clock_manager_hal_get_oscillator_precision(oscillator, precision);
}

/***************************************************************************//**
 * Gets frequency of given clock branch.
 ******************************************************************************/
sl_status_t sl_clock_manager_get_clock_branch_frequency(sl_clock_branch_t clock_branch,
                                                        uint32_t          *frequency)
{
  if (frequency == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  return sli_clock_manager_hal_get_clock_branch_frequency(clock_branch, frequency);
}

/***************************************************************************//**
 * Gets precision of given clock branch.
 ******************************************************************************/
sl_status_t sl_clock_manager_get_clock_branch_precision(sl_clock_branch_t clock_branch,
                                                        uint16_t          *precision)
{
  if (precision == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  return sli_clock_manager_hal_get_clock_branch_precision(clock_branch, precision);
}

/***************************************************************************//**
 * Enables the given module's bus clock.
 ******************************************************************************/
sl_status_t sl_clock_manager_enable_bus_clock(sl_bus_clock_t module_bus_clock)
{
  return sli_clock_manager_hal_enable_bus_clock(module_bus_clock, true);
}

/***************************************************************************//**
 * Disables the given module's bus clock.
 ******************************************************************************/
sl_status_t sl_clock_manager_disable_bus_clock(sl_bus_clock_t module_bus_clock)
{
  return sli_clock_manager_hal_enable_bus_clock(module_bus_clock, false);
}

/***************************************************************************//**
 * Configures one clock export output with specified clock source.
 ******************************************************************************/
sl_status_t sl_clock_manager_set_gpio_clock_output(sl_clock_manager_export_clock_source_t export_clock_source,
                                                   sl_clock_manager_export_clock_output_select_t output_select,
                                                   uint16_t hfexp_divider,
                                                   uint32_t port,
                                                   uint32_t pin)
{
  return sli_clock_manager_hal_set_gpio_clock_output(export_clock_source, output_select, hfexp_divider, port, pin);
}

/***************************************************************************//**
 * Sets the RC oscillator frequency tuning control.
 ******************************************************************************/
sl_status_t sl_clock_manager_set_rc_oscillator_calibration(sl_oscillator_t oscillator,
                                                           uint32_t        val)
{
  return sli_clock_manager_hal_set_rc_oscillator_calibration(oscillator, val);
}

/***************************************************************************//**
 * Gets the RC oscillator frequency tuning setting.
 ******************************************************************************/
sl_status_t sl_clock_manager_get_rc_oscillator_calibration(sl_oscillator_t oscillator,
                                                           uint32_t        *val)
{
  if (val == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  return sli_clock_manager_hal_get_rc_oscillator_calibration(oscillator, val);
}

/***************************************************************************//**
 * Sets the HFXO calibration value.
 ******************************************************************************/
sl_status_t sl_clock_manager_set_hfxo_calibration(uint32_t val)
{
  return sli_clock_manager_hal_set_hfxo_calibration(val);
}

/***************************************************************************//**
 * Gets the HFXO calibration value.
 ******************************************************************************/
sl_status_t sl_clock_manager_get_hfxo_calibration(uint32_t *val)
{
  if (val == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  return sli_clock_manager_hal_get_hfxo_calibration(val);
}

/***************************************************************************//**
 * Sets the HFXO CTUNE setting.
 ******************************************************************************/
sl_status_t slx_clock_manager_hfxo_set_ctune(uint32_t ctune)
{
  return sli_clock_manager_hal_hfxo_set_ctune(ctune);
}

/***************************************************************************//**
 * Gets the HFXO CTUNE setting.
 ******************************************************************************/
sl_status_t slx_clock_manager_hfxo_get_ctune(uint32_t *ctune)
{
  if (ctune == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  return sli_clock_manager_hal_hfxo_get_ctune(ctune);
}

/***************************************************************************//**
 * Updates the tuning capacitances and calibrate the Core Bias Current.
 ******************************************************************************/
sl_status_t slx_clock_manager_hfxo_calibrate_ctune(uint32_t ctune)
{
  return sli_clock_manager_hal_hfxo_calibrate_ctune(ctune);
}

/***************************************************************************//**
 * Gets the HFXO's average startup time.
 ******************************************************************************/
sl_status_t sli_clock_manager_get_hfxo_average_startup_time(uint32_t *val)
{
  if (val == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  return sli_clock_manager_hal_get_hfxo_average_startup_time(val);
}

/***************************************************************************//**
 * Sets the LFXO frequency tuning control.
 ******************************************************************************/
sl_status_t sl_clock_manager_set_lfxo_calibration(uint32_t val)
{
  return sli_clock_manager_hal_set_lfxo_calibration(val);
}

/***************************************************************************//**
 * Gets the LFXO frequency tuning setting.
 ******************************************************************************/
sl_status_t sl_clock_manager_get_lfxo_calibration(uint32_t *val)
{
  if (val == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  return sli_clock_manager_hal_get_lfxo_calibration(val);
}

/***************************************************************************//**
 * Configures the RCO calibration.
 ******************************************************************************/
sl_status_t sl_clock_manager_configure_rco_calibration(uint32_t cycles,
                                                       sl_clock_manager_clock_calibration_t down_counter_selection,
                                                       sl_clock_manager_clock_calibration_t up_counter_selection,
                                                       bool continuous_calibration)
{
  return sli_clock_manager_hal_configure_rco_calibration(cycles, down_counter_selection, up_counter_selection, continuous_calibration);
}

/***************************************************************************//**
 * Starts the RCO calibration.
 ******************************************************************************/
void sl_clock_manager_start_rco_calibration(void)
{
  sli_clock_manager_hal_start_rco_calibration();
}

/***************************************************************************//**
 * Stops the RCO calibration.
 ******************************************************************************/
void sl_clock_manager_stop_rco_calibration(void)
{
  sli_clock_manager_hal_stop_rco_calibration();
}

/***************************************************************************//**
 * Waits for the RCO calibration to finish.
 ******************************************************************************/
void sl_clock_manager_wait_rco_calibration(void)
{
  sli_clock_manager_hal_wait_rco_calibration();
}

/***************************************************************************//**
 * Gets calibration count value.
 ******************************************************************************/
sl_status_t sl_clock_manager_get_rco_calibration_count(uint32_t *count)
{
  if (count == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  return sli_clock_manager_hal_get_rco_calibration_count(count);
}

/***************************************************************************//**
 * Sets SYSCLK clock source.
 ******************************************************************************/
sl_status_t slx_clock_manager_set_sysclk_source(sl_oscillator_t oscillator)
{
  return sli_clock_manager_hal_set_sysclk_source(oscillator);
}

/***************************************************************************//**
 * Gets SYSCLK clock source.
 ******************************************************************************/
sl_status_t sl_clock_manager_get_sysclk_source(sl_oscillator_t *oscillator)
{
  if (oscillator == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  return sli_clock_manager_hal_get_sysclk_source(oscillator);
}

/***************************************************************************//**
 * Waits for USBPLL clock to be ready.
 ******************************************************************************/
sl_status_t sl_clock_manager_wait_usbpll(void)
{
  return sli_clock_manager_hal_wait_usbpll();
}

/***************************************************************************//**
 * When this callback function is called, it means that HFXO failed twice in
 * a row to start with normal configurations. This may mean that there is a
 * bad crystal. When getting this callback, HFXO is running but its properties
 * (frequency, precision) are not guaranteed. This should be considered as an
 * error situation.
 ******************************************************************************/
__WEAK void sl_clock_manager_hfxo_notify_consecutive_failed_startups(void)
{
  EFM_ASSERT(false);
}

/***************************************************************************//**
 * Sets the external FLASH reference clock.
 ******************************************************************************/
sl_status_t sl_clock_manager_set_ext_flash_clk(sl_oscillator_t oscillator)
{
  return sli_clock_manager_hal_set_ext_flash_clk(oscillator);
}

/***************************************************************************//**
 * Gets the external FLASH clock source.
 ******************************************************************************/
sl_status_t sl_clock_manager_get_ext_flash_clk(sl_oscillator_t *oscillator)
{
  return sli_clock_manager_hal_get_ext_flash_clk(oscillator);
}

/***************************************************************************//**
 * Retrieves the FREQPLAN NWP SOCPLL config.
 ******************************************************************************/
sl_status_t sli_clock_manager_get_nwp_socpll_freqplan_config(const uint16_t **socpll_freqplan_config,
                                                             uint8_t *target_frequency_index)
{
  if (socpll_freqplan_config == NULL || target_frequency_index == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  return sli_clock_manager_hal_get_nwp_socpll_freqplan_config(socpll_freqplan_config, target_frequency_index);
}
