/***************************************************************************//**
 * @file sl_rail_sdk_wmbus_sensor_cli.c
 * @brief Wireless M-Bus sensor command line interface.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_cli.h"
#include "app_log.h"
#include "sl_rail_sdk_wmbus_sensor_core.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void cli_list_sensors(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_rail_sdk_wmbus_sensor_t* sensors = sl_rail_sdk_wmbus_sensor_core_list_available_sensors();

  for (size_t i = 0; i < sl_rail_sdk_wmbus_sensor_core_get_sensor_count(); i++) {
    app_log_info("\t ID: %d: %s \n", sensors[i].id, sensors[i].name);
  }
}

void cli_get_active_sensor(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_rail_sdk_wmbus_sensor_t* sensor = sl_rail_sdk_wmbus_sensor_core_get_active_sensor();
  app_log_info("Active sensor: %s\n", sensor->name);
}

void cli_set_active_sensor(sl_cli_command_arg_t *arguments)
{
  int sensor_id = sl_cli_get_argument_uint8(arguments, 0);
  if (sl_rail_sdk_wmbus_sensor_core_set_active_sensor(sensor_id) != SL_STATUS_OK) {
    app_log_error("Invalid sensor ID\n");
    return;
  }
  cli_get_active_sensor(arguments);
}
