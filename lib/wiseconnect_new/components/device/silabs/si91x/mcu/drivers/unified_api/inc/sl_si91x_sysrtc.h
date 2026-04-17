/******************************************************************************
* @file sl_si91x_sysrtc.h
* @brief Sysrtc API implementation
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

#ifndef SL_SI91X_SYSRTC_H
#define SL_SI91X_SYSRTC_H

#ifdef __cplusplus
extern "C" {
#endif

//// Includes
#include "sl_status.h"
#include "rsi_sysrtc_headers.h"
#include "rsi_sysrtc.h"
#include "sl_gpio_board.h"
#include "sl_si91x_driver_gpio.h"
/***************************************************************************/
/**
 * @addtogroup SYSRTC System RTC
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 *
 ******************************************************************************/
// -----------------------------------------------------------------------------
// Macros for SYSRTC parameters
// -----------------------------------------------------------------------------
// Data Types

typedef rsi_sysrtc_clk_inp_t sl_clock_sources_t;            ///< clock sources enum
typedef rsi_sysrtc_config_t sl_sysrtc_config_t;             ///< SYSRTC config structure
typedef rsi_sysrtc_group_config_t sl_sysrtc_group_config_t; ///< SYSRTC group config structure
typedef rsi_sysrtc_group_channel_compare_config_t
  sl_sysrtc_group_compare_channel_action_config_t; ///< renaming group compare channel config structure
typedef rsi_sysrtc_group_channel_capture_config_t
  sl_sysrtc_group_capture_channel_input_edge_config_t; ///< renaming group capture channel config structure
/// Rising edge value for capture channel configuration structure.
#define SYSRTC_GROUP_CHANNEL_CAPTURE_CONFIG_RISE_EDGE \
  {                                                   \
    RSI_SYSRTC_CAPTURE_EDGE_RISING                    \
  }
///  Falling edge value for capture channel configuration structure.
#define SYSRTC_GROUP_CHANNEL_CAPTURE_CONFIG_FALL_EDGE \
  {                                                   \
    RSI_SYSRTC_CAPTURE_EDGE_FALLING                   \
  }
/// both rise and fall edge for capture channel configuration structure.
#define SYSRTC_GROUP_CHANNEL_CAPTURE_CONFIG_BOTH_EDGE \
  {                                                   \
    RSI_SYSRTC_CAPTURE_EDGE_BOTH                      \
  }
/// clear value for compare channel configuration structure.
#define SYSRTC_GROUP_CHANNEL_COMPARE_CONFIG_CLEAR \
  {                                               \
    RSI_SYSRTC_COMPARE_MATCH_OUT_ACTION_CLEAR     \
  }
/// Set value for compare channel configuration structure.
#define SYSRTC_GROUP_CHANNEL_COMPARE_CONFIG_SET \
  {                                             \
    RSI_SYSRTC_COMPARE_MATCH_OUT_ACTION_SET     \
  }
/// pulse value  for compare channel configuration structure.
#define SYSRTC_GROUP_CHANNEL_COMPARE_CONFIG_PULSE \
  {                                               \
    RSI_SYSRTC_COMPARE_MATCH_OUT_ACTION_PULSE     \
  }
/// toggle value for compare channel configuration structure.
#define SYSRTC_GROUP_CHANNEL_COMPARE_CONFIG_TOGGLE \
  {                                                \
    RSI_SYSRTC_COMPARE_MATCH_OUT_ACTION_TOGGLE     \
  }
/// Export CMPIF value for compare channel configuration structure.
#define SYSRTC_GROUP_CHANNEL_COMPARE_CONFIG_CMPIF \
  {                                               \
    RSI_SYSRTC_COMPARE_MATCH_OUT_ACTION_CMPIF     \
  }

/***************************************************************************/
/**
 * @brief Typedef for the SYSRTC callback function.
 * 
 * @details This typedef defines the function signature for the callback function used by the SYSRTC.
 *          The callback function is called when a specific SYSRTC event occurs.
 * 
 * @param[in] callback_flag Pointer to a user-defined flag or data structure that can be used within the callback function.
 ******************************************************************************/
typedef void (*sl_sysrtc_callback_t)(void *callback_flag);

/***************************************************************************/
/**
 * @brief Enumeration to represent SYSRTC group numbers.
 *
 * @details This enumeration defines the group numbers for the SYSRTC module. It includes options for group 0, group 1, and a validation member.
 */
typedef enum {
  SL_SYSRTC_GROUP_0,    ///< Enumerator for SYSRTC group 0
  SL_SYSRTC_GROUP_1,    ///< Enumerator for SYSRTC group 1
  SL_SYSRTC_GROUP_LAST, ///< Last enumerator for validation
} sl_sysrtc_group_number_t;

/***************************************************************************/
/**
 * @brief Enumeration to represent SYSRTC channel numbers.
 *
 * @details This enumeration defines the channel numbers for the SYSRTC module. It includes options for channel 0, channel 1, and a validation member.
 */
typedef enum {
  SL_SYSRTC_CHANNEL_0,    ///< Enumerator for SYSRTC channel 0
  SL_SYSRTC_CHANNEL_1,    ///< Enumerator for SYSRTC channel 1
  SL_SYSRTC_CHANNEL_LAST, ///< Last enumerator for validation
} sl_sysrtc_channel_number_t;

/***************************************************************************/
/**
 * @brief Structure to hold the parameters of the SYSRTC clock-source configurations.
 *
 * @details This structure contains the configuration parameters for the SYSRTC clock source. It includes the clock source and the division factor.
 */
typedef struct {
  uint8_t clock_source;     ///< Input clock source from sl_clock_sources_t (clock sources enum)
  uint32_t division_factor; ///< Clock division factor
} sl_sysrtc_clock_config_t;

/***************************************************************************/
/**
 * @brief Structure to hold the parameters for SYSRTC group interrupt enabling bits.
 *
 * @details This structure contains the configuration parameters for enabling or disabling interrupts for SYSRTC groups.
 *          Each member represents whether a specific interrupt type is enabled (true) or disabled (false).
 */
typedef struct {
  boolean_t group0_overflow_interrupt_is_enabled; ///< true to enable group 0 overflow interrupt, false to disable it
  boolean_t group0_compare0_interrupt_is_enabled; ///< true to enable group 0 compare 0 interrupt, false to disable it
  boolean_t group0_compare1_interrupt_is_enabled; ///< true to enable group 0 compare 1 interrupt, false to disable it
  boolean_t group0_capture0_interrupt_is_enabled; ///< true to enable group 0 capture 0 interrupt, false to disable it
  boolean_t group1_overflow_interrupt_is_enabled; ///< true to enable group 1 overflow interrupt, false to disable it
  boolean_t group1_compare0_interrupt_is_enabled; ///< true to enable group 1 compare 0 interrupt, false to disable it
  boolean_t group1_compare1_interrupt_is_enabled; ///< true to enable group 1 compare 1 interrupt, false to disable it
  boolean_t group1_capture0_interrupt_is_enabled; ///< true to enable group 1 capture 0 interrupt, false to disable it
} sl_sysrtc_interrupt_enables_t;

/***************************************************************************/
/**
 * @brief Structure to hold the versions of the peripheral API.
 *
 * @details This structure contains the version information for the peripheral API. It includes the release version number, major version number, and minor version number.
 */
typedef struct {
  uint8_t release; ///< Release version number
  uint8_t major;   ///< Major version number
  uint8_t minor;   ///< Minor version number
} sl_sysrtc_version_t;
// -----------------------------------------------------------------------------
// Prototypes

/***************************************************************************/
/**
 * @brief To initialize SYSRTC and enable the peripheral.
 * 
 * @details This API initializes the SYSRTC and configures it to run during debug halt, based on user input.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_sysrtc_configure_clock must be called before this function.
 * 
 * @param[in] config_ptr Pointer to the SYSRTC configuration structure \ref sl_sysrtc_config_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, SYSRTC initialized properly.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_init(const sl_sysrtc_config_t *config_ptr);

/***************************************************************************/
/**
 * @brief To configure the SYSRTC input clock source frequency.
 * 
 * @details This API configures the clock source for the SYSRTC. It allows the selection of 32 KHz RC, 32 KHz RO, or 1 KHz RO clock sources based on user input.
 *          For using the 1kHz clock source, pass the division factor value as '16'; otherwise, pass '0'.
 * 
 * @param[in] clk_ptr Pointer to the clock configuration structure \ref sl_sysrtc_clock_config_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, timer clock-source parameters configured properly.
 *         - SL_STATUS_INVALID_PARAMETER  - Clock source parameter has an invalid value.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_configure_clock(sl_sysrtc_clock_config_t *clk_ptr);

/***************************************************************************/
/**
 * @brief To configure the SYSRTC groups of compare and capture channels.
 * 
 * @details Configures the specified group number and enables its compare and capture channels.
 *          Additionally, sets up match-out actions for the corresponding compare channels and input events for the capture channels.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_sysrtc_configure_clock must be called before this function.
 *      - \ref sl_si91x_sysrtc_init must be called before this function.
 * 
 * @param[in] group_number SYSRTC group number to be used.
 * @param[in] config_ptr Pointer to the group configuration structure \ref sl_sysrtc_group_config_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, parameters configured properly.
 *         - SL_STATUS_INVALID_PARAMETER  - Counter direction parameter has an invalid value.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_configure_group(sl_sysrtc_group_number_t group_number,
                                            sl_sysrtc_group_config_t const *config_ptr);

/***************************************************************************/
/**
 * @brief To register the callback for the timer interrupt.
 * 
 * @details Registers the callback for the timer interrupt and enables the corresponding interrupts based on the selected interrupt flags.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_sysrtc_init must be called before this function.
 *      - \ref sl_si91x_sysrtc_configure_clock must be called before this function.
 *      - \ref sl_si91x_sysrtc_configure_group() must be called with the respective interrupt channel enabled.
 *      - \ref sl_si91x_sysrtc_unregister_callback() must be called if already registered for any interrupt.
 * 
 * @param[in] on_sysrtc_callback Function pointer to the callback function to be called when a timer interrupt occurs.
 * @param[in] callback_flag_value Pointer to the interrupt flag value variable \ref sl_sysrtc_callback_t.
 * @param[in] group_number SYSRTC group number whose interrupts need to be enabled.
 * @param[in] interrupt_enable_ptr Pointer to the interrupt enable structure \ref sl_sysrtc_interrupt_enables_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully registered timer timeout callback.
 *         - SL_STATUS_BUSY  - The callback is already registered; unregister the previous callback before registering a new one.
 *         - SL_STATUS_INVALID_PARAMETER  - The group_number parameter has an invalid value.
 *         - SL_STATUS_NULL_POINTER  - The interrupt_enable_ptr parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_register_callback(sl_sysrtc_callback_t on_sysrtc_callback,
                                              void *callback_flag_value,
                                              sl_sysrtc_group_number_t group_number,
                                              sl_sysrtc_interrupt_enables_t *interrupt_enable_ptr);

/***************************************************************************/
/**
 * @brief To unregister the timer interrupt callback.
 * 
 * @details Unregisters the timer interrupt callback and disables interrupts according to the selected interrupt flag.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_sysrtc_register_callback() must be called first to register a particular interrupt flag.
 * 
 * @param[in] group_number SYSRTC group number whose interrupts need to be disabled.
 * @param[in] interrupt_enabled_handle Pointer to the interrupt enable structure \ref sl_sysrtc_interrupt_enables_t.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully unregistered timer interrupt callback.
 *         - SL_STATUS_INVALID_PARAMETER  - The group_number parameter has an invalid value.
 *         - SL_STATUS_NULL_POINTER  - The interrupt_enabled_handle parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_unregister_callback(sl_sysrtc_group_number_t group_number,
                                                sl_sysrtc_interrupt_enables_t *interrupt_enabled_handle);

/***************************************************************************/
/**
 * @brief To set the SYSRTC compare value for the selected channel of the given group.
 * 
 * @details This API sets the SYSRTC compare register value for the specified channel within the given group.
 * 
 * @pre Pre-conditions:
 *      - First enable the compare channel of the respective group through
 *        \ref sl_si91x_sysrtc_configure_group.
 * 
 * @param[in] group_number SYSRTC group number to use.
 * @param[in] channel Channel number to use.
 * @param[in] compare_value Compare register value.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, parameters configured properly.
 *         - SL_STATUS_INVALID_PARAMETER  - The group_number or channel parameter has an invalid value.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_set_compare_value(sl_sysrtc_group_number_t group_number,
                                              sl_sysrtc_channel_number_t channel,
                                              uint32_t compare_value);

/***************************************************************************/
/**
 * @brief To get the SYSRTC current compare register value for the selected channel of the selected group.
 * 
 * @details This API retrieves the current compare register value for the specified channel within the given group.
 * 
 * @param[in] group_number SYSRTC group number to use.
 * @param[in] channel Channel number to use.
 * @param[out] compare_value Pointer to the variable to store the compare value read.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, parameters configured properly.
 *         - SL_STATUS_INVALID_PARAMETER  - The group_number or channel parameter has an invalid value.
 *         - SL_STATUS_NULL_POINTER  - The compare_value parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_get_compare_value(sl_sysrtc_group_number_t group_number,
                                              sl_sysrtc_channel_number_t channel,
                                              uint32_t *compare_value);

/***************************************************************************/
/**
 * @brief To set register-input for the capture channel of the selected group of SYSRTC.
 * 
 * @details This API sets the register-input for the capture channel of the selected SYSRTC group, configuring the SYSRTC Input (Capture) from the register.
 * 
 * @pre Pre-conditions:
 *      - First, enable and configure the capture channel of the respective group through
 *        \ref sl_si91x_sysrtc_configure_group.
 *      - Disable GPIO input through \ref sl_si91x_sysrtc_enable_input_output_gpio by
 *        passing false.
 * 
 * @param[in] group_number SYSRTC group number to use.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, parameters configured properly.
 *         - SL_STATUS_INVALID_PARAMETER  - The group_number parameter has an invalid value.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_sets_register_capture_input(sl_sysrtc_group_number_t group_number);

/***************************************************************************/
/**
 * @brief To configure input pins for the SYSRTC capture channel of the selected group.
 * 
 * @details Configures GPIO-0 and GPIO-1 as input pins for the capture channel of group 0 and group 1, respectively.
 *          Sets pins mode, mux, and direction.
 * 
 * @pre Pre-conditions:
 *      - First, enable and configure the capture channel of the respective group through
 *        \ref sl_si91x_sysrtc_configure_group.
 *      - Enable GPIO IO through \ref sl_si91x_sysrtc_enable_input_output_gpio by
 *        passing true.
 * 
 * @param[in] group_number SYSRTC group number to use.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, parameters configured properly.
 *         - SL_STATUS_INVALID_PARAMETER  - The group_number parameter has an invalid value.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_set_gpio_as_capture_input(sl_sysrtc_group_number_t group_number);

/***************************************************************************/
/**
 * @brief To configure the output pins of the selected compare channel of the given group.
 * 
 * @details Configures GPIO-3 and GPIO-4 as output pins for compare channels 0 and 1 of group 0, respectively.
 *          Configures GPIO-1 and GPIO-2 as output pins for compare channels 0 and 1 of group 1, respectively.
 *          Sets pins mode, mux, and direction, and enables the input buffer for the pins.
 * 
 * @pre Pre-conditions:
 *      - First, enable and configure the output action of the compare channel of the respective group through
 *        \ref sl_si91x_sysrtc_configure_group.
 * 
 * @param[in] group_number SYSRTC group number to use.
 * @param[in] channel Channel number to use.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, parameters configured properly.
 *         - SL_STATUS_INVALID_PARAMETER  - The group_number or channel parameter has an invalid value.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_set_compare_output_gpio(sl_sysrtc_group_number_t group_number,
                                                    sl_sysrtc_channel_number_t channel);

/***************************************************************************/
/**
 * @brief To configure the PRS input pins for the SYSRTC capture channel of the selected group.
 * 
 * @details Configures GPIO-0 and GPIO-1 as input pins for the capture channel of group 0 and group 1, respectively.
 *          Sets pin mode, mux, and direction, and enables the input buffer for the pins.
 * 
 * @pre Pre-conditions:
 *      - Enable and configure the capture channel of the respective group through
 *        \ref sl_si91x_sysrtc_configure_group.
 *      - Enable GPIO IO through \ref sl_si91x_sysrtc_enable_input_output_gpio by
 *        passing true.
 * 
 * @param[in] group_number SYSRTC group number to use.
 * @param[in] sysrtc_prs_gpio - Structure containing the pin configuration information.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, parameters configured properly.
 *         - SL_STATUS_INVALID_PARAMETER  - The group_number parameter has an invalid value.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_set_capture_input_prs_gpio(sl_sysrtc_group_number_t group_number,
                                                       sl_si91x_gpio_pin_config_t sysrtc_prs_gpio);
/***************************************************************************/
/**
 * @brief To configure the PRS output pins of the selected compare channel of the given group.
 * 
 * @details Configures GPIO-3 and GPIO-0 as output pins for compare channels 0 and 1 of group 0, respectively.
 *          Configures GPIO-1 and GPIO-2 as output pins for compare channels 0 and 1 of group 1, respectively.
 *          Sets pin mode, mux, and direction.
 * 
 * @pre Pre-conditions:
 *      -  Enable and configure the output action of the compare channel of the respective group through
 *        \ref sl_si91x_sysrtc_configure_group.
 * 
 * @param[in] group_number SYSRTC group number to use.
 * @param[in] channel Channel number to use.
 * @param[in] sysrtc_prs_gpio - Structure containing the pin configuration information.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, parameters configured properly.
 *         - SL_STATUS_INVALID_PARAMETER  - The group_number or channel parameter has an invalid value.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_set_compare_output_prs_gpio(sl_sysrtc_group_number_t group_number,
                                                        sl_sysrtc_channel_number_t channel,
                                                        sl_si91x_gpio_pin_config_t sysrtc_prs_gpio);
/***************************************************************************/
/**
 * @brief To get the SYSRTC current counter register value.
 * 
 * @details Updates the current count of the timer to the count_value input parameter.
 * 
 * @param[in] count_value Pointer to the variable to store the count value read.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, count value retrieved properly.
 *         - SL_STATUS_NULL_POINTER  - The count_value parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_get_count(uint32_t *count_value);

/***************************************************************************/
/**
 * @brief To get the SYSRTC capture register value of a given group.
 * 
 * @details Updates the capture count value of the timer to the capture_value input parameter.
 * 
 * @param[in] group_number SYSRTC group number to use.
 * @param[out] capture_value Pointer to the variable to store the capture value read.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, parameters configured properly.
 *         - SL_STATUS_INVALID_PARAMETER  - The group_number parameter has an invalid value.
 *         - SL_STATUS_NULL_POINTER  - The capture_value parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_get_capture_value(sl_sysrtc_group_number_t group_number, uint32_t *capture_value);

/***************************************************************************/
/**
 * @brief To read the SYSRTC compare channel output bit value of a given group.
 * 
 * @details Updates the bit value to the compare_output_bit input parameter.
 * 
 * @param[in] group_number SYSRTC group number to use.
 * @param[in] channel Channel number to use.
 * @param[out] compare_output_bit Pointer to the variable to store the compare output.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, parameters configured properly.
 *         - SL_STATUS_INVALID_PARAMETER  - The group_number or channel parameter has an invalid value.
 *         - SL_STATUS_NULL_POINTER  - The compare_output_bit parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_get_compare_output(sl_sysrtc_group_number_t group_number,
                                               sl_sysrtc_channel_number_t channel,
                                               uint32_t *compare_output_bit);

/***************************************************************************/
/**
 * @brief To get the SYSRTC running status.
 * 
 * @details Updates the running_status input parameter to 'true' if SYSRTC is running; otherwise, it updates the parameter to 'false'.
 * 
 * @param[in] running_status Pointer to the variable to store the SYSRTC running status.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, parameters configured properly.
 *         - SL_STATUS_NULL_POINTER  - The running_status parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_is_running(boolean_t *running_status);

/***************************************************************************/
/**
 * @brief To get the SYSRTC lock status.
 * 
 * @details Updates the lock_status input parameter to 'true' if SYSRTC is locked or to 'false' if it is unlocked.
 * 
 * @param[in] lock_status Pointer to the variable to store SYSRTC lock status.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success, parameters configured properly.
 *         - SL_STATUS_NULL_POINTER  - The lock_status parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sysrtc_is_locked(boolean_t *lock_status);

/***************************************************************************/
/**
 * @brief To set the SYSRTC input/output mode for the capture and compare channels.
 * 
 * @details If "is_gpio_enabled" is set to "true", the input to capture and output from compare
 *          channels will be through GPIO. If set to "false", the input to capture and output from compare
 *          channels can be set through registers.
 * 
 * @param[in] is_gpio_enabled Boolean to enable or disable IO through GPIO.
 ******************************************************************************/
void sl_si91x_sysrtc_enable_input_output_gpio(bool is_gpio_enabled);

/***************************************************************************/
/**
 * @brief To start the SYSRTC counter.
 * 
 * @details This API sends a start command to the SYSRTC peripheral and waits until the command has been executed.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_sysrtc_init must be called.
 *      - \ref sl_si91x_sysrtc_configure_clock() must be called with software trigger disabled.
 *      - \ref sl_si91x_sysrtc_configure_group() must be called with the respective interrupt channel enabled.
 *      - \ref sl_si91x_sysrtc_register_callback must be called with the respective interrupt enabled.
 *      - \ref sl_si91x_sysrtc_set_count must be called.
 * 
 * @note This function will send a start command to the SYSRTC peripheral. 
 * @note The SYSRTC peripheral will use some LF clock ticks before the command is executed.
 * @note The rsi_sysrtc_wait_sync() function is used to wait for the start command to be executed.
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_sysrtc_start(void)
{
  rsi_sysrtc_start();
}

/***************************************************************************/
/**
 * @brief To stop the SYSRTC counter.
 * 
 * @details This function sends a stop command to the SYSRTC peripheral and waits until the command is executed.
 * 
 * @note This function requires the SYSRTC to be enabled.
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_sysrtc_stop(void)
{
  rsi_sysrtc_stop();
}

/***************************************************************************/
/**
 * @brief To reset the SYSRTC to its default state.
 * 
 * @details This function restores the SYSRTC peripheral to its reset state.
 * 
 * @note This function requires the SYSRTC to be enabled.
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_sysrtc_reset(void)
{
  rsi_sysrtc_reset();
}

/***************************************************************************/
/**
 * @brief To lock the SYSRTC registers.
 * 
 * @details When SYSRTC registers are locked, the SYSRTC_EN, SYSRTC_CFG, SYSRTC_CMD,
 *          SYSRTC_SWRST, SYSRTC_CNT, and SYSRTC_TOPCNT registers cannot be written to.
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_sysrtc_lock(void)
{
  rsi_sysrtc_lock();
}

/***************************************************************************/
/**
 * @brief To unlock the SYSRTC registers.
 * 
 * @details When SYSRTC registers are locked, the SYSRTC_EN, SYSRTC_CFG, SYSRTC_CMD,
 *          SYSRTC_SWRST, SYSRTC_CNT, and SYSRTC_TOPCNT registers cannot be written to.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_sysrtc_lock must be called before this function.
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_sysrtc_unlock(void)
{
  rsi_sysrtc_unlock();
}

/***************************************************************************/
/**
 * @brief To set the SYSRTC counter register value.
 * 
 * @details This API is used to change the count of SYSRTC when it is stopped.
 *          After calling this API, the user should start SYSRTC.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_sysrtc_stop must be called before this function.
 * 
 * @param[in] value The new SYSRTC counter value.
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_sysrtc_set_count(uint32_t value)
{
  rsi_sysrtc_set_counter(value);
}

/***************************************************************************/
/**
 * @brief To get the SYSRTC version.
 * 
 * @details Retrieve the API version of SYSRTC.
 * 
 * @return sl_sysrtc_version_t Structure containing the version information.
 ******************************************************************************/
sl_sysrtc_version_t sl_si91x_sysrtc_get_version(void);

/***************************************************************************/
/**
 * @brief To de-initialize the SYSRTC by disabling its clock.
 * 
 * @details This API de-initializes the SYSRTC by disabling its clock.
 *          Unregisters the SYSRTC callback and disables all interrupts.
 ******************************************************************************/
void sl_si91x_sysrtc_deinit(void);

/** @} end group SYSRTC */

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/** @addtogroup SYSRTC System RTC
* @{
*
* @details
*
* @n @section SYSRTC_Intro Introduction
* The System Real-Time Clock (SYSRTC) offers precise timekeeping and timer
* functionality essential for various applications. It is fundamental for tasks that require accurate time measurement,
* event scheduling, and power management. This SYSRTC driver provides a comprehensive set of functions to configure 
* and utilize peripherals effectively, ensuring optimal performance and reliability in time-critical applications.
*
* @section SYSRTC_Config Configuration
* The configuration parameters of the SYSRTC function include setting the clock frequency and initializing and enabling the peripherals:
* - Setting the clock source frequency: Choose between 32kHz RC or 1kHz RO clock sources using the @ref sl_si91x_sysrtc_configure_clock function.
* - Initialization: Initialize and enable the SYSRTC peripheral with the @ref sl_si91x_sysrtc_init function.
* - Additionally, configure the SYSRTC to continue running during a debug halt by specifying the appropriate setting in the initialization function.
*
* @image html sysrtc_blockdiagram.png "SYSRTC block diagram"
*
* - For more information on configuring available parameters, see the respective peripheral example readme document.
*
* @section SYSRTC_Usage Usage
*
* The SYSRTC has to be initialized by calling the following parameters:
* 1. Configure the SYSRTC input clock frequency by using the @ref sl_si91x_sysrtc_configure_clock function.
* 2. Initialize the SYSRTC and enable the peripherals by calling the @ref sl_si91x_sysrtc_init function.
* 3. Once configured, the SYSRTC can be customized to specific application requirements by using the following functions:
*  - Configure the group number and enable its compare and capture channels by using the @ref sl_si91x_sysrtc_configure_group function.
*  - To handle timer interrupts and execute actions accordingly, use the @ref sl_si91x_sysrtc_register_callback function to register callback functions.
* 4. Further functions allow for fine-grained control over time-sensitive operations:
*  - Setting compare values: Use the @ref sl_si91x_sysrtc_set_compare_value function.
*  - Retrieving the current counter value: Use the @ref sl_si91x_sysrtc_get_count function.
*  - Start the timer: Enable the timer with the @ref sl_si91x_sysrtc_start function.
*  - Stop the timer: Disable the timer using the @ref sl_si91x_sysrtc_stop function.
*  - Lock SYSRTC registers: Secure the SYSRTC registers with the @ref sl_si91x_sysrtc_lock function.
*
* The following functions will initiate and configure the SYSRTC, representing the general flow for implementation:
* 1. Configure the SYSRTC input clock frequency by using the @ref sl_si91x_sysrtc_configure_clock function.
* 2. Initialize the SYSRTC and enable the peripherals by calling the @ref sl_si91x_sysrtc_init function.
* 3. Configure the SYSRTC group number and enable its compare and capture channels by using the @ref sl_si91x_sysrtc_configure_group function.
* 4. Register the callback for the timer interrupt by using the @ref sl_si91x_sysrtc_register_callback function.
* 5. Set the SYSRTC counter register value with the @ref sl_si91x_sysrtc_set_count function.
* 6. Set the SYSRTC compare value for the selected channel of the group by using the @ref sl_si91x_sysrtc_set_compare_value function, if the compare channel is enabled.
* 7. Get the SYSRTC running status with the @ref sl_si91x_sysrtc_is_running function, if required to check the timer running status.
* 8. Start the SYSRTC counter by using the @ref sl_si91x_sysrtc_start function.
* 9. Get the SYSRTC current counter register value by using the @ref sl_si91x_sysrtc_get_count function if needed.
* 10. Stop the SYSRTC counter by using the @ref sl_si91x_sysrtc_stop function.
* 11. De-initialize the SYSRTC by disabling its clock by calling the @ref sl_si91x_sysrtc_deinit function.
*
*
* @} end group SYSRTC ********************************************************/

#ifdef __cplusplus
}
#endif

#endif /*SL_SI917_SYSRTC*/
