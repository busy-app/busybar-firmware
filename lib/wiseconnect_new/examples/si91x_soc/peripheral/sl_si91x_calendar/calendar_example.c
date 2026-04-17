/***************************************************************************/ /**
 * @file calendar_example.c
 * @brief Calendar example
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_si91x_calendar.h"
#include "calendar_example.h"
#include "rsi_debug.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define MAX_SECOND          60u        // Total seconds in one minute
#define MAX_MINUTE          60u        // Total minutes in one hour
#define MAX_HOUR            24u        // Total hours in one day
#define SECONDS_IN_HOUR     3600u      // Total seconds in one hour
#define UNIX_TEST_TIMESTAMP 981117600u // Unix Time Stamp for 02/02/2001, 18:10:0
#define MS_DEBUG_DELAY      1000u      // Debug prints after every 1000 counts (callback trigger)

#define TEST_CENTURY      2u
#define TEST_YEAR         1u
#define TEST_MONTH        February
#define TEST_DAY_OF_WEEK  Friday
#define TEST_DAY          2u
#define TEST_HOUR         18u
#define TEST_MINUTE       10u
#define TEST_SECONDS      10u
#define TEST_MILLISECONDS 100u

#define ALARM_CENTURY      2u
#define ALARM_YEAR         1u
#define ALARM_MONTH        February
#define ALARM_DAY_OF_WEEK  Friday
#define ALARM_DAY          2u
#define ALARM_HOUR         18u
#define ALARM_MINUTE       10u
#define ALARM_SECONDS      15u
#define ALARM_MILLISECONDS 100u

#define CAL_RC_CLOCK 2u
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void calendar_print_datetime(sl_calendar_datetime_config_t data);
#if defined(ALARM_EXAMPLE) && (ALARM_EXAMPLE == ENABLE)
static void on_alarm_callback(void);
boolean_t is_alarm_callback_triggered = false;
#endif
#if defined(SEC_INTR) && (SEC_INTR == ENABLE)
static void on_sec_callback(void);
boolean_t is_sec_callback_triggered = false;
#endif
#if defined(MILLI_SEC_INTR) && (MILLI_SEC_INTR == ENABLE)
static void on_msec_callback(void);
boolean_t is_msec_callback_triggered = false;
#endif
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * Calendar example initialization function
 ******************************************************************************/
void calendar_example_init(void)
{
  sl_calendar_datetime_config_t datetime_config;
  sl_calendar_datetime_config_t get_datetime;
  sl_calendar_datetime_config_t datetime_for_unix_demo;
  uint32_t unix_timestamp;
  sl_status_t status;

  do {

    // Initialization of calendar
    sl_si91x_calendar_init();

    //Setting datetime for Calendar
    status = sl_si91x_calendar_build_datetime_struct(&datetime_config,
                                                     TEST_CENTURY,
                                                     TEST_YEAR,
                                                     TEST_MONTH,
                                                     TEST_DAY_OF_WEEK,
                                                     TEST_DAY,
                                                     TEST_HOUR,
                                                     TEST_MINUTE,
                                                     TEST_SECONDS,
                                                     TEST_MILLISECONDS);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_build_datetime_struct: Invalid Parameters, Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Successfully built datetime structure\n");
    status = sl_si91x_calendar_set_date_time(&datetime_config);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_set_date_time: Invalid Parameters, Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Successfully set calendar datetime\n");
    // Printing datetime for Calendar
    status = sl_si91x_calendar_get_date_time(&get_datetime);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_get_date_time: Invalid Parameters, Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Successfully fetched the calendar datetime \n");
    calendar_print_datetime(get_datetime);

    /** Demo APIs related to Unix timestamp conversions */
    datetime_for_unix_demo = get_datetime;
    sl_si91x_calendar_convert_calendar_datetime_to_unix_time(&datetime_for_unix_demo, &unix_timestamp);
    DEBUGOUT("\nIts equivalent Unix timestamp: %lu\n", unix_timestamp);

    unix_timestamp += 300; // increment by 5min (300 in sec), to demo Unix-to-calendar conversion
    DEBUGOUT("\nUnix Timestamp incremented by 5min");
    DEBUGOUT("\nIncremented Unix timestamp: %lu\n", unix_timestamp);
    sl_si91x_calendar_convert_unix_time_to_calendar_datetime(unix_timestamp, &datetime_for_unix_demo);
    DEBUGOUT("\nUnix to Calendar datetime conversion:\n");
    DEBUGOUT("Time Format: hour:%d, min:%d, sec:%d, msec:%d\n",
             datetime_for_unix_demo.Hour,
             datetime_for_unix_demo.Minute,
             datetime_for_unix_demo.Second,
             datetime_for_unix_demo.MilliSeconds);
    DEBUGOUT("Date Format: DayOfWeek DD/MM/YY: %.2d %.2d/%.2d/%.2d ",
             datetime_for_unix_demo.DayOfWeek,
             datetime_for_unix_demo.Day,
             datetime_for_unix_demo.Month,
             datetime_for_unix_demo.Year);
    DEBUGOUT(" Century: %d in GMT Time Zone\n", datetime_for_unix_demo.Century);
    /*************************************************************/

#if defined(ALARM_EXAMPLE) && (ALARM_EXAMPLE == ENABLE)
    sl_calendar_datetime_config_t alarm_config;
    sl_calendar_datetime_config_t get_alarm;
    status = sl_si91x_calendar_build_datetime_struct(&alarm_config,
                                                     ALARM_CENTURY,
                                                     ALARM_YEAR,
                                                     ALARM_MONTH,
                                                     ALARM_DAY_OF_WEEK,
                                                     ALARM_DAY,
                                                     ALARM_HOUR,
                                                     ALARM_MINUTE,
                                                     ALARM_SECONDS,
                                                     ALARM_MILLISECONDS);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_build_datetime_struct: Invalid Parameters, Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Successfully built datetime structure\n");
    //Setting the alarm configuration
    status = sl_si91x_calendar_set_alarm(&alarm_config);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_set_alarm: Invalid Parameters, Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Successfully configured Alarm\n");
    status = sl_si91x_calendar_register_alarm_trigger_callback(on_alarm_callback);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_register_alarm_trigger_callback: Invalid Parameters, Error Code : %lu \n", status);
      break;
    }
    // Printing datetime for Alarm
    status = sl_si91x_calendar_get_alarm(&get_alarm);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_get_alarm: Invalid Parameters, Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Successfully fetched the alarm datetime \n");
    calendar_print_datetime(get_alarm);
    DEBUGOUT("\n");
#endif

#if defined(SEC_INTR) && (SEC_INTR == ENABLE)
    //One second trigger
    status = sl_si91x_calendar_register_sec_trigger_callback(on_sec_callback);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_register_sec_trigger_callback: Invalid Parameters, Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Successfully enabled one second trigger \n");
#endif

#if defined(MILLI_SEC_INTR) && (MILLI_SEC_INTR == ENABLE)
    //One millisecond trigger
    status = sl_si91x_calendar_register_msec_trigger_callback(on_msec_callback);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_register_msec_trigger_callback: Invalid Parameters, Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Successfully enabled one milisecond trigger \n");
#endif

#if defined(TIME_CONVERSION) && (TIME_CONVERSION == ENABLE)
    uint32_t unix = UNIX_TEST_TIMESTAMP;
    uint32_t ntp  = 0;
    status        = sl_si91x_calendar_convert_unix_time_to_ntp_time(unix, &ntp);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_convert_unix_time_to_ntp_time: Invalid Parameters, Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("Unix Time: %lu\n", unix);
    DEBUGOUT("NTP Time: %lu\n", ntp);
    uint32_t unix_new = 0;
    status            = sl_si91x_calendar_convert_ntp_time_to_unix_time(ntp, &unix_new);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_convert_ntp_time_to_unix_time: Invalid Parameters, Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("NTP Time: %lu\n", ntp);
    DEBUGOUT("Unix Time: %lu\n", unix_new);
#endif
  } while (false);
}

/*******************************************************************************
 * Function will run continuously and will wait for trigger
 ******************************************************************************/
void calendar_example_process_action(void)
{
#if defined(ALARM_EXAMPLE) && (ALARM_EXAMPLE == ENABLE)
  if (is_alarm_callback_triggered) {
    DEBUGOUT("Alarm Callback is Triggered \n");
    is_alarm_callback_triggered = false;
  }
#endif
#if defined(SEC_INTR) && (SEC_INTR == ENABLE)
  if (is_sec_callback_triggered) {
    DEBUGOUT("One Sec Callback is Triggered \n");
    is_sec_callback_triggered = false;
  }
#endif
#if defined(MILLI_SEC_INTR) && (MILLI_SEC_INTR == ENABLE)
  if (is_msec_callback_triggered) {
    DEBUGOUT("One Milli-Sec Callback triggered 1000 times\n");
    is_msec_callback_triggered = false;
  }
#endif
}

/*******************************************************************************
 * Function to print date and time from given structure
 * 
 * @param[in] data pointer to the datetime structure
 * @return none
 ******************************************************************************/
static void calendar_print_datetime(sl_calendar_datetime_config_t data)
{
  DEBUGOUT("\n***Calendar time****\n");
  DEBUGOUT("Time Format: hour:%d, min:%d, sec:%d, msec:%d\n", data.Hour, data.Minute, data.Second, data.MilliSeconds);
  DEBUGOUT("Date Format: DayOfWeek DD/MM/YY: %.2d %.2d/%.2d/%.2d ", data.DayOfWeek, data.Day, data.Month, data.Year);
  DEBUGOUT(" Century: %d\n", data.Century);
}

/*******************************************************************************
 * Callback function of alarm, it is a periodic alarm
 * After the callback is triggered, new alarm is set according to the ALARM_TRIGGER_TIME
 * macro in header file.
 * 
 * @param none
 * @return none
 ******************************************************************************/
#if defined(ALARM_EXAMPLE) && (ALARM_EXAMPLE == ENABLE)
static void on_alarm_callback(void)
{
  is_alarm_callback_triggered = true;
}
#endif

/*******************************************************************************
 * Callback function of one second trigger
 * 
 * @param none
 * @return none
 ******************************************************************************/
#if defined(SEC_INTR) && (SEC_INTR == ENABLE)
static void on_sec_callback(void)
{
  is_sec_callback_triggered = true;
}
#endif

/*******************************************************************************
 * Callback function of one millisecond trigger
 * 
 * @param none
 * @return none
 ******************************************************************************/
#if defined(MILLI_SEC_INTR) && (MILLI_SEC_INTR == ENABLE)
static void on_msec_callback(void)
{
  static uint16_t temp = 0;
  temp++;
  if (temp >= MS_DEBUG_DELAY) {
    is_msec_callback_triggered = true;
    temp                       = 0;
  }
}
#endif
