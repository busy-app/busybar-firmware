/***************************************************************************/ /**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_si91x_ble.h"
#include "sl_rsi_utility.h"
#include "sli_wifi_power_profile.h"
extern bool device_initialized;

/*=======================================================================*/

/**
 * @brief Sets the performance profile for the Si91x Bluetooth module.
 *
 * This function sets the performance profile for the Si91x Bluetooth module based on the provided profile.
 * The function takes a backup of the current Bluetooth profile and computes the selected coexistence (coex) profile.
 * If the selected coex profile is the same as the current coex profile, the function returns SL_STATUS_OK.
 * Otherwise, the function sends a power save request with the selected coex profile.
 * If the power save request fails, the function restores the previous Bluetooth profile, and returns the corresponding status.
 * If the selected coex profile is DEEP_SLEEP_WITHOUT_RAM_RETENTION, the device_initialized flag is set to false, and the coex current performance profile is reset.
 *
 * @param profile Pointer to the performance profile to be set.
 * @return SL_STATUS_OK if the performance profile is set successfully, or an appropriate error code if an error occurs.
 */
sl_status_t sl_si91x_bt_set_performance_profile(const sl_bt_performance_profile_t *profile)
{
  sl_status_t status;
  sl_wifi_system_performance_profile_t selected_coex_profile_mode = { 0 };
  sl_bt_performance_profile_t current_bt_profile_mode             = { 0 };

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  SL_WIFI_ARGS_CHECK_NULL_POINTER(profile);

  // Take backup of current bt profile
  sli_get_bt_current_performance_profile(&current_bt_profile_mode);

  // Send the power save command for the requested profile
  status = sli_si91x_send_power_save_request(NULL, profile);
  if (status != SL_STATUS_OK) {
    sli_save_bt_current_performance_profile(&current_bt_profile_mode);
    return status;
  }
  sli_get_coex_performance_profile(&selected_coex_profile_mode);

  // Set device_initialized as false since RAM of module would not be retained
  // in ULTRA_POWER_SAVE and module needs to be started from init again.
  if (selected_coex_profile_mode == DEEP_SLEEP_WITHOUT_RAM_RETENTION) {
    device_initialized = false;
    sli_reset_coex_current_performance_profile();
  }
  return SL_STATUS_OK;
}

sl_status_t sl_si91x_bt_get_performance_profile(sl_bt_performance_profile_t *profile)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sli_get_bt_current_performance_profile(profile);
  return SL_STATUS_OK;
}
