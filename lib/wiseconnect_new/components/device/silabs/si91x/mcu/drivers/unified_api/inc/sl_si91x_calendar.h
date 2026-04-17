/******************************************************************************
* @file sl_si91x_calendar.h
* @brief Calendar API implementation
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

#ifndef SL_SI91X_CALENDAR_H_
#define SL_SI91X_CALENDAR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"
#include "rsi_power_save.h"
#include "rsi_rtc.h"
#include "base_types.h"

/***************************************************************************/
/**
 * @addtogroup CALENDAR Calendar
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 ******************************************************************************/

// -----------------------------------------------------------------------------
//Macros for IRQ Handlers
#define SLI_ALARM_IRQHandler    IRQ028_Handler     ///< Alarm IRQ Handler
#define SLI_MSEC_SEC_IRQHandler IRQ029_Handler     ///< RTC IRQ Handler
#define SLI_NVIC_ALARM          MCU_CAL_ALARM_IRQn ///< Alarm NVIQ enable
#define SLI_NVIC_MSEC_SEC       MCU_CAL_RTC_IRQn   ///< RTC NVIQ enable

// -----------------------------------------------------------------------------
// Data Types

#define TIME_CONVERSION_ENUM(name) \
  typedef uint8_t name;            \
  enum name##_enum ///< Time Conversion format enum declaration
#define RC_CLOCK_CALIBRATION_ENUM(name) \
  typedef uint8_t name;                 \
  enum name##_enum ///< Time Conversion format enum declaration
#define RO_CLOCK_CALIBRATION_ENUM(name) \
  typedef uint8_t name;                 \
  enum name##_enum ///< Time Conversion format enum declaration

typedef AON_CLK_T sl_calendar_clock_t;                   ///< Renaming clock type enum
typedef RTC_TIME_CONFIG_T sl_calendar_datetime_config_t; ///< Renaming datetime structure
typedef RTC_MONTH_T sl_calendar_month_t;                 ///< Renaming month structure
typedef RTC_DAY_OF_WEEK_T sl_calendar_days_of_week_t;    ///< Renaming days of week structure

/***************************************************************************/ /**
 * Typedef for the function pointer of the callback function
 ******************************************************************************/
typedef void (*calendar_callback_t)(void);

/**
 * @brief Structure to hold the parameters of clock calibration. Trigger time macros can be used to fill trigger time.
 */
typedef struct {
  boolean_t rc_enable_calibration;          ///< true to enable and false to disable RC calibration
  boolean_t rc_enable_periodic_calibration; ///< true to enable and false to disable RC periodic calibration
  uint8_t rc_trigger_time;                  ///< RC trigger time, 5 sec, 10 sec, 15 sec, 30 sec, 1 min, 2 min
  boolean_t ro_enable_calibration;          ///< true to enable and false to disable RO calibration
  boolean_t ro_enable_periodic_calibration; ///< true to enable and false to disable periodic calibration
  uint8_t ro_trigger_time;                  ///< RO trigger time, 1 sec, 2 sec, 4 sec, 8 sec
} clock_calibration_config_t;

/**
 * @brief Enumeration to represent time conversion format.
 */
TIME_CONVERSION_ENUM(time_conversion_enum){
  TIME_FORMAT_UNIX, ///< Number of seconds since January 1, 1970, 00:00. Type is signed, so represented on 31 bit.
  TIME_FORMAT_NTP,  ///< Number of seconds since January 1, 1900, 00:00. Type is unsigned, so represented on 32 bit.
  TIME_FORMAT_LAST, ///< Last member of enum for validation
};

/**
 * @brief Enumeration to represent the different RC clock calibration configurations.
 */
RC_CLOCK_CALIBRATION_ENUM(rc_clock_calibration_enum){
  SL_RC_FIVE_SEC,    ///< RC 5 Second Calibration
  SL_RC_TEN_SEC,     ///< RC 10 Second Calibration
  SL_RC_FIFTEEN_SEC, ///< RC 15 Second Calibration
  SL_RC_THIRTY_SEC,  ///< RC 30 Second Calibration
  SL_RC_ONE_MIN,     ///< RC 1 Minute Calibration
  SL_RC_TWO_MIN,     ///< RC 2 Minute Calibration
  SL_RC_LAST_ENUM,   ///< Last member of enum for validation
};

/**
 * @brief Enumeration to represent different RO clock calibration configurations.
 */
RO_CLOCK_CALIBRATION_ENUM(ro_clock_calibration_enum){
  SL_RO_ONE_SEC,   ///< RO 1 Second Calibration
  SL_RO_TWO_SEC,   ///< RO 2 Second Calibration
  SL_RO_FOUR_SEC,  ///< RO 4 Second Calibration
  SL_RO_EIGHT_SEC, ///< RO 8 Second Calibration
  SL_RO_LAST_ENUM, ///< Last member of enum for validation
};

/**
 * @brief Structure to hold the different versions of peripheral API.
 */
typedef struct {
  uint8_t release; ///< Release version number
  uint8_t major;   ///< SQA version number
  uint8_t minor;   ///< DEV version number
} sl_calendar_version_t;

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/
/**
 * @brief This API is no longer supported due to the restriction on peripheral drivers to configuring clocks.
 * @brief To configure and initialize Calendar (that is, the RTC clock).
 * 
 * @details This API is no longer supported due to the restriction on peripheral drivers to configure clock.
 *
 * @param[in] clock_type (sl_calendar_clock_t) Enum for RTC Clock Type (RC or XTAL)
 *
 * @return status, error code as follows:
 *         - SL_STATUS_OK -  return ok to support backward compatibility.
 *
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_calendar_set_configuration(sl_calendar_clock_t clock_type) SL_DEPRECATED_API_WISECONNECT_4_0;

/***************************************************************************/
/**
 * @brief To configure and initialize Calendar (that is, the RTC clock).
 * 
 * @details Sets the date and time of the Calendar RTC.
 * The input parameters include a date-time structure, with the following members:
 *     - date Pointer to the Date Configuration Structure
 *     - Century (uint8_t) Century (0-3)
 *     - Year (uint8_t) Year (0-99)
 *     - Month (enum) Month from the sl_calendar_month_t enum
 *     - DayOfWeek (enum) Day of Week from the sl_calendar_days_of_week_t enum
 *     - Day (uint8_t) Day (1-31)
 *     - Hour (uint8_t) Hour (0-23)
 *     - Minute (uint8_t) Minute (0-59)
 *     - Second (uint8_t) Second (0-59)
 *     - Milliseconds (uint16_t) Milliseconds (0-999)
 * 
 * @pre Pre-conditions:
 *     - \ref sl_si91x_calendar_init 
 * 
 * @param[in] config Pointer to the Date Configuration Structure (sl_calendar_datetime_config_t).
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_calendar_set_date_time(sl_calendar_datetime_config_t *config);

/***************************************************************************/
/**
 * @brief To fetch the current date and time of an existing Calendar that is, RTC clock.
 * 
 * @details The input parameter consists of a date-time structure. The structure is updated with the current date-time parameters.
 * The members of the structure are listed below:
 *     - date Pointer to the Date Configuration Structure
 *     - Century (uint8_t) Century (0-3)
 *     - Year (uint8_t) Year (0-99)
 *     - Month (enum) Month from the sl_calendar_month_t enum
 *     - DayOfWeek (enum) Day of Week from the sl_calendar_days_of_week_t enum
 *     - Day (uint8_t) Day (1-31)
 *     - Hour (uint8_t) Hour (0-23)
 *     - Minute (uint8_t) Minute (0-59)
 *     - Second (uint8_t) Second (0-59)
 *     - Milliseconds (uint16_t) Milliseconds (0-999)
 * 
 * @pre Pre-conditions:
 *     - \ref sl_si91x_calendar_init 
 *     - \ref sl_si91x_calendar_set_date_time 
 * 
 * @param[in] config Pointer to the Date Configuration Structure (sl_calendar_datetime_config_t).
 * 
 * @return status Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_calendar_get_date_time(sl_calendar_datetime_config_t *config);

/***************************************************************************/
/**
 * @brief To calibrate the RC clock.
 * 
 * @details This API is no longer supported due to the restriction on peripheral drivers to configure clock.
 *
 * @param[in] clock_calibration_config Pointer to the clock calibration structure (\ref clock_calibration_config_t).
 *
 * @return status, error code as follows:
 *         - SL_STATUS_OK -  return ok to support backward compatibility.
 *
 * For more information on the status documentation, please refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_calendar_rcclk_calibration(clock_calibration_config_t *clock_calibration_config)
  SL_DEPRECATED_API_WISECONNECT_4_0;

/***************************************************************************/
/**
 * @brief To calibrate the RO clock.
 * 
 * @details This API is no longer supported due to the restriction on peripheral drivers to configure clock.
 *
 * @param[in] clock_calibration_config Pointer to the clock calibration structure (\ref clock_calibration_config_t).
 *
 * @return status, error code as follows:
 *         - SL_STATUS_OK -  return ok to support backward compatibility.
 *
 * For more information on the status documentation, please refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_calendar_roclk_calibration(clock_calibration_config_t *clock_calibration_config)
  SL_DEPRECATED_API_WISECONNECT_4_0;

/***************************************************************************/
/**
 * @brief To register the callback for a one-millisecond trigger and enable it.
 * 
 * @details At the time of the trigger, the callback function passed as an argument to this function is called.
 * It expects the callback function pointer as an input argument.
 * Before calling this function again, it is mandatory to call \ref sl_si91x_calendar_unregister_msec_trigger_callback,
 * otherwise, it returns the SL_STATUS_BUSY error code.
 *
 * @param[in] callback Callback function pointer (\ref calendar_callback_t) to be called when the millisecond 
 * interrupt is triggered.
 *
 * @return Status code indicating the results:
 *         - SL_STATUS_OK  - Success. 
 *         - SL_STATUS_BUSY  - The callback is already registered. 
 *           Deregister the previous callback before registering a new one.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_calendar_register_msec_trigger_callback(calendar_callback_t callback);

/***************************************************************************/
/**
 * @brief To register the callback for a one-second trigger and enable it.
 * 
 * @details At the time of the trigger, the callback function passed as an argument to this function is called.
 * It expects the callback function pointer as an input argument.
 * Before calling this function again, it is mandatory to call \ref sl_si91x_calendar_unregister_sec_trigger_callback,
 * otherwise, it returns the SL_STATUS_BUSY error code.
 *
 * @param[in] callback Callback function pointer (\ref calendar_callback_t) to be called when the second 
 * interrupt is triggered.
 *
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_BUSY  - The callback is already registered. 
 *           Deregister the previous callback before registering a new one.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_calendar_register_sec_trigger_callback(calendar_callback_t callback);

/***************************************************************************/
/**
 * @brief To register the callback for an alarm trigger and enable it.
 * 
 * @details At the time of the trigger, the callback function passed as an argument to this function is called.
 * It expects the callback function pointer as an input argument.
 * Before calling this function again, it is mandatory to call \ref sl_si91x_calendar_unregister_alarm_trigger_callback.
 * Otherwise, it returns the SL_STATUS_BUSY error code.
 *
 * @param[in] callback Callback function pointer (\ref calendar_callback_t) to be called when the alarm 
 * interrupt is triggered.
 *
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_BUSY  - The callback is already registered. 
 *           Deregister the previous callback before registering a new one.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_calendar_register_alarm_trigger_callback(calendar_callback_t callback);

/***************************************************************************/
/**
 * @brief To deregister the callback for one millisecond trigger and disable it.
 * 
 * @note It is mandatory to call this function before registering the callback again.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_calendar_register_msec_trigger_callback
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - The function failed.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_calendar_unregister_msec_trigger_callback(void);

/***************************************************************************/
/**
 * @brief To deregister the callback for the one-second trigger and disable it.
 * 
 * @note It is mandatory to call this function before registering the callback again.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_calendar_register_sec_trigger_callback
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - Function failed.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_calendar_unregister_sec_trigger_callback(void);

/***************************************************************************/
/**
 * @brief To deregister the callback for the alarm trigger and disable it.
 * 
 * @note It is mandatory to call this function before registering the callback again.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_calendar_register_alarm_trigger_callback
 * 
 * @return Status code indicating the results:
 *         - SL_STATUS_OK  - Success. 
 *         - SL_STATUS_FAIL  - Function failed.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_calendar_unregister_alarm_trigger_callback(void);

/***************************************************************************/
/**
 * @brief To set the date and time of a new alarm in RTC.
 * 
 * @details It is a one-shot alarm; after triggering the alarm, it elapses.
 * The input parameters consist of a date-time structure. The members of the structure are listed below:
 *     - date Pointer to the Date Configuration Structure
 *     - Century (uint8_t) Century (0-3)
 *     - Year (uint8_t) Year (0-99)
 *     - Month (enum) Month from the sl_calendar_month_t enum
 *     - DayOfWeek (enum) Day of Week from the sl_calendar_days_of_week_t enum
 *     - Day (uint8_t) Day (1-31)
 *     - Hour (uint8_t) Hour (0-23)
 *     - Minute (uint8_t) Minute (0-59)
 *     - Second (uint8_t) Second (0-59)
 *     - Milliseconds (uint16_t) Milliseconds (0-999)
 * 
 * @pre Pre-conditions:
 *     - \ref sl_si91x_calendar_init 
 * 
 * @param[in] alarm Pointer to the Date Configuration Structure (\ref sl_calendar_datetime_config_t). 
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid. 
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_calendar_set_alarm(sl_calendar_datetime_config_t *alarm);

/***************************************************************************/
/**
 * @brief To fetch the date and time of an existing alarm set in RTC.
 * 
 * @details The input parameters consist of a dummy date-time structure.
 * The structure is updated with the current date-time parameters.
 * The members of the structure are listed below:
 *     - date Pointer to the Date Configuration Structure
 *     - Century (uint8_t) Century (0-3)
 *     - Year (uint8_t) Year (0-99)
 *     - Month (enum) Month from the sl_calendar_month_t enum
 *     - DayOfWeek (enum) Day of Week from the sl_calendar_days_of_week_t enum
 *     - Day (uint8_t) Day (1-31)
 *     - Hour (uint8_t) Hour (0-23)
 *     - Minute (uint8_t) Minute (0-59)
 *     - Second (uint8_t) Second (0-59)
 *     - Milliseconds (uint16_t) Milliseconds (0-999)
 * 
 * @pre Pre-conditions:
 *     - \ref sl_si91x_calendar_init 
 *     - \ref sl_si91x_calendar_set_alarm 
 * 
 * @param[in] alarm Pointer to the Date Configuration Structure (\ref sl_calendar_datetime_config_t).
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid. 
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_calendar_get_alarm(sl_calendar_datetime_config_t *alarm);

/***************************************************************************/
/**
 * @brief To build the structure for date-time configuration.
 * 
 * @details By providing the following parameters as input, it validates and fills the 
 * date-time configuration structure with the respective values.
 * 
 * @param[in] date Pointer to the Date Configuration Structure (\ref sl_calendar_datetime_config_t)
 * @param[in] Century (uint8_t) Century (0-3)
 * @param[in] Year (uint8_t) Year (0-99)
 * @param[in] Month (enum) Month from the sl_calendar_month_t enum
 * @param[in] DayOfWeek (enum) Day of Week from the sl_calendar_days_of_week_t enum
 * @param[in] Day (uint8_t) Day (1-31)
 * @param[in] Hour (uint8_t) Hour (0-23)
 * @param[in] Minute (uint8_t) Minute (0-59)
 * @param[in] Second (uint8_t) Second (0-59)
 * @param[in] Milliseconds (uint16_t) Milliseconds (0-999)
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_calendar_build_datetime_struct(sl_calendar_datetime_config_t *date,
                                                    uint8_t Century,
                                                    uint8_t Year,
                                                    sl_calendar_month_t Month,
                                                    sl_calendar_days_of_week_t DayOfWeek,
                                                    uint8_t Day,
                                                    uint8_t Hour,
                                                    uint8_t Minute,
                                                    uint8_t Second,
                                                    uint16_t Milliseconds);

/***************************************************************************/
/**
 * @brief To convert a Unix timestamp to an NTP timestamp.
 * 
 * @details This function converts a given Unix timestamp to an NTP timestamp.
 * The Unix timestamp is provided as an input parameter, and the resulting NTP timestamp
 * is stored in the variable pointed to by ntp_time.
 * 
 * @param[in] time Unix timestamp (uint32_t)
 * @param[out] ntp_time Pointer to a variable to store the NTP timestamp (uint32_t)
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_calendar_convert_unix_time_to_ntp_time(uint32_t time, uint32_t *ntp_time);

/***************************************************************************/
/**
 * @brief To convert NTP timestamp to Unix timestamp.
 * 
 * @details This function converts a given NTP timestamp to a Unix timestamp.
 * 
 * @param[in] ntp_time NTP timestamp.
 * @param[out] time Variable to store the Unix timestamp.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER       - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_calendar_convert_ntp_time_to_unix_time(uint32_t ntp_time, uint32_t *time);

/***************************************************************************/
/**
 * @brief To return the state of the one millisecond trigger of RTC (enabled or disabled).
 * 
 * @details If enabled, returns true. If disabled, returns false.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_calendar_register_msec_trigger_callback
 * 
 * @return (boolean) true if the trigger is enabled, false otherwise.
 ******************************************************************************/
boolean_t sl_si91x_calendar_is_msec_trigger_enabled(void);

/***************************************************************************/
/**
 * @brief To check the state of the one-second trigger of RTC.
 * 
 * @details This function returns the state of the one-second trigger of RTC (enabled or disabled).
 * If enabled, it returns true. If disabled, it returns false.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_calendar_register_sec_trigger_callback
 * 
 * @return (boolean) true if the trigger is enabled, false otherwise.
 ******************************************************************************/
boolean_t sl_si91x_calendar_is_sec_trigger_enabled(void);

/***************************************************************************/
/**
 * @brief To check the state of the alarm trigger of RTC.
 * 
 * @details This function returns the state of the alarm trigger of RTC (enabled or disabled).
 * If enabled, it returns true. If disabled, it returns false.
 *         
 * @pre Pre-condition:
 *      - \ref sl_si91x_calendar_register_alarm_trigger_callback
 * 
 * @return (boolean) true if the trigger is enabled, false otherwise.
 ******************************************************************************/
boolean_t sl_si91x_calendar_is_alarm_trigger_enabled(void);

/***************************************************************************/ /**
 * Convert Unix timestamp to Calendar RTC timestamp.
 * Timezone is not part of Calendar, assumes and converts in GMT format
 * 
 * @param[in] unix_time (uint32_t) Unix timestamp
 * @param[in] cal_date_time (sl_calendar_datetime_config_t *) Pointer to the Date Configuration Structure
 * @return  status 0 if successful, else error code as follows:
 *         - SL_STATUS_OK  - Success
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_calendar_convert_unix_time_to_calendar_datetime(uint32_t unix_time,
                                                                     sl_calendar_datetime_config_t *cal_date_time);

/***************************************************************************/ /**
 * Convert Calendar RTC timestamp to Unix timestamp.
 * Timezone is not part of Calendar, converts in GMT format
 * 
 * @param[in] cal_date_time (sl_calendar_datetime_config_t *) Pointer to the Date Configuration Structure
 * @param[in] unix_time (uint32_t *) Pointer to the Unix timestamp variable
 * @return  status 0 if successful, else error code as follows:
 *         - SL_STATUS_OK  - Success
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid
 *         - SL_STATUS_INVALID_RANGE  - Parameters are invalid
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_calendar_convert_calendar_datetime_to_unix_time(sl_calendar_datetime_config_t *cal_date_time,
                                                                     uint32_t *unix_time);

/***************************************************************************/ /**
 * Starts the Calendar RTC.
 * @pre Pre-conditions:
 *      - \ref sl_si91x_calendar_init
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_calendar_rtc_start(void)
{
  RSI_RTC_Start(RTC);
}

/***************************************************************************/
/**
 * @brief To stop the Calendar RTC.
 * 
 * @details This function stops the RTC clock.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_calendar_init 
 *      - \ref sl_si91x_calendar_rtc_start
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_calendar_rtc_stop(void)
{
  RSI_RTC_Stop(RTC);
}

/***************************************************************************/
/**
 * @brief To initialize the calibration for Calendar clocks.
 * 
 * @details This function initiates the calibration for Calendar clocks. 
 * It is mandatory to call this function before calling RO clock calibration or
 * RC clock calibration function.
 *
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_calendar_calibration_init(void)
{
  RSI_RTC_CalibInitilization();
}

/***************************************************************************/
/**
 * @brief To clear the one millisecond trigger.
 * 
 * @details This function clears the one millisecond trigger. It is generally called in the IRQ handler.
 * 
 * @pre Pre-conditions:
 *      - The one millisecond trigger must be set and the IRQ handler must be configured.
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_calendar_clear_msec_trigger(void)
{
  RSI_RTC_IntrClear(RTC_MSEC_INTR);
}

/***************************************************************************/
/**
 * @brief To clear the one-second trigger.
 * 
 * @details This function clears the one-second trigger. It is generally called in the IRQ handler.
 * 
 * @pre Pre-conditions:
 *      - The one-second trigger must be set and the IRQ handler must be configured.
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_calendar_clear_sec_trigger(void)
{
  RSI_RTC_IntrClear(RTC_SEC_INTR);
}

/***************************************************************************/
/**
 * @brief To clear the alarm trigger.
 * 
 * @details This function clears the alarm trigger. It is generally called in the IRQ handler.
 * 
 * @pre Pre-conditions:
 *      - The alarm must be set and the IRQ handler must be configured.
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_calendar_clear_alarm_trigger(void)
{
  RSI_RTC_IntrClear(RTC_ALARM_INTR);
}

/***************************************************************************/
/**
 * @brief To initialize calendar operation.
 * 
 * @details This function powers up the RTC domain and starts the calendar clock.
 * 
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_calendar_init(void)
{
  RSI_RTC_Init(RTC);
}

/***************************************************************************/
/**
 * @brief To de-initialize calendar operation.
 * 
 * @details This function powers down the RTC domain and stops the calendar clock.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_calendar_init
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_calendar_deinit(void)
{
  RSI_PS_NpssPeriPowerDown(SLPSS_PWRGATE_ULP_MCURTC);
}

/***************************************************************************/
/**
 * @brief To get the calendar version.
 * 
 * @details This function returns the API version of the calendar.
 * 
 * @returns \ref sl_calendar_version_t type version
 ******************************************************************************/
sl_calendar_version_t sl_si91x_calendar_get_version(void);

/** @} end group CALENDAR */

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY! ***********************
/**
* @addtogroup CALENDAR Calendar
* @{
*
* @details
*
* 
* @section CALENDAR_Intro Introduction
*
* The Calendar application manages dates, events, alarms, and reminders within hardware constraints.
* This section provides a brief overview of building such an application.
*
* 
* @section CALENDAR_Config Configuration
*
* The Calendar provides three types of triggers: Second, Millisecond, and Alarm.
*
* - **Second Trigger**: Provides a callback every second.
*     To configure the Second Trigger, use @ref sl_si91x_calendar_register_sec_trigger_callback().
*
* - **Millisecond Trigger**: Provides a callback every millisecond.
*     To configure the Millisecond Trigger, use @ref sl_si91x_calendar_register_msec_trigger_callback().
*
* - **Alarm Trigger**: Provides a callback at the alarm time configured by the user.
*     To configure the alarm time, use @ref sl_si91x_calendar_set_alarm(), and to configure the alarm trigger, use @ref sl_si91x_calendar_register_alarm_trigger_callback().
*
* To set up a calendar in the si91x, use the @ref sl_calendar_clock_t structure. 
* It is mandatory to call @ref sl_si91x_calendar_calibration_init function before calling RO clock calibration or
* RC clock calibration function.
* 
* For more detailed information on configuring available parameters, see the respective peripheral example README document.
*
* 
* @section CALENDAR_Usage Usage
*
* Common Calendar functions can be used after the Calendar structures are specified, passing an instance of @ref sl_calendar_datetime_config_t.
* These functions will initiate and configure the Calendar, following the implementation flow below:
*
* 1. @ref sl_si91x_calendar_init - Initializes the calendar module.
* 2. @ref sl_si91x_calendar_build_datetime_struct - Builds the datetime structure.
* 3. @ref sl_si91x_calendar_set_date_time - Sets the date and time.
* 4. @ref sl_si91x_calendar_deinit - Deinitialize the calendar module.
*/
/** @} end group CALENDAR ********************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_CALENDAR_H_ */
