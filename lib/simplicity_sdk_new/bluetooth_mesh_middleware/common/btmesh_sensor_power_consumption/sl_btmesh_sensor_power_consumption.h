/***************************************************************************//**
 * @file
 * @brief BT Mesh Energy monitor sensor header
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

#ifndef SL_BTMESH_SENSOR_POWER_CONSUMPTION_H
#define SL_BTMESH_SENSOR_POWER_CONSUMPTION_H

#include "sl_btmesh_device_properties.h"

#define SL_BTMESH_SENSOR_ENERGY_USAGE_VALUE_IS_NOT_KNOWN  (0xFFFFFFFF)

/***************************************************************************//**
 * @addtogroup btmesh_sensor_power_consumption
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * Set the power consumption value. It could be used to initialize the sensor.
 *
 * @param[in] energy_usage Power consumption value to set
 ******************************************************************************/
void sl_btmesh_set_power_consumption(energy32_t energy_usage);

/***************************************************************************//**
 * Get the current power consumption value measured by sensor.
 *
 * @return Current value of power consumption.
 ******************************************************************************/
energy32_t sl_btmesh_get_power_consumption(void);

/** @} (end addtogroup btmesh_sensor_power_consumption) */

#endif /* SL_BTMESH_SENSOR_POWER_CONSUMPTION_H */
