/***************************************************************************/
/**
 * @file sl_ulp_adc_example.c
 * @brief ADC example
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
#include "sl_ulp_adc_example.h"

#include "rsi_debug.h"
#include "sl_adc_instances.h"
#include "sl_si91x_adc.h"
#include "sl_si91x_adc_common_config.h"
#include "sl_si91x_power_manager.h"
#include "sl_si91x_wireless_shutdown.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
/* Macros */
#define CHANNEL_SAMPLE_LENGTH 1023       // Number of ADC sample collect for operation
#define ADC_MAX_OP_VALUE      4095       // Maximum output value get from adc data register
#define ADC_DATA_CLEAR        0xF7FF     // Clear the data if 12th bit is enabled
#define VREF_VALUE            3.3        // reference voltage
#define ADC_PING_BUFFER       0x24060800 // ADC buffer starting address

#define MS_DELAY_COUNTER  4600 // Delay count
#define FIVE_SECOND_DELAY 5000 // giving the 5 second delay in between state changes

/*******************************************************************************
 ******************************  Data Types  ***********************************
 ******************************************************************************/
// Enum for different transmission scenarios
typedef enum {
  SL_ULP_ADC_PROCESS_ACTION,         // adc event triggering and operational
                                     // transforming
  SL_ULP_ADC_POWER_STATE_TRANSITION, // adc power state transitioning mode
  SL_ULP_ADC_TRANSMISSION_COMPLETED, // adc transmission completed mode
} ulp_adc_enum_t;

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static float vref_value              = (float)VREF_VALUE;
static boolean_t chnl0_complete_flag = false;
static int16_t adc_output[CHANNEL_SAMPLE_LENGTH];
static uint8_t adc_channel                 = 0;
static ulp_adc_enum_t ulp_adc_current_mode = SL_ULP_ADC_PROCESS_ACTION;
sl_status_t status;
static sl_power_state_t current_power_state = SL_SI91X_POWER_MANAGER_PS4;

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void callback_event(uint8_t channel_no, uint8_t event);
static void adc_read_data_static_mode(void);
static void adc_read_data_fifo_mode(void);
static void delay(uint32_t idelay);
static void configuring_ps2_power_state(void);
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/*******************************************************************************
 * ADC example initialization function
 ******************************************************************************/
void adc_example_init(void)
{
  sl_adc_version_t version;
  sl_adc_channel_config.channel                        = SL_ADC_CHANNEL_1;
  adc_channel                                          = sl_adc_channel_config.channel;
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
    /* Due to calling trim_efuse API on ADC init in driver it will change the
      clock frequency, if we don't initialize the debug again it will print
      the garbage data in console output. */
    DEBUGINIT();
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_adc_init: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("ADC Initialization Success\n");
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
void adc_example_process_action(void)
{
  /*************************************************************************************************
   * This section manages power state transitions within the system, optimizing
   *power consumption while maintaining essential functionality. It transitions
   *the system from a higher power state (PS4) to a lower one (PS2) during
   *specific operations to conserve power. This involves adjusting clock
   * references and shutting down unnecessary power supplies. After completing
   *the operation, the code transitions back to the higher power state (PS4) to
   *ensure adequate resources for subsequent tasks. This approach balances power
   *efficiency with operational requirements across various system functions.
   ***************************************************************************************************/
  switch (ulp_adc_current_mode) {
    case SL_ULP_ADC_PROCESS_ACTION:
      if (chnl0_complete_flag) {
        // ADC operation mode if FIFO then it will execute, here it will give
        // equivalent voltage of 12 bit adc output.
        if (!sl_adc_config.operation_mode) {
          adc_read_data_fifo_mode();
        } else {
          adc_read_data_static_mode();
        }
        chnl0_complete_flag = false;
        // current mode being updated with power state transition to change the
        // power state mode
        ulp_adc_current_mode = SL_ULP_ADC_POWER_STATE_TRANSITION;
      }
      break;
    case SL_ULP_ADC_POWER_STATE_TRANSITION:
      if (current_power_state == SL_SI91X_POWER_MANAGER_PS4) {
        DEBUGOUT("Switching adc from PS4->PS2 state \n");
        // Stop the ADC
        status = sl_si91x_adc_stop(sl_adc_config);
        // De-initialise the ADC
        status = sl_si91x_adc_deinit(sl_adc_config);
        // Control power management by adjusting clock references and shutting down
        // the power supply
        // This function is for demonstration purpose only. For more details, refer to the README file.
        sl_si91x_wireless_shutdown();
        // switching the power state PS4 to PS2 mode.
        status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS2);
        if (status != SL_STATUS_OK) {
          DEBUGOUT("sl_si91x_power_manager_add_ps_requirement: Error Code : %lu \n", status);
          break;
        }
        /* Due to calling trim_efuse API on power manager it will change the clock
    frequency, if we are not initialize the debug again it will print the
    garbage data or no data in console output. */
        DEBUGINIT();
        // Configuring the ps2 power state by configuring
        // the ram retention and removing the unused peripherals
        configuring_ps2_power_state();
        // Adding 5 seconds delay for verifying the current consumption
        delay(FIVE_SECOND_DELAY);
        adc_example_init();
        // current power state is updated to PS2
        current_power_state = SL_SI91X_POWER_MANAGER_PS2;
        // current mode is updated with process action for verifying the
        // triggering and processing
        ulp_adc_current_mode = SL_ULP_ADC_PROCESS_ACTION;
      } else if (current_power_state == SL_SI91X_POWER_MANAGER_PS2) {
        DEBUGOUT("Switching the adc from PS2->PS4 state\n");
        // Stop the ADC
        status = sl_si91x_adc_stop(sl_adc_config);
        // De-initialise the ADC
        status = sl_si91x_adc_deinit(sl_adc_config);
        // switching the power state from PS2 to PS4 mode
        status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS4);
        if (status != SL_STATUS_OK) {
          DEBUGOUT("sl_si91x_power_manager_add_ps_requirement: Error Code : %lu \n", status);
          break;
        }
        /* Due to calling trim_efuse API om power manager it will change the clock
    frequency, if we are not initialize the debug again it will print the
    garbage data or no data in console output. */
        DEBUGINIT();
        // Adding 5 seconds delay for verifying the current consumption
        delay(FIVE_SECOND_DELAY);
        adc_example_init();
        // current power state is updated to last enum after the power state cycle
        // is completed
        current_power_state = LAST_ENUM_POWER_STATE;
        // current mode is updated with process action for verifying the
        // triggering and processing
        ulp_adc_current_mode = SL_ULP_ADC_PROCESS_ACTION;
      } else {
        //  stop the adc
        status = sl_si91x_adc_stop(sl_adc_config);
        if (status != SL_STATUS_OK) {
          DEBUGOUT("sl_si91x_adc_stop: Error Code : %lu \n", status);
        }
        DEBUGOUT("ADC stopped successfully \n");
        //  de initializing the adc
        status = sl_si91x_adc_deinit(sl_adc_config);
        if (status != SL_STATUS_OK) {
          DEBUGOUT("sl_si91x_adc_deinit: Error Code : %lu \n", status);
        }
        chnl0_complete_flag = false;
        DEBUGOUT("ADC deinit successfully \n");
        ulp_adc_current_mode = SL_ULP_ADC_TRANSMISSION_COMPLETED;
      }
      break;
    case SL_ULP_ADC_TRANSMISSION_COMPLETED:
      break;
    default:
      break;
  }
}

/*******************************************************************************
 * Callback event function
 * It is responsible for the event which are triggered by ADC interface
 *
 * @param  event       : INTERNAL_DMA => Single channel data acquisition done.
 ******************************************************************************/
static void callback_event(uint8_t channel_no, uint8_t event)
{
  if (event == SL_INTERNAL_DMA) {
    if (channel_no == adc_channel) {
      chnl0_complete_flag = true;
    }
  } else if (event == SL_ADC_STATIC_MODE_EVENT) {
    chnl0_complete_flag = true;
  }
}

/*******************************************************************************
 * Delay function for 1ms
 ******************************************************************************/
static void delay(uint32_t idelay)
{
  for (uint32_t x = 0; x < MS_DELAY_COUNTER * idelay; x++) {
    __NOP();
  }
}

/*******************************************************************************
 * ADC read data in fifo mode will be responsible for reading the ADC data
 * and computing the vout value
 ******************************************************************************/
static void adc_read_data_fifo_mode(void)
{
  uint32_t sample_length;
  uint8_t chnl_num = 0;
  float vout       = 0.0f;
  for (chnl_num = 0; chnl_num < sl_adc_config.num_of_channel_enable; chnl_num++) {
    status = sl_si91x_adc_read_data(sl_adc_channel_config, adc_channel);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_adc_read_data: Error Code : %lu \n", status);
    }
    for (sample_length = 0; sample_length < sl_adc_channel_config.num_of_samples[adc_channel]; sample_length++) {
      /* In two’s complement format, the MSb (11th bit) of the conversion
               result determines the polarity, when the MSb = ‘0’, the result is
               positive, and when the MSb = ‘1’, the result is negative*/
      if (adc_output[sample_length] & SIGN_BIT) {
        // Full-scale would be represented by a hexadecimal value,
        // full-scale range of ADC result values in two’s complement.
        adc_output[sample_length] = (int16_t)(adc_output[sample_length] & (ADC_DATA_CLEAR));
      } else { // set the MSb bit.
        adc_output[sample_length] = adc_output[sample_length] | SIGN_BIT;
      }
      vout = (((float)adc_output[sample_length] / (float)ADC_MAX_OP_VALUE) * vref_value);
      // For differential type it will give vout.
      if (sl_adc_channel_config.input_type[adc_channel]) {
        vout = vout - (vref_value / 2);
      }
      printf("ADC Measured input[%ld] :%.2fV \n", sample_length, (double)vout);
    }
  }
}
/*******************************************************************************
 * ADC read data in static mode will be responsible for reading the ADC data
 * and computing the vout value
 ******************************************************************************/
static void adc_read_data_static_mode(void)
{
  uint16_t adc_value;
  float vout = 0.0f;
  status     = sl_si91x_adc_read_data_static(sl_adc_channel_config, sl_adc_config, &adc_value);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("sl_si91x_adc_read_data_static: Error Code : %lu \n", status);
  }
  // Read the data from register and store it in variable.
  adc_output[0] = (int16_t)adc_value;
  /* In two’s complement format, the MSb (11th bit) of the conversion
           result determines the polarity, when the MSb = ‘0’, the result is
           positive, and when the MSb = ‘1’, the result is negative*/
  if (adc_output[0] & SIGN_BIT) {
    // Full-scale would be represented by a hexadecimal value, full-scale
    // range of ADC result values in two’s complement.
    adc_output[0] = (int16_t)(adc_output[0] & (ADC_DATA_CLEAR));
  } else { // set the MSb bit.
    adc_output[0] = adc_output[0] | SIGN_BIT;
  }
  vout = (((float)adc_output[0] / (float)ADC_MAX_OP_VALUE) * vref_value);
  // For differential type it will give vout.
  if (sl_adc_channel_config.input_type[adc_channel]) {
    vout = vout - (vref_value / 2);
    DEBUGOUT("Differential ended input  :%lf\n", (double)vout);
  } else {
    DEBUGOUT("Single ended input :%lf\n", (double)vout);
  }
}
/*******************************************************************************
 * powering off the peripherals not in use,
 * Configuring power manager ram-retention
 ******************************************************************************/
static void configuring_ps2_power_state(void)
{
  sl_power_peripheral_t peri;
  sl_power_ram_retention_config_t config;
  // Configure RAM banks for retention during power management
  config.configure_ram_banks = true; // Enable RAM bank configuration
  config.m4ss_ram_banks      = SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_8 | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_9
                          | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_10; // Specify the RAM banks to be
                                                                     // retained during power
                                                                     // management
  config.ulpss_ram_banks = 0;
  peri.m4ss_peripheral   = 0;
  // Ored value for ulpss peripheral.
  peri.ulpss_peripheral = SL_SI91X_POWER_MANAGER_ULPSS_PG_MISC | SL_SI91X_POWER_MANAGER_ULPSS_PG_SSI
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_I2S | SL_SI91X_POWER_MANAGER_ULPSS_PG_I2C
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_IR | SL_SI91X_POWER_MANAGER_ULPSS_PG_FIM;
  // Ored value for npss peripheral.
  peri.npss_peripheral = SL_SI91X_POWER_MANAGER_NPSS_PG_MCURTC | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUWDT
                         | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUPS | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUTS
                         | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE2 | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE3
                         | SL_SI91X_POWER_MANAGER_NPSS_PG_TIMEPERIOD;
  do {
    // Peripherals passed in this API are powered off.
    status = sl_si91x_power_manager_remove_peripheral_requirement(&peri);
    if (status != SL_STATUS_OK) {
      // If status is not OK, return with the error code.
      DEBUGOUT("sl_si91x_power_manager_remove_peripheral_requirement failed, "
               "Error Code: 0x%lX",
               status);
      break;
    }
    // RAM retention modes are configured and passed into this API.
    status = sl_si91x_power_manager_configure_ram_retention(&config);
    if (status != SL_STATUS_OK) {
      // If status is not OK, return with the error code.
      DEBUGOUT("sl_si91x_power_manager_configure_ram_retention failed, Error "
               "Code: 0x%lX",
               status);
      break;
    }
  } while (false);
}
