/***************************************************************************/ /**
 * @file sensor_type.h
 * @brief Sensor data
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

#ifndef SENSOR_TYPE_H_
#define SENSOR_TYPE_H_
#include <stdint.h>

/*******************************************************************************
 ***************** Sensor data Unions/Structure variables  *********************
 ******************************************************************************/
//Accelerometer sensor data
#ifndef SL_AXIS3_T
typedef union {
  struct {
    float x;
    float y;
    float z;
  };
  float axis[3];
} sl_axis3_t;
#define SL_AXIS3_T sl_axis3_t
#endif

//Accelerometer sensor axis data
#ifndef SL_AXIS_T
typedef struct {
  float x;
  float y;
  float z;
} sl_axis_t;
#define SL_AXIS_T sl_axis_t
#endif

//Sensor RGBW data
#ifndef SL_RGBW_T
typedef struct {
  float r;
  float g;
  float b;
  float w;
  float proximity;
  char gesture;
} sl_rgbw_t;
#define SL_RGBW_T sl_rgbw_t
#endif

//Sensor UV data
#ifndef SL_UV_T
typedef struct {
  float uv;
  float uva;
  float uvb;
} sl_uv_t;
#define SL_UV_T sl_uv_t
#endif

//Sensor ADXL335 GY61 analog data
#ifndef SL_ADC_GY61_T
typedef struct {
  uint16_t *x;
  uint16_t *y;
  uint16_t *z;
} sl_adc_gy61_t;
#define SL_ADC_GY61_T sl_adc_gy61_t
#endif

//Sensor operations
typedef enum {
  SL_COMMAND_SET_MODE,
  SL_COMMAND_SET_RANGE,
  SL_COMMAND_SET_ODR,
  SL_COMMAND_SET_POWER,
  SL_COMMAND_SELF_TEST,
  SL_COMMAND_MAX
} sl_sensor_command_t;

//Sensor operations mode
typedef enum { SL_SENSOR_POWER_MODE_WAKEUP, SL_SENSOR_POWER_MODE_SLEEP, SL_SENSOR_POWER_MAX } sl_sensor_power_mode_t;

//Sensor operations range
typedef enum {
  SL_RANGE_DEFAULT,
  SL_RANGE_MIN,
  SL_RANGE_MEDIUM,
  SL_RANGE_MAX,
} sl_sensor_range_t;

//Sensor data group
typedef struct {
  uint8_t min_delay; ///<  minimum delay between two events, unit: ms
  union {
    SL_RGBW_T rgbw;          ///< Color               unit: lux
    SL_AXIS_T accelerometer; ///< Accelerometer       unit: G
    float temperature;       ///< Temperature         unit: dCelsius
    float light;             ///< Light               unit: lux
    SL_UV_T uv;              ///< ultraviole          unit: lux
    SL_ADC_GY61_T gy61;      ///< ADC GY61            uint: gy61
    uint16_t *adc;           ///< ADC output          unit: mV

#ifdef SH_SDC_ENABLE
    int16_t sh_sdc_data[16];
#endif
  };
} sl_sensor_data_t;

//Sensor data group structure
typedef struct {
  uint8_t number;                ///< effective data number
  sl_sensor_data_t *sensor_data; ///< data buffer
} sl_sensor_data_group_t;

#endif
