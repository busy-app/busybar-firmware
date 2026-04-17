/***************************************************************************//**
 * @file sl_rail_util_mp_transition_time.c
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_rail_util_mp_transition_time.h"
#include "sl_rail_util_mp_transition_time_config.h"
#include "sl_clock_manager.h"
#include "sl_rail.h"

sl_status_t sl_rail_util_mp_transition_time_get(const sl_rail_util_mp_transition_time_entry_t *p_table,
                                                const size_t table_size,
                                                sl_rail_time_t *p_transition_time)
{
  if (table_size == 0U || p_table == NULL || p_transition_time == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  uint32_t current_system_clock_frequency_hz = 0U;
  sl_status_t status = sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_SYSCLK, &current_system_clock_frequency_hz);
  if (status != SL_STATUS_OK) {
    return status;
  }

  for (size_t i = 0U; i < table_size; i++) {
    if (current_system_clock_frequency_hz >= p_table[i].minimum_frequency_hz) {
      *p_transition_time = p_table[i].transition_time_us;
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

void sl_rail_util_mp_transition_time_init()
{
  sl_rail_time_t transition_time_us = 0U;
  sl_status_t status = sl_rail_util_mp_transition_time_get(sl_rail_util_mp_transition_time_table,
                                                           SL_RAIL_UTIL_MP_TRANSITION_TIME_TABLE_ENTRIES,
                                                           &transition_time_us);
  if (status != SL_STATUS_OK) {
    return;
  }
  sl_rail_set_transition_time(SL_RAIL_EFR32_HANDLE, transition_time_us);
}
