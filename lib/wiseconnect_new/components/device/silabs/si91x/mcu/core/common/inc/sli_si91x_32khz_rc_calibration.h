/******************************************************************************
 * @file sli_si91x_32khz_rc_calibration.h
 * @brief 32KHz RC Calibration API implementation
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
 ******************************************************************************/
#ifndef SL_SI91X_32KHZ_RC_CALIBRATION_H_
#define SL_SI91X_32KHZ_RC_CALIBRATION_H_

#ifdef __cplusplus
extern "C" {
#endif
#if (SL_SI91X_32KHZ_RC_CALIBRATION_ENABLED)
#include <stdlib.h>
#include <stdbool.h>
#include "sl_status.h"
#include "si91x_device.h"
#include "sl_si91x_calendar.h"
#include "rsi_time_period.h"
#if defined(SL_SI91X_POWER_MANAGER_UC_AVAILABLE)
#include "sl_si91x_power_manager_wakeup_handler.h"
#endif

// Flag to indicate whether the interrupt event is for a calibration event. By default, it is set to true, as calibration is required for the first time.
static bool sli_si91x_calibration_interrupt_flag = true;
// Flag to indicate whether the interrupt event is for an alarm event.
static bool sli_si91x_callback_trigger_flag = false;

/***************************************************************************/
/**
 * @brief Initializes the calibration framework.
 *
 * @details This function sets up the calibration framework and applies the calibration for the first time.
 * It ensures that the calibration process is ready for subsequent operations.
 *
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 ******************************************************************************/
sl_status_t sli_si91x_calibration_initialization(void);

/***************************************************************************/
/**
 * @brief Performs low-frequency FSM RC calibration.
 *
 * @details This function handles the low-frequency FSM RC calibration process.
 * It ensures that the RC oscillator is calibrated for accurate timing operations.
 *
 * @return None.
 ******************************************************************************/
void sli_si91x_lf_fsm_rc_calibration(void);

/***************************************************************************/
/**
 * @brief Configures the RTC clock time period.
 *
 * @details This API sets the RTC clock time period using the provided parameters.
 *
 * @param[in] pstcTimePeriod(*pstcTimePeriod) is pointer to the time period calibration registration instance.
 * @param[in] u32TimePeriod(u32TimePeriod) is RTC time period in pico seconds as 32Khz as reference
 *
 * @return  status 0 if successful, else error code as follows:
 *         - SL_STATUS_OK (0x0000) - Success
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - Parameters are invalid
 *  *********************************************************************************************/
sl_status_t sli_si91x_rtc_timeperiod(TIME_PERIOD_Type *pstcTimePeriod, uint32_t u32TimePeriod);

/******************************************************************************
 * This API restarts the calibration counter and triggers the calibration again.
 * @param none
 * @return none
 ******************************************************************************/
void sli_si91x_restart_calibration_counter(sl_calendar_datetime_config_t *config);

/******************************************************************************
 * This API adjusts the alarm time value to match the calibration framework.
 * @param none
 * @return none
 ******************************************************************************/
void sli_si91x_calibration_adjustment(sl_calendar_datetime_config_t *alarm);

/******************************************************************************
 * @brief Sets the alarm time in milliseconds.
 *
 * @details This API configures the alarm time in milliseconds. The specified
 *          time will be used to trigger the alarm event at the appropriate moment.
 *
 * @param none
 * @return none
 ******************************************************************************/
void sli_si91x_set_periodic_alarm(uint32_t alarm_time);

/***************************************************************************/
/**
 * @brief Handles the next calibration event.
 *
 * @details This API is used to get the next event time. It uses 2 flags ( sli_si91x_calibration_interrupt_flag and sli_si91x_callback_trigger_flag) to indicate that the next time is for the calibration or alarm event.
 * Calibration event handler handles the next event time for both periodic trigger (Alarm wakeup source) and one shot trigger (Alarm peripheral).
 *
 * @param[in] sl_si91x_mode  Mode to determine the event type.
 * 1 - An adjustment is required, here may be  difference between the alarm counter start and the configured alarm time. (Alarm Peripheral or Alarm Wakeup source without UC - User can start the alarm counter and set the alarm value at any time).
 * 0 - No adjustments is required. (Alarm Wakeup source without UC- we will start the alarm counter and set the alarm value consecutively).
 *
 * @return The time until the next event time in milliseconds.
 ******************************************************************************/
uint32_t sli_si91x_get_calib_next_event_time_handler(bool sl_si91x_mode);

/****************************************************************************/
/**
 * @brief Sets the remaining time variable for the calibration event handler.
 *
 * @details This API sets the alarm time to variable of the event handler. Initially, the remaining time will be set to the maximum value between the alarm time and the calibration time. 
 * The event handler will update the value, which the calibration event handler will then use to determine the next event.
 *
 * @param[in] alarm_in_millisecond (uint32_t) The alarm time in milliseconds.
 *
 * @return None.
 ******************************************************************************/
void sli_si91x_set_remaining_time(uint32_t alarm_in_milisecond);

/***************************************************************************/
/**
 * @brief Sets the calibration application flag.
 *
 * @details The current calibration framework includes certain calculations in the set alarm and set date APIs.
 * This flag is used to differentiate whether the set date-time API or set alarm API is being called by the application or the driver.
 * The flag being set indicates that set date-time API or set alarm API is being invoked application.
 *
 * @return None.
 ******************************************************************************/
void sli_si91x_set_calibration_application_flag(void);

/***************************************************************************/
/**
 * @brief Clears the calibration application flag.
 *
 * @details The current calibration framework includes certain calculations in the set alarm and set date APIs.
 * This flag is used to differentiate whether the set date-time API or set alarm API is being called by the application or the driver.
 * The flag being cleared indicates that set date-time API or set alarm API is being invoked by driver.
 *
 * @return None.
 ******************************************************************************/
void sli_si91x_clear_calibration_application_flag(void);

/***************************************************************************
 * @brief Retrieves the current state of the calibration application flag.
 *
 * @details This function checks whether the calibration application flag is set.
 *          The flag indicates whether the set date-time API or set alarm API 
 *          is being invoked by the application or the driver.
 * @return The status of the Calibration application flag (true or false).
 ***************************************************************************/
bool sli_si91x_get_calendar_application_flag_status(void);

/***************************************************************************/
/**
 * @brief Sets the calendar wakeup peripheral flag.
 *
 * @details This function sets the flag indicating that the calendar alarm is  configured as a calendar wakeup source.
 *
 * @return None.
 ******************************************************************************/
void sli_si91x_set_calendar_wakeup_peripheral_flag(void);

/***************************************************************************/
/**
 * @brief Clears the calendar wakeup peripheral flag.
 *
 * @details This function clears the flag indicating that the calendar alarm is configured as a calendar peripheral.
 *
 * @return None.
 ******************************************************************************/
void sli_si91x_clear_calendar_wakeup_peripheral_flag(void);

/**********************************************************************************************
 * @brief Enables both alarm and calibration events flags.
 *
 * @details This function sets the flags to enable both alarm and calibration events.
 * It ensures that both events are triggered as required.
 *
 * @param none
 * @return none
 **********************************************************************************************/

__STATIC_INLINE void sli_si91x_enable_alarm_and_calibration_flag(void)
{
  sli_si91x_calibration_interrupt_flag = 1;
  sli_si91x_callback_trigger_flag      = 1;
}
/**********************************************************************************************
 * @brief Enables alarm and disables calibration flags.
 *
 * @details This function sets the flags to enable the alarm event while disabling the calibration event.
 * It ensures that only the alarm event is triggered.
 *
 * @param none
 * @return none
 **********************************************************************************************/
__STATIC_INLINE void sli_si91x_enable_alarm_disable_calibration_flag(void)
{
  sli_si91x_calibration_interrupt_flag = 0;
  sli_si91x_callback_trigger_flag      = 1;
}

/**********************************************************************************************
 * @brief Disables alarm and enables calibration flags.
 *
 * @details This function sets the flags to disable the alarm event while enabling the calibration event.
 * It ensures that only the calibration event is triggered.
 *
 * @param none
 * @return none
 **********************************************************************************************/
__STATIC_INLINE void sli_si91x_disable_alarm_enable_calibration_flag(void)
{
  sli_si91x_calibration_interrupt_flag = 1;
  sli_si91x_callback_trigger_flag      = 0;
}

/***************************************************************************
 * @brief Retrieves the status of the calibration interrupt flag.
 *
 * @details This function returns the current state of the calibration interrupt flag,
 *          which indicates whether a calibration event needs to be triggered.
 *
 * @param none
 * @return The status of the calibration interrupt flag (true or false).
 ******************************************************************************/
bool sli_si91x_get_calibration_interrupt_flag_status(void);

/***************************************************************************
 * @brief Retrieves the status of the callback trigger flag.
 *
 * @details This function returns the current state of the callback trigger flag,
 *          which indicates whether a callback event needs to be triggered.
 *
 * @param none
 * @return The status of the callback trigger flag (true or false).
 ******************************************************************************/
bool sli_si91x_get_callback_trigger_flag_status(void);

/***************************************************************************
 * @brief Clears the wakeup overhead flag.
 *
 * @details This function clears the wakeup overhead flag, indicating that 
 *          the system does not need to adjust the wakeup overhead.
 *
 * @param none
 * @return none
 ***************************************************************************/
void sli_si91x_clear_wakeup_overhead_flag(void);

/***************************************************************************
 * @brief Sets the wakeup overhead flag.
 *
 * @details This function sets the wakeup overhead flag, indicating that 
 *          the system needs to adjust the wakeup overhead.
 *
 * @param none
 * @return none
 ***************************************************************************/
void sli_si91x_set_wakeup_overhead_flag(void);
#endif
#ifdef __cplusplus
}
#endif

#endif //SL_SI91X_32KHZ_RC_CALIBRATION_H_