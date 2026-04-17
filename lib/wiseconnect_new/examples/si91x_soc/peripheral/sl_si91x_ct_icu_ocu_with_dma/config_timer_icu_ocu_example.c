/*******************************************************************************
 * @file config_timer_icu_ocu_with_dma example.c
 * @brief Config Timer Input Capture and Output Compare features with DMA example
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 ****************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

/*==============================================================================
 * @brief : This files contains example describing usage of Configurable Timer
            APIs.
 * @section Description :
 * CT DMA mode Use-case :
 * Here it will generate 2 PWM outputs, Counter-0 and Counter-1 will generate
 * PWM output with varied duty cycle.
 * CT input event mode Use-case :
 * It will capture the input event on GPIO pin and store the captured value in
 * capture_value variable.
 =============================================================================*/

/* Includes ------------------------------------------------------------------*/
#include "config_timer_icu_ocu_example.h"
#include "rsi_debug.h"
#include "sl_si91x_config_timer.h"
#include "sl_gpio_board.h"
/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define CT_COUNTER_USED     SL_COUNTER_0          // counter number used for normal counter application
#define CT_IC_INTERUPT_SEL  SL_EVENT0_RISING_EDGE //Input Capture Interrupt select
#define DIV_FACTOR          2                     // Duty cycle factor for counter 1 in PWM mode
#define TIME_PERIOD_VALUE   1000                  // Time period in microseconds
#define STEP_SIZE_COUNTER_0 400                   // Step size for counter0 increments
#define ARRAY_SIZE          100                   // Array size for DMA mode

#define SET   1 // for setting any value high
#define CLEAR 0 // for clearing any value
/*******************************************************************************
 **********************  GLOBAL variables   ***************************
 ******************************************************************************/

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void on_config_timer_callback(void *callback_flag);
/*******************************************************************************
 **********************  Local variables   ***************************
 ******************************************************************************/
static sl_status_t status;
static void *callback_flag_data;
#if (CT_COUNTER_DMA_MODE_USECASE == SET)
uint32_t ct0_compare_values[ARRAY_SIZE] = { 0 }; // Array to store counter0 values in DMA mode
volatile boolean_t interrupt_dma_flag   = 0;
uint8_t counter1_flag                   = 0;
#endif
#if (CT_COUNTER_INPUT_EVENT_USECASE == SET)
uint16_t capture_value              = 0;
uint32_t event_interrupt_flag_value = 0;
boolean_t capture_flag              = 0;
#endif
static sl_config_timer_interrupt_flags_t ct_interrupt_flags;
/*******************************************************************************
**************************   GLOBAL FUNCTIONS   *******************************
******************************************************************************/
/*******************************************************************************
 * Config-Timer-ICU-OCU-With-DMA example initialization function
 ******************************************************************************/
void config_timer_icu_ocu_example_init(void)
{
  sl_config_timer_version_t version;
  sl_config_timer_config_t ct_config;
  static uint32_t match_value, duty_cycle_value;

  // Initializing ct configuration structure
  ct_config.is_counter_mode_32bit_enabled = SL_COUNTER_16BIT;

  ct_config.counter0_direction               = SL_COUNTER0_UP;
  ct_config.is_counter0_periodic_enabled     = true;
  ct_config.is_counter0_sync_trigger_enabled = true;

  ct_config.counter1_direction               = SL_COUNTER0_UP;
  ct_config.is_counter1_periodic_enabled     = true;
  ct_config.is_counter1_sync_trigger_enabled = true;

  // This macro enables input event capture mode.
#if (CT_COUNTER_INPUT_EVENT_USECASE == SET)
  // Enable this for counter0 to get increment/decrement, counter should be active and hit with selected event.
  ct_config.is_counter0_trigger_enabled = true;
  // Enable this for counter1 to get increment/decrement, counter should be active and hit with selected event.
  ct_config.is_counter1_trigger_enabled = true;
#endif
  // This macro enables DMA mode for varied PWM duty cycle.
#if (CT_COUNTER_DMA_MODE_USECASE == SET)
  for (uint32_t i = 0; i < ARRAY_SIZE; i++) {
    ct0_compare_values[i] = (i + 1) * STEP_SIZE_COUNTER_0; // Populate array with increments of STEP_SIZE_COUNTER_0
  }
#endif
  //Version information of config timer
  version = sl_si91x_config_timer_get_version();
  DEBUGOUT("\r\n API version is %d.%d.%d\r\n", version.release, version.major, version.minor);
  do {
    // Initializing CT
    sl_si91x_config_timer_init();
    DEBUGOUT("\r\n CT initialized successfully \r\n");
    // Configuring CT parameters from UC values
    status = sl_si91x_config_timer_set_configuration(&ct_config);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_set_configuration, Error code: %lu\r\n", status);
      break;
    }
    // Get the match value of the timer
    status = sl_si91x_config_timer_get_match_value(TIME_PERIOD_VALUE, &match_value);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_get_match_value, Error code: %lu \r\n", status);
      break;
    }
    DEBUGOUT("\r\n CT match value get is successful \r\n");
#if (CT_COUNTER_INPUT_EVENT_USECASE == SET && CT_COUNTER_DMA_MODE_USECASE == SET)
    DEBUGOUT("\r\n Set Any One Usecase \r\n");
    break;
#endif

#if (CT_COUNTER_INPUT_EVENT_USECASE == SET)
    // Setting match value
    status = sl_si91x_config_timer_set_match_count(SL_COUNTER_16BIT, CT_COUNTER_USED, match_value);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_set_match_count, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("\r\n CT Match Count is set successfully\r\n");
    status = sl_si91x_config_timer_select_action_event(START, SL_EVENT0_LEVEL0, SL_NO_EVENT);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_select_action_event for Start Event, Error code: %lu\r\n", status);
      break;
    }
    status = sl_si91x_config_timer_select_action_event(CAPTURE, CT_IC_INTERUPT_SEL, SL_NO_EVENT);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_select_action_event for Capture Event, Error code: %lu\r\n", status);
      break;
    }
    status = sl_si91x_config_timer_select_action_event(INTERRUPT, CT_IC_INTERUPT_SEL, SL_NO_EVENT);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_select_action_event for Interrupt Event, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("\r\n Config timer action event capture done successfully \r\n");
    if (CT_COUNTER_USED == SL_COUNTER_0) {
      ct_interrupt_flags.is_counter0_event_interrupt_enabled = true;
    } else {
      ct_interrupt_flags.is_counter1_event_interrupt_enabled = true;
    }
    // Registering callback
    status = sl_si91x_config_timer_register_callback(on_config_timer_callback, callback_flag_data, &ct_interrupt_flags);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_register_callback, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("\r\n CT callback registered successfully \r\n");

    (void)duty_cycle_value;
#endif
#if (CT_COUNTER_DMA_MODE_USECASE == SET)
    // Setting match value
    status = sl_si91x_config_timer_set_match_count(SL_COUNTER_16BIT, SL_COUNTER_0, match_value);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_set_match_count, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("\r\n CT Match Count is set successfully\r\n");
    // Setting match value in DMA mode
    status = sl_si91x_config_timer_set_match_count(SL_COUNTER_16BIT, SL_COUNTER_1, match_value);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_set_match_count, Error code: %lu\r\n", status);
      break;
    }
    sl_config_timer_ocu_config_t ct_ocu_config = { 0 };
    // Initializing CT OCU configuration structure
    ct_ocu_config.is_counter0_ocu_output_enabled         = true;
    ct_ocu_config.is_counter0_ocu_dma_enabled            = true;
    ct_ocu_config.is_counter0_toggle_output_high_enabled = true;
    ct_ocu_config.is_counter0_toggle_output_low_enabled  = true;
    ct_ocu_config.is_counter1_ocu_output_enabled         = true;
    ct_ocu_config.is_counter1_ocu_dma_enabled            = true;
    ct_ocu_config.is_counter1_toggle_output_high_enabled = true;
    ct_ocu_config.is_counter1_toggle_output_low_enabled  = true;
    // Setting OCU configurations
    status = sl_si91x_config_timer_set_ocu_configuration(&ct_ocu_config);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_set_ocu_configuration, Error code: %lu\r\n", status);
      break;
    }
    // Enabling interrupt at match value for counter-used
    ct_interrupt_flags.is_counter0_hit_peak_interrupt_enabled = true;
    ct_interrupt_flags.is_counter1_hit_peak_interrupt_enabled = true;
    // Un-registering callback
    status = sl_si91x_config_timer_unregister_callback(&ct_interrupt_flags);
    // Registering callback
    status = sl_si91x_config_timer_register_callback(on_config_timer_callback, callback_flag_data, &ct_interrupt_flags);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_register_callback, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("\r\n CT callback registered successfully \r\n");

    // DMA configure for CT counter-0
    status = sl_si91x_config_timer_set_dma_configuration(ct0_compare_values, CT_DMA_CHANNEL_0);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_dma_configure, Error code: %lu\r\n", status);
      break;
    }
    // DMA transfer for CT counter-0
    status = sl_si91x_config_timer_dma_transfer(ct0_compare_values, CT_DMA_CHANNEL_0, &ct_dma_transfer_channel_0);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_dma_transfer, Error code: %lu\r\n", status);
      break;
    }
    // Starting CT counter0 on software trigger
    status = sl_si91x_config_timer_start_on_software_trigger(SL_COUNTER_0);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_start_on_software_trigger, Error code: %lu\r\n", status);
      break;
    }

    duty_cycle_value = match_value / DIV_FACTOR;
    // DMA configure for CT counter-1
    status = sl_si91x_config_timer_set_dma_configuration(&duty_cycle_value, CT_DMA_CHANNEL_8);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_dma_configure, Error code: %lu\r\n", status);
      break;
    }
    // DMA transfer for CT counter-1
    status = sl_si91x_config_timer_dma_transfer(&duty_cycle_value, CT_DMA_CHANNEL_8, &ct_dma_transfer_channel_1);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_dma_transfer, Error code: %lu\r\n", status);
      break;
    }
    // Starting CT counter1 on software trigger
    status = sl_si91x_config_timer_start_on_software_trigger(SL_COUNTER_1);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_start_on_software_trigger, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("\r\n CT started successfully on software trigger \r\n");
#endif
  } while (false);
}

/*******************************************************************************
 * Config-Timer-ICU-OCU-With-DMA example process action function
 ******************************************************************************/
void config_timer_icu_ocu_example_process_action(void)
{
#if (CT_COUNTER_INPUT_EVENT_USECASE == SET)
  if (capture_flag == 1) {
    status = sl_si91x_config_timer_read_capture(CT_COUNTER_USED, &capture_value);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_read_capture, Error code: %lu \r\n", status);
    }
    DEBUGOUT("\r\n capture_value:%d\r\n", capture_value);
  }
  capture_flag = 0;
#endif
#if (CT_COUNTER_DMA_MODE_USECASE == SET)
  do {
    if (interrupt_dma_flag == 1) {
      interrupt_dma_flag = 0;
      if (ct_dma_transfer_flag_channel_0) {
        ct_dma_transfer_flag_channel_0 = 0;
        status = sl_si91x_config_timer_dma_transfer(ct0_compare_values, CT_DMA_CHANNEL_0, &ct_dma_transfer_channel_0);
        if (status != SL_STATUS_OK) {
          DEBUGOUT("\r\n sl_si91x_config_timer_dma_transfer, Error code: %lu\r\n", status);
          break;
        }
      }
    }
  } while (false);
  if (ct_dma_transfer_index_channel_0 == ARRAY_SIZE) {
    ct_dma_transfer_index_channel_0 = 0;
  }
#endif
}

/*******************************************************************************
 * @brief         Config-Timer callback function
 * @return        none
 ******************************************************************************/
void on_config_timer_callback(void *callback_flag)
{
  (void)callback_flag;
#if (CT_COUNTER_DMA_MODE_USECASE == SET)
  interrupt_dma_flag = 1;
#endif

#if (CT_COUNTER_INPUT_EVENT_USECASE == SET)
  // Updating expected interrupt flag value as per enabled interrupt and counter used
  if (CT_COUNTER_USED == SL_COUNTER_0) {
    event_interrupt_flag_value = SL_CT_EVENT_INTR_0_FLAG;
  } else {
    event_interrupt_flag_value = SL_CT_EVENT_INTR_1_FLAG;
  }
  // Checking interrupt flag value
  if (*(uint32_t *)callback_flag == event_interrupt_flag_value) {
    capture_flag = 1;
  }
#endif
}
