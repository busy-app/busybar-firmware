/*******************************************************************************
 * @file bod_example.c
 * @brief BOD example
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

/* Includes ------------------------------------------------------------------*/
#include "bod_example.h"
#include "rsi_debug.h"
#include "sl_si91x_bod.h"
#include "sl_si91x_clock_manager.h"

/*******************************************************************************
***************************  Defines / Macros  ********************************
******************************************************************************/

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static float vbatt                          = 0;
static float vbat_percentage                = 0;
static volatile boolean_t sl_bod_inter_flag = 0;
sl_bod_uc_param_t usr_config_params         = { .slot_value = SL_BOD_DEFAULT_SLOT_VALUE, .blackout_en = DISABLE };
/******************************************************************************
**************************   GLOBAL VARIABLES   *******************************
******************************************************************************/

/******************************************************************************
**************************   GLOBAL FUNCTIONS   *******************************
******************************************************************************/

/******************************************************************************
**************************   BOD Callback Function   **************************
******************************************************************************/
static void bod_callback(void)
{
  // Set the BOD interrupt flag
  sl_bod_inter_flag = 1;
}

/******************************************************************************
************   Initializes the BOD (Brown-Out Detection) example   ************
******************************************************************************/
void bod_example_init(void)
{
  sl_status_t status    = SL_STATUS_FAIL; // Variable to store the status of BOD operations
  float threshold_value = 0;              // Variable to store the BOD threshold value
  do {
    // Initialize the BOD (Brown-Out Detector)
    status = sl_si91x_bod_init();
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n BOD Initialization Failed\r\n");
      break;
    } else {
      DEBUGOUT("\r\n BOD Initialization is Successful\r\n");
    }
    // Register the BOD callback function
    status = sl_si91x_bod_register_callback(bod_callback);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n  BOD callback Registered Fail \r\n");
      break;
    } else {
      DEBUGOUT("\r\n Successfully Registered  BOD callback \r\n");
    }
    // Set the BOD threshold
    status = sl_si91x_bod_set_threshold(SL_BOD_DEFAULT_THRESHOLD);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n BOD Threshold Configuration Failed\r\n");
      break; // Return if setting threshold fails
    } else
      DEBUGOUT("\r\n BOD Threshold Configuration Successful\r\n");

    // Set the BOD configuration parameters
    status = sl_si91x_bod_set_configuration(usr_config_params);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n BOD Configurations Failed\r\n");
      break;
    } else {
      DEBUGOUT("\r\n BOD Configurations Successful\r\n");
    }

    // Get the calibrated threshold value
    status = sl_si91x_bod_get_threshold(&threshold_value);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n BOD Configurations Failed\r\n");
      break;
    } else {
      DEBUGOUT("\r\n BOD Threshold Value is %.2f V\r\n", threshold_value);
    }

    // Enable BOD interrupt
    sl_si91x_bod_enable_interrupt();
    DEBUGOUT("\r\n BOD Interrupt Enabled successfully \r\n");
  } while (false);
}

/*****************************************************************************
 ******** Process action for the BOD (Brown-Out Detection) example. **********
 *****************************************************************************/
void bod_example_process_action(void)
{
  sl_status_t status = SL_STATUS_FAIL; // Variable to store the status of BOD operations
  do {
    if (sl_bod_inter_flag) {
      // Get the battery status (voltage level)
      status = sl_si91x_bod_get_battery_status(&vbatt);
      if (status == SL_STATUS_OK) {
        // Get the battery percentage based on the voltage level
        status = sl_si91x_bod_battery_percentage(vbatt, &vbat_percentage);
        if (status == SL_STATUS_OK) {
          // Check if the BOD interrupt flag is set
          if (sl_si91x_bod_get_blackout_status()) {
            // Enable blackout reset mode
            DEBUGOUT("Blackout mode is enabled. It may pull down the RESET pin, and the current "
                     "battery level is %.2f%% \r\n\n",
                     vbat_percentage);
          } else {
            DEBUGOUT(" Your Vbatt status is less than the threshold voltage i.e "
                     "%.2fV battery Percentage is -- %.3f%% \r\n\n",
                     vbatt,
                     vbat_percentage);
          }
          // Clear the BOD interrupt flag
          sl_bod_inter_flag = 0;
        } else {
          DEBUGOUT(" Battery Percentage read failed \r\n");
          break;
        }
      } else {
        DEBUGOUT(" \r\nBattery read fail\r\n");
        break; // Battery Status read failed
      }
      sl_si91x_delay_ms(SL_BOD_500MS_DELAY);
    }
  } while (false);
}