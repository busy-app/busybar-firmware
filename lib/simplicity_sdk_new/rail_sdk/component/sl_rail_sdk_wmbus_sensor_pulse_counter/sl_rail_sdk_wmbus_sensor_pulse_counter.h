/***************************************************************************//**
 * @file sl_rail_sdk_wmbus_sensor_pulse_counter.h
 * @brief Wireless M-Bus Button Pulse Counter (Wireless M-Bus Meter Only)
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

#ifndef SL_RAIL_SDK_WMBUS_SENSOR_PULSE_COUNTER_H
#define SL_RAIL_SDK_WMBUS_SENSOR_PULSE_COUNTER_H

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
 * \addtogroup rail_sdk_wmbus_sensor_pulse_counter
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
 * @brief Wireless M-Bus Pulse counter initialization function.
 *
 * @return sl_status_t:  SL_STATUS_OK if init was successful
 *                        SL_STATUS_FAIL if init failed
 */
sl_status_t sl_rail_sdk_wmbus_sensor_pulse_counter_init(void);

/**
 * @brief Wireless M-Bus Pulse counter deinitialization function.
 *
 * @return sl_status_t:  SL_STATUS_OK if init was successful
 *                        SL_STATUS_FAIL if init failed
 */
sl_status_t sl_rail_sdk_wmbus_sensor_pulse_counter_deinit(void);

/**
 * @brief Wireless M-Bus Pulse counter measure function.
 *
 * @return sl_status_t:  SL_STATUS_OK if measure was successful
 *                        SL_STATUS_FAIL if measure failed
 */
sl_status_t sl_rail_sdk_wmbus_sensor_pulse_counter_measure(void);

/**
 * @brief Retrieves the sensor data for the Wireless M-Bus pulse counter sensor.
 *
 * Returns a pointer to the @ref sl_rail_sdk_wmbus_sensor_data_t structure that contains
 * the sensor data for the Wireless M-Bus pulse counter sensor.
 *
 * @return Pointer to a @ref sl_rail_sdk_wmbus_sensor_data_t, the sensor data for the
 *                                   Wireless M-Bus pulse counter sensor
 */
sl_rail_sdk_wmbus_sensor_data_t* sl_rail_sdk_wmbus_sensor_pulse_counter_get_data(void);

/**
 * @brief Wireless M-Bus Pulse counter 7 segment LCD print function.
 *
 * @return sl_status_t:  SL_STATUS_OK if 7 segment LCD print was successfully
 *                        SL_STATUS_FAIL if 7 segment LCD print failed
 */
sl_status_t sl_rail_sdk_wmbus_sensor_pulse_counter_print(void);

/**
 * @brief Wireless M-Bus Pulse counter button pressed function. This function
 *        handles the button press event for the Wireless M-Bus pulse counter,
 *        in this case, it increments the counter value.
 *
 * @return sl_status_t:  SL_STATUS_OK if button press was successfully
 *                        SL_STATUS_FAIL if button press failed
 */
sl_status_t sl_rail_sdk_wmbus_sensor_pulse_counter_button_pressed(void);

#endif // SL_RAIL_SDK_WMBUS_SENSOR_PULSE_COUNTER_H
/** @} */ // end of wmbus_sensor_pulse_counter group
/** @} */ // end of Wireless M-Bus group
