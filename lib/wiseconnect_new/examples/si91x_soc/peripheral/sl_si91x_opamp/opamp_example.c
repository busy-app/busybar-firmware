/*******************************************************************************
* @file  opamp_example.c
* @brief OPAMP Example
*******************************************************************************
* # License
* <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
*******************************************************************************/

// Include necessary headers
#include "rsi_debug.h"
#include "sl_si91x_driver_gpio.h"
#include "opamp_example.h"
#include "sl_si91x_opamp_common_config.h"
#include "sl_si91x_opamp_init.h"
#include "sl_si91x_opamp.h"
#include "sl_si91x_bod.h"
#if INPUT_DAC_NEG_INPUT_EXTERNAL == ENABLE
#include "sl_si91x_dac.h"
#include "sl_si91x_dac_config.h"
#endif
/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define OPAMP_VIN_P_SEL   0  // Positive input pin selection
#define OPAMP_VREF_SEL    10 // Reference voltage selection
#define OPAMP_ENABLE      1  // Enable OPAMP
#define OPAMP_LP_MODE     0  // Low power mode disabled
#define OPAMP_R1_SEL      0  // R1 resistor selection
#define OPAMP_R2_SEL      8  // R2 resistor selection
#define OPAMP_OUT_MUX_EN  1  // Output mux enabled
#define OPAMP_OUT_MUX_SEL 1  // Output mux selection
// Macros for OPAMP negative input selection
#define OPAMP1_VINN_SEL 4 // For OPAMP1
#define OPAMP2_VINN_SEL 3 // For OPAMP2
#define OPAMP3_VINN_SEL 3 // For OPAMP3

// Enable or disable DAC output with external voltage from UC
#if INPUT_DAC_NEG_INPUT_EXTERNAL == ENABLE
#define VREF_VALUE 3.3f // Reference voltage
static float vref_value = (float)VREF_VALUE;
// Define DAC Macros
#define DAC_SAMPLING_RATE          5000000
#define NUMBER_OF_INPUT_SAMPLE     1
#define MAX_DAC_INPUT_SAMPLE_VALUE 1023
#define DAC_INPUT_SAMPLE_VALUE     1023
#endif

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/

/*******************************************************************************
 **********************  Local variables   *************************************
 ******************************************************************************/

#if INPUT_DAC_NEG_INPUT_EXTERNAL == ENABLE
static const int16_t dac_input_sample_data[1] = { 0x3FF };
static boolean_t dac_static_intr_flag         = false;
static boolean_t dac_fifo_intr_flag           = false;
static void dac_callback_event(uint8_t event);
#endif

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************
 * Initialize the OPAMP example
 * 
 * This function initializes the OPAMP peripheral, configures it based on the 
 * selected instance, and sets up the necessary GPIO pins for OPAMP operation.
 ******************************************************************************/
void opamp_example_init(void)
{
  sl_status_t status = SL_STATUS_OK; // Variable to store the status of OPAMP operations
  do {
#if INPUT_DAC_NEG_INPUT_EXTERNAL == ENABLE
    sl_dac_clock_config_t dac_clock_config;
    //Initializing DAC peripheral
    status = sl_si91x_dac_init(&dac_clock_config);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_dac_init: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("SL_DAC initialization is successful \n");

    // DAC configuration
    status = sl_si91x_dac_set_configuration(sl_dac_config, vref_value);
    /* Due to calling trim_efuse API on DAC configuration in driver it will change the clock frequency,
       if we are not initialize the debug again it will print the garbage data in console output. */
    DEBUGINIT();
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_dac_set_configuration: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("SL_DAC set configuration is successful \n");
    // Register user callback function
    status = sl_si91x_dac_register_event_callback(dac_callback_event);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_dac_register_event_callback: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("SL_DAC register event callback is successful \n");
    // DAC input sample data writing
    status = sl_si91x_dac_write_data((int16_t *)dac_input_sample_data, NUMBER_OF_INPUT_SAMPLE);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_dac_write_data: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("SL_DAC data write is successful \n");
    DEBUGOUT("SL_DAC start \n");
    // Start DAC peripheral
    status = sl_si91x_dac_start();
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_dac_start: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("SL_DAC start is successful \n");

#endif
    // Initialize the OPAMP
    status = sl_si91x_opamp_init();
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_opamp_init: Error code: %lu \n", status);
      break;
    }
    DEBUGOUT("\r\n SL OPAMP Initialization is Successful\r\n");

    // Configure the OPAMP instance from UC
#ifdef SL_OPAMP_OPAMP1
    sl_opamp_config_t opamp1_config = { .opamp_number = SL_OPAMP_1, // Select OPAMP 1 instance

                                        .features = SL_OPAMP_UNITY_GAIN, // Unity Gain Mode

                                        .opamp_config = {
                                          .vin_p_sel   = OPAMP_VIN_P_SEL,  // Positive input pin selection
                                          .vin_n_sel   = OPAMP1_VINN_SEL,  // Negative input pin selection
                                          .vref_sel    = OPAMP_VREF_SEL,   // Reference voltage selection
                                          .enable      = OPAMP_ENABLE,     // Enable OPAMP
                                          .lp_mode     = OPAMP_LP_MODE,    // Low power mode disabled
                                          .r1_sel      = OPAMP_R1_SEL,     // R1 resistor selection
                                          .r2_sel      = OPAMP_R2_SEL,     // R2 resistor selection
                                          .out_mux_en  = OPAMP_OUT_MUX_EN, // Output mux enabled
                                          .out_mux_sel = OPAMP_OUT_MUX_SEL // Output mux selection
                                        } };
    // Configure the OPAMP1 instance from UC
    status = sl_si91x_opamp_set_configuration(&opamp1_config);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_opamp_set_configuration: Error code: %lu \n", status);
      break;
    } else {
      DEBUGOUT("\r\n SL OPAMP1 configuration are set successfully  \n");
    }
#endif
#ifdef SL_OPAMP_OPAMP2
    sl_opamp_config_t opamp2_config = { .opamp_number = SL_OPAMP_2, // Select OPAMP 2 instance

                                        .features = SL_OPAMP_UNITY_GAIN, // Unity Gain Mode

                                        .opamp_config = {
                                          .vin_p_sel   = OPAMP_VIN_P_SEL,  // Positive input pin selection
                                          .vin_n_sel   = OPAMP2_VINN_SEL,  // Negative input pin selection
                                          .vref_sel    = OPAMP_VREF_SEL,   // Reference voltage selection
                                          .enable      = OPAMP_ENABLE,     // Enable OPAMP
                                          .lp_mode     = OPAMP_LP_MODE,    // Low power mode disabled
                                          .r1_sel      = OPAMP_R1_SEL,     // R1 resistor selection
                                          .r2_sel      = OPAMP_R2_SEL,     // R2 resistor selection
                                          .out_mux_en  = OPAMP_OUT_MUX_EN, // Output mux enabled
                                          .out_mux_sel = OPAMP_OUT_MUX_SEL // Output mux selection
                                        } };
    // Configure the OPAMP2 instance from UC
    status = sl_si91x_opamp_set_configuration(&opamp2_config);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_opamp_set_configuration: Error code: %lu \n", status);
      break;
    } else {
      DEBUGOUT("\r\n SL OPAMP2 configuration are set successfully  \n");
    }
#endif
#ifdef SL_OPAMP_OPAMP3
    sl_opamp_config_t opamp3_config = { .opamp_number = SL_OPAMP_3, // Select OPAMP 3 instance

                                        .features = SL_OPAMP_UNITY_GAIN, // Unity Gain Mode

                                        .opamp_config = {
                                          .vin_p_sel   = OPAMP_VIN_P_SEL,  // Positive input pin selection
                                          .vin_n_sel   = OPAMP3_VINN_SEL,  // Negative input pin selection
                                          .vref_sel    = OPAMP_VREF_SEL,   // Reference voltage selection
                                          .enable      = OPAMP_ENABLE,     // Enable OPAMP
                                          .lp_mode     = OPAMP_LP_MODE,    // Low power mode disabled
                                          .r1_sel      = OPAMP_R1_SEL,     // R1 resistor selection
                                          .r2_sel      = OPAMP_R2_SEL,     // R2 resistor selection
                                          .out_mux_en  = OPAMP_OUT_MUX_EN, // Output mux enabled
                                          .out_mux_sel = OPAMP_OUT_MUX_SEL // Output mux selection
                                        } };
    // Configure the OPAMP3 instance from UC
    status = sl_si91x_opamp_set_configuration(&opamp3_config);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_opamp_set_configuration: Error code: %lu \n", status);
      break;
    } else {
      DEBUGOUT("\r\n SL OPAMP3 configuration are set successfully  \n");
    }
#endif

  } while (false);
}

/***************************************************************************
 * Process actions for the OPAMP example
 * 
 * This function retrieves OPAMP features and optionally integrates DAC 
 * functionality if enabled. To check the battery voltage using the OPAMP 
 * peripheral.
 ******************************************************************************/
void opamp_example_process_action(void)
{
#if INPUT_DAC_NEG_INPUT_EXTERNAL == ENABLE
  do {
    if (dac_static_intr_flag == true) {
      dac_static_intr_flag = false;
      DEBUGOUT("Data successfully sampled \n");
      DEBUGOUT("SL OPAMP DAC input sample = %d\n", dac_input_sample_data[0]);
      // DAC output
      float dac_output = ((float)dac_input_sample_data[0] / (float)(MAX_DAC_INPUT_SAMPLE_VALUE + 1)) * (vref_value);
      DEBUGOUT("SL OPAMP DAC output voltage = %f\n", dac_output);
    }
  } while (false);
#endif
}

/*******************************************************************************
 * DAC callback function
 *
 * This function handling DAC events. It is invoked when
 * a DAC event occurs and can be customized for specific event handling needs.
 ******************************************************************************/
#if INPUT_DAC_NEG_INPUT_EXTERNAL == ENABLE
static void dac_callback_event(uint8_t event)
{
  if (event == SL_DAC_STATIC_MODE_EVENT) {
    dac_static_intr_flag = true;
  }
  if (event == SL_DAC_FIFO_MODE_EVENT) {
    dac_fifo_intr_flag = true;
  }
}
#endif