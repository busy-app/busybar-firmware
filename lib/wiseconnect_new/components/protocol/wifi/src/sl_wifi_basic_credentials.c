/***************************************************************************/ /**
 * @file  sl_wifi_basic_credentials.c
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

#include <string.h>
#include "sl_wifi.h"
#include "sl_wifi_types.h"
#include "sl_common.h"

#ifndef SL_WIFI_MAX_CREDENTIAL_COUNT
#define SL_WIFI_MAX_CREDENTIAL_COUNT 14
#endif

typedef struct {
  sl_wifi_credential_type_t type;
  uint16_t data_length;
  uint8_t data[];
} sl_wifi_basic_credential_entry_t;

static sl_wifi_basic_credential_entry_t *credentials[SL_WIFI_MAX_CREDENTIAL_COUNT] = { 0 };

sl_status_t sl_wifi_set_credential(sl_wifi_credential_id_t id,
                                   sl_wifi_credential_type_t type,
                                   const void *credential,
                                   uint32_t credential_length)
{
  sl_wifi_basic_credential_entry_t *entry = NULL;

  if (id >= SL_WIFI_MAX_CREDENTIAL_COUNT) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Check if the credential is invalid parameter
  if ((NULL == credential) || (0 == credential_length)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // If a credential already exists for the given ID, free the allocated memory and allocate memory for the new credential
  if (credentials[id] != NULL) {
    free(credentials[id]);
  }
  credentials[id] = malloc(sizeof(sl_wifi_basic_credential_entry_t) + credential_length);
  if (credentials[id] == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  memset(credentials[id], 0, sizeof(sl_wifi_basic_credential_entry_t) + credential_length);

  entry              = credentials[id];
  entry->type        = type;
  entry->data_length = (uint16_t)credential_length;
  memcpy(entry->data, credential, entry->data_length);

  return SL_STATUS_OK;
}

sl_status_t sl_wifi_get_credential(sl_wifi_credential_id_t id,
                                   sl_wifi_credential_type_t *type,
                                   void *credential,
                                   uint32_t *credential_length)
{
  const sl_wifi_basic_credential_entry_t *entry = NULL;

  // Check if the credential ID is invalid parameter
  if (id >= SL_WIFI_MAX_CREDENTIAL_COUNT) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Check if the credential is invalid parameter
  if ((NULL == credential) || (0 == *credential_length)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  entry = credentials[id];

  if (NULL == entry) {
    return SL_STATUS_NOT_FOUND;
  }

  if (*credential_length < entry->data_length) {
    return SL_STATUS_FAIL;
  }

  *type              = entry->type;
  *credential_length = entry->data_length;
  memcpy(credential, entry->data, entry->data_length);

  return SL_STATUS_OK;
}

sl_status_t sl_wifi_delete_credential(sl_wifi_credential_id_t id)
{
  // Check if the credential ID is invalid parameter
  if (id >= SL_WIFI_MAX_CREDENTIAL_COUNT) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (NULL != credentials[id]) {
    free(credentials[id]);
    credentials[id] = NULL;
  }

  return SL_STATUS_OK;
}
