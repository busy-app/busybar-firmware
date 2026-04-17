/***************************************************************************//**
 * @file
 * @brief Power Manager execution modes HAL API implementation.
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
#include "sl_power_manager_config.h"
#if defined(SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN) && (SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN == 1)
#include "sl_power_manager.h"
#include "sli_power_manager_execution_modes_private.h"
#include "sli_clock_manager.h"
#include "sl_assert.h"

/*******************************************************************************
 *******************************  DEFINES   ************************************
 ******************************************************************************/

// The execution modes feature on SIXG301 changes the SYSCLK clock source.
// For SIXG301 devices, those clock sources are prefined to the below values.
#define EXECUTION_MODE_STANDARD_CLOCK_SOURCE     SL_OSCILLATOR_HFRCODPLL
#define EXECUTION_MODE_PERFORMANCE_CLOCK_SOURCE  SL_OSCILLATOR_SOCPLL0

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Initialize execution modes with original mode as standard mode.
 ******************************************************************************/
void sli_power_manager_executions_modes_hal_init(void)
{
  sli_power_manager_hal_apply_standard_mode();
}

/***************************************************************************//**
 * Updates SYSCLK to achieve performance mode execution.
 ******************************************************************************/
void sli_power_manager_hal_apply_performance_mode(void)
{
  if ((HFXO0->STATUS & _HFXO_STATUS_RDY_MASK) == 0) {
    // Force-enable HFXO.
    HFXO0->CTRL_SET = HFXO_CTRL_FORCEEN;
  } else {
    sl_status_t status = slx_clock_manager_set_sysclk_source(EXECUTION_MODE_PERFORMANCE_CLOCK_SOURCE);
    EFM_ASSERT(status == SL_STATUS_OK);
  }
}

/***************************************************************************//**
 * Updates SYSCLK to achieve standard mode execution.
 ******************************************************************************/
void sli_power_manager_hal_apply_standard_mode(void)
{
  sl_status_t status = slx_clock_manager_set_sysclk_source(EXECUTION_MODE_STANDARD_CLOCK_SOURCE);
  EFM_ASSERT(status == SL_STATUS_OK);
}
#endif // SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN
