/******************************************************************************
 * @file sli_si91x_32khz_rc_calibration.c
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
 *
 ******************************************************************************/
#if (SL_SI91X_32KHZ_RC_CALIBRATION_ENABLED)
#include "sli_si91x_32khz_rc_calibration.h"
// This is will be used for the RC Calibration framework.
// The users are strictly suggested to not use this functions.
/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/
// The thresold time between alarm and calibration in milisconds. If the subsequent alarm and calibration event occurs within this threshold, both the alarm and calibration will be activated together.
#define SLI_SI91X_ALARM_CALIBRATION_THRESHOLD 10
// Updated calibration time period register.
#define SL_SI91X_CALIBRATED_TIMEPERIOD (*(volatile uint32_t *)0x24048214) & 0xFFFFFF
// Default value to intialize the event handler structure.
#define DEFAULT_DATE_TIME_INIT                                                                          \
  {                                                                                                     \
    .Century = 0, .Year = 0, .Month = 0, .DayOfWeek = 0, .Day = 0, .Hour = 0, .Minute = 0, .Second = 0, \
    .MilliSeconds = 0                                                                                   \
  }
// Respective defines to set the alarm.
#define SLI_SI91X_NO_OF_MSEC_IN_SEC       1000 // Number of milliseconds in second
#define SLI_SI91X_NO_OF_SEC_IN_MIN        60   // Number of seconds in a minute
#define SLI_SI91X_NO_OF_MIN_IN_HOUR       60   // Number of minutes in an hour
#define SLI_SI91X_NO_OF_HOUR_IN_DAY       24   // Number of hour in a day
#define SLI_SI91X_NO_OF_DAY_IN_MONTH_1    28   // Number of days in february month
#define SLI_SI91X_NO_OF_DAY_IN_MONTH_2    29   // Number of days in february month which has leap year
#define SLI_SI91X_NO_OF_DAY_IN_MONTH_3    30   // Number of days in even month
#define SLI_SI91X_NO_OF_DAY_IN_MONTH_4    31   // Number of days in odd month
#define SLI_SI91X_NO_OF_MONTH_IN_YEAR     12   // Number of months in a year
#define SLI_SI91X_NEXT_OCCURENECE         1    // Next time occurence
#define SLI_SI91X_RESET_DAY               1    // Reset day after incrementing month
#define SLI_SI91X_RESET_MONTH             1    // Reset month after incrementing year
#define SLI_SI91X_LEAP_YEAR_VALIDATION    4    // Value to validate the leap year
#define SLI_SI91X_DAY_IN_MONTH_VALIDATION 2    // Value to validate the days in month

#define SLI_SI91X_CENTURY       2  // Default Century to be configured
#define SLI_SI91X_YEAR          1  // Default year to be configured
#define SLI_SI91X_DAY           2  // Default day to be configured
#define SLI_SI91X_HOUR          18 // Default hour to be configured
#define SLI_SI91X_MINUTE        10 // Default minute to be configured
#define SLI_SI91X_SECONDS       10 // Default seconds to be configured
#define SLI_SI91X_MILLI_SECONDS 0  // Default milli seconds to be configured

// The following is the wakeup overhead in miliseconds caused due to bootloader execution.
#define SLI_SI91X_WAKEUP_OVERHEAD 4
#define SLI_SI91X_NO_OVERHEAD     0
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
// Flag to differentiate between wake up source or peripheral trigger.
static bool sli_si91x_calendar_wakeup_peripheral_flag = false;
// This flag is used to differentiate whether the set date-time API or set alarm API is being called by the application or the driver. By default value is set to true, indicating that set date-time API or set alarm API is being invoked application.
static bool sli_si91x_calendar_application_flag = true;
// Configurable value that stores the alarm trigger time in milliseconds.
static uint32_t sli_si91x_alarm_time = 0;
// Fixed value representing the calibration trigger time in milliseconds.
static uint32_t sli_si91x_calib_time = 1800000;
// Stores the remaining time to determine whether to trigger an alarm or a calibration event.
static uint32_t sli_si91x_remaining_time = 0;
// This variable stores the remaining time left to apply the calibration when the calendar peripheral or wakeup source without UC is configured.
static uint32_t sli_si91x_remaining_calib_time = 0;
// This variable stores the remaining time left to trigger the alarm when the calendar peripheral or wakeup source without UC is configured.
static uint32_t sli_si91x_remaining_alarm_time = 0;
// Holds the last recorded value of the remaining time, used for subsequent event cycles.
static uint32_t sli_si91x_last_value = 0;
// This flag indicates whether the m4 wakeup overhead.
static uint8_t sli_si91x_wakeup_overhead_flag = 0;

// calendar variable required by the calibration event handler
typedef struct {
  sl_calendar_datetime_config_t *sli_si91x_default_date_time;
  sl_calendar_datetime_config_t *sli_si91x_last_calib_time;
  sl_calendar_datetime_config_t *sli_si91x_current_time;
  sl_calendar_datetime_config_t *sli_si91x_set_alarm_time;
  uint32_t sli_si91x_default_ntp_time;
  uint32_t sli_si91x_last_calib_ntp_time;
  uint32_t sli_si91x_current_ntp_time;
  uint32_t sli_si91x_alarm_ntp_time;
} sl_si91x_calendar_times_t;

// Initialize the calendar variable required by the calibration event handler.
static sl_calendar_datetime_config_t default_date_time = DEFAULT_DATE_TIME_INIT;
static sl_calendar_datetime_config_t last_calib_time   = DEFAULT_DATE_TIME_INIT;
static sl_calendar_datetime_config_t current_time      = DEFAULT_DATE_TIME_INIT;
static sl_calendar_datetime_config_t set_alarm_time    = DEFAULT_DATE_TIME_INIT;

// Static instance of the structure with default values.
static sl_si91x_calendar_times_t si91x_calendar_data = { .sli_si91x_default_date_time   = &default_date_time,
                                                         .sli_si91x_last_calib_time     = &last_calib_time,
                                                         .sli_si91x_current_time        = &current_time,
                                                         .sli_si91x_set_alarm_time      = &set_alarm_time,
                                                         .sli_si91x_default_ntp_time    = 0,
                                                         .sli_si91x_last_calib_ntp_time = 0,
                                                         .sli_si91x_current_ntp_time    = 0,
                                                         .sli_si91x_alarm_ntp_time      = 0 };

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
static uint32_t sli_si91x_handle_alarm_greater_than_calibration_event(bool sl_si91x_mode);
static uint32_t sli_si91x_handle_alarm_smaller_than_calibration_event(bool sl_si91x_mode);
static uint8_t sli_si91x_delay_handling();
/******************************************************************************
* Initialize the calibration framework and apply the calibration for the first time

* @return  status 0 if successful, else error.
******************************************************************************/
sl_status_t sli_si91x_calibration_initialization(void)
{
  sl_status_t status = SL_STATUS_OK;
// Initialize the calibration and apply it for the first time.
#if defined(SL_SI91X_POWER_MANAGER_UC_AVAILABLE) && defined(ENABLE_ALARM) && (ENABLE_ALARM == ENABLE) // alarm
  // Initialize the calendar wakeup source.
  sli_si91x_power_manager_calendar_init();
#else
#if defined(SL_SI91X_POWER_MANAGER_UC_AVAILABLE) && defined(ENABLE_SECOND) && (ENABLE_SECOND == ENABLE) // secomd
  // Only Initialize the second based wakeup source.
  sli_si91x_power_manager_calendar_init();
#else
  // Calendar is initialized for scenarios where the calendar wakeup source is not configured via UC.
  sl_si91x_calendar_init();
#endif
  sl_calendar_datetime_config_t datetime_config;
  // Current date time is fetched
  status = sl_si91x_calendar_get_date_time(&datetime_config);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // If the date time is not configured, then only it is configured. Evaluated by the default values
  if ((!datetime_config.Year) && !(datetime_config.Century) && !(datetime_config.Hour)) {
    // Setting datetime for Calendar
    status = sl_si91x_calendar_build_datetime_struct(&datetime_config,
                                                     SLI_SI91X_CENTURY,
                                                     SLI_SI91X_YEAR,
                                                     February,
                                                     Friday,
                                                     SLI_SI91X_DAY,
                                                     SLI_SI91X_HOUR,
                                                     SLI_SI91X_MINUTE,
                                                     SLI_SI91X_SECONDS,
                                                     SLI_SI91X_MILLI_SECONDS);
    if (status != SL_STATUS_OK) {
      return status;
    }
    // Clear the application flag.
    sli_si91x_calendar_application_flag = false;
    status                              = sl_si91x_calendar_set_date_time(&datetime_config);
    if (status != SL_STATUS_OK) {
      return status;
    }
    // Set the application flag.
    sli_si91x_calendar_application_flag = true;
  }
  // Enable the calibration flag
  sli_si91x_calibration_interrupt_flag = true;
  // Trigger the calibration whenever the set date and time is called.
  sli_si91x_lf_fsm_rc_calibration();

  // set the alarm time for next calibration as per the next event returned by the event handler.
  // In the calibration event handler we are passing DISABLE, indicating that initial adjustment is not required.
  sli_si91x_set_periodic_alarm(sli_si91x_get_calib_next_event_time_handler(DISABLE));

  // Enable alarm in RTC.
  RSI_RTC_AlamEnable(RTC, true);
  // Unmask the alarm interrupt.
  RSI_RTC_IntrUnMask(RTC_ALARM_INTR);
  // Enable the alarm IRQ.
  NVIC_EnableIRQ(SLI_NVIC_ALARM);
  // Set alarm as a wake up source.
  RSI_PS_SetWkpSources(ALARM_BASED_WAKEUP);
#endif
  return status;
}

/******************************************************************************
 * This API initiates LF-FSM RC calibration and applies the calibrated time period to the RTC block.
 * @param none
 * @return none
 ******************************************************************************/
void sli_si91x_lf_fsm_rc_calibration(void)
{
  // Calibration will only be triggered if the sli_si91x_calibration_interrupt_flag is set.
  if (sli_si91x_calibration_interrupt_flag == true) {
    // Disable the Alarm IRQ.
    NVIC_DisableIRQ(MCU_CAL_ALARM_IRQn);
    // Initialization of RTC Calibration.
    RSI_RTC_CalibInitilization();
    // Configure the RC Clock calibration.
    RSI_RTC_RCCLK_Calib(TIME_PERIOD, ENABLE, DISABLE, DISABLE);
    // Introduce a delay of 3.04ms to apply the calibration.
    for (uint32_t i = 0; i < 30400; i++) {
      __ASM volatile("nop");
    }
    // Read the calibrated time period and update the RTC block.
    sli_si91x_rtc_timeperiod(TIME_PERIOD, SL_SI91X_CALIBRATED_TIMEPERIOD);
    // Reset the calibration flag after applying the calibration.
    sli_si91x_calibration_interrupt_flag = false;
    // Store the last calibration time.
    sl_si91x_calendar_get_date_time(si91x_calendar_data.sli_si91x_last_calib_time);
    // Enable the Alarm IRQ.
    NVIC_EnableIRQ(MCU_CAL_ALARM_IRQn);
  }
}

/**********************************************************************************************
 * This API is used to select the RTC clock timeperiod.
 * @param[in] pstcTimePeriod(*pstcTimePeriod) is pointer to the timperiod calibration registration instance
 * @param[in] u32TimePeriod(u32TimePeriod) is RTC time period in pico seconds as 32Khz as reference
 * @return  status 0 if successful, else error code as follows:
 *         - SL_STATUS_OK (0x0000) - Success
 *         - SL_STATUS_INVALID_PARAMETER (0x0021) - Parameters are invalid
 *  *********************************************************************************************/
sl_status_t sli_si91x_rtc_timeperiod(TIME_PERIOD_Type *pstcTimePeriod, uint32_t u32TimePeriod)
{
  /*Check for the NULL parameter*/
  if (pstcTimePeriod == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  /*Update the timer period */
  pstcTimePeriod->MCU_CAL_TIMER_CLOCK_PERIOD_b.RTC_TIMER_CLK_PERIOD = (unsigned int)(u32TimePeriod & 0x01FFFFFF);
  /*Indicated SOC programmed rtc_timer clock period is applied at KHz clock domain*/
  while (!pstcTimePeriod->MCU_CAL_TIMER_CLOCK_PERIOD_b.SPI_RTC_TIMER_CLK_PERIOD_APPLIED_b)
    ;
  return SL_STATUS_OK;
}

/******************************************************************************
 * This API restarts the calibration counter and triggers the calibration value again.
 * @param none
 * @return none
 ******************************************************************************/
void sli_si91x_restart_calibration_counter(sl_calendar_datetime_config_t *config)
{
  // Store the date and time into the variable
  if (sli_si91x_calendar_application_flag == true) {
    // Enable the calibration flag
    sli_si91x_calibration_interrupt_flag = true;
    // Trigger the calibration whenever the set date and time is called.
    sli_si91x_lf_fsm_rc_calibration();
  }
  *si91x_calendar_data.sli_si91x_default_date_time = *config;
  *si91x_calendar_data.sli_si91x_last_calib_time   = *config;
}

/******************************************************************************
 * This API calculates and updates the remaining calibration and alarm times. These values are utilized by the event handler to determine the next steps in the calibration process.
 * @param none
 * @return none
 ******************************************************************************/
void sli_si91x_calibration_adjustment(sl_calendar_datetime_config_t *alarm)
{
  // This will be enabled if the user enables the calibration.
  if (sli_si91x_calendar_application_flag == true) {
    // Disable the Alarm IRQ.
    NVIC_DisableIRQ(MCU_CAL_ALARM_IRQn);

    // Store the alarm time in the structure. This value will be utilized by the calibration event handler to decide the next event.
    si91x_calendar_data.sli_si91x_set_alarm_time = alarm;

    // Store the current time in the structure. This value will be utilized by the calibration event handler to decide the next event.
    sl_si91x_calendar_get_date_time(si91x_calendar_data.sli_si91x_current_time);

    // Covert the calendar variable like default time, current time, last calibration time and alarm time into NTP timing for calculation. Stores this into calibration event handler variable.
    sl_si91x_calendar_convert_calendar_datetime_to_unix_time(si91x_calendar_data.sli_si91x_set_alarm_time,
                                                             &si91x_calendar_data.sli_si91x_alarm_ntp_time);
    sl_si91x_calendar_convert_calendar_datetime_to_unix_time(si91x_calendar_data.sli_si91x_default_date_time,
                                                             &si91x_calendar_data.sli_si91x_default_ntp_time);
    sl_si91x_calendar_convert_calendar_datetime_to_unix_time(si91x_calendar_data.sli_si91x_last_calib_time,
                                                             &si91x_calendar_data.sli_si91x_last_calib_ntp_time);
    sl_si91x_calendar_convert_calendar_datetime_to_unix_time(si91x_calendar_data.sli_si91x_current_time,
                                                             &si91x_calendar_data.sli_si91x_current_ntp_time);

    // Convert the alarm time into milliseconds.
    uint32_t alarm_time =
      ((si91x_calendar_data.sli_si91x_alarm_ntp_time - si91x_calendar_data.sli_si91x_default_ntp_time) * 1000
       + (si91x_calendar_data.sli_si91x_set_alarm_time->MilliSeconds
          - si91x_calendar_data.sli_si91x_default_date_time->MilliSeconds));

    // Calculate the remaining alarm and calibration event time from the current time. This value will be utilized by the calibration event handler to decide the next event.
    sli_si91x_remaining_alarm_time =
      ((si91x_calendar_data.sli_si91x_alarm_ntp_time - si91x_calendar_data.sli_si91x_current_ntp_time) * 1000
       + (si91x_calendar_data.sli_si91x_set_alarm_time->MilliSeconds
          - si91x_calendar_data.sli_si91x_current_time->MilliSeconds));
    sli_si91x_remaining_calib_time =
      sli_si91x_calib_time
      - (si91x_calendar_data.sli_si91x_current_ntp_time - si91x_calendar_data.sli_si91x_last_calib_ntp_time) * 1000
      - (si91x_calendar_data.sli_si91x_current_time->MilliSeconds
         - si91x_calendar_data.sli_si91x_last_calib_time->MilliSeconds);

    // This API set the alarm time(in miliseconds) in static variable and remaining time for the calibration event handler.
    sli_si91x_set_remaining_time(alarm_time);

    // Calls the calibration event handler for the next event. Set the calendar alarm based on the return value of event handler.
    // In the calibration event handler we are passing ENABLE, indicating that initial adjustment is not required..
    sli_si91x_set_periodic_alarm(sli_si91x_get_calib_next_event_time_handler(ENABLE));

    // Enable the Alarm IRQ.
    NVIC_EnableIRQ(MCU_CAL_ALARM_IRQn);
  }
}

/**********************************************************************************************
 * This API is used to get the next event time. It uses 2 flags ( sli_si91x_calibration_interrupt_flag and sli_si91x_callback_trigger_flag) to indicate that the next time is for the calibration or alarm event.
 * Calibration event handler handles the next event time for both periodic trigger (Alarm wakeup source) and one shot trigger (Alarm peripheral).
 * @param[in] sl_si91x_mode (bool) Mode to determine the event type.
 * 1 - An adjustment is required, here may be  difference between the alarm counter start and the configured alarm time. (Alarm Peripheral or Alarm Wakeup source without UC - User can start the alarm counter and set the alarm value at any time)
 * 0 - No adjustments is required. (Alarm Wakeup source without UC- we will start the alarm counter and set the alarm value consecutively)
 * @return  The next event time.
 *********************************************************************************************/
uint32_t sli_si91x_get_calib_next_event_time_handler(bool sl_si91x_mode)
{
  if (sli_si91x_alarm_time == 0) {
    // If the alarm value is cleared. Indicating that the one shot alarm has been trigger or alarm event is not set. Thus, next events will only be for calibration.
    sli_si91x_disable_alarm_enable_calibration_flag();
    return sli_si91x_calib_time - sli_si91x_delay_handling();
  } else if (sli_si91x_alarm_time > sli_si91x_calib_time) {
    // Returns the next event time and sets the alarm or calibration event flag accordingly when the alarm event duration is greater than the calibration event.
    return sli_si91x_handle_alarm_greater_than_calibration_event(sl_si91x_mode) - sli_si91x_delay_handling();
  } else if (sli_si91x_alarm_time < sli_si91x_calib_time) {
    // Returns the next event time and sets the alarm or calibration event flag accordingly when the alarm event duration is less than the calibration event.
    return sli_si91x_handle_alarm_smaller_than_calibration_event(sl_si91x_mode) - sli_si91x_delay_handling();
  } else {
    // Returns the next event time the alarm time and enables both alarm and calibration event.
    sli_si91x_enable_alarm_and_calibration_flag();
    if (sl_si91x_mode) {
      uint32_t sl_si91x_temp_alarm_time = sli_si91x_alarm_time;
      // Return the alarm time and clears the alarm value.
      sli_si91x_alarm_time = 0;
      return sl_si91x_temp_alarm_time - sli_si91x_delay_handling();
    }
    return sli_si91x_alarm_time - sli_si91x_delay_handling();
  }
}

/**********************************************************************************************
 * This API Handles the scenario where alarm event time is greater than calibration event time.
 * @param[in] sl_si91x_mode (bool) Mode to determine the event type.
 * @return The next event time.
 *********************************************************************************************/
static uint32_t sli_si91x_handle_alarm_greater_than_calibration_event(bool sl_si91x_mode)
{
  if (sl_si91x_mode) {
    // Check if the remaining calibration time is greater than the remaining alarm time.
    if (sli_si91x_remaining_calib_time > sli_si91x_remaining_alarm_time) {
      // Calculate the difference between calibration and alarm times.
      sli_si91x_remaining_time = sli_si91x_remaining_calib_time - sli_si91x_remaining_alarm_time;
      // Enable alarm and disable calibration flags.
      sli_si91x_enable_alarm_disable_calibration_flag();
      // If the remaining time is below the threshold, enable both alarm and calibration flags.
      if (sli_si91x_remaining_time < SLI_SI91X_ALARM_CALIBRATION_THRESHOLD) {
        sli_si91x_alarm_time = 0;
        sli_si91x_enable_alarm_and_calibration_flag();
      }
      // Calculate the last value based on the difference between alarm and last calibration times.
      sli_si91x_last_value =
        (si91x_calendar_data.sli_si91x_alarm_ntp_time - si91x_calendar_data.sli_si91x_last_calib_ntp_time) * 1000
        - (si91x_calendar_data.sli_si91x_set_alarm_time->MilliSeconds
           - si91x_calendar_data.sli_si91x_last_calib_time->MilliSeconds);
      // Set the remaining time to the calibration time.
      sli_si91x_remaining_time = sli_si91x_calib_time;
      return sli_si91x_remaining_alarm_time;
    } else if (sli_si91x_remaining_calib_time < sli_si91x_remaining_alarm_time) {
      // Calculate the remaining time for calibration.
      sli_si91x_remaining_time =
        sli_si91x_remaining_time - sli_si91x_remaining_calib_time
        - (si91x_calendar_data.sli_si91x_current_ntp_time - si91x_calendar_data.sli_si91x_default_ntp_time) * 1000
        - (si91x_calendar_data.sli_si91x_current_time->MilliSeconds
           - si91x_calendar_data.sli_si91x_default_date_time->MilliSeconds);
      // Disable alarm and enable calibration flags.
      sli_si91x_disable_alarm_enable_calibration_flag();
      // If the remaining time is below the threshold, enable both alarm and calibration flags.
      if (sli_si91x_remaining_time < SLI_SI91X_ALARM_CALIBRATION_THRESHOLD) {
        sli_si91x_alarm_time = 0;
        sli_si91x_enable_alarm_and_calibration_flag();
      }
      return sli_si91x_remaining_calib_time;
    } else {
      // If calibration and alarm times are equal, enable both flags and reset the alarm time.
      sli_si91x_alarm_time = 0;
      sli_si91x_enable_alarm_and_calibration_flag();
      return sli_si91x_remaining_alarm_time;
    }
  } else {
    // Handle the case where sl_si91x_mode is false.
    if (sli_si91x_remaining_time >= sli_si91x_calib_time) {
      if (sli_si91x_last_value > 0) {
        uint32_t sl_si91x_temp_last_value = sli_si91x_last_value;
        // Adjust the remaining time for the next event cycle.
        sli_si91x_remaining_time -= sli_si91x_calib_time - sli_si91x_last_value;
        // Clear the last value for the next event cycle.
        sli_si91x_last_value = 0;
        // Disable alarm and enable calibration flags.
        sli_si91x_disable_alarm_enable_calibration_flag();
        if (sli_si91x_calendar_wakeup_peripheral_flag == 0) {
          // Clear the alarm value, indicating that the one-shot alarm is triggered.
          sli_si91x_alarm_time = 0;
        }
        return sli_si91x_calib_time - sl_si91x_temp_last_value;
      }
      // Subtract the remaining time with calibration time and return the calibration time accordingly.
      sli_si91x_remaining_time -= sli_si91x_calib_time;
      // Disable alarm and enable calibration flags.
      sli_si91x_disable_alarm_enable_calibration_flag();
      // If the remaining time is below the threshold or zero, enable both alarm and calibration flags.
      if (sli_si91x_remaining_time <= SLI_SI91X_ALARM_CALIBRATION_THRESHOLD || sli_si91x_remaining_time == 0) {
        uint32_t sl_si91x_temp_remaining_time = sli_si91x_remaining_time;
        sli_si91x_remaining_time              = sli_si91x_alarm_time;
        sli_si91x_enable_alarm_and_calibration_flag();
        if (sli_si91x_calendar_wakeup_peripheral_flag == 0) {
          sli_si91x_alarm_time = 0;
        }
        if (sli_si91x_remaining_time <= SLI_SI91X_ALARM_CALIBRATION_THRESHOLD) {
          return sli_si91x_calib_time + sl_si91x_temp_remaining_time;
        }
      }
      return sli_si91x_calib_time;
    } else {
      // Set last value and adjust remaining time for alarm.
      sli_si91x_last_value     = sli_si91x_remaining_time;
      sli_si91x_remaining_time = sli_si91x_alarm_time;
      // If the difference between calibration time and last value is below the threshold, enable both flags.
      if (sli_si91x_calib_time - sli_si91x_last_value <= SLI_SI91X_ALARM_CALIBRATION_THRESHOLD) {
        uint32_t sl_si91x_temp_last_value = sli_si91x_last_value;
        sli_si91x_enable_alarm_and_calibration_flag();
        sli_si91x_last_value = 0;
        if (sli_si91x_calendar_wakeup_peripheral_flag == 0) {
          sli_si91x_alarm_time = 0;
        }
        return sl_si91x_temp_last_value;
      }
      // Enable alarm and disable calibration flags.
      sli_si91x_enable_alarm_disable_calibration_flag();
      return sli_si91x_last_value;
    }
  }
}

/**********************************************************************************************
 * This API Handles the case where alarm event time is less than calibration event time.
 * @param sl_si91x_mode (bool) Mode to determine the event type.
 * @return The next event time.
 *********************************************************************************************/
static uint32_t sli_si91x_handle_alarm_smaller_than_calibration_event(bool sl_si91x_mode)
{
  if (!sli_si91x_calendar_wakeup_peripheral_flag && !sl_si91x_mode) {
    // Clear the alarm value. Indicating that the one shot alarm is trigger.
    sli_si91x_alarm_time = 0;

    // disable alarm event flag and enable calibration event flag.
    sli_si91x_disable_alarm_enable_calibration_flag();
    return sli_si91x_remaining_time;
  }
  if (sli_si91x_remaining_time >= sli_si91x_alarm_time) {
    if (sli_si91x_last_value > 0) {
      uint32_t sl_si91x_temp_last_value = sli_si91x_last_value;
      // Adjust the remaining time for the next event cycle.
      sli_si91x_remaining_time -= sli_si91x_alarm_time - sli_si91x_last_value;
      // Clear the last value.
      sli_si91x_last_value = 0;
      sli_si91x_enable_alarm_disable_calibration_flag();
      return sli_si91x_alarm_time - sl_si91x_temp_last_value;
    }
    // Subtract the remaining time with alarm_value and return the alarm value accordingly.
    sli_si91x_remaining_time -= sli_si91x_alarm_time;

    // Enable alarm event flag and disable calibration event flag.
    sli_si91x_enable_alarm_disable_calibration_flag();
    if ((sli_si91x_remaining_time == 0) || (sli_si91x_remaining_time <= SLI_SI91X_ALARM_CALIBRATION_THRESHOLD)) {
      // If the remaining time is 0 that means the calibration and alarm value needs to be triggered at the same time.
      // If the event between calibration and alarm is with 10milisecond then trigger the alarm interrupt and calibration together.
      sli_si91x_remaining_time = sli_si91x_calib_time;
      sli_si91x_enable_alarm_and_calibration_flag();
    }
    return sl_si91x_mode ? sli_si91x_remaining_alarm_time : sli_si91x_alarm_time;
  } else {
    // Set last value and adjust remaining time for calibration
    sli_si91x_last_value     = sli_si91x_remaining_time;
    sli_si91x_remaining_time = sli_si91x_calib_time;

    if (sli_si91x_alarm_time - sli_si91x_last_value <= SLI_SI91X_ALARM_CALIBRATION_THRESHOLD) {
      // If the event between calibration and alarm is with 10milisecond then trigger the alarm interrupt and calibration together.
      sli_si91x_enable_alarm_and_calibration_flag();
      // reset the last value;
      sli_si91x_last_value = 0;
      return sli_si91x_alarm_time;
    }

    // Disable the alarm event flag and enable the calibration event flag.
    sli_si91x_disable_alarm_enable_calibration_flag();
    return sli_si91x_last_value;
  }
}

/******************************************************************************
 * This API configures the alarm time in milliseconds. The specified time will be used to trigger the alarm event at the appropriate moment
 * @param alarm_time (uint32_t) alarm time in miliseconds.
 * @return none
 ******************************************************************************/
void sli_si91x_set_periodic_alarm(uint32_t alarm_time)
{
  sl_calendar_datetime_config_t set_alarm_config, get_datetime_config;

  sl_si91x_calendar_get_date_time(&get_datetime_config);

  set_alarm_config = get_datetime_config;

  set_alarm_config.MilliSeconds = set_alarm_config.MilliSeconds + (alarm_time % SLI_SI91X_NO_OF_MSEC_IN_SEC);
  if (set_alarm_config.MilliSeconds >= SLI_SI91X_NO_OF_MSEC_IN_SEC) {
    set_alarm_config.MilliSeconds -= SLI_SI91X_NO_OF_MSEC_IN_SEC;
    set_alarm_config.Second += SLI_SI91X_NEXT_OCCURENECE;
  }

  alarm_time /= SLI_SI91X_NO_OF_MSEC_IN_SEC;

  set_alarm_config.Second = set_alarm_config.Second + (alarm_time % SLI_SI91X_NO_OF_SEC_IN_MIN);
  if (set_alarm_config.Second >= SLI_SI91X_NO_OF_SEC_IN_MIN) {
    set_alarm_config.Second -= SLI_SI91X_NO_OF_SEC_IN_MIN;
    set_alarm_config.Minute += SLI_SI91X_NEXT_OCCURENECE;
  }

  alarm_time /= SLI_SI91X_NO_OF_SEC_IN_MIN;

  set_alarm_config.Minute = set_alarm_config.Minute + (alarm_time % SLI_SI91X_NO_OF_MIN_IN_HOUR);
  if (set_alarm_config.Minute >= SLI_SI91X_NO_OF_MIN_IN_HOUR) {
    set_alarm_config.Minute -= SLI_SI91X_NO_OF_MIN_IN_HOUR;
    set_alarm_config.Hour += SLI_SI91X_NEXT_OCCURENECE;
  }

  alarm_time /= SLI_SI91X_NO_OF_MIN_IN_HOUR;

  set_alarm_config.Hour = set_alarm_config.Hour + alarm_time;
  if (set_alarm_config.Hour >= SLI_SI91X_NO_OF_HOUR_IN_DAY) {
    set_alarm_config.Hour -= SLI_SI91X_NO_OF_HOUR_IN_DAY;
    set_alarm_config.Day += SLI_SI91X_NEXT_OCCURENECE;
  }

  if (set_alarm_config.Day > SLI_SI91X_NO_OF_DAY_IN_MONTH_1) {
    if (set_alarm_config.Month == February) {
      if (set_alarm_config.Year % SLI_SI91X_LEAP_YEAR_VALIDATION) {
        set_alarm_config.Day = SLI_SI91X_RESET_DAY;
        set_alarm_config.Month += SLI_SI91X_NEXT_OCCURENECE;
      } else if (set_alarm_config.Day > SLI_SI91X_NO_OF_DAY_IN_MONTH_2) {
        set_alarm_config.Day = SLI_SI91X_RESET_DAY;
        set_alarm_config.Month += SLI_SI91X_NEXT_OCCURENECE;
      }
    }

    if (set_alarm_config.Month <= July) {
      if (!(set_alarm_config.Month % SLI_SI91X_DAY_IN_MONTH_VALIDATION)) {
        if (set_alarm_config.Day > SLI_SI91X_NO_OF_DAY_IN_MONTH_3) {
          set_alarm_config.Day = SLI_SI91X_RESET_DAY;
          set_alarm_config.Month += SLI_SI91X_NEXT_OCCURENECE;
        } else if (set_alarm_config.Day > SLI_SI91X_NO_OF_DAY_IN_MONTH_4) {
          set_alarm_config.Day = SLI_SI91X_RESET_DAY;
          set_alarm_config.Month += SLI_SI91X_NEXT_OCCURENECE;
        }
      }
    } else if (!(set_alarm_config.Month % SLI_SI91X_DAY_IN_MONTH_VALIDATION)) {
      if (set_alarm_config.Day > SLI_SI91X_NO_OF_DAY_IN_MONTH_4) {
        set_alarm_config.Day = SLI_SI91X_RESET_DAY;
        set_alarm_config.Month += SLI_SI91X_NEXT_OCCURENECE;
      } else if (set_alarm_config.Day > SLI_SI91X_NO_OF_DAY_IN_MONTH_3) {
        set_alarm_config.Day = SLI_SI91X_RESET_DAY;
        set_alarm_config.Month += SLI_SI91X_NEXT_OCCURENECE;
      }
    }
  }

  if (set_alarm_config.Month > SLI_SI91X_NO_OF_MONTH_IN_YEAR) {
    set_alarm_config.Month = SLI_SI91X_RESET_MONTH;
    set_alarm_config.Year += SLI_SI91X_NEXT_OCCURENECE;
  }

  // The flag is cleared indicating execution is happening through driver files.
  sli_si91x_calendar_application_flag = 0;

  sl_si91x_calendar_set_alarm(&set_alarm_config);

  // The flag is cleared indicating execution is happening through driver files.
  sli_si91x_calendar_application_flag = 1;
}
/****************************************************************************
 * Sets the remaining time for the alarm.
 * This API set the set the alarm time to variable of the event handler. This value will be utilized by the calibration event handler to decide the next event.
 * This vaiable also updates the remaining time for the next event.
 * @param[in] alarm_in_millisecond The remaining time for the alarm in milliseconds.
 * @return None.
 ******************************************************************************/
void sli_si91x_set_remaining_time(uint32_t alarm_in_milisecond)
{
  sli_si91x_alarm_time     = alarm_in_milisecond;
  sli_si91x_remaining_time = (sli_si91x_alarm_time > sli_si91x_calib_time) ? sli_si91x_alarm_time
                                                                           : sli_si91x_calib_time;
}

/***************************************************************************
 * Sets the calibration application flag.
 * The current calibration framework includes certain calculations in the set alarm and set date APIs.
 * This flag is used to differentiate whether the set date-time API or set alarm API is being called by the application or the driver.
 * The flag being set indicates that set date-time API or set alarm API is being invoked application.
 * @param none
 * @return None.
 ******************************************************************************/
void sli_si91x_set_calibration_application_flag(void)
{
  sli_si91x_calendar_application_flag = true;
}

/***************************************************************************
 * Clears the calibration application flag.
 * The current calibration framework includes certain calculations in the set alarm and set date APIs.
 * This flag is used to differentiate whether the set date-time API or set alarm API is being called by the application or the driver.
 * The flag being cleared indicates that set date-time API or set alarm API is being invoked by driver.
 * @param none
 * @return None.
 ******************************************************************************/
void sli_si91x_clear_calibration_application_flag(void)
{
  sli_si91x_calendar_application_flag = false;
}

/***************************************************************************
 * This function returns the current state of the calibration application flag..
 *
 * @param none
 * @return The status of the calibration interrupt flag (true or false).
 ******************************************************************************/
bool sli_si91x_get_calendar_application_flag_status(void)
{
  return sli_si91x_calendar_application_flag;
}

/***************************************************************************
 * Sets the calendar wakeup peripheral flag.
 * This function sets the flag indicating that the calendar alarm is  configured as a calendar wakeup source.
 * @param none
 * @return None.
 ******************************************************************************/
void sli_si91x_set_calendar_wakeup_peripheral_flag(void)
{
  sli_si91x_calendar_wakeup_peripheral_flag = true;
}

/***************************************************************************
 * Clears the calendar wakeup peripheral flag.
 * This function clears the flag indicating that the calendar alarm is configured as a calendar peripheral.
 * @param none
 * @return None.
 ******************************************************************************/
void sli_si91x_clear_calendar_wakeup_peripheral_flag(void)
{
  sli_si91x_calendar_wakeup_peripheral_flag = false;
}

/***************************************************************************
 * This function returns the current state of the calibration interrupt flag, which indicates whether a calibration event needs to be triggered.
 *
 * @param none
 * @return The status of the calibration interrupt flag (true or false).
 ******************************************************************************/
bool sli_si91x_get_calibration_interrupt_flag_status(void)
{
  return sli_si91x_calibration_interrupt_flag;
}

/***************************************************************************
 * This function returns the current state of the callback trigger flag, which indicates whether a callback event needs to be triggered.
 *
 * @param none
 * @return The status of the callback trigger flag (true or false).
 ******************************************************************************/
bool sli_si91x_get_callback_trigger_flag_status(void)
{
  return sli_si91x_callback_trigger_flag;
}

/***************************************************************************
 * This function clears the wakeup overhead flag, indicating that the system does not need to adjust the wakeup overhead.
 *
 * @param none
 * @return none
 ***************************************************************************/
void sli_si91x_clear_wakeup_overhead_flag(void)
{
  sli_si91x_wakeup_overhead_flag = 0;
}

/***************************************************************************
 * This function sets the wakeup overhead flag, indicating that the system have to adjust the wakeup overhead.
 *
 * @param none
 * @return none
 ***************************************************************************/
void sli_si91x_set_wakeup_overhead_flag(void)
{
  sli_si91x_wakeup_overhead_flag = 1;
}

/***************************************************************************
 * This function returns the overhead due to m4 wakeup.
 *
 * @param none
 * @return none
 ***************************************************************************/
static uint8_t sli_si91x_delay_handling(void)
{
  if (sli_si91x_wakeup_overhead_flag == 1) {
    sli_si91x_clear_wakeup_overhead_flag();
    return SLI_SI91X_WAKEUP_OVERHEAD;
  } else {
    return SLI_SI91X_NO_OVERHEAD;
  }
}
#endif
