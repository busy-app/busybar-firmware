/*******************************************************************/
/**
 * @file sl_si91x_bod.h
 * @brief BOD API implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef __SL_SI91X_BOD_H__
#define __SL_SI91X_BOD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "si91x_device.h"
#include "sl_status.h"
#include "base_types.h"

/***************************************************************************/
/**
  * @addtogroup BOD BOD
  * @ingroup SI91X_PERIPHERAL_APIS
  * @{
  *
  ******************************************************************************/
/*******************************************************************************
  ***************************  DEFINES / MACROS   ********************************
  ******************************************************************************/
#define BOD_IRQ_Handler IRQ023_Handler ///< BOD interrupt handler

#define SL_BGSAMPLE (*(volatile uint32_t *)(0x24048140)) ///< Define the SL_BGSAMPLE register address
#define SL_BOD_CLOCK_DIVISON_FACTOR \
  (*(volatile uint32_t *)(0x24050000) & 0xF) ///< Define BOD clock division factor and ULP processor clock selection

#define SL_BOD_ULP_PROC_CLK_SEL (*(volatile uint32_t *)(0x24041414) & 0x1E) ///< Define ULP processor clock selection

#define SL_NPSS_GPIO_2_ANALOG_MODE       (*(volatile uint32_t *)0x24048624) ///< NPSS GPIO 2 analog mode
#define SL_NPSS_GPIO_2_ANALOG_MODE_VALUE 0x7                                ///< NPSS GPIO 2 analog mode value
#define SL_BOD_ENABLE_SIGNAL             BIT(16)                            ///< Enable signal for bod detection
#define SL_SCDC_IN_LP_MODE_EN            BIT(0)                             ///< SCDC in LP mode Bit
#define SL_ZERO_VAL                      0x00                               ///< Zero Value define
// Define bit masks for clearing button range value and BOD comparator selection
// register
#define SL_BUTTON_RANGE_VALUE_CLR 0x3 ///< Button range value clear mask

// Define mode selection constants
#define SL_BOD_MANUAL_MODE    1 ///< Manual mode selection
#define SL_BOD_AUTOMATIC_MODE 0 ///< Automatic mode selection

// Define enable and disable flags
#define SL_ENABLE  1 ///< Enable flag
#define SL_DISABLE 0 ///< Disable flag

// Define minimum and maximum BOD threshold values
#define SL_BOD_MIN_THRESHOLD 1.75f ///< Minimum BOD threshold value
#define SL_BOD_MAX_THRESHOLD 3.65f ///< Maximum BOD threshold value

// Define NVIC BOD interrupt number
#define SL_NVIC_BOD NPSS_TO_MCU_BOD_INTR_IRQn ///< NVIC BOD interrupt number

// Define automatic BOD mode
#define SL_AUTOMATIC_BOD 1 ///< Automatic BOD mode

// Define status and threshold constants
#define SL_BOD_MAX_MODE_VALUE          2   ///< Maximum mode value
#define SL_BOD_THRESHOLD_OFFSET_VALUE  0.5 ///< Threshold offset value
#define SL_BOD_THRESHOLD_ROUNDUP_VALUE 1   ///< Threshold roundup value

// Define bit masks for manual comparator multiplexer selection
#define SL_BOD_MANUAL_CMP_MUX_SEL_BUTTON_CLR   0xE0000 ///< Manual comparator multiplexer selection button clear mask
#define SL_BOD_MANUAL_CMP_MUX_SEL_BUTTON_VALUE 0xA0000 ///< Manual comparator multiplexer selection button value

// Define bit mask for clearing BOD test selection bits
#define BOD_TEST_SEL_BITS_CLR 0x180000 ///< BOD test selection bits clear mask

// Define maximum battery status value
#define SL_BOD_BATTERY_STATUS_MAX 38 ///< Maximum battery status value

// Define constants for BOD VBATT computation
#define SL_BOD_VBATT_COMPUT_FIRST_ELEMENT  3.3f    ///< BOD VBATT computation first element
#define SL_BOD_VBATT_COMPUT_SECOND_ELEMENT 34.814f ///< BOD VBATT computation second element
#define SL_BOD_VBATT_COMPUT_THIRD_ELEMENT  31      ///< BOD VBATT computation third element

// Define slot value range
#define SL_BOD_SLOT_MAX_VALUE 65535 ///< Slot maximum value
#define SL_BOD_SLOT_MIN_VALUE 2     ///< Slot minimum value

// Define bit mask for clearing battery status bits in EFUSE
#define SL_BOD_CLEAR_BATTERY_STATUS_BITS_EFUSE 0x3FFFFF ///< BOD clear battery status bits in EFUSE mask

#define SL_BOD_MAX_BATTERY_VOLTAGE 3.3f ///< Maximum battery voltage for Brown-Out Detector (BOD)
#define SL_BOD_MIN_BATTERY_VOLTAGE 1.6f ///< Minimum battery voltage for Brown-Out Detector (BOD)

// Define callback type for BOD interrupts
#define SL_BOD_NVIC_BUTTON                   NPSS_TO_MCU_BUTTON_INTR_IRQn ///< BOD NVIC button interrupt
#define SL_BOD_NPSS_TO_MCU_BUTTON_IRQHandler IRQ024_Handler               ///< BOD NPSS to MCU button interrupt handler

#define SL_BOD_CMP_MUX_SEL_EN              4     ///< BOD comparator multiplexer selection enable value
#define SL_BOD_MAX_BUFF_SIZE               39    ///< Maximum buffer size for BOD array
#define SL_BOD_MIN_BUFF_SIZE               0     ///< Minimum buffer size for BOD array
#define SL_BOD_VBATT_STATUS_MIN            0.0f  ///< Minimum VBATT status value
#define SL_BOD_BUTTON_ONE                  1     ///< BOD button one
#define SL_BOD_BUTTON_TWO                  2     ///< BOD button two
#define SL_BOD_BUTTON_THREE                3     ///< BOD button three
#define SL_BOD_BUTTON_TOTAL_NUM_OF_BUTTONS 3     ///<Total number of buttons can configured
#define SL_BOD_BUTTON_MIN_VAL              1.08f ///< BOD button minimum value
#define SL_BOD_BUTTON_MAX_VAL              2.27f ///< BOD button maximum value

// BOD status codes
#define SL_STATUS_BOD_NOT_ENABLED ((sl_status_t)0x0C1F) ///< Status code indicating that the BOD is not enabled
#define SL_STATUS_BOD_THRESHOLD_CONFIG_FAIL \
  ((sl_status_t)0x0C20) ///< Status code indicating a failure in configuring the BOD threshold
#define SL_STATUS_BOD_SLOT_VALUE_CONFIG_FAIL \
  ((sl_status_t)0x0C21) ///< Status code indicating a failure in configuring the BOD slot value
#define SL_STATUS_BOD_BUTTON_CONFIG_FAIL \
  ((sl_status_t)0x0C22) ///< Status code indicating a failure in configuring the BOD button

// -----------------------------------------------------------------------------
typedef void (*bod_callback_t)(void); ///< BOD callback function type

// Define button callback function type
typedef void (*bod_button_callback_t)(void); ///< Button callback function type

// Define button enable type
typedef uint8_t sl_button_en_t; ///< Button enable type

// Define a struct for voltage percentage configuration
/**
  * @struct sl_bod_button_uc_config_param_t
  * @brief Structure to configure voltage percentage thresholds for multiple buttons.
  *
  * This structure is used to define the minimum and maximum voltage percentages
  * for up to three buttons. These thresholds can be used for percentage-based
  * button detection or monitoring.
  *
  * @note The supported percentage range for button thresholds is 33% to 66%.
  *       For example, with a maximum battery voltage (VBATT) of 3.3V, valid threshold
  *       voltages for button configuration range from approximately 1.1V (33%) to 2.17V (66%).
  *       Values outside this range are not supported by the hardware.
  *       This structure is useful for applications where button activation
  *       is based on battery voltage levels rather than fixed voltage thresholds.
  */
typedef struct {
  boolean_t button_wakeup_enable;        ///< Enable or disable button wakeup functionality.
  float button_1_min_voltage_percentage; ///< Minimum voltage percentage for Button 1.
  float button_1_max_voltage_percentage; ///< Maximum voltage percentage for Button 1.
  float button_2_min_voltage_percentage; ///< Minimum voltage percentage for Button 2.
  float button_2_max_voltage_percentage; ///< Maximum voltage percentage for Button 2.
  float button_3_min_voltage_percentage; ///< Minimum voltage percentage for Button 3.
  float button_3_max_voltage_percentage; ///< Maximum voltage percentage for Button 3.
} sl_bod_button_uc_config_param_t;       ///< Structure for voltage percentage configuration

// Define a structure for BOD UC parameters
/**
  * @struct sl_bod_uc_param_t
  * @brief Structure to configure the BOD (Brown-Out Detection) parameters.
  *
   * This structure is used to configure the BOD parameters, including the slot value and blackout enable value.
  */
typedef struct {
  uint16_t slot_value; ///< Slot value for BOD configuration
  en_t blackout_en;    ///< Black out enable value
} sl_bod_uc_param_t;

/*******************************************************************/
/**
  * @brief Set the BOD mode.
  *
  * @param[in] mode The mode to set (manual or automatic).
  *
  * @pre Pre-conditions:
  *      - The BOD module must be initialized using @ref sl_si91x_bod_init().
  *
  * @return sl_status_t Status of the operation.
  *         - SL_STATUS_OK if the operation was successful.
  *         - SL_STATUS_INVALID_PARAMETER if the mode is invalid.
  *         - SL_STATUS_NOT_INITIALIZED if the BOD is not enabled. 
  *
  * For more information on status codes, see [SL STATUS DOCUMENTATION](
  * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
  * * @note The mode can be set to either `SL_BOD_MANUAL_MODE` or `SL_BOD_AUTOMATIC_MODE`.
  *       - `SL_BOD_MANUAL_MODE` allows manual control of the BOD settings.
  *       - `SL_BOD_AUTOMATIC_MODE` enables automatic BOD operation based on predefined settings.
  *****************************************************************/
sl_status_t sl_si91x_bod_set_mode(uint8_t mode);

/*******************************************************************/
/**
  * @brief Configure the slot value for BOD.
  *
  * @param[in] slot_value The slot value to set, which must be in the range 2 to 65535.
  * 
  * @pre Pre-conditions:
  *     - The BOD module must be initialized using @ref sl_si91x_bod_init().
  *     - The BOD mode must be set using @ref sl_si91x_bod_set_mode() As `SL_BOD_MODE_AUTOMATIC`.
  *
  * @return sl_status_t Status of the operation.
  *         - SL_STATUS_OK if the operation was successful.
  *         - SL_STATUS_INVALID_PARAMETER if the slot value is out of range.
  *         - SL_STATUS_NOT_INITIALIZED if the BOD is not enabled.
  *         - SL_STATUS_FAIL if the slot value could not be configured.
  *
  * For more information on status codes, see [SL STATUS DOCUMENTATION](
  * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
  *****************************************************************/
sl_status_t sl_si91x_bod_config_slot_value(uint16_t slot_value);

/*******************************************************************/
/**
  * @brief Initialize the BOD module.
  *
  * This function initializes the BOD module with the specified battery voltage
  * threshold.
  *
  * @return sl_status_t Status of the BOD initialization operation.
  *         - SL_STATUS_OK if the initialization was successful.
  *         - SL_STATUS_FAIL if the initialization failed.
  *
  * For more information on status codes, see [SL STATUS DOCUMENTATION](
  * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
  *****************************************************************/
sl_status_t sl_si91x_bod_init(void);

/*******************************************************************/
/**
  * @brief Deinitialize the BOD module.
  *
  * This function deinitializes the BOD module.
  * 
  *  @pre Pre-conditions:
  *     - The BOD module must be initialized using @ref sl_si91x_bod_init().
  *
  * @return sl_status_t Status of the BOD deinitialization operation.
  *         - SL_STATUS_OK if the deinitialization was successful.
  *         - SL_STATUS_FAIL if the deinitialization failed.
  *
  * For more information on status codes, see [SL STATUS DOCUMENTATION](
  * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
  *****************************************************************/
sl_status_t sl_si91x_bod_deinit(void);

/*******************************************************************/
/**
  * @brief Get the BOD threshold value.
  *
  * This function retrieves the current threshold value set for the BOD.
  * 
  * @pre Pre-conditions:
  *    - The BOD module must be initialized using @ref sl_si91x_bod_init().
  * 
  * @param[out] vbatt_threshold to store the current threshold value
  * @return sl_status_t Status of the BOD configuration operation.
  *         - SL_STATUS_OK if the configuration was successful.
  *         - SL_STATUS_INVALID_PARAMETER if the provided pointer is NULL.
  *
  * For more information on status codes, see [SL STATUS DOCUMENTATION](
  * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
  *****************************************************************/
sl_status_t sl_si91x_bod_get_threshold(float *vbatt_threshold);

/*******************************************************************/
/**
  * @brief Enable the BOD interrupt in the NVIC.
  *
  * This function enables the interrupt for the BOD in the NVIC, allowing the
  * system to handle BOD-related interrupts.
  *
  * @note
  *   Moving forward, this API will be deprecated. This is retained for backward compatibility.
  *****************************************************************/
void sl_si91x_bod_NVIC_enable_irq(void) SL_DEPRECATED_API_WISECONNECT_3_5;

/*******************************************************************/
/** @brief sl_si91x_bod_get_blackout_status
  *
  *  This function is used to get the blackout status.
  *
  *  @return en_t Returns the blackout status.
  *         - Enable if blackout is enabled.
  *         - Disable if blackout is disabled.
  *
  *******************************************************************/
en_t sl_si91x_bod_get_blackout_status(void);

/*******************************************************************/
/**
  * @brief Set the BOD configuration.
  *
  * @param[in] usr_config_params The BOD configuration parameters.
  * 
  * @pre Pre-conditions:
  *   - The BOD module must be initialized using @ref sl_si91x_bod_init().
  *
  * @return sl_status_t Status of the BOD configuration operation.
  *         - SL_STATUS_OK if the configuration was successful.
  *         - SL_STATUS_INVALID_PARAMETER if the provided parameters are invalid.
  *
  * For more information on status codes, see [SL STATUS DOCUMENTATION](
  * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
  *****************************************************************/
sl_status_t sl_si91x_bod_set_configuration(sl_bod_uc_param_t usr_config_params);

/*******************************************************************/
/**
  * @brief Configures the BOD (Brown-Out Detector) button values.
  * 
  * @pre Pre-conditions:
  *    - The BOD module must be initialized using @ref sl_si91x_bod_init() and 
  *    - The BOD button must be enabled using @ref sl_si91x_bod_button_wakeup_enable_v2().
  *
  * This function sets the maximum value for the button and the minimum values
  * for three different buttons.
  *
  * @param[in] button_max_value The maximum value for the button.
  * @param[in] button1_min_value The minimum value for button 1.
  * @param[in] button2_min_value The minimum value for button 2.
  * @param[in] button3_min_value The minimum value for button 3.
  *
  *****************************************************************/
void sl_si91x_bod_button_configuration(uint16_t button_max_value,
                                       uint16_t button1_min_value,
                                       uint16_t button2_min_value,
                                       uint16_t button3_min_value);

/*******************************************************************/
/**
  * @brief Disables the button wakeup functionality for the BOD (Brown-Out Detector).
  *
  * This function disables the button wakeup feature for the BOD. 
  * When disabled, the system cannot wake up from a low-power state using a button press.
  *
  * @return sl_status_t Status of the button wakeup operation.
  *         - SL_STATUS_OK if the operation completed successfully.
  *         - SL_STATUS_FAIL if the operation was unsuccessful.
  *****************************************************************/
sl_status_t sl_si91x_bod_button_wakeup_disable(void);

/*******************************************************************/
/**
  * @brief Enable or disable button wakeup functionality for the BOD (Brown-Out Detector).
  *
  * This function allows the user to enable or disable the button wakeup feature
  * for the BOD. When enabled, the system can wake up from a low-power state
  * using a button press.
  *
  * @pre Pre-conditions:
  *   - The BOD module must be initialized using @ref sl_si91x_bod_init().
  *
  * @param[in] enable
  *            - 1: Enable button wakeup functionality.
  *            - 0: Disable button wakeup functionality.
  *
  * @note
  *   Moving forward, this API will be deprecated. Instead, use @ref sl_si91x_bod_button_wakeup_enable_v2 API. This is retained for backward compatibility.
  */
void sl_si91x_bod_button_wakeup_enable(uint8_t enable) SL_DEPRECATED_API_WISECONNECT_3_5;

/*******************************************************************/
/**
  * @brief Enable button wakeup functionality for the BOD (Brown-Out Detector).
  *
  * This function allows the user to enable or disable the button wakeup feature
  * for the BOD. When enabled, the system can wake up from a low-power state
  * using a button press.
  *
  * @param[in] bod_button_enable 
  *      - 1: Enable button wakeup functionality.
  *      - 0: Disable button wakeup functionality.
  * 
  * @pre Pre-conditions:
  *   - The BOD module must be initialized using @ref sl_si91x_bod_init().
  * 
  * @return sl_status_t Status of the callback registration operation.
  *         - SL_STATUS_OK if the operation was successful.
  *         - SL_STATUS_FAIL if the operation failed.
  *
  * For more information on status codes, see [SL STATUS DOCUMENTATION](
  * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
  * *****************************************************************/
sl_status_t sl_si91x_bod_button_wakeup_enable_v2(boolean_t bod_button_enable);

/*******************************************************************/
/**
  * @brief Register a callback function for BOD interrupts.
  *
  * This function registers a callback function to be called when a BOD interrupt
  * occurs.
  * @pre Pre-conditions:
  *  - The BOD module must be initialized using @ref sl_si91x_bod_init().
  *
  * @param[in] callback The callback function to register.
  *
  * @return sl_status_t Status of the callback registration operation.
  *         - SL_STATUS_OK if the operation was successful.
  *         - SL_STATUS_NULL_POINTER if the provided callback is NULL.
  *         - SL_STATUS_BUSY if a callback is already registered.
  *
  * For more information on status codes, see [SL STATUS DOCUMENTATION](
  * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
  *****************************************************************/
sl_status_t sl_si91x_bod_register_callback(bod_callback_t callback);

/*******************************************************************/
/**
  * @brief Register a callback function for BOD button interrupts.
  *
  * This function registers a callback function to be called when a BOD button
  * interrupt occurs.
  * 
  * @pre Pre-conditions:
  *  - The BOD module must be initialized using @ref sl_si91x_bod_init().and
  *  - The BOD button must be enabled using @ref sl_si91x_bod_button_wakeup_enable_v2().
  *
  * @param[in] callback The callback function to register.
  *
  * @return sl_status_t Status of the callback registration operation.
  *         - SL_STATUS_OK if the operation was successful.
  *
  * For more information on status codes, see [SL STATUS DOCUMENTATION](
  * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
  *****************************************************************/
sl_status_t sl_si91x_bod_button_register_callback(bod_button_callback_t callback);

/*******************************************************************/
/**
  * @brief Unregister the callback function for BOD interrupts.
  * @pre Pre-conditions:
  *  - The BOD module must be initialized using @ref sl_si91x_bod_init().
  *  - BOD callback must be registered using @ref sl_si91x_bod_register_callback().
  *
  * This function unregisters the callback function for BOD interrupts.
  *****************************************************************/
void sl_si91x_bod_unregister_callback(void);

/*******************************************************************/
/**
  * @brief Get the current battery status.
  *
  * This function retrieves the current battery status.
  * 
  * @pre Pre-conditions:
  * - The BOD module must be initialized using @ref sl_si91x_bod_init().
  *
  * @param[out] battery_status Pointer to store the battery status.
  *
  * @return sl_status_t Status of the battery status retrieval operation.
  *         - SL_STATUS_OK if the operation was successful.
  *         - SL_STATUS_INVALID_PARAMETER if the battery_status pointer is NULL.
  *
  * For more information on status codes, see [SL STATUS DOCUMEN TATION](
  * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
  *****************************************************************/
sl_status_t sl_si91x_bod_get_battery_status(float *battery_status);

/*******************************************************************/
/**
  * @brief Set the BOD voltage threshold.
  *
  * This function sets the BOD voltage threshold to the specified value.
  * 
  * @pre Pre-conditions:
  *  - The BOD module must be initialized using @ref sl_si91x_bod_init().
  *
  * @param[in] threshold The BOD voltage threshold to set.
  *
  * @return sl_status_t Status of the BOD threshold setting operation.
  *         - SL_STATUS_OK if the setting was successful.
  *         - SL_STATUS_INVALID_PARAMETER if the threshold is out of range.
  *
  * For more information on status codes, see [SL STATUS DOCUMENTATION](
  * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
  *****************************************************************/
sl_status_t sl_si91x_bod_set_threshold(float threshold);

/*******************************************************************/
/**
  * @brief Enable BOD interrupt.
  *
  * This function enables the BOD interrupt for detecting voltage drops below the
  * threshold.
  * @pre Pre-conditions:
  * - The BOD module must be initialized using @ref sl_si91x_bod_init().
  *
  *****************************************************************/
void sl_si91x_bod_enable_interrupt(void);

/*******************************************************************/
/**
 * @brief Enable the interrupt for the BOD (Brown-Out Detection) button.
 *
 * This function configures and enables the interrupt associated with the
 * BOD button, allowing the system to respond to brown-out detection events.
 * @pre Pre-conditions:
 * - The BOD module must be initialized using @ref sl_si91x_bod_init().
 * - The BOD button must be enabled using @ref sl_si91x_bod_button_wakeup_enable_v2().
 * @note Ensure that the BOD button hardware and related configurations
 *       are properly initialized before calling this function.
 *****************************************************************/

void sl_si91x_bod_button_enable_interrupt(void);

/*******************************************************************/
/**
  * @brief Disable BOD interrupt.
  *
  * This function disables the BOD interrupt.
  * 
  * @pre Pre-conditions:
  * - The BOD module must be initialized using @ref sl_si91x_bod_init().
  * - BOD interrupt must be enabled using @ref sl_si91x_bod_enable_interrupt().
  *
  *****************************************************************/
void sl_si91x_bod_disable_interrupt(void);

/*******************************************************************/
/**
 * @brief Disable the interrupt for the BOD button.
 * This function disables the interrupt associated with the BOD button.
 * @pre Pre-conditions:
 * - The BOD module must be initialized using @ref sl_si91x_bod_init().
 * - BOD button interrupt must be enabled using @ref sl_si91x_bod_button_enable_interrupt().
 * @note This function is typically used when the BOD button is no longer needed
 *      or when the system needs to disable the button interrupt for any reason.
 * @note Ensure that the BOD button hardware and related configurations
 *       are properly initialized before calling this function.
 *
 *****************************************************************/
void sl_si91x_bod_button_disable_interrupt(void);

/*******************************************************************/
/**
  * @brief Clear BOD interrupt.
  * @pre Pre-conditions:
  * - The BOD module must be initialized using @ref sl_si91x_bod_init().
  * - BOD interrupt must be enabled using @ref sl_si91x_bod_enable_interrupt().
  * This function clears the BOD interrupt flag.
  *
  *****************************************************************/
void sl_si91x_bod_clear_interrupt(void);

/*******************************************************************/
/**
  * @brief Read the value of the BOD button.
  * @pre Pre-conditions:
  * - The BOD module must be initialized using @ref sl_si91x_bod_init().
  * This function reads the value of the BOD button.
  *
  * @return The current value of the BOD button, which can be 1, 2, or 3.
  *
  *****************************************************************/
uint8_t sl_si91x_bod_button_value_read(void);

/*******************************************************************/
/**
  * @brief Clear the BOD button interrupt.
  *
  * This function clears the interrupt flag for the BOD button.
  *
  * @pre Pre-conditions:
  * - The BOD module must be initialized using @ref sl_si91x_bod_init().
  * - BOD button interrupt must be enabled using @ref sl_si91x_bod_button_enable_interrupt().
  *
  *****************************************************************/
void sl_si91x_bod_button_clear_interrupt(void);

/*******************************************************************/
/**
  * @brief Configure the voltage percentage thresholds for the BOD buttons.
  *
  * This function configures the voltage percentage thresholds for up to three BOD buttons,
  * based on the provided configuration structure. It calculates and outputs the corresponding
  * button values for the maximum and minimum voltage percentages for each button.
  * @pre Pre-conditions:
  * - The BOD module must be initialized using @ref sl_si91x_bod_init().
  * 
  * @param[in] uc_config_param The voltage percentage configuration structure for the buttons.
  * @param[out] button_max_value Pointer to store the calculated maximum button value.
  * @param[out] button_1_min_value Pointer to store the calculated minimum value for button 1.
  * @param[out] button_2_min_value Pointer to store the calculated minimum value for button 2.
  * @param[out] button_3_min_value Pointer to store the calculated minimum value for button 3.
  *
  * @return sl_status_t Status of the configuration operation.
  *         - SL_STATUS_OK if the configuration was successful.
  *         - SL_STATUS_INVALID_PARAMETER if any pointer is NULL or configuration is invalid.
  *
  * @note The voltage percentages in the configuration structure should be between 0.0 and 100.0.
  *       The calculated button values are based on the battery voltage range and the specified percentages.
  *****************************************************************/
sl_status_t sl_si91x_bod_button_set_configuration(sl_bod_button_uc_config_param_t uc_config_param,
                                                  uint8_t *button_max_value,
                                                  uint8_t *button_1_min_value,
                                                  uint8_t *button_2_min_value,
                                                  uint8_t *button_3_min_value);
/*******************************************************************/
/**
  * @brief Calculate the battery percentage based on the given voltage.
  *
  * This function takes a voltage value and calculates the corresponding battery
  * percentage, storing the result in the provided pointer.
  * 
  * @pre Pre-conditions:
  *    - BOD battery status must be read using @ref sl_si91x_bod_get_battery_status()
  *
  * @param[in] voltage The input voltage value to be converted to battery
  * percentage.
  * @param[out] vbat_per Pointer to a float where the calculated battery
  * percentage will be stored.
  *
  * @return sl_status_t Returns the status of the operation.
  *                      - SL_STATUS_OK if the operation was successful.
  *                      - SL_STATUS_INVALID_PARAMETER if the input voltage is
  * out of range.
  *****************************************************************/
sl_status_t sl_si91x_bod_battery_percentage(float voltage, float *vbat_per);

/*******************************************************************/
/**
  * @brief Controls the Black Out Monitor reset functionality.
  *
  * This function enables or disables the Black Out Monitor reset functionality.
  * The BOD reset is used to reset the system when the supply voltage drops
  * below a certain threshold, ensuring the system does not operate under
  * unsafe conditions.
  *
  * @param enable A value of type en_t that specifies whether to enable or
  *               disable the BOD reset.
  *               - If enable is true, the BOD reset is enabled.
  *               - If enable is false, the BOD reset is disabled.
  *
  *****************************************************************/
void sl_si91x_bod_black_out_reset(en_t enable);

/*******************************************************************/
/**
  * @brief Clears the BOD (Brown-Out Detector) button interrupt.
  *
  * This function is used to clear the interrupt triggered by the BOD button.
  * It ensures that the interrupt flag is reset, allowing for subsequent
  * interrupts to be detected and handled appropriately.
  *
  *****************************************************************/
void sl_si91x_bod_button_interrupt_clear(void);

/*******************************************************************/
/**
  * @brief Enable the blackout reset in sleep mode.
  *
  * This API is used to enable the blackout reset functionality when the device
  * is in sleep mode. Blackout reset helps in protecting the device from
  * unexpected power failures by resetting the device.
  * @pre Pre-conditions:
  *  - The BOD module must be initialized using @ref sl_si91x_bod_init().
  * - The BOD module must be configured to enable blackout reset using @ref sl_si91x_bod_black_out_reset().
  *
  *****************************************************************/
void sl_si91x_bod_enable_blackout_in_sleep_mode(void);

/*******************************************************************/
/**
  * @brief Disable the blackout reset in sleep mode.
  *
  * This API is used to disable the blackout reset functionality when the device
  * is in sleep mode. Blackout reset helps in protecting the device from
  * unexpected power failures by resetting the device.
  *
  *****************************************************************/
void sl_si91x_bod_disable_blackout_in_sleep_mode(void);

#ifdef __cplusplus
}
#endif

/** @} end group BOD */

/***************************************************************************/
/******** THE REST OF THE FILE IS DOCUMENTATION ONLY !**********************/
/***************************************************************************/
/***************************************************************************/
/** 
  * @addtogroup BOD BOD
  * @{
  *
  * @details
  *
  * @section BOD_Intro Introduction
  *
  * - **Brown-Out Detector (BOD)** is a hardware module designed to monitor the supply voltage of the system. 
  * It ensures that the system operates within safe voltage levels by detecting voltage drops below a defined threshold. 
  * When a brown-out condition is detected, the BOD can trigger an interrupt or reset the system to prevent malfunction.
  * 
  * - **Blackout Monitoring** is a feature that enables the BOD to reset the system during Active/Sleep mode when the supply 
  * voltage drops less then ~1.65v to VMCU.
  * 
  * Common use cases for BOD include:
  * - **Voltage Monitoring**: Ensuring the supply voltage remains within safe operating limits.
  * - **System Reset**: Resetting the system during unsafe voltage (voltage drops less then ~1.65v to VMCU) conditions to prevent damage or malfunction.
  * - **Interrupt Generation**: Notifying the system of voltage drops via interrupts.
  *
  * @section BOD_Config Configuration
  *
  * The BOD module offers several configurable parameters, such as:
  * 
  * - **Threshold Voltage:** Specifies the voltage level at which the BOD will trigger an interrupt or system reset.
  * 
  * - **Slot Value:** Specifies the number of inactive (empty) slots inserted between two active comparator sampling slots. Each empty slot corresponds to 6 cycles of the 32KHz (ulp_fsm_clk), determining the interval between consecutive active sampling events.
  * Configuring the slot value allows you to adjust the timing and frequency of BOD checks, which can help balance synchronization requirements and power consumption.
  * Each slot defines a time interval based on 6 cycles of the 32KHz (ulp_fsm_clk).
  * - Slots are used to control the timing between active operations, supporting proper synchronization and improved power efficiency.
  *   
  *    - For example, if cmp_slot_value is programmed as 2, there will be 2 empty slots between active slots, each consisting of 6 cycles.
  * - **Blackout Enable**: Enabling or disabling blackout reset functionality.
  *
  * These configurations are encapsulated in the @ref sl_bod_uc_param_t structure and initialized using the respective BOD APIs.
  *
  * For more details on configuration parameters, see the respective peripheral example readme document.
  *
  * @section BOD_Usage Usage
  *
  * After defining the BOD configuration structures and passing an instance of @ref sl_bod_uc_param_t, the following functions can be used to initiate and configure the BOD module. The typical flow for implementation is as follows:
  *
  * 1. Initialize the BOD module: @ref sl_si91x_bod_init
  * 2. Register a callback for BOD interrupts: @ref sl_si91x_bod_register_callback
  * 3. Set the voltage threshold: @ref sl_si91x_bod_set_threshold
  * 4. Set the BOD configuration parameters @ref sl_si91x_bod_set_configuration.
  * 5. Get the calibrated threshold value @ref sl_si91x_bod_get_threshold (Optional)
  * 6. Enable BOD interrupt: @ref sl_si91x_bod_enable_interrupt
  * 7. Monitor battery status: @ref sl_si91x_bod_get_battery_status
  * 8. Clear BOD interrupt: @ref sl_si91x_bod_clear_interrupt
  * 9. Deinitialize the BOD module: @ref sl_si91x_bod_deinit
  *
  * @section BOD_APIs APIs
  *
  * The following APIs are available for configuring and using the BOD module:
  *
  * - **Initialization and Deinitialization**:
  *   - @ref sl_si91x_bod_init
  *   - @ref sl_si91x_bod_deinit
  *
  * - **Configuration**:
  *   - @ref sl_si91x_bod_set_mode
  *   - @ref sl_si91x_bod_config_slot_value
  *   - @ref sl_si91x_bod_set_threshold
  *   - @ref sl_si91x_bod_set_configuration
  *
  * - **Interrupt Handling**:
  *   - @ref sl_si91x_bod_enable_interrupt
  *   - @ref sl_si91x_bod_disable_interrupt
  *   - @ref sl_si91x_bod_clear_interrupt
  *   - @ref sl_si91x_bod_register_callback
  *   - @ref sl_si91x_bod_unregister_callback
  *
  * - **Battery Monitoring**:
  *   - @ref sl_si91x_bod_get_battery_status
  *   - @ref sl_si91x_bod_battery_percentage
  *
  * - **Blackout Reset**:
  *   - @ref sl_si91x_bod_enable_blackout_in_sleep_mode
  *   - @ref sl_si91x_bod_disable_blackout_in_sleep_mode
  *   - @ref sl_si91x_bod_black_out_reset
  *
  * - **Button Configuration**:
  *   - @ref sl_si91x_bod_button_configuration
  *   - @ref sl_si91x_bod_button_wakeup_enable_v2
  *   - @ref sl_si91x_bod_button_interrupt_clear
  *
  * @section BOD_Example Example
  *
  * Below is an example of how to configure and use the BOD module:
  *
  * ```c
  * #include "sl_si91x_bod.h"
  *
  * void bod_callback(void) {
  *   // Handle BOD interrupt
  * }
  *
  * int main(void) {
  *   sl_status_t status;
  *   sl_bod_uc_param_t bod_config = {
  *     .slot_value = 2,
  *     .blackout_en = SL_ENABLE,
  *   };
  *
  *   // Initialize the BOD module
  *   status = sl_si91x_bod_init();
  *   if (status != SL_STATUS_OK) {
  *     // Handle initialization error
  *   }
  *
  *   // Configure the slot value
  *   status = sl_si91x_bod_config_slot_value(2);
  *   if (status != SL_STATUS_OK) {
  *     // Handle slot value configuration error
  *   }
  *
  *   // Set the voltage threshold
  *   status = sl_si91x_bod_set_threshold(2.5f);
  *   if (status != SL_STATUS_OK) {
  *     // Handle threshold setting error
  *   }
  *
  *   // Register a callback for BOD interrupts
  *   status = sl_si91x_bod_register_callback(bod_callback);
  *   if (status != SL_STATUS_OK) {
  *     // Handle callback registration error
  *   }
  *
  *   // Enable BOD interrupt
  *   sl_si91x_bod_enable_interrupt();
  *
  *   while (1) {
  *     // Main loop
  *   }
  *
  *   return 0;
  * }
  * ```
  *
  * @section BOD_Notes Notes
  *
  * - Ensure that the BOD module is properly initialized before using any other BOD APIs.
  * - The voltage threshold must be within the range defined by @ref SL_BOD_MIN_THRESHOLD and @ref SL_BOD_MAX_THRESHOLD.
  * - Use the blackout reset functionality to protect the system from unexpected power failures.
  *
  *
  * @} end group BOD
  */
#endif /* __SL_SI91X_BOD_H__ */
