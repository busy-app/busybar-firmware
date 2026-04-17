/******************************************************************************
* @file  sl_si91x_watchdog_timer.h
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

#ifndef SL_SI91X_WATCHDOG_TIMER_
#define SL_SI91X_WATCHDOG_TIMER_

#ifdef __cplusplus
extern "C" {
#endif

//// Includes
#include "sl_status.h"
#include "rsi_power_save.h"
#include "rsi_wwdt.h"
#include "base_types.h"

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
#ifndef UNUSED_VARIABLE
#define UNUSED_VARIABLE(x) (void)(x)
#endif // UNUSED_VARIABLE
/** @endcond */

/***************************************************************************/
/**
 * @addtogroup WATCHDOG-TIMER Watchdog Timer
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Macros for watchdog-timer parameters

// -----------------------------------------------------------------------------
// Data Types
/***************************************************************************/ /**
 * Typedef for the function pointer of the callback function
 ******************************************************************************/
typedef void (*watchdog_timer_callback_t)(void);
typedef FSM_CLK_T high_freq_fsm_clock_t; ///< Renaming high frequency fsm-clock type enum
typedef AON_CLK_T low_freq_fsm_clock_t;  ///< Renaming low frequency fsm-clock type enum

/***************************************************************************/
/**
 * @brief Enumeration to represent possible time delay values for WDT interrupt time and system reset time with a 32.768 KHz clock frequency.
 * 
 * @details This enumeration defines the possible time delay values for the Watchdog Timer (WDT) interrupt and system reset times when using a 32.768 KHz clock frequency.
 * 
 */
typedef enum {
  TIME_DELAY_0,    ///< Time delay for 1 clock pulse
  TIME_DELAY_1,    ///< Time delay for 2 clock pulses
  TIME_DELAY_2,    ///< Time delay for 4 clock pulses
  TIME_DELAY_3,    ///< Time delay for 8 clock pulses
  TIME_DELAY_4,    ///< Time delay for 16 clock pulses
  TIME_DELAY_5,    ///< Time delay for 32 clock pulses
  TIME_DELAY_6,    ///< Time delay for 64 clock pulses
  TIME_DELAY_7,    ///< Time delay for 128 clock pulses
  TIME_DELAY_8,    ///< Time delay for 256 clock pulses
  TIME_DELAY_9,    ///< Time delay for 512 clock pulses
  TIME_DELAY_10,   ///< Time delay for 1024 clock pulses
  TIME_DELAY_11,   ///< Time delay for 2048 clock pulses
  TIME_DELAY_12,   ///< Time delay for 4096 clock pulses
  TIME_DELAY_13,   ///< Time delay for 8192 clock pulses
  TIME_DELAY_14,   ///< Time delay for 16384 clock pulses
  TIME_DELAY_15,   ///< Time delay for 32768 clock pulses
  TIME_DELAY_16,   ///< Time delay for 65536 clock pulses
  TIME_DELAY_17,   ///< Time delay for 131072 clock pulses
  TIME_DELAY_18,   ///< Time delay for 262144 clock pulses
  TIME_DELAY_19,   ///< Time delay for 524288 clock pulses
  TIME_DELAY_20,   ///< Time delay for 1048576 clock pulses
  TIME_DELAY_21,   ///< Time delay for 2097152 clock pulses
  TIME_DELAY_22,   ///< Time delay for 4194304 clock pulses
  TIME_DELAY_23,   ///< Time delay for 8388608 clock pulses
  TIME_DELAY_24,   ///< Time delay for 16777216 clock pulses
  TIME_DELAY_25,   ///< Time delay for 33554432 clock pulses
  TIME_DELAY_26,   ///< Time delay for 67108864 clock pulses
  TIME_DELAY_27,   ///< Time delay for 134217728 clock pulses
  TIME_DELAY_28,   ///< Time delay for 268435456 clock pulses
  TIME_DELAY_29,   ///< Time delay for 536870912 clock pulses
  TIME_DELAY_30,   ///< Time delay for 1073741824 clock pulses
  TIME_DELAY_31,   ///< Time delay for 2147483648 clock pulses
  TIME_DELAY_LAST, ///< Time delay value validation
} time_delays_t;

/***************************************************************************/
/**
 * @brief Structure to hold the parameters for Watchdog Timer clock source configurations.
 * 
 * @details This structure contains the configuration parameters for the Watchdog Timer's clock sources, including low frequency FSM clock source, high frequency FSM clock source, and BG-PMU clock source.
 */
typedef struct {
  uint8_t low_freq_fsm_clock_src;  ///< Low frequency FSM clock source, see \ref low_freq_fsm_clock_t
  uint8_t high_freq_fsm_clock_src; ///< High frequency FSM clock source, see \ref high_freq_fsm_clock_t
  uint8_t bg_pmu_clock_source;     ///< BG-PMU clock source
} watchdog_timer_clock_config_t;

/***************************************************************************/
/**
 * @brief Structure to hold the parameters for the Watchdog Timer configurations.
 * 
 * @details This structure contains the configuration parameters for the Watchdog Timer, including the interrupt time, system reset time, and window time. Each parameter is represented as the number of clock pulses, calculated as 2^(parameter value).
 */
typedef struct {
  uint8_t
    interrupt_time; ///< Timer timeout interrupt duration, number of clock pulses = 2^(interrupt_time), see \ref time_delays_t
  uint8_t
    system_reset_time; ///< Timer system reset duration, number of clock pulses = 2^(system_reset_time), see \ref time_delays_t
  uint8_t
    window_time; ///< Timer window duration, number of clock pulses = 2^(window_time), see \ref time_delays_t. Should be less than TIME_DELAY_16
} watchdog_timer_config_t;

/***************************************************************************/
/**
 * @brief Structure to hold the version information of the Watchdog Timer peripheral API.
 * 
 * @details This structure contains the version information of the Watchdog Timer peripheral API, including the release, major, and minor version numbers.
 */
typedef struct {
  uint8_t release; ///< Release version number
  uint8_t major;   ///< Major version number
  uint8_t minor;   ///< Minor version number
} sl_watchdog_timer_version_t;

// -----------------------------------------------------------------------------
// Prototypes

/***************************************************************************/
/**
 * @brief To initialize the Watchdog Timer.
 * 
 * @details This API initializes the Watchdog Timer by enabling the peripheral power. 
 *          It also unmasks the WDT interrupt.
 * 
 * @note This function should be called before using any other Watchdog Timer functions.
 *******************************************************************************/
void sl_si91x_watchdog_init_timer(void);

/***************************************************************************/
/**
 * @brief This API is no longer supported due to the restriction on peripheral drivers to configuring clocks.
 * 
 * @details This API configures the Watchdog Timer's low frequency and BG-PMU clock sources.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_watchdog_init_timer
 * 
 * @param[in] timer_clk_config_ptr Pointer to the timer clock configuration structure.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK -  return ok to support backward compatibility.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_watchdog_configure_clock(watchdog_timer_clock_config_t *timer_clk_config_ptr)
  SL_DEPRECATED_API_WISECONNECT_4_0;

/***************************************************************************/
/**
 * @brief To configure Watchdog Timer parameters.
 * 
 * @details This API configures the Watchdog Timer parameters, including interrupt time (WDT restart time), 
 *          system reset time, and window time (lower time stamp for WDT restart, if necessary). 
 *          The interrupt time should be greater than the window time; otherwise, the API will return an error.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_watchdog_init_timer 
 * 
 * @param[in] timer_config_ptr Pointer to the timer configuration structure \ref watchdog_timer_config_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, timer parameters configured properly.
 *         - SL_STATUS_INVALID_PARAMETER  - Timer configuration structure members have invalid values.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 *         - SL_STATUS_INVALID_CONFIGURATION  - Timer configuration structure member 'interrupt_time' is less than or equal to 'window_time'. It should be greater than the window time of the timer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_watchdog_set_configuration(watchdog_timer_config_t *timer_config_ptr);

/***************************************************************************/
/**
 * @brief To register the Watchdog Timer timeout callback.
 * 
 * @details This API registers a callback function to be called when the Watchdog Timer timeout interrupt occurs.
 *          Registering the callback also enables its interrupts. Before registering a new callback, the user must 
 *          unregister any previously registered callback using the \ref sl_si91x_watchdog_unregister_timeout_callback function.
 * 
 * @param[in] on_timeout_callback Function pointer to the callback function to be called when the timer timeout interrupt occurs.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully registered the timer timeout callback.
 *         - SL_STATUS_BUSY  - The callback is already registered; the user must unregister the previous callback before registering again.
 *         - SL_STATUS_NULL_POINTER  - The callback function pointer parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_watchdog_register_timeout_callback(watchdog_timer_callback_t on_timeout_callback);

/***************************************************************************/
/**
 * @brief To set the Watchdog Timer interrupt time (Overflow or Restart time).
 * 
 * @details This API sets the Watchdog Timer interrupt time, also referred to as the upper time-stamp for WDT restart.
 *          The maximum value for the interrupt time is 31.
 *          - Number of clock pulses for timer timeout = 2^(interrupt_time)
 * 
 * @note The 'interrupt_time' value should be greater than the window time.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_watchdog_init_timer
 *      - \ref sl_si91x_watchdog_set_system_reset_time
 * 
 * @param[in] interrupt_time Timer timeout interrupt duration, represented as \ref time_delays_t.
 *                           Number of clock pulses = 2^(interrupt_time).
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully set the Watchdog Timer timeout value.
 *         - SL_STATUS_INVALID_PARAMETER  - The 'interrupt_time' parameter has an invalid value.
 *         - SL_STATUS_INVALID_CONFIGURATION  - The 'interrupt_time' value is less than the window time.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_watchdog_set_interrupt_time(time_delays_t interrupt_time);

/***************************************************************************/
/**
 * @brief To read the configured Watchdog Timer interrupt time.
 * 
 * @details This API returns the interrupt time configured for the Watchdog Timer in terms of the power of two.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_watchdog_set_interrupt_time
 * 
 * @return uint8_t The configured interrupt time value, see \ref time_delays_t.
 *******************************************************************************/
uint8_t sl_si91x_watchdog_get_interrupt_time(void);

/***************************************************************************/
/**
 * @brief To set the Watchdog Timer system reset time duration.
 * 
 * @details This API sets the duration for the Watchdog Timer system reset. The number of clock pulses for the system reset time is calculated as 2^(system_reset_time). The maximum value for system_reset_time is 31. This value should be greater than the timer window time.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_watchdog_init_timer
 * 
 * @param[in] system_reset_time Timer system reset duration, represented as \ref time_delays_t.
 *                              Number of clock pulses = 2^(system_reset_time).
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully set the Watchdog Timer system reset time value.
 *         - SL_STATUS_INVALID_PARAMETER  - The 'system_reset_time' parameter has an invalid value.
 *         - SL_STATUS_INVALID_CONFIGURATION  - The 'system_reset_time' value is less than the window time.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_watchdog_set_system_reset_time(time_delays_t system_reset_time);

/***************************************************************************/
/**
 * @brief To read the configured Watchdog Timer system reset time.
 * 
 * @details This API returns the system reset time configured for the Watchdog Timer in terms of the power of two.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_watchdog_set_system_reset_time
 * 
 * @return uint8_t The configured system reset time value, see \ref time_delays_t.
 *******************************************************************************/
uint8_t sl_si91x_watchdog_get_system_reset_time(void);

/***************************************************************************/
/**
 * @brief To set the Watchdog Timer window time value.
 * 
 * @details This API sets the window time value for the Watchdog Timer. By setting this value, the timer becomes a window Watchdog Timer. If this value is zero, it acts as a regular WDT. It is also referred to as the lower time-stamp for WDT restart. The maximum value is 15.
 *          - Number of clock pulses for window time = 2^(window_time).
 *          - This value should be less than the timer interrupt time and system reset time.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_watchdog_init_timer 
 *      - \ref sl_si91x_watchdog_set_system_reset_time 
 *      - \ref sl_si91x_watchdog_set_interrupt_time 
 * 
 * @param[in] window_time Timer window time, represented as \ref time_delays_t.
 *                        Number of clock pulses = 2^(window_time).
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully set the Watchdog Timer window time value.
 *         - SL_STATUS_INVALID_PARAMETER  - The 'window_time' parameter has an invalid value.
 *         - SL_STATUS_INVALID_CONFIGURATION  - The 'window_time' value is greater than the interrupt time or system reset time.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_watchdog_set_window_time(time_delays_t window_time);

/***************************************************************************/
/**
 * @brief To read the configured Watchdog Timer window time.
 * 
 * @details This API returns the set window time value (not the current count) in terms of the power of two.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_watchdog_set_window_time
 * 
 * @return uint8_t The configured window time value, see \ref time_delays_t.
 *******************************************************************************/
uint8_t sl_si91x_watchdog_get_window_time(void);

/***************************************************************************/
/**
 * @brief To read the Watchdog Timer system reset status.
 * 
 * @details This API returns the system reset status of the Watchdog Timer. It returns true if the system was reset by the Watchdog Timer, and false if it was a power-on reset.
 * 
 * @return boolean_t Returns true if the Watchdog Timer reset the system, otherwise returns false.
 *******************************************************************************/
boolean_t sl_si91x_watchdog_get_timer_system_reset_status(void);

/***************************************************************************/
/**
 * @brief To de-initialize the Watchdog Timer.
 * 
 * @details This API de-initializes the Watchdog Timer by masking interrupts, unregistering the callback, and disabling the timer.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_watchdog_init_timer 
 *      - \ref sl_si91x_watchdog_unregister_timeout_callback
 *******************************************************************************/
void sl_si91x_watchdog_deinit_timer(void);

/***************************************************************************/
/**
 * @brief To de-initialize the Watchdog Timer.
 * 
 * @details This API de-initializes the Watchdog Timer by masking interrupts, unregistering the callback, and disabling the timer.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_watchdog_init_timer 
 *      - \ref sl_si91x_watchdog_unregister_timeout_callback
 *******************************************************************************/
void sl_si91x_watchdog_deinit_timer(void);

/***************************************************************************/
/**
 * @brief To unregister the Watchdog Timer timeout callback.
 * 
 * @details This API unregisters the callback function for the Watchdog Timer timeout interrupt.
 *          It clears the callback function address and sets the callback variable to NULL.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_watchdog_register_timeout_callback
 *******************************************************************************/
void sl_si91x_watchdog_unregister_timeout_callback(void);

/***************************************************************************/
/**
 * @brief To get the API version of the Watchdog timer.
 * 
 * @details This API retrieves the version information of the Watchdog timer.
 * 
 * @return sl_watchdog_timer_version_t Structure containing the version information.
 ******************************************************************************/
sl_watchdog_timer_version_t sl_si91x_watchdog_get_version(void);

/***************************************************************************/
/**
 * @brief To start and enable the Watchdog Timer.
 * 
 * @details Once the Watchdog Timer (WDT) is started, the interrupt time will begin incrementing. 
 *          When it reaches the set interrupt time value, the WDT will generate an interrupt, and the timer 
 *          will load the system reset time value. Before it reaches this value, the WDT should be restarted 
 *          (kicked) to avoid a system reset.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_watchdog_init_timer 
 *      - \ref sl_si91x_watchdog_register_timeout_callback 
 *******************************************************************************/
__STATIC_INLINE void sl_si91x_watchdog_start_timer(void)
{
  RSI_WWDT_Start(MCU_WDT);
}

/***************************************************************************/
/**
 * @brief To stop the Watchdog Timer by disabling it.
 * 
 * @details This API stops an already running Watchdog Timer. It disables the timer by updating the 'WWD_TIMER_ENABLE' register bits.
 *          Specifically, it updates the 'wwd_timer_en' (23:16) bits of the timer enable register to 0xF0, thereby disabling the WDT timer.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_watchdog_start_timer
 *******************************************************************************/
__STATIC_INLINE void sl_si91x_watchdog_stop_timer(void)
{
  RSI_WWDT_Disable(MCU_WDT);
}

/*******************************************************************************/
/**
 * @brief To restart (kick) an already running Watchdog Timer.
 * 
 * @details This API sets the restart bit of the Watchdog Timer when it is already enabled.
 *          It should be used for an already started WDT timer, not for starting a stopped timer.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_watchdog_init_timer 
 *      - \ref sl_si91x_watchdog_start_timer
 *******************************************************************************/
__STATIC_INLINE void sl_si91x_watchdog_restart_timer(void)
{
  RSI_WWDT_ReStart(MCU_WDT);
}

/***************************************************************************/
/**
 * @brief To enable the Watchdog Timer to reset the system on processor lockup.
 * 
 * @details This API enables the Watchdog Timer (WDT) to generate a system reset when the processor gets stuck or reaches a lockup state. 
 *          It achieves this by setting the 'PROCESSOR_STUCK_RESET_EN' bit of the 'WWD_ARM_STUCK_EN' register of the WDT.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_watchdog_init_timer
 *******************************************************************************/
__STATIC_INLINE void sl_si91x_watchdog_enable_system_reset_on_processor_lockup(void)
{
  RSI_WWDT_SysRstOnProcLockEnable(MCU_WDT);
}

/***************************************************************************/
/**
 * @brief To disable the Watchdog Timer from resetting the system on processor lockup.
 * 
 * @details This API disables the Watchdog Timer (WDT) from generating a system reset when the processor gets stuck or reaches a lockup state. 
 *          It achieves this by writing '0' to the 'PROCESSOR_STUCK_RESET_EN' bit of the 'WWD_ARM_STUCK_EN' register of the WDT.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_watchdog_init_timer 
 *      - \ref sl_si91x_watchdog_enable_system_reset_on_processor_lockup
 *******************************************************************************/
__STATIC_INLINE void sl_si91x_watchdog_disable_system_reset_on_processor_lockup(void)
{
  RSI_WWDT_SysRstOnProcLockDisable(MCU_WDT);
}

/** @} end group WATCHDOG-TIMER */

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY! ***********************
/** @addtogroup WATCHDOG-TIMER Watchdog Timer
* @{
* 
* @details
* 
* @section WATCHDOG-TIMER_Intro Introduction
* 
* The Watchdog Timer is used to generate an interrupt upon a timeout and to perform a reset in the event of a system failure, which 
* may result from an external event such as an Electrostatic Discharge (ESD) pulse or a software malfunction.
* The interrupt can also be used as a wakeup source for transitioning from SLEEP/STANDBY to ACTIVE states.
* 
* **Features supported by the Watchdog Timer**
* @li  Independent window watchdog timer.
* @li  Interrupt is generated before the system reset is applied, which can be used as a wakeup source.
* @li  Individually controllable power domain for low-power applications.
* @li  Able to operate when the CPU is in the SLEEP state during power-save applications.
* @li  Configurable timeout period.
* @li  Option to be clocked using either a low-power internal RC (32 KHz) or an external 32KHz XTAL.
* 
* @section WATCHDOG-TIMER_Config Configuration
* 
* The Watchdog Timer can be configured using several parameters: 
* 1. *Initialize the Watchdog Timer:* @ref sl_si91x_watchdog_init_timer
* 2. *Set Watchdog Parameters:* Configure interrupt time, system reset time, and window time using @ref sl_si91x_watchdog_set_configuration
* 
* @li For more information on configuring available parameters, see the respective peripheral example readme document.
* 
* @section WATCHDOG-TIMER_Use Usage
* 
* @li The following functions will initiate and configure the Watchdog Timer, representing the general flow for implementation:
* 1. *Initialize the Watchdog Timer:* @ref sl_si91x_watchdog_init_timer
* 2. *System reset status:* @ref sl_si91x_watchdog_get_timer_system_reset_status (Optional, if required to check whether it is a power-on reset or WDT system reset.)
* 3. *Configure the Watchdog Timer:* @ref sl_si91x_watchdog_set_configuration
* 4. *Register Watchdog Timer timeout callback:* @ref sl_si91x_watchdog_register_timeout_callback
* 5. *Start Watchdog Timer:* @ref sl_si91x_watchdog_start_timer
* 6. *Restart Watchdog Timer (To kick the Watchdog Timer):* @ref sl_si91x_watchdog_restart_timer
* 7. *Stop Watchdog Timer:* @ref sl_si91x_watchdog_stop_timer
* 8. *De-initialize Watchdog Timer:* @ref sl_si91x_watchdog_deinit_timer
* 
* @li To read time values, use the following APIs:
* 
* 1. *Read System-Reset Time:* @ref sl_si91x_watchdog_get_system_reset_time
* 2. *Read Interrupt Time:* @ref sl_si91x_watchdog_get_interrupt_time 
* 3. *Read Window Time:* @ref sl_si91x_watchdog_get_window_time 
*
*
* @}(end addtogroup Watchdog-Timer) */

#ifdef __cplusplus
}
#endif

#endif /*SL_SI91X_WATCHDOG_TIMER_*/
