/***************************************************************************/ /**
 * @file  sl_net_basic_profiles.c
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

#include "sl_net.h"
#include "sl_net_ip_types.h"
#include "sl_net_wifi_types.h"
#include "sl_net_default_values.h"
#include <string.h>

static sl_net_wifi_client_profile_t wifi_client_profiles[MAX_WIFI_CLIENT_PROFILES] = { 0 };
static sl_net_wifi_ap_profile_t wifi_ap_profiles[MAX_WIFI_AP_PROFILES]             = { 0 };

static sl_status_t sli_si91x_validate_sl_net_profile(const sl_net_profile_t *profile, sl_net_interface_t interface)
{
  switch (interface) {
#ifdef SL_WIFI_COMPONENT_INCLUDED
    case SL_NET_WIFI_CLIENT_INTERFACE: {
      if (
        (((const sl_net_wifi_client_profile_t *)profile)->config.ssid.length == 0)
        || (((const sl_net_wifi_client_profile_t *)profile)->config.ssid.length
            > SL_WIFI_MAX_SSID_LENGTH
                - 2)) { //The maximum length of the SSID is 34 characters with 2 characters reserved for NULL termination and internal alignment. Therefore, used `SL_WIFI_MAX_SSID_LENGTH - 2`
        SL_DEBUG_LOG("Invalid SSID length: %d\n", ((const sl_net_wifi_client_profile_t *)profile)->config.ssid.length);
        return SL_STATUS_INVALID_PARAMETER;
      }
      if (((((const sl_net_wifi_client_profile_t *)profile)->config.security == SL_WIFI_OPEN)
           && (((const sl_net_wifi_client_profile_t *)profile)->config.credential_id != SL_WIFI_NO_CREDENTIAL_ID))
          || ((((const sl_net_wifi_client_profile_t *)profile)->config.security != SL_WIFI_OPEN)
              && (((const sl_net_wifi_client_profile_t *)profile)->config.credential_id == SL_WIFI_NO_CREDENTIAL_ID))) {
        SL_DEBUG_LOG("Mismatch between security and credential_id\n");
        return SL_STATUS_INVALID_CONFIGURATION;
      }
      return SL_STATUS_OK;
    }
    case SL_NET_WIFI_AP_INTERFACE: {
      if ((((const sl_net_wifi_ap_profile_t *)profile)->config.ssid.length == 0)
          || (((const sl_net_wifi_ap_profile_t *)profile)->config.ssid.length > SL_WIFI_MAX_SSID_LENGTH - 2)) {
        SL_DEBUG_LOG("Invalid SSID length: %d\n", ((const sl_net_wifi_ap_profile_t *)profile)->config.ssid.length);
        return SL_STATUS_INVALID_PARAMETER;
      }
      if (((((const sl_net_wifi_ap_profile_t *)profile)->config.security == SL_WIFI_OPEN)
           && (((const sl_net_wifi_ap_profile_t *)profile)->config.credential_id != SL_WIFI_NO_CREDENTIAL_ID))
          || ((((const sl_net_wifi_ap_profile_t *)profile)->config.security != SL_WIFI_OPEN)
              && (((const sl_net_wifi_ap_profile_t *)profile)->config.credential_id == SL_WIFI_NO_CREDENTIAL_ID))) {
        SL_DEBUG_LOG("Mismatch between security and credential_id\n");
        return SL_STATUS_INVALID_CONFIGURATION;
      }
      return SL_STATUS_OK;
    }
#endif
    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

sl_status_t sl_net_set_profile(sl_net_interface_t interface,
                               sl_net_profile_id_t profile_id,
                               const sl_net_profile_t *profile)
{
  sl_status_t status;

  if (profile == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  status = sli_si91x_validate_sl_net_profile(profile, interface);
  if (status != SL_STATUS_OK) {
    return status;
  }
  if (profile_id == SL_NET_AUTO_JOIN)
    return SL_STATUS_NOT_SUPPORTED;
  switch (interface) {
#ifdef SL_WIFI_COMPONENT_INCLUDED
    case SL_NET_WIFI_CLIENT_INTERFACE: {
      if (profile_id >= MAX_WIFI_CLIENT_PROFILES) {
        return SL_STATUS_INVALID_INDEX;
      }
      memcpy(&wifi_client_profiles[profile_id],
             (const sl_net_wifi_client_profile_t *)profile,
             sizeof(sl_net_wifi_client_profile_t));
      return SL_STATUS_OK;
    }
    case SL_NET_WIFI_AP_INTERFACE: {
      if (profile_id >= MAX_WIFI_AP_PROFILES) {
        return SL_STATUS_INVALID_INDEX;
      }
      memcpy(&wifi_ap_profiles[profile_id],
             (const sl_net_wifi_ap_profile_t *)profile,
             sizeof(sl_net_wifi_ap_profile_t));
      return SL_STATUS_OK;
    }
#endif
    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

sl_status_t sl_net_get_profile(sl_net_interface_t interface, sl_net_profile_id_t profile_id, sl_net_profile_t *profile)
{

  if (profile_id == SL_NET_AUTO_JOIN)
    return SL_STATUS_NOT_SUPPORTED;
  SL_WIFI_ARGS_CHECK_NULL_POINTER(profile);
  switch (interface) {
#ifdef SL_WIFI_COMPONENT_INCLUDED
    case SL_NET_WIFI_CLIENT_INTERFACE:
      if (profile_id >= MAX_WIFI_CLIENT_PROFILES) {
        return SL_STATUS_INVALID_INDEX;
      }
      memcpy(profile, &wifi_client_profiles[profile_id], sizeof(sl_net_wifi_client_profile_t));
      return SL_STATUS_OK;

    case SL_NET_WIFI_AP_INTERFACE:
      if (profile_id >= MAX_WIFI_AP_PROFILES) {
        return SL_STATUS_INVALID_INDEX;
      }
      memcpy(profile, &wifi_ap_profiles[profile_id], sizeof(sl_net_wifi_ap_profile_t));
      return SL_STATUS_OK;
#endif

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

sl_status_t sl_net_delete_profile(sl_net_interface_t interface, sl_net_profile_id_t profile_id)
{
  if (profile_id == SL_NET_AUTO_JOIN)
    return SL_STATUS_NOT_SUPPORTED;
  switch (interface) {
#ifdef SL_WIFI_COMPONENT_INCLUDED
    case SL_NET_WIFI_CLIENT_INTERFACE:
      if (profile_id >= MAX_WIFI_CLIENT_PROFILES) {
        return SL_STATUS_INVALID_INDEX;
      }
      memset(&wifi_client_profiles[profile_id], 0, sizeof(sl_net_wifi_client_profile_t));
      return SL_STATUS_OK;

    case SL_NET_WIFI_AP_INTERFACE:
      if (profile_id >= MAX_WIFI_AP_PROFILES) {
        return SL_STATUS_INVALID_INDEX;
      }
      memset(&wifi_ap_profiles[profile_id], 0, sizeof(sl_net_wifi_ap_profile_t));
      return SL_STATUS_OK;
#endif

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}
