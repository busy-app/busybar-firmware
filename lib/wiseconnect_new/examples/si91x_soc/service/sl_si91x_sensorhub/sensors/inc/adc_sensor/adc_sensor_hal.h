/***************************************************************************/ /**
 * @file adc_sensor_hal.h
 * @brief adc sensor hal driver
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

#ifndef ADC_SENSOR_HAL_H_
#define ADC_SENSOR_HAL_H_
#include "sensor_type.h"
#include "sl_si91x_adc.h"

/***************************************************************************/ /**
 * Typedef for different pointer handles
 ******************************************************************************/

typedef void *sl_sensor_adc_handle_t;

typedef int32_t sl_adc_error_t;

/* Macros for sensor hub */
#define SL_SH_ADC_CHANNEL0
//#define SL_SH_ADC_CHANNEL1
//#define SL_SH_ADC_CHANNEL2
//#define SL_SH_ADC_CHANNEL3

#ifdef SL_SH_ADC_CHANNEL0
#include "adc_sensor_hal.h"
#endif

/* ADC peripheral configurations */
#define SL_SH_ADC_SAMPLING_RATE       1000 ///< ADC sampling rate, 10sps, 20 milliseconds per sample */
#define SL_SH_ADC_NUM_CHANNELS_ENABLE 1    ///< ADC number of channels enabled */

/* ADC channel 0 configurations */
#ifdef SL_SH_ADC_CHANNEL0
#define SL_SH_ADC_CH0_P_INPUT     4          ///< ADC channel 0 positive input selection ULP_GPIO_2 */
#define SL_SH_ADC_CH0_CHANNEL     0          ///< ADC channel 0 channel number */
#define SL_SH_ADC_CH0_OPAMP_GAIN  2          ///< ADC channel 0 opamp gain */
#define SL_SH_ADC_CH0_NUM_SAMPLES 2          ///< ADC channel 0 number of samples */
#define ADC_PING_BUFFER0          0x24060000 ///< ADC ping buffer starting address for channel 0.
#endif

/* ADC channel 1 configurations */
#ifdef SL_SH_ADC_CHANNEL1
#define SL_SH_ADC_CH1_P_INPUT     5          ///< ADC channel 0 positive input selection ULP_GPIO_8 */
#define SL_SH_ADC_CH1_CHANNEL     1          ///< ADC channel 0 channel number */
#define SL_SH_ADC_CH1_OPAMP_GAIN  2          ///< ADC channel 0 opamp gain */
#define SL_SH_ADC_CH1_NUM_SAMPLES 2          ///< ADC channel 0 number of samples */
#define ADC_PING_BUFFER1          0x24060800 ///< ADC ping buffer starting address for channel 1.
#endif

/* ADC channel 2 configurations */
#ifdef SL_SH_ADC_CHANNEL2
#define SL_SH_ADC_CH2_P_INPUT     10         ///< ADC channel 0 positive input selection ULP_GPIO_10 */
#define SL_SH_ADC_CH2_CHANNEL     2          ///< ADC channel 0 channel number */
#define SL_SH_ADC_CH2_OPAMP_GAIN  2          ///< ADC channel 0 opamp gain */
#define SL_SH_ADC_CH2_NUM_SAMPLES 2          ///< ADC channel 0 number of samples */
#define ADC_PING_BUFFER2          0x24061000 ///< ADC ping buffer starting address for channel 2.
#endif

/* ADC channel 3 configurations */
#ifdef SL_SH_ADC_CHANNEL3
#define SL_SH_ADC_CH3_P_INPUT     10         ///< ADC channel 0 positive input selection ULP_GPIO_1 */
#define SL_SH_ADC_CH3_CHANNEL     3          ///< ADC channel 0 channel number */
#define SL_SH_ADC_CH3_OPAMP_GAIN  2          ///< ADC channel 0 opamp gain */
#define SL_SH_ADC_CH3_NUM_SAMPLES 2          ///< ADC channel 0 number of samples */
#define ADC_PING_BUFFER3          0x24061800 ///< ADC ping buffer starting address for channel 2.
#endif

#define SL_SH_ADC_MAX_OP_VALUE 4095 ///< Maximum voltage output value from adc */
#define SL_SH_ADC_VREF_VALUE   3.3f ///< Reference voltage */

#define SL_SH_ADC_SENSOR0_NUM_OF_SAMPLES 1 ///< Number of samples for ADC sensor0
#define SL_SH_ADC_SENSOR1_NUM_OF_SAMPLES 1 ///< Number of samples for ADC sensor1
#define SL_SH_ADC_SENSOR2_NUM_OF_SAMPLES 1 ///< Number of samples for ADC sensor2

/* ADC channel numbers used for sensors */
#define JS_ADC_CHANNEL   0
#define GUVA_ADC_CHANNEL 0
#ifdef GY61_ADC_SENSOR
#define GY61_X_AXIS_ADC_CHANNEL 0
#define GY61_Y_AXIS_ADC_CHANNEL 1
#define GY61_Z_AXIS_ADC_CHANNEL 2
#define GY61_ADC_SENSOR_CHANNELS \
  (BIT(GY61_X_AXIS_ADC_CHANNEL) | BIT(GY61_Y_AXIS_ADC_CHANNEL) | BIT(GY61_Z_AXIS_ADC_CHANNEL))

/* ADXL335 GY61 sensor g conversion values*/
#define GY61_X_RAW_MIN   3005
#define GY61_X_RAW_MAX   3390
#define GY61_Y_RAW_MIN   3005
#define GY61_Y_RAW_MAX   3380
#define GY61_Z_RAW_MIN   1750
#define GY61_Z_RAW_MAX   2550
#define GY61_G_SCALE_MIN -100
#define GY61_G_SCALE_MAX 100
#endif

#define IS_MULTI_CHANNEL(X)  ((X & (X - 1)) != 0)
#define IS_SINGLE_CHANNEL(X) ((X & (X - 1)) == 0)

/* ADC sensor implementations for sensors */
typedef struct {
  sl_status_t (*channel_init)(sl_adc_channel_config_t *, sl_adc_config_t *);
  sl_status_t (*channel_enable)(uint8_t);
  sl_status_t (*channel_disable)(uint8_t);
  sl_status_t (*deinit)(sl_adc_config_t *);
  sl_status_t (*sample_adc_static)(sl_adc_channel_config_t *, sl_adc_config_t *, uint16_t *);
  sl_status_t (*channel_sample)(sl_adc_channel_config_t *, uint8_t);
  sl_status_t (*sleep)(void);
  sl_status_t (*wakeup)(void);
} adc_sensor_impl_t;

/* ADC sensor hal configurations */
typedef struct {
  uint8_t channel;
  sl_sensor_bus_t bus;
  bool is_init;
  const adc_sensor_impl_t *impl;
} sensor_adc_t;

/* ADC sensor sample configuration*/
typedef struct {
  uint8_t channel;
  int32_t (*sample)(uint8_t, bool, uint16_t *);
  int32_t (*sleep)();
  int32_t (*wakeup)();
  bool is_init;
} sl_adc_sensor_data_t;

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Prototypes

/***************************************************************************/ /**
 * @fn        sl_sensor_adc_handle_t sl_si91x_adc_sensor_create(sl_adc_bus_handle_t bus, int channel)
 * @brief     ADC sensor create
 *
 * @param[in] bus : bus type (ADC)
 * @param[in] id : respective channel number
 * @return adc sensor handle
 ******************************************************************************/
sl_sensor_adc_handle_t sl_si91x_adc_sensor_create(sl_sensor_bus_t bus, int id);

/***************************************************************************/ /**
 * @fn        sl_status_t sl_si91x_adc_sensor_delete(sl_sensor_adc_handle_t *sensor)
 * @brief     ADC sensor delete
 *
 * @param[in] sensor : ADC sensor handle
 * @return status 0 if successful, else error code
 *         \ref SL_STATUS_OK  - Success
 *         \ref SL_STATUS_NULL_POINTER  - The parameter is null pointer
 *         \ref SL_STATUS_INVALID_PARAMETER  - Parameters are invalid
 *         \ref SL_STATUS_FAIL  - The function is failed
 *         \ref SL_STATUS_NOT_INITIALIZED  - Clock is not initialized
 ******************************************************************************/
sl_adc_error_t sl_si91x_adc_sensor_delete(sl_sensor_adc_handle_t *sensor);

/***************************************************************************/ /**
 * @fn        sl_status_t sl_si91x_adc_sensor_sample_static(sl_sensor_adc_handle_t *sensor, uint16_t *adc_value)
 * @brief     ADC sensor read static data
 *
 * @param[in] sensor : NONE
 * @return status 0 if successful, else error code
 *         \ref SL_STATUS_OK  - Success
 *         \ref SL_STATUS_NULL_POINTER  - The parameter is null pointer
 *         \ref SL_STATUS_INVALID_PARAMETER  - Parameters are invalid
 *         \ref SL_STATUS_FAIL  - The function is failed
 *         \ref SL_STATUS_NOT_INITIALIZED  - Clock is not initialized
 ******************************************************************************/
sl_status_t sl_si91x_adc_sensor_sample_static();

/*******************************************************************************
 * @fn        sl_status_t sl_si91x_sensor_sample(uint8_t channel)
 * @brief     ADC read the sample
 *            This function will read sample data from a particular sample
 *
 * @param[in] sensor: ADC channel number from which to sample
 *
 * @return    respective sl error code (SL_STATUS_OK, SL_STATUS_FAIL, ETC)
*******************************************************************************/
sl_adc_error_t sl_si91x_sensor_channel_sample(uint8_t channel);

/***************************************************************************/ /**
 * @fn        sl_status_t sl_si91x_adc_sensor_sleep(sl_sensor_adc_handle_t sensor)
 * @brief     ADC sensor sleep
 *
 * @param[in] sensor : ADC sensor handle
 * @return status 0 if successful, else error code
 *         \ref SL_STATUS_OK  - Success
 *         \ref SL_STATUS_NULL_POINTER  - The parameter is null pointer
 *         \ref SL_STATUS_INVALID_PARAMETER  - Parameters are invalid
 *         \ref SL_STATUS_FAIL  - The function is failed
 *         \ref SL_STATUS_NOT_INITIALIZED  - Clock is not initialized
 ******************************************************************************/
sl_status_t sl_si91x_adc_sensor_sleep(sl_sensor_adc_handle_t sensor);

/***************************************************************************/ /**
 * @fn        sl_status_t sl_si91x_adc_sensor_wakeup(sl_sensor_adc_handle_t sensor)
 * @brief     ADC sensor wakeup
 *
 * @param[in] sensor : ADC sensor handle
 * @return status 0 if successful, else error code
 *         \ref SL_STATUS_OK  - Success
 *         \ref SL_STATUS_NULL_POINTER  - The parameter is null pointer
 *         \ref SL_STATUS_INVALID_PARAMETER  - Parameters are invalid
 *         \ref SL_STATUS_FAIL  - The function is failed
 *         \ref SL_STATUS_NOT_INITIALIZED  - Clock is not initialized
 ******************************************************************************/
sl_status_t sl_si91x_adc_sensor_wakeup(sl_sensor_adc_handle_t sensor);

/***************************************************************************/ /**
 * @fn        sl_status_t sl_si91x_adc_sensor_set_power(sl_sensor_adc_handle_t sensor, sl_sensor_power_mode_t power_mode)
 * @brief     ADC sensor set power mode
 *
 * @param[in] sensor : ADC sensor handle
 * @param[in] power_mode : set ADC sensor to requested power mode
 * @return status 0 if successful, else error code
 *         \ref SL_STATUS_OK  - Success
 *         \ref SL_STATUS_NULL_POINTER  - The parameter is null pointer
 *         \ref SL_STATUS_INVALID_PARAMETER  - Parameters are invalid
 *         \ref SL_STATUS_FAIL  - The function is failed
 *         \ref SL_STATUS_NOT_INITIALIZED  - Clock is not initialized
 ******************************************************************************/
sl_status_t sl_si91x_adc_sensor_set_power(sl_sensor_adc_handle_t sensor, sl_sensor_power_mode_t power_mode);

/***************************************************************************/ /**
 * @fn        sl_status_t sl_si91x_adc_sensor_sample(sl_sensor_adc_handle_t *sensor, sl_sensor_data_group_t *data_group)
 * @brief     ADC sensor sample
 *
 * @param[in] sensor : ADC sensor handle
 * @param[in] data_group : sensor hub data group structure that contains
 *            memory to store ADC data and number of samples
 * @return status 0 if successful, else error code
 *         \ref SL_STATUS_OK  - Success
 *         \ref SL_STATUS_NULL_POINTER  - The parameter is null pointer
 *         \ref SL_STATUS_INVALID_PARAMETER  - Parameters are invalid
 *         \ref SL_STATUS_FAIL  - The function is failed
 *         \ref SL_STATUS_NOT_INITIALIZED  - Clock is not initialized
 ******************************************************************************/
sl_adc_error_t sl_si91x_adc_sensor_sample(sl_sensor_adc_handle_t sensor, sl_sensor_data_group_t *data_group);

/***************************************************************************/ /**
 * @fn        sl_adc_error_t sl_si91x_sdc_sensor_sample(sl_sensor_adc_handle_t sensor, sl_sensor_data_group_t *data_group);
 * @brief     SDC sensor sample
 *
 * @param[in] sensor : SDC sensor handle
 * @param[in] data_group : sensor hub data group structure that contains
 *            memory to store ADC data and number of samples
 * @return status 0 if successful, else error code
 *         \ref SL_STATUS_OK  - Success
 *         \ref SL_STATUS_NULL_POINTER  - The parameter is null pointer
 *         \ref SL_STATUS_INVALID_PARAMETER  - Parameters are invalid
 *         \ref SL_STATUS_FAIL  - The function is failed
 *         \ref SL_STATUS_NOT_INITIALIZED  - Clock is not initialized
 ******************************************************************************/
sl_adc_error_t sl_si91x_sdc_sensor_sample(sl_sensor_adc_handle_t sensor, sl_sensor_data_group_t *data_group);

/***************************************************************************/ /**
 * @fn        sl_status_t sl_si91x_adc_sensor_control(sl_sensor_adc_handle_t sensor, sl_sensor_command_t cmd, UNUSED_PARAM void *args)
 * @brief     ADC sensor control
 *
 * @param[in] sensor : ADC sensor handle
 * @param[in] cmd : command to change the sensor modes
 * @return status 0 if successful, else error code
 *         \ref SL_STATUS_OK  - Success
 *         \ref SL_STATUS_NULL_POINTER  - The parameter is null pointer
 *         \ref SL_STATUS_INVALID_PARAMETER  - Parameters are invalid
 *         \ref SL_STATUS_FAIL  - The function is failed
 *         \ref SL_STATUS_NOT_INITIALIZED  - Clock is not initialized
 ******************************************************************************/
sl_adc_error_t sl_si91x_adc_sensor_control(sl_sensor_adc_handle_t sensor, sl_sensor_command_t cmd, void *args);

#ifdef __cplusplus
extern "C"
}
#endif

#endif /* ADC_SENSOR_HAL_H_ */
