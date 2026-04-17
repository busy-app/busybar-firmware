/***************************************************************************//**
 * @file
 * @brief Zigbee Dynamic Node Type CLI Handler
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
#include "app/framework/util/af-main.h"
#include "dynamic-node-type.h"

void sl_zigbee_af_dynamic_node_type_switch_to_router_cli_handler(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_af_switch_role_router();  // updates stack
}

void sl_zigbee_af_dynamic_node_type_switch_to_sleepy_end_device_cli_handler(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_node_type_t end_device_configuration = (sl_zigbee_node_type_t)sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_af_switch_role_sleepy_end_device(end_device_configuration);
}
