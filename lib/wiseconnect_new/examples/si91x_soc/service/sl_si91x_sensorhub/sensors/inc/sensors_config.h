/***************************************************************************/ /**
* @file sensors_config
* @brief sensors_config example
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

#ifndef SENSORS_CONFIG_H_
#define SENSORS_CONFIG_H_

/*******************************************************************************
 ****************************** Sensor Defines  ********************************
 ******************************************************************************/
#define SL_CONFIG_SENSOR_LIGHT
#define SL_CONFIG_SENSOR_TEMPERATURE
#define SL_CONFIG_SENSOR_APDS9960
#define SL_CONFIG_SENSOR_ADXL345
#define SL_CONFIG_SENSOR_ADC

#define SL_SENSOR_ID_MASK            0XF0
#define SL_SENSOR_ID_OFFSET          4
#define SL_SENSOR_DATA_GROUP_MAX_NUM 6
#define SL_MAX_NUM_SENSORS           4 ///< Max number of sensors used in the sensor hub
#define SH_BUTTON_PIN                2
typedef enum { SL_SH_I2C = 1, SL_SH_SPI, SL_SH_UART, SL_SH_ADC, SL_SH_GPIO } sl_sensor_bus_t;
//Type of sensors
typedef enum {
  SL_NULL_ID,
  SL_HUMITURE_ID,
  SL_TEMPERATURE_SENSOR_ID,
  SL_IMU_ID,
  SL_LIGHT_SENSOR_ID,
  SL_GPIO_SENSOR_ID,
  SL_GESTURE_PROXIMITY_RGB_SENSOR_ID,
  SL_ACCELEROMETER_SENSOR_ID,
  SL_ADC_SENSOR_ID,
  SL_SENSOR_TYPE_MAX,
} sl_sensor_type_t;
/// @brief Enumeration for UULP gpio numbers
typedef enum { SL_SH_GPIO_0, SL_SH_GPIO_1, SL_SH_GPIO_2, SL_SH_GPIO_3 } sl_gpio_sensor_id_t;

typedef enum {
  SL_BH1750_ID = 0x01, /*!< BH1750 light sensor id*/
  SL_VEML6040_ID,      /*!< VEML6040 light sensor id*/
  SL_VEML6075_ID,      /*!< VEML6075 light sensor id*/
  SL_LIGHT_MAX_ID,     /*!< max light sensor id*/
} sl_light_sensor_id_t;

typedef enum {
  SL_LM75_ID = 0x01,     /*!< LM75 temperature sensor id*/
  SL_TEMPERATURE_MAX_ID, /*!< max temperature sensor id*/
} sl_temp_sensor_id_t;

//apds sensor ids
typedef enum {
  SL_APDS9960_ID = 0x01, /*!< APDS9960 sensor id*/
  SL_APDS9960_MAX_ID,    /*!< max temperature sensor id*/
} sl_apds9960_sensor_id_t;

typedef enum {
  SL_ADXL345_ID = 0x01,    /*!< ADXL345 accelerometer sensor id*/
  SL_ACCELEROMETER_MAX_ID, /*!< max accelerometer sensor id*/
} sl_accelerometer_sensor_id_t;

// ADC sensor ids
typedef enum {
  SL_ADC_JOYSTICK_ID = 0x00, /*!< adc joystick id*/
  SL_ADC_GUVA_S12D_ID,       /*!< adc uv sensor */
  SL_ADC_GY61_ID,            /*!< adc gy61 sensor */
  SL_ADC_MAX_ID,             /*!< max adc sensor id*/
} adc_sensor_id_t;

/// @brief Enumeration for Sensors ID'S
typedef enum {
#ifdef SL_CONFIG_SENSOR_LIGHT
  SL_SENSOR_BH1750_ID = (SL_LIGHT_SENSOR_ID << SL_SENSOR_ID_OFFSET) | SL_BH1750_ID, ///< Bh1750 sensor id
#endif
#ifdef SL_CONFIG_SENSOR_TEMPERATURE
  SL_SENSOR_LM75_ID = (SL_TEMPERATURE_SENSOR_ID << SL_SENSOR_ID_OFFSET) | SL_LM75_ID, ///< LM75 sensor id
#endif
#ifdef SL_CONFIG_SENSOR_APDS9960
  SL_SENSOR_APDS9960_ID = (SL_GESTURE_PROXIMITY_RGB_SENSOR_ID << SL_SENSOR_ID_OFFSET)
                          | SL_APDS9960_ID, ///< APDS9960 sensor id
#endif
#ifdef SL_CONFIG_SENSOR_ADXL345
  SL_SENSOR_ADXL345_ID = (SL_ACCELEROMETER_SENSOR_ID << SL_SENSOR_ID_OFFSET) | SL_ADXL345_ID, ///< ADXL345 sensor id
#endif
  SL_GPIO_SENSE_GPIO_ID   = (SL_GPIO_SENSOR_ID << SL_SENSOR_ID_OFFSET) | SL_SH_GPIO_1, ///< GPIOs id
  SL_GPIO_SENSE_BUTTON_ID = (SL_GPIO_SENSOR_ID << SL_SENSOR_ID_OFFSET) | SL_SH_GPIO_0, ///< Button id
#ifdef SL_CONFIG_SENSOR_ADC
  SL_SENSOR_ADC_JOYSTICK_ID  = (SL_ADC_SENSOR_ID << SL_SENSOR_ID_OFFSET) | SL_ADC_JOYSTICK_ID,  ///< ADC joystick id
  SL_SENSOR_ADC_GUVA_S12D_ID = (SL_ADC_SENSOR_ID << SL_SENSOR_ID_OFFSET) | SL_ADC_GUVA_S12D_ID, ///< ADC GUVA sensor id
  SL_SENSOR_ADC_GY_61_ID     = (SL_ADC_SENSOR_ID << SL_SENSOR_ID_OFFSET) | SL_ADC_GY61_ID,      ///< ADC GY61 sensor id
#endif

} sl_sensor_id_t;
#endif /* SENSORS_CONFIG_H_ */
