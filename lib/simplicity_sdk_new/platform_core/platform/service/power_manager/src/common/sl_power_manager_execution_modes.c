/***************************************************************************//**
 * @file
 * @brief Power Manager execution modes API implementation.
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
#include "sl_power_manager.h"
#include "sli_power_manager.h"
#include "sl_power_manager_execution_modes.h"
#include "sli_power_manager_execution_modes_private.h"
#include "sli_clock_manager.h"
#include "sl_assert.h"
#include "sl_core.h"

#include <stdlib.h>
#include <stdint.h>

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

#if defined(SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN) && (SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN == 1)
// Counter that indicates the presence (not zero) or absence (zero) of
// requirements on the performance execution mode.
static uint8_t performance_mode_requirement = 0;

static sl_oscillator_t current_sysclk_source_clock;
#endif

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

extern __INLINE void sl_power_manager_add_performance_mode_requirement(void);
extern __INLINE void sl_power_manager_remove_performance_mode_requirement(void);

/***************************************************************************//**
 * Updates requirement on the given energy mode.
 *
 * @param   add   Flag indicating if requirement is added (true) or removed
 *                (false).
 ******************************************************************************/
void sli_power_manager_update_execution_mode_requirement(bool add)
{
#if defined(SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN) && (SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN == 1)
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  // Cannot increment above 255 (wraparound not allowed)
  EFM_ASSERT(!((performance_mode_requirement == UINT8_MAX) && (add == true)));
  // Cannot decrement below 0 (wraparound not allowed)
  EFM_ASSERT(!((performance_mode_requirement == 0) && (add == false)));

  // Increment (add) or decrement (remove) energy mode counter.
  performance_mode_requirement += (add) ? 1 : -1;

  // Update SYSCLK based on requirements if current energy mode is EM0
  if (sli_power_manager_get_current_em() == SL_POWER_MANAGER_EM0) {
    if (performance_mode_requirement == 1) {
      sli_power_manager_hal_apply_performance_mode();
    } else if (performance_mode_requirement == 0) {
      sli_power_manager_hal_apply_standard_mode();
    }
  }
  CORE_EXIT_CRITICAL();
#else
  (void)add;
#endif
}

#if defined(SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN) && (SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN == 1)
/***************************************************************************//**
 * Initializes execution mode feature.
 ******************************************************************************/
void sli_power_manager_executions_modes_init(void)
{
  sli_power_manager_executions_modes_hal_init();
}
#endif

#if defined(SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN) && (SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN == 1)
/***************************************************************************//**
 * Implement execution mode if not already implemented during a wakeup event.
 ******************************************************************************/
void sli_power_manager_implement_execution_mode_on_wakeup(void)
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  sl_clock_manager_get_sysclk_source(&current_sysclk_source_clock);
  if ((performance_mode_requirement >= 1) && (current_sysclk_source_clock != SL_OSCILLATOR_SOCPLL0)) {
    // If there are performance mode requiremets but SYSCLK has not been updated to performance mode, peform update after entering EM0.
    sli_power_manager_hal_apply_performance_mode();
  } else if (performance_mode_requirement == 0) {
    // If there are no peformance mode requirements, but SYSCLK has not been switched to standard mode, peform update after entering EM0.
    sli_power_manager_hal_apply_standard_mode();
  }
  CORE_EXIT_CRITICAL();
}
#endif

#if defined(SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN) && (SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN == 1)
/***************************************************************************//**
 * When this callback function is called, it means that HFXO is ready and
 * notifies the power manager that the execution mode can be updated to
 * performance mode.
 ******************************************************************************/
void sli_clock_manager_notify_hfxo_ready(void)
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  if ((performance_mode_requirement >= 1) && (current_sysclk_source_clock != SL_OSCILLATOR_SOCPLL0)) {
    sl_status_t status = slx_clock_manager_set_sysclk_source(SL_OSCILLATOR_SOCPLL0);
    EFM_ASSERT(status == SL_STATUS_OK);
  }
  // Remove HFXO FORCEEN.
  HFXO0->CTRL_CLR = HFXO_CTRL_FORCEEN;
  CORE_EXIT_CRITICAL();
}
#endif
