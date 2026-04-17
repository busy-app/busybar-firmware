/***************************************************************************//**
 * @file sl_rail_sdk_wmbus_packet_assembler.h
 * @brief Wireless M-Bus packet assembler.
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

#ifndef SL_RAIL_SDK_WMBUS_PACKET_ASSEMBLER_H
#define SL_RAIL_SDK_WMBUS_PACKET_ASSEMBLER_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "sl_rail_sdk_wmbus_support.h"

/**
 * \addtogroup rail_sdk_wmbus
 * @{
 */
/**
 * \addtogroup rail_sdk_wmbus_support
 * @{
 */

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @addtogroup rail_sdk_wmbus_support_types Type definitions
 * @ingroup rail_sdk_wmbus_support
 * @{
 *****************************************************************************/
/**
 * @brief Structure representing a Wireless M-Bus sensor data.
 *
 * Contains the value information field and the data field.
 */
typedef struct sl_rail_sdk_wmbus_sensor_data{
  uint32_t vif; /*!< value information field */
  uint32_t data; /*!< data field */
  struct sl_rail_sdk_wmbus_sensor_data* next; /*!< pointer to the next item */
} sl_rail_sdk_wmbus_sensor_data_t;

/** @} (end rail_sdk_wmbus_support_types) */

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/**
 * @brief Sets up a Wireless M-Bus frame.
 *
 * Sets up a Wireless M-Bus frame by populating the provided buffer with the necessary information.
 *
 * @param[out] buffer The buffer to store the Wireless M-Bus frame.
 * @param[in] access_number The access number of the frame.
 * @param[in] accessibility The accessibility of the frame.
 * @param[in] new_device_type The device type of the frame.
 * @param[in] sensors_data An array of data to be included in the frame.
 * @param[in] periodic Indicates whether the frame is periodic or not.
 * @param[in] encrypt Indicates whether the frame should be encrypted or not.
 * @return The length of the assembled frame.
 */
uint16_t sl_rail_sdk_wmbus_setup_frame(uint8_t *buffer, uint8_t access_number, sl_rail_sdk_wmbus_accessibility_t accessibility, sl_rail_sdk_wmbus_device_type_t new_device_type, sl_rail_sdk_wmbus_sensor_data_t* sensors_data, bool periodic, bool encrypt);

#endif // SL_RAIL_SDK_WMBUS_PACKET_ASSEMBLER_H
/** @} */ // end of wmbus_support group
/** @} */ // end of Wireless M-Bus group
