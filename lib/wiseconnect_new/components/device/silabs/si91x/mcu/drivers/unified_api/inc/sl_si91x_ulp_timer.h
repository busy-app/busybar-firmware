/******************************************************************************
* @file sl_si91x_ulp_timer.h
* @brief ULP Timers API implementation
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

#ifndef SL_SI91X_ULP_TIMER_
#define SL_SI91X_ULP_TIMER_

#ifdef __cplusplus
extern "C" {
#endif

//// Includes
#include "sl_status.h"
#include "rsi_timers.h"
#include "base_types.h"
#include "rsi_pll.h"

/***************************************************************************/
/**
 * @addtogroup ULP-TIMER ULP Timers
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 * 
 ******************************************************************************/

// Macros
#define SL_TIMER_MATCH_VALUE_DEFAULT \
  20000000 ///< default 1-second timer match-value for down-counter timer-type with 20Mhz clock
#define SL_ULP_TIMER_CLK_TYPE_STATIC  1 ///< Static clock type for the ULP timer
#define SL_ULP_TIMER_CLK_TYPE_DYNAMIC 0 ///< Dynamic clock type for the ULP timer
// -----------------------------------------------------------------------------
// Data Types
/***************************************************************************/ /**
 * Typedef for the function pointer of the callback function
 ******************************************************************************/
typedef void (*ulp_timer_callback_t)(void);

typedef CLK_ENABLE_T ulp_timer_clock_t;        ///< Renaming clock type enum
typedef ulp_timer_dir_t ulp_timer_direction_t; ///< Renaming clock type enum

/***************************************************************************/
/**
 * @brief Enumeration to represent ULP timer instances.
 * 
 * @details This enumeration defines the possible instances of the Ultra Low-Power (ULP) timer.
 */
typedef enum {
  ULP_TIMER_0,    ///< ULP Timer 0 Instance
  ULP_TIMER_1,    ///< ULP Timer 1 Instance
  ULP_TIMER_2,    ///< ULP Timer 2 Instance
  ULP_TIMER_3,    ///< ULP Timer 3 Instance
  ULP_TIMER_LAST, ///< Last member of the enum for validation
} ulp_timer_instance_t;

/***************************************************************************/
/**
 * @brief Enumeration to represent ULP timer modes.
 * 
 * @details This enumeration defines the possible modes for the Ultra Low-Power (ULP) timer.
 */
typedef enum {
  ULP_TIMER_MODE_ONESHOT,  ///< ULP Timer one-shot mode
  ULP_TIMER_MODE_PERIODIC, ///< ULP Timer periodic mode
  ULP_TIMER_MODE_LAST,     ///< Last member of the enum for validation
} ulp_timer_mode_t;

/***************************************************************************/
/**
 * @brief Enumeration to represent ULP timer types.
 * 
 * @details This enumeration defines the possible types for the Ultra Low-Power (ULP) timer.
 */
typedef enum {
  ULP_TIMER_TYP_DEFAULT, ///< ULP Timer normal down counter type
  ULP_TIMER_TYP_1US,     ///< ULP Timer one microsecond type
  ULP_TIMER_TYP_256US,   ///< ULP Timer 256 microsecond type
  ULP_TIMER_TYP_LAST,    ///< Last member of the enum for validation
} ulp_timer_type_t;

/***************************************************************************/
/**
 * @brief Enumeration to represent values of clock sources to select as Timer clock.
 * 
 * @details This enumeration defines the possible clock sources that can be selected as the timer clock for the Ultra Low-Power (ULP) timer.
 */
typedef enum {
  ULP_TIMER_REF_CLK_SRC,        ///< Reference clock input source
  ULP_TIMER_RESERVED_1_CLK_SRC, ///< Reserved
  ULP_TIMER_32KHZ_RC_CLK_SRC,   ///< 32 kHz RC clock input source
  ULP_TIMER_32KHZ_XTAL_CLK_SRC, ///< 32 kHz XTAL clock input source
  ULP_TIMER_MHZ_RC_CLK_SRC,     ///< 32 MHz RC clock input source
  ULP_TIMER_RESERVED_2_CLK_SRC, ///< Reserved
  ULP_TIMER_ULP_CLK_SRC_LAST,   ///< Last member of the enum for validation
} ulp_timer_clk_input_source_t;

/***************************************************************************/
/**
 * @brief Structure to hold the parameters of timer clock-source configurations.
 * 
 * @details This structure defines the parameters required for configuring the clock source of the Ultra Low-Power (ULP) timer.
 */
typedef struct {
  uint8_t ulp_timer_clk_type; ///< true to enable static and false to enable dynamic clock type, \ref ulp_timer_clock_t
  boolean_t
    ulp_timer_sync_to_ulpss_pclk;  ///< true to enable and false to disable ULP timer in synchronous mode to ULPSS pclk
  uint8_t ulp_timer_clk_input_src; ///< Timer input clock source, \ref ulp_timer_clk_input_source_t for possible values
  boolean_t ulp_timer_skip_switch_time; ///< true to wait and false to skip waiting for switching timer clock
} ulp_timer_clk_src_config_t;

/***************************************************************************/
/**
 * @brief Structure to hold the parameters of timer configurations.
 * 
 * @details This structure defines the parameters required for configuring the Ultra Low-Power (ULP) timer.
 */
typedef struct {
  uint8_t timer_num;          ///< Timer number, for possible values see \ref ulp_timer_instance_t
  uint8_t timer_mode;         ///< Timer mode, for possible values see \ref ulp_timer_mode_t
  uint8_t timer_type;         ///< Timer type, for possible values see \ref ulp_timer_type_t
  uint32_t timer_match_value; ///< Timer match value SL_ULP_TIMER_MATCH_VALUE (delay time) in microseconds
  uint8_t timer_direction;    ///< Timer direction, for possible values see \ref ulp_timer_direction_t
} ulp_timer_config_t;

/***************************************************************************/
/**
 * @brief Structure to hold the different versions of the peripheral API.
 * 
 * @details This structure defines the version information for the Ultra Low-Power (ULP) timer peripheral API.
 */
typedef struct {
  uint8_t release; ///< Release version number.
  uint8_t major;   ///< Major version number.
  uint8_t minor;   ///< Minor version number.
} sl_ulp_timer_version_t;

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/
/**
 * @brief To initialize the ULP-Timer by enabling its clock source.
 * 
 * @details This API configures the clock frequency, clock type, enables/disables synchronization to ULPSS pclk, 
 *          and skips/allows waiting for timer clock switching.
 * 
 * @param[in] timer_clk_ptr Pointer to the timer clock configuration structure.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, timer clock-source parameters configured properly.
 *         - SL_STATUS_INVALID_PARAMETER  - Timer clock type or timer clock source values are invalid.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer
 *         - SL_STATUS_INVALID_CONFIGURATION  - Timer clock-source configuration structure members have invalid configurations.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_ulp_timer_init(ulp_timer_clk_src_config_t *timer_clk_ptr);

/***************************************************************************/
/**
 * @brief To configure and enable the ULP-Timer input clock source.
 * 
 * @details This API configures the clock frequency, clock type, enables/disables synchronization to ULPSS pclk, 
 *          and determines whether to skip/allow waiting for timer clock switching.
 *          This function is invoked within the \ref sl_si91x_ulp_timer_init API.
 * 
 * @note 
 * @li There are two XTAL clock sources: internal and external.
 * To enable the external XTAL clock source, you must configure the UULP_GPIO pins
 * using \ref sl_si91x_ulp_timer_configure_xtal_clock, and it should be called first.
 * @li To use the SOC clock source, you need to use the \ref sl_si91x_ulp_timer_configure_soc_clock API 
 * to configure the ULPSS SOC rather than the clock configuration API.
 *
 * @param[in] timer_clk_ptr Pointer to the timer clock configuration structure \ref ulp_timer_clk_src_config_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, timer clock-source parameters configured properly.
 *         - SL_STATUS_INVALID_PARAMETER  - Timer clock type or timer clock source values are invalid.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer. 
 *         - SL_STATUS_INVALID_CONFIGURATION  - Timer clock-source configuration structure members have invalid configurations.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_ulp_timer_configure_clock(ulp_timer_clk_src_config_t *timer_clk_ptr);

/***************************************************************************/
/**
 * @brief To configure the ULP-Timer parameters.
 * 
 * @details This API configures the parameters such as timer number, mode, type, match value, and direction.
 * Additionally, it configures the integral and fractional values of clock cycles per microsecond or
 * per 256 microseconds as per the selected timer type.
 *
 * @pre Pre-conditions:
 *      - \ref sl_si91x_ulp_timer_init();
 *
 * @param[in] timer_config_ptr Pointer to the timer configuration structure \ref ulp_timer_config_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, timer parameters are configured properly.
 *         - SL_STATUS_INVALID_PARAMETER  - Timer configuration structure member 'timer_direction' has an invalid value.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 *         - SL_STATUS_INVALID_MODE  - Timer configuration structure member 'timer_mode' has an invalid value.
 *         - SL_STATUS_INVALID_TYPE  - Timer configuration structure member 'timer_type' has an invalid value.
 *         - SL_STATUS_INVALID_INDEX  - Timer configuration structure member 'timer_num' has an invalid value.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_ulp_timer_set_configuration(ulp_timer_config_t *timer_config_ptr);

/***************************************************************************/
/**
 * @brief To retrieve the match value for ULP timer.
 * 
 * @details This API calculates the match value for a given ULP timer type based on the 
 *          provided time in microseconds. The match value corresponds to the number of 
 *          clock cycles required for the timer to reach the specified time.
 *          
 *          The function supports different timer types, including:
 *           - ULP_TIMER_TYP_DEFAULT: Uses the clock cycles per microsecond to calculate the match value.
 *           - ULP_TIMER_TYP_1US: Returns the match value as the number of microseconds.
 *           - ULP_TIMER_TYP_256US: Returns the match value as the number of 256-microsecond units.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_ulp_timer_init();
 * 
 * @param[in] timer_type Enum for ULP-timer type (default, 1us, 256us), see \ref ulp_timer_type_t for possible values.
 * @param[in] time_us Time in microseconds for which the match value needs to be calculated.
 * @param[out] match_value Pointer to store the calculated match value.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK               - Successfully calculated the match value.
 *         - SL_STATUS_INVALID_TYPE    - Timer configuration structure member 'timer_type' has an invalid value.
 *         - SL_STATUS_INVALID_PARAMETER - The 'time_us' parameter is invalid for the specified timer type.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_ulp_timer_get_match_value(ulp_timer_type_t timer_type, uint32_t time_us, uint32_t *match_value);

/***************************************************************************/
/**
 * @brief To start the ULP-Timer.
 * 
 * @details This API is used to start the Ultra Low-Power (ULP) timer.
 * 
 * @note The timer instance must not be in a running state when calling this API.
 *
 * @pre Pre-conditions:
 *      - \ref sl_si91x_ulp_timer_init
 *      - \ref sl_si91x_ulp_timer_set_configuration
 *      - \ref sl_si91x_ulp_timer_register_timeout_callback
 * 
 * @param[in] timer_num Enum for ULP-timer number (0 to 3), see \ref ulp_timer_instance_t for possible values.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully started the timer instance.
 *         - SL_STATUS_INVALID_INDEX  - The 'timer_num' parameter value is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_ulp_timer_start(ulp_timer_instance_t timer_num);

/***************************************************************************/
/**
 * @brief To stop the ULP-Timer instance.
 * 
 * @details This API stops the Ultra Low-Power (ULP) timer instance but does not disable it. 
 *          To disable the timer, use \ref sl_si91x_ulp_timer_deinit.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_ulp_timer_init
 *      - \ref sl_si91x_ulp_timer_set_configuration 
 *      - \ref sl_si91x_ulp_timer_start 
 * 
 * @param[in] timer_num Enum for ULP-timer number (0 to 3), see \ref ulp_timer_instance_t for possible values.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully stopped the timer instance.
 *         - SL_STATUS_INVALID_INDEX  - The 'timer_num' parameter value is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_ulp_timer_stop(ulp_timer_instance_t timer_num);

/***************************************************************************/
/**
 * @brief To restart the ULP-Timer.
 * 
 * @details This API restarts the Ultra Low-Power (ULP) timer instance. 
 *          If the timer is running, it stops the timer first and then starts it again. 
 *          If the timer is stopped, it simply starts the timer.
 *
 * @pre Pre-conditions:
 *      - \ref sl_si91x_ulp_timer_init
 *      - \ref sl_si91x_ulp_timer_set_configuration 
 *      - \ref sl_si91x_ulp_timer_start 
 * 
 * @param[in] timer_num Enum for ULP-timer number, see \ref ulp_timer_instance_t for possible values.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully restarted the timer instance.
 *         - SL_STATUS_INVALID_INDEX  - The 'timer_num' parameter value is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_ulp_timer_restart(ulp_timer_instance_t timer_num);

/***************************************************************************/
/**
 * @brief To set the ULP-Timer type.
 * 
 * @details This API sets the ULP-Timer type, which includes 256 µs, 1 µs, and count-down types.
 *          - 1 microsecond type: The time unit is 1 µs.
 *          - 256 microsecond type: The time unit is 256 µs.
 *          - Count-down timer type: The time unit is one clock cycle period.
 * 
 * @note This API should be used when the timer is stopped.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_ulp_timer_init
 *      - \ref sl_si91x_ulp_timer_stop
 * 
 * @param[in] timer_num Enum for ULP-timer number, see \ref ulp_timer_instance_t for possible values.
 * @param[in] timer_type Enum for ULP-timer type, see \ref ulp_timer_type_t for possible values.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully set the timer type.
 *         - SL_STATUS_INVALID_TYPE  - 'timer_type' parameter value is invalid.
 *         - SL_STATUS_INVALID_INDEX  - 'timer_num' parameter value is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_ulp_timer_set_type(ulp_timer_instance_t timer_num, ulp_timer_type_t timer_type);

/***************************************************************************/
/**
 * @brief To set the ULP-Timer direction.
 * 
 * @details This API sets the Ultra Low-Power (ULP) timer direction as either up-counting or down-counting.
 *          By default, it operates as a down-counter.
 * 
 * @note The API should be used when the timer is stopped.
 *
 * @pre Pre-conditions:
 *      - \ref sl_si91x_ulp_timer_init
 *      - \ref sl_si91x_ulp_timer_stop 
 * 
 * @param[in] timer_num Enum for ULP-timer number, see \ref ulp_timer_instance_t for possible values.
 * @param[in] timer_direction Enum for ULP-timer direction, see \ref ulp_timer_direction_t for possible values.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully set the timer direction.
 *         - SL_STATUS_INVALID_PARAMETER  - The 'timer_direction' parameter value is invalid.
 *         - SL_STATUS_INVALID_INDEX  - The 'timer_num' parameter value is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_ulp_timer_set_direction(ulp_timer_instance_t timer_num, ulp_timer_direction_t timer_direction);

/***************************************************************************/
/**
 * @brief To set the ULP-Timer mode as one-shot or periodic.
 * 
 * @details In a one-shot operation, the timer counts down to the timeout and then generates a single interrupt 
 *          after which it returns to the idle state. In contrast, in periodic operation, the timer continuously generates interrupts 
 *          at intervals equal to the timeout period.
 * 
 * @note This API should be used before starting the timer.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_ulp_timer_init
 *      - \ref sl_si91x_ulp_timer_stop 
 * 
 * @param[in] timer_num Enum for ULP-timer number, see \ref ulp_timer_instance_t for possible values.
 * @param[in] timer_mode Enum for ULP-timer mode, see \ref ulp_timer_mode_t for possible values.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully set the timer mode.
 *         - SL_STATUS_INVALID_MODE  - The 'timer_mode' parameter value is invalid.
 *         - SL_STATUS_INVALID_INDEX  - The 'timer_num' parameter value is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_ulp_timer_set_mode(ulp_timer_instance_t timer_num, ulp_timer_mode_t timer_mode);

/***************************************************************************/
/**
 * @brief To set the ULP-Timer match value.
 * 
 * @details When the timer reaches the match value and if the interrupt is enabled, it
 *          will generate an interrupt to the processor.
 *          - For one microsecond type, match value = number of microseconds.
 *          - For 256 microsecond type, match value = (time in microseconds) / 256.
 *          - For normal up/down counter type, timer = (number of clock cycles per microsecond * time in microseconds).
 * 
 * @note The API should be used when the timer is stopped.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_ulp_timer_init
 *      - \ref sl_si91x_ulp_timer_stop
 *      - \ref sl_si91x_ulp_timer_set_type
 *      - \ref sl_si91x_ulp_timer_set_direction
 * 
 * @param[in] timer_num Enum for ULP-timer number, see \ref ulp_timer_instance_t for possible values.
 * @param[in] timer_match_value ULP-timer timeout value.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully set the timer match value.
 *         - SL_STATUS_INVALID_INDEX  - The 'timer_num' parameter value is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_ulp_timer_set_count(ulp_timer_instance_t timer_num, uint32_t timer_match_value);

/***************************************************************************/
/**
 * @brief To get the ULP-Timer current count.
 * 
 * @details If the counter is set to down-counting, the counter register bits indicate the time remaining until the timeout,
 *          initially read as 32'hFFFF_FFFF. If the counter is set to up-counting, these bits directly show the up-running counter/timer value.
 *          The timer current-count value is updated to the count-value variable provided by the user as the second argument.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_ulp_timer_init
 *      - \ref sl_si91x_ulp_timer_start
 *      - \ref sl_si91x_ulp_timer_set_count
 * 
 * @param[in] timer_num Enum for ULP-timer number, see \ref ulp_timer_instance_t for possible values.
 * @param[out] count_value Pointer to the variable which will store the current count of the timer.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully read the timer's current count value.
 *         - SL_STATUS_NULL_POINTER  - The pointer to the 'count_value' parameter is a null pointer.
 *         - SL_STATUS_INVALID_INDEX  - The 'timer_num' parameter value is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_ulp_timer_get_count(ulp_timer_instance_t timer_num, uint32_t *count_value);

/***************************************************************************/
/**
 * @brief To get the ULP-Timer type.
 * 
 * @details This API updates the timer type value to the timer_type variable, passed as
 *          reference by the user.
 *          - '0' for down-counter type
 *          - '1' for 1us type
 *          - '2' for 256us type
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_ulp_timer_init
 *      - \ref sl_si91x_ulp_timer_set_type
 * 
 * @param[in] timer_num Enum for ULP-timer number, see \ref ulp_timer_instance_t for possible values.
 * @param[out] timer_type Pointer to the variable which will store the current type of the timer.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully read the timer's current type.
 *         - SL_STATUS_NULL_POINTER  - The pointer to the 'timer_type' parameter is a null pointer.
 *         - SL_STATUS_INVALID_INDEX  - The 'timer_num' parameter value is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_ulp_timer_get_type(ulp_timer_instance_t timer_num, uint32_t *timer_type);

/***************************************************************************/
/**
 * @brief To get the ULP-Timer mode.
 * 
 * @details Updates the timer mode value to the timer_mode variable, passed as
 *          reference by the user.
 *          - '0' for one-shot mode
 *          - '1' for periodic mode
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_ulp_timer_configure_clock
 *      - \ref sl_si91x_ulp_timer_set_mode
 * 
 * @param[in] timer_num Enum for ULP-timer number, see \ref ulp_timer_instance_t for possible values.
 * @param[out] timer_mode Pointer to the variable which will store the current mode of the timer, see \ref ulp_timer_mode_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully read the timer's current mode.
 *         - SL_STATUS_NULL_POINTER  - The pointer to the 'timer_mode' parameter is a null pointer.
 *         - SL_STATUS_INVALID_INDEX  - The 'timer_num' parameter value is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_ulp_timer_get_mode(ulp_timer_instance_t timer_num, uint32_t *timer_mode);

/***************************************************************************/
/**
 * @brief To get the ULP-Timer direction (up-counter or down-counter).
 * 
 * @details Updates the timer direction to the timer_direction variable, passed as
 *          reference by the user.
 *          - '0' for down-counting
 *          - '1' for up-counting
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_ulp_timer_init
 *      - \ref sl_si91x_ulp_timer_set_direction
 * 
 * @param[in] timer_num Enum for ULP-timer number, see \ref ulp_timer_instance_t for possible values.
 * @param[out] timer_direction Pointer to the variable which will store the current direction of the timer, see \ref ulp_timer_direction_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully read the timer's current direction.
 *         - SL_STATUS_NULL_POINTER  - The pointer to 'timer_direction' parameter is a null pointer.
 *         - SL_STATUS_INVALID_INDEX  - The 'timer_num' parameter value is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_ulp_timer_get_direction(ulp_timer_instance_t timer_num, uint32_t *timer_direction);

/***************************************************************************/
/**
 * @brief To register the callback for the timer timeout interrupt and enable the interrupt.
 * 
 * @details This API registers the callback function for the timer timeout interrupt and enables the interrupt
 *          for the specified timer instance. If a callback is already registered, the user must first unregister
 *          the existing callback before registering a new one. Otherwise, it will return a SL_STATUS_BUSY error.
 *
 * @param[in] timer_num Enum for ULP-timer number, see \ref ulp_timer_instance_t for possible values.
 * @param[in] on_timeout_callback Function pointer to the callback function to be called when the timer timeout interrupt occurs.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully registered the timer timeout callback.
 *         - SL_STATUS_BUSY  - The callback is already registered, unregister the previous callback before registering a new one.
 *         - SL_STATUS_ALLOCATION_FAILED - Timer interrupt enabling failed.
 *         - SL_STATUS_NULL_POINTER  - The pointer to 'callback_data_input' parameter is a null pointer.
 *         - SL_STATUS_INVALID_INDEX  - The 'timer_num' parameter value is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_ulp_timer_register_timeout_callback(ulp_timer_instance_t timer_num,
                                                         ulp_timer_callback_t on_timeout_callback);

/***************************************************************************/
/**
 * @brief To unregister the callback for the timer interrupt and disable its interrupts.
 * 
 * @details This API is used to unregister the existing callback interrupt for the specified timer instance.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_ulp_timer_register_timeout_callback
 * 
 * @param[in] timer_num Enum for ULP-timer number, see \ref ulp_timer_instance_t for possible values.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully unregistered the timer timeout callback.
 *         - SL_STATUS_ALLOCATION_FAILED - Timer interrupt disabling failed.
 *         - SL_STATUS_INVALID_INDEX  - The 'timer_num' parameter value is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_ulp_timer_unregister_timeout_callback(ulp_timer_instance_t timer_num);

/***************************************************************************/
/**
 * @brief To configure and enable the SOC clock source for the timer.
 * 
 * @details This API is used to select the ULPSS processor clock source as the timer input clock 
 *          source, which is greater than 100MHz (used for high-power mode).
 *          If `div_factor_type` value is 1, the `div_factor` should be an odd number; otherwise, it will throw an error.
 *          The `div_factor_type` selects the type of divider for `m4_soc_clk_2ulpss`:
 *          - '0' Even Divider is selected
 *          - '1' Odd Divider is selected
 *          Use this API in place of the timer init API.
 * 
 * @param[in] div_factor Value to divide the clock. Ensure that it should be an odd number
 *                       if `div_factor_type` is 1, and vice versa.
 * @param[in] div_factor_type Selects the type of divider for `m4_soc_clk_2ulpss`:
 *                            - '0' Even Divider is selected
 *                            - '1' Odd Divider is selected
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The 'div_factor' is not valid according to `div_factor_type`.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_ulp_timer_configure_soc_clock(boolean_t div_factor_type, uint16_t div_factor);

/***************************************************************************/
/**
 * @brief To configure the XTAL clock when the clock source is an external XTAL clock.
 * 
 * @details This API enables the external XTAL clock source. It needs to be called before 
 *          calling the \ref sl_si91x_ulp_timer_configure_clock API.
 *
 * @param[in] xtal_pin Pin number of UULP_GPIO. Possible values are 0 to 4.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The 'xtal_pin' parameter value is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_ulp_timer_configure_xtal_clock(uint8_t xtal_pin);

/***************************************************************************/
/**
 * @brief To de-initialize the ULP-Timer clock by disabling the peripheral clock.
 * 
 * @details This API disables the peripheral clock for the ULP-Timer and unregisters the callback
 *          for all timer instances.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_ulp_timer_init
 *      - \ref sl_si91x_ulp_timer_unregister_timeout_callback
 * 
 * @note Unregisters the callbacks of all timer instances.
 *******************************************************************************/
void sl_si91x_ulp_timer_deinit(void);

/***************************************************************************/
/**
 * @brief To get the ULP-Timer API version.
 * 
 * @details This API is used to retrieve the version information of the ULP-Timer API.
 * 
 * @return sl_ulp_timer_version_t The version of the ULP-Timer API.
 *******************************************************************************/
sl_ulp_timer_version_t sl_si91x_ulp_timer_get_version(void);

/// @} end group ULP-TIMER ********************************************************/

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY! ***********************
/** @addtogroup ULP-TIMER ULP Timers
* @{
* @details
*
* @section ULP-TIMER_Intro Introduction
*
* Ultra-Low-Power (ULP) timers are typically used to operate with extremely low power consumption.
* These timers are used in battery-powered or energy-efficient devices, where minimizing
* power usage is essential for extending battery life.
*
* ULP timers offer configurable settings, allowing developers to adjust parameters such
* as clock frequency, timer resolution, and interrupt behavior to suit the specific needs of their applications.
* @li *Key features include:*
*  - The ULP-Timer module supports four different instances @ref ulp_timer_instance_t.
*  - Each ULP-timer can be independently programmed to operate in periodic or one-shot mode @ref ulp_timer_mode_t.
*  - Each ULP-timer can be independently configured as either a 32-bit counter or a microsecond timer @ref ulp_timer_type_t.
*
* @image html ulp_timer_block_diagram.png "ULP timer block diagram"
*
* @section ULP-TIMER_Config Configuration
* The configurable parameters for the ULP-Timer include:
*  1. The ULP-timer is initialized and its clocks are configured using @ref sl_si91x_ulp_timer_init.
*  2. Parameters such as match count, timer type, mode, and timer direction can be configured using @ref sl_si91x_ulp_timer_set_configuration.
*
* @li For more information on configuring available parameters, see the respective peripheral example readme document.
* 
* @section ULP-TIMER_Use Usage
* The common ULP-timer functions can be used after ULP-timer structures are specified,
* passing an instance of @ref ulp_timer_config_t.
* @li These functions will initiate and configure the ULP-timer below, which is the flow for implementation.
*     1. Initialize the ULP-Timer by enabling its clock source using the @ref sl_si91x_ulp_timer_init function.
*     2. Configure the ULP-Timer parameters using the @ref sl_si91x_ulp_timer_set_configuration function.
*     3. Register the callback for the timer timeout interrupt and enable the interrupt using the @ref sl_si91x_ulp_timer_register_timeout_callback function.
*     4. Start the ULP-Timer using the @ref sl_si91x_ulp_timer_start function.
*     5. Stop the ULP-Timer using the @ref sl_si91x_ulp_timer_stop function.
*     6. De-initialize the ULP-Timer clock by disabling the peripheral clock using the @ref sl_si91x_ulp_timer_deinit function.
*
* The above functions allow for initializing the ULP-TIMER instance and generating interrupts.
*
* @li Additional Timer Configuration functions for managing the ULP timer, you can use the following functions to adjust the timer's type, direction, and callback settings:
* 1. *Set the ULP-Timer type:* @ref sl_si91x_ulp_timer_set_type
* 2. *Set the ULP-Timer mode as one-shot or periodic:* @ref sl_si91x_ulp_timer_set_mode
* 3. *Set the ULP-Timer direction:* @ref sl_si91x_ulp_timer_set_direction
* 4. *Set the ULP-Timer match value:* @ref sl_si91x_ulp_timer_set_count
*
* @li To retrieve information about the current configuration and status of the ULP timer, you can use the following functions:
* 1. *Read the ULP-Timer mode:*  @ref sl_si91x_ulp_timer_get_mode
* 2. *Read the ULP-Timer type:*  @ref sl_si91x_ulp_timer_get_type
* 3. *Read the ULP-Timer direction (up-counter or down-counter):*  @ref sl_si91x_ulp_timer_get_direction
* 4. *Read the ULP-Timer current count:*  @ref sl_si91x_ulp_timer_get_count
*
* @} end group ULP-TIMER ********************************************************/

#ifdef __cplusplus
}
#endif

#endif /*SL_SI917_ULP_TIMER_*/
