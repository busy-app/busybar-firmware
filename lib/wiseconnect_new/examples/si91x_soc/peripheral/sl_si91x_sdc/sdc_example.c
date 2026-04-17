/***************************************************************************/
/**
 * @file sdc_example.c
 * @brief SDC example
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
 ******************************************************************************/

#include "sdc_example.h"
#include "sl_si91x_power_manager.h"
#include "rsi_debug.h"
#include "rsi_rom_clks.h"
#include "rsi_rom_ulpss_clk.h"
#include "sl_si91x_sdc_common_config.h"
#include "sl_sdc_instances.h"
#include "sl_si91x_wireless_shutdown.h"

/***************************************************************************************************************************************
************************************************************ DEFINES ***************************************************************
**************************************************************************************************************************************** */
#define SDC_VREF                  3.3 //reference voltage
#define SDC_BUFFER_SIZE           16  //buffer size
#define SDC_CLOCK_DIVISION_FACTOR 1   //Clock division factor for SDC
#define SDC_NUMBER_OF_SAMPLES     SDC_BUFFER_SIZE
#define SDC_CHANNEL1_INDEX        0
#define SDC_CHANNEL2_INDEX        1
#define SDC_CHANNEL3_INDEX        2
#define SDC_CHANNEL4_INDEX        3

/***************************************************************************************************************************************
************************************************************ Enums ***************************************************************
**************************************************************************************************************************************** */

// Enum for different transmission scenarios
typedef enum {
  SL_SDC_PROCESS_ACTION,       //SDC app process actions
  SL_SDC_PS2_POWER_TRANSITION, // switch to ps2
  SL_SDC_PS1_POWER_TRANSITION, // switch to ps1
  SL_SDC_SAMPLING_FAILED,      // sdc start failed
  SL_SDC_SAMPLING_COMPLETED,   // sdc transmission completed
  SL_SDC_DATA_PROCESSING,      // sdc data processing
  SL_SDC_END_STATE
} ulp_adc_enum_t;

/***************************************************************************************************************************************
****************************************************** Local Function Prototypes ***************************************************
**************************************************************************************************************************************** */

static void callback_event(uint8_t channel_no, sl_sdc_event_t event);

static void configuring_ps2_power_state(void);

static void process_sdc_samples();

/***************************************************************************************************************************************
************************************************************ Variables ***************************************************************
**************************************************************************************************************************************** */

float sdc_vref             = SDC_VREF; // Voltage reference
uint32_t sdc_sampling_done = 0;        // Trasfer done flag
uint32_t sdc_data_ready    = 0;        //SDC data ready flag
uint32_t sdc_current_mode  = SL_SDC_PROCESS_ACTION;

int16_t sdc_read_buffer[SDC_BUFFER_SIZE] = { 0 }; //buffer

/*
  * This function processes the SDC samples read from the buffer.
  * It converts the raw SDC values to voltage based on the configured mode (single-ended or
  * differential) and prints the converted values.
*/
static void process_sdc_samples()
{
  float data = 0;
  for (int i = 0; i < SDC_NUMBER_OF_SAMPLES; i++) {
    if (sl_sdc_pin_config[SDC_CHANNEL1_INDEX].sdc_adc_mode == SL_SDC_SINGLE_ENDED_MODE)
      data = ((float)sdc_read_buffer[i] / (float)SDC_MAX_VALUE) * sdc_vref;
    else if (sl_sdc_pin_config[SDC_CHANNEL1_INDEX].sdc_adc_mode == SL_SDC_DIFFERENTIAL_MODE)
      data = ((((float)sdc_read_buffer[i] / (float)SDC_MAX_VALUE) * sdc_vref) - (sdc_vref / 2));
    DEBUGOUT("\r\n SDC sample : %f \r\n", data);
  }
}

/*
  * This function is a callback that handles SDC events.
  * It updates the flags for data readiness and sampling completion based on the event type.
  * It also processes the SDC samples when a buffer reset event occurs.
*/
static void callback_event(uint8_t channel_no, sl_sdc_event_t event)
{
  (void)channel_no;
  switch (event) {
    case SDC_EVENT_DATA_READY:
      sdc_data_ready = 1;
      break;
    case SDC_EVENT_SAMPLE_COMPLETE:
      sdc_sampling_done = 1;
      break;
    case SDC_EVENT_BUFFER_RESET:
      process_sdc_samples();
      DEBUGOUT("\r\n Channel %u SDC Buffer reset \r\n", channel_no);
      sdc_current_mode  = SL_SDC_PS1_POWER_TRANSITION;
      sdc_sampling_done = 0;
      sdc_data_ready    = 0;
      break;

    case SDC_EVENT_BUFFER_FULL:
      DEBUGOUT("\r\n Channel %u SDC Buffer full \r\n", channel_no);
      break;

    default:
      break;
  }
}

/*
  * This function initializes the SDC driver with the specified configuration.
  * It sets up the SDC driver with the reference voltage, sampling parameters, and channel configurations.
  * It also registers a callback function to handle SDC events.
  * If any step fails, it updates the current mode to indicate a sampling failure.
*/
void sdc_example_init()
{
  do {
    sl_status_t status = SL_STATUS_OK;
    sl_si91x_sdc_channel_info_t sdc_channel_config;
    sl_si91x_sdc_config_t sl_sdc_config = {

      .number_of_channels    = SDC_NUMBER_OF_CHANNELS,
      .sample_threshold      = SL_SDC_SAMPLING_THRESHOLD,
      .trigger_ms_sec_select = SL_SDC_MILLI_SEC_ENABLE,
      .sampling_interval     = SL_SDC_SAMPLING_INTERVAL,
      .clock_division_factor = SDC_CLOCK_DIVISION_FACTOR

    };
    sdc_channel_config.recieve_buffer[SDC_CHANNEL1_INDEX] = sdc_read_buffer;
    sdc_channel_config.sample_length[SDC_CHANNEL1_INDEX]  = SDC_NUMBER_OF_SAMPLES;

    // Initialize the SDC driver with the reference voltage.
    status = sl_si91x_sdc_driver_init(sdc_vref);
    if (status != SL_STATUS_OK) {
      // If status is not OK, return with the error code.
      DEBUGOUT("Error Code: 0x%lX, SDC driver init Failed \n", status);
      sdc_current_mode = SL_SDC_SAMPLING_FAILED;
      break;
    } else {
      DEBUGOUT("\r\n SDC Driver Init Success \r\n");
    }

    // Configure the SDC driver with sampling parameters.
    status = sl_si91x_sdc_driver_config(&sl_sdc_config);

    if (status != SL_STATUS_OK) {
      // If status is not OK, return with the error code.
      DEBUGOUT("Error Code: 0x%lX, driver config Failed \n", status);
      sdc_current_mode = SL_SDC_SAMPLING_FAILED;
      break;
    } else {
      DEBUGOUT("\r\n SDC Driver Configuration done \r\n");
    }

    // Configure the SDC driver channels with pin configuration and channel settings.
    status = sl_si91x_sdc_driver_channel_config(sl_sdc_pin_config, &sdc_channel_config, sl_sdc_channel_misc_config);

    if (status != SL_STATUS_OK) {
      // If status is not OK, return with the error code.
      DEBUGOUT("Error Code: 0x%lX, driver channel config Failed \n", status);
      sdc_current_mode = SL_SDC_SAMPLING_FAILED;
      break;
    } else {
      DEBUGOUT("\r\n SDC Driver channel Configuration done \r\n");
    }

    // Register the callback function for SDC events.
    status = sl_si91x_sdc_driver_register_callback(callback_event);

    if (status != SL_STATUS_OK) {
      // If status is not OK, return with the error code.
      DEBUGOUT("Error Code: 0x%lX, register callback Failed \n", status);
      sdc_current_mode = SL_SDC_SAMPLING_FAILED;
      break;
    } else {
      DEBUGOUT("\r\n SDC Driver Register callback done \r\n");
    }
  } while (false);
}

/*
  * This function processes the SDC application actions based on the current mode.
  * It handles the SDC data reading process, power state transitions, and data processing.
  * It also manages the power management requirements and transitions between different power states.
  * The function updates the current mode based on the success or failure of each action.
*/
void sdc_example_app_process_action()
{
  sl_status_t status = SL_STATUS_OK;
  do {
    switch (sdc_current_mode) {
      case SL_SDC_PROCESS_ACTION:

        // Start the SDC data reading process.
        status = sl_si91x_sdc_driver_read_data_start();

        if (status != SL_STATUS_OK) {
          DEBUGOUT("\r\n SDC read data failed %lu \r\n", status);
          sdc_current_mode = SL_SDC_SAMPLING_FAILED;
          break;
        }
        sdc_current_mode = SL_SDC_PS2_POWER_TRANSITION;
        break;
      case SL_SDC_PS2_POWER_TRANSITION:
        // Control power management by adjusting clock references and shutting down
        // the power supply
        // This function is for demonstration purpose only. For more details, refer to the README file.
        sl_si91x_wireless_shutdown();
        // switching the power state PS4 to PS2 mode.
        status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS2);

        if (status != SL_STATUS_OK) {
          DEBUGOUT("sl_si91x_power_manager_add_ps_requirement: Error Code : %lu \n", status);
          sdc_current_mode = SL_SDC_SAMPLING_FAILED;
          break;
        }
        // Configuring the ps2 power state by configuring
        // the ram retention and removing the unused peripherals
        configuring_ps2_power_state();

        DEBUGINIT(); // ReInitialize debug prints after configuring PS2 state.

        // Remove PS2 state requirement to transition to PS1 state.
        // This is done to ensure that the system can transition to PS1 state after PS2
        // configuration is complete.
        status = sl_si91x_power_manager_remove_ps_requirement(SL_SI91X_POWER_MANAGER_PS2);
        if (status != SL_STATUS_OK) {
          DEBUGOUT("sl_si91x_power_manager_add_ps_requirement: Error Code : %lu \n", status);
          break;
        }
        sdc_current_mode = SL_SDC_PS1_POWER_TRANSITION;
        break;

      case SL_SDC_PS1_POWER_TRANSITION:
        //Set SDC as wakeup source;
        status = sl_si91x_power_manager_set_wakeup_sources(SDCSS_BASED_WAKEUP, 1);
        if (status != SL_STATUS_OK) {
          printf("\r\n set sdc as wakeup source failed %lu \r\n", status);
          sdc_current_mode = SL_SDC_SAMPLING_FAILED;
          break;
        }

        // PS1 State requirement is added, it transits to PS1 state.
        status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS1);
        if (status != SL_STATUS_OK) {
          DEBUGOUT("sl_si91x_power_manager_add_ps_requirement123: Error Code : %lu \n", status);
          sdc_current_mode = SL_SDC_SAMPLING_FAILED;
          break;
        }
        sdc_current_mode = SL_SDC_DATA_PROCESSING;
        break;
      case SL_SDC_DATA_PROCESSING:
        // Process the SDC data after waking up from PS1 state.

        DEBUGINIT(); // Initialize debug prints after waking up.

        if (sdc_data_ready) {
          sl_si91x_sdc_driver_store_data();
        }
        if (sdc_sampling_done) {
          sdc_current_mode = SL_SDC_SAMPLING_COMPLETED;
        } else {
          sdc_current_mode = SL_SDC_PS1_POWER_TRANSITION;
        }
        break;

      case SL_SDC_SAMPLING_FAILED:
        DEBUGINIT();
        DEBUGOUT("\r\n SDC Transmission Failed\n");
        sdc_current_mode = SL_SDC_END_STATE;
        break;

      case SL_SDC_SAMPLING_COMPLETED:
        DEBUGINIT();
        DEBUGOUT("\r\n SDC Sampling completed \r\n");
        process_sdc_samples();
        sdc_current_mode = SL_SDC_END_STATE;
        break;

      case SL_SDC_END_STATE:
        break;
    }
  } while (false);
}

/*
  * This function configures the PS2 power state by removing unused peripherals and configuring RAM retention.
  * It sets up the peripheral requirements for M4SS, ULPSS, and NPSS, and configures the RAM banks for retention.
  * If any step fails, it prints an error message with the status code.
*/
static void configuring_ps2_power_state(void)
{
  sl_status_t status;
  sl_power_peripheral_t peri;
  sl_power_ram_retention_config_t config;
  peri.m4ss_peripheral = 0;
  // Configure RAM banks for retention during power management
  config.configure_ram_banks = true; // Enable RAM bank configuration
  config.m4ss_ram_banks      = SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_8 | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_9
                          | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_10; // Specify the RAM banks to be
                                                                     // retained during power
                                                                     // management
  config.ulpss_ram_banks = SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_2 | SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_3;
  // Ored value for ulpss peripheral.
  peri.ulpss_peripheral = SL_SI91X_POWER_MANAGER_ULPSS_PG_MISC | SL_SI91X_POWER_MANAGER_ULPSS_PG_SSI
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_I2S | SL_SI91X_POWER_MANAGER_ULPSS_PG_I2C
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_IR | SL_SI91X_POWER_MANAGER_ULPSS_PG_FIM;
  // Ored value for npss peripheral.
  peri.npss_peripheral = SL_SI91X_POWER_MANAGER_NPSS_PG_MCUWDT | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUPS
                         | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUTS | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE2
                         | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE3;
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