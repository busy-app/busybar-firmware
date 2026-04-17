/***************************************************************************//**
 * @file sl_rail_sdk_wmbus_sensor_virtual_water_meter.h
 * @brief Wireless M-Bus Virtual Water Meter Sensor (Wireless M-Bus Meter Only)
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

#ifndef SL_RAIL_SDK_WMBUS_SENSOR_VIRTUAL_WATER_METER_H
#define SL_RAIL_SDK_WMBUS_SENSOR_VIRTUAL_WATER_METER_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"

/**
 * \addtogroup rail_sdk_wmbus
 * @{
 */
/**
 * \addtogroup rail_sdk_wmbus_sensor_virtual_water_meter
 * @{
 */
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/**
 * @brief Wireless M-Bus Virtual Water Meter initialization function.
 *
 * @return sl_status_t:  SL_STATUS_OK if init was successful
 *                        SL_STATUS_FAIL if init failed
 */
sl_status_t sl_rail_sdk_wmbus_sensor_virtual_water_meter_init(void);

/**
 * @brief Wireless M-Bus Virtual Water Meter deinitialization function.
 *
 * @return sl_status_t:  SL_STATUS_OK if init was successful
 *                        SL_STATUS_FAIL if init failed
 */
sl_status_t sl_rail_sdk_wmbus_sensor_virtual_water_meter_deinit(void);

/**
 * @brief Wireless M-Bus Virtual Water Meter measure function.
 *
 * @return sl_status_t:  SL_STATUS_OK if measure was successful
 *                        SL_STATUS_FAIL if measure failed
 */
sl_status_t sl_rail_sdk_wmbus_sensor_virtual_water_meter_measure(void);

/**
 * @brief Retrieves the sensor data for the Wireless M-Bus virtual water meter sensor.
 *
 * Returns a pointer to the @ref sl_rail_sdk_wmbus_sensor_data_t structure that contains
 * the sensor data for the Wireless M-Bus virtual water meter sensor.
 *
 * @return Pointer of a @ref sl_rail_sdk_wmbus_sensor_data_t, to the sensor data for the
 *                                   Wireless M-Bus virtual water meter sensor
 */
sl_rail_sdk_wmbus_sensor_data_t* sl_rail_sdk_wmbus_sensor_virtual_water_meter_get_data(void);

/**
 * @brief Wireless M-Bus Virtual Water Meter 7 segment LCD print function.
 *
 * @return sl_status_t:  SL_STATUS_OK if 7 segment LCD print was successfully
 *                        SL_STATUS_FAIL if 7 segment LCD print failed
 */
sl_status_t sl_rail_sdk_wmbus_sensor_virtual_water_meter_print(void);

/**
 * @brief Wireless M-Bus Virtual Water Meter button pressed function. This function
 *        handles the button press event for the Wireless M-Bus virtual water meter sensor,
 *        in this case, it resets the total water consumption.
 *
 * @return sl_status_t:  SL_STATUS_OK if button press was successful
 *                        SL_STATUS_FAIL if button pressed failed
 */
sl_status_t sl_rail_sdk_wmbus_sensor_virtual_water_meter_button_pressed(void);

#endif // SL_RAIL_SDK_WMBUS_SENSOR_VIRTUAL_WATER_METER_H
/** @} */ // end of wmbus_sensor_virtual_water_meter group
/** @} */ // end of Wireless M-Bus group
