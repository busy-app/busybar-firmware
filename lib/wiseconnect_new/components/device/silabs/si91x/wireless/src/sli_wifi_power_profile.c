/***************************************************************************/ /**
 * @file sli_wifi_power_profile.c
 * @brief This file contains the implementation of the WiFi power profile functions.
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
#include "sli_wifi_power_profile.h"
#include "sli_wifi_types.h"
#include <string.h>
#include "sli_wifi_utility.h"
#define SLI_MAX_SIZE_OF_UINT16_T          65535
#define SLI_ULP_WITH_RAM_RETENTION        1
#define SLI_MAX_PSP                       0
#define SLI_FAST_PSP                      1
#define SLI_ULP_WITHOUT_RAM_RET_RETENTION 2
#define DEFAULT_BEACON_MISS_IGNORE_LIMIT  1
#define SLI_DEFAULT_MONITOR_INTERVAL      50

extern bool device_initialized;
extern sli_wifi_performance_profile_t performance_profile;
static sl_wifi_system_coex_mode_t coex_mode = 0;

void sli_save_coex_mode(sl_wifi_system_coex_mode_t mode)
{
  coex_mode = mode;
}
sl_wifi_system_coex_mode_t sli_get_coex_mode(void)
{
  return coex_mode;
}

void sli_wifi_reset_coex_current_performance_profile(void)
{
  memset(&performance_profile, 0, sizeof(sli_wifi_performance_profile_t));
}

void sli_wifi_save_current_performance_profile(const sl_wifi_performance_profile_v2_t *profile)
{
  SL_ASSERT(profile != NULL);
  memcpy(&performance_profile.wifi_performance_profile, profile, sizeof(sl_wifi_performance_profile_v2_t));

  performance_profile.coex_mode = sli_get_coex_mode();
}

// Get the coexistence performance profile based on the current coexistence mode
void sli_get_coex_performance_profile(sl_wifi_system_performance_profile_t *profile)
{
  SL_ASSERT(profile != NULL);
  uint8_t mode_decision                       = 0;
  sl_wifi_system_coex_mode_t stored_coex_mode = performance_profile.coex_mode;
  if (stored_coex_mode == SL_SI91X_WLAN_ONLY_MODE) { // Treat SL_SI91X_WLAN_ONLY_MODE as SL_SI91X_WLAN_MODE
    stored_coex_mode = SL_SI91X_WLAN_MODE;
  }
  // Determine the mode decision based on the coexistence mode
  switch (stored_coex_mode) {
    case SL_SI91X_WLAN_MODE: {
      // Wi-Fi only mode
      mode_decision = (uint8_t)((performance_profile.wifi_performance_profile.profile << 4)
                                | (performance_profile.wifi_performance_profile.profile));
    } break;
    case SL_SI91X_BLUETOOTH_MODE:
    case SL_SI91X_BLE_MODE:
    case SL_SI91X_DUAL_MODE: {
      // Bluetooth only or dual-mode (BT + Wi-Fi) mode
      mode_decision = (uint8_t)((performance_profile.bt_performance_profile.profile << 4)
                                | (performance_profile.bt_performance_profile.profile));
    } break;
    case SL_SI91X_WLAN_BLUETOOTH_MODE:
    case SL_SI91X_WLAN_DUAL_MODE:
    case SL_SI91X_WLAN_BLE_MODE: {
      // Wi-Fi + Bluetooth mode
      mode_decision = (uint8_t)((performance_profile.wifi_performance_profile.profile << 4)
                                | (performance_profile.bt_performance_profile.profile));
    } break;
    default:
      break;
  }

  // Determine the performance profile based on the mode decision
  switch (mode_decision) {
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x10:
    case 0x20:
    case 0x30:
    case 0x40: {
      *profile = HIGH_PERFORMANCE; // High performance mode
    } break;
    case 0x11:
    case 0x12:
    case 0x31:
    case 0x13:
    case 0x14:
    case 0x41: {
      *profile = ASSOCIATED_POWER_SAVE; // Power save mode
    } break;
    case 0x22:
    case 0x21:
    case 0x32:
    case 0x23:
    case 0x42:
    case 0x24: {
      *profile = ASSOCIATED_POWER_SAVE_LOW_LATENCY; // Low latency power save mode
    } break;
    case 0x33: {
      *profile = DEEP_SLEEP_WITHOUT_RAM_RETENTION; // Power save mode
    } break;
    case 0x44: {
      *profile = DEEP_SLEEP_WITH_RAM_RETENTION; // Power save mode with RAM retention
    } break;
    default: {
      // Do nothing
    } break;
  }
  return;
}
// Get the current Wi-Fi performance profile
void sli_wifi_get_current_performance_profile(sl_wifi_performance_profile_v2_t *profile)
{
  SL_ASSERT(profile != NULL);
  memcpy(profile, &performance_profile.wifi_performance_profile, sizeof(sl_wifi_performance_profile_v2_t));
}

sl_status_t sli_wifi_set_performance_profile(const sl_wifi_performance_profile_t *profile)
{
  sl_status_t status;
  sl_wifi_system_performance_profile_t selected_coex_profile_mode = { 0 };
  sl_wifi_performance_profile_v2_t current_wifi_profile_mode      = { 0 };
  sl_wifi_performance_profile_v2_t profile_v2                     = { 0 };

  profile_v2.profile                  = profile->profile;
  profile_v2.dtim_aligned_type        = profile->dtim_aligned_type;
  profile_v2.num_of_dtim_skip         = profile->num_of_dtim_skip;
  profile_v2.listen_interval          = (uint32_t)profile->listen_interval;
  profile_v2.monitor_interval         = profile->monitor_interval;
  profile_v2.twt_request              = profile->twt_request;
  profile_v2.twt_selection            = profile->twt_selection;
  profile_v2.beacon_miss_ignore_limit = 1;

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  SL_WIFI_ARGS_CHECK_NULL_POINTER(&profile_v2);

  if (profile_v2.profile > DEEP_SLEEP_WITH_RAM_RETENTION) {
    return SL_STATUS_INVALID_MODE;
  }

  // Take backup of current wifi profile
  sli_wifi_get_current_performance_profile(&current_wifi_profile_mode);

  // Send the power save command for the requested profile
  status = sli_wifi_send_power_save_request(&profile_v2, NULL);
  if (status != SL_STATUS_OK) {
    sli_wifi_save_current_performance_profile(&current_wifi_profile_mode);
    return status;
  }
  sli_get_coex_performance_profile(&selected_coex_profile_mode);

  // Deinitialize the driver since the module's RAM will not be retained
  // in ULTRA_POWER_SAVE and module needs to be started from init again
  if (selected_coex_profile_mode == DEEP_SLEEP_WITHOUT_RAM_RETENTION) {
    device_initialized = false;

#ifdef SLI_SI91X_MCU_INTERFACE
    // In soc mode m4 does not get the card ready for next init after deinit, but if device in DEEP_SLEEP_WITHOUT_RAM_RETENTION mode, m4 should wait for card ready for next init
    sli_wifi_set_card_ready_required(true);
#endif
    sli_wifi_reset_coex_current_performance_profile();
  }

  return SL_STATUS_OK;
}

sl_status_t sli_wifi_set_performance_profile_v2(const sl_wifi_performance_profile_v2_t *profile)
{
  sl_status_t status;
  sl_wifi_system_performance_profile_t selected_coex_profile_mode = { 0 };
  sl_wifi_performance_profile_v2_t current_wifi_profile_mode      = { 0 };

  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  SL_WIFI_ARGS_CHECK_NULL_POINTER(profile);

  if (profile->profile > DEEP_SLEEP_WITH_RAM_RETENTION) {
    return SL_STATUS_INVALID_MODE;
  }

  // Check if the listen interval in the profile exceeds the maximum size supported by NWP
  if (profile->listen_interval > SLI_MAX_SIZE_OF_UINT16_T) {
    return SL_STATUS_INVALID_RANGE;
  }

  // Take backup of current wifi profile
  sli_wifi_get_current_performance_profile(&current_wifi_profile_mode);

  // Send the power save command for the requested profile
  status = sli_wifi_send_power_save_request(profile, NULL);
  if (status != SL_STATUS_OK) {
    sli_wifi_save_current_performance_profile(&current_wifi_profile_mode);
    return status;
  }
  sli_get_coex_performance_profile(&selected_coex_profile_mode);

  if (selected_coex_profile_mode == DEEP_SLEEP_WITHOUT_RAM_RETENTION) {

#ifdef SLI_SI91X_MCU_INTERFACE
    // In soc mode m4 does not get the card ready for next init after deinit, but if device in DEEP_SLEEP_WITHOUT_RAM_RETENTION mode, m4 should wait for card ready for next init
    sli_wifi_set_card_ready_required(true);
#endif
    sli_wifi_reset_coex_current_performance_profile();
  }

  return SL_STATUS_OK;
}

sl_status_t sli_wifi_get_performance_profile(sl_wifi_performance_profile_t *profile)
{
  UNUSED_PARAMETER(profile);
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sl_wifi_performance_profile_v2_t profile_v2 = { 0 };
  sli_wifi_get_current_performance_profile(&profile_v2);
  memcpy(profile, &profile_v2, sizeof(sl_wifi_performance_profile_t));

  return SL_STATUS_OK;
}

sl_status_t sli_wifi_get_performance_profile_v2(sl_wifi_performance_profile_v2_t *profile)
{
  if (!device_initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sli_wifi_get_current_performance_profile(profile);
  return SL_STATUS_OK;
}

void sli_save_bt_current_performance_profile(const sl_bt_performance_profile_t *profile)
{
  SL_ASSERT(profile != NULL);
  memcpy(&performance_profile.bt_performance_profile, profile, sizeof(sl_bt_performance_profile_t));
}

void sli_convert_performance_profile_to_power_save_command(sl_wifi_system_performance_profile_t profile,
                                                           sli_wifi_power_save_request_t *power_save_request)
{
  SL_ASSERT(power_save_request != NULL);
  if (performance_profile.wifi_performance_profile.monitor_interval) {
    power_save_request->monitor_interval = performance_profile.wifi_performance_profile.monitor_interval;
  } else {
    power_save_request->monitor_interval = SLI_DEFAULT_MONITOR_INTERVAL;
  }

  power_save_request->ulp_mode_enable   = SLI_ULP_WITH_RAM_RETENTION;
  power_save_request->dtim_aligned_type = performance_profile.wifi_performance_profile.dtim_aligned_type;
  power_save_request->num_of_dtim_skip  = performance_profile.wifi_performance_profile.num_of_dtim_skip;
  power_save_request->listen_interval   = (uint16_t)performance_profile.wifi_performance_profile.listen_interval;
  power_save_request->psp_type          = SLI_MAX_PSP;
  if (performance_profile.wifi_performance_profile.beacon_miss_ignore_limit) {
    power_save_request->beacon_miss_ignore_limit =
      performance_profile.wifi_performance_profile.beacon_miss_ignore_limit;
  } else {
    power_save_request->beacon_miss_ignore_limit = DEFAULT_BEACON_MISS_IGNORE_LIMIT;
  }

  // Depending on the specified performance profile, configure the power_save_request structure
  switch (profile) {
    case HIGH_PERFORMANCE: {
      // For HIGH_PERFORMANCE profile, reset all fields in the power_save_request structure to zero
      memset(power_save_request, 0, sizeof(sli_wifi_power_save_request_t));
      break;
    }

    case ASSOCIATED_POWER_SAVE: {
#ifdef SLI_SI91X_MCU_INTERFACE
      power_save_request->power_mode = SLI_CONNECTED_M4_BASED_PS;
#else
      power_save_request->power_mode = SLI_CONNECTED_GPIO_BASED_PS;
#endif
      break;
    }

    case ASSOCIATED_POWER_SAVE_LOW_LATENCY: {
#ifdef SLI_SI91X_MCU_INTERFACE
      power_save_request->power_mode = SLI_CONNECTED_M4_BASED_PS;
#else
      power_save_request->power_mode = SLI_CONNECTED_GPIO_BASED_PS;
#endif
      power_save_request->psp_type = SLI_FAST_PSP;
      break;
    }

    case DEEP_SLEEP_WITHOUT_RAM_RETENTION: {
#ifdef SLI_SI91X_MCU_INTERFACE
      power_save_request->power_mode = SLI_M4_BASED_DEEP_SLEEP;
#else
      power_save_request->power_mode = SLI_GPIO_BASED_DEEP_SLEEP;
#endif
      power_save_request->ulp_mode_enable = SLI_ULP_WITHOUT_RAM_RET_RETENTION;
      break;
    }

    case DEEP_SLEEP_WITH_RAM_RETENTION: {
#ifdef SLI_SI91X_MCU_INTERFACE
      power_save_request->power_mode = SLI_M4_BASED_DEEP_SLEEP;
#else
      power_save_request->power_mode = SLI_GPIO_BASED_DEEP_SLEEP;
#endif
      break;
    }
    default: {
      // Do nothing
    } break;
  }

  return;
}
