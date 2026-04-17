/***************************************************************************//**
 * @file
 * @brief CLI for the Update App Link Key plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "update-app-link-key.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"

void sl_zigbee_af_set_app_link_key_update_request_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t eui;
  sl_zigbee_copy_eui64_arg(arguments, 0, eui, true);
  sl_status_t status = sl_zigbee_af_update_app_link_key(eui);
  sl_zigbee_af_core_println("Requesting app link key: 0x%02X", status);
}
