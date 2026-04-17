/*******************************************************************************
 * @file  sl_net_credentials.c
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
#include "sl_net_default_values.h"
#include "sl_wifi_credentials.h"
#include "sl_common.h"
#if defined(SLI_SI917)
#include "sl_net_si91x.h"
#endif
#include <string.h>

#define CRED_TYPE_CERT 0
#define CRED_TYPE_CRED 1

// [SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID]
// [SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID]

static int sli_si91x_check_cred_type(sl_net_credential_type_t type)
{
  if ((SL_NET_CERTIFICATE == type) || (SL_NET_PUBLIC_KEY == type) || (SL_NET_PRIVATE_KEY == type)
      || (SL_NET_SIGNING_CERTIFICATE == type) || (SL_NET_PACK_FILE == type) || (SL_NET_TLS_PRIVATE_KEY_CBC_WRAP == type)
      || (SL_NET_TLS_PRIVATE_KEY_ECB_WRAP == type)) {
    return CRED_TYPE_CERT;
  }

  return CRED_TYPE_CRED;
}

static sl_status_t sli_si91x_get_wifi_credential_type(sl_net_credential_type_t type,
                                                      sl_wifi_credential_type_t *wifi_type)
{
  // Map the network credential type to WiFi credential type
  switch (type) {
    case SL_NET_WIFI_PSK:
      // Set the credential type to Pre-Shared Key (PSK)
      *wifi_type = SL_WIFI_PSK_CREDENTIAL;
      break;
    case SL_NET_WIFI_PMK:
      // Set the credential type to Pairwise Master Key (PMK)
      *wifi_type = SL_WIFI_PMK_CREDENTIAL;
      break;
    case SL_NET_WIFI_WEP:
      // Set the credential type to Wired Equivalent Privacy (WEP)
      *wifi_type = SL_WIFI_WEP_CREDENTIAL;
      break;
    case SL_NET_EAP_CLIENT_CREDENTIAL:
      // Set the credential type to Extensible Authentication Protocol (EAP)
      *wifi_type = SL_WIFI_EAP_CREDENTIAL;
      break;
    default:
      return SL_STATUS_INVALID_PARAMETER;
  }

  return SL_STATUS_OK;
}

static sl_status_t sli_si91x_get_net_credential_type(sl_wifi_credential_type_t type, sl_net_credential_type_t *net_type)
{
  // Map the WiFi credential type to network credential type
  switch (type) {
    case SL_WIFI_PSK_CREDENTIAL:
      // Set the credential type to Pre-Shared Key (PSK)
      *net_type = SL_NET_WIFI_PSK;
      break;
    case SL_WIFI_PMK_CREDENTIAL:
      // Set the credential type to Pairwise Master Key (PMK)
      *net_type = SL_NET_WIFI_PMK;
      break;
    case SL_WIFI_WEP_CREDENTIAL:
      // Set the credential type to Wired Equivalent Privacy (WEP)
      *net_type = SL_NET_WIFI_WEP;
      break;
    case SL_WIFI_EAP_CREDENTIAL:
      // Set the credential type to Extensible Authentication Protocol (EAP)
      *net_type = SL_NET_EAP_CLIENT_CREDENTIAL;
      break;
    default:
      return SL_STATUS_INVALID_PARAMETER;
  }

  return SL_STATUS_OK;
}

sl_status_t sl_net_set_credential(sl_net_credential_id_t id,
                                  sl_net_credential_type_t type,
                                  const void *credential,
                                  uint32_t credential_length)
{
  // Check if the credential is invalid parameter
  if ((credential == NULL) || (credential_length == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  int group_id                        = 0;
  int cred_id                         = 0;
  sl_status_t status                  = 0;
  sl_wifi_credential_type_t cred_type = 0;

  if (CRED_TYPE_CERT == sli_si91x_check_cred_type(type)) {
#if defined(SLI_SI917)
    return sli_net_set_credential(id, type, credential, credential_length);
#else
    return SL_STATUS_FAIL;
#endif
  }

  group_id = (id & SL_NET_CREDENTIAL_GROUP_MASK);

  if (group_id > 0) {
    cred_id = (SL_NET_USER_CREDENTIAL_ID + (group_id >> 8));
  } else {
    group_id = id;
    cred_id  = id;
  }

  switch (type) {
    case SL_NET_WIFI_PSK: {
      // PSK must be between 8 and 63 characters long
      if ((credential_length < SL_WIFI_MIN_PSK_LENGTH) || (credential_length > (SL_WIFI_MAX_PSK_LENGTH - 1))) {
        SL_DEBUG_LOG("\n PSK: Invalid credential length: %ld\n", credential_length);
        return SL_STATUS_SI91X_INVALID_PSK_LENGTH;
      }
      break;
    }
    case SL_NET_WIFI_PMK: {
      // PMK must be exactly 32 bytes in hex format
      if (credential_length != SL_WIFI_MAX_PMK_LENGTH) {
        SL_DEBUG_LOG("\n PMK: Invalid credential length: %ld\n", credential_length);
        return SL_STATUS_SI91X_INVALID_PMK_LEN;
      }
      break;
    }
    default:
      break;
  }

  switch (group_id) {
    case SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID:
    case SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID:
    case SL_NET_WIFI_EAP_CLIENT_CREDENTIAL_ID:
    case SL_NET_WIFI_EAP_SERVER_CREDENTIAL_ID:
    case SL_NET_WIFI_CLIENT_CREDENTIAL_ID_1:
    case SL_NET_WIFI_AP_CREDENTIAL_ID_1:
      status = sli_si91x_get_wifi_credential_type(type, &cred_type);
      VERIFY_STATUS_AND_RETURN(status);
      break;
    case SL_NET_USER_CREDENTIAL_ID:
      status = sli_si91x_get_wifi_credential_type(type, &cred_type);
      if (status != SL_STATUS_OK) {
        cred_type = (SL_WIFI_USER_CREDENTIAL | type);
      }
      break;
    case SL_NET_TLS_CLIENT_CREDENTIAL_START:
    case SL_NET_TLS_SERVER_CREDENTIAL_START:
    case SL_NET_MQTT_SERVER_CREDENTIAL_START:
    case SL_NET_MQTT_CLIENT_CREDENTIAL_START:
    case SL_NET_HTTP_SERVER_CREDENTIAL_START:
    case SL_NET_HTTP_CLIENT_CREDENTIAL_START:
      cred_type = (SL_WIFI_USER_CREDENTIAL | type);
      break;

    default:
      return SL_STATUS_FAIL;
  }

  return sl_wifi_set_credential(cred_id, cred_type, credential, credential_length);
}

sl_status_t sl_net_get_credential(sl_net_credential_id_t id,
                                  sl_net_credential_type_t *type,
                                  void *credential,
                                  uint32_t *credential_length)
{
  // Check if the credential ID is one of the invalid parameters
  if ((id == SL_NET_WIFI_EAP_SERVER_CREDENTIAL_ID) || (SL_NET_TLS_CLIENT_CREDENTIAL_START == (id & ~0xff))
      || (SL_NET_TLS_SERVER_CREDENTIAL_START == (id & ~0xff))) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Check if the credential is invalid parameter
  if ((NULL == credential) || (0 == *credential_length)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  int group_id                        = 0;
  int cred_id                         = 0;
  sl_status_t status                  = 0;
  sl_wifi_credential_type_t cred_type = 0;

  group_id = (id & SL_NET_CREDENTIAL_GROUP_MASK);

  if (group_id > 0) {
    cred_id = (SL_NET_USER_CREDENTIAL_ID + (group_id >> 8));
  } else {
    group_id = id;
    cred_id  = id;
  }

  status = sl_wifi_get_credential(cred_id, &cred_type, credential, credential_length);
  VERIFY_STATUS_AND_RETURN(status);

  switch (group_id) {
    case SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID:
    case SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID:
    case SL_NET_WIFI_EAP_CLIENT_CREDENTIAL_ID:
    case SL_NET_WIFI_EAP_SERVER_CREDENTIAL_ID:
    case SL_NET_WIFI_CLIENT_CREDENTIAL_ID_1:
    case SL_NET_WIFI_AP_CREDENTIAL_ID_1:
      sli_si91x_get_net_credential_type(cred_type, type);
      break;
    case SL_NET_USER_CREDENTIAL_ID:
    case SL_NET_TLS_CLIENT_CREDENTIAL_START:
    case SL_NET_TLS_SERVER_CREDENTIAL_START:
    case SL_NET_MQTT_SERVER_CREDENTIAL_START:
    case SL_NET_MQTT_CLIENT_CREDENTIAL_START:
    case SL_NET_HTTP_SERVER_CREDENTIAL_START:
    case SL_NET_HTTP_CLIENT_CREDENTIAL_START:
      *type = ((~SL_WIFI_USER_CREDENTIAL) & cred_type);
      break;
    default:
      return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

sl_status_t sl_net_delete_credential(sl_net_credential_id_t id, sl_net_credential_type_t type)
{
  if (CRED_TYPE_CERT == sli_si91x_check_cred_type(type)) {
#if defined(SLI_SI917)
    return sli_net_delete_credential(id, type);
#else
    return SL_STATUS_FAIL;
#endif
  }

  int group_id = 0;
  int cred_id  = 0;

  group_id = (id & SL_NET_CREDENTIAL_GROUP_MASK);

  if (group_id > 0) {
    cred_id = (SL_NET_USER_CREDENTIAL_ID + (group_id >> 8));
  } else {
    cred_id = id;
  }

  return sl_wifi_delete_credential(cred_id);
}
