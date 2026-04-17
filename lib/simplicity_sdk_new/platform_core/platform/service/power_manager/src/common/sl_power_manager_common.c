/***************************************************************************//**
 * @file
 * @brief Power Manager common API implementation.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_power_manager.h"
#include "sl_power_manager_config.h"
#include "sli_power_manager.h"
#include "sli_power_manager_private.h"
#include "sli_clock_manager.h"
#include "sl_assert.h"
#include "sl_atomic.h"
#include "sl_clock_manager.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#include "em_device.h"
#if !defined(_SILICON_LABS_32B_SERIES_3)
#include "em_emu.h"
#else
#include "sli_power_manager_execution_modes_private.h"
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

// Events subscribers lists
static sl_slist_node_t *power_manager_em_transition_event_list = NULL;

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Last-chance check before sleep.
 *
 * @return  True, if the system should actually sleep.
 *          False, if not.
 *
 * @note This is the fallback implementation of the callback, it can be
 *       overridden by the application or other components.
 ******************************************************************************/
__WEAK bool sl_power_manager_is_ok_to_sleep(void)
{
  return true;
}

/***************************************************************************//**
 * Check if the MCU can sleep after an interrupt.
 *
 * @return  True, if the system can sleep after the interrupt.
 *          False, otherwise.
 *
 * @note This is the fallback implementation of the callback, it can be
 *       overridden by the application or other components.
 ******************************************************************************/
__WEAK bool sl_power_manager_sleep_on_isr_exit(void)
{
  return false;
}

/***************************************************************************//**
 * Enable or disable fast wake-up in EM2 and EM3
 *
 * @note Will also update the wake up time from EM2 to EM0.
 *
 * @note This function will do nothing when a project contains the
 *       power_manager_no_deepsleep component, which configures the
 *       lowest energy mode as EM1.
 ******************************************************************************/
void sl_power_manager_em23_voltage_scaling_enable_fast_wakeup(bool enable)
{
#if (defined(SLI_POWER_MANAGER_VSCALE_EM23_PRESENT) && !defined(SL_CATALOG_POWER_MANAGER_NO_DEEPSLEEP_PRESENT))
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();

  sli_power_manager_em23_voltage_scaling_enable_fast_wakeup(enable);

  CORE_EXIT_CRITICAL();
#else
  (void)enable;
#endif
}

/***************************************************************************//**
 * Registers a callback to be called on given Energy Mode transition(s).
 *
 * @note Adding/Removing requirement(s) from the callback is not supported.
 ******************************************************************************/
void sl_power_manager_subscribe_em_transition_event(sl_power_manager_em_transition_event_handle_t *event_handle,
                                                    const sl_power_manager_em_transition_event_info_t *event_info)
{
  CORE_DECLARE_IRQ_STATE;

  event_handle->info = event_info;
  CORE_ENTER_CRITICAL();
  sl_slist_push(&power_manager_em_transition_event_list, &event_handle->node);
  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * Unregisters an event callback handle on Energy mode transition.
 ******************************************************************************/
void sl_power_manager_unsubscribe_em_transition_event(sl_power_manager_em_transition_event_handle_t *event_handle)
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  sl_slist_remove(&power_manager_em_transition_event_list, &event_handle->node);
  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * Initializes energy mode transition list.
 ******************************************************************************/
void sli_power_manager_em_transition_event_list_init(void)
{
  sl_slist_init(&power_manager_em_transition_event_list);
}

/***************************************************************************//**
 * Notify subscribers about energy mode transition.
 ******************************************************************************/
void sli_power_manager_notify_em_transition(sl_power_manager_em_t from,
                                            sl_power_manager_em_t to)
{
  sl_power_manager_em_transition_event_handle_t *handle;
  sl_power_manager_em_transition_event_t transition = 0;

  switch (to) {
    case SL_POWER_MANAGER_EM0:
      transition = SL_POWER_MANAGER_EVENT_TRANSITION_ENTERING_EM0;
      break;

    case SL_POWER_MANAGER_EM1:
      transition = SL_POWER_MANAGER_EVENT_TRANSITION_ENTERING_EM1;
      break;

    case SL_POWER_MANAGER_EM2:
      transition = SL_POWER_MANAGER_EVENT_TRANSITION_ENTERING_EM2;
      break;

    default:
      EFM_ASSERT(0);
  }

  switch (from) {
    case SL_POWER_MANAGER_EM0:
      transition |= SL_POWER_MANAGER_EVENT_TRANSITION_LEAVING_EM0;
      break;

    case SL_POWER_MANAGER_EM1:
      transition |= SL_POWER_MANAGER_EVENT_TRANSITION_LEAVING_EM1;
      break;

    case SL_POWER_MANAGER_EM2:
      transition |= SL_POWER_MANAGER_EVENT_TRANSITION_LEAVING_EM2;
      break;

    default:
      EFM_ASSERT(0);
  }

  SL_SLIST_FOR_EACH_ENTRY(power_manager_em_transition_event_list, handle, sl_power_manager_em_transition_event_handle_t, node) {
    if ((handle->info->event_mask & transition) > 0) {
      handle->info->on_event(from, to);
    }
  }
}
