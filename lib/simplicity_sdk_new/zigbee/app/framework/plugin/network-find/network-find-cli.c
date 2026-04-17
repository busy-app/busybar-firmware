/***************************************************************************//**
 * @file
 * @brief CLI for the Network Find plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "app/framework/include/af.h"
#include "network-find.h"

//------------------------------------------------------------------------------
// Definitions for legacy CLI

#include "app/util/serial/sl_zigbee_command_interpreter.h"
void findMaskCommand(sl_cli_command_arg_t *arguments)
{
  const uint8_t page = sl_cli_get_argument_uint8(arguments, 0);
  const uint32_t mask = sl_cli_get_argument_uint32(arguments, 1);

  const sl_status_t status = sl_zigbee_af_set_form_and_join_channel_mask(page, mask);

  switch (status) {
    case SL_STATUS_OK:
      break;
    case SL_STATUS_TRANSMIT_INVALID_CHANNEL:
      sl_zigbee_af_core_println("Invalid channel page %d", page);
      break;
    case SL_STATUS_INVALID_CHANNEL_MASK:
      sl_zigbee_af_core_println("Invalid mask for channel page %d", page);
      break;
    default:
      sl_zigbee_af_core_println("Error 0x%02X", status);
      break;
  }
}

void findModeCommand(sl_cli_command_arg_t *arguments)
{
  const uint8_t mode = sl_cli_get_argument_uint8(arguments, 0);

  const sl_status_t status = sl_zigbee_af_set_form_and_join_search_mode(mode);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_core_println("Error 0x%02X", status);
  }
}

void findPrintCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  uint8_t page;
  uint8_t mode;
  bool allowAllChannels;

  // Only page 0 (and 28-31 if sub-GHz support is included) are valid.
  // For simplicity, go from 0 to 31 and simply skip any invalid page.
  for (page = 0; page <= 31; ++page) {
    const uint32_t mask = sl_zigbee_af_get_form_and_join_channel_mask(page);
    if (mask != 0xFFFFFFFFU) {
      sl_zigbee_af_core_print("Page %d mask 0x%08X, %s", page, mask, "channels: ");
      sl_zigbee_af_core_debug_exec(sl_zigbee_af_print_channel_list_from_mask(mask));
      sl_zigbee_af_core_println("");
    }
  }

  mode = sl_zigbee_af_get_form_and_join_search_mode();
  sl_zigbee_af_core_println("%s%s scan is %sABLED",
                            "2.4 ",
                            "GHz",
                            (mode & FIND_AND_JOIN_MODE_ALLOW_2_4_GHZ) ? "EN" : "DIS");
  sl_zigbee_af_core_println("%s%s scan is %sABLED",
                            "Sub-",
                            "GHz",
                            (mode & FIND_AND_JOIN_MODE_ALLOW_SUB_GHZ) ? "EN" : "DIS");

  allowAllChannels = sl_zigbee_af_network_find_get_enable_scanning_all_channels_cb();
  sl_zigbee_af_core_println("%s%s scan is %sABLED",
                            "Fallback on ",
                            "all channels",
                            allowAllChannels ? "EN" : "DIS");
}
