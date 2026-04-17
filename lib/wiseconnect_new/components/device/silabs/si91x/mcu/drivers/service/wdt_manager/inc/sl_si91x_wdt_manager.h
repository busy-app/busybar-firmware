/************************************************************************************
 * @file sl_si91x_wdt_manager.h
 * @brief Watchdog Manager Service API implementation
 ************************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ************************************************************************************
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
 ************************************************************************************/

#ifndef SL_SI91X_WDT_MANAGER_
#define SL_SI91X_WDT_MANAGER_

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"
#include "rsi_wwdt.h"
#include "sl_si91x_watchdog_timer.h"

/***************************************************************************/
/**
 * @addtogroup  WDT-MANAGER Watchdog Manager
 * @ingroup SI91X_SERVICE_APIS
 * @{ 
 *
 ******************************************************************************/
// -----------------------------------------------------------------------------------
// GLOBAL DEFINES / MACROS
// -----------------------------------------------------------------------------------

#define SL_SI91X_WAKEUP_INDICATION       BIT(0)  ///< Wake-up event
#define SL_SI91X_TIMEOUT_WAKEUP          BIT(1)  ///< Timeout wake-up
#define SL_SI91X_HOST_BASED_WAKEUP_S     BIT(2)  ///< Host wake-up
#define SL_SI91X_MCU_PROCESSOR_WAKE_STAT BIT(4)  ///< MCU wake-up status
#define SL_SI91X_MCU_WWD_RESET           BIT(5)  ///< MCU watchdog reset
#define SL_SI91X_MCU_WWD_WINDOW_RESET    BIT(6)  ///< MCU watchdog window reset
#define SL_SI91X_NWP_WWD_RESET           BIT(8)  ///< Network processor watchdog reset
#define SL_SI91X_NWP_WWD_WINDOW_RESET    BIT(9)  ///< Network processor watchdog window reset
#define SL_SI91X_HOST_RESET_REQUEST      BIT(10) ///< Host reset request

/***************************************************************************/
/**
 * @def SL_SI91X_WDT_RESET
 * @brief Watchdog reset status flags.
 * @details This macro combines multiple watchdog reset status flags, indicating various watchdog reset sources:
 *          - MCU watchdog window reset
 *          - MCU watchdog reset
 *          - Network processor watchdog window reset
 *          - Network processor watchdog reset
 *          - Host reset request
 */
#define SL_SI91X_WDT_RESET                                                                                         \
  (SL_SI91X_MCU_WWD_WINDOW_RESET | SL_SI91X_MCU_WWD_RESET | SL_SI91X_NWP_WWD_WINDOW_RESET | SL_SI91X_NWP_WWD_RESET \
   | SL_SI91X_HOST_RESET_REQUEST)

// -----------------------------------------------------------------------------------
// DATA TYPES
// -----------------------------------------------------------------------------------
/***************************************************************************/
/**
 * @brief Reason for the system being reset.
 * 
 * @details This enumeration defines the possible reasons for the system being reset.
 *          It includes power-on reset, watchdog reset, and software-invoked reset.
 */
typedef enum {
  SL_SI91X_WATCHDOG_MANAGER_RESET_POR,      ///< A power-on reset was performed.
  SL_SI91X_WATCHDOG_MANAGER_RESET_WATCHDOG, ///< System was reset by the watchdog manager service.
  SL_SI91X_WATCHDOG_MANAGER_RESET_SOFTWARE, ///< A reset was invoked from the system firmware.
  SL_SI91X_WATCHDOG_MANAGER_RESET_MAX       ///< Maximum value for reset reasons.
} sl_watchdog_manager_reset_reason_t;

/***************************************************************************/
/**
 * @brief Enumeration to represent possible time delay values for Watchdog Timer (WDT) interrupt time and system reset time with 32 kHz clock frequency.
 * 
 * @details This enumeration defines the possible time delays for the WDT interrupt and system reset times.
 *          The delays are based on a 32 kHz clock frequency.
 */
typedef enum {
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_0,   ///< Time delay of 0.03125 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_1,   ///< Time delay of 0.0625 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_2,   ///< Time delay of 0.125 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_3,   ///< Time delay of 0.25 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_4,   ///< Time delay of 0.5 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_5,   ///< Time delay of 1 millisecond
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_6,   ///< Time delay of 2 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_7,   ///< Time delay of 4 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_8,   ///< Time delay of 8 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_9,   ///< Time delay of 16 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_10,  ///< Time delay of 32 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_11,  ///< Time delay of 64 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_12,  ///< Time delay of 125 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_13,  ///< Time delay of 250 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_14,  ///< Time delay of 500 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_15,  ///< Time delay of 1000 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_16,  ///< Time delay of 2000 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_17,  ///< Time delay of 4000 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_18,  ///< Time delay of 8000 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_19,  ///< Time delay of 16000 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_20,  ///< Time delay of 32000 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_21,  ///< Time delay of 64000 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_22,  ///< Time delay of 128000 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_23,  ///< Time delay of 256000 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_24,  ///< Time delay of 512000 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_25,  ///< Time delay of 10240000 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_26,  ///< Time delay of 2048000 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_27,  ///< Time delay of 4096000 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_28,  ///< Time delay of 8192000 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_29,  ///< Time delay of 16384000 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_30,  ///< Time delay of 32768000 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_31,  ///< Time delay of 65536000 milliseconds
  SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_MAX, ///< Maximum value for time delay validation
} sl_watchdog_manager_wdog_timeout_t;

// -----------------------------------------------------------------------------------
// GLOBAL FUNCTION PROTOTYPES
// -----------------------------------------------------------------------------------

/***************************************************************************/
/**
 * @brief To configure the watchdog manager service.
 * 
 * @details Configures the hardware watchdog timer.
 * - The 91x watchdog peripheral's interrupt interval is set to 2 seconds less than the configured value of `SL_WDT_MANAGER_INTERRUPT_TIME`.
 *   The hardware watchdog triggers an interrupt when this timeout expires, allowing the software to feed the watchdog within this interval.
 * 
 *  - Assigns a system reset value to 4 seconds. If the WDT is not reset within this interval, 
 *    the system will reset after 4 seconds. The reset counter begins counting after the interrupt time expires. 
 *    The system reset value is updated based on the configured interrupt time, referenced by `SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_PERIOD`.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success
 *         - SL_STATUS_FAIL  - Generic error
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_watchdog_manager_init(void);

/***************************************************************************/
/**
 * @brief To start the watchdog manager service.
 * 
 * @details This function starts the WDT and begins counting. 
 *          The hardware watchdog triggers an interrupt when the WDT timeout interval expires.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_watchdog_manager_init must be called before this function.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success
 *         - SL_STATUS_FAIL  - Generic error
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_watchdog_manager_start(void);

/***************************************************************************/
/**
 * @brief To retrieve the system reset reason.
 * 
 * @details Returns the reason for the system being reset. This may be invoked during a startup flow
 *          to determine if the system has recovered from an error condition such as a crash.
 * 
 * @return sl_watchdog_manager_reset_reason_t Enumeration value indicating the reset reason:
 *         - \ref SL_SI91X_WATCHDOG_MANAGER_RESET_WATCHDOG - WDT System Reset
 *         - \ref SL_SI91X_WATCHDOG_MANAGER_RESET_SOFTWARE - Software System Reset
 *         - \ref SL_SI91X_WATCHDOG_MANAGER_RESET_POR - Power-on Reset
 ******************************************************************************/
sl_watchdog_manager_reset_reason_t sl_wdt_manager_get_system_reset_status(void);

/***************************************************************************/
/**
 * @brief To disable the hardware watchdog in sleep mode.
 * 
 * @details This function stops the WDT counter from running when the system enters sleep mode.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_watchdog_manager_init must be called before this function.
 * 
 * @note This function does not return a status code.
 ******************************************************************************/
void sl_watchdog_disable_wdt_in_sleep(void);

/***************************************************************************/
/**
 * @brief To enable the hardware watchdog in sleep mode.
 * 
 * @details This function starts the WDT counter running in sleep mode.
 *          It enables the WDT as a wake-up source to the system to serve the ISR and kick the watchdog.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_watchdog_manager_init must be called before this function.
 ******************************************************************************/
void sl_watchdog_enable_wdt_in_sleep(void);

/// @} end addtogroup WDT-MANAGER ******************************************************/

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY! ***********************
/***************************************************************************/
/***************************************************************************/
/** 
 * @addtogroup WDT-MANAGER
 * @{
 *
 * @details
 *
 * @section WDT_Manager_Intro Introduction
 *
 * The Si91x SOC provides a robust mechanism to handle state changes and recovery operations. In standalone mode, it can notify applications running on the M4 core about different state changes, which helps ensure system stability and recovery from faults.
 *
 * The different boot states managed by the SOC include:
 * - **Cold Boot**: A standard boot from a powered-off state.
 * - **Power Save Boot**: Resuming from a low-power state, typically retention-based.
 * - **Crash Boot**: A reboot triggered by internal events such as hangs, crashes, watchdog triggers, or planned reboots to prevent known issues.
 *
 * This feature helps detect processor hangs and ensures recovery through system resets, enhancing the overall reliability of the SOC.
 *
 * @section WDT_Manager_Config Configuration
 *
 * Configuration involves setting up mechanisms to handle various boot states and utilizing the internal watchdog and reset features to manage unexpected faults or hangs.
 * - Ensure that all boot states are properly monitored.
 * - Use watchdog timers to detect hangs or crashes.
 * - Implement firmware routines to handle planned reboots and crash scenarios effectively.
 *
 * @section WDT_Manager_Usage Usage
 *
 * The typical sequence to utilize WDT-MANAGER effectively includes:
 *
 * 1. Initialize the watchdog and monitoring mechanisms at startup.
 * 2. Configure the system to handle different boot states:
 *    - Cold Boot
 *    - Power Save Boot
 *    - Crash Boot
 * 3. Implement interrupt handling for detecting system hangs or crashes.
 * 4. Trigger system reset in case of detected faults.
 * 5. Log state changes and recovery actions for diagnostics.
 *
 * @section WDT_Manager_Benefits Benefits
 *
 * - Detects and recovers from processor hangs.
 * - Provides mechanisms to handle planned and unplanned reboots.
 * - Ensures system reliability and stability by managing state transitions effectively.
 * 
 * @note When the RC clock is used as the source for the LF-FSM, the timeout durations may vary due to the inherent frequency instability of the RC oscillator.
 * 
 * @} (end addtogroup WDT-MANAGER)
 *******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /*SL_SI91X_WDT_MANAGER*/
