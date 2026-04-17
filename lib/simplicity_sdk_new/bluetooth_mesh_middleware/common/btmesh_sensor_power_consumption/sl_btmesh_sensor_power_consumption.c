/***************************************************************************//**
 * @file
 * @brief BT Mesh Power consumption sensor implementation
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

#include "sl_core.h"
#include "sl_component_catalog.h"

#ifdef SL_CATALOG_CLI_PRESENT
#include "sl_cli.h"
#include "app_btmesh_util.h"
#endif // SL_CATALOG_CLI_PRESENT

#include "app_log.h"
#include "sl_btmesh_sensor_power_consumption.h"
#include "sl_btmesh_sensor_power_consumption_config.h"

// Power consumption
static energy32_t energy_usage = SL_BTMESH_SENSOR_ENERGY_USAGE_VALUE_IS_NOT_KNOWN;

/*******************************************************************************
 * Set the power consumption value. It could be used to initialize the sensor.
 *
 * @param[in] power_consumption_value Power consumption value to set
 ******************************************************************************/
void sl_btmesh_set_power_consumption(energy32_t energy_usage_value)
{
  CORE_ATOMIC_SECTION(
    energy_usage = energy_usage_value;
    )
}

/*******************************************************************************
 * Get the power consumption value measured by sensor.
 *
 * @return Current value of power consumption.
 ******************************************************************************/
energy32_t sl_btmesh_get_power_consumption(void)
{
  energy32_t ret_val;
  CORE_ATOMIC_SECTION(
    ret_val = energy_usage;
    )
  return ret_val;
}

/**************************************************************************//**
 * CLI Callbacks
 *****************************************************************************/
#ifdef SL_CATALOG_CLI_PRESENT
void power_consumption_set_from_cli(sl_cli_command_arg_t *arguments)
{
  energy32_t value;
  value = (energy32_t)sl_cli_get_argument_uint32(arguments, 0);
  sl_btmesh_set_power_consumption(value);
}

void power_consumption_get_from_cli(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  app_log("%lu\r\n", (uint32_t)energy_usage);
}

#endif // SL_CATALOG_CLI_PRESENT
