/***************************************************************************/ /**
 * @file adc_fifo_mode_example.c
 * @brief ADC example for FIFO mode
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "sl_si91x_adc.h"
#include "adc_fifo_mode_example.h"
#include "rsi_debug.h"

#include "sl_adc_instances.h"
#include "sl_si91x_adc_common_config.h"
#ifdef DAC_FIFO_MODE_EN
#include "sl_si91x_dac.h"
#include "sl_si91x_dac_config.h"
#endif
#include "rsi_rom_clks.h"
/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
/* Core clock modification Macros */
#define PS4_SOC_FREQ          180000000  /*<! PLL out clock 180MHz            */
#define SOC_PLL_REF_FREQUENCY 40000000   /*<! PLL input REFERENCE clock 40MHZ */
#define DVISION_FACTOR        0          // Division factor
#define CHANNEL_SAMPLE_LENGTH 1023       // Number of ADC sample collect for operation
#define ADC_MAX_OP_VALUE      4095       // Maximum output value get from adc data register
#define ADC_DATA_CLEAR        0xF7FF     // Clear the data if 12th bit is enabled
#define VREF_VALUE            3.3        // reference voltage
#define ADC_PING_BUFFER       0x0000A000 // ADC buffer starting address
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static float vref_value                    = (float)VREF_VALUE;
static boolean_t data_sample_complete_flag = false;
static int16_t adc_output[CHANNEL_SAMPLE_LENGTH];
static uint8_t adc_channel = 0;
#ifdef DAC_FIFO_MODE_EN
static boolean_t dac_fifo_intr_flag = false;
#endif
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void callback_event(uint8_t event_channel, uint8_t event);
#ifdef DAC_FIFO_MODE_EN
static void dac_callback_event(uint8_t event);
#endif
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * ADC example initialization function
 ******************************************************************************/
void adc_fifo_mode_example_init(void)
{
  sl_adc_version_t version;
  sl_status_t status;
  sl_adc_channel_config.channel = SL_ADC_CHANNEL_1;
  adc_channel                   = sl_adc_channel_config.channel;

#ifdef DAC_FIFO_MODE_EN
  sl_dac_clock_config_t dac_clock_config;
  dac_clock_config.soc_pll_clock           = PS4_SOC_FREQ;
  dac_clock_config.soc_pll_reference_clock = SOC_PLL_REF_FREQUENCY;
  dac_clock_config.division_factor         = DVISION_FACTOR;
#endif
  // DMA Ping/Pong buffer configuration for FIFO mode:
  // These fields MUST be configured before calling sl_si91x_adc_init() and
  // sl_si91x_adc_set_channel_configuration() for FIFO+DMA operation.
  // The init function does NOT automatically set these addresses.
  sl_adc_channel_config.rx_buf[adc_channel]            = adc_output;
  sl_adc_channel_config.chnl_ping_address[adc_channel] = ADC_PING_BUFFER; /* ADC Ping address */
  sl_adc_channel_config.chnl_pong_address[adc_channel] =
    ADC_PING_BUFFER + (sl_adc_channel_config.num_of_samples[adc_channel]); /* ADC Pong address */
  do {
    // Version information of ADC driver
    version = sl_si91x_adc_get_version();
    DEBUGOUT("ADC version is fetched successfully \n");
    DEBUGOUT("API version is %d.%d.%d\n", version.release, version.major, version.minor);

    status = sl_si91x_adc_init(sl_adc_channel_config, sl_adc_config, vref_value);
    /* Due to calling trim_efuse API on ADC init in driver it will change the clock frequency,
      if we are not initialize the debug again it will print the garbage data in console output. */
    DEBUGINIT();
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_adc_init: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("ADC Initialization Success\n");
    // Configure ADC channel.
    status = sl_si91x_adc_set_channel_configuration(sl_adc_channel_config, sl_adc_config);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_adc_channel_set_configuration: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("ADC Channel Configuration Successfully \n");
    // Register user callback function
    status = sl_si91x_adc_register_event_callback(callback_event);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_adc_register_event_callback: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("ADC user event callback registered successfully \n");
#ifdef DAC_FIFO_MODE_EN
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
#endif
    status = sl_si91x_adc_start(sl_adc_config);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_adc_start: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("ADC started Successfully\n");
  } while (false);
}

/*******************************************************************************
 * Function will run continuously and will wait for trigger
 ******************************************************************************/
void adc_fifo_mode_example_process_action(void)
{
  sl_status_t status;
  uint32_t sample_length;
  float vout = 0.0f;
  if (data_sample_complete_flag) {
    // ADC operation mode on FIFO mode, it will execute, here it will give equivalent voltage of 12 bit adc output.
    status = sl_si91x_adc_read_data(sl_adc_channel_config, adc_channel);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_adc_read_data: Error Code : %lu \n", status);
    }
#ifdef DAC_FIFO_MODE_EN
    if (dac_fifo_intr_flag == false) {
      // DAC input sample data writing
      status = sl_si91x_dac_write_data((int16_t *)adc_output, sl_adc_channel_config.num_of_samples[0]);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_dac_write_data: Error Code : %lu \n", status);
      }
      // Start DAC peripheral
      status = sl_si91x_dac_start();
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_dac_start: Error Code : %lu \n", status);
      }
    } else {
      if (dac_fifo_intr_flag == true) {
        dac_fifo_intr_flag = false;
        // DAC input sample data re-writing
        status = sl_si91x_dac_rewrite_data((int16_t *)adc_output, sl_adc_channel_config.num_of_samples[0]);
        if (status != SL_STATUS_OK) {
          DEBUGOUT("sl_si91x_dac_rewrite_data: Error Code : %lu \n", status);
        }
      }
    }
#endif
    for (sample_length = 0; sample_length < sl_adc_channel_config.num_of_samples[adc_channel]; sample_length++) {
      /* In two’s complement format, the MSb (11th bit) of the conversion result determines the polarity,
             when the MSb = ‘0’, the result is positive, and when the MSb = ‘1’, the result is negative*/
      if (adc_output[sample_length] & SIGN_BIT) {
        // Full-scale would be represented by a hexadecimal value, full-scale range of ADC result values in two’s complement.
        adc_output[sample_length] = (int16_t)(adc_output[sample_length] & (ADC_DATA_CLEAR));
      } else { // set the MSb bit.
        adc_output[sample_length] = adc_output[sample_length] | SIGN_BIT;
      }
      vout = (((float)adc_output[sample_length] / (float)ADC_MAX_OP_VALUE) * vref_value);
      //For differential type it will give vout.
      if (sl_adc_channel_config.input_type[adc_channel]) {
        vout = vout - (vref_value / 2);
      }
      DEBUGOUT("ADC Measured input[%ld] :%0.2fV \n", sample_length, (double)vout);
    }
    data_sample_complete_flag = false;
  }
}

/*******************************************************************************
 * Callback event function
 * It is responsible for the event which are triggered by ADC interface
 * @param  event       : INTERNAL_DMA => Single channel data acquisition done.
 *                       ADC_STATIC_MODE_CALLBACK => Static mode adc data
 *                       acquisition done.
 ******************************************************************************/
static void callback_event(uint8_t event_channel, uint8_t event)
{
  if ((event == SL_INTERNAL_DMA) && (event_channel == adc_channel)) {
    data_sample_complete_flag = true;
  }
}
#ifdef DAC_FIFO_MODE_EN
/*******************************************************************************
* Callback event function
* It is responsible for the event which are triggered by DAC interface
* @param  event       : SL_DAC_FIFO_MODE_EVENT => DAC input sample for
*                                 standard sine wave data acquisition done.
*                       SL_DAC_STATIC_MODE_EVENT => Static mode DAC data
*                                                      acquisition done.
******************************************************************************/
static void dac_callback_event(uint8_t event)
{
  if (event == SL_DAC_FIFO_MODE_EVENT) {
    dac_fifo_intr_flag = true;
  }
}
#endif
