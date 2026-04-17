/***************************************************************************//**
 * @file sl_rail_sdk_wmbus_sensor_virtual_water_meter_config.h
 * @brief Configuration file for the Wireless M-Bus virtual water meter sensor.
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

#ifndef SL_RAIL_SDK_WMBUS_SENSOR_VIRTUAL_WATER_METER_CONFIG_H
#define SL_RAIL_SDK_WMBUS_SENSOR_VIRTUAL_WATER_METER_CONFIG_H

/**************************************************************************//**
 * @defgroup rail_sdk_wmbus_sensor_virtual_water_meter_defines Configurations
 * @ingroup rail_sdk_wmbus_sensor_virtual_water_meter
 * @{
 *****************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Default sleeptimer priority
// <o SL_RAIL_SDK_WMBUS_SENSOR_VIRTUAL_WATER_METER_SLEEPTIMER_PRIORITY> Default sleeptimer priority
// <i> Default: 0
#define SL_RAIL_SDK_WMBUS_SENSOR_VIRTUAL_WATER_METER_SLEEPTIMER_PRIORITY 10 ///< Default sleeptimer priority
// </h> Starting default channel

// <h> Default sleeptimer timeout
// <o SL_RAIL_SDK_WMBUS_SENSOR_VIRTUAL_WATER_METER_SLEEPTIMER_TIMEOUT> Default sleeptimer timeout
// <i> Default: 0
#define SL_RAIL_SDK_WMBUS_SENSOR_VIRTUAL_WATER_METER_SLEEPTIMER_TIMEOUT 1000 ///< Default sleeptimer timeout
// </h> Starting default channel

// <<< end of configuration section >>>

/** @}*/
#endif // SL_RAIL_SDK_WMBUS_SENSOR_VIRTUAL_WATER_METER_CONFIG_H
