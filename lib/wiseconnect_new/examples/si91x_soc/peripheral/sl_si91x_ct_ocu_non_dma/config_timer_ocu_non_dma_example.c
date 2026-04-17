/***********************************************************************************
 * @file config_timer_ocu_non_dma_example.c
 * @brief Config Timer Basic features and Output Compare feature without DMA example
 ***********************************************************************************
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
 * Counter mode Use-case :
 * One as free running timer. The timer instance is configured to generate peak 
 * reached console message.
 * The counter-0 is set to expire at 1-millisecond (1KHz) intervals.
 * PWM output mode Use-case :
 * Here it will generate 2 PWM outputs, Counter-1 will generate square wave
 * output (50%-duty cycle) and counter-0 will generates PWM output whose duty
 * cycle continuously varies from 100% to 0% then 0% to 100%.
 =============================================================================*/

/* Includes ------------------------------------------------------------------*/
#include "config_timer_ocu_non_dma_example.h"
#include "rsi_debug.h"
#include "sl_si91x_config_timer.h"
/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define CT_COUNTER_USED   SL_COUNTER_0 // counter number used for normal counter application
#define DIV_FACTOR        2            // Duty cycle factor for counter 1 in PWM mode
#define INITIAL_VALUE     0            // for zero value
#define TIME_PERIOD_VALUE 1000         // Time period in microseconds

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

#if (CT_PWM_MODE_USECASE == SET)
volatile uint32_t pwm_flag = INITIAL_VALUE;
static sl_config_timer_ocu_control_t ocu_params0;
static sl_config_timer_ocu_control_t ocu_params1;
static sl_config_timer_ocu_params_t vsOCUparams = { INITIAL_VALUE };
sl_config_timer_pwm_callback_t pwm_callback;
#endif

static sl_config_timer_interrupt_flags_t ct_interrupt_flags;
static uint32_t match_value;

#if (CT_COUNTER_MODE_USECASE == SET)
volatile boolean_t interrupt_flag = 0;
#endif
/*******************************************************************************
**************************   GLOBAL FUNCTIONS   *******************************
******************************************************************************/
/*******************************************************************************
 * Config-Timer example initialization function
 ******************************************************************************/
void config_timer_example_init(void)
{
  sl_config_timer_version_t version;
  sl_config_timer_config_t ct_config;
  // Initializing ct configuration structure
  ct_config.is_counter_mode_32bit_enabled = SL_COUNTER_16BIT;

  ct_config.counter0_direction               = SL_COUNTER0_UP;
  ct_config.is_counter0_periodic_enabled     = true;
  ct_config.is_counter0_sync_trigger_enabled = true;
  ct_config.counter1_direction               = SL_COUNTER0_UP;
  ct_config.is_counter1_periodic_enabled     = true;
  ct_config.is_counter1_sync_trigger_enabled = true;

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
    DEBUGOUT("\r\n CT configuration is set successfully \r\n");
    // Get the match value of the timer
    status = sl_si91x_config_timer_get_match_value(TIME_PERIOD_VALUE, &match_value);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_get_match_value, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("\r\n CT match value get is successful \r\n");
    // Enabling interrupt at match value for counter-used

#if (CT_COUNTER_MODE_USECASE == SET && CT_PWM_MODE_USECASE == SET)
    DEBUGOUT("\r\n Set Any One Usecase \r\n");
    break;
#endif

    if (CT_COUNTER_USED == SL_COUNTER_0) {
      ct_interrupt_flags.is_counter0_hit_peak_interrupt_enabled = true;
    } else {
      ct_interrupt_flags.is_counter1_hit_peak_interrupt_enabled = true;
    }
    // Registering callback
    status = sl_si91x_config_timer_register_callback(on_config_timer_callback, callback_flag_data, &ct_interrupt_flags);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_register_callback, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("\r\n CT callback registered successfully \r\n");

#if (CT_COUNTER_MODE_USECASE == SET)
    // Setting match value
    status = sl_si91x_config_timer_set_match_count(SL_COUNTER_16BIT, CT_COUNTER_USED, match_value);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_set_match_count, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("\r\n CT Match Count is set successfully\r\n");
    // Starting CT_COUNTER_USED on software trigger
    status = sl_si91x_config_timer_start_on_software_trigger(CT_COUNTER_USED);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_start_on_software_trigger, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("\r\n CT started successfully on software trigger \r\n");
#endif

#if (CT_PWM_MODE_USECASE == SET)
    // Initializing ct OCU parameters structure with default values
    sl_config_timer_ocu_config_t ct_ocu_config = { 0 };
    // Initializing ct OCU configuration structure
    ct_ocu_config.is_counter0_ocu_output_enabled         = true;
    ct_ocu_config.is_counter1_ocu_output_enabled         = true;
    ct_ocu_config.is_counter0_ocu_sync_enabled           = true;
    ct_ocu_config.is_counter1_ocu_sync_enabled           = true;
    ct_ocu_config.is_counter0_toggle_output_high_enabled = true;
    ct_ocu_config.is_counter1_toggle_output_high_enabled = true;
    ct_ocu_config.is_counter0_toggle_output_low_enabled  = true;
    ct_ocu_config.is_counter1_toggle_output_low_enabled  = true;

    // Setting OCU configurations
    status = sl_si91x_config_timer_set_ocu_configuration(&ct_ocu_config);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_set_ocu_configuration, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("\r\n CT OCU configuration is set successfully \r\n");

    vsOCUparams.CompareVal1_0 = INITIAL_VALUE;
    vsOCUparams.CompareVal1_1 = (uint16_t)(match_value / DIV_FACTOR);

    ocu_params0.is_counter_number_1  = SL_COUNTER_0;
    ocu_params0.is_dma_state_enabled = false;
    ocu_params0.params               = &vsOCUparams;
    ocu_params0.callback             = &pwm_callback;
    status                           = sl_si91x_config_timer_set_ocu_control(&ocu_params0);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_set_ocu_compare_values, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("\r\n CT OCU Counter0 configuration is set successfully \r\n");
    ocu_params1.is_counter_number_1  = SL_COUNTER_1;
    ocu_params1.is_dma_state_enabled = false;
    ocu_params1.params               = &vsOCUparams;
    ocu_params1.callback             = &pwm_callback;
    status                           = sl_si91x_config_timer_set_ocu_control(&ocu_params1);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_set_ocu_compare_values, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("\r\n CT OCU Counter1 configuration is set successfully \r\n");
    // Setting match value for counter 0
    status = sl_si91x_config_timer_set_match_count(SL_COUNTER_16BIT, SL_COUNTER_0, match_value);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_set_match_count, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("\r\n Counter0 Match Count is set successfully \r\n");
    // Setting match value for counter 1
    status = sl_si91x_config_timer_set_match_count(SL_COUNTER_16BIT, SL_COUNTER_1, match_value);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_set_match_count, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("\r\n Counter1 Match Count is set successfully \r\n");
    // Starting CT counter0 on software trigger
    status = sl_si91x_config_timer_start_on_software_trigger(SL_COUNTER_0);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_start_on_software_trigger, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("\r\n CT Counter0 started successfully on software trigger \r\n");
    // Starting CT counter1 on software trigger
    status = sl_si91x_config_timer_start_on_software_trigger(SL_COUNTER_1);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n sl_si91x_config_timer_start_on_software_trigger, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("\r\n CT Counter1 started successfully on software trigger \r\n");
#endif
  } while (false);
}

/*******************************************************************************
 * Config-Timer example process action function
 ******************************************************************************/
void config_timer_example_process_action(void)
{
#if (CT_COUNTER_MODE_USECASE == SET)
  // interrupt_flag is set when interrupt count is greater than TENTH_INTERRUPT_COUNT
  if (interrupt_flag) {
    interrupt_flag = 0;
    DEBUGOUT("\r\n Config timer Counter Peak Reached \r\n");
    DEBUGOUT("\r\n Config timer unregistered & de-inits callback \r\n");
  }
#endif
#if (CT_PWM_MODE_USECASE == SET)
  static uint32_t oc_width = CLEAR, oc_dir = CLEAR;
  if (pwm_flag) {
    if (oc_width == match_value)
      oc_dir = SET;
    if (oc_width == CLEAR)
      oc_dir = CLEAR;

    if (oc_dir == CLEAR)
      oc_width++;
    if (oc_dir == SET)
      oc_width--;

    vsOCUparams.CompareVal1_0 = oc_width;
    ocu_params0.params        = &vsOCUparams;
    status                    = sl_si91x_config_timer_set_ocu_control(&ocu_params0);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\nsl_si91x_config_timer_set_ocu_compare_values, Error code: %lu\r\n", status);
    }
  }
  pwm_flag = INITIAL_VALUE;
#endif
}

/*******************************************************************************
 * @brief         Config-Timer callback function
 * @return        none
 ******************************************************************************/
void on_config_timer_callback(void *callback_flag)
{
#if (CT_PWM_MODE_USECASE == SET)
  if (*(uint32_t *)callback_flag == SL_CT_COUNTER_0_IS_PEAK_FLAG) {
    // incrementing delay variable on every interrupt
    pwm_flag++;
  }
#endif
#if (CT_COUNTER_MODE_USECASE == SET)
  uint32_t interrupt_flag_value = 0;
  // Updating expected interrupt flag value as per enabled interrupt and counter used
  if (CT_COUNTER_USED == SL_COUNTER_0) {
    interrupt_flag_value = SL_CT_COUNTER_0_IS_PEAK_FLAG;
  } else {
    interrupt_flag_value = SL_CT_COUNTER_1_IS_PEAK_FLAG;
  }
  if (*(uint32_t *)callback_flag == interrupt_flag_value) {
    interrupt_flag = 1;
    // De-initializing config-timer
    sl_si91x_config_timer_deinit();
  }
#endif
}
