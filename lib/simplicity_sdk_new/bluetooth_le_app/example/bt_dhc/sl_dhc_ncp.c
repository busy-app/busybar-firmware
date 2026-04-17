/***************************************************************************//**
 * @file
 * @brief Dynamic Hardware Configuration NCP transport layer.
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
#include "sl_ncp.h"
#include "sl_bt_api.h"
#include "sl_dhc.h"

void sl_dhc_init(void)
{
  // Nothing to do
}

void sl_dhc_send_response(sl_status_t status, uint8_t *data, size_t data_len, void *msg)
{
  (void)msg;
  sl_bt_send_rsp_user_message_to_target(status, data_len, data);
}

void sl_ncp_user_cmd_message_to_target_cb(void *data)
{
  uint8array *data_arr = (uint8array *)data;

  // A valid command is at least 3 bytes long: ID (0xDC), read/write, setting
  if (data_arr->len < DHC_HEADER_LEN) {
    sl_dhc_send_response(SL_STATUS_INVALID_COUNT, NULL, 0, NULL);
    return;
  }

  // Check for the Dynamic Hardware Configuration ID
  if (data_arr->data[0] != DHC_ID) {
    sl_dhc_send_response(SL_STATUS_INVALID_PARAMETER, NULL, 0, NULL);
    return;
  }

  // Check the read/write command byte validity
  uint8_t command_raw = data_arr->data[1];
  if (command_raw >= DHC_CMD_COUNT) {
    sl_dhc_send_response(SL_STATUS_INVALID_MODE, NULL, 0, NULL);
    return;
  }
  dhc_command_t command = (dhc_command_t)command_raw;

  // Check the setting byte validity
  uint8_t setting_raw = data_arr->data[2];
  if (setting_raw >= DHC_SETTING_COUNT) {
    sl_dhc_send_response(SL_STATUS_INVALID_CONFIGURATION, NULL, 0, NULL);
    return;
  }
  dhc_setting_t setting = (dhc_setting_t)setting_raw;

  // Handle the received command
  switch (command) {
    case DHC_CMD_READ:
      sl_dhc_handle_read(setting, NULL);
      break;

    case DHC_CMD_WRITE:
      sl_dhc_handle_write(setting, data_arr->data, data_arr->len, NULL);
      break;

    default:
      return;
  }
}
