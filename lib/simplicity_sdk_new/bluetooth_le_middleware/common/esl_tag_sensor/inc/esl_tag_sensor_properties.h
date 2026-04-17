/***************************************************************************//**
 * @file
 * @brief Bluetooth mesh device properties used by ESL tag sensors.
 *
 * For a complete list of device properties, see sl_btmesh_device_properties.h.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef ESL_TAG_SENSOR_PROPERTIES_H
#define ESL_TAG_SENSOR_PROPERTIES_H

#include <stdint.h>

/**
 * Voltage
 * The Voltage characteristic is used to represent a measure of positive
 * electric potential difference in units of volts.
 * Unit is volt with a resolution of 1/64V.
 * Range: 0.0 to 1022.0
 * A value of 0xFFFF represents 'value is not known'.
 * The minimum representable value represents the minimum value or lower,
 * the maximum representable value represents the maximum value or higher.
 */
typedef uint16_t voltage_t;

/**
 * Date UTC
 * Date as days elapsed since the Epoch (Jan 1, 1970) in the Coordinated Universal Time (UTC) time zone.
 * Unit is a day with a resolution of 1. Range is from 1 to 16777214.
 * A value of 0x000000 represents 'value is not known'.
 * NOTE: It is a 24-bit field;
 */
typedef uint32_t date_utc_t;

/**
 * The Temperature 8 characteristic is used to represent a measure of
 * temperature with a unit of 0.5 degree Celsius.
 * Unit is degree Celsius with a resolution of 0.5
 * A value of 0xFF represents 'value is not known'.
 */
typedef int8_t temperature_8_t;

/**
 * Fixed String 8 *
 * The Fixed String 8 characteristic represents a 8-octet UTF-8 string.
 */
typedef uint8_t fixed_string8[8];

/**
 * Temperature
 * Unit is in degrees Celsius with a resolution of 0.01 degrees Celsius
 * Range: 327.68 to 327.67.
 * NOTE: there is no 'UNKNOWN' value defined in the specification
 */
typedef int16_t temperature_t;

/**
 * Temperature Range
 * This characteristic aggregates two instances of the Temperature characteristic to represent a temperature range.
 */
typedef struct temperature_range {
  /** Minimum Temperature */
  temperature_t minimum;
  /** Maximum temperature */
  temperature_t maximum;
} temperature_range_t;

/** Device Date Of Manufacture
 * Type: Date Utc */
#define DEVICE_DATE_OF_MANUFACTURE                       0x000C
/** Device Firmware Revision
 * Type: Fixed String 8 */
#define DEVICE_FIRMWARE_REVISION                         0x000E
/** Device Operating Temperature Range Specification
 * Type: Temperature Range */
#define DEVICE_OPERATING_TEMPERATURE_RANGE_SPECIFICATION 0x0013
/** Present Ambient Temperature
 * Type: Temperature 8 */
#define PRESENT_AMBIENT_TEMPERATURE                      0x004F
/** Present Device Operating Temperature
 * Type: Temperature */
#define PRESENT_DEVICE_OPERATING_TEMPERATURE             0x0054
/** Present Input Voltage
 * Type: Voltage */
#define PRESENT_INPUT_VOLTAGE                            0x0059

#endif // ESL_TAG_SENSOR_PROPERTIES_H
