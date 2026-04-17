/***************************************************************************/ /**
 * @file sensorhub_error_codes.h
 * @brief sensorhub_error_codes example
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef ERROR_CODES_H_
#define ERROR_CODES_H_

#include "Driver_Common.h"

/**************************************************************************/ /**
 * @addtogroup SENSOR-HUB Sensor Hub
 * @ingroup SI91X_SERVICE_APIS
 * @{
 *
 ******************************************************************************/
/* ERROR CODES */

/****************************ERRORS MASK******************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
#define SL_SH_SENSORHUB_ERRORS_MASK 0
/** @endcond */

/****************************TIMER ERRORS******************************/

#define SL_SH_TIMER_CREATION_FAILED (1 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Creating OS timer for the sensor failed
#define SL_SH_TIMER_DELETION_FAILED (2 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Deleting OS timer of the sensor failed
#define SL_SH_TIMER_START_FAIL      (3 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Starting OS timer of the sensor failed
#define SL_SH_TIMER_STOP_FAIL       (4 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Stopping OS timer of the sensor failed

/****************************SENSORHUB ERRORS******************************/

#define SL_SH_MAX_SENSORS_REACHED (5 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Max number of sensor limit reached
#define SL_SH_MEMORY_LIMIT_EXCEEDED \
  (6 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Memory allocation for sensor data storage failed
#define SL_SH_SENSOR_CREATE_FAIL \
  (7 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Sensor API is called for a sensor without creating it
#define SL_SH_COMMAND_SET_POWER_FAIL (8 << SL_SH_SENSORHUB_ERRORS_MASK)  ///< Sensor set power command execution failed
#define SL_SH_COMMAND_SET_RANGE_FAIL (9 << SL_SH_SENSORHUB_ERRORS_MASK)  ///< Sensor set range command execution failed
#define SL_SH_COMMAND_SELF_TEST_FAIL (10 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Sensor self-test command execution failed
#define SL_SH_INVALID_PARAMETERS     (11 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Generic error code for any invalid parameters
#define SL_SH_GPIO_OUT_OF_RANGE      (12 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Invalid gpio number
#define SL_SH_SENSOR_IMPLEMENTATION_NOT_FOUND \
  (13 << SL_SH_SENSORHUB_ERRORS_MASK) ///< No hal implementation found for given sensor type
#define SL_SH_INTERRUPT_TYPE_CONFIG_FAIL (14 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Interrupt type configuration failed
#define SL_SH_MEMORY_ALLOCATION_FAILED   (15 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Allocating memory for sensor hal failed
#define SL_SH_SENSOR_INDEX_NOT_FOUND     (0xFF)                              ///< Sensor not created

/****************************SENSOR CONFIGURATION ERRORS******************************/

#define SL_SH_CONFIG_NOT_FOUND (16 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Sensor configuration not found
#define SL_SH_INVALID_ADDRESS  (17 << SL_SH_SENSORHUB_ERRORS_MASK) ///< No sensor found at given address
#define SL_SH_WRONG_INTERRUPT_TYPE \
  (18 << SL_SH_SENSORHUB_ERRORS_MASK)                          ///< Invalid interrupt type is given for the sensor
#define SL_SH_INVALID_MODE (19 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Invalid mode is given for the sensor
#define SL_SH_INVALID_DELIVERY_MODE \
  (20 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Invalid delivery mode is given for the sensor

/****************************HAL ERRORS******************************/

#define SL_SH_HAL_SENSOR_CREATION_FAILED (21 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Create sensor in HAL failed
#define SL_SH_HAL_SENSOR_DELETION_FAILED (22 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Delete sensor in HAL failed
#define SL_SH_HAL_SENSOR_SAMPLE_FAIL     (23 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Sample sensor in HAL failed
#define SL_SH_HAL_SENSOR_CONTROL_FAIL    (24 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Control sensor in HAL failed

/******************************Blocking Errors********************************/

#define SL_SH_POWER_TASK_CREATION_FAILED  (25 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Power task creation failed
#define SL_SH_SENSOR_TASK_CREATION_FAILED (26 << SL_SH_SENSORHUB_ERRORS_MASK) ///< Sensor task creation failed
#define SL_SH_EM_TASK_CREATION_FAILED     (27 << SL_SH_SENSORHUB_ERRORS_MASK) ///< EM task creation failed
#define SL_ALL_PERIPHERALS_INIT_FAILED \
  (28 << SL_SH_SENSORHUB_ERRORS_MASK) ///< All(i2c,spi,adc) peripheral's initialization failed
/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
#define SL_DRIVER_OK ARM_DRIVER_OK ///< Operation succeeded
/** @endcond */

/// @} end group SENSOR-HUB ********************************************************/

#endif
