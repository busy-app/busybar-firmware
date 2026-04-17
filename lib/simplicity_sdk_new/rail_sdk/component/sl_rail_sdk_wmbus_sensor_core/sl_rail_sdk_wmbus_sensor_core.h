/***************************************************************************//**
 * @file sl_rail_sdk_wmbus_sensor_core.h
 * @brief Wireless M-Bus Sensor Core (Wireless M-Bus Meter Only)
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

#ifndef SL_RAIL_SDK_WMBUS_SENSOR_CORE_H
#define SL_RAIL_SDK_WMBUS_SENSOR_CORE_H

/**
 * \addtogroup rail_sdk_wmbus
 * @{
 */
/**
 * \addtogroup rail_sdk_wmbus_sensor_core
 * @{
 */

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "sl_rail.h"
#include "sl_status.h"
#include "sl_rail_sdk_wmbus_packet_assembler.h"

/**************************************************************************//**
 * @addtogroup rail_sdk_wmbus_sensor_core_types Type definitions
 * @ingroup rail_sdk_wmbus_sensor_core
 * @{
 *****************************************************************************/
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/**
 * @typedef sl_rail_sdk_wmbus_sensor_init_t
 * @brief Typedef for a function pointer to initialize the Wireless M-Bus sensor.
 *
 * Defines a function pointer type for a function that initializes
 * the Wireless M-Bus sensor. Returns a status of type sl_status_t.
 *
 * @return sl_status_t Status code indicating the result of the initialization.
 */
typedef sl_status_t (*sl_rail_sdk_wmbus_sensor_init_t)(void);

/**
 * @typedef sl_rail_sdk_wmbus_sensor_deinit_t
 * @brief Typedef for a function pointer used to deinitialize the Wireless M-Bus sensor.
 *
 * Defines a function pointer type for a function that deinitialize
 * the Wireless M-Bus sensor. Returns a status of type sl_status_t.
 *
 * @return sl_status_t Status code indicating the result of the deinitialization.
 */
typedef sl_status_t (*sl_rail_sdk_wmbus_sensor_deinit_t)(void);

/**
 * @typedef sl_rail_sdk_wmbus_sensor_measure_t
 * @brief Typedef for a function pointer that performs a sensor measurement.
 *
 * Defines a function pointer type for a function that performs
 * a sensor measurement and returns a status code of type sl_status_t.
 *
 * @return sl_status_t Status code indicating the result of the measurement.
 */
typedef sl_status_t (*sl_rail_sdk_wmbus_sensor_measure_t)(void);

/**
 * @typedef sl_rail_sdk_wmbus_sensor_get_data_t
 * @brief Function pointer type for getting Wireless M-Bus sensor data.
 *
 * Defines a function pointer type for a function that returns
 * a pointer to a @ref sl_rail_sdk_wmbus_sensor_data_t structure.
 *
 * @return Pointer to a @ref sl_rail_sdk_wmbus_sensor_data_t structure.
 */
typedef sl_rail_sdk_wmbus_sensor_data_t* (*sl_rail_sdk_wmbus_sensor_get_data_t)(void);

/**
 * @typedef sl_rail_sdk_wmbus_sensor_print_t
 * @brief Typedef for a function pointer used to print sensor information.
 *
 * Defines a function pointer type for functions that print
 * sensor information in the Wireless M-Bus sensor core SDK.
 *
 * @return A status code of type sl_status_t indicating the result of the print operation.
 */
typedef sl_status_t (*sl_rail_sdk_wmbus_sensor_print_t)(void);

/**
 * @typedef sl_rail_sdk_wmbus_sensor_button_pressed_t
 * @brief Typedef for a function pointer that handles button press events.
 *
 * Defines a function pointer type for a function that is called
 * when a button is pressed in the Wireless M-Bus sensor application.
 *
 * @return A status code of type sl_status_t indicating the result of the button press handling.
 */
typedef sl_status_t (*sl_rail_sdk_wmbus_sensor_button_pressed_t)(void);

/**
 * @brief Structure representing a Wireless M-Bus sensor.
 *
 * This structure contains function pointers to initialize the sensor,
 * deinitialize the sensor, measure data, get data, print data on
 * a 7 segment LCD display, and handle button press events.
 */
typedef struct {
  uint8_t id;                                               /**< Sensor ID */
  const char* name;                                         /**< Name of the sensor */
  sl_rail_sdk_wmbus_sensor_init_t init_fnc_ptr;                      /**< Function pointer to initialize the sensor */
  sl_rail_sdk_wmbus_sensor_init_t deinit_fnc_ptr;                    /**< Function pointer to deinitialize the sensor */
  sl_rail_sdk_wmbus_sensor_measure_t measure_fnc_ptr;                /**< Function pointer to measure data */
  sl_rail_sdk_wmbus_sensor_get_data_t get_data_fnc_ptr;              /**< Function pointer to get data */
  sl_rail_sdk_wmbus_sensor_print_t print_fnc_ptr;                    /**< Function pointer to print data on an 7 segment LCD display */
  sl_rail_sdk_wmbus_sensor_button_pressed_t button_pressed_fnc_ptr;  /**< Function pointer to handle button press */
} sl_rail_sdk_wmbus_sensor_t;

/** @} (end rail_sdk_wmbus_sensor_core_types) */

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/**
 * @brief Get the active sensor.
 *
 * @return @ref sl_rail_sdk_wmbus_sensor_t* Pointer to the active sensor.
 */
sl_rail_sdk_wmbus_sensor_t* sl_rail_sdk_wmbus_sensor_core_get_active_sensor(void);

/**
 * @brief Set the active sensor.
 *
 * @param[in] id: Sensor ID
 *
 * @return sl_status_t: SL_STATUS_OK if successful, SL_STATUS_INVALID_PARAMETER
 *                       if the sensor ID is invalid
 */
sl_status_t sl_rail_sdk_wmbus_sensor_core_set_active_sensor(uint8_t id);

/**
 * @brief Get the list of available sensors.
 *
 * @return @ref sl_rail_sdk_wmbus_sensor_t*: Pointer to the list of available sensors
 */
sl_rail_sdk_wmbus_sensor_t* sl_rail_sdk_wmbus_sensor_core_list_available_sensors(void);

/**
 * @brief Get the number of available sensors.
 *
 * @return size_t: Number of available sensors
 */
size_t sl_rail_sdk_wmbus_sensor_core_get_sensor_count(void);

/**
 * @brief Initialize the Wireless M-Bus sensor core.
 *
 * Calls the initialization function of the active sensor.
 *
 * @return sl_status_t: SL_STATUS_OK if successful, SL_STATUS_FAILED if error
 */
sl_status_t sl_rail_sdk_wmbus_sensor_core_init(void);

/**
 * @brief Initialize the Wireless M-Bus sensor core LCD.
 */
void sl_rail_sdk_wmbus_sensor_core_lcd_init(void);

/**
 * @brief Process the Wireless M-Bus sensor core.
 *
 * Calls the packet assembler functions of the active sensor.
 * It also passes the tx_buffer, access_number, and Wireless M-Bus accessibility to the packet assembler function.
 *
 * @param[in] tx_buffer The buffer to store the transmitted data.
 * @param[out] access_number The access number for the Wireless M-Bus packet.
 * @return The result of the packet assembler function.
 */
uint16_t sl_rail_sdk_wmbus_sensor_core_process(uint8_t* tx_buffer, uint8_t* access_number);

#endif // SL_RAIL_SDK_WMBUS_SENSOR_CORE_H
/** @} */ // end of wmbus_sensor_core group
/** @} */ // end of Wireless M-Bus group
