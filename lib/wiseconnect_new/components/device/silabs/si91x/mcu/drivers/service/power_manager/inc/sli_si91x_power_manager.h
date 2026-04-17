/***************************************************************************/ /**
 * @file sli_si91x_power_manager.h
 * @brief Power Manager Internal API implementation
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

#ifndef SLI_SI91X_POWER_MANAGER_H
#define SLI_SI91X_POWER_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_si91x_power_manager.h"
#include "system_si91x.h"
#include "rsi_power_save.h"

/***************************************************************************/
/**
 * @addtogroup POWER-MANAGER
 * @ingroup SI91X_SERVICE_APIS
 * @{
 ******************************************************************************/
/***************************************************************************/
/**
 * @defgroup Data_Types Data Types
 * @brief This section defines the data types used in the power manager module.
 * @{
 ******************************************************************************/

/** @} */ // end of Data_Types

/**
 * @brief Structure to store configuration parameters for sleep.
 */
typedef struct {
  uint32_t stack_address;           // Stack address
  uint32_t vector_offset;           // Vector offset
  uint32_t wakeup_callback_address; // Wakeup callback address
  uint32_t mode;                    // Mode for wakeup \ref sli_power_sleep_mode_t
  uint8_t low_freq_clock;           // Low freq clock \ref sli_power_low_freq_clock_t
} sli_power_sleep_config_t;

/**
 * @brief Enumeration for the sleep modes.
 */
typedef enum {
#if (SLI_SI917)
  SLI_SI91X_POWER_MANAGER_WAKEUP_FROM_FLASH_MODE = 0x11, // Wakeup from flash mode
#else
  SLI_SI91X_POWER_MANAGER_WAKEUP_FROM_FLASH_MODE = 1, // Wakeup from flash mode
#endif
  SLI_SI91X_POWER_MANAGER_WAKEUP_WITHOUT_RETENTION = 2,               // Wakeup without retention
  SLI_SI91X_POWER_MANAGER_WAKEUP_WITH_RETENTION,                      // Wakeup with retention
  SLI_SI91X_POWER_MANAGER_WAKEUP_WITH_RETENTION_WITHOUT_ULPSS_RAM,    // Wakeup with retention without ULPSS RAM
  SLI_SI91X_POWER_MANAGER_WAKEUP_WITHOUT_RETENTION_WITHOUT_ULPSS_RAM, // Wakeup without retention without ULPSS RAM
  LAST_ENUM_POWER_SLEEP_MODE,                                         // Last Enum for validation
} sli_power_sleep_mode_t;

/**
 * @brief Enumeration for low frequency clocks.
 */
typedef enum {
  SLI_SI91X_POWER_MANAGER_DISABLE_LF_MODE   = DISABLE_LF_MODE,    // Disable low frequency clock
  SLI_SI91X_POWER_MANAGER_LF_32_KHZ_RC      = (uint32_t)1 << (0), // Low Frequency 32 KHz RC Clock
  SLI_SI91X_POWER_MANAGER_LF_32_KHZ_XTAL    = (uint32_t)1 << (1), // Low Frequency 32 KHz XTAL Clock
  SLI_SI91X_POWER_MANAGER_EXTERNAL_CAP_MODE = (uint32_t)1 << (2), // External cap mode
  LAST_ENUM_POWER_LOW_FREQ_CLOCK,                                 // Last Enum for validation
} sli_power_low_freq_clock_t;

/***************************************************************************/
/**
 * @defgroup Prototypes Prototypes
 * @brief This section defines the function prototypes used in the power manager module.
 * @{
 ******************************************************************************/

/** @} */ // end of Prototypes
/***************************************************************************/
/**
 * @brief To update the Power State as per the from and to parameters.
 * 
 * @note FOR INTERNAL USE ONLY.
 * 
 * @param[in] from Power State from which the transition takes place (of type \ref sl_power_state_t).
 * @param[in] to   Power State to which the transition takes place (of type \ref sl_power_state_t).
 * 
 * @return Status code of the operation.
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter is passed.
 ******************************************************************************/
sl_status_t sli_si91x_power_manager_change_power_state(sl_power_state_t from, sl_power_state_t to);

/***************************************************************************/
/**
 * @brief To configure the parameters for sleep and transitions to sleep mode.
 * 
 * @note FOR INTERNAL USE ONLY.
 * 
 * @param[in] state Current Power state (of type \ref sl_power_state_t).
 * 
 * @return Status code of the operation.
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter is passed.
 ******************************************************************************/
sl_status_t sli_si91x_power_manager_set_sleep_configuration(sl_power_state_t state);

/***************************************************************************/
/**
 * @brief To update the peripheral power state, i.e., enable and disable the peripheral
 *        as per requirements.
 * 
 * @details If the flag is set, the peripherals which are passed in the \ref sl_power_peripheral_t
 *          structure will be powered on. If the flag is cleared, the peripherals which are passed 
 *          in the \ref sl_power_peripheral_t structure will be powered off.
 * 
 * @note FOR INTERNAL USE ONLY.
 * 
 * @param[in] peripheral \ref sl_power_peripheral_t type structure.
 * @param[in] add Boolean value: true to add peripheral, false to remove peripheral.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter is passed.
 *         - SL_STATUS_NULL_POINTER  - Null pointer is passed.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sli_power_manager_update_peripheral(sl_power_peripheral_t *peripheral, boolean_t add);

/***************************************************************************/
/**
 * @brief To validate the power state transitions. If invalid, returns false.
 * 
 * @note FOR INTERNAL USE ONLY.
 * 
 * @param[in] from Power State from which the transition takes place (of type \ref sl_power_state_t).
 * @param[in] to   Power State to which the transition takes place (of type \ref sl_power_state_t).
 * 
 * @return Boolean value: true if the transition is valid, false otherwise.
 ******************************************************************************/
boolean_t sli_si91x_power_manager_is_valid_transition(sl_power_state_t from, sl_power_state_t to);

/***************************************************************************/
/**
 * @brief To configure the wakeup sources.
 * 
 * If the flag is set, the wakeup sources passed in the source variable are set.
 * If the flag is cleared, the wakeup sources passed in the source variable are cleared.
 * 
 * @note FOR INTERNAL USE ONLY.
 * 
 * @param[in] source Ored value of wakeup sources (uint32_t).
 * @param[in] flag   Boolean value: true to add the wakeup source, false to remove the wakeup source.
 * 
 * @return Status code of the operation.
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter is passed.
 ******************************************************************************/
sl_status_t sli_si91x_power_configure_wakeup_resource(uint32_t source, boolean_t add);

/***************************************************************************/
/**
 * @brief To set the RAM retention and configures the RAM banks that need to be retained.
 * 
 * @note FOR INTERNAL USE ONLY.
 * 
 * @param[in] sram_bank RAM retention configuration (of type \ref sl_power_ram_retention_config_t).
 * 
 * @return Status code of the operation.
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter is passed.
 *         - SL_STATUS_NULL_POINTER  - Null pointer is passed.
 ******************************************************************************/
sl_status_t sli_si91x_power_manager_set_ram_retention_configuration(sl_power_ram_retention_config_t *sram_bank);

/***************************************************************************/
/**
 * @brief To set the initial hardware configuration.
 * 
 * @note FOR INTERNAL USE ONLY.
 * 
 * This function sets the initial hardware configuration.
 * 
 * @param[in] none No parameters.
 * 
 * @return none No return value.
 ******************************************************************************/
void sli_si91x_power_manager_init_hardware(void);

/******************************************************************************/
/**
 * @brief Configures the hardware for low power mode.
 *
 * @note FOR INTERNAL USE ONLY.
 *
 * This function sets the hardware for low power mode.
 * Disables the components and clocks which are not required.
 *
 * @param[in] is_sleep  Boolean value: true for sleep mode,false for non-sleep mode.
 *
 * @return none No return value.
 ******************************************************************************/
void sli_si91x_power_manager_low_power_hw_config(boolean_t is_sleep);

/***************************************************************************/
/**
 * @brief To initialize the debugging feature.
 * 
 * @note FOR INTERNAL USE ONLY.
 * 
 * This function initializes the debugging feature.
 * 
 * @param[in] none No parameters.
 * 
 * @return none No return value.
 ******************************************************************************/
void sli_si91x_power_manager_init_debug(void);

/** @} (end addtogroup POWER-MANAGER) */

#ifdef __cplusplus
}
#endif

#endif /* SLI_SI91X_POWER_MANAGER_H */