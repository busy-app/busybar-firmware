/***************************************************************************/ /**
 * @file sl_si91x_power_manager.h
 * @brief Power Manager Service API implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SI91X_POWER_MANAGER_H
#define SL_SI91X_POWER_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

// Current module name for debugging features
#ifndef CURRENT_MODULE_NAME
#define CURRENT_MODULE_NAME "Anonymous" ///< current module name
#endif

#include "sl_status.h"
#include "sl_slist.h"
#include "system_si91x.h"
#include "base_types.h"
#include "rsi_power_save.h"

#ifdef SL_SI91X_POWER_MANAGER_DEBUG_COMPONENT
#include "sl_si91x_power_manager_debug_config.h"
#endif

/***************************************************************************/
/**
 * @addtogroup POWER-MANAGER Power Manager
 * @ingroup SI91X_SERVICE_APIS
 * @{
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Defines

// Flags are used to subscribe to the Power Manager transition events, that is, provides callback after state transitions.
// This can be used individually or can be ored and pass to the \ref sl_si91x_power_manager_subscribe_ps_transition_event.

#define SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS4 \
  (1 << 0) ///< sl power manager event transition for entering PS4 state.
#define SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS4 \
  (1 << 1) ///< sl power manager event transition for leaving PS4 state.
#define SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS3 \
  (1 << 2) ///< sl power manager event transition for entering PS3 state.
#define SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS3 \
  (1 << 3) ///< sl power manager event transition for leaving PS3 state.
#define SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS2 \
  (1 << 4) ///< sl power manager event transition for entering PS2 state.
#define SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS2 \
  (1 << 5) ///< sl power manager event transition for leaving PS2 state.
#define SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS1 \
  (1 << 6) ///< sl power manager event transition for leaving PS1 state.
#define SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_SLEEP \
  (1 << 7) ///< sl power manager event transition for leaving sleep state.
#define SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_STANDBY \
  (1 << 8) ///< Event transition for leaving standby state.

//  Wakeup sources
#define SL_SI91X_POWER_MANAGER_DST_WAKEUP        DST_BASED_WAKEUP      ///< Deep Sleep Timer based wakeup source.
#define SL_SI91X_POWER_MANAGER_WIRELESS_WAKEUP   WIRELESS_BASED_WAKEUP ///< Wireless based wakeup source.
#define SL_SI91X_POWER_MANAGER_GPIO_WAKEUP       GPIO_BASED_WAKEUP     ///< GPIO based wakeup source.
#define SL_SI91X_POWER_MANAGER_COMPARATOR_WAKEUP COMPR_BASED_WAKEUP    ///< Comparator based wakeup source.
#define SL_SI91X_POWER_MANAGER_SYSRTC_WAKEUP     SYSRTC_BASED_WAKEUP   ///< Sysrtc based wakeup source.
#define SL_SI91X_POWER_MANAGER_ULPSS_WAKEUP      ULPSS_BASED_WAKEUP    ///< ULP peripheral based wakeup source.
#define SL_SI91X_POWER_MANAGER_SDCSS_WAKEUP      SDCSS_BASED_WAKEUP ///< SDC (Sensor data collector) based wakeup source.
#define SL_SI91X_POWER_MANAGER_ALARM_WAKEUP      ALARM_BASED_WAKEUP    ///< Alarm based wakeup source.
#define SL_SI91X_POWER_MANAGER_SEC_WAKEUP        SEC_BASED_WAKEUP      ///< Second based wakeup source.
#define SL_SI91X_POWER_MANAGER_MSEC_WAKEUP       MSEC_BASED_WAKEUP     ///< Milli second based wakeup source.
#define SL_SI91X_POWER_MANAGER_WDT_WAKEUP        WDT_INTR_BASED_WAKEUP ///< Watchdog interrupt based wakeup source.

// M4SS peripherals.
#define SL_SI91X_POWER_MANAGER_M4SS_PG_EFUSE        M4SS_PWRGATE_ULP_EFUSE_PERI   ///< M4SS EFUSE Power Gate.
#define SL_SI91X_POWER_MANAGER_M4SS_PG_RPDMA        M4SS_PWRGATE_ULP_RPDMA        ///< M4SS RPDMA Power Gate.
#define SL_SI91X_POWER_MANAGER_M4SS_PG_SDIO_SPI     M4SS_PWRGATE_ULP_SDIO_SPI     ///< M4SS SDIO SPI Power Gate.
#define SL_SI91X_POWER_MANAGER_M4SS_PG_QSPI         M4SS_PWRGATE_ULP_QSPI_ICACHE  ///< M4SS QSPI and ICACHE Power Gate.
#define SL_SI91X_POWER_MANAGER_M4SS_PG_IID          M4SS_PWRGATE_ULP_IID          ///< M4SS IID Power Gate.
#define SL_SI91X_POWER_MANAGER_M4SS_PG_M4_DEBUG     M4SS_PWRGATE_ULP_M4_DEBUG_FPU ///< M4SS M4 Debug Power Gate.
#define SL_SI91X_POWER_MANAGER_M4SS_PG_M4_CORE      M4SS_PWRGATE_ULP_M4_CORE      ///< M4SS M4 Core Power Gate.
#define SL_SI91X_POWER_MANAGER_M4SS_PG_EXTERNAL_ROM M4SS_PWRGATE_ULP_EXT_ROM      ///< M4SS External ROM Power Gate.

// ULPSS peripherals.
#define SL_SI91X_POWER_MANAGER_ULPSS_PG_MISC ULPSS_PWRGATE_ULP_MISC ///< ULP Miscellaneous Power Gate.
#define SL_SI91X_POWER_MANAGER_ULPSS_PG_CAP \
  ULPSS_PWRGATE_ULP_CAP ///< DEPRECATED - ULP Capacitive Touch Sensor Power Gate.
#define SL_SI91X_POWER_MANAGER_ULPSS_PG_UART ULPSS_PWRGATE_ULP_UART ///< ULP UART Power Gate.
#define SL_SI91X_POWER_MANAGER_ULPSS_PG_SSI  ULPSS_PWRGATE_ULP_SSI  ///< ULP SSI Power Gate.
#define SL_SI91X_POWER_MANAGER_ULPSS_PG_I2S  ULPSS_PWRGATE_ULP_I2S  ///< ULP I2S Power Gate.
#define SL_SI91X_POWER_MANAGER_ULPSS_PG_I2C  ULPSS_PWRGATE_ULP_I2C  ///< ULP I2C Power Gate.
#define SL_SI91X_POWER_MANAGER_ULPSS_PG_AUX  ULPSS_PWRGATE_ULP_AUX  ///< ULP AUX Power Gate.
#define SL_SI91X_POWER_MANAGER_ULPSS_PG_IR   ULPSS_PWRGATE_ULP_IR   ///< ULP IR Power Gate.
#define SL_SI91X_POWER_MANAGER_ULPSS_PG_UDMA ULPSS_PWRGATE_ULP_UDMA ///< ULP UDMA Power Gate.
#define SL_SI91X_POWER_MANAGER_ULPSS_PG_FIM  ULPSS_PWRGATE_ULP_FIM  ///< ULP FIM Power Gate.

// NPSS peripherals.
#define SL_SI91X_POWER_MANAGER_NPSS_PG_MCUBFFS    SLPSS_PWRGATE_ULP_MCUBFFS ///< NPSS MCU BFFS (Battery FF's) Power Gate.
#define SL_SI91X_POWER_MANAGER_NPSS_PG_MCUFSM     SLPSS_PWRGATE_ULP_MCUFSM  ///< NPSS MCU FSM Power Gate.
#define SL_SI91X_POWER_MANAGER_NPSS_PG_MCURTC     SLPSS_PWRGATE_ULP_MCURTC ///< NPSS MCU RTC (Real Time Clock) Power Gate.
#define SL_SI91X_POWER_MANAGER_NPSS_PG_MCUWDT     SLPSS_PWRGATE_ULP_MCUWDT ///< NPSS MCU WDT (Watchdog Timer) Power Gate.
#define SL_SI91X_POWER_MANAGER_NPSS_PG_MCUPS      SLPSS_PWRGATE_ULP_MCUPS  ///< NPSS MCU Process Sensor Power Gate.
#define SL_SI91X_POWER_MANAGER_NPSS_PG_MCUTS      SLPSS_PWRGATE_ULP_MCUTS  ///< NPSS MCU Temperature Sensor Power Gate.
#define SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE1  SLPSS_PWRGATE_ULP_MCUSTORE1  ///< NPSS MCU Storage 1 Power Gate.
#define SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE2  SLPSS_PWRGATE_ULP_MCUSTORE2  ///< NPSS MCU Storage 2 Power Gate.
#define SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE3  SLPSS_PWRGATE_ULP_MCUSTORE3  ///< NPSS MCU Storage 3 Power Gate.
#define SL_SI91X_POWER_MANAGER_NPSS_PG_TIMEPERIOD SLPSS_PWRGATE_ULP_TIMEPERIOD ///< NPSS Time Period Power Gate.
#define SL_SI91X_POWER_MANAGER_NPSS_PG_NWPAPB_MCU_CTRL \
  SLPSS_PWRGATE_ULP_NWPAPB_MCU_CTRL ///< NPSS MCU APB Control Power Gate.

// RAM Bank numbers.
#define SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_1  RAM_BANK_0      ///< 4 KB (Bank 1 of first 192k chunk)
#define SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_2  RAM_BANK_1      ///< 4 KB (Bank 2 of first 192k chunk)
#define SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_3  RAM_BANK_2      ///< 4 KB (Bank 3 of first 192k chunk)
#define SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_4  RAM_BANK_3      ///< 4 KB (Bank 4 of first 192k chunk)
#define SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_5  RAM_BANK_4      ///< 4 KB (Bank 5 of first 192k chunk)
#define SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_6  RAM_BANK_5      ///< 32 KB (Bank 6-7 of first 192k chunk)
#define SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_7  RAM_BANK_6      ///< 64 KB (Bank 9-11 of first 192k chunk)
#define SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_8  RAM_BANK_7      ///< 64 KB (Bank 12-15 of first 192k chunk)
#define SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_9  RAM_BANK_8      ///< 64 KB (Bank 1-4 of second 192k chunk)
#define SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_10 RAM_BANK_9      ///< 64 KB (Bank 1-4 of third 192k chunk)
#define SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_1 ULPSS_2K_BANK_0 ///< 2 KB
#define SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_2 ULPSS_2K_BANK_1 ///< 2 KB
#define SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_3 ULPSS_2K_BANK_2 ///< 2 KB
#define SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_4 ULPSS_2K_BANK_3 ///< 2 KB

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
#define SL_SI91X_POWER_MANAGER_CORE_ENTER_CRITICAL sl_si91x_power_manager_core_entercritical()
#define SL_SI91X_POWER_MANAGER_CORE_EXIT_CRITICAL  sl_si91x_power_manager_core_exitcritical()
/** @endcond */

// -----------------------------------------------------------------------------
// Data Types.

/// @brief Struture to store configuration parameters for RAM retention.
typedef struct {
  uint16_t m4ss_ram_size_kb;  ///< M4SS RAM size that needs to be restored.
  uint16_t ulpss_ram_size_kb; ///< ULPSS RAM size that needs to be restored.
  boolean_t
    configure_ram_banks;    ///< Enable will set the RAM banks using size, disable sets RAM banks using bank number.
  uint32_t m4ss_ram_banks;  ///< M4SS RAM bank number that needs to be restored.
  uint32_t ulpss_ram_banks; ///< ULPSS RAM bank number that needs to be restored.
} sl_power_ram_retention_config_t;

/// @brief Structure to store masked values of peripherals to enable/disable it.
typedef struct {
  uint32_t m4ss_peripheral;  ///< Masked value of M4SS Peripherals.
  uint32_t ulpss_peripheral; ///< Masked value of ULPSS Peripherals.
  uint32_t npss_peripheral;  ///< Masked value of NPSS Peripherals.
} sl_power_peripheral_t;

/// @brief Enumeration for the power states.
typedef enum {
  SL_SI91X_POWER_MANAGER_PS0 = 0, ///< PS0 Power State.
  SL_SI91X_POWER_MANAGER_PS1,     ///< PS1 Power State.
  SL_SI91X_POWER_MANAGER_PS2,     ///< PS2 Power State.
  SL_SI91X_POWER_MANAGER_PS3,     ///< PS3 Power State.
  SL_SI91X_POWER_MANAGER_PS4,     ///< PS4 Power State.
  SL_SI91X_POWER_MANAGER_SLEEP,   ///< Sleep.
  SL_SI91X_POWER_MANAGER_STANDBY, ///< Standby.
  LAST_ENUM_POWER_STATE,          ///< Last enum for validation.
} sl_power_state_t;

/// @brief Enumeration for clock scaling parameters.
typedef enum {
  SL_SI91X_POWER_MANAGER_POWERSAVE,   ///< Minimum supported frequency in a power state.
  SL_SI91X_POWER_MANAGER_PERFORMANCE, ///< Maximum supported frequency in a power state.
  LAST_ENUM_CLOCK_SCALING,            ///< Last enum for validation.
} sl_clock_scaling_t;

/// On ISR Exit Hook answer.
typedef enum {
  SL_SI91X_POWER_MANAGER_ISR_IGNORE =
    (1UL << 0UL), ///< The module did not trigger an ISR and it does not want to contribute to the decision.
  SL_SI91X_POWER_MANAGER_ISR_SLEEP =
    (1UL << 1UL), ///< The module was the one that caused the system wakeup and the system SHOULD go back to sleep.
  SL_SI91X_POWER_MANAGER_ISR_WAKEUP =
    (1UL << 2UL), ///< The module was the one that caused the system wakeup and the system MUST NOT go back to sleep.
} sl_si91x_power_manager_on_isr_exit_t;

/// @brief Mask of all the event(s) to listen to.
typedef uint32_t sl_power_manager_ps_transition_event_t;

/***************************************************************************/
/**
 * @brief Typedef for the user-supplied callback function which is called when
 *        a power state transition occurs.
 * 
 * This typedef defines a callback function that is called when a power state transition occurs.
 * 
 * @param from Power state we are leaving.
 * @param to   Power state we are entering.
 ******************************************************************************/
typedef void (*sl_power_manager_ps_transition_on_event_t)(sl_power_state_t from, sl_power_state_t to);

/**
 * @brief Structure representing power state transition event information.
 */
typedef struct {
  const sl_power_manager_ps_transition_event_t
    event_mask; ///< Mask of the transitions on which the callback should be called.
  const sl_power_manager_ps_transition_on_event_t on_event; ///< Function that must be called when the event occurs.
} sl_power_manager_ps_transition_event_info_t;

/// @brief Structure representing power state transition event handle
typedef struct {
  sl_slist_node_t node;                              ///< List node.
  sl_power_manager_ps_transition_event_info_t *info; ///< Handle event info.
} sl_power_manager_ps_transition_event_handle_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// -----------------------------------------------------------------------------
// Internal API Prototypes
/*******************************************************************************
 * @brief To update the power state requirement, requirement table, and the current state variable.
 * 
 * This function updates the power state requirement, requirement table, and the current state variable.
 * 
 * @note FOR INTERNAL USE ONLY.
 * 
 * @param[in] state Power state requirement that needs to be updated.
 * @param[in] add   Flag indicating if the requirement is added (true) or removed (false).
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_ALREADY_INITIALIZED  - Power Manager is already initialized.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 ******************************************************************************/
sl_status_t sli_si91x_power_manager_update_ps_requirement(sl_power_state_t state, boolean_t add);

//  To make sure that we are able to optimize out the string argument when the
// debug feature is disabled, we use a pre-processor macro resulting in a no-op.
// We also make sure to always have a definition for the function regardless if
// the debug feature is enable or not for binary compatibility.
#if (SL_SI91X_POWER_MANAGER_DEBUG == 1)
void sli_si91x_power_manager_debug_log_ps_requirement(sl_power_state_t ps, bool add, const char *name);
#else
#define sli_si91x_power_manager_debug_log_ps_requirement(em, add, name) /* no-op */
#endif
/** @endcond */

// -----------------------------------------------------------------------------
// Prototypes

/***************************************************************************/
/**
 * @brief To initialize the Power Manager service.
 * 
 * @details The application is configured to start in the PS3 Powersave state with a clock frequency of 40MHz.
 * 
 * @note The Power Manager initialization is automatically handled within the sl_service_init() function. 
 *       Users do not need to manually initialize the Power Manager, ensuring a hassle-free setup process.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_ALREADY_INITIALIZED - Power Manager is already initialized.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_init(void);
/***************************************************************************/
/**
 * @brief To disable the interrupts.
 * 
 * @details Disables all interrupts by setting PRIMASK. Fault exception handlers will still be enabled.
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_power_manager_core_entercritical(void)
{
#if (SL_SI91X_TICKLESS_MODE == 0)
  __disable_irq();
#endif
}
/***************************************************************************/
/**
 * @brief To enable the interrupts.
 * 
 * @details Enables interrupts by clearing PRIMASK.
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_power_manager_core_exitcritical(void)
{
#if (SL_SI91X_TICKLESS_MODE == 0)
  __enable_irq();
#endif
}
/***************************************************************************/
/**
 * @brief To add a requirement on power states.
 * 
 * @details The default state for the Power Manager is PS3. The maximum number of requirements 
 *          that can be added is 255. The Power Manager switches to the state if it is a valid 
 *          transition. Before transitioning from one state to another, make sure to remove 
 *          requirements of previous states if any were added. If an invalid state requirement 
 *          is added, it returns SL_STATUS_INVALID_PARAMETER. If the Power Manager service is 
 *          not initialized, it returns SL_STATUS_NOT_INITIALIZED. To initialize, call 
 *          \ref sl_si91x_power_manager_init. To get the requirements on all power states, call 
 *          \ref sl_si91x_power_manager_get_requirement_table. To know the current power state, 
 *          use \ref sl_si91x_power_manager_get_current_state.
 * 
 * @pre Pre-conditions:
 * - \ref sl_si91x_power_manager_init
 * - \ref sl_si91x_power_manager_ps2_pre_check (if PS2 state is added along with WiFi component)
 * 
 * @param[in] state Power state to add requirement: \ref sl_power_state_t
 *                  - SL_POWER_MANAGER_PS4
 *                  - SL_POWER_MANAGER_PS3
 *                  - SL_POWER_MANAGER_PS2
 *                  - SL_POWER_MANAGER_PS1
 *
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_NOT_INITIALIZED  - Power Manager is not initialized.
 *         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter is passed.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 ******************************************************************************/
__STATIC_INLINE sl_status_t sl_si91x_power_manager_add_ps_requirement(sl_power_state_t state)
{
  sl_status_t status = SL_STATUS_OK;

  SL_SI91X_POWER_MANAGER_CORE_ENTER_CRITICAL;

  // updates the current power state.
  status = sli_si91x_power_manager_update_ps_requirement(state, true);
  if (status != SL_STATUS_OK) {
    if (state != SL_SI91X_POWER_MANAGER_PS1) {
      SL_SI91X_POWER_MANAGER_CORE_EXIT_CRITICAL;
    }
    return status;
  }
  if ((state != SL_SI91X_POWER_MANAGER_PS1) && (state != SL_SI91X_POWER_MANAGER_PS0)) {
    sli_si91x_power_manager_debug_log_ps_requirement(state, true, (const char *)CURRENT_MODULE_NAME);
  }
  if (state != SL_SI91X_POWER_MANAGER_PS1) {
    SL_SI91X_POWER_MANAGER_CORE_EXIT_CRITICAL;
  }
  return status;
}

/***************************************************************************/ /**
 * @brief To remove requirement on power states.
 * @details Default state for Power Manager is PS3.
 * Requirements added to a power state must be removed in pairs to ensure proper state transitions. 
 * If a requirement is added for a specific power state (e.g., PS4) and not removed, the application 
 * will be unable to transition to a lower power state. For instance, if a requirement on PS4 is added 
 * but not removed, the Power Manager will remain in PS4 and cannot transition to a lower power state.
 * 
 * @note: If the current state is PS4 and no other requirements are added, removing the PS4 requirement 
 * will cause the device to remain in its last active state, which in this case is PS4.
 * 
 * @pre Pre-conditions:
 * - \ref sl_si91x_power_manager_init
 * 
 * @param[in] state Power state to remove requirement: \ref sl_power_state_t 
 *                  - SL_POWER_MANAGER_PS4
 *                  - SL_POWER_MANAGER_PS3
 *                  - SL_POWER_MANAGER_PS2
 *                  - SL_POWER_MANAGER_PS1
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Success.
 *         - SL_STATUS_NOT_INITIALIZED    - The Power Manager is not initialized.
 *         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter is passed.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 ******************************************************************************/
__STATIC_INLINE sl_status_t sl_si91x_power_manager_remove_ps_requirement(sl_power_state_t state)
{
  SL_SI91X_POWER_MANAGER_CORE_ENTER_CRITICAL;
  sl_status_t status = SL_STATUS_OK;
  // updated the current power state.
  status = sli_si91x_power_manager_update_ps_requirement(state, false);
  if (status != SL_STATUS_OK) {
    SL_SI91X_POWER_MANAGER_CORE_EXIT_CRITICAL;
    return status;
  }
  if ((state != SL_SI91X_POWER_MANAGER_PS1) && (state != SL_SI91X_POWER_MANAGER_PS0)) {
    sli_si91x_power_manager_debug_log_ps_requirement(state, false, (const char *)CURRENT_MODULE_NAME);
  }
  SL_SI91X_POWER_MANAGER_CORE_EXIT_CRITICAL;
  return status;
}

/***************************************************************************/
/**
 * @brief To configure the clock scaling.
 * 
 * @details PS4 and PS3 states are supported only. Possible values for clock scaling are:
 *          - POWERSAVE and PERFORMANCE 
 *          - PS4 Performance: 180 MHz clock 
 *          - PS4 Power-save: 100 MHz clock 
 *          - PS3 Performance: 80 MHz clock 
 *          - PS3 Power-save: 40 MHz clock 
 *          - For PS2 state, 20 MHz clock is default. 
 * 
 * If the Power Manager service is not initialized, it returns SL_STATUS_NOT_INITIALIZED. 
 * To initialize, call \ref sl_si91x_power_manager_init.
 * 
 * @pre Pre-conditions:
 * - \ref sl_si91x_power_manager_init 
 * 
 * @param[in] mode Clock scaling mode (of type \ref sl_clock_scaling_t).
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_NOT_INITIALIZED  - Power Manager is not initialized.
 *         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter is passed.
 *         - SL_STATUS_INVALID_CONFIGURATION  - Invalid configuration of mode.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_set_clock_scaling(sl_clock_scaling_t mode);

/***************************************************************************/ /**
 * @brief the clock scaling mode in PS4 and PS3 power state.
 * Possible return values: *
 * SL_SI91X_POWER_MANAGER_POWERSAVE   (Minimum supported frequency in a power state)
 * SL_SI91X_POWER_MANAGER_PERFORMANCE (Maximum supported frequency in a power state)
 * 
 * @return The following values are returned:
 * - sl_clock_scaling_t enum value indicating current clock scaling mode
 ******************************************************************************/
sl_clock_scaling_t sl_si91x_power_manager_get_clock_scaling(void);

/***************************************************************************/ /**
 * Adds the peripheral requirement.
 * Power on the peripherals the valid peripherals passed in the structure. 
 * Structure member possible values: \ref sl_power_peripheral_t
 * - m4ss_peripheral  -> Accepts masked value of m4ss peripherals.
 * - ulpss_peripheral -> Accepts masked value of ulpss peripherals.
 * - npss_peripheral  -> Accepts masked value of npss peripherals. 
 * The values of enums can be combined by using 'OR' operator and then passed to the variable.
 * 
 * @pre Pre-conditions:
 * - \ref sl_si91x_power_manager_init 
 * 
 * @param[in] peripheral Structure for different peripherals \ref sl_power_peripheral_t.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_STATE (0x0002) - Not a valid transition.
 *         - SL_STATUS_NOT_INITIALIZED  - Power Manager is not initialized.
 *         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter is passed.
 * 
 * @note The user must take care of the initialization of the peripherals added.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_add_peripheral_requirement(sl_power_peripheral_t *peripheral);

/***************************************************************************/
/**
 * @brief To remove the peripheral requirement.
 * 
 * @details Powers off the peripherals specified in the structure. Valid peripherals are passed in the structure 
 *          \ref sl_power_peripheral_t. The structure members can have the following values:
 *          - m4ss_peripheral  -> Accepts masked value of M4SS peripherals.
 *          - ulpss_peripheral -> Accepts masked value of ULPSS peripherals.
 *          - npss_peripheral  -> Accepts masked value of NPSS peripherals.
 * 
 * The values of enums can be combined using the 'OR' operator and then passed to the variable.
 * 
 * @pre Pre-conditions:
 * - \ref sl_si91x_power_manager_init 
 * 
 * @param[in] peripheral Structure for different peripherals \ref sl_power_peripheral_t.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_NOT_INITIALIZED  - Power Manager is not initialized.
 *         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter is passed.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_remove_peripheral_requirement(sl_power_peripheral_t *peripheral);

/***************************************************************************/
/**
 * @brief To register a callback to be called on given power state transition(s).
 * 
 * @details If the Power Manager service is not initialized, it returns SL_STATUS_NOT_INITIALIZED. 
 *          To initialize, call \ref sl_si91x_power_manager_init.
 * 
 * @pre Pre-conditions:
 * - \ref sl_si91x_power_manager_init 
 *
 * @param[in] event_handle Event handle (no initialization needed).
 * @param[in] event_info   Event info structure that contains the event mask and the callback that must be called.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_NOT_INITIALIZED  - Power Manager is not initialized.
 *         - SL_STATUS_NULL_POINTER  - Null pointer is passed.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * 
 * @note Adding and removing power state transition requirement(s) from a callback on a transition event
 *       is not supported.
 * @note The parameters passed must be persistent, meaning that they need to survive
 *       until the callback fires.
 * @note An ASSERT is thrown if the handle is not found.
 *
 * Usage example:
 * ```c
 * #define PS_EVENT_MASK          (  SL_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS4 \
 *                                 | SL_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS4  \
 *                                 | SL_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS3 \
 *                                 | SL_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS3  \
 *                                 | SL_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS2 \
 *                                 | SL_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS2  \
 *                                 | SL_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS0 \
 *                                 | SL_POWER_MANAGER_EVENT_TRANSITION_LEAVING_SLEEP)
 *
 * sl_power_manager_ps_transition_event_handle_t handle;
 * sl_power_manager_ps_transition_event_info_t info = { .event_mask = PS_EVENT_MASK,
 *                                                      .on_event   = transition_callback };
 *
 * void transition_callback(sl_power_state_t from, sl_power_state_t to)
 * {
 *   [...]
 * }
 *
 * void main(void)
 * {
 *   sl_status_t status;
 * 
 *   status = sl_si91x_power_manager_init();
 *   // Validate the status
 * 
 *   status = sl_si91x_power_manager_subscribe_ps_transition_event(&handle, &info);
 *   // Validate the status
 * }
 * ```
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_subscribe_ps_transition_event(
  sl_power_manager_ps_transition_event_handle_t *event_handle,
  const sl_power_manager_ps_transition_event_info_t *event_info);

/***************************************************************************/
/**
 * @brief To unregister an event callback handle on power state transition.
 * 
 * @details If the Power Manager service is not initialized, it returns SL_STATUS_NOT_INITIALIZED. 
 *          To initialize, call \ref sl_si91x_power_manager_init.
 * 
 * @pre Pre-conditions:
 * - \ref sl_si91x_power_manager_init 
 * 
 * \if defined(SL_SI91X_POWER_MANAGER_DEBUG_COMPONENT)
 * @ref sl_power_manager_debug_print_ps_requirements() Prints a table of power state requirements and their owners.
 * \endif
 * 
 * @param[in] event_handle Event handle which must be unregistered (must have been registered previously).
 * @param[in] event_info   Event info structure that contains the event mask and the callback that must be called.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_NOT_INITIALIZED  - Power Manager is not initialized.
 *         - SL_STATUS_NULL_POINTER  - Null pointer is passed.
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *  
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * @note An ASSERT is thrown if the handle is not found.
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_unsubscribe_ps_transition_event(
  sl_power_manager_ps_transition_event_handle_t *event_handle,
  const sl_power_manager_ps_transition_event_info_t *event_info);

/***************************************************************************/
/**
 * @brief To move into sleep mode and wait for the peripheral to be set as a wakeup source
 *        to trigger and wake up the M4 SoC.
 * 
 * @details It supports PS4, PS3, and PS2 only; it cannot enter sleep mode from any other 
 *          active states. If any error occurs, it returns the error code and does not transition 
 *          to sleep mode.
 * 
 * @note This function expects and calls a callback with the following
 *       signature: `boolean_t sl_si91x_power_manager_is_ok_to_sleep(void)`.
 *       This function can be used to cancel a sleep action and handle the
 *       possible race condition where an ISR that would cause a wakeup is
 *       triggered right after the decision to call sl_si91x_power_manager_sleep()
 *       has been made.
 * 
 * This function also expects and calls a callback with the following
 * signature: `boolean_t sl_si91x_power_manager_isr_wakeup(void)` after 
 * wakeup from sleep. The possible return values are:
 *         - SL_SI91X_POWER_MANAGER_ISR_IGNORE
 *         - SL_SI91X_POWER_MANAGER_ISR_SLEEP
 *         - SL_SI91X_POWER_MANAGER_ISR_WAKEUP
 * 
 * @note It can end up in an infinite sleep-wakeup loop if continuously SL_SI91X_POWER_MANAGER_ISR_SLEEP 
 *       return value is passed.
 * 
 * @pre Pre-conditions:
 * - sl_si91x_power_manager_init
 * - sl_si91x_power_manager_configure_ram_retention
 * - sl_si91x_power_manager_set_wakeup_sources
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_NOT_INITIALIZED  - Power Manager is not initialized.
 *         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter is passed.
 *         - SL_STATUS_INVALID_STATE (0x0002) - Not a valid transition.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_sleep(void);
/***************************************************************************/
/**
 * @note Applications using RTOS with tickless mode enabled must not call this API.
 *       This API is not supposed to be used directly in the application and is called
 *       automatically when the system is in an idle state with tickless mode.
 */
/***************************************************************************/
/**
 * @brief To move into standby state and wait for the interrupt.
 * 
 * @details Standby transition is possible from PS4, PS3, and PS2 states only.
 *          Transition from sleep, PS1, or PS0 is not supported.
 * 
 * @pre Pre-conditions:
 * - \ref sl_si91x_power_manager_init
 ******************************************************************************/
void sl_si91x_power_manager_standby(void);

/***************************************************************************/
/**
 * @brief To configure the wakeup sources.
 * 
 * @details Once the wakeup source is set by the Universal Configurator (UC), 
 *          the Power Manager automatically handles the initialization, User need to 
 *          install the appropriate wakeup component based on the configured wakeup source.
 * 
 * @pre Pre-conditions:
 * - \ref sl_si91x_power_manager_init 
 * - \ref sl_si91x_power_manager_configure_ram_retention 
 * 
 * @post 
 * - \ref sl_si91x_power_manager_sleep 
 * - \ref sl_si91x_power_manager_standby 
 * 
 * @param[in] source (uint32_t) Wakeup sources.
 * @param[in] add (boolean_t) True enables and false disables the wakeup source.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_NOT_INITIALIZED  - Power Manager is not initialized.
 *         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter is passed.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_set_wakeup_sources(uint32_t source, boolean_t add);

/***************************************************************************/
/**
 * @brief To retain the RAM in low power state either by using size or RAM bank as input parameters.
 * 
 * @details Structure member possible values: \ref sl_power_ram_retention_config_t
 *          - configure_ram_banks -> Boolean to switch between RAM Bank retentions. Either by size
 *                                   or by RAM bank number.
 *              - Enable -> Use RAM Bank Number.
 *              - Disable -> Use Size.
 *          - m4ss_ram_size_kb -> Retains M4SS RAM banks according to the size.
 *              - Less than 320 KB (Enter 100 for 100 KB).
 *          - ulpss_ram_size_kb -> Retains ULPSS RAM banks according to the size.
 *              - Less than 8 KB (Enter 5 for 5 KB).
 *          - ram_bank_number -> Retains the M4SS and ULPSS RAM Bank using bank number.
 * 
 * @pre Pre-conditions:
 * - \ref sl_si91x_power_manager_init 
 * 
 * \if defined(SL_SI91X_POWER_MANAGER_DEBUG_COMPONENT)
 * @ref sl_power_manager_debug_print_ps_requirements() Prints a table of power state requirements and their owners.
 * \endif
 *  
 * @post \ref sl_si91x_power_manager_sleep
 * 
 * @param[in] config Structure for the parameters of RAM retention \ref sl_power_ram_retention_config_t.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_NOT_INITIALIZED  - Power Manager is not initialized.
 *         - SL_STATUS_NULL_POINTER  - Null pointer is passed.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_configure_ram_retention(sl_power_ram_retention_config_t *config);

/***************************************************************************/
/**
 * @brief To return the current power state.
 * 
 * @details Possible return values:
 *          - 2: SL_POWER_MANAGER_PS2, ///< PS2 Power State
 *          - 3: SL_POWER_MANAGER_PS3, ///< PS3 Power State
 *          - 4: SL_POWER_MANAGER_PS4, ///< PS4 Power State
 * 
 * @return sl_power_state_t enum value indicating the current power state.
 ******************************************************************************/
sl_power_state_t sl_si91x_power_manager_get_current_state(void);

/***************************************************************************/
/**
 * @brief To get the current requirements on all the power states.
 * 
 * @details It returns 5 values starting from PS0 to PS4.
 * 
 * @pre Pre-conditions:
 * - \ref sl_si91x_power_manager_init 
 * 
 * @return Pointer to a uint8_t type array which contains 5 elements.
 * 
 * Usage example:
 * ```c
 * void main()
 * {
 *   uint8_t *requirement_table;
 *   sl_si91x_power_manager_init();
 *   requirement_table = sl_si91x_power_manager_get_requirement_table();
 *   DEBUGOUT("PS4: %d, PS3: %d, PS2: %d, PS1: %d, PS0: %d", 
 *            requirement_table[4], 
 *            requirement_table[3], 
 *            requirement_table[2], 
 *            requirement_table[1], 
 *            requirement_table[0]); 
 * }
 * ```
 ******************************************************************************/
/**
 * @brief Retrieve the power state requirement table for the power manager.
 *
 * This function provides access to the power state requirement table, which is used
 * to manage and track the power state requirements of various components or modules
 * in the system. The table contains information about the power states or
 * dependencies required by different modules.
 *
 * @note The returned pointer points to a table managed internally by the power
 *       manager. Ensure that the table is not modified directly to avoid
 *       unexpected behavior.
 *
 * @return Pointer to the power state requirement table.
 *
 * @details  
 * This API can be used to query the current power state requirements of the system
 * and make decisions based on the power states of various components. For
 * example, it can be used in scenarios where power optimization or power state
 * transitions are required.
 */

uint8_t *sl_si91x_power_manager_get_requirement_table(void);

/*****************************************************************************/
/**
 * @fn      sl_status_t sl_si91x_power_manager_request_ps1_state(void)
 *
 * @brief   Requests the PS1 state requirement during tickless idle mode.
 * 
 * @details This function requests the PS1 power state from the power manager during tickless idle mode. If the system is in the IDLE state, it transitions into the PS1 state.
 *
 * @return  Status code indicating the result:
 *          - SL_STATUS_OK (0x0000) - PS1 state requirement successfully added.
 *          - SL_STATUS_INVALID_STATE (0x0002) - Invalid request to add PS1 state.
 *          - SL_STATUS_NOT_INITIALIZED (0x0011) - Power manager service is not initialized.
 *          - SL_STATUS_INVALID_PARAMETER (0x0021) - Invalid parameter.
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_request_ps1_state(void);

/******************************************************************************/
/**
 * @fn      sl_status_t sl_si91x_power_manager_remove_ps1_state_request(void)
 * 
 * @brief   Remove the PS1 state request during tickless idle mode.
 * 
 * @details This function removes the PS1 state request from the power manager during tickless idle mode.
 * 
 * @return  Status code indicating the result:  
 *          - SL_STATUS_OK (0x0000) - Ps1 state requirement successfully removed.
 *          - SL_STATUS_INVALID_STATE (0x0002) - Invalid request to remove PS1 state.
 *          - SL_STATUS_NOT_INITIALIZED (0x0011) - Power manager service is not initialized.
 *          - SL_STATUS_INVALID_PARAMETER (0x0021) - Invalid parameter.
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_remove_ps1_state_request(void);

/******************************************************************************/
/**
 * @fn      bool sl_si91x_power_manager_get_ps1_state_status(void)
 * 
 * @brief   Get the current ps1 state request status from the power manager during tickless idle mode.
 * 
 * @details This function indicates whether a PS1 state request has been added to the power manager during tickless idle mode.
 *
 * @return  Status code indicating the result:
 *          - true - PS1 state requirement is added.
 *          - false - PS1 state requirement is not added.
 ******************************************************************************/
bool sl_si91x_power_manager_get_ps1_state_status(void);

/******************************************************************************/
/**
 * @fn      sl_status_t sl_si91x_power_manager_request_standby_state(void)
 * 
 * @brief   Requests the standby state during tickless idle mode.
 * 
 * @details This function requests the standby power state from the power manager during tickless idle mode. If the system is in the IDLE state, it transitions into the standby state.
 *
 * @return  Status code indicating the result:
 *          - SL_STATUS_OK (0x0000) - Standby state requirement successfully added.
 *          - SL_STATUS_INVALID_STATE (0x0002) - Invalid request to add standby state.
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_request_standby_state(void);

/*****************************************************************************/
/**
 * @fn      sl_status_t sl_si91x_power_manager_remove_standby_state_request(void)
 * 
 * @brief   Remove the standby state request during tickless idle mode.
 * 
 * @details This function removes the standby state request from the power manager during tickless idle mode.
 *
 * @return  Status code indicating the result:
 *          - SL_STATUS_OK (0x0000) : Standby state requirement successfully removed.
 *          - SL_STATUS_INVALID_STATE (0x0002) - Invalid request to remove standby state.
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_remove_standby_state_request(void);

/***************************************************************************/
/**
 * @fn      bool sl_si91x_power_manager_get_standby_state_status(void)
 * 
 * @brief   Get the current standby state request status during tickless idle mode.
 * 
 * @details This function indicates whether a standby state request has been added to the power manager during tickless idle mode.
 *
 * @return  Status code indicating the result:
 *          - true : Standby state requirement is added.
 *          - false : Standby state requirement is not added.
 ******************************************************************************/
bool sl_si91x_power_manager_get_standby_state_status(void);

/***************************************************************************/
/**
 * @brief Check if a TX command is currently in progress.
 * 
 * @details This function is used to check the TX command status. It returns true if
 *          a TX command is currently in progress; otherwise, it returns false otherwise.
 *
 * @pre Pre-conditions:
 * - The Wi-Fi component must be added to the project for this function to work properly.
 *   If it is not included, this function will always return false.
 *
 * @return  bool - Returns true if TX command is in progress; otherwise, returns false otherwise.
 *
 * @note    This function is useful for power management decisions and determining
 *          when the system is ready for sleep or other power state transitions.
 ******************************************************************************/
bool sl_si91x_power_manager_is_tx_command_in_progress(void);

/***************************************************************************/
/**
 * @brief To de-initialize the Power Manager service.
 * 
 * @details It clears all the power requirements and callback event subscriptions.
 *          If the Power Manager service is not initialized, it returns SL_STATUS_NOT_INITIALIZED. 
 *          To initialize, call \ref sl_si91x_power_manager_init.
 * 
 * @pre Pre-conditions:
 * - \ref sl_si91x_power_manager_init 
 ******************************************************************************/
void sl_si91x_power_manager_deinit(void);

/******************************************************************************/
/**
 * @fn      bool sl_si91x_power_manager_ps2_pre_check(void)
 * 
 * @brief   Performs pre-transition checks before entering PS2 state.
 * 
 * @details This function verifies whether the Network Wireless Processor (NWP) has any pending 
 *          packet transfers to the M4 core. If pending packets are detected, it returns false 
 *          to prevent PS2 state transition and avoid data loss. Wi-Fi component must be included
 *          in the project for this function to operate correctly.
 *
 * @return  Status code indicating the result:
 *          - true - Safe to enter PS2 state (no pending packets).
 *          - false - Pending packets detected, PS2 transition should be blocked.
 ******************************************************************************/
bool sl_si91x_power_manager_ps2_pre_check(void);

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
/******************************************************************************* 
 * @brief To get the lowest possible power state from the requirement table.
 * 
 * @details It runs a for loop and gets the lowest possible power state.
 *          If all state requirements are 0, it is considered as the default state, i.e., PS4 Active.
 ******************************************************************************/
sl_power_state_t sl_si91x_get_lowest_ps(void);

/*******************************************************************************
 ********  Mandatory callback that allows to cancel sleeping action. ********
 ******************************************************************************/
boolean_t sl_si91x_power_manager_is_ok_to_sleep(void);

/** @endcond */

/** @} (end addtogroup POWER-MANAGER) */

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/** 
* @addtogroup POWER-MANAGER Power Manager
* @{
*
*   @details
*
*    @section POWER-MANAGER_Intro Introduction
*   The Power Manager is a platform-level software module that manages the system's power states. The power state requirements are set by the different software modules (drivers, stacks, application code, etc...).
*   Power Manager also offers a notification mechanism through which any piece of software module can be notified of power state transitions through callbacks.
*
*   ***Initialization***
*
*   Power Manager must be initialized before any call to the Power Manager API.
*   If sl_system is used, only sl_system_init() must be called, otherwise
*   @ref sl_si91x_power_manager_init() must be called manually.
*
*   ***Power Manager Architecture***
*
* @image html power_manager_arch.png "Power manager architecture"
*
*   ***Add and remove requirements***
*
*   The driver/application can add and remove power state requirements at runtime. Adding requirement function calls changes the power state. Removing requirement function calls will not have any effect for state transition.
*   @ref sl_si91x_power_manager_add_peripheral_requirement()
*
*   @ref sl_si91x_power_manager_remove_peripheral_requirement()
*
*   ***Subscribe to events***
*
*   It is possible to get notified when the system transitions from a power state to
*   another power state. This can allow to do some operations depending on which level
*   the system goes, such as saving/restoring context.
*   @ref sl_si91x_power_manager_subscribe_ps_transition_event()
*
*   @ref sl_si91x_power_manager_unsubscribe_ps_transition_event()
*
*   ***Memory Footprint Analysis***
*
*   - Memory footprint analysis helps in understanding the memory usage of the application.            
*   - It includes details about stack, heap, and global/static memory usage.
*   - Tools like map files or IDE-specific memory analyzers can be used to analyze the memory footprint.
*   - Optimizing memory usage ensures efficient resource utilization and avoids memory overflows.
*   - In this example, ensure that the stack size (3072 bytes) and other memory allocations are sufficient for the application's requirements.
*
*   ***Tickless idle Mode***
*
*   - Application Task: This represents tasks running in your application. These tasks might call vTaskDelay or other blocking functions, leading the scheduler to consider them idle.
*   - Scheduler (Idle): When no tasks are ready to run, the scheduler enters an idle state.
*   - Check for Idle: The scheduler checks if FreeRTOS tickless mode is enabled (configUSE_TICKLESS_IDLE=1) and if all tasks are blocked.
*   - Suspend Ticks & Enter Sleep: If conditions are met, the scheduler calls vPortSuppressTicksAndSleep to:
*      - Disable the system tick interrupt.
*      - start a timer to track the sleep duration.
*      - Enter a low-power mode using MCU power management features (e.g., Wait For Interrupt instruction).
*   - Wakeup Interrupt: An interrupt (event or timer) wakes up the MCU.
*   - Scheduler Update: The scheduler calculates the sleep duration and adjusts the system tick counter accordingly.
*   - Resume Scheduler & Tasks: The scheduler resumes the tick interrupt and exits the low-power mode. Tasks become ready to run again.
*   - Application Code: Application tasks continue execution based on the updated system time.
* 
* @image html tickless_idle_mode.png "Tickless idle mode"
*
*   ***Sleep***
*
*   When the software has no more operations and only needs to wait for an event, the
*   software must call @ref sl_si91x_power_manager_sleep().
*
*   Query callback functions
*
*   Is OK to sleep
*
*   Between the time @ref sl_si91x_power_manager_sleep() is called and the MCU goes to sleep, an ISR may occur and require the system to resume at that time instead of sleeping. In this case, a callback is called in a critical section to validate that the MCU can go to sleep.
*
*   The function sl_si91x_power_manager_is_ok_to_sleep() will be generated automatically by Simplicity Studio's wizard.
*   The function will look at multiple software modules from the SDK to make a decision.
*   The application can contribute to the decision by defining the function `app_is_ok_to_sleep()`.
*   If any of the software modules (including the application via app_is_ok_to_sleep()) return false,
*   the process of entering in sleep will be aborted.
*
*   ***Sleep on ISR exit***
*
*   When the system enters sleep, the only way to wake it up is via an interrupt or
*   exception. By default, Power Manager will assume that when an interrupt
*   occurs and the corresponding ISR has been executed, the system must not go back
*   to sleep. However, in the case where all the processing related to this interrupt
*   is performed in the ISR, it is possible to go back to sleep by using this hook.
*
*   The function sl_si91x_power_manager_sleep_on_isr_exit() will be generated
*   automatically by Simplicity Studio's wizard. The function will look at multiple software modules from the SDK
*   to make a decision. The application can contribute to the decision by defining the
*   function app_sleep_on_isr_exit().
*   The generated function will make a decision based on the value returned by the different software modules
*   (including the application via app_sleep_on_isr_exit()):
*
*   - SL_SI91X_POWER_MANAGER_ISR_IGNORE: if the software module did not cause the system wakeup and/or doesn't want to contribute to the decision.
*
*   - SL_SI91X_POWER_MANAGER_ISR_SLEEP: if the software module did cause the system wakeup, but the system should go back to sleep.
*
*   - SL_SI91X_POWER_MANAGER_ISR_WAKEUP: if the software module did cause the system wakeup, and the system should not go back to sleep.
*
*   If any software module returned SL_SI91X_POWER_MANAGER_ISR_SLEEP and none returned SL_SI91X_POWER_MANAGER_ISR_WAKEUP,
*   the system will go back to sleep. Any other combination will cause the system not to go back to sleep.
*
* @image html init_deinit_sequence.png "Peripheral init/deinit sequence for sleep/wakeup"
*
*   ***Debugging feature***
*
*   By installing the Power Manager debug component and setting the configuration define SL_SI91X_POWER_MANAGER_DEBUG to 1, it is possible
*   to record the requirements currently set and their owner. It is possible to print
*   a table at any time that lists all the added requirements and their owner. This
*   table can be printed by calling the function:
*   \if defined(SL_SI91X_POWER_MANAGER_DEBUG_COMPONENT)
*   @ref sl_power_manager_debug_print_ps_requirements() Prints a table of power state requirements and their owners.
*   \endif
*   Make sure to add the following define
*
*  ```C
*   #define CURRENT_MODULE_NAME // Module printable name here
*  ```
*   to any application code source file that adds and removes requirements.
*
*
*   @n @section POWER-MANAGER_Config Configuration
*
*   Power Manager allows configuration of RAM retention, peripheral states, wakeup sources, and clock scaling.
*
*   - Clock Scaling:
*    - Clock can be configured as power-save or performance based on the user requirement. By default after the state change, the clock is configured as powersave. The below API is used to set the clock scaling.
*      @ref sl_si91x_power_manager_set_clock_scaling(); where mode can be either power-save or performance.
*   - Peripheral States:
*    - High power, ULPSS, and NPSS peripherals can be configured using the below APIs. They can be powered on or off based on user requirements.
*      @ref sl_si91x_power_manager_add_peripheral_requirement()
*      @ref sl_si91x_power_manager_remove_peripheral_requirement();
*   - RAM retention:
*    - Retains the RAM in low power state either by using size or RAM bank as input parameter.
*      @ref sl_si91x_power_manager_configure_ram_retention();
*   - Wakeup source:
*    - The wakeup sources can be configured using the below API.
*       @ref sl_si91x_power_manager_set_wakeup_sources();
*
*   @n @section POWER-MANAGER_Usage Usage
*
*   - Initialization:
*    - @ref sl_si91x_power_manager_init(): Initializes the Power Manager service and sets the initial power state.
*   - Peripheral requirements:
*    - @ref sl_si91x_power_manager_add_peripheral_requirement(): Turns on/off the peripheral power
*    - @ref sl_si91x_power_manager_remove_peripheral_requirement(): Removes the peripheral requirement.
*   - Power State Transitions:
*    - @ref sl_si91x_power_manager_subscribe_ps_transition_event(): Registers a callback for a specific power state transition.
*    - @ref sl_si91x_power_manager_sleep(): To Put the device into sleep mode.
*    - @ref sl_si91x_power_manager_unsubscribe_ps_transition_event(): Unregisters the callback for the power state transition.
*   - De-initialization:
*    - @ref sl_si91x_power_manager_deinit(): De-initializes the Power Manager service.
*
*   ***Usage Example***
*
*   ```C
*   #define  PS_EVENT_MASK      ( SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS4\
*                               | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS4 \
*                               | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS3\
*                               | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS3 \
*                               | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS2\
*                               | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS2 \
*                               | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_SLEEP)
*
*   static void power_manager_app(void)
*   {
*   sl_status_t status;
*   sl_power_manager_ps_transition_event_handle_t handle;
*   sl_power_manager_ps_transition_event_info_t info = { .event_mask = PS_EVENT_MASK, .on_event = transition_callback };
*
*
*   // Subscribe the state transition callback events, the ored value of flag and function pointer is passed in this API.
*   status = sl_si91x_power_manager_subscribe_ps_transition_event(&handle, &info);
*   if (status != SL_STATUS_OK) {
*     // If status is not OK, return with the error code.
*     return;
*    }
*   // Configuring the RAM retention used for sleep-wakeup.
*   sl_power_ram_retention_config_t config;
*    config.configure_ram_banks = true;
*    config.m4ss_ram_banks      = SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_8 | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_9 | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_10;
*    config.ulpss_ram_banks     = SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_2 | SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_3;
*    // RAM retention modes are configured and passed into this API.
*    status = sl_si91x_power_manager_configure_ram_retention(&config);
*    if (status != SL_STATUS_OK) {
*    // If status is not OK, return with the error code.
*     return;
*    }
*
*   // Change state to PS2
*   sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS2);
*
*   // CODE BLOCK //
*
*   // Change state to PS4
*   // Removed PS2 requirement as it is no longer required.
*   sl_si91x_power_manager_remove_ps_requirement(SL_SI91X_POWER_MANAGER_PS2);
*   sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS2);
*
*   // SLEEP_WAKEUP
*
*   // Initialize wakeup source
*   // Replace the wakeup source peripheral macro defined in sl_si91x_power_manager.h file
*   // It sets the below peripheral as wakeup source
*   sl_si91x_power_manager_set_wakeup_sources(WAKEUP_SOURCE, true);
*   sl_si91x_power_manager_sleep();
*   }
*
*   ```
*
* @} end group POWER-MANAGER ********************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_POWER_MANAGER_H */
