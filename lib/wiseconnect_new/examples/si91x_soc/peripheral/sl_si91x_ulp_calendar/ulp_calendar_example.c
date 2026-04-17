/***************************************************************************/
/**
 * @file ulp_calendar_example.c
 * @brief Ulp Calendar Example
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
#include "ulp_calendar_example.h"

#include "rsi_debug.h"
#include "sl_si91x_calendar.h"
#include "sl_si91x_power_manager.h"
#include "sl_si91x_wireless_shutdown.h"

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

static sl_power_state_t current_power_state = SL_SI91X_POWER_MANAGER_PS4;
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
static void configuring_ps2_power_state(void);
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
  sl_status_t status;
  do {
    // Initialization of Calendar
    sl_si91x_calendar_init();
    // Setting datetime for Calendar
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
      DEBUGOUT("sl_si91x_calendar_build_datetime_struct: Invalid Parameters, "
               "Error Code : %lu \n",
               status);
      break;
    }
    DEBUGOUT("Successfully built datetime structure\n");
    status = sl_si91x_calendar_set_date_time(&datetime_config);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_set_date_time: Invalid Parameters, Error "
               "Code : %lu \n",
               status);
      break;
    }
    DEBUGOUT("Successfully set calendar datetime\n");
    // Printing datetime for Calendar
    status = sl_si91x_calendar_get_date_time(&get_datetime);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_get_date_time: Invalid Parameters, Error "
               "Code : %lu \n",
               status);
      break;
    }
    DEBUGOUT("Successfully fetched the calendar datetime \n");
    calendar_print_datetime(get_datetime);
    DEBUGOUT("\n");

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
      DEBUGOUT("sl_si91x_calendar_build_datetime_struct: Invalid Parameters, "
               "Error Code : %lu \n",
               status);
      break;
    }
    DEBUGOUT("Successfully built datetime structure\n");
    // Setting the alarm configuration
    status = sl_si91x_calendar_set_alarm(&alarm_config);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_set_alarm: Invalid Parameters, Error Code : "
               "%lu \n",
               status);
      break;
    }
    DEBUGOUT("Successfully configured Alarm\n");
    status = sl_si91x_calendar_register_alarm_trigger_callback(on_alarm_callback);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_register_alarm_trigger_callback: Invalid "
               "Parameters, Error Code : %lu \n",
               status);
      break;
    }
    // Printing datetime for Alarm
    status = sl_si91x_calendar_get_alarm(&get_alarm);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_get_alarm: Invalid Parameters, Error Code : "
               "%lu \n",
               status);
      break;
    }
    DEBUGOUT("Successfully fetched the alarm datetime \n");
    calendar_print_datetime(get_alarm);
    DEBUGOUT("\n");
#endif

#if defined(SEC_INTR) && (SEC_INTR == ENABLE)
    // One second trigger
    status = sl_si91x_calendar_register_sec_trigger_callback(on_sec_callback);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_register_sec_trigger_callback: Invalid "
               "Parameters, Error Code : %lu \n",
               status);
      break;
    }
    DEBUGOUT("Successfully enabled one second trigger \n");
#endif

#if defined(MILLI_SEC_INTR) && (MILLI_SEC_INTR == ENABLE)
    // One millisecond trigger
    status = sl_si91x_calendar_register_msec_trigger_callback(on_msec_callback);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_register_msec_trigger_callback: Invalid "
               "Parameters, Error Code : %lu \n",
               status);
      break;
    }
    DEBUGOUT("Successfully enabled one milisecond trigger \n");
#endif

#if defined(TIME_CONVERSION) && (TIME_CONVERSION == ENABLE)
    uint32_t unix = UNIX_TEST_TIMESTAMP;
    uint32_t ntp  = 0;
    status        = sl_si91x_calendar_convert_unix_time_to_ntp_time(unix, &ntp);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_convert_unix_time_to_ntp_time: Invalid "
               "Parameters, Error Code : %lu \n",
               status);
      break;
    }
    DEBUGOUT("Unix Time: %lu\n", unix);
    DEBUGOUT("NTP Time: %lu\n", ntp);
    uint32_t unix_new = 0;
    status            = sl_si91x_calendar_convert_ntp_time_to_unix_time(ntp, &unix_new);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_calendar_convert_ntp_time_to_unix_time: Invalid "
               "Parameters, Error Code : %lu \n",
               status);
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
  sl_status_t status;
  sl_calendar_datetime_config_t get_datetime;
  if (current_power_state == SL_SI91X_POWER_MANAGER_PS4) {
    do {
      DEBUGOUT("Switching the Calendar from PS4 -> PS2 state\n");
      // Control power management by adjusting clock references and shutting down
      // the power supply
      // This function is for demonstration purpose only. For more details, refer to the README file.
      sl_si91x_wireless_shutdown();
      // switching the power state PS4 to PS2 mode.
      status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS2);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_power_manager_add_ps_requirement: Error Code : %lu \n", status);
        break;
      }
      /* Due to calling trim_efuse API om power manager it will change the clock
      frequency, if we are not initialize the debug again it will print the
      garbage data or no data in console output. */
      DEBUGINIT();
      // Configuring the ps2 power state by configuring
      // the ram retention and removing the unused peripherals
      configuring_ps2_power_state();
      // get the datetime for Calendar
      status = sl_si91x_calendar_get_date_time(&get_datetime);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_calendar_get_date_time: Invalid Parameters, Error Code "
                 ": %lu \n",
                 status);
        break;
      }
      DEBUGOUT("Successfully fetched the calendar datetime \n");
      // Printing datetime for Calendar
      calendar_print_datetime(get_datetime);
      // current power state is updated to PS2
      current_power_state = SL_SI91X_POWER_MANAGER_PS2;
    } while (false);
  }
#if defined(ALARM_EXAMPLE) && (ALARM_EXAMPLE == ENABLE)
  if (is_alarm_callback_triggered) {
    DEBUGOUT("Alarm Callback is Triggered \n");
    is_alarm_callback_triggered = false;
    if (current_power_state == SL_SI91X_POWER_MANAGER_PS2) {
      DEBUGOUT("Switching the Calendar from PS2 -> PS4 state\n");
      // switching the power state from PS2 to PS4 mode
      status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS4);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_power_manager_add_ps_requirement: Error Code : %lu \n", status);
        return;
      }
      // get the datetime for Calendar
      status = sl_si91x_calendar_get_date_time(&get_datetime);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_calendar_get_date_time: Invalid Parameters, Error Code "
                 ": %lu \n",
                 status);
        return;
      }
      DEBUGOUT("Successfully fetched the calendar datetime \n");
      // Printing datetime for Calendar
      calendar_print_datetime(get_datetime);
      // current power state is updated to last enum after the power state cycle
      // is completed
      current_power_state = LAST_ENUM_POWER_STATE;
      //  de initializing the calendar
      sl_si91x_calendar_deinit();
      return;
    }
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
  DEBUGOUT("Date Format: DD/MM/YY: %.2d/%.2d/%.2d \n", data.Day, data.Month, data.Year);
  DEBUGOUT(" Century: %d\n", data.Century);
}

/*******************************************************************************
 * Callback function of alarm, it is a periodic alarm
 * After the callback is triggered, new alarm is set according to the
 *ALARM_TRIGGER_TIME macro in header file.
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
/*******************************************************************************
 * powering off the peripherals not in use,
 * Configuring power manager ram-retention
 ******************************************************************************/
static void configuring_ps2_power_state(void)
{
  sl_status_t status;
  sl_power_peripheral_t peri;
  sl_power_ram_retention_config_t config;
  peri.m4ss_peripheral = 0;
  // Configure RAM banks for retention during power management
  config.configure_ram_banks = true; // Enable RAM bank configuration
  config.m4ss_ram_banks      = SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_8 | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_9
                          | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_10; // Specify the RAM banks to be
                                                                     // retained during power
                                                                     // management
  config.ulpss_ram_banks = SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_2 | SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_3;
  // Ored value for ulpss peripheral.
  peri.ulpss_peripheral = SL_SI91X_POWER_MANAGER_ULPSS_PG_MISC | SL_SI91X_POWER_MANAGER_ULPSS_PG_SSI
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_I2S | SL_SI91X_POWER_MANAGER_ULPSS_PG_I2C
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_IR | SL_SI91X_POWER_MANAGER_ULPSS_PG_FIM
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_AUX;
  // Ored value for npss peripheral.
  peri.npss_peripheral = SL_SI91X_POWER_MANAGER_NPSS_PG_MCUWDT | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUPS
                         | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUTS | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE2
                         | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE3;
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
