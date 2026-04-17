/******************************************************************************
* @file  sl_si91x_watchdog_timer.c
* @brief Watchdog-Timer API implementation
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
#include "sl_si91x_watchdog_timer.h"
#ifndef SL_WDT_MANAGER_PRESENT
#include "sl_si91x_watchdog_timer_config.h"
#endif
#include "rsi_rtc.h"
/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/
#define WATCHDOG_TIMER_IRQHandler      IRQ020_Handler            // WDT IRQ Handler
#define NVIC_WDT                       NPSS_TO_MCU_WDT_INTR_IRQn // WDT NVIC IRQ Number
#define TIME_PERIOD_VALUE              0x003E7FFF                // time-period value for rtc clock
#define WATCHDOG_TIMER_RELEASE_VERSION 0                         // Watchdog-timer Release version
#define WATCHDOG_TIMER_MAJOR_VERSION   0                         // Watchdog-timer SQA version
#define WATCHDOG_TIMER_MINOR_VERSION   2                         // Watchdog-timer Developer version

/*******************************************************************************
 ***************************  Local TYPES  ********************************
 ******************************************************************************/

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static watchdog_timer_callback_t timeout_callback_function_pointer = NULL;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
void WATCHDOG_TIMER_IRQHandler(void);

/*******************************************************************************
 **********************  Local Function Definition****************************
 ******************************************************************************/

/*******************************************************************************
***********************  Global function Definitions *************************
*******************************************************************************/

/*******************************************************************************
* @brief: Initializes watchdog-timer
*
* @details:
* Initializes(power-ups) the timer
* Sets RTC clock time-period, for value /ref TIME_PERIOD_VALUE
* un-masks timer interrupt
*******************************************************************************/
void sl_si91x_watchdog_init_timer(void)
{
  // Un-masking WDT interrupt
  RSI_WWDT_IntrUnMask();
  // Initializing watchdog-timer (powering up WDT)
  RSI_WWDT_Init(MCU_WDT);
}

/*******************************************************************************
* @brief: This API is no longer supported due to the restriction on peripheral drivers to configuring clock
*******************************************************************************/
sl_status_t sl_si91x_watchdog_configure_clock(watchdog_timer_clock_config_t *timer_clk_config_ptr)
{
  sl_status_t status = SL_STATUS_OK; // return ok to support backward compatibility

  // FSM LF Clock has already been configured in sl_system_init,
  // This API is no longer supported due to the restriction on peripheral drivers to configure clocks
  UNUSED_VARIABLE(timer_clk_config_ptr);

  return status;
}

/*******************************************************************************
* @brief: Configures watchdog-timer parameters
*
* @details:
*  Sets interrupt timeout time
*  Sets system reset time
*  Sets system window time
*******************************************************************************/
sl_status_t sl_si91x_watchdog_set_configuration(watchdog_timer_config_t *timer_config_ptr)
{
  sl_status_t status = SL_STATUS_OK;
  /* WDT_TIMER_UC is defined by default. when this macro (WDT_TIMER_UC) is defined, peripheral
   * configuration is directly taken from the configuration set in the universal configuration (UC).
   * if the application requires the configuration to be changed in run-time, undefined this macro
   * and change the peripheral configuration through the sl_si91x_watchdog_set_configuration API.
   */
  do {
    // To validate the structure pointer, if the parameters is NULL, it
    // will return an error code
    if (timer_config_ptr == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validating WDT interrupt time, system-reset time and window time value
    if ((timer_config_ptr->interrupt_time >= TIME_DELAY_LAST)
        || (timer_config_ptr->system_reset_time >= TIME_DELAY_LAST)
        || (timer_config_ptr->window_time >= TIME_DELAY_16)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Comparing interrupt time with window time
    if (timer_config_ptr->interrupt_time <= timer_config_ptr->window_time) {
      status = SL_STATUS_INVALID_CONFIGURATION;
      break;
    }
#if (WDT_TIMER_UC == 1)
    timer_config_ptr = &sl_watchdog_timer_config_handle;
#endif
    // Configure the WDT system reset value
    RSI_WWDT_ConfigSysRstTimer(MCU_WDT, timer_config_ptr->system_reset_time);
    // configure the WDT timeout interrupt time
    RSI_WWDT_ConfigIntrTimer(MCU_WDT, timer_config_ptr->interrupt_time);
    // configure the WDT window time
    RSI_WWDT_ConfigWindowTimer(MCU_WDT, timer_config_ptr->window_time);
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Registers Callback on timer expiration and enables NVIC
*
* @details:
* It will register the callback, i.e., stores the callback function address
* and pass to the variable that will be called in Interrupt Handler.
*
* If any callback is already registered, the user needs to unregister the
* callback first before registering another callback.
*
* Maps timer interrupt to their respective IRQ numbers(Enables NVIC),
* so should be called before registering timeout-callbacks
*******************************************************************************/
sl_status_t sl_si91x_watchdog_register_timeout_callback(watchdog_timer_callback_t on_timeout_callback)
{
  sl_status_t status;
  // Initializing SL status variable with OK status
  status = SL_STATUS_OK;
  do {
    // Validating the 'on_timeout_callback function pointer' parameter
    // if it is NULL will return an error code
    if (on_timeout_callback == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // To validate the function pointer, if the parameters is not NULL then it
    //will return an busy error code
    if (timeout_callback_function_pointer != NULL) {
      status = SL_STATUS_BUSY;
      break;
    }
    NVIC_EnableIRQ(NVIC_WDT);
    // The function pointer is fed to the static variable, which will be called in the IRQ handler
    timeout_callback_function_pointer = on_timeout_callback;
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Sets watchdog-timer overflow time (also restart-time) duration
*
* @details:
* This API sets the time by which watchdog-timer timeout time, by updating
* 'WWD_INTERRUPT_TIMER' register (5-bit), so its maximum value is 31.
*
* It is used to calculate clock pulse count of FSM clock, for timer overflow or
* timer restart.
*
* Number of clock pulses for timer timeout = 2^(interrupt_time)
*******************************************************************************/
sl_status_t sl_si91x_watchdog_set_interrupt_time(time_delays_t interrupt_time)
{
  sl_status_t status;
  status = SL_STATUS_OK;
  do {
    // Validating interrupt time value
    if (interrupt_time >= TIME_DELAY_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validating interrupt time as per window time value
    if (RSI_WWDT_GetWindowTime(MCU_WDT) >= interrupt_time) {
      status = SL_STATUS_INVALID_CONFIGURATION;
      break;
    }
    // Setting timer interrupt time
    RSI_WWDT_ConfigIntrTimer(MCU_WDT, (uint16_t)interrupt_time);
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Sets watchdog-timer window time duration
*
* @details:
* This API sets the time by which watchdog-timer resets system, by updating
* 'WWD_WINDOW_TIMER' register (4-bit), so its maximum value is 15.
*
* The window watchdog-timer (WDT) has two time stamps within them timer restart is allowed:
* First is the dedicated selected time after WDT start and second is the overflow time,
* the so called "open window". When a restart is triggered during this time frame, WDT restart is done.
* If the restart is triggered outside this window, a reset of the micro-controller is
* initiated.
*
* It is used to calculate clock pulse count of FSM clock, for WDT window time
*  Number of clock pulses for window time = 2^(window_time).
*
* This value should be less than timer interrupt time and system-reset time
*******************************************************************************/
sl_status_t sl_si91x_watchdog_set_window_time(time_delays_t window_time)
{
  sl_status_t status;
  status = SL_STATUS_OK;
  // Validating Window time value
  do {
    // Validating Window time value
    if (window_time >= TIME_DELAY_16) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validating window time as per interrupt time value
    if (RSI_WWDT_GetIntrTime(MCU_WDT) <= window_time) {
      status = SL_STATUS_INVALID_CONFIGURATION;
      break;
    }
    // Validating window time as per system reset time value
    if (RSI_WWDT_GetSysRstTime(MCU_WDT) <= window_time) {
      status = SL_STATUS_INVALID_CONFIGURATION;
      break;
    }
    // Setting timer window time
    RSI_WWDT_ConfigWindowTimer(MCU_WDT, (uint8_t)window_time);
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Sets watchdog-timer system-reset time duration.
*
* @details:
* This API sets the time by which watchdog-timer resets system, by updating
* 'WWD_SYSTEM_RESET_TIMER' register (5-bit), so its maximum value is 31.
*
* It is used to calculate clock pulse count of FSM clock, to reset the system
* Number of clock pulses for system reset delay = 2^(system_reset_time)
* This value should be greater than timer window time.
*******************************************************************************/
sl_status_t sl_si91x_watchdog_set_system_reset_time(time_delays_t system_reset_time)
{
  sl_status_t status;
  status = SL_STATUS_OK;
  do {
    // Validating system-reset time value
    if (system_reset_time >= TIME_DELAY_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validating system reset time as per window time value
    if (RSI_WWDT_GetWindowTime(MCU_WDT) >= system_reset_time) {
      status = SL_STATUS_INVALID_CONFIGURATION;
      break;
    }
    // Setting system reset time
    RSI_WWDT_ConfigSysRstTimer(MCU_WDT, (uint16_t)system_reset_time);
  } while (false);
  return status;
}

/*******************************************************************************
* @brief: Unregisters Callback on timer expiration and disables interrupts
*
* @details:
* It will unregister the callback, i.e., clear the callback function address
* and pass NULL value to the variable
*******************************************************************************/
void sl_si91x_watchdog_unregister_timeout_callback(void)
{
  //The callback should be null in the unregister callback api because
  //one cannot register the callback if it is not null
  timeout_callback_function_pointer = NULL;
}

/*******************************************************************************
* @brief: Reads watchdog-timer system-reset status
*
* @details:
* It checks the status of 5th bit of wake up/ NPSS common interrupt status register,
* which is true when watchdog-timer resets processor and returns false when it is
* a power-on reset
*******************************************************************************/
boolean_t sl_si91x_watchdog_get_timer_system_reset_status(void)
{
  boolean_t reset_status = false;
  if (RSI_PS_GetComnIntrSts() & WATCH_DOG_WINDOW_RESET_INTERRUPT) {
    reset_status = true;
  } else {
    reset_status = false;
  }
  return reset_status;
}

/*******************************************************************************
* @brief: De-initializes watchdog-timer
*
* @details:
* It will unregister the timer callback
* It masks the timer interrupt.
* Disables the timer and de-powers it.
*******************************************************************************/
void sl_si91x_watchdog_deinit_timer(void)
{
  // Unregistering the callback
  sl_si91x_watchdog_unregister_timeout_callback();
  // Masking the interrupt
  RSI_WWDT_IntrMask();
  // Disabling the timer
  RSI_WWDT_DeInit(MCU_WDT);
  // Disabling WDT power
  RSI_PS_NpssPeriPowerDown(SLPSS_PWRGATE_ULP_MCUWDT);
}

/*******************************************************************************
* @brief: To read interrupt(timeout) time value(in terms of power of two)
*
* @details:
* It returns the set interrupt time value (not current count)
*******************************************************************************/
uint8_t sl_si91x_watchdog_get_interrupt_time(void)
{
  uint8_t interrupt_time;
  // Reading set interrupt time value
  interrupt_time = RSI_WWDT_GetIntrTime(MCU_WDT);
  return interrupt_time;
}

/*******************************************************************************
* @brief: To read system_reset time value(in terms of power of two)
*
* @details:
* It returns the set system_reset time value (not current count)
*******************************************************************************/
uint8_t sl_si91x_watchdog_get_system_reset_time(void)
{
  uint8_t sys_rst_time;
  // Reading set system reset time value
  sys_rst_time = RSI_WWDT_GetSysRstTime(MCU_WDT);
  return sys_rst_time;
}

/*******************************************************************************
* @brief: To read window time value (in terms of power of two)
*
* @details:
* It returns the set window time value (not current count)
*******************************************************************************/
uint8_t sl_si91x_watchdog_get_window_time(void)
{
  uint8_t window_time;
  // Reading set window time value
  window_time = RSI_WWDT_GetWindowTime(MCU_WDT);
  return window_time;
}

/*******************************************************************************
 * To get the release, sqa and dev version of Watchdog-timer
 * It returns the structure for watchdog-timer version.
 * Structure includes three members:
 * - Release version
 * - Major version (SQA version)
 * - Minor version (Dev version)
 ******************************************************************************/
sl_watchdog_timer_version_t sl_si91x_watchdog_get_version()
{
  sl_watchdog_timer_version_t version;
  version.release = WATCHDOG_TIMER_RELEASE_VERSION;
  version.major   = WATCHDOG_TIMER_MAJOR_VERSION;
  version.minor   = WATCHDOG_TIMER_MINOR_VERSION;
  return version;
}
/*******************************************************************************
* Interrupt handler for watchdog-timer
*
* @param none
* @return none
*******************************************************************************/
void WATCHDOG_TIMER_IRQHandler(void)
{
  // Clears interrupt
  RSI_WWDT_IntrClear();
  // calling the timeout callback function
  timeout_callback_function_pointer();
}
