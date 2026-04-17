/***************************************************************************//**
 * @file sl_rail_sdk_wmbus_sensor_pulse_counter_config.h
 * @brief Configuration file for the Wireless M-Bus pulse counter sensor.
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

#ifndef SL_RAIL_SDK_WMBUS_SENSOR_PULSE_COUNTER_CONFIG_H
#define SL_RAIL_SDK_WMBUS_SENSOR_PULSE_COUNTER_CONFIG_H

/**************************************************************************//**
 * @defgroup rail_sdk_wmbus_sensor_pulse_counter_defines Configurations
 * @ingroup rail_sdk_wmbus_sensor_pulse_counter
 * @{
 *****************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Pulse counter sleeptimer priority
// <o SL_RAIL_SDK_WMBUS_SENSOR_PULSE_COUNTER_SLEEPTIMER_PRIORITY> Priority of the sleeptimer
// <i> Default: 10
// <i> Priority of the sleeptimer used to periodically read the pulse counter.
#define SL_RAIL_SDK_WMBUS_SENSOR_PULSE_COUNTER_SLEEPTIMER_PRIORITY 10 ///< Priority of the sleeptimer used to periodically read the pulse counter.
// </h> Starting active sensor

// <h> Pulse counter sleeptimer timeout
// <o SL_RAIL_SDK_WMBUS_SENSOR_PULSE_COUNTER_SLEEPTIMER_TIMEOUT> Timeout of the sleeptimer
// <i> Default: 1000
// <i> Timeout of the sleeptimer used to periodically read the pulse counter.
#define SL_RAIL_SDK_WMBUS_SENSOR_PULSE_COUNTER_SLEEPTIMER_TIMEOUT 1000 ///< Timeout of the sleeptimer used to periodically read the pulse counter.
// </h> Starting active sensor

// <<< end of configuration section >>>

/** @}*/

#endif // SL_RAIL_SDK_WMBUS_SENSOR_PULSE_COUNTER_CONFIG_H
