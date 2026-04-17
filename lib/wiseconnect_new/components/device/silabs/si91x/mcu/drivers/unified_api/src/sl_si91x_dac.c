/******************************************************************************
* @file sl_si91x_dac.c
* @brief DAC API implementation
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
#include "sl_si91x_dac.h"
#include "clock_update.h"
#include "aux_reference_volt_config.h"
#include "rsi_bod.h"
#include "rsi_dac.h"
#include "rsi_pll.h"
#include "rsi_rom_table_si91x.h"
#include "rsi_rom_clks.h"
#include "rsi_rom_ulpss_clk.h"
#include "rsi_opamp.h"
/*******************************************************************************
 ***************************  DEFINES / MACROS   *******************************
 ******************************************************************************/
#define ICACHE2_ADDR_TRANSLATE_1_REG \
  *(volatile uint32_t *)(0x20280000 + 0x24) // Address Translate Value Segment Register_1
#define MISC_CFG_SRAM_REDUNDANCY_CTRL *(volatile uint32_t *)(0x46008000 + 0x18) // AHB bridge Control Register.
#define MISC_CONFIG_MISC_CTRL1        *(volatile uint32_t *)(0x46008000 + 0x44) // MCU Generic Control Register1
#define SOC_PREFETCH_LIMIT            120000000                                 // Limit for soc pll clock.
#define ODD_DIV_FACTOR                0                                         // Odd division factor
#define SWALLO_ENABLE                 0                                         // Enable/Disable swallo.
#define EVEN_VALUE                    2                                         // clock divider to set even value.
#define DAC_MAX_SAMPLE_RATE           5000000                                   // Maximum sampling rate 5 Msps.
#define DAC_MINI_SAMPLE_RATE          63                                        // Minimum sampling rate 16sps.
#define INVALID_POWER_STATE           2                                         // Invalid power state.
#define DAC_MAX_THRESHOLD             7                                         // Maximum threshold value
#define MINIMUM_REF_VOLT              1.8f                                      // Maximum reference voltage
#define MAXIMUM_REF_VOLT              3.6f                                      // Maximum reference voltage
#define CHANNEL_BITMAP_28             1                                         // Enable DAC to dynamic mode.
#define MINIMUM_DATA_LEN              1                                         // Minimum data length.
#define MAXIMUM_DATA_LEN              1024                                      // Maximum data length.
#define STATIC_MAX_LEN                1 // DAC static mode maximum sample length.
#define DAC_RELEASE_VERSION           0 // DAC Release version
#define DAC_SQA_VERSION               0 // DAC SQA version
#define DAC_DEV_VERSION               1 // DAC Developer version
#if defined(SLI_SI917B0)
//DAC1 with OPAMP1
#define OPAMP_OUT_MUX_SEL      1 /* set 1 to send OpAmp output at GPIO_30 */
#define OPAMP_DYN_MODE_EN      0 /*OPAMP Dynamic mode*/
#define CHANNEL_NO             0 /*adc channel no in dynamic mode*/
#define OPAMP_LP_MODE          0 /*OPAMP low power enable*/
#define DAC_OUTPUT_INPUT_OPAMP 6 /*DAC output to OPAMP input*/
#define OUTPUT_MUX_ENABLE      1 /* Enabled the MUX before sending OpAmp output to GPIO_30 */
#endif

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static sl_dac_callback_t user_callback = NULL;
static sl_dac_operation_mode_t dac_operation_mode;
static sl_channel_id_for_adc_t adc_channel;
static boolean_t static_flag = false;
/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
static sl_status_t convert_rsi_to_sl_error_code(rsi_error_t error);
static sl_status_t validate_dac_configuration_parameters(sl_dac_config_t dac_config);
static void dac_callback_event_handler(uint8_t event);
/*******************************************************************************
 * To get the driver version
 *
 * @param[in] None
 * @return returns driver version
*******************************************************************************/
sl_dac_version_t sl_si91x_dac_get_version(void)
{
  sl_dac_version_t sl_dac_version;
  sl_dac_version.release = DAC_RELEASE_VERSION;
  sl_dac_version.major   = DAC_SQA_VERSION;
  sl_dac_version.minor   = DAC_DEV_VERSION;
  return sl_dac_version;
}

/*******************************************************************************
 * Initialization of DAC
 * This API will power-up the DAC peripheral.
 * If SOC PLL clock above prefetch limit it will set SOC PLL to 220MHZ,
 * set the soc pll reference clock as 40MHZ, switch the M4 to PLL clock for
 * speed operation, and set the ULP processor reference clock as 90MHZ.
 * RSI errors are converted to the SL errors via convert_rsi_to_sl_error_code
 * function.
 ******************************************************************************/
sl_status_t sl_si91x_dac_init(sl_dac_clock_config_t *dac_clock)
{
  sl_status_t status;
  rsi_error_t error_status;
  (void)error_status;
  do {
    // Validate dac_clock NULL or not
    if (dac_clock == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }

#ifdef SLI_SI91X_MCU_ENABLE_RAM_BASED_EXECUTION
    /* Power-up Button Calibration*/
    RSI_PS_BodPwrGateButtonCalibEnable();
    /* Enable PTAT for Analog Peripherals*/
    RSI_PS_AnalogPeriPtatEnable();
    /* Power-up Analog(IPMU) Domain peripherals*/
    RSI_IPMU_PowerGateSet(AUXDAC_PG_ENB | CMP_NPSS_PG_ENB);
    /*Turn on ULPSS SRAM Core/Periphery domains*/
    RSI_PS_UlpssRamBanksPowerUp(ULPSS_2K_BANK_2 | ULPSS_2K_BANK_3);
    RSI_PS_UlpssRamBanksPeriPowerUp(ULPSS_2K_BANK_2 | ULPSS_2K_BANK_3);
#endif
    // DAC power gate enable
    RSI_DAC_PowerControl(DAC_POWER_ON);
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/**************************************************************************************************
 * Configuration DAC control parameters.
 * The configurations are below:
 *  - DAC operating mode (Static/FIFO/Reference voltage for ADC),
 *  - Sample Rate based on this value dac sample frequency will set. it range from 16 sps to 5 Msps.
 *  - DAC FIFO threshold ranging from 0 to 7.
 * RSI errors are converted to the SL errors via convert_rsi_to_sl_error_code function.
 **************************************************************************************************/
sl_status_t sl_si91x_dac_set_configuration(sl_dac_config_t dac_config, float vref_value)
{
  sl_status_t status;
  rsi_error_t error_status;
  float battery_status;
  float max_ip_volt_scdc = (float)2.4;
  do {
    //Validate reference voltage it should range from 1.8v to 3.6v
    if ((vref_value > (float)MAXIMUM_REF_VOLT) || (vref_value < (float)MINIMUM_REF_VOLT)) {
      status = SL_STATUS_INVALID_RANGE;
      break;
    }
    //Validate DAC parameters.
    status = validate_dac_configuration_parameters(dac_config);
    if (status != SL_STATUS_OK) {
      break;
    }
    battery_status = RSI_BOD_SoftTriggerGetBatteryStatus();
    // If input to chip less than 2.4V then switch input voltage supply from SCDC to HPLDO
    if (battery_status < max_ip_volt_scdc) {
      RSI_IPMU_ProgramConfigData(hp_ldo_voltsel);
    }
    // Configure the DAC peripheral for Static/FIFO mode.
    DAC_Init(dac_config.operating_mode, dac_config.dac_sample_rate, dac_callback_event_handler);
    DAC_PinMux_config(dac_config.dac_pin, dac_config.dac_port);
    if (dac_config.operating_mode != SL_DAC_OUTPUT_REF_VOLTAGE_FOR_ADC) {
#if defined(SLI_SI917B0)
      // Setting up the DAC output to be an input for OPAMP1.
      RSI_OPAMP1_UGB(DAC_OUTPUT_INPUT_OPAMP,
                     ENABLE,
                     OPAMP_LP_MODE,
                     OUTPUT_MUX_ENABLE,
                     OPAMP_OUT_MUX_SEL,
                     OPAMP_DYN_MODE_EN,
                     CHANNEL_NO);
#endif
    } else { // Configure the DAC control parameter for ADC input.
      error_status = RSI_DAC_DynamicModeConfig(AUX_ADC_DAC_COMP, dac_config.adc_channel, DISABLE, DISABLE);
      status       = convert_rsi_to_sl_error_code(error_status);
      if (status != SL_STATUS_OK) {
        break;
      }
    }
    if (!(dac_config.operating_mode)) {
      //Set the DAC threshold value
      error_status = RSI_DAC_SetFifoThreshold(AUX_ADC_DAC_COMP, dac_config.dac_fifo_threshold);
      status       = convert_rsi_to_sl_error_code(error_status);
      if (status != SL_STATUS_OK) {
        break;
      }
    }
    // These parameters are taken from UC for configuring operating mode and channel.
    dac_operation_mode = dac_config.operating_mode;
    adc_channel        = dac_config.adc_channel;
    /* Configure reference voltage for analog peripheral ,here till 2.8V generate by using
       AUX_LDO so more than 2.8V enable LDO bypass mode */
    error_status = RSI_AUX_RefVoltageConfig(vref_value, battery_status);
    status       = convert_rsi_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * Writing digital sample value in DAC input register.
 * The input buffer will be write in DAC input data register, length,
 * this value will be '1' when DAC using static mode for operation.
 * RSI errors are converted to the SL errors via convert_rsi_to_sl_error_code
 * function.
 ******************************************************************************/
sl_status_t sl_si91x_dac_write_data(int16_t *data, uint16_t length)
{
  sl_status_t status;
  rsi_error_t error_status;
  do {
    // Validate the data is NULL or not.
    if (data == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validate range of data length.
    if ((length < MINIMUM_DATA_LEN) || (length > MAXIMUM_DATA_LEN)) {
      status = SL_STATUS_INVALID_RANGE;
      break;
    }
    // Validate length for static mode, it should be 1.
    if ((dac_operation_mode == SL_DAC_STATIC_MODE) && (length > STATIC_MAX_LEN)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if ((dac_operation_mode == SL_DAC_STATIC_MODE) && (static_flag == true)) {
      error_status = RSI_DAC_InterruptUnMask(AUX_ADC_DAC_COMP, dac_operation_mode);
      status       = convert_rsi_to_sl_error_code(error_status);
      if (status != SL_STATUS_OK) {
        break;
      }
    }
    // Write data in DAC input register for Static and FIFO mode.
    if (dac_operation_mode == SL_DAC_STATIC_MODE) {
      error_status = DAC_WriteData(dac_operation_mode, data, length);
      status       = convert_rsi_to_sl_error_code(error_status);
    } else if (dac_operation_mode == SL_DAC_FIFO_MODE) {
      error_status = DAC_WriteData(dac_operation_mode, data, length);
      status       = convert_rsi_to_sl_error_code(error_status);
    } else {
      error_status = RSI_DAC_DynamicModeWriteData(AUX_ADC_DAC_COMP, adc_channel, (uint16_t *)data, length);
      status       = convert_rsi_to_sl_error_code(error_status);
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * Get the DAC sampling frequency.
 * Based on sampling rate DAC clock division factor will configure and return
 * the DAC sample clock.
 ******************************************************************************/
sl_status_t sl_si91x_dac_get_achieved_sample_clock(uint32_t sample_rate, uint32_t *sample_clock)
{
  sl_status_t status;
  uint32_t read_dac_sample_clock = 0;
  // Validate DAC sampling rate.
  if ((sample_rate > DAC_MAX_SAMPLE_RATE) || (sample_rate < DAC_MINI_SAMPLE_RATE)) {
    status = SL_STATUS_INVALID_RANGE;
  } else {
    read_dac_sample_clock = dac_set_clock(sample_rate);
    *sample_clock         = (uint32_t)read_dac_sample_clock;
    status                = SL_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
 * Read output data of DAC.
 * Read the output data of DAC in dynamic and static mode, it returns the last
 * data played on DAC.
 ******************************************************************************/
sl_status_t sl_si91x_dac_read_data(uint16_t *dac_output_data)
{
  uint16_t read_dac_data = 0;
  //Read output data of DAC in dynamic or static mode.
  if (dac_operation_mode != SL_DAC_OUTPUT_REF_VOLTAGE_FOR_ADC) {
    read_dac_data    = RSI_DAC_ReadData(AUX_ADC_DAC_COMP);
    *dac_output_data = (uint16_t)read_dac_data;
  } else {
    RSI_DAC_DynamicModeReadData(AUX_ADC_DAC_COMP, adc_channel, read_dac_data);
    *dac_output_data = (uint16_t)read_dac_data;
  }
  return SL_STATUS_OK;
}

/*******************************************************************************
 * Rewrite the input sample data in DAC FIFO operating mode.
 * This API used for reconfigure the udma ping or pong descriptor for
 * playing continuous digital word in DAC input buffer.
 * RSI errors are converted to the SL errors via convert_rsi_to_sl_error_code
 * function.
 ******************************************************************************/
sl_status_t sl_si91x_dac_rewrite_data(int16_t *data, uint16_t length)
{
  sl_status_t status = 0;
  rsi_error_t error_status;
  do {
    // Validate the data is NULL or not.
    if (data == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validate range of data length.
    if ((length < MINIMUM_DATA_LEN) || (length > MAXIMUM_DATA_LEN)) {
      status = SL_STATUS_INVALID_RANGE;
      break;
    }
    //Reconfigure the UDMA ping or pong decriptor.
    error_status = DAC_PingPongReconfig(data, length);
    status       = convert_rsi_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * To register the user event callback
 * It registers the callback, i.e., stores the callback function address
 * and pass to the variable that is called in Interrupt Handler.
 * If another callback is registered without unregistering previous callback then, it
 * returns an error code, so it is mandatory to unregister the callback before registering
 * another callback.
 * It will returns error if any callback is already registered.
 ******************************************************************************/
sl_status_t sl_si91x_dac_register_event_callback(sl_dac_callback_t callback_event)
{
  sl_status_t status;
  do {
    // Validate user callback, if the parameters is NULL, it returns an error code.
    if (callback_event == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // To validate the function pointer if the parameters is not NULL then, it
    // returns an error code
    if (user_callback != NULL) {
      status = SL_STATUS_BUSY;
      break;
    }
    // User callback address is passed to the static variable which is called
    // at the time of interrupt
    user_callback = callback_event;
    // Returns SL_STATUS_OK if callback is successfully registered
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * To unregister the user event callback
 * It unregisters the callback, i.e., clear the callback function address
 * and pass NULL value to the variable
 ******************************************************************************/
sl_status_t sl_si91x_dac_unregister_event_callback(void)
{
  // Pass the NULL value to the static variable which is called at the time of
  // interrupt.
  // It is further validated in register callback API.
  user_callback = NULL;
  return SL_STATUS_OK;
}

/*******************************************************************************
 * Start the DAC operation.
 * This API will activates the DAC path in Aux ADC-DAC controller.
 * Data samples will be played on  DAC only when DAC start.
 * Enable signal to DAC start the operating.
 * RSI errors are converted to the SL errors via convert_rsi_to_sl_error_code
 * function.
 ******************************************************************************/
sl_status_t sl_si91x_dac_start(void)
{
  sl_status_t status;
  rsi_error_t error_status;
  // Start the DAC operation for static and FIFO mode.
  if (dac_operation_mode != SL_DAC_OUTPUT_REF_VOLTAGE_FOR_ADC) {
    error_status = DAC_Start(dac_operation_mode);
    status       = convert_rsi_to_sl_error_code(error_status);
  } else { // Start the DAC operation for dynamic mode.
    error_status = RSI_DAC_DynamicModeStart(AUX_ADC_DAC_COMP, adc_channel, CHANNEL_BITMAP_28);
    status       = convert_rsi_to_sl_error_code(error_status);
  }
  return status;
}

/*******************************************************************************
 * Stop the DAC operation.
 * RSI errors are converted to the SL errors via convert_rsi_to_sl_error_code
 * function.
 ******************************************************************************/
sl_status_t sl_si91x_dac_stop(void)
{
  sl_status_t status;
  rsi_error_t error_status;
  //DAC operation stop.
  if (dac_operation_mode != SL_DAC_OUTPUT_REF_VOLTAGE_FOR_ADC) {
    error_status = DAC_Stop();
    status       = convert_rsi_to_sl_error_code(error_status);
  } else {
    error_status = RSI_DAC_DynamicModeStop(AUX_ADC_DAC_COMP, adc_channel);
    status       = convert_rsi_to_sl_error_code(error_status);
  }
  return status;
}

/*******************************************************************************
 * To De-initialize the DAC.
 * This API will power off the DAC power control and
 * stop the DAC operation mode.
 * RSI errors are converted to the SL errors via convert_rsi_to_sl_error_code
 * function.
 ******************************************************************************/
sl_status_t sl_si91x_dac_deinit(void)
{
  sl_status_t status;
  rsi_error_t error_status;
  error_status = DAC_Deinit();
  status       = convert_rsi_to_sl_error_code(error_status);
  // NULL the user callback function.
  user_callback = NULL;
  return status;
}

/*******************************************************************************
 * To validate the RSI error code
 * While calling the RSI APIs, it returns the RSI Error codes.
 * This function converts the RSI error codes into SL error codes.
 * It takes argument as RSI error type and returns the SL error type.
 * It has a single switch statement which is mapped with the SL error code and
 * after successful conversion it breaks the switch statement.
 * If the error code is not listed, by default is SL_STATUS_FAIL.
 ******************************************************************************/
static sl_status_t convert_rsi_to_sl_error_code(rsi_error_t error)
{
  sl_status_t status;
  switch (error) {
    case RSI_OK:
      status = SL_STATUS_OK;
      break;
    case INVALID_PARAMETERS:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ERROR_INVALID_INPUT_FREQUENCY:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ERROR_CLOCK_NOT_ENABLED:
      status = SL_STATUS_NOT_INITIALIZED;
      break;
    case INVALID_SAMPLING_RATE:
      status = SL_STATUS_INVALID_RANGE;
      break;
    case INVALID_SAMPLE_LENGTH:
      status = SL_STATUS_INVALID_COUNT;
      break;
    case ERROR_ADC_INVALID_ARG:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ERROR_PS_INVALID_STATE:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ERROR_UDMA_INVALID_ARG:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    default:
      status = SL_STATUS_FAIL;
      break;
  }
  return status;
}

/*******************************************************************************
 * To validate the parameters of DAC configuration
 * It validating the invalid parameters if any it will return error code as '33'
 * According to the values in DAC configuration structure, it performs the
 * 'OR' operation of the values.
 ******************************************************************************/
static sl_status_t validate_dac_configuration_parameters(sl_dac_config_t dac_config)
{
  sl_status_t status;
  do {
    // Validate DAC sampling rate.
    if ((dac_config.dac_sample_rate > DAC_MAX_SAMPLE_RATE) || (dac_config.dac_sample_rate < DAC_MINI_SAMPLE_RATE)) {
      status = SL_STATUS_INVALID_RANGE;
      break;
    }
    // Validate DAC operating mode it should be static/fifo/Reference voltage for ADC.
    if (dac_config.operating_mode >= SL_DAC_OPERATION_MODE_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validate DAC fifo threshold it will be maximum 7.
    if (dac_config.dac_fifo_threshold > DAC_MAX_THRESHOLD) {
      status = SL_STATUS_INVALID_RANGE;
      break;
    }
    // Returns SL_STATUS_OK if the parameter are appropriate
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * Static callback function for handling the callback events.
 * When DAC operation mode in STATIC the DAC_STATIC_MODE_CALLBACK event
 * will trigger and when in FIFO mode, DAC_UDMA_PING_PONG_CONFIG event
 * will trigger.
 ******************************************************************************/
static void dac_callback_event_handler(uint8_t event)
{
  switch (event) {
    case DAC_STATIC_MODE_CALLBACK:
      user_callback(SL_DAC_STATIC_MODE_EVENT);
      static_flag = true;
      break;
    case DAC_UDMA_PING_PONG_CONFIG:
      user_callback(SL_DAC_FIFO_MODE_EVENT);
      break;
  }
}
