/***************************************************************************/
/**
 * @file ulp_timer_example.c
 * @brief ULP Timer example
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licenser of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
/*==================================================================================
 * @brief : This file contains example application for ULP-Timer and LED0 toggle
 * @section Description :
 * This example shows how to use ULP timer to toggle LED at 1sec periodic rate,
   configured with default parameters.
 * Timer is configured to generate interrupts upon expiration and toggles LED.
 * After toggling LED for five times, timer stops and configured with new
 * parameters and toggles LED again five times.
 * After that timer is stopped and and switches the power state from High power
 to
 * Ultra low power and configured with new parameters and toggles LED again five
 times.
 * similarly timer is stopped and switches the power state from Ultra Low Power
 to
 * High power and configured with new parameters and toggles LED again five
 times.
 * At the end of this example "Unregistered timer timeout callback, on timer
 * operation completion"
 ===================================================================================*/
#include "ulp_timer_example.h"

#include "rsi_debug.h"
#include "sl_si91x_led.h"
#include "sl_si91x_power_manager.h"
#include "sl_si91x_ulp_timer.h"
#include "sl_ulp_timer_instances.h"
#include "sl_si91x_ulp_timer_common_config.h"
#include <stdint.h>
#include "sl_si91x_wireless_shutdown.h"
#include <stdint.h>

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define TIME_IN_MICROSECONDS 1000000 // timer timeout for 1 second in microseconds

/*****************************************************************************
 * ULP Timer-match value calculation :
 * For one microsecond type, match-value = number of microseconds
 * For 256 microsecond type, match-value = (time in microseconds)/256
 * For normal up/down counter type, timer =  (number of clock cycles per
 *microseconds * time in microseconds)
 ******************************************************************************/
#define LED0                  0  // For On-board LED-0
#define ZERO_INTERRUPT_COUNT  0  // Count for zeroth timeout interrupt
#define FIFTH_INTERRUPT_COUNT 5  // Count for fifth timeout interrupt
#define TENTH_INTERRUPT_COUNT 10 // Count for tenth timeout interrupt

// Macros used to construct ulp-timer instance
#define ULP_TIMER_INSTANCE                   0 // timer instance used, pass selected timer instance number in place of '0'
#define _CONCAT_TWO_TOKENS(token_1, token_2) token_1##token_2
#define CONCAT_TWO_TOKENS(token_1, token_2)  _CONCAT_TWO_TOKENS(token_1, token_2)
#define SL_ULP_TIMER_HANDLE                  CONCAT_TWO_TOKENS(sl_timer_handle_timer, ULP_TIMER_INSTANCE)
#define SL_ULP_TIMER_CALLBACK                CONCAT_TWO_TOKENS(on_timeout_callback, ULP_TIMER_INSTANCE)

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void SL_ULP_TIMER_CALLBACK(void);
static void configuring_ps2_power_state(void);
/*******************************************************************************
 **********************  Local variables   ***************************
 ******************************************************************************/
static bool state                          = false;
static bool timer_parameter_change_flag    = false;
static bool timer_callback_unregister_flag = false;
static uint32_t timer_count_value;
static uint32_t timer_direction;
sl_status_t status;
static uint8_t interrupt_count              = 0;
static sl_power_state_t current_power_state = SL_SI91X_POWER_MANAGER_PS4;
// Define a flag to track the power state transition
static bool ps4_to_ps2_transition_done = false;
static bool ps2_to_ps4_transition_done = false;
uint32_t match_value                   = 0;
/*******************************************************************************
 ******************************  Data Types  ***********************************
 ******************************************************************************/
// Enum for different transmission scenarios
typedef enum {
  SL_ULP_TIMER_PROCESS_ACTION,         // ulp timer event triggering and operational
                                       // transforming
  SL_ULP_TIMER_POWER_STATE_TRANSITION, // ulp timer power transition mode
  SL_ULP_TIMER_TRANSMISSION_COMPLETED, // ulp timer transmission completed mode
} ulp_timer_enum_t;

static ulp_timer_enum_t ulp_timer_current_mode = SL_ULP_TIMER_PROCESS_ACTION;

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
void ulp_timer_example_init(void)
{
  do {
    // Timer Peripheral input clock source configuration, common for all
    // ulp-timer instances
    status = sl_si91x_ulp_timer_init(&sl_timer_clk_handle);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_ulp_timer_init : Invalid Parameters, Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Successfully Configured ULP-timer clock input source \n");
    if (current_power_state == SL_SI91X_POWER_MANAGER_PS4) {
      DEBUGOUT("Timer executing at PS4 state \n");
      // Get match value
      status = sl_si91x_ulp_timer_get_match_value(SL_ULP_TIMER_HANDLE.timer_type, TIME_IN_MICROSECONDS, &match_value);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_ulp_timer_get_match_value : Invalid Parameters Error "
                 "Code : %lu \n",
                 status);
        break;
      } else {
        DEBUGOUT("Successfully match value is fetched\n");
      }
      // Updating timer match-value
      SL_ULP_TIMER_HANDLE.timer_match_value = match_value;
      // Configuring timer instance parameters: mode-periodic, type-1us,
      // match-value: 1second
      status = sl_si91x_ulp_timer_set_configuration(&(SL_ULP_TIMER_HANDLE));
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_ulp_timer_set_configuration : Invalid Parameters "
                 "Error Code : %lu \n",
                 status);
        break;
      }
      DEBUGOUT("Successfully Configured ULP-timer parameters with mhz "
               "parameters \n");
    } else if (current_power_state == SL_SI91X_POWER_MANAGER_PS2) {
      status = sl_si91x_ulp_timer_get_match_value(SL_ULP_TIMER_HANDLE.timer_type, TIME_IN_MICROSECONDS, &match_value);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_ulp_timer_get_match_value : Invalid Parameters Error "
                 "Code : %lu \n",
                 status);
        break;
      } else {
        DEBUGOUT("Successfully match value is fetched\n");
      }
      // Updating timer match-value
      SL_ULP_TIMER_HANDLE.timer_match_value = match_value;
      // Configuring timer instance parameters: mode-periodic, type-1us,
      // match-value: 1second
      status = sl_si91x_ulp_timer_set_configuration(&(SL_ULP_TIMER_HANDLE));
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_ulp_timer_set_configuration : Invalid Parameters "
                 "Error Code : %lu \n",
                 status);
        break;
      }
      DEBUGOUT("Successfully Configured ULP-timer parameters with 20mhz "
               "parameters \n");
    }
    // Registering timeout callback for the selected timer instance, which will
    // also enable its interrupt
    status = sl_si91x_ulp_timer_register_timeout_callback(ULP_TIMER_INSTANCE, &(SL_ULP_TIMER_CALLBACK));
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_ulp_timer_timeout_callback_register : Invalid "
               "Parameters Error Code : %lu \n",
               status);
      break;
    }
    DEBUGOUT("Successfully Registered timer instance timeout callback \n");
    // Starting Timer instance with default parameters
    status = sl_si91x_ulp_timer_start(ULP_TIMER_INSTANCE);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_ulp_timer_start : Invalid Parameters Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Successfully started ulp-timer instance with default parameters \n");
  } while (false);
}

/*******************************************************************************
 * Function will run continuously and will wait for trigger
 ******************************************************************************/
void ulp_timer_example_process_action(void)
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
  switch (ulp_timer_current_mode) {
    case SL_ULP_TIMER_PROCESS_ACTION:
      // Checking timer callback unregister flag value
      if (timer_callback_unregister_flag) {
        ulp_timer_current_mode = SL_ULP_TIMER_POWER_STATE_TRANSITION;
      }
      // Checking parameter-change flag, for changing timer-instance default
      // parameters to new parameters
      if (timer_parameter_change_flag) {
        // Resetting parameter-change flag
        timer_parameter_change_flag = false;
        if (current_power_state == SL_SI91X_POWER_MANAGER_PS4) {
          // Changing timer-type to 256US mode
          status = sl_si91x_ulp_timer_set_type(ULP_TIMER_INSTANCE, ULP_TIMER_TYP_256US);
          if (status != SL_STATUS_OK) {
            DEBUGOUT("sl_si91x_ulp_timer_set_type : Invalid Parameters Error "
                     "Code : %lu \n",
                     status);
          } else {
            DEBUGOUT("Successfully changed timer-type to 256US mode \n");
          }
          // Resetting timer direction
          status = sl_si91x_ulp_timer_set_direction(ULP_TIMER_INSTANCE, DOWN_COUNTER);
          if (status != SL_STATUS_OK) {
            DEBUGOUT("sl_si91x_ulp_timer_set_direction : Invalid Parameters "
                     "Error Code : %lu \n",
                     status);
          } else {
            DEBUGOUT("Successfully changed timer-direction to up-counter \n");
          }
          // Get match value
          status = sl_si91x_ulp_timer_get_match_value(ULP_TIMER_TYP_256US, TIME_IN_MICROSECONDS, &match_value);
          if (status != SL_STATUS_OK) {
            DEBUGOUT("sl_si91x_ulp_timer_get_match_value : Invalid Parameters Error "
                     "Code : %lu \n",
                     status);
          } else {
            DEBUGOUT("Successfully match value is fetched\n");
          }
          // Changing timer-instance match value to number of ticks required for
          // 1sec time-out, in 256us mode ( 1000000/256)
          status = sl_si91x_ulp_timer_set_count(ULP_TIMER_INSTANCE, match_value);
          if (status != SL_STATUS_OK) {
            DEBUGOUT("sl_si91x_ulp_timer_set_count : Invalid Parameters Error "
                     "Code : %lu \n",
                     status);
          } else {
            DEBUGOUT("Successfully changed the timer match value\n");
          }
        } else if (current_power_state == SL_SI91X_POWER_MANAGER_PS2) {
          // Changing timer-type to 256US mode
          status = sl_si91x_ulp_timer_set_type(ULP_TIMER_INSTANCE, ULP_TIMER_TYP_256US);
          if (status != SL_STATUS_OK) {
            DEBUGOUT("sl_si91x_ulp_timer_set_type : Invalid Parameters Error "
                     "Code : %lu \n",
                     status);
          } else {
            DEBUGOUT("Successfully changed timer-type to 256US mode \n");
          }
          // Resetting timer direction
          status = sl_si91x_ulp_timer_set_direction(ULP_TIMER_INSTANCE, DOWN_COUNTER);
          if (status != SL_STATUS_OK) {
            DEBUGOUT("sl_si91x_ulp_timer_set_direction : Invalid Parameters "
                     "Error Code : %lu \n",
                     status);
          } else {
            DEBUGOUT("Successfully changed timer-direction to up-counter \n");
          }
          // Get match value
          status = sl_si91x_ulp_timer_get_match_value(ULP_TIMER_TYP_256US, TIME_IN_MICROSECONDS, &match_value);
          if (status != SL_STATUS_OK) {
            DEBUGOUT("sl_si91x_ulp_timer_get_match_value : Invalid Parameters Error "
                     "Code : %lu \n",
                     status);
          } else {
            DEBUGOUT("Successfully match value is fetched\n");
          }
          // Changing timer-instance match value to number of ticks required for
          // 1sec time-out, in 256us mode ( 1000000/256)
          status = sl_si91x_ulp_timer_set_count(ULP_TIMER_INSTANCE, match_value);
          if (status != SL_STATUS_OK) {
            DEBUGOUT("sl_si91x_ulp_timer_set_count : Invalid Parameters Error "
                     "Code : %lu \n",
                     status);
          } else {
            DEBUGOUT("Successfully changed the timer match value\n");
          }
        }
        // Starting the timer instance in one-shot mode with 256us type and
        // 1-second time-out value
        status = sl_si91x_ulp_timer_start(ULP_TIMER_INSTANCE);
        if (status != SL_STATUS_OK) {
          DEBUGOUT("sl_si91x_ulp_timer_start : Invalid Parameters Error Code : %lu \n", status);
        } else {
          DEBUGOUT("Successfully started timer instance with new parameters \n");
        }
        // Reading timer count after starting timer instance
        status = sl_si91x_ulp_timer_get_count(ULP_TIMER_INSTANCE, &timer_count_value);
        if (status != SL_STATUS_OK) {
          DEBUGOUT("sl_si91x_ulp_timer_get_count : Invalid Parameters Error Code "
                   ": %lu \n",
                   status);
        } else {
          DEBUGOUT("Timer count value = %lu, after starting timer\n", timer_count_value);
        }
        // Checking counter direction
        status = sl_si91x_ulp_timer_get_direction(ULP_TIMER_INSTANCE, &timer_direction);
        if (status != SL_STATUS_OK) {
          DEBUGOUT("sl_si91x_ulp_timer_get_direction : Invalid Parameters Error Code "
                   ": %lu \n",
                   status);
        } else {
          if (timer_direction) {
            DEBUGOUT("Timer running as up-counter\n");
          } else {
            DEBUGOUT("Timer running as down-counter\n");
          }
        }
      }
      break;
    case SL_ULP_TIMER_POWER_STATE_TRANSITION:
      if (interrupt_count == TENTH_INTERRUPT_COUNT) {
        // Resetting interrupt to count five.
        interrupt_count = FIFTH_INTERRUPT_COUNT;
        // Setting parameter-change flag to true, so that APIs to change timer
        // parameter will get called
        timer_parameter_change_flag = true;
        if ((ps4_to_ps2_transition_done && ps2_to_ps4_transition_done)) {
          sl_si91x_ulp_timer_deinit();
          DEBUGOUT("De Initialization is successful\n");
          ps2_to_ps4_transition_done = false;
          ps4_to_ps2_transition_done = false;
          ulp_timer_current_mode     = SL_ULP_TIMER_TRANSMISSION_COMPLETED;
        }
        // Switching power state from PS4 to PS2 or vice versa
        else if (current_power_state == SL_SI91X_POWER_MANAGER_PS4) {
          DEBUGOUT("Switching power state from PS4 to PS2\n");
          // Control power management by adjusting clock references and shutting down
          // the power supply
          // This function is for demonstration purpose only. For more details, refer to the README file.
          sl_si91x_wireless_shutdown();
          // switching the power state PS4 to PS2 mode.
          status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS2);
          /* Due to calling trim_efuse API om power manager it will change the
           clock frequency, if we are not initialize the debug again it will
           print the garbage data or no data in console output. */
          DEBUGINIT();
          if (status != SL_STATUS_OK) {
            DEBUGOUT("sl_si91x_power_manager_add_ps_requirement: Error Code : %lu \n", status);
            break;
          }
          // Configuring the ps2 power state by configuring
          // the ram retention and removing the unused peripherals
          configuring_ps2_power_state();
          // Update flag
          ps4_to_ps2_transition_done = true;
          // current power state is updated to PS2
          current_power_state    = SL_SI91X_POWER_MANAGER_PS2;
          ulp_timer_current_mode = SL_ULP_TIMER_PROCESS_ACTION;
        } else if (current_power_state == SL_SI91X_POWER_MANAGER_PS2) {
          DEBUGOUT("Switching power state from PS2 to PS4\n");
          // switching the power state from PS2 to PS4 mode
          status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS4);
          if (status != SL_STATUS_OK) {
            DEBUGOUT("sl_si91x_power_manager_add_ps_requirement: Error Code : %lu \n", status);
            break;
          }
          /* Due to calling trim_efuse API om power manager it will change the
           clock frequency, if we are not initialize the debug again it will
           print the garbage data or no data in console output. */
          DEBUGINIT();
          // Update flag
          ps2_to_ps4_transition_done = true;
          // current power state is updated to PS4
          current_power_state    = SL_SI91X_POWER_MANAGER_PS4;
          ulp_timer_current_mode = SL_ULP_TIMER_PROCESS_ACTION;
        }
      }
      break;
    case SL_ULP_TIMER_TRANSMISSION_COMPLETED:
      current_power_state = LAST_ENUM_POWER_STATE;
      break;
    default:
      break;
  }
}

/*******************************************************************************
 * Callback function of ulp-timer instance
 *
 * @param none
 * @return none
 ******************************************************************************/
// Note : Remove all debugouts for exact delays
static void SL_ULP_TIMER_CALLBACK(void)
{
  DEBUGOUT("On timer%d callback \n", ULP_TIMER_INSTANCE);
  // Incrementing interrupt count
  interrupt_count++;
  // To toggle LED0
  state = !state;
  sl_si91x_led_toggle(RTE_LED0_PIN);
  // Checking interrupt count
  if (interrupt_count == FIFTH_INTERRUPT_COUNT) {
    // Stopping the timer instance, after five LED toggles.
    status = sl_si91x_ulp_timer_stop(ULP_TIMER_INSTANCE);
    if (status != SL_STATUS_OK) {
      return;
    }
    // Setting parameter-change flag to true, so that APIs to change timer
    // parameter will get called
    timer_parameter_change_flag = true;
  }
  // Checking interrupt count
  if (interrupt_count == TENTH_INTERRUPT_COUNT) {
    // Setting unregister flag true, so that after timer operation
    // callback-unregister API should get called
    timer_callback_unregister_flag = true;
    // Stopping the timer instance, after five more LED toggles.
    status = sl_si91x_ulp_timer_stop(ULP_TIMER_INSTANCE);
    if (status != SL_STATUS_OK) {
      return;
    }
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
  // Clear the peripheral configuration
  peri.m4ss_peripheral = 0;
  // Ored value for ulpss peripheral.
  peri.ulpss_peripheral = SL_SI91X_POWER_MANAGER_ULPSS_PG_AUX | SL_SI91X_POWER_MANAGER_ULPSS_PG_SSI
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_I2S | SL_SI91X_POWER_MANAGER_ULPSS_PG_I2C
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_IR | SL_SI91X_POWER_MANAGER_ULPSS_PG_FIM;
  // Ored value for npss peripheral.
  peri.npss_peripheral = SL_SI91X_POWER_MANAGER_NPSS_PG_MCUWDT | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUPS
                         | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUTS | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE2
                         | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE3;
  // Configure RAM banks for retention during power management
  config.configure_ram_banks = true; // Enable RAM bank configuration
  config.m4ss_ram_banks      = SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_8 | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_9
                          | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_10; // Specify the RAM banks to be
                                                                     // retained during power
                                                                     // management
  config.ulpss_ram_banks = SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_2 | SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_3;
  // Ored value for ulpss peripheral.
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
