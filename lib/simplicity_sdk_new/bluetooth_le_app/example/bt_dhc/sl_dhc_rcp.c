/***************************************************************************//**
 * @file
 * @brief Dynamic Hardware Configuration RCP (HCI) transport layer.
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
#include "sl_btctrl_hci_handler.h"
#include "sl_dhc.h"

// OGF: 0x003f Vendor-Specific debug commands
// OCF: 0x0309
// Opcode = OGF (upper 6 bits) | OCF (lower 10 bits) = 0xff09
// Example: 'sudo hcitool -i hci0 cmd 0x3f 0x309 dc 00 01'
#define DHC_HCI_VENDOR_MESSAGE_ID 0xff09

static sl_btctrl_command_handler_t dhc_hci_handler;
static bool dhc_hci_message_handler(struct sl_btctrl_hci_message *msg);
static bool handle_dhc_hci_command(uint8_t *params, size_t params_len, struct sl_btctrl_hci_message *msg);

void sl_dhc_init(void)
{
  sl_btctrl_hci_register_handler(&dhc_hci_handler, &dhc_hci_message_handler);
}

void sl_dhc_send_response(sl_status_t sc, uint8_t *data, size_t data_len, void *msg)
{
  sc = sl_btctrl_hci_message_set_response((struct sl_btctrl_hci_message *)msg, (uint8_t)sc, data, data_len);
  if (sc != SL_STATUS_OK) {
    sl_btctrl_hci_message_set_response((struct sl_btctrl_hci_message *)msg, (uint8_t)sc, NULL, 0);
  }
}

static bool dhc_hci_message_handler(struct sl_btctrl_hci_message *msg)
{
  uint16_t opcode;
  if (sl_btctrl_hci_message_get_opcode(msg, &opcode) != SL_STATUS_OK) {
    return false;
  }

  if (opcode == DHC_HCI_VENDOR_MESSAGE_ID) {
    uint8_t params[SL_BTCTRL_MAX_HCI_RESPONSE_LENGTH];
    memset(params, 0xFF, sizeof(params));

    if (sl_btctrl_hci_message_get_parameters(msg, (uint8_t*)&params, sizeof(params)) != SL_STATUS_OK) {
      sl_dhc_send_response(SL_STATUS_INVALID_PARAMETER, NULL, 0, msg);
      return true;
    }

    size_t length;
    if (sl_btctrl_hci_message_get_length(msg, &length) != SL_STATUS_OK) {
      return true;
    }

    return handle_dhc_hci_command(params, length, msg);
  }

  return false; // Command is not handled
}

static bool handle_dhc_hci_command(uint8_t *params, size_t params_len, struct sl_btctrl_hci_message *msg)
{
  // A valid command is at least 3 bytes long: ID (0xDC), read/write, setting
  if (params_len < DHC_HEADER_LEN) {
    sl_dhc_send_response(SL_STATUS_INVALID_COUNT, NULL, 0, msg);
    return true;
  }

  // Check for the DHC ID
  if (params[0] != DHC_ID) {
    sl_dhc_send_response(SL_STATUS_INVALID_PARAMETER, NULL, 0, msg);
    return true;
  }

  // Check the read/write command byte validity
  uint8_t command_raw = params[1];
  if (command_raw >= DHC_CMD_COUNT) {
    sl_dhc_send_response(SL_STATUS_INVALID_MODE, NULL, 0, msg);
    return true;
  }
  dhc_command_t command = (dhc_command_t)command_raw;

  // Check the setting byte validity
  uint8_t setting_raw = params[2];
  if (setting_raw >= DHC_SETTING_COUNT) {
    sl_dhc_send_response(SL_STATUS_INVALID_CONFIGURATION, NULL, 0, msg);
    return true;
  }
  dhc_setting_t setting = (dhc_setting_t)setting_raw;

  // Handle the received command
  switch (command) {
    case DHC_CMD_READ:
      sl_dhc_handle_read(setting, msg);
      break;

    case DHC_CMD_WRITE:
      sl_dhc_handle_write(setting, params, params_len, msg);
      break;

    default:
      break;
  }

  return true; // Command is handled
}
