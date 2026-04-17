/***************************************************************************/ /**
 * @file sli_si91x_power_manager_wakeup_initialization.c
 * @brief Power Manager Wakeup Source Initialization API implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_si91x_power_manager.h"
#include "sl_si91x_power_manager_wakeup_handler.h"
#include "rsi_rom_power_save.h"
#if defined(SL_SI91X_32KHZ_RC_CALIBRATION_ENABLED) && (SL_SI91X_32KHZ_RC_CALIBRATION_ENABLED == ENABLE)
#include "sli_si91x_32khz_rc_calibration.h"
#endif
/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/
#define DS_IRQ                   IRQ027_Handler     // DS IRQ Handler
#define NVIC_DS_TIMER            NPSS_MCU_INTR_IRQn // DS Timer NVIC enable
#define WIRELESS_WAKEUP_IRQ      IRQ026_Handler     // Wireless Wakeup IRQ Handler
#define WIRELESS_WAKEUP_Priority 8                  // Wireless Wakeup IRQ priority
#define UULP_GPIO_0              0                  // UULP GPIO Pin 0
#define UULP_GPIO_1              1                  // UULP GPIO Pin 1
#define UULP_GPIO_2              2                  // UULP GPIO Pin 2
#define UULP_GPIO_3              3                  // UULP GPIO Pin 3
#define POLARITY_LOW             0                  // low polarity
#define POLARITY_HIGH            1                  // High polarity
#define CALENDAR_RC_CLOCK        2                  // RC clock value for Calendar RTC
#define NO_OF_MSEC_IN_SEC        1000               // Number of milliseconds in second
#define NO_OF_SEC_IN_MIN         60                 // Number of seconds in a minute
#define NO_OF_MIN_IN_HOUR        60                 // Number of minutes in an hour
#define NO_OF_HOUR_IN_DAY        24                 // Number of hour in a day
#define NO_OF_DAY_IN_MONTH_1     28                 // Number of days in february month
#define NO_OF_DAY_IN_MONTH_2     29                 // Number of days in february month which has leap year
#define NO_OF_DAY_IN_MONTH_3     30                 // Number of days in even month
#define NO_OF_DAY_IN_MONTH_4     31                 // Number of days in odd month
#define NO_OF_MONTH_IN_YEAR      12                 // Number of months in a year
#define NEXT_OCCURENECE          1                  // Next time occurence
#define RESET_DAY                1                  // Reset day after incrementing month
#define RESET_MONTH              1                  // Reset month after incrementing year
#define LEAP_YEAR_VALIDATION     4                  // Value to validate the leap year
#define DAY_IN_MONTH_VALIDATION  2                  // Value to validate the days in month
#define CENTURY                  2                  // Default Century to be configured
#define YEAR                     1                  // Default year to be configured
#define DAY                      2                  // Default day to be configured
#define HOUR                     18                 // Default hour to be configured
#define MINUTE                   10                 // Default minute to be configured
#define SECONDS                  10                 // Default seconds to be configured
#define MILLI_SECONDS            100                // Default milli seconds to be configured

/*******************************************************************************
 ***************************  Local Types  ********************************
 ******************************************************************************/
/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
#if defined(SL_ENABLE_CALENDAR_WAKEUP_SOURCE) && (SL_ENABLE_CALENDAR_WAKEUP_SOURCE == ENABLE)
#if (SL_SI91X_32KHZ_RC_CALIBRATION_ENABLED == ENABLE)
// In the current design, if calibration is initialized, the alarm wakeup source should only be initialized once.
// Flag to indicate whether the calendar alarm wakeup source has been initialized for the first time.
static bool sli_si91x_calibration_calendar_single_execution = false;
#endif
static sl_status_t sli_si91x_power_manager_calendar_deinit(void);
#if defined(ENABLE_ALARM) && (ENABLE_ALARM == ENABLE)
__attribute__((weak)) void calendar_alarm_callback_function(void);
static void set_periodic_alarm(uint32_t alarm_time);
#endif // ENABLE_ALARM
#if defined(ENABLE_SECOND) && (ENABLE_SECOND == ENABLE)
__attribute__((weak)) void calendar_second_callback_function(void);
#endif // ENABLE_SECOND
#endif // SL_ENABLE_CALENDAR_WAKEUP_SOURCE

#if defined(SL_ENABLE_GPIO_WAKEUP_SOURCE) && (SL_ENABLE_GPIO_WAKEUP_SOURCE == ENABLE)
__attribute__((weak)) void gpio_uulp_pin_interrupt_callback(uint32_t pin_intr);
static sl_status_t uulp_gpio_configuration(uint8_t pin, uint8_t polarity);
#endif // SL_ENABLE_GPIO_WAKEUP_SOURCE

#if defined(SL_ENABLE_WDT_WAKEUP_SOURCE) && (SL_ENABLE_WDT_WAKEUP_SOURCE == ENABLE)
static void wdt_callback_function(void);
#endif // SL_ENABLE_WDT_WAKEUP_SOURCE

#if defined(SL_ENABLE_DST_WAKEUP_SOURCE) && (SL_ENABLE_DST_WAKEUP_SOURCE == ENABLE)
static sl_status_t sli_si91x_power_manager_dst_deinit(void);
__attribute__((weak)) void dst_callback_function(void);
#endif // SL_ENABLE_DST_WAKEUP_SOURCE

#if defined(SL_ENABLE_WIRELESS_WAKEUP_SOURCE) && (SL_ENABLE_WIRELESS_WAKEUP_SOURCE == ENABLE)
static sl_status_t sli_si91x_power_manager_wireless_wakeup_deinit(void);
#endif
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
#if defined(SL_ENABLE_DST_WAKEUP_SOURCE) && (SL_ENABLE_DST_WAKEUP_SOURCE == ENABLE)
typedef void (*sl_dst_irq_callback_t)(void);
static sl_dst_irq_callback_t dst_callback = NULL;
#endif // SL_ENABLE_DST_WAKEUP_SOURCE
/*******************************************************************************
***********************  Global function Definitions *************************
 ******************************************************************************/

/*******************************************************************************
 * Calendar based wakeup is initialized.
 ******************************************************************************/
sl_status_t sli_si91x_power_manager_calendar_init(void)
{
  sl_status_t status = SL_STATUS_OK;
#if defined(SL_ENABLE_CALENDAR_WAKEUP_SOURCE) && (SL_ENABLE_CALENDAR_WAKEUP_SOURCE == ENABLE)
#if (SL_SI91X_32KHZ_RC_CALIBRATION_ENABLED == ENABLE)
  // In the current design, if calibration is initialized, the alarm wakeup source should only be initialized once.
  if (sli_si91x_calibration_calendar_single_execution == false) {
#endif
    // Calendar is de-initialized.
    status = sli_si91x_power_manager_calendar_deinit();
    if (status != SL_STATUS_OK) {
      // If status is not OK, return with the error code.
      return status;
    }
    // Calendar is initialized.
    sl_si91x_calendar_init();
#if defined(ENABLE_SECOND) && (ENABLE_SECOND == ENABLE)
    // Second based callback is configured.
    status = sl_si91x_calendar_register_sec_trigger_callback(calendar_second_callback_function);
    if (status != SL_STATUS_OK) {
      // If status is not OK, return with the error code.
      return status;
    }
    // Second based wakeup source is configured
    status = sl_si91x_power_manager_set_wakeup_sources(SL_SI91X_POWER_MANAGER_SEC_WAKEUP, true);
    if (status != SL_STATUS_OK) {
      // If status is not OK, return with the error code.
      return status;
    }
#endif // ENABLE_SECOND

#if defined(ENABLE_ALARM) && (ENABLE_ALARM == ENABLE)
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
                                                       CENTURY,
                                                       YEAR,
                                                       February,
                                                       Friday,
                                                       DAY,
                                                       HOUR,
                                                       MINUTE,
                                                       SECONDS,
                                                       MILLI_SECONDS);
      if (status != SL_STATUS_OK) {
        return status;
      }
#if (SL_SI91X_32KHZ_RC_CALIBRATION_ENABLED == ENABLE)
      // Apply the calibration for the first time.
      sli_si91x_lf_fsm_rc_calibration();
#endif
      status = sl_si91x_calendar_set_date_time(&datetime_config);
      if (status != SL_STATUS_OK) {
        return status;
      }
    }
#if (SL_SI91X_32KHZ_RC_CALIBRATION_ENABLED == ENABLE)
    // For the event handler to provide the next event, we need to set the alarm time to variable of the event handler.
    // This value will be utilized by the calibration event handler to decide the next event.
    sli_si91x_set_remaining_time(ALARM_TIME_MSEC);
    // Set the wakeup flag to indicate that the wakeup source has been configured.
    // If this flag is set, the event handler will return the next event, considering the alarm interrupt (wakeup interrupt) as a periodically event. Otherwise, the alarm interrupt will be configured as a one-time event.
    sli_si91x_set_calendar_wakeup_peripheral_flag();
    // Invokes the calibration event handler to determine the next event.
    // Configures the calendar alarm based on the return value of the event handler.
    set_periodic_alarm(sli_si91x_get_calib_next_event_time_handler(DISABLE));
#else
    set_periodic_alarm(ALARM_TIME_MSEC);
#endif

    // Alarm based callback is registered
    status = sl_si91x_calendar_register_alarm_trigger_callback(calendar_alarm_callback_function);
    if (status != SL_STATUS_OK) {
      return status;
    }
    // Alarm based wakeup source is configured
    status = sl_si91x_power_manager_set_wakeup_sources(SL_SI91X_POWER_MANAGER_ALARM_WAKEUP, true);
    if (status != SL_STATUS_OK) {
      return status;
    }
#endif // ENABLE_ALARM

#if defined(ENABLE_MSEC) && (ENABLE_MSEC == ENABLE)
    // Milli Second based callback is configured.
    status = sl_si91x_calendar_register_msec_trigger_callback(calendar_alarm_callback_function);
    if (status != SL_STATUS_OK) {
      // If status is not OK, return with the error code.
      return status;
    }
    status = sl_si91x_power_manager_set_wakeup_sources(SL_SI91X_POWER_MANAGER_MSEC_WAKEUP, true);
    if (status != SL_STATUS_OK) {
      // If status is not OK, return with the error code.
      return status;
    }
#endif // ENABLE_MSEC
#if (SL_SI91X_32KHZ_RC_CALIBRATION_ENABLED == ENABLE)
    // Set the flag. So that calendar alarm wakeup source through UC will be initialized for the first time only.
    sli_si91x_calibration_calendar_single_execution = true;
  }
#endif
#endif // SL_ENABLE_CALENDAR_WAKEUP_SOURCE
  return status;
}

#if defined(SL_ENABLE_CALENDAR_WAKEUP_SOURCE) && (SL_ENABLE_CALENDAR_WAKEUP_SOURCE == ENABLE)
/*******************************************************************************
 * Calendar based wakeup is de-initialized.
 ******************************************************************************/
static sl_status_t sli_si91x_power_manager_calendar_deinit(void)
{
  sl_status_t status = SL_STATUS_OK;
#if defined(ENABLE_ALARM) && (ENABLE_ALARM == ENABLE)
  // Unregister the alarm based callback.
  status = sl_si91x_calendar_unregister_alarm_trigger_callback();
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif
#if defined(ENABLE_SECOND) && (ENABLE_SECOND == ENABLE)
  // Unregister the second based callback.
  status = sl_si91x_calendar_unregister_sec_trigger_callback();
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif
  return status;
}
#endif
/*******************************************************************************
 * GPIO based wakeup is initialized.
 ******************************************************************************/
sl_status_t sli_si91x_power_manager_gpio_init(void)
{
  sl_status_t status = SL_STATUS_OK;

#if defined(SL_ENABLE_GPIO_WAKEUP_SOURCE) && (SL_ENABLE_GPIO_WAKEUP_SOURCE == ENABLE)

#if defined(ENABLE_NPSS_GPIO_0) && (ENABLE_NPSS_GPIO_0 == ENABLE)
  status = uulp_gpio_configuration(UULP_GPIO_0, NPSS_GPIO_0_POLARITY);
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif // ENABLE_NPSS_GPIO_0

#if defined(ENABLE_NPSS_GPIO_1) && (ENABLE_NPSS_GPIO_1 == ENABLE)
  status = uulp_gpio_configuration(UULP_GPIO_1, NPSS_GPIO_1_POLARITY);
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif // ENABLE_NPSS_GPIO_1

#if defined(ENABLE_NPSS_GPIO_2) && (ENABLE_NPSS_GPIO_2 == ENABLE)
  status = uulp_gpio_configuration(UULP_GPIO_2, NPSS_GPIO_2_POLARITY);
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif // ENABLE_NPSS_GPIO_2

#if defined(ENABLE_NPSS_GPIO_3) && (ENABLE_NPSS_GPIO_3 == ENABLE)
  status = uulp_gpio_configuration(UULP_GPIO_3, NPSS_GPIO_3_POLARITY);
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif // ENABLE_NPSS_GPIO_4
#endif // SL_ENABLE_GPIO_WAKEUP_SOURCE
  return status;
}

/*******************************************************************************
 * WDT based wakeup is initialized.
 ******************************************************************************/
sl_status_t sli_si91x_power_manager_wdt_init(void)
{
  sl_status_t status = SL_STATUS_OK;
#if defined(SL_ENABLE_WDT_WAKEUP_SOURCE) && (SL_ENABLE_WDT_WAKEUP_SOURCE == ENABLE)
  watchdog_timer_config_t wdt_config;
  sl_si91x_watchdog_init_timer();
  // Configuring watchdog-timer
  status = sl_si91x_watchdog_set_configuration(&wdt_config);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Registering timeout callback
  status = sl_si91x_watchdog_register_timeout_callback(wdt_callback_function);
  if (status != SL_STATUS_OK) {
    return status;
  }
  status = sl_si91x_power_manager_set_wakeup_sources(SL_SI91X_POWER_MANAGER_WDT_WAKEUP, true);
  if (status != SL_STATUS_OK) {
    // If status is not OK, return with the error code.
    return status;
  }
  sl_si91x_watchdog_start_timer();
#endif // SL_ENABLE_WDT_WAKEUP_SOURCE
  return status;
}
/*******************************************************************************
 * Deep sleep timer based wakeup is initialized.
 ******************************************************************************/
sl_status_t sli_si91x_power_manager_dst_init(void)
{
  sl_status_t status = SL_STATUS_OK;
#if defined(SL_ENABLE_DST_WAKEUP_SOURCE) && (SL_ENABLE_DST_WAKEUP_SOURCE == ENABLE)
  // Deep-sleep timer is de-initialized.
  status = sli_si91x_power_manager_dst_deinit();
  if (status != SL_STATUS_OK) {
    // If status is not OK, return with the error code.
    return status;
  }
  // Power-up the RTC and Time period block
  RSI_PS_NpssPeriPowerUp(SLPSS_PWRGATE_ULP_MCURTC | SLPSS_PWRGATE_ULP_TIMEPERIOD);

  // Power-up the Deep-sleep timer
  RSI_PS_PowerSupplyEnable(POWER_ENABLE_DEEPSLEEP_TIMER);

  // by using this API we programmed the RTC timer clock in SOC
  // MSB 8-bits for the Integer part &
  // LSB 17-bits for the Fractional part
  // Eg:- 32KHz = 31.25µs ==> 31.25*2^17 = 4096000 = 0x3E8000
  /* Time Period Programming */
  RSI_TIMEPERIOD_TimerClkSel(TIME_PERIOD, 0x003E7FFF);

  RSI_DST_DurationSet(DST_WAKEUP_TIME);
  status = sl_si91x_power_manager_set_wakeup_sources(SL_SI91X_POWER_MANAGER_DST_WAKEUP, true);
  if (status != SL_STATUS_OK) {
    // If status is not OK, return with the error code.
    return status;
  }
  // To validate the function spointer and void pointer, if the parameters is not NULL then, it
  // returns an error code
  // If another callback is registered without unregistering previous callback then, it
  // returns an error code, so it is mandatory to unregister the callback before registering
  // another callback
  if (dst_callback != NULL) {
    return SL_STATUS_BUSY;
  }
  // The function pointer is feeded to the static variable which is called in the IRQ handler
  dst_callback = dst_callback_function;
  /* Interrupt unmask */
  RSI_DST_IntrUnMask();
  NVIC_EnableIRQ(NVIC_DS_TIMER);
  // When the trigger is enabled, the bit 15 (deep sleep timer) is 0, so the comparison is to validate whether the
  // deep sleep timer trigger is enabled or not.
  if ((NPSS_INTR_MASK_CLR_REG & NPSS_TO_MCU_WAKEUP_INTR) != DISABLE) {
    return SL_STATUS_FAIL;
  }
#endif // SL_ENABLE_DST_WAKEUP_SOURCE
  return status;
}
#if defined(SL_ENABLE_DST_WAKEUP_SOURCE) && (SL_ENABLE_DST_WAKEUP_SOURCE == ENABLE)
/*******************************************************************************
 * Deep sleep timer based wakeup is de-initialized.
 ******************************************************************************/
static sl_status_t sli_si91x_power_manager_dst_deinit(void)
{
  sl_status_t status;
  RSI_DST_IntrMask();
  NVIC_DisableIRQ(NVIC_DS_TIMER);
  dst_callback = NULL;
  // Validate if deep sleep timer trigger is disabled or not and return the error code accordingly
  if ((NPSS_INTR_MASK_SET_REG & NPSS_TO_MCU_WAKEUP_INTR) == DISABLE) {
    status = SL_STATUS_FAIL;
  } else {
    status = SL_STATUS_OK;
  }
  return status;
}
#endif
/*******************************************************************************
 * Wireless based wakeup is initialized.
 ******************************************************************************/
sl_status_t sli_si91x_power_manager_wireless_wakeup_init(void)
{
  sl_status_t status = SL_STATUS_OK;

#if defined(SL_ENABLE_WIRELESS_WAKEUP_SOURCE) && (SL_ENABLE_WIRELESS_WAKEUP_SOURCE == ENABLE)
  // Wireless wake up is de-initialized.
  status = sli_si91x_power_manager_wireless_wakeup_deinit();
  if (status != SL_STATUS_OK) {
    // If status is not OK, return with the error code.
    return status;
  }

  NVIC_SetPriority(NPSS_TO_MCU_WIRELESS_INTR_IRQn, WIRELESS_WAKEUP_Priority);
  status = sl_si91x_power_manager_set_wakeup_sources(SL_SI91X_POWER_MANAGER_WIRELESS_WAKEUP, true);
  if (status != SL_STATUS_OK) {
    // If status is not OK, return with the error code.
    return status;
  }
  /* Enable NVIC */
  NVIC_EnableIRQ(NPSS_TO_MCU_WIRELESS_INTR_IRQn);
#endif // SL_ENABLE_WIRELESS_WAKEUP_SOURCE

  return status;
}
#if defined(SL_ENABLE_WIRELESS_WAKEUP_SOURCE) && (SL_ENABLE_WIRELESS_WAKEUP_SOURCE == ENABLE)
/*******************************************************************************
 * Wireless based wakeup is de-initialized.
 ******************************************************************************/
static sl_status_t sli_si91x_power_manager_wireless_wakeup_deinit(void)
{
  /* Disable NVIC */
  NVIC_DisableIRQ(NPSS_TO_MCU_WIRELESS_INTR_IRQn);
  return SL_STATUS_OK;
}
#endif

#if defined(SL_ENABLE_CALENDAR_WAKEUP_SOURCE) && (SL_ENABLE_CALENDAR_WAKEUP_SOURCE == ENABLE)
#if defined(ENABLE_ALARM) && (ENABLE_ALARM == ENABLE)
/*******************************************************************************
 * Set the period alarm time.
 ******************************************************************************/
static void set_periodic_alarm(uint32_t alarm_time)
{
  sl_calendar_datetime_config_t set_alarm_config, get_datetime_config;

  sl_si91x_calendar_get_date_time(&get_datetime_config);

  set_alarm_config = get_datetime_config;

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
#if (SL_SI91X_32KHZ_RC_CALIBRATION_ENABLED == ENABLE)
  // The flag is cleared indicating execution is happening through driver files.
  sli_si91x_clear_calibration_application_flag();
#endif

  sl_si91x_calendar_set_alarm(&set_alarm_config);

#if (SL_SI91X_32KHZ_RC_CALIBRATION_ENABLED == ENABLE)
  // The flag is cleared indicating execution is happening through driver files.
  sli_si91x_set_calibration_application_flag();
#endif
}
#endif // ENABLE_ALARM
#if defined(ENABLE_ALARM) && (ENABLE_ALARM == ENABLE)
/*******************************************************************************
 * Alarm based wakeup callback function.
 ******************************************************************************/
__attribute__((weak)) void calendar_alarm_callback_function(void)
{
#if defined(ENABLE_ALARM) && (ENABLE_ALARM == ENABLE) && (SL_SI91X_32KHZ_RC_CALIBRATION_ENABLED == DISABLE)
  set_periodic_alarm(ALARM_TIME_MSEC);
#endif // ENABLE_ALARM
}
#endif
#if defined(ENABLE_SECOND) && (ENABLE_SECOND == ENABLE)
/*******************************************************************************
 * Second based wakeup callback function.
 ******************************************************************************/
__attribute__((weak)) void calendar_second_callback_function(void)
{
}
#endif // ENABLE_SECOND
#endif // SL_ENABLE_CALENDAR_WAKEUP_SOURCE

#if defined(SL_ENABLE_GPIO_WAKEUP_SOURCE) && (SL_ENABLE_GPIO_WAKEUP_SOURCE == ENABLE)
/*******************************************************************************
 * Set the GPIO configuration for wakeup.
 ******************************************************************************/
static sl_status_t uulp_gpio_configuration(uint8_t pin, uint8_t polarity)
{
  sl_status_t status = SL_STATUS_OK;
  sl_si91x_gpio_interrupt_config_flag_t polarity_flag;
  if (pin > 3) {
    status = SL_STATUS_INVALID_PARAMETER;
    return status;
  }
  // Unregister NPSS GPIO interrupts
  status = sl_gpio_driver_unregister(UULP_GPIO_INSTANCE, pin, pin);
  if (status != SL_STATUS_OK) {
    return status;
  }
  status = sl_si91x_gpio_driver_select_uulp_npss_receiver(pin, SET);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Set NPSS GPIO pin MUX
  status = sl_si91x_gpio_driver_set_uulp_npss_pin_mux(pin, NPSS_GPIO_PIN_MUX_MODE2);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Set NPSS GPIO pin direction
  status = sl_si91x_gpio_driver_set_uulp_npss_direction(pin, (sl_si91x_gpio_direction_t)GPIO_INPUT);
  if (status != SL_STATUS_OK) {
    return status;
  }
  status = sl_si91x_gpio_driver_select_uulp_npss_polarity(pin, polarity);
  if (status != SL_STATUS_OK) {
    return status;
  }
  status = sl_si91x_gpio_driver_set_uulp_npss_wakeup_interrupt(pin);
  if (status != SL_STATUS_OK) {
    return status;
  }
  if (polarity == POLARITY_HIGH) {
    polarity_flag = SL_GPIO_INTERRUPT_LEVEL_HIGH;
  } else if (polarity == POLARITY_LOW) {
    polarity_flag = SL_GPIO_INTERRUPT_LEVEL_LOW;
  }
  status = sl_si91x_gpio_driver_configure_uulp_interrupt(polarity_flag, pin, (void *)&gpio_uulp_pin_interrupt_callback);
  if (status != SL_STATUS_OK) {
    return status;
  }
  status = sl_si91x_power_manager_set_wakeup_sources(SL_SI91X_POWER_MANAGER_GPIO_WAKEUP, true);
  if (status != SL_STATUS_OK) {
    // If status is not OK, return with the error code.
    return status;
  }
  return status;
}
/*******************************************************************************
 * GPIO based wakeup callback function.
 ******************************************************************************/
__attribute__((weak)) void gpio_uulp_pin_interrupt_callback(uint32_t pin_intr)
{
  (void)(pin_intr);
  // Empty funtion
}
#endif // SL_ENABLE_GPIO_WAKEUP_SOURCE

#if defined(SL_ENABLE_WDT_WAKEUP_SOURCE) && (SL_ENABLE_WDT_WAKEUP_SOURCE == ENABLE)
/*******************************************************************************
 * WDT based wakeup callback function.
 ******************************************************************************/
static void wdt_callback_function(void)
{
  // Empty funtion
}
#endif // SL_ENABLE_WDT_WAKEUP_SOURCE

#if defined(SL_ENABLE_DST_WAKEUP_SOURCE) && (SL_ENABLE_DST_WAKEUP_SOURCE == ENABLE)
/*******************************************************************************
 * Deep sleep timer interrupt IRQ handler.
 ******************************************************************************/
void DS_IRQ(void)
{
  // Deep sleep timer interrupt
  // It checks the trigger is enabled or not, then it clears the trigger
  // and calls the callback function
  if (NPSS_INTR_STATUS_REG & NPSS_TO_MCU_WAKEUP_INTR) {
    RSI_DST_TimerIntrClear();
    // Call the callback function;
    dst_callback();
  }
}
/*******************************************************************************
 * Deep sleep timer based wakeup callback function.
 ******************************************************************************/
__attribute__((weak)) void dst_callback_function(void)
{
}
#endif // SL_ENABLE_DST_WAKEUP_SOURCE

#if defined(SL_ENABLE_WIRELESS_WAKEUP_SOURCE) && (SL_ENABLE_WIRELESS_WAKEUP_SOURCE == ENABLE)
/*******************************************************************************
 * Wireless interrupt IRQ handler.
 ******************************************************************************/
void WIRELESS_WAKEUP_IRQ()
{
  /*Clear interrupt */
  RSI_PS_ClrWkpUpStatus(NPSS_TO_MCU_WIRELESS_INTR);
}
#endif // SL_ENABLE_WIRELESS_WAKEUP_SOURCE