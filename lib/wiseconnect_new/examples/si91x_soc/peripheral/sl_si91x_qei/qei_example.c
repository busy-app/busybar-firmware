/*******************************************************************************
* @file qei_example.c
* @brief Quadrature Encoder Interface (QEI) example
*******************************************************************************
* # License
* <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************

* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "sl_si91x_qei.h"
#include "qei_example.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_gpio_board.h"
#include "rsi_debug.h"
#include "sl_si91x_qei_config.h"
#include "sl_si91x_ulp_timer.h"
#include "sl_ulp_timer_instances.h"
#include "sl_si91x_ulp_timer_common_config.h"
/*******************************************************************************
 ***************************  Defines / Macros  *****************************
 ******************************************************************************/
#define BLINK_RATE           1000     // 1000 ticks per second
#define QEI_OPERATING_FREQ   30000000 // 30MHz is QEI module frequency
#define QEI_IDX_REVOLUTIONS  10       // Number of index revolutions for simulation
#define SL_TIMER_MATCH_VALUE 5000     // Timer match value for down-counter type
#define ULP_TIMER_INSTANCE   0        // ULP Timer Instance
/*******************************************************************************
 **********************  GLOBAL variables   ************************************
 ******************************************************************************/
/*******************************************************************************
 **********************  Local Function prototypes   **************************
 ******************************************************************************/
static void qei_emulate_phase_and_index_signals(void);
static void qei_stimulus_pin_mux_init(void);
#if defined(VELOCITY) && (VELOCITY == 1)
static void qei_callback(void *callback_flag);
static volatile uint32_t velocity = 0;
boolean_t qei_callback_triggered  = false;
#endif
static void ulp_timer_callback(void);
/*******************************************************************************
 **********************  Local variables   ************************************
 ******************************************************************************/
static sl_si91x_gpio_pin_config_t sl_gpio_pin29_config = { { SL_SI91X_GPIO_29_PORT, SL_SI91X_GPIO_29_PIN },
                                                           GPIO_OUTPUT };
static sl_si91x_gpio_pin_config_t sl_gpio_pin30_config = { { SL_SI91X_GPIO_30_PORT, SL_SI91X_GPIO_30_PIN },
                                                           GPIO_OUTPUT };
static sl_si91x_gpio_pin_config_t sl_gpio_pin6_config  = { { SL_SI91X_GPIO_6_PORT, SL_SI91X_GPIO_6_PIN }, GPIO_OUTPUT };
/*******************************************************************************
**************************   GLOBAL FUNCTIONS   *******************************
******************************************************************************/
/*******************************************************************************
 * Initialize GPIO pins for QEI stimulus signals. Configures GPIO pins for phase A,
 *  phase B, and index signals.
 ******************************************************************************/
static void qei_stimulus_pin_mux_init(void)
{
  sl_gpio_set_configuration(sl_gpio_pin29_config);
  sl_gpio_set_configuration(sl_gpio_pin30_config);
  sl_gpio_set_configuration(sl_gpio_pin6_config);

  sl_gpio_driver_clear_pin(&sl_gpio_pin29_config.port_pin);
  sl_gpio_driver_set_pin(&sl_gpio_pin30_config.port_pin);
  sl_gpio_driver_clear_pin(&sl_gpio_pin6_config.port_pin);
}

/*******************************************************************************
 * Emulate phase and index signals for QEI.Toggles GPIO pins to simulate the QEI behavior.
 * These are kind of dummy pulses generated which are fed to driver pins.
 ******************************************************************************/
static void qei_emulate_phase_and_index_signals(void)
{
  static volatile uint8_t x = 0, y = 0, index_count = 0;
  if (y == 0) {
    y = 1;
    if (x == 0) {
      x = 1;
      sl_gpio_driver_set_pin(&sl_gpio_pin29_config.port_pin);
    } else {
      x = 0;
      sl_gpio_driver_clear_pin(&sl_gpio_pin29_config.port_pin);
    }
  } else {
    y = 0;
    if (x == 0) {
      x = 0;
      index_count++;
      sl_gpio_driver_clear_pin(&sl_gpio_pin30_config.port_pin);
      if (index_count > QEI_IDX_REVOLUTIONS) {
        index_count = 0;
        sl_gpio_driver_set_pin(&sl_gpio_pin6_config.port_pin);
      }
    } else {
      x = 1;
      sl_gpio_driver_set_pin(&sl_gpio_pin30_config.port_pin);
      sl_gpio_driver_clear_pin(&sl_gpio_pin6_config.port_pin);
    }
  }
}

/*******************************************************************************
 * QEI Example Initialization function
 ******************************************************************************/
void qei_example_init(void)
{
  sl_status_t status;
  sl_qei_version_t version;

  // Get QEI version
  version = sl_si91x_qei_get_version();
  DEBUGOUT("QEI version is fetched successfully \n");
  DEBUGOUT("API version is %d.%d.%d\n", version.release, version.major, version.minor);
  do {
    // Initialize QEI stimulus pin mux for QEI
    qei_stimulus_pin_mux_init();
    status = sl_si91x_ulp_timer_init(&sl_timer_clk_handle);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_ulp_timer_init : Invalid Parameters, Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Successfully Configured ULP-timer clock input source \n");
    // Updating timer match-value
    sl_timer_handle_timer0.timer_match_value = SL_TIMER_MATCH_VALUE;
    status                                   = sl_si91x_ulp_timer_set_configuration(&(sl_timer_handle_timer0));
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_ulp_timer_set_configuration : Invalid Parameters "
               "Error Code : %lu \n",
               status);
      break;
    }
    DEBUGOUT("Successfully Configured ULP-timer parameters \n");
    status = sl_si91x_ulp_timer_register_timeout_callback(ULP_TIMER_INSTANCE, &(ulp_timer_callback));
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_ulp_timer_timeout_callback_register : Invalid "
               "Parameters Error Code : %lu \n",
               status);
      break;
    }
    DEBUGOUT("Successfully Registered timer instance timeout callback \n");
    // Initialize QEI and set parameters
    status = sl_si91x_qei_init(&sl_qei_init);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("Error in sl_si91x_qei_init, Error code: %lu", status);
      break;
    }
    DEBUGOUT("QEI initialized successfully \n");
    // Set QEI module frequency
    status = sl_si91x_qei_set_module_frequency(QEI_OPERATING_FREQ);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("Error in sl_si91x_qei_set_module_frequency, Error code: %lu", status);
      break;
    }
    DEBUGOUT("QEI successfully set module frequency \n");
#if defined(ENCODER_MODE) && (ENCODER_MODE == 1)
    sl_qei_config.config_param =
      (SL_QEI_NO_SWAP_AB << QEI_SWAP_PHASE_AB_MASK) | (SL_QEI_DIGITAL_FILTER << QEI_DIGITAL_FILTER_BYPASS_MASK)
      | (SL_QEI_POS_CNT_32 << QEI_POS_CNT_MODE_MASK) | (SL_QEI_ENCODER_MODE << QEI_TIMER_MODE_MASK);
    sl_qei_config.ctrl_param = SL_QEI_ENCODE_MODE_1X & QEI_ENCODING_MODE_MASK;
    // Set QEI configuration parameters
    status = sl_si91x_qei_set_configuration(SL_QEI_SET, &sl_qei_config);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("Error in sl_si91x_qei_set_configuration, Error code: %lu", status);
      break;
    }
    DEBUGOUT("QEI configuration parameters set successfully \n");
#endif
#if defined(VELOCITY) && (VELOCITY == 1)
    static void *qei_callback_flag;
    static sl_qei_intr_unmask_t qei_interrupt_flag;
    qei_interrupt_flag.qei_velocity_computation_over_intr_unmask = true;
    // Start the velocity counter for QEI
    status = sl_si91x_qei_start_velocity(SL_QEI_DELTA_TIME);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("Error in sl_si91x_qei_start_velocity, Error code: %lu", status);
      break;
    }
    DEBUGOUT("QEI velocity counter started successfully \n");
    // Register callback for QEI interrupts
    status = sl_si91x_qei_register_callback(qei_callback, qei_callback_flag, &qei_interrupt_flag);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("Error in sl_si91x_qei_register_callback, Error code: %lu", status);
      break;
    }
    DEBUGOUT("QEI interrupt callback registered successfully \n");
#endif
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
 * QEI example process action function
 ******************************************************************************/
void qei_example_process_action(void)
{

#if defined(ENCODER_MODE) && (ENCODER_MODE == 1)
  uint32_t qei_position, qei_index;
  sl_qei_direction_t qei_direction;

  qei_position = sl_si91x_qei_get_position_counter();
  DEBUGOUT("QEI Position:%ld\n", qei_position);
  // Get QEI index count
  qei_index = sl_si91x_qei_get_index_counter();
  DEBUGOUT("QEI Index:%ld\n", qei_index);
  // Get QEI direction
  qei_direction = sl_si91x_qei_get_direction();
  DEBUGOUT("QEI Direction:%d\n", qei_direction);
#endif
#if defined(VELOCITY) && (VELOCITY == 1)
  if (qei_callback_triggered) {
    DEBUGOUT("qei_velocity = %ld\r\n", velocity);
    qei_callback_triggered = false;
  }
#endif
}

/*******************************************************************************
 * QEI callback function to handle interrupts.
 ******************************************************************************/
#if defined(VELOCITY) && (VELOCITY == 1)
static void qei_callback(void *callback_flag)
{
  // Check if the interrupt flag matches
  if (*(uint32_t *)callback_flag == SL_QEI_VELOCITY_COMP_OVER_INTR_LVL) {
    velocity               = sl_si91x_qei_get_velocity();
    qei_callback_triggered = true;
  }
}
#endif

/*******************************************************************************
 * Callback function of ulp-timer instance
 ******************************************************************************/
static void ulp_timer_callback(void)
{
  qei_emulate_phase_and_index_signals();
}
