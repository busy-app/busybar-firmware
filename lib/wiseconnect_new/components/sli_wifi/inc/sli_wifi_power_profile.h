/***************************************************************************/ /**
 * @file sli_wifi_power_profile.h
 * @brief This file contains the declaration of the WiFi power profile functions.
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
#ifndef SLI_WIFI_POWER_PROFILE_H
#define SLI_WIFI_POWER_PROFILE_H
#include "sl_wifi_device.h"
#include "sl_status.h"
#include "sli_wifi_types.h"
sl_status_t sli_wifi_set_performance_profile(const sl_wifi_performance_profile_t *profile);
sl_status_t sli_wifi_set_performance_profile_v2(const sl_wifi_performance_profile_v2_t *profile);
sl_status_t sli_wifi_get_performance_profile(sl_wifi_performance_profile_t *profile);
sl_status_t sli_wifi_get_performance_profile_v2(sl_wifi_performance_profile_v2_t *profile);
/* Function used to retrieve the coex performance profile */
void sli_get_coex_performance_profile(sl_wifi_system_performance_profile_t *profile);
/* Function used to update the coex mode */
void sli_save_coex_mode(sl_wifi_system_coex_mode_t coex_mode);

/* Function used to retrieve the coex mode */
sl_wifi_system_coex_mode_t sli_get_coex_mode(void);
/* Function used to zero out the coex performance profile */
void sli_wifi_reset_coex_current_performance_profile(void);
void sli_wifi_save_current_performance_profile(const sl_wifi_performance_profile_v2_t *profile);
/* Function used to get current wifi performance profile */
void sli_wifi_get_current_performance_profile(sl_wifi_performance_profile_v2_t *profile);
/* Function used to set the bluetooth performance profile */
void sli_save_bt_current_performance_profile(const sl_bt_performance_profile_t *profile);
void sli_convert_performance_profile_to_power_save_command(sl_wifi_system_performance_profile_t profile,
                                                           sli_wifi_power_save_request_t *power_save_request);

#endif