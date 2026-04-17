/*******************************************************************************
 * @file  at_m4_power_save.c
 * @brief Powersave AT Commands
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifdef AT_M4_POWER_SAVE
#include <string.h>
#include "cmsis_os2.h"
#include "sl_status.h"
#include "sl_wifi.h"
#include "sl_wifi_device.h"
#include "sl_net_wifi_types.h"
#include "sl_si91x_driver.h"
#include "rsi_rtc.h"

#include "rsi_wisemcu_hardware_setup.h"
#include "rsi_power_save.h"
#include "sli_siwx917_soc.h"
#include "console.h"
#include "rsi_debug.h"
#include "rsi_m4.h"
#include "sl_si91x_host_interface.h"
#include "rsi_ps_config.h"

#include "sl_si91x_power_manager.h"
#include "sl_si91x_calendar.h"
#include "sl_si91x_m4_ps.h"
#include "at_utility.h"

#define NO_OF_MSEC_IN_SEC       1000 // Number of milliseconds in second
#define NO_OF_SEC_IN_MIN        60   // Number of seconds in a minute
#define NO_OF_MIN_IN_HOUR       60   // Number of minutes in an hour
#define NO_OF_HOUR_IN_DAY       24   // Number of hour in a day
#define NO_OF_DAY_IN_MONTH_1    28   // Number of days in february month
#define NO_OF_DAY_IN_MONTH_2    29   // Number of days in february month which has leap year
#define NO_OF_DAY_IN_MONTH_3    30   // Number of days in even month
#define NO_OF_DAY_IN_MONTH_4    31   // Number of days in odd month
#define NO_OF_MONTH_IN_YEAR     12   // Number of months in a year
#define NEXT_OCCURENECE         1    // Next time occurence
#define RESET_DAY               1    // Reset day after incrementing month
#define RESET_MONTH             1    // Reset month after incrementing year
#define LEAP_YEAR_VALIDATION    4    // Value to validate the leap year
#define DAY_IN_MONTH_VALIDATION 2    // Value to validate the days in month

#define ALARM_TIME_MSEC 5000

/******************************************************
*               Function Declarations
******************************************************/
void calendar_callback_function_pm(void);
void set_periodic_alarm_pm(uint32_t alarm_time);
sl_status_t set_wakeup_source_handler(uint32_t period_ms);
void set_alarm_interrupt_timer(uint16_t interval);

/******************************************************
*               Variable Definitions
******************************************************/
static uint32_t time_ms                              = ALARM_TIME_MSEC;
static sl_calendar_datetime_config_t datetime_config = { 0 };
static bool calender_start                           = false;
/******************************************************
*               Function Definitions
******************************************************/
#ifdef SLI_SI91X_MCU_INTERFACE
sl_status_t m4_powersave_command_handler(void)
{
#if SL_SI91X_MCU_ALARM_BASED_WAKEUP
  initialize_m4_alarm();
#endif

#if (defined(SLI_SI91X_MCU_INTERFACE) && (SL_SI91X_TICKLESS_MODE == 0))
  printf("\r\nM4 in sleep\r\n");
  sl_si91x_power_manager_sleep();
  printf("\r\nM4 wake up\r\n");
#endif

  return SL_STATUS_OK;
}

// at+xtal-enable=<enable-xtal-flag>
sl_status_t m4_ta_secure_handshake_command_handler(const console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);
  uint8_t xtal_enable = (uint8_t)GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint8_t);

  sl_status_t status = sl_si91x_m4_ta_secure_handshake(SL_SI91X_ENABLE_XTAL, 1, &xtal_enable, 0, NULL);
  if (status != SL_STATUS_OK) {
    return status;
  }

  PRINT_AT_CMD_SUCCESS;
  return status;
}
#endif

sl_status_t m4_power_manager_handler(console_args_t *arguments)
{
  time_ms            = (uint32_t)GET_OPTIONAL_COMMAND_ARG(arguments, 0, ALARM_TIME_MSEC, uint32_t);
  sl_status_t status = SL_STATUS_OK;
  osSemaphoreId_t wait_semaphore;
  wait_semaphore = osSemaphoreNew(1, 0, NULL);
  if (wait_semaphore == NULL) {
    printf("Failed to create semaphore\r\n");
    return SL_STATUS_FAIL;
  }

  printf("\r\nM4 in sleep\r\n");
  // Waiting for time_ms using semaphore to put M4 to sleep in tick less mode
  osSemaphoreAcquire(wait_semaphore, time_ms);
  printf("\r\nM4 wake up\r\n");

  PRINT_AT_CMD_SUCCESS;
  return status;
}

sl_status_t m4_power_manager_deep_sleep_handler(console_args_t *arguments)
{
  sl_status_t status     = SL_STATUS_OK;
  sl_power_state_t state = (uint8_t)GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_SI91X_POWER_MANAGER_PS0, uint8_t);
  time_ms                = (uint32_t)GET_OPTIONAL_COMMAND_ARG(arguments, 1, ALARM_TIME_MSEC, uint32_t);
#ifdef SLI_SI91X_MCU_INTERFACE
  status = set_wakeup_source_handler(time_ms);
  printf("\r\nM4 in sleep\r\n");
  sl_si91x_power_manager_add_ps_requirement(state);
  printf("\r\nM4 wake up\r\n");
#endif

  PRINT_AT_CMD_SUCCESS;
  return status;
}

// at+cal-stop
sl_status_t power_manager_calender_stop_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  if (calender_start == false) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sl_status_t status;

  // Calendar clock is stopped as it is not needed.
  sl_si91x_calendar_rtc_stop();
  // Second trigger callback is unregistered and disabled.
  status = sl_si91x_calendar_unregister_sec_trigger_callback();
  VERIFY_STATUS_AND_RETURN(status);

  calender_start = false;
  PRINT_AT_CMD_SUCCESS;
  return status;
}

// at+cal-start=<time_ms>
sl_status_t power_manager_calender_start_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);
  UNUSED_PARAMETER(arguments);

  if (calender_start == true) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  sl_status_t status;
  time_ms = GET_OPTIONAL_COMMAND_ARG(arguments, 0, ALARM_TIME_MSEC, uint32_t);

  // Calendar is initialized.
  sl_si91x_calendar_init();
  // Current date time is fetched
  status = sl_si91x_calendar_get_date_time(&datetime_config);
  VERIFY_STATUS_AND_RETURN(status);

  calender_start = true;
  // Periodic alarm setting API is called.
  set_periodic_alarm_pm(time_ms);

  // Clear any existing alarm callback
  status = sl_si91x_calendar_unregister_alarm_trigger_callback();
  VERIFY_STATUS_AND_RETURN(status);

  // Alarm callback is registered
  status = sl_si91x_calendar_register_alarm_trigger_callback(calendar_callback_function_pm);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return status;
}

// at+wifisys-wake-src=<source>,<add>
sl_status_t power_manager_set_wakeup_source_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);
  UNUSED_PARAMETER(arguments);
  sl_status_t status;
  uint32_t source = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint32_t);
  uint8_t add     = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 1, uint8_t);

  switch (source) {
    case SL_SI91X_POWER_MANAGER_ALARM_WAKEUP:
      // Alarm based wakeup source is configured
      status = sl_si91x_power_manager_set_wakeup_sources(SL_SI91X_POWER_MANAGER_ALARM_WAKEUP, add);
      VERIFY_STATUS_AND_RETURN(status);
      break;
    default:
      return SL_STATUS_INVALID_PARAMETER;
  }

  PRINT_AT_CMD_SUCCESS;
  return status;
}

// at+wifisys-psreq?
sl_status_t power_manager_get_ps_requirement_table_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

  PRINT_AT_CMD_SUCCESS;
  uint8_t *get_table = sl_si91x_power_manager_get_requirement_table();
  for (uint8_t i = SL_SI91X_POWER_MANAGER_PS0; i <= SL_SI91X_POWER_MANAGER_PS4; i++) {
    printf("%d ", get_table[i]);
  }

  return SL_STATUS_OK;
}

// at+wifisys-psreq=<state>,<add>
sl_status_t power_manager_config_ps_requirement_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);
  UNUSED_PARAMETER(arguments);
  sl_status_t status;

  uint16_t state = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint16_t);
  uint8_t add    = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint8_t);

  if (add == 0) {
    status = sl_si91x_power_manager_remove_ps_requirement(sl_si91x_power_manager_get_current_state());
    VERIFY_STATUS_AND_RETURN(status);
  } else {
    set_periodic_alarm_pm(time_ms);
    status = sl_si91x_power_manager_add_ps_requirement(state);
    VERIFY_STATUS_AND_RETURN(status);
  }

  PRINT_AT_CMD_SUCCESS;
  return status;
}

sl_status_t set_wakeup_source_handler(uint32_t period_ms)
{
  sl_status_t status = SL_STATUS_OK;

  // Calendar is initialized.
  sl_si91x_calendar_init();
  // Current date time is fetched
  status = sl_si91x_calendar_get_date_time(&datetime_config);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Periodic alarm setting API is called.
  set_periodic_alarm_pm(period_ms);

  // Alarm callback is registered
  status = sl_si91x_calendar_register_alarm_trigger_callback(calendar_callback_function_pm);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Alarm based wakeup source is configured
  status = sl_si91x_power_manager_set_wakeup_sources(SL_SI91X_POWER_MANAGER_ALARM_WAKEUP, true);
  if (status != SL_STATUS_OK) {
    return status;
  }

  return status;
}

void set_periodic_alarm_pm(uint32_t alarm_time)
{
  sl_calendar_datetime_config_t set_alarm_config, get_datetime_config;
  sl_si91x_calendar_get_date_time(&get_datetime_config);
  set_alarm_config = get_datetime_config;

  if (calender_start == false) {
    return;
  }
  set_alarm_config.MilliSeconds = set_alarm_config.MilliSeconds + (alarm_time % NO_OF_MSEC_IN_SEC);
  if (set_alarm_config.MilliSeconds >= NO_OF_MSEC_IN_SEC) {
    set_alarm_config.MilliSeconds -= NO_OF_MSEC_IN_SEC;
    set_alarm_config.Second += NEXT_OCCURENECE;
  }

  alarm_time /= NO_OF_MSEC_IN_SEC;
  set_alarm_config.Second = set_alarm_config.Second + (alarm_time % NO_OF_SEC_IN_MIN);
  if (set_alarm_config.Second >= NO_OF_SEC_IN_MIN) {
    set_alarm_config.Second -= NO_OF_SEC_IN_MIN;
    set_alarm_config.Minute += NEXT_OCCURENECE;
  }

  alarm_time /= NO_OF_SEC_IN_MIN;
  set_alarm_config.Minute = set_alarm_config.Minute + (alarm_time % NO_OF_MIN_IN_HOUR);
  if (set_alarm_config.Minute >= NO_OF_MIN_IN_HOUR) {
    set_alarm_config.Minute -= NO_OF_MIN_IN_HOUR;
    set_alarm_config.Hour += NEXT_OCCURENECE;
  }

  alarm_time /= NO_OF_MIN_IN_HOUR;
  set_alarm_config.Hour = set_alarm_config.Hour + alarm_time;
  if (set_alarm_config.Hour >= NO_OF_HOUR_IN_DAY) {
    set_alarm_config.Hour -= NO_OF_HOUR_IN_DAY;
    set_alarm_config.Day += NEXT_OCCURENECE;
  }

  if (set_alarm_config.Day > NO_OF_DAY_IN_MONTH_1) {
    if (set_alarm_config.Month == February) {
      if (set_alarm_config.Year % LEAP_YEAR_VALIDATION) {
        set_alarm_config.Day = RESET_DAY;
        set_alarm_config.Month += NEXT_OCCURENECE;
      } else if (set_alarm_config.Day > NO_OF_DAY_IN_MONTH_2) {
        set_alarm_config.Day = RESET_DAY;
        set_alarm_config.Month += NEXT_OCCURENECE;
      }
    }

    if (set_alarm_config.Month <= July) {
      if (!(set_alarm_config.Month % DAY_IN_MONTH_VALIDATION)) {
        if (set_alarm_config.Day > NO_OF_DAY_IN_MONTH_3) {
          set_alarm_config.Day = RESET_DAY;
          set_alarm_config.Month += NEXT_OCCURENECE;
        } else if (set_alarm_config.Day > NO_OF_DAY_IN_MONTH_4) {
          set_alarm_config.Day = RESET_DAY;
          set_alarm_config.Month += NEXT_OCCURENECE;
        }
      }
    } else if (!(set_alarm_config.Month % DAY_IN_MONTH_VALIDATION)) {
      if (set_alarm_config.Day > NO_OF_DAY_IN_MONTH_4) {
        set_alarm_config.Day = RESET_DAY;
        set_alarm_config.Month += NEXT_OCCURENECE;
      } else if (set_alarm_config.Day > NO_OF_DAY_IN_MONTH_3) {
        set_alarm_config.Day = RESET_DAY;
        set_alarm_config.Month += NEXT_OCCURENECE;
      }
    }
  }

  if (set_alarm_config.Month > NO_OF_MONTH_IN_YEAR) {
    set_alarm_config.Month = RESET_MONTH;
    set_alarm_config.Year += NEXT_OCCURENECE;
  }

  sl_si91x_calendar_set_alarm(&set_alarm_config);
}

void calendar_callback_function_pm(void)
{
  set_periodic_alarm_pm(time_ms);
}

// at+wifisys-ps?
sl_status_t power_manager_get_current_state_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

  PRINT_AT_CMD_SUCCESS;
  printf("%d \r\n", sl_si91x_power_manager_get_current_state());
  return SL_STATUS_OK;
}

// at+wifisys-sleep
sl_status_t set_power_manager_sleep_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

#if (defined(SLI_SI91X_MCU_INTERFACE) && (SL_SI91X_TICKLESS_MODE == 0))
  // Call the sleep function.
  sl_status_t status = sl_si91x_power_manager_sleep();
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    SL_DEBUG_LOG("sl_si91x_power_manager_sleep failed, Error Code: 0x%lX \n", status);
    return status;
  }
#else
  osSemaphoreId_t wait_semaphore;
  wait_semaphore = osSemaphoreNew(1, 0, NULL);
  if (wait_semaphore == NULL) {
    SL_DEBUG_LOG("Failed to create semaphore\r\n");
    return SL_STATUS_FAIL;
  }

  SL_DEBUG_LOG("\r\nM4 in sleep\r\n");
  // Waiting for time_ms using semaphore to put M4 to sleep in tick less mode
  osSemaphoreAcquire(wait_semaphore, time_ms);
  SL_DEBUG_LOG("\r\nM4 wake up\r\n");
#endif
  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+at+wifisys-standby
sl_status_t set_power_manager_standby_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

#if (defined(SLI_SI91X_MCU_INTERFACE) && (SL_SI91X_TICKLESS_MODE == 0))
  // Call the standby function
  sl_si91x_power_manager_standby();
#endif

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}
#endif // AT_M4_POWER_SAVE