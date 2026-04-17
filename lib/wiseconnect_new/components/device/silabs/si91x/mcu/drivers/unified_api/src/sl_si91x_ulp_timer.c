/******************************************************************************
* @file sl_si91x_ulp_timer.c
* @brief ULP TIMER API implementation
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
#include "sl_si91x_ulp_timer.h"
#include "rsi_timers.h"
#include "rsi_rom_timer.h"
#include "rsi_rom_clks.h"
#include "rsi_rom_ulpss_clk.h"
#include "clock_update.h"

/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/
#define MINIMUM_MATCH_COUNT     0u       // for validation of timer match count
#define CLOCKS_PER_SECOND_40MHZ 40000000 // clocks per second for 40MHz clock frequency
#define CLOCKS_PER_SECOND_32MHZ 32000000 // clocks per second for 32MHz clock frequency
#define CLOCKS_PER_SECOND_20MHZ 20000000 // clocks per second for 20mhz clock frequency
#define CLOCKS_PER_SECOND_32KHZ 32000    // clocks per second for 32khz clock frequency
#define FIRST_INDEX             0        // for start value of iteration variable
#define LAST_INDEX              4        // for last value of iteration variable

#define INTEGRAL_PART_40MHZ_1US (40 & 0xFFFF) // Integral part of timer clock cycles per us for 40 mhz clock source
#define INTEGRAL_PART_32MHZ_1US (32 & 0xFFFF) // Integral part of timer clock cycles per us for 32 mhz clock source
#define INTEGRAL_PART_32KHZ_1US (0 & 0xFFFF)  // Integral part of timer clock cycles per us for 32 khz clock source
#define INTEGRAL_PART_20MHZ_1US (20 & 0xFFFF) // Integral part of timer clock cycles per us for 20 mhz clock source

#define FRACTIONAL_PART_40MHZ_1US (0 & 0xFF) // Fractional part of timer clock cycles per us for 40 mhz clock source
#define FRACTIONAL_PART_32MHZ_1US (0 & 0xFF) // Fractional part of timer clock cycles per us for 32 mhz clock source
#define FRACTIONAL_PART_32KHZ_1US (8 & 0xFF) // Fractional part of timer clock cycles per us for 32 khz clock source
#define FRACTIONAL_PART_20MHZ_1US (0 & 0xFF) // Fractional part of timer clock cycles per us for 20 mhz clock source

#define INTEGRAL_PART_40MHZ_256US \
  (10240 & 0xFFFF) // Integral part of timer clock cycles per 256us for 40 mhz clock source
#define INTEGRAL_PART_32MHZ_256US \
  (8192 & 0xFFFF)                              // Integral part of timer clock cycles per 256us for 32 mhz clock source
#define INTEGRAL_PART_32KHZ_256US (8 & 0xFFFF) // Integral part of timer clock cycles per 256us for 32 khz clock source
#define INTEGRAL_PART_20MHZ_256US \
  (5120 & 0xFFFF) // Integral part of timer clock cycles per 256us for 20 mhz clock source

#define FRACTIONAL_PART_40MHZ_256US \
  (0 & 0xFF) // Fractional part of timer clock cycles per 256us for 40 mhz clock source
#define FRACTIONAL_PART_32MHZ_256US \
  (0 & 0xFF) // Fractional part of timer clock cycles per 256us for 32 mhz clock source
#define FRACTIONAL_PART_32KHZ_256US \
  (49 & 0xFF) // Fractional part of timer clock cycles per 256us for 32 khz clock source
#define FRACTIONAL_PART_20MHZ_256US \
  (0 & 0xFF) // Fractional part of timer clock cycles per 256us for 20 mhz clock source

#define ULP_TIMER0_IRQHandler IRQ002_Handler // ULP Timer0 IRQ Handler
#define ULP_TIMER1_IRQHandler IRQ003_Handler // ULP Timer1 IRQ Handler
#define ULP_TIMER2_IRQHandler IRQ004_Handler // ULP Timer2 IRQ Handler
#define ULP_TIMER3_IRQHandler IRQ005_Handler // ULP Timer3 IRQ Handler

#define ULP_TIMER_RELEASE_VERSION 0 // ULP_TIMER Release version
#define ULP_TIMER_SQA_VERSION     0 // ULP_TIMER SQA version
#define ULP_TIMER_DEV_VERSION     2 // ULP_TIMER Developer version

/*******************************************************************************
 ***************************  Local TYPES  ********************************
 ******************************************************************************/
typedef struct {
  uint32_t timer_clk_source_value;
  uint16_t clk_integral_part;
  uint8_t clk_fractional_part;
} clk_int_frac_values_t;

typedef IRQn_Type IRQn_Type_t; ///< Renaming Interrupt numbers type enum

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static ulp_timer_callback_t timeout_callback_function_pointers[] = { NULL, NULL, NULL, NULL };
static clk_int_frac_values_t int_frac_values_1US[]               = {
                { CLOCKS_PER_SECOND_40MHZ, INTEGRAL_PART_40MHZ_1US, FRACTIONAL_PART_40MHZ_1US },
                { CLOCKS_PER_SECOND_32MHZ, INTEGRAL_PART_32MHZ_1US, FRACTIONAL_PART_32MHZ_1US },
                { CLOCKS_PER_SECOND_20MHZ, INTEGRAL_PART_20MHZ_1US, FRACTIONAL_PART_20MHZ_1US },
                { CLOCKS_PER_SECOND_32KHZ, INTEGRAL_PART_32KHZ_1US, FRACTIONAL_PART_32KHZ_1US }
};
static clk_int_frac_values_t int_frac_values_256US[] = {
  { CLOCKS_PER_SECOND_40MHZ, INTEGRAL_PART_40MHZ_256US, FRACTIONAL_PART_40MHZ_256US },
  { CLOCKS_PER_SECOND_32MHZ, INTEGRAL_PART_32MHZ_256US, FRACTIONAL_PART_32MHZ_256US },
  { CLOCKS_PER_SECOND_20MHZ, INTEGRAL_PART_20MHZ_256US, FRACTIONAL_PART_20MHZ_256US },
  { CLOCKS_PER_SECOND_32KHZ, INTEGRAL_PART_32KHZ_256US, FRACTIONAL_PART_32KHZ_256US }
};
static IRQn_Type_t ulp_timer_irq_numbers[] = { TIMER0_IRQn, TIMER1_IRQn, TIMER2_IRQn, TIMER3_IRQn };

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
static sl_status_t ulp_timer_get_interrupt_status(ulp_timer_instance_t timer_num, uint8_t *int_status);
static sl_status_t ulp_timer_clear_interrupt(ulp_timer_instance_t timer_num);
static sl_status_t ulp_timer_enable_interrupt(ulp_timer_instance_t timer_num);
static sl_status_t ulp_timer_disable_interrupt(ulp_timer_instance_t timer_num);
static sl_status_t get_int_frac_parts(ulp_timer_type_t timer_type, uint16_t *integral_part, uint8_t *fractional_part);
/*******************************************************************************
 **********************  Local Function Definition****************************
 ******************************************************************************/

/*******************************************************************************
 ***********************  Global function Prototypes *************************
 ******************************************************************************/
void IRQ002_Handler(void);
void IRQ003_Handler(void);
void IRQ004_Handler(void);
void IRQ005_Handler(void);
/*******************************************************************************
 ***********************  Global function Definitions *************************
 ******************************************************************************/

/*******************************************************************************
* @brief: Configures ulp- timer input clock source

* @details:
* Enables ulp-timer clock source, should be called first before configuring
  ulp-timer instance parameters

* By- default configures 'reference clock source'(32 MHZ) as timer clock source,
  clock-type as 'static', disables synchronization to ULPSS pclk and skip waiting
  for switching of timer clock.

* Configurations will remain same for all ulp-timer instances.

* @note:
* There are two \ref XTAL Clk sources one is Internal and external \ref XTAL clk source.
  In order to enable the external XTAL clk source need to configure the \ref NPSS_GPIO pins
  which can be done through sl_si91x_ulp_timer_configure_xtal_clock(uint8_t xtal_pin) API
  i.e user need to call that API first.
* In order to enable the soc CLK source need to configure the Ulpss soc Clk from M4 soc clk
  please use \ref sl_si91x_ulp_timer_configure_soc_clock(boolean_t div_factor_type, 
                                          uint16_t div_factor )
 *******************************************************************************/
sl_status_t sl_si91x_ulp_timer_configure_clock(ulp_timer_clk_src_config_t *timer_clk_ptr)
{
  sl_status_t status;
  rsi_error_t error_status;
  status = SL_STATUS_OK;
  uint8_t clk_src;
  do {
    // To validate the structure pointer, if the parameters is NULL, it
    // will return an error code
    if (timer_clk_ptr == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // To validate the clock-type, if it is not valid type will return an error code
    if (timer_clk_ptr->ulp_timer_clk_type > ENABLE_STATIC_CLK) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // To store input timer input clock source parameter value and validating it.
    // If its not a valid ulp-timer clock source, will return an error code
    clk_src = timer_clk_ptr->ulp_timer_clk_input_src;
    if (clk_src >= ULP_TIMER_ULP_CLK_SRC_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // It will store the RSI error code which is returned by the below API and
    // will return the SL error code
    error_status = RSI_ULPSS_TimerClkConfig(ULPCLK,
                                            timer_clk_ptr->ulp_timer_clk_type,
                                            timer_clk_ptr->ulp_timer_sync_to_ulpss_pclk,
                                            timer_clk_ptr->ulp_timer_clk_input_src,
                                            (uint8_t)timer_clk_ptr->ulp_timer_skip_switch_time);
    if (error_status != RSI_OK) {
      status = SL_STATUS_INVALID_CONFIGURATION;
    }
  } while (false);
  return status;
}

/*******************************************************************************
* @brief:Configures timer instance parameters

* @details:
*  Configures the ulp-timer number, mode, type, match-value and direction with following
   default values:
   -Timer number : 0
   -Timer mode : periodic
   -Timer type : Normal Down-counter
   -Timer direction : Down-counter

*  Should be called after timer clock configurations, when timer is not running.

*  Updates integral and fractional values of clock cycles as per timer-type and clock source.

*  Maps timer interrupt to their respective IRQ numbers(Enables NVIC),
   so should be called before registering timeout-callbacks
*******************************************************************************/
sl_status_t sl_si91x_ulp_timer_set_configuration(ulp_timer_config_t *timer_config_ptr)
{
  sl_status_t status;
  rsi_error_t error_status;
  uint16_t integral_part;
  uint8_t fractional_part;
  uint8_t timer_index;
  status = SL_STATUS_OK;
  do {
    // To validate the structure pointer, if the parameters is NULL, it
    // will return an error code
    if (timer_config_ptr == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Setting ulp-timer mode:
    // It will store the RSI error code which is returned by the below API and
    // will return the SL error code
    error_status = RSI_TIMERS_SetTimerMode(TIMERS, timer_config_ptr->timer_mode, timer_config_ptr->timer_num);
    if (error_status == ERROR_INVAL_TIMER_MODE) {
      status = SL_STATUS_INVALID_MODE;
      break;
    }
    if (error_status == ERROR_INVAL_TIMER_NUM) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // Setting ulp-timer type
    // It will store the RSI error code which is returned by the below API and
    // will return the SL error code
    error_status = RSI_TIMERS_SetTimerType(TIMERS, timer_config_ptr->timer_type, timer_config_ptr->timer_num);
    if (error_status == ERROR_INVAL_TIMERTYPE) {
      status = SL_STATUS_INVALID_TYPE;
      break;
    }
    if (error_status == ERROR_INVAL_TIMER_NUM) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    if ((timer_config_ptr->timer_type == ULP_TIMER_TYP_1US) || (timer_config_ptr->timer_type == ULP_TIMER_TYP_256US)) {
      // Updating clock cycles integral and fractional parts variables, as per input clock and type
      status = get_int_frac_parts(timer_config_ptr->timer_type, &integral_part, &fractional_part);
      if (status == SL_STATUS_INVALID_TYPE) {
        break;
      }
      // Configuring integral and fractional part registers of timer, as per timer
      // type and input clock source selected.
      // It will store the RSI error code which is returned by the below API and
      // will return the SL error code
      error_status = RSI_TIMERS_MicroSecTimerConfig(TIMERS,
                                                    timer_config_ptr->timer_num,
                                                    integral_part,
                                                    fractional_part,
                                                    timer_config_ptr->timer_type);
      // Correcting error code returned by above RSI API, as it is returning
      // invalid-mode error for invalid-type
      if (error_status == ERROR_INVAL_TIMER_MODE) {
        status = SL_STATUS_INVALID_TYPE;
        break;
      }
    }
    // Setting ulp-timer direction: up-conter or down-counter
    // It will store the RSI error code which is returned by the below API and
    // will return the SL error code
    error_status = RSI_TIMERS_SetDirection(TIMERS, timer_config_ptr->timer_num, timer_config_ptr->timer_direction);
    if (error_status == ERROR_INVAL_COUNTER_DIR) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (error_status == ERROR_INVAL_TIMER_NUM) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // Setting ulp-timer match value
    // It will store the RSI error code which is returned by the below API and
    // will return the SL error code
    error_status = RSI_TIMERS_SetMatch(TIMERS, timer_config_ptr->timer_num, timer_config_ptr->timer_match_value);
    if (error_status == ERROR_INVAL_TIMER_NUM) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // Timer interrupt mapping to ARM
    for (timer_index = ULP_TIMER_0; timer_index < ULP_TIMER_LAST; timer_index++) {
      if (timer_index == timer_config_ptr->timer_num) {
        NVIC_EnableIRQ(ulp_timer_irq_numbers[timer_index]);
        break;
      }
    }
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Retrieves the match value for ULP timer

* @details:
*  This function calculates the match value for a given ULP timer type based on the 
*  input time in microseconds. The match value represents the number of clock cycles 
*  required for the timer to reach the specified time. The timer type can be one of 
*  the following:
*   - ULP_TIMER_TYP_DEFAULT: Uses the clock cycles per microsecond to compute the match value.
*   - ULP_TIMER_TYP_1US: Returns the match value as the number of microseconds.
*   - ULP_TIMER_TYP_256US: Returns the match value as the number of 256-microsecond units.
* 
*  The function performs validation of the timer type and the time in microseconds to 
*  ensure they meet the expected criteria. If the input is valid, it calculates the 
*  match value, otherwise returns an appropriate error status:
*   - SL_STATUS_INVALID_INDEX: Invalid timer type.
*   - SL_STATUS_INVALID_PARAMETER: Invalid time value for the specified timer type.
* 
*   - The ULP timer clock frequency is retrieved from the system and used for calculation.
*   - The clock cycles per microsecond are derived from the ULP timer clock frequency.
* 
*  The function should be called to retrieve the match value before configuring 
*  and starting the ULP timer for counting operations.
*******************************************************************************/
sl_status_t sl_si91x_ulp_timer_get_match_value(ulp_timer_type_t timer_type, uint32_t time_us, uint32_t *match_value)
{
  sl_status_t status;
  uint32_t ulp_timer_clock;
  uint32_t clock_cycles_per_us;
  status = SL_STATUS_OK;
  // Reading ulp-timer peripheral clock frequency
  ulp_timer_clock = RSI_CLK_GetBaseClock(ULPSS_TIMER);
  // Calculate clock cycles per microsecond
  clock_cycles_per_us = ulp_timer_clock / 1000000;
  do {
    // Validating timer number parameter
    if (timer_type >= ULP_TIMER_TYP_LAST) {
      status = SL_STATUS_INVALID_TYPE;
      break;
    }
    if (timer_type == ULP_TIMER_TYP_DEFAULT) {
      // Match value = (clock cycles per microsecond) * (time in microseconds)
      *match_value = (clock_cycles_per_us * time_us);
    } else if (timer_type == ULP_TIMER_TYP_1US) {
      if (time_us < 1) {
        status = SL_STATUS_INVALID_PARAMETER;
        break;
      }
      // Match value = number of microseconds
      *match_value = time_us;
    } else if (timer_type == ULP_TIMER_TYP_256US) {
      if (time_us < 256) {
        status = SL_STATUS_INVALID_PARAMETER;
        break;
      }
      // Match value = (time in microseconds) / 256
      *match_value = (time_us / 256);
    }
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: starts ulp-timer instance

* @details:
*  Starts the timer instance with configured parameters

*  Sets the start bit of timer control register, which is a self clearing bit.

*  Should be called after clock & timer configurations and timeout-callback registrations.

*  Timer instance must not be in running state when using this API
*******************************************************************************/
sl_status_t sl_si91x_ulp_timer_start(ulp_timer_instance_t timer_num)
{
  sl_status_t status;
  rsi_error_t error_status;
  status = SL_STATUS_OK;
  do {
    // Validating timer number parameter
    if (timer_num >= ULP_TIMER_LAST) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // Starting the timer instance:
    // It will store the RSI error code which is returned by the below API and
    // will return the SL error code
    error_status = RSI_TIMERS_TimerStart(TIMERS, timer_num);
    if (error_status != RSI_OK) {
      status = SL_STATUS_INVALID_INDEX;
    }
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: stops ulp-timer instance

* @details:
*  Sets the stop bit of timer control register, which is a self clearing bit.

*  Should be called to stop any running timer instance.

*  This will not disable timer interrupt and unregister time-out callback
*******************************************************************************/
sl_status_t sl_si91x_ulp_timer_stop(ulp_timer_instance_t timer_num)
{
  sl_status_t status;
  rsi_error_t error_status;
  status = SL_STATUS_OK;
  do {
    // Validating timer number parameter
    if (timer_num >= ULP_TIMER_LAST) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // Stopping the timer instance:
    // It will store the RSI error code which is returned by the below API and
    // will return the SL error code
    error_status = RSI_TIMERS_TimerStop(TIMERS, timer_num);
    if (error_status != RSI_OK) {
      status = SL_STATUS_INVALID_INDEX;
    }
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: restarts a ulp-timer

* @details:
*  Restarts an already running timer with already configured parameters.

*  It first stops the timer instance and then start it again.
*******************************************************************************/
sl_status_t sl_si91x_ulp_timer_restart(ulp_timer_instance_t timer_num)
{
  sl_status_t status;
  status = SL_STATUS_OK;
  do {
    // Validating timer number parameter
    if (timer_num >= ULP_TIMER_LAST) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // Stopping the timer instance, it will update 'status' with SL API return value
    status = sl_si91x_ulp_timer_stop(timer_num);
    if (status != SL_STATUS_OK) {
      break;
    }
    // Starting the timer instance, it will update 'status 'with SL API return value
    status = sl_si91x_ulp_timer_start(timer_num);
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Sets the ulp-timer mode to one-shot or periodic

* @details:
*  Used to set or change the timer mode parameter, when timer is stopped

*  It sets the timer-mode bit of timer control register for periodic mode and
   resets the same for one-shot mode

*  In periodic mode, timer gets reset when timeout occurs and starts again
   automatically while in one-shot mode it stops after one timeout-interrupt

*  User must stop the timer instance before changing timer parameter

* In one-shot mode, timer will stop and disable interrupt after first timeout
  interrupt.
*******************************************************************************/
sl_status_t sl_si91x_ulp_timer_set_mode(ulp_timer_instance_t timer_num, ulp_timer_mode_t timer_mode)
{
  sl_status_t status;
  rsi_error_t error_status;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    // Validating timer number parameter
    if (timer_num >= ULP_TIMER_LAST) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // Setting ulp-timer mode
    // It will store the RSI-error code, returned by the below API and then
    // updating SL-error code as per RSI error code, if RSI-error code is OK
    // then no need to update, as it is already initialized with
    // SL_STATUS_OK
    error_status = RSI_TIMERS_SetTimerMode(TIMERS, timer_mode, timer_num);
    if (error_status == ERROR_INVAL_TIMER_MODE) {
      status = SL_STATUS_INVALID_MODE;
      break;
    }
    if (error_status == ERROR_INVAL_TIMER_NUM) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Gets the ulp-timer mode to one-shot or periodic

* @details:
*  It reads the timer-mode bit of timer control register, for periodic updates one
   & zero for one-shot mode to the variable to store timer-mode value

*  It updates variable to store timer-type by:
   - 0 for one-shot mode
   - 1 for periodic mode

*  Returns status 0 if successful, else error code
*******************************************************************************/
sl_status_t sl_si91x_ulp_timer_get_mode(ulp_timer_instance_t timer_num, uint32_t *timer_mode)
{
  sl_status_t status;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    // Validating timer number parameter
    if (timer_num >= ULP_TIMER_LAST) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // Validating the 'pointer to count_value' parameter,
    // if it is NULL will return an error code
    if (timer_mode == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Reading timer direction
    *timer_mode = RSI_TIMERS_GetTimerMode(TIMERS, timer_num);
    if (*timer_mode == ERROR_INVAL_TIMER_NUM) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Sets the ulp-timer direction as up-counting or down-counting

* @details:
*  Used to set or change the timer-direction, when timer is stopped

*  It sets the timer-direction bit of timer control register, 1 for up-counting
   and 0 for down-counting, reset value of this bit is 0 means by-default it works
   as down-counter
*******************************************************************************/
sl_status_t sl_si91x_ulp_timer_set_direction(ulp_timer_instance_t timer_num, ulp_timer_direction_t counter_dir)
{
  sl_status_t status;
  rsi_error_t error_status;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    // Validating timer number parameter
    if (timer_num >= ULP_TIMER_LAST) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // Validating counter direction parameter
    if (counter_dir >= LAST_DIRECTION) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    error_status = RSI_TIMERS_SetDirection(TIMERS, timer_num, counter_dir);
    if (error_status == ERROR_INVAL_COUNTER_DIR) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (error_status == ERROR_INVAL_TIMER_NUM) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: gets the ulp-timer direction as up-counting or down-counting

* @details:
*  It reads the timer-direction bit of timer control register and updates 1
   for up-counting and 0 for down-counting to the variable to store timer-direction
   Returns status 0 if successful, else error code
*******************************************************************************/
sl_status_t sl_si91x_ulp_timer_get_direction(ulp_timer_instance_t timer_num, uint32_t *timer_direction)
{
  sl_status_t status;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    // Validating timer number parameter
    if (timer_num >= ULP_TIMER_LAST) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // Validating the 'pointer to count_value' parameter,
    // if it is NULL will return an error code
    if (timer_direction == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Reading timer direction
    *timer_direction = RSI_TIMERS_getDirection(TIMERS, timer_num);
    if (*timer_direction == ERROR_INVAL_TIMER_NUM) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
  } while (false);
  return status;
}
/*******************************************************************************
* @brief: Sets the ulp-timer type to one microsecond or 256 microsecond type

* @details:
*  Used to set or change the timer-type parameter, when timer is stopped

*  It sets the timer-type bit of timer control register as:
   '2'- 256 µs mode
   '1'-1µs mode
   '0'- Count-down timer

*  In 1µs mode, the time unit is 1µs, number of microseconds required to be
   counted has to be programmed as match-value while in 256µs mode the time unit
   is 256µ & number of 256µs units required to be counted has to be programmed
   as match value.

*  User must stop the timer instance before changing timer parameter
*******************************************************************************/
sl_status_t sl_si91x_ulp_timer_set_type(ulp_timer_instance_t timer_num, ulp_timer_type_t timer_type)
{
  sl_status_t status;
  rsi_error_t error_status;
  uint16_t clk_integral_part;
  uint8_t clk_fractional_part;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    // Validating timer number parameter
    if (timer_num >= ULP_TIMER_LAST) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // Setting ulp-timer instance type:
    // It will store the RSI-error code, returned by the below API and then
    // updating SL-error code as per RSI error code, if RSI-error code is OK
    // then no need to update, as it is already initialized with SL_STATUS_OK
    error_status = RSI_TIMERS_SetTimerType(TIMERS, timer_type, timer_num);
    if (error_status == ERROR_INVAL_TIMERTYPE) {
      status = SL_STATUS_INVALID_TYPE;
      break;
    }
    if (error_status == ERROR_INVAL_TIMER_NUM) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    if ((timer_type == ULP_TIMER_TYP_1US) || (timer_type == ULP_TIMER_TYP_256US)) {
      // Calculating clock integral and fractional parts variables, as per input clock
      status = get_int_frac_parts(timer_type, &clk_integral_part, &clk_fractional_part);
      if (status == SL_STATUS_INVALID_TYPE) {
        break;
      }
      // Updating clock cycle integral and fractional parts variables to the integral and
      // fractional value registers of timer peripheral
      error_status =
        RSI_TIMERS_MicroSecTimerConfig(TIMERS, timer_num, clk_integral_part, clk_fractional_part, timer_type);
      // Correcting error code returned by RSI API, as it is returning invalid-mode error
      // for invalid-type
      if (error_status == ERROR_INVAL_TIMER_MODE) {
        status = SL_STATUS_INVALID_TYPE;
        break;
      }
    }
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: gets the ulp-timer type

* @details:
*  It reads the timer-direction bit of timer control register
*  It updates variable to store timer-type by:
   - 0 for down-counter type
   - 1 for 1us type
   - 2 for 256us type
*  Returns status 0 if successful, else error code
*******************************************************************************/
sl_status_t sl_si91x_ulp_timer_get_type(ulp_timer_instance_t timer_num, uint32_t *timer_type)
{
  sl_status_t status;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    // Validating timer number parameter
    if (timer_num >= ULP_TIMER_LAST) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // Validating the 'pointer to count_value' parameter,
    // if it is NULL will return an error code
    if (timer_type == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Reading timer direction
    *timer_type = RSI_TIMERS_GetTimerType(TIMERS, timer_num);
    if (*timer_type == ERROR_INVAL_TIMER_NUM) {
      status = SL_STATUS_INVALID_INDEX;
    }
  } while (false);
  return status;
}
/*******************************************************************************
* @brief: Gets ulp-timer current count.

* @details:
*  Reads the value of timer-match register value of the timer-instance.

*  If counter-down is set register bits indicate time remaining before timeout
   (read as 32'hFFFF_FFFF initially).

*  If counter-up is set these bits directly gives out the up-running
   counter/timer value.

*  Updates the timer current-count value to the count-value variable, passed as
   reference by user (as 2nd argument)
*******************************************************************************/
sl_status_t sl_si91x_ulp_timer_get_count(ulp_timer_instance_t timer_num, uint32_t *count_value)
{
  sl_status_t status;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    // Validating timer number parameter
    if (timer_num >= ULP_TIMER_LAST) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // Validating the 'pointer to count_value' parameter,
    // if it is NULL will return an error code
    if (count_value == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // reading counter current count value
    *count_value = (TIMERS->MATCH_CTRL[timer_num].MCUULP_TMR_MATCH);
  } while (false);
  return status;
}

/*******************************************************************************
*@brief: Sets ulp-timer match-value.

*@details:
* Sets the match-value of the timer instance, by updating its timer-match-register bits.

* Match-value calculations:
* For one microsecond type, match-value = number of microseconds
* For 256 microsecond type, match-value = (time in microseconds)/256
* For normal up/down counter type, timer =  (number of clock cycles per microseconds * time in microseconds)

* User must stop the timer instance before changing timer parameter
*******************************************************************************/
sl_status_t sl_si91x_ulp_timer_set_count(ulp_timer_instance_t timer_num, uint32_t timer_match_value)
{
  sl_status_t status;
  rsi_error_t error_status;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    // Validating timer number parameter
    if (timer_num >= ULP_TIMER_LAST) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // Setting timer instance match value:
    // It will store the RSI-error code, returned by the below API and then
    // updating SL-error code as per RSI error code, if RSI-error code is OK
    // then no need to update, as it is already initialized with SL_STATUS_OK
    error_status = RSI_TIMERS_SetMatch(TIMERS, timer_num, timer_match_value);
    if (error_status == ERROR_INVAL_TIMER_NUM) {
      status = SL_STATUS_INVALID_INDEX;
    }
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Registers Callback on timer expiration and enables timer interrupt

* @details:
* It will register the callback, i.e., stores the callback function address
  and pass to the variable that will be called in Interrupt Handler.

* It will also enables the timer interrupt.

* If any callback is already registered, the user needs to unregister the
* callback first before registering another callback.
*******************************************************************************/
sl_status_t sl_si91x_ulp_timer_register_timeout_callback(ulp_timer_instance_t timer_num,
                                                         ulp_timer_callback_t on_timeout_callback)
{
  sl_status_t status;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    // Validating the 'on_timeout_callback function pointer' parameter and void pointer to callback data,
    // if they are NULL will return an error code
    if (on_timeout_callback == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validating timer number parameter
    if (timer_num >= ULP_TIMER_LAST) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // To validate the function pointer, if the parameters is not NULL then it
    //will return an busy error code
    if (timeout_callback_function_pointers[timer_num] != NULL) {
      status = SL_STATUS_BUSY;
      break;
    }
    // Enabling timer interrupt and validating its return status
    status = ulp_timer_enable_interrupt(timer_num);
    if (status != SL_STATUS_OK) {
      break;
    }
    // The function pointer is fed to the static variable, which will be called in the IRQ handler
    timeout_callback_function_pointers[timer_num] = on_timeout_callback;
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Unregisters Callback on timer expiration and disables interrupts

* @details:
* It will unregister the callback, i.e., clear the callback function address
   and pass NULL value to the variable

* It will also mask and disable the timer interrupt.
*******************************************************************************/
sl_status_t sl_si91x_ulp_timer_unregister_timeout_callback(ulp_timer_instance_t timer_num)
{
  sl_status_t status;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    // Disabling passed timer instance interrupt
    status = ulp_timer_disable_interrupt(timer_num);
    if (status != SL_STATUS_OK) {
      break;
    }
    //The callback should be null in the unregister callback api because
    //one cannot register the callback if it is not null
    timeout_callback_function_pointers[timer_num] = NULL;
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief: To configure the external xtal clock source
 
 * @details:
 * In order to enable the external XTAL clk source need to configure the \ref NPSS_GPIO pins
   which can be done through RSI_CLK_XtalClkConfig(uint8_t xtalPin) API i.e user need to call
   that API first (before configure clock api)

 * Pin number of NPSS_GPIO for xtal clock source are 0,1,2,3,4
*******************************************************************************/
sl_status_t sl_si91x_ulp_timer_configure_xtal_clock(uint8_t xtal_pin)
{
  sl_status_t status;
  rsi_error_t error_status;
  status       = SL_STATUS_OK;
  error_status = RSI_CLK_XtalClkConfig(xtal_pin);
  if (error_status == INVALID_PARAMETERS) {
    status = SL_STATUS_INVALID_PARAMETER;
  }
  return status;
}

/*******************************************************************************
 * @brief: Configures the Ulpss soc Clk from M4 soc clk, to enable the soc CLK source
   
 * @details:
 * This API is used to select the ULPSS processor clock source when input is soc clk
   source which is greater than 100MHz(used for high-power mode)
 * To divide the clock, ensure that it should be odd number if div_factor_type is 1 & viceversa
 * Div_factor_type selects the type of divider for m4_soc_clk_2ulpss
    -  0 => Even Divider is selected
    -  1 => Odd Divider is selected
 * Use this API in place of timer clock config API, when using soc clock source
   with timer in high power mode
*******************************************************************************/
sl_status_t sl_si91x_ulp_timer_configure_soc_clock(boolean_t div_factor_type, uint16_t div_factor)
{
  sl_status_t status;
  rsi_error_t error_status;
  status       = SL_STATUS_OK;
  uint16_t two = 2;
  do {
    // Validating div_factor value for odd div_factor_type
    if (div_factor_type) {
      if (!(div_factor % two)) {
        status = SL_STATUS_INVALID_PARAMETER;
        break;
      }
    }
    // Validating div_factor value for odd div_factor_type
    if (!(div_factor_type)) {
      if (div_factor % two) {
        status = SL_STATUS_INVALID_PARAMETER;
        break;
      }
    }
    error_status = RSI_ULPSS_ClockConfig(M4CLK, true, div_factor, div_factor_type);
    if (error_status == INVALID_PARAMETERS) {
      status = SL_STATUS_INVALID_PARAMETER;
    }
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Initialization of ulp timer clock
*
* @details:
* This API is used to initialize timer by configuring its clock
*******************************************************************************/
sl_status_t sl_si91x_ulp_timer_init(ulp_timer_clk_src_config_t *timer_clk_ptr)
{
  sl_status_t status;
  status = sl_si91x_ulp_timer_configure_clock(timer_clk_ptr);
  return status;
}

/*******************************************************************************
* De-Initialization of ulp timer clock
* Unregisters callback of all timer instances
*
* @details:
* This API is used to de-initializes timer by disabling peripheral clock
*******************************************************************************/
void sl_si91x_ulp_timer_deinit(void)
{
  //Unregistering callback for timer-0
  sl_si91x_ulp_timer_unregister_timeout_callback(ULP_TIMER_0);
  //Unregistering callback for timer-1
  sl_si91x_ulp_timer_unregister_timeout_callback(ULP_TIMER_1);
  //Unregistering callback for timer-0
  sl_si91x_ulp_timer_unregister_timeout_callback(ULP_TIMER_2);
  //Unregistering callback for timer-0
  sl_si91x_ulp_timer_unregister_timeout_callback(ULP_TIMER_3);
  // Disabling clock for ULP-timer peripheral
  RSI_ULPSS_TimerClkDisable(ULPCLK);
}

/*******************************************************************************
 * @brief
 * To get the release, sqa and dev version of ULP_TIMER
 * It returns the structure for ULP_TIMER version.
 * Structure includes three members:
 * - Release version
 * - SQA version
 * - Dev version
 ******************************************************************************/
sl_ulp_timer_version_t sl_si91x_ulp_timer_get_version(void)
{
  sl_ulp_timer_version_t version;
  version.minor   = ULP_TIMER_DEV_VERSION;
  version.release = ULP_TIMER_RELEASE_VERSION;
  version.major   = ULP_TIMER_SQA_VERSION;
  return version;
}

/*******************************************************************************
* @brief: Internal function to get ulp-timer interrupt status
          It updates:
          '1' for Interrupt present
          '0' for no interrupt present
* @param[in]  timer_num enum for ULP-timer Number, \ref #ulp_timer_instance_t for possible values.
* @return    status 0 if successful, else error code
           SL_STATUS_INVALID_INDEX (0x0027) - 'timer_num' parameter value is invalid.
           SL_STATUS_NULL_POINTER (0x0022) -'Pointer to int_status variable' parameter is a null pointer.
           SL_STATUS_OK (0x0000) - Successfully cleared the timer interrupt.
*******************************************************************************/
static sl_status_t ulp_timer_get_interrupt_status(ulp_timer_instance_t timer_num, uint8_t *int_status)
{
  sl_status_t status;
  status = SL_STATUS_OK;
  do {
    // Validating timer number parameter
    if (timer_num >= ULP_TIMER_LAST) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    //Validating the 'pointer to int_status' parameter, if it is NULL will return an error code
    if (int_status == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Reading timer instance interrupt status, and store the return value to int_status
    *int_status = RSI_TIMERS_InterruptStatus(TIMERS, timer_num);
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Internal function to clear the ulp-timer interrupt

* @param[in]  timer_num enum for ULP-timer Number, \ref #ulp_timer_instance_t for possible values.
* @return    status 0 if successful, else error code
           SL_STATUS_INVALID_INDEX (0x0027) - 'timer_num' parameter value is invalid.
           SL_STATUS_OK (0x0000) - Successfully cleared the timer interrupt.
*******************************************************************************/
static sl_status_t ulp_timer_clear_interrupt(ulp_timer_instance_t timer_num)
{

  sl_status_t status;
  rsi_error_t error_status;
  status = SL_STATUS_OK;
  do {
    // Validating timer number parameter
    if (timer_num >= ULP_TIMER_LAST) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // Clearing ulp-timer instance interrupt:
    // It will store the RSI-error code, returned by the below API and then
    // updating SL-error code as per RSI error code, if RSI-error code is OK
    // then no need to update, as it is already initialized with SL_STATUS_OK
    error_status = RSI_TIMERS_InterruptClear(TIMERS, timer_num);
    if (error_status == ERROR_INVAL_TIMER_NUM) {
      status = SL_STATUS_INVALID_INDEX;
    }
    // It will validate the register with the appropriate values, if the values are
    // not equal to which we set, it will return an error code
    if (TIMERS->MATCH_CTRL[timer_num].MCUULP_TMR_CNTRL_b.TMR_INTR_CLR != DISABLE) {
      status = SL_STATUS_ALLOCATION_FAILED;
    }
  } while (false);
  return status;
}

/*******************************************************************************
*  @brief: Internal function to enable ULP-Timer interrupt

 * @param[in]  timer_num enum for ULP-timer Number (0 to 3)
 * @return    status 0 if successful, else error code
 *         SL_STATUS_INVALID_INDEX (0x0027) - Invalid index.
 *         SL_STATUS_ALLOCATION_FAILED   (0x0019) - Generic allocation error.
 *         SL_STATUS_OK (0x0000) - Success
*******************************************************************************/
static sl_status_t ulp_timer_enable_interrupt(ulp_timer_instance_t timer_num)
{
  sl_status_t status;
  rsi_error_t error_status;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    // Validating timer number parameter
    if (timer_num >= ULP_TIMER_LAST) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // Enabling timer instance interrupt
    error_status = RSI_TIMERS_InterruptEnable(TIMERS, timer_num);
    if (error_status != RSI_OK) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    //Validating register value
    if (TIMERS->MATCH_CTRL[timer_num].MCUULP_TMR_CNTRL_b.TMR_INTR_ENABLE != ENABLE) {
      status = SL_STATUS_ALLOCATION_FAILED;
    }
  } while (false);
  return status;
}

/*******************************************************************************
*  @brief: Internal function to disable ulp-timer interrupt.
*
* @param[in]  timer_num enum for ULP-timer Number (0 to 3)
* @return    status 0 if successful, else error code
*         SL_STATUS_INVALID_INDEX (0x0027) - Invalid index.
*         SL_STATUS_ALLOCATION_FAILED (0x0019) - Generic allocation error.
*         SL_STATUS_OK (0x0000) - Success
*******************************************************************************/
static sl_status_t ulp_timer_disable_interrupt(ulp_timer_instance_t timer_num)
{
  sl_status_t status;
  rsi_error_t error_status;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    if (timer_num >= ULP_TIMER_LAST) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // Disabling timer instance interrupt
    error_status = RSI_TIMERS_InterruptDisable(TIMERS, timer_num);
    if (error_status == ERROR_INVAL_TIMER_NUM) {
      status = SL_STATUS_INVALID_INDEX;
      break;
    }
    // Validating register value
    if (TIMERS->MATCH_CTRL[timer_num].MCUULP_TMR_CNTRL_b.TMR_INTR_ENABLE != DISABLE) {
      status = SL_STATUS_ALLOCATION_FAILED;
    }
  } while (false);
  return status;
}

/*******************************************************************************
*  @brief: Internal function to calculate Integral and fractional part of clock cycles per microseconds
*
* @param[in] timer_type timer_type enum for ULP-timer Type, \ref #ulp_timer_type_t for possible values
* @param[in] integral_part (uint16_t pointer), pointer to variable which will store
*            integral part of clock cycles per microseconds
* @param[in] fractional_part (uint16_t pointer), pointer to variable which will store
*            fractional part of clock cycles per microseconds
* @return status 0 if successful, else error code
*          SL_STATUS_INVALID_TYPE (0x0026) - 'timer_type' parameter value is invalid.
*******************************************************************************/
static sl_status_t get_int_frac_parts(ulp_timer_type_t timer_type, uint16_t *integral_part, uint8_t *fractional_part)
{
  int index = 0;
  uint32_t clock_value;
  sl_status_t status;
  status = SL_STATUS_OK;
  // Reading ulp-timer peripheral clock frequency
  clock_value = RSI_CLK_GetBaseClock(ULPSS_TIMER);
  do {
    // Assigning integral and fractional values of clock cycles per microseconds
    // or per 256-microseconds, as per timer type
    if (timer_type == ULP_TIMER_TYP_1US) {
      for (index = FIRST_INDEX; index < LAST_INDEX; index++) {
        if (int_frac_values_1US[index].timer_clk_source_value == clock_value) {
          *integral_part   = int_frac_values_1US[index].clk_integral_part;
          *fractional_part = int_frac_values_1US[index].clk_fractional_part;
        }
      }
      break;
    }
    if (timer_type == ULP_TIMER_TYP_256US) {
      for (index = FIRST_INDEX; index < LAST_INDEX; index++) {
        if (int_frac_values_256US[index].timer_clk_source_value == clock_value) {
          *integral_part   = int_frac_values_256US[index].clk_integral_part;
          *fractional_part = int_frac_values_256US[index].clk_fractional_part;
        }
      }
      break;
    }
    status = SL_STATUS_INVALID_TYPE;
  } while (false);
  return status;
}

/*******************************************************************************
* Interrupt handler for timer 0
*
* @param none
* @return none
*******************************************************************************/
void ULP_TIMER0_IRQHandler(void)
{
  uint8_t int_status;
  // Reading interrupt status
  ulp_timer_get_interrupt_status(ULP_TIMER_0, &int_status);
  if (int_status) {
    // Clearing interrupt if not clear
    ulp_timer_clear_interrupt(ULP_TIMER_0);
  }
  timeout_callback_function_pointers[ULP_TIMER_0]();
}

/*******************************************************************************
* Interrupt handler for timer 1
*
* @param none
* @return none
*******************************************************************************/
void ULP_TIMER1_IRQHandler(void)
{
  uint8_t int_status;
  // Reading interrupt status
  ulp_timer_get_interrupt_status(ULP_TIMER_1, &int_status);
  if (int_status) {
    // Clearing interrupt if not clear
    ulp_timer_clear_interrupt(ULP_TIMER_1);
  }
  timeout_callback_function_pointers[ULP_TIMER_1]();
}

/*******************************************************************************
* Interrupt handler for timer 2
*
* @param none
* @return none
*******************************************************************************/
void ULP_TIMER2_IRQHandler(void)
{
  uint8_t int_status;
  // Reading interrupt status
  ulp_timer_get_interrupt_status(ULP_TIMER_2, &int_status);
  if (int_status) {
    // Clearing interrupt if not clear
    ulp_timer_clear_interrupt(ULP_TIMER_2);
  }
  timeout_callback_function_pointers[ULP_TIMER_2]();
}

/*******************************************************************************
* Interrupt handler for timer 3
*
* @param none
* @return none
*******************************************************************************/
void ULP_TIMER3_IRQHandler(void)
{
  uint8_t int_status;
  // Reading interrupt status
  ulp_timer_get_interrupt_status(ULP_TIMER_3, &int_status);
  if (int_status) {
    // Clearing interrupt if not clear
    ulp_timer_clear_interrupt(ULP_TIMER_3);
  }
  timeout_callback_function_pointers[ULP_TIMER_3]();
}

/**
 * @brief  Main program.
 * @param  None
 * @retval None
 */
