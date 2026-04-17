/********************************************************************************
 * @file  app_xapi_net_cred.c
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
#include "sl_status.h"
#include "app_wifi_xapi_cmd_rx_handler.h"
#include "si91x_device.h"
#include "sl_net.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_driver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sl_net_wifi_types.h"

DEFINE_XAPI_CLASS(wifi, net_cred, WIFI, NET_CRED, NULL, NULL);

/******************************************************
*               Function Definitions
******************************************************/

void app_wifi_cmd_net_cred_set(app_wifi_cmd_net_cred_set_t *cmd_input)
{
  sl_status_t status = SL_STATUS_FAIL;

  // Validate input parameter
  if (cmd_input == NULL) {
    app_wifi_rsp_net_cred_set(SL_STATUS_INVALID_PARAMETER);
    return;
  }

  // Store the credential using ID, type, and provided data
  status = sl_net_set_credential(cmd_input->id,
                                 cmd_input->type,
                                 (const void *)(cmd_input->credential.data),
                                 cmd_input->credential.len);

  // Send response with operation status
  app_wifi_rsp_net_cred_set(status);
  return;
}

void app_wifi_cmd_net_cred_get(app_wifi_cmd_net_cred_get_t *cmd_input)
{
  uint8_t credential[256];                                       // Buffer to hold retrieved credential data
  uint32_t credential_length               = sizeof(credential); // Initial length of credential buffer
  sl_net_credential_type_t credential_type = cmd_input->type;    // Credential type to retrieve
  sl_status_t status                       = SL_STATUS_FAIL;

  // Retrieve credential data based on ID and type
  status = sl_net_get_credential(cmd_input->id, &credential_type, &credential, &credential_length);

  // Send response with credential data and status
  app_wifi_rsp_net_cred_get(status, credential_length, credential_length, credential);
  return;
}

void app_wifi_cmd_net_cred_delete(app_wifi_cmd_net_cred_delete_t *cmd_input)
{
  sl_status_t status = SL_STATUS_FAIL;

  // Delete credential using provided ID and type
  status = sl_net_delete_credential(cmd_input->id, cmd_input->type);

  // Send response with operation status
  app_wifi_rsp_net_cred_delete(status);
  return;
}
