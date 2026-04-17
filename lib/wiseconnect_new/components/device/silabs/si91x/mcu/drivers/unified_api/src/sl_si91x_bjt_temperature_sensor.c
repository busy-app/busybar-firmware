/******************************************************************************
* @file sl_si91x_bjt_temperature_sensor.c
* @brief BJT temperature sensor API implementation
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
#include "sl_si91x_bjt_temperature_sensor.h"

#include "rsi_bod.h"
#include "rsi_opamp.h"
#include "rsi_ipmu.h"

#ifdef SL_SI91X_ACX_MODULE
#include "rsi_rom_egpio.h"
#endif
#include "rsi_rom_ulpss_clk.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define CHANNEL_SAMPLE_LENGTH         1       // Number of ADC sample collect for operation
#define VREF_VALUE                    2.61f   // default reference voltage
#define BOD_TEST_SEL_VALUE            3       // BOD test selection value
#define LOAD_BJT_TEMP                 25      // load bjt temperature
#define SAMPLING_RATE                 9000    // sampling rate value for adc
#define CHANNEL_NUMBER                0       // channel number for adc
#define POS_IP_OPAMP                  20      // Positive input to ADC using OPAMP
#define POS_IP_BJT                    23      // positive input to ADC using bjt temperature sensor
#define OPAMP_GAIN_OPAMP              51      // OPAMP gain value if opamp output is one of input to ADC
#define BJT_DATA_CLEAR                0xF7FF  // Clear the data if 12th bit is enabled
#define BJT_SIGN_BIT                  BIT(11) // Sign/magnitude bit
#define OPAMP_CHANNEL                 0       // opamp channel initializing with 0
#define NUMBER_OF_CHANNELS            1       // configuring the number of channels with 1
#define MINIMUM_NUMBER_OF_CHANNELS    1       // Minimum number of channel enable
#define MAXIMUM_NUMBER_OF_CHANNELS    16      // Maximum number of channel enable
#define MAX_SAMPLING_RATE             2500000 // Maximum sampling rate 2.5 Msps.
#define MINIMUM_DEGREE_TEMP_VALUE     -40     // minimum degree celsius bjt temperature sensor value
#define MAXIMUM_DEGREE_TEMP_VALUE     125     // maximum bjt degree celsius temperature sensor value
#define MINIMUM_FAHRENHEIT_TEMP_VALUE -40     // minimum fahrenheit bjt temperature sensor value
#define MAXIMUM_FAHRENHEIT_TEMP_VALUE 257     // maximum bjt fahrenheit temperature sensor value
#define MINIMUM_KELVIN_TEMP_VALUE     -233    // minimum kelvin bjt temperature sensor value
#define MAXIMUM_KELVIN_TEMP_VALUE     398     // maximum kelvin bjt temperature sensor value
#define READ_DATA_LIMIT               10      //  iteration limit to read the bjt sensor data

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static boolean_t chnl0_complete_flag               = false;
static float adc_output_bjt[CHANNEL_SAMPLE_LENGTH] = { 0 };
static float adc_output_bg[CHANNEL_SAMPLE_LENGTH]  = { 0 };

const adc_config_t sl_bjt_config            = { SL_ADC_STATIC_MODE, NUMBER_OF_CHANNELS };
const adc_ch_config_t sl_bjt_channel_config = { .input_type[0]     = SL_ADC_SINGLE_ENDED,
                                                .sampling_rate[0]  = SAMPLING_RATE,
                                                .num_of_samples[0] = CHANNEL_SAMPLE_LENGTH };

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
static sl_status_t sl_si91x_bjt_temperature_sensor_disable(void);
static void callback_event(uint8_t channel_no, uint8_t event);
static sl_status_t validate_bjt_parameters(const adc_config_t *sl_bjt_temperature_sensor_config);
static sl_status_t validate_bjt_channel_parameters(const adc_ch_config_t *sl_bjt_temperature_sensor_channel_config);
static const OPAMP_CONFIG_T UnityGB = { { 0,
#ifdef SL_SI91X_ACX_MODULE
                                          5,
#else
                                          1,
#endif
                                          4,
                                          0,
                                          1,
                                          0,
                                          0,
                                          0,
                                          0,
                                          1,
                                          0,
                                          1 },
                                        {
                                          0,
                                          2,
                                          3,
                                          0,
                                          1,
                                          0,
                                          0,
                                          0,
                                          0,
                                          1,
                                          1,
                                        },
                                        {
                                          0,
                                          0,
                                          0,
                                          0,
                                          0,
                                          0,
                                          0,
                                          0,
                                          0,
                                          0,
                                          0,
                                        } };

/*******************************************************************************
 * To validate the parameters BJT configuration
 ******************************************************************************/
static sl_status_t validate_bjt_parameters(const adc_config_t *sl_bjt_temperature_sensor_config)
{
  sl_status_t status = SL_STATUS_OK;

  if (sl_bjt_temperature_sensor_config == NULL) {
    status = SL_STATUS_NULL_POINTER;
  }
  // Validates the ADC operation mode, if it is not either static or fifo mode it returns error code.
  // If the number of channel is not in range i.e., between 1 and 16, returns the error code.
  else if ((sl_bjt_temperature_sensor_config->operation_mode != SL_ADC_STATIC_MODE)
           || (sl_bjt_temperature_sensor_config->num_of_channel_enable < MINIMUM_NUMBER_OF_CHANNELS)
           || (sl_bjt_temperature_sensor_config->num_of_channel_enable > MAXIMUM_NUMBER_OF_CHANNELS)) {
    status = SL_STATUS_INVALID_PARAMETER;
  }

  // Returns SL_STATUS_OK if the parameters are appropriate
  return status;
}

/*******************************************************************************
* To validate the parameters BJT channel configuration
 ******************************************************************************/
static sl_status_t validate_bjt_channel_parameters(const adc_ch_config_t *sl_bjt_temperature_sensor_channel_config)
{
  // Returns SL_STATUS_OK if the parameter are appropriate
  sl_status_t status = SL_STATUS_OK;

  if (sl_bjt_temperature_sensor_channel_config == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } // Validate input type
  else if (sl_bjt_temperature_sensor_channel_config->input_type[0] != SL_ADC_SINGLE_ENDED) {
    status = SL_STATUS_INVALID_PARAMETER;
  } // Verify the user given sampling rate is proper or not
  else if (sl_bjt_temperature_sensor_channel_config->sampling_rate[0] > MAX_SAMPLING_RATE) {
    status = SL_STATUS_INVALID_RANGE;
  } else if ((sl_bjt_temperature_sensor_channel_config->num_of_samples[0] < CHANNEL_SAMPLE_LENGTH)
             || (sl_bjt_temperature_sensor_channel_config->num_of_samples[0] > CHANNEL_SAMPLE_LENGTH)) {
    status = SL_STATUS_INVALID_COUNT;
  }

  return status;
}

/*******************************************************************************
 * BJT Temperature Sensor example initialization function
 ******************************************************************************/
sl_status_t sl_si91x_bjt_temperature_sensor_init(adc_ch_config_t sl_bjt_temperature_sensor_channel_config,
                                                 adc_config_t sl_bjt_temperature_sensor_config)
{
  sl_status_t status;
  // Validate BJT parameters, if the parameters incorrect
  // If the status is not equal to SL_STATUS_OK, returns error code.
  status = validate_bjt_parameters(&sl_bjt_temperature_sensor_config);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Validate BJT channel parameters, if the parameters incorrect
  // If the status is not equal to SL_STATUS_OK, returns error code.
  status = validate_bjt_channel_parameters(&sl_bjt_temperature_sensor_channel_config);
  if (status != SL_STATUS_OK) {
    return status;
  }
  //Initializing the adc to initialize the channel and operation mode
  status = sl_si91x_adc_init(sl_bjt_temperature_sensor_channel_config, sl_bjt_temperature_sensor_config, VREF_VALUE);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Configure ADC to initialize the channel and operation mode
  status =
    sl_si91x_adc_set_channel_configuration(sl_bjt_temperature_sensor_channel_config, sl_bjt_temperature_sensor_config);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Register user callback function
  status = sl_si91x_adc_register_event_callback(callback_event);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // starting the adc configuration.
  status = sl_si91x_adc_start(sl_bjt_temperature_sensor_config);
  return status;
}

/*******************************************************************************
 * Function will enable the bg and configure the OPAMP1
 * Here we configure Unity band gap values and which are later given input to ADC
 ******************************************************************************/
sl_status_t sl_si91x_bjt_temperature_sensor_bg_set_channel_configuration(
  adc_ch_config_t sl_bjt_temperature_sensor_channel_config)
{
  sl_status_t status;
  /* Configure ADC channel related parameter */
  sl_bjt_temperature_sensor_channel_config.pos_inp_sel[0] = POS_IP_OPAMP; /* Positive input to ADC using OPAMP1 */
  sl_bjt_temperature_sensor_channel_config.opamp_gain[0] =
    OPAMP_GAIN_OPAMP; /* OPAMP gain value if opamp output is one of input to ADC*/
  // Validate BJT channel parameters, if the parameters incorrect
  // If the status is not equal to SL_STATUS_OK, returns error code.
  status = validate_bjt_channel_parameters(&sl_bjt_temperature_sensor_channel_config);
  if (status == SL_STATUS_OK) {
    // Initialize opamp1 to give band gap input to opamp that gives it's output to ADC
    RSI_OPAMP1_Config(OPAMP, OPAMP_CHANNEL, &UnityGB);
    // Configure ADC to configure band gap the positive input selection and opamp gain
    status = sl_si91x_adc_set_channel_configuration(sl_bjt_temperature_sensor_channel_config, sl_bjt_config);
    if (status != SL_STATUS_OK) {
      status = SL_STATUS_FAIL;
    }
  }
  return status;
}

/*******************************************************************************
 * To read the BJT samples.
 * It will calculate the temperature value.
 ******************************************************************************/
sl_status_t sl_si91x_bjt_temperature_sensor_read_data(double *temp_data)
{
  sl_status_t status;
  uint8_t data_process_en_opamp = 1;
  uint8_t data_process_en_bjt   = 0;
  uint8_t read_data             = 0;
  int32_t Voffset               = 0;
  uint32_t i                    = 0;

#ifdef SL_SI91X_ACX_MODULE
  RSI_EGPIO_UlpPadDriverDisableState(9, ulp_Pullup);
#endif

  float adc_off = 0;
  float Vbg     = 0;
  if (temp_data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  //Initializing and configuring the band gap
  status = sl_si91x_bjt_temperature_sensor_bg_set_channel_configuration(sl_bjt_channel_config);
  if (status != SL_STATUS_OK) {
    return status;
  }
  if (chnl0_complete_flag) {
    chnl0_complete_flag = false;
    adc_output_bg[i] =
      RSI_ADC_ReadDataStatic(AUX_ADC_DAC_COMP, data_process_en_opamp, sl_bjt_channel_config.input_type[0]);
  }
  //Initializing and configuring the bjt temperature sensor
  status = sl_si91x_bjt_temperature_sensor_set_channel_configuration(sl_bjt_channel_config);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // We are just reading multiple times so that we will not get previous ADC values
  while (read_data < READ_DATA_LIMIT) {
    adc_output_bjt[i] =
      RSI_ADC_ReadDataStatic(AUX_ADC_DAC_COMP, data_process_en_bjt, sl_bjt_channel_config.input_type[0]);
    read_data++;
  }
  /* In two’s complement format, the MSb (11th bit) of the conversion result determines the polarity,
          when the MSb = ‘0’, the result is positive, and when the MSb = ‘1’, the result is negative*/
  if ((uint32_t)adc_output_bjt[i] & BIT(11)) {
    adc_output_bjt[i] = (float)((uint32_t)(adc_output_bjt[i]) & BJT_DATA_CLEAR);
  } else {
    adc_output_bjt[i] = (float)((uint32_t)adc_output_bjt[i] | BJT_SIGN_BIT);
  }
  //This API is used to program The offset value for AUX ADC single ended mode
  adc_off = (float)RSI_IPMU_Auxadcoff_SeEfuse();
  //This API is used to program the BG voltage for BJT temperature sensor
  Vbg = (float)RSI_IPMU_Vbg_Tsbjt_Efuse();
  // This API is used to program the offset voltage
  Voffset = (int32_t)(961 - RSI_IPMU_Delvbe_Tsbjt_Efuse());
  /* formula to calculate BJT temperature sensor it is the division of bjt temperature sensor *
     * output to the opamp2 output which is multipled with opamp voltage Vbg and offset voltage */
  *temp_data = -273
               + (310
                  * ((((adc_output_bjt[i] - adc_off) / (adc_output_bg[i] - adc_off)) * (Vbg / (float)1000))
                     + ((float)(Voffset) / (float)1000)));

  return status;
}

/*******************************************************************************
 * Function will enable the bjt register and configure the temperature sensor
 ******************************************************************************/
sl_status_t sl_si91x_bjt_temperature_sensor_set_channel_configuration(
  adc_ch_config_t sl_bjt_temperature_sensor_channel_config)
{
  sl_status_t status;
  // Validate BJT channel parameters, if the parameters incorrect
  // If the status is not equal to SL_STATUS_OK, returns error code.
  status = validate_bjt_channel_parameters(&sl_bjt_temperature_sensor_channel_config);
  if (status == SL_STATUS_OK) {
    status = sl_si91x_bjt_temperature_sensor_state(SL_BJT_TEMPERATURE_SENSOR_ENABLE);
    if (status == SL_STATUS_OK) {
      sl_bjt_temperature_sensor_channel_config.pos_inp_sel[0] =
        POS_IP_BJT; /* Positive input to ADC from temperature sensor */
      // Configure ADC channel to configure the BJT positive input selection
      status = sl_si91x_adc_set_channel_configuration(sl_bjt_temperature_sensor_channel_config, sl_bjt_config);
    }
  }
  return status;
}

/******************************************************************************
 * Used to enable the bjt temperature sensor
 *****************************************************************************/
sl_status_t sl_si91x_bjt_temperature_sensor_state(sl_bjt_temperature_sensor_state_t state)
{
  sl_status_t status;

  switch (state) {
    case SL_BJT_TEMPERATURE_SENSOR_DISABLE:
      // Disable the sensor based on sl_bjt_temperature_sensor_state_t selection
      status = sl_si91x_bjt_temperature_sensor_disable();
      break;
    case SL_BJT_TEMPERATURE_SENSOR_ENABLE:
      // Enable the sensor based on sl_bjt_temperature_sensor_state_t selection
      status = sl_si91x_adc_temperature_sensor_enable();
      break;
    default:
      // Returns invalid parameter if other than enabling (or) disabling is chosen
      status = SL_STATUS_INVALID_PARAMETER;
      break;
  }

  return status;
}

/******************************************************************************
 * Used to disable the bjt temperature sensor
 *****************************************************************************/
static sl_status_t sl_si91x_bjt_temperature_sensor_disable(void)
{
  //Disable the sensor based on sl_bjt_temperature_sensor_state_t selection
  AUX_ADC_DAC_COMP->TS_PTAT_ENABLE_b.TS_PTAT_EN = 0U;
  return SL_STATUS_OK;
}

/*******************************************************************************
 * To De-initialize the BJT Temperature Sensor.
 * This API will power off the BJT Temperature Sensor power control and
 * stop the BJT Temperature Sensor operation mode.
 * RSI errors are converted to the SL errors via convert_rsi_to_sl_error_code
 * function.
 ******************************************************************************/
sl_status_t sl_si91x_bjt_temperature_sensor_deinit(adc_config_t sl_bjt_temperature_sensor_config)
{
  sl_status_t status;

  status = validate_bjt_parameters(&sl_bjt_temperature_sensor_config);
  if (status != SL_STATUS_OK) {
    return status;
  }

  if (AUX_ADC_DAC_COMP->AUXADC_CTRL_1_b.ADC_ENABLE == ENABLE) {
    status = sl_si91x_adc_stop(sl_bjt_temperature_sensor_config);
    if (status != SL_STATUS_OK) {
      return status;
    }
  }

  status = sl_si91x_adc_deinit(sl_bjt_temperature_sensor_config);
  return status;
}

/******************************************************************************
 * Converts temperature from degrees Celsius to Fahrenheit and Kelvin.
 * This API takes a temperature measurement in degrees Celsius and converts it
 * to the temperature scale specified by current_temperature_mode parameter(Fahrenheit or Kelvin).
 *****************************************************************************/
sl_status_t sl_si91x_get_bjt_temperature_sensor_conversion(double *temp_data,
                                                           sl_bjt_temperature_sensor_enum_t current_temperature_mode)
{
  sl_status_t status;
  do {
    if (temp_data == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    switch (current_temperature_mode) {
      case SL_BJT_DEGREE_CELSIUS:
        //by default the *temp_data value is in degree celsius
        if (!((*temp_data >= MINIMUM_DEGREE_TEMP_VALUE) && (*temp_data <= MAXIMUM_DEGREE_TEMP_VALUE))) {
          status = SL_STATUS_FAIL;
          break;
        }
        status = SL_STATUS_OK;
        break;
      case SL_BJT_FAHRENHEIT:
        *temp_data = ((*temp_data * 9 / 5) + 32);
        if (!((*temp_data >= MINIMUM_FAHRENHEIT_TEMP_VALUE) && (*temp_data <= MAXIMUM_FAHRENHEIT_TEMP_VALUE))) {
          status = SL_STATUS_FAIL;
          break;
        }
        status = SL_STATUS_OK;
        break;
      case SL_BJT_KELVIN:
        *temp_data = (*temp_data + 273.15);
        if (!((*temp_data >= MINIMUM_KELVIN_TEMP_VALUE) && (*temp_data <= MAXIMUM_KELVIN_TEMP_VALUE))) {
          status = SL_STATUS_FAIL;
          break;
        }
        status = SL_STATUS_OK;
        break;
      default:
        status = SL_STATUS_INVALID_PARAMETER;
        break;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * Callback event function
 * It is responsible for the event which are triggered by ADC interface
 * @param  event       : ADC_STATIC_MODE_CALLBACK => Static mode adc data
 *                       acquisition done
 ******************************************************************************/
static void callback_event(uint8_t channel_no, uint8_t event)
{
  (void)&channel_no;
  if (event == SL_ADC_STATIC_MODE_EVENT) {
    chnl0_complete_flag = true;
  }
}
