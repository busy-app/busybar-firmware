/***************************************************************************/ /**
* @file sl_si91x_button.h
* @brief Button API documentation.
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

#ifndef SL_SI91X_BUTTON_H
#define SL_SI91X_BUTTON_H

#include <stdint.h>
#include "sl_si91x_peripheral_gpio.h"
#include "sl_status.h"
#include "base_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
#ifndef UNUSED_VARIABLE
#define UNUSED_VARIABLE(x) (void)(x)
#endif // UNUSED_VARIABLE
/** @endcond */

/**
 * @addtogroup BUTTON Button
 * @ingroup SI91X_HARDWARE_DRIVER_APIS
 * @{
 */

/**
 * @name Macros
 * @brief A set of numerical macros for use with the button APIs.
 * @{
 */

/*******************************************************************************
 ******************************   DEFINES   ************************************
 ******************************************************************************/

/**
 * @brief To configure or to trigger the interrupt based on logic levels (LOW, HIGH, or both) or edge changes (rising, falling, or both).
 */
#define HIGH_LEVEL_INTERRUPT SL_GPIO_INTERRUPT_HIGH ///< Interrupt on level high/released button state.
#define LOW_LEVEL_INTERRUPT  SL_GPIO_INTERRUPT_LOW  ///< Interrupt on level low/pressed button state can be configured.
#define HIGH_LEVEL_AND_LOW_LEVEL_INTERRUPT \
  (HIGH_LEVEL_INTERRUPT                    \
   | LOW_LEVEL_INTERRUPT) ///< Interrupt on level low/pressed and high/released button state can be configured.
#define RISE_EDGE_INTERRUPT SL_GPIO_INTERRUPT_RISING_EDGE  ///< Interrupt on level rising edge of the button press.
#define FALL_EDGE_INTERRUPT SL_GPIO_INTERRUPT_FALLING_EDGE ///< Interrupt on level falling edge of the button press.
#define RISE_EDGE_AND_FALL_EDGE_INTERRUPT \
  SL_GPIO_INTERRUPT_RISE_FALL_EDGE ///< Interrupt on level rising edge and falling edge of the button press.
#define BUTTON_PRESSED       1     ///< Button state is pressed.
#define BUTTON_RELEASED      0     ///< Button state is released.
#define BUTTON_STATE_INVALID -1    ///< Button state is invalid.

/** @} END Macros */

/**
 * @brief Structure representing a button configuration.
 */
typedef struct {
  uint8_t pin;              ///< Pin number of the button.
  uint8_t port;             ///< Port number of the button.
  uint8_t button_number;    ///< Button number (for identification).
  uint8_t pad;              ///< Pad byte (for alignment).
  uint8_t interrupt_config; ///< Interrupt configuration for the button.
} sl_button_t;

/***************************************************************************/
/**
 * @brief To initialize the specified button with the given configuration.
 * 
 * @details This function configures the GPIO settings and interrupt configurations for the specified button.
 *          It handles different configurations based on the board version and button number.
 *          For UULP (Ultra-Low Power) and M4 (High-Performance) GPIOs, it sets the pin direction, pin mode, and interrupt settings.
 * 
 * @param[in] handle Pointer to an @ref sl_button_t button configuration structure containing the specific button details.
 * 
 * @pre The button configuration structure must be properly initialized before calling this function.
 * @post The specified button will be configured and ready for use with the specified settings.
 * 
 * @note This function handles different configurations based on the board version and button number.
 *       For UULP (Ultra-Low Power) and M4 (High-Performance) GPIOs, it sets the pin direction, pin mode, and interrupt settings.
 *****************************************************************************/
void sl_si91x_button_init(const sl_button_t *handle);

/***************************************************************************/
/**
 * @brief Deinitialize the specified button and release associated resources.
 * 
 * @details This function deinitializes the specified button.
 *          After deinitialization, the button is no longer functional until reinitialized.
 * 
 * @param[in] handle Pointer to an \ref sl_button_t structure containing the button configuration details.
 *  
 * @return sl_status_t Status of the operation:
 *         - SL_STATUS_OK if the button was successfully deinitialized.
 *         - SL_STATUS_INVALID_HANDLE if the button number is invalid.
 * 
 * @pre The button must have been previously initialized with \ref sl_si91x_button_init.
 * 
 * @note This function handles deinitialization for HP (High-Performance), ULP and UULP buttons.
 *****************************************************************************/
sl_status_t sl_si91x_button_deinit(const sl_button_t *handle);

/***************************************************************************/
/**
 * @brief To handle the interrupt when a button changes its state.
 *
 * @details This callback is called in the interrupt context whenever a button changes its state.
 *          It must be implemented by the application to handle the functionality in response to
 *          the button state change.
 *
 * @note This function should contain the functionality to be executed in response to changes of state
 *       in each of the buttons or callbacks to the appropriate functionality.
 *
 * @param[in] pin The button pin which has changed state, either BUTTON0 pin or BUTTON1 pin as defined.
 *
 * @param[in] state The new state of the button referenced by the button parameter,
 *                  either ::BUTTON_PRESSED if the button has been pressed or ::BUTTON_RELEASED if
 *                  the button has been released.
 *****************************************************************************/
void sl_si91x_button_isr(uint8_t pin, int8_t state);

/***************************************************************************/
/**
 * @brief To get the current state (pressed or released) of a button.
 *
 * @details This function returns the shadow state of the button rather than reading the actual state of the pin.
 *          It is correlated with the interrupts and their callbacks, providing a "soft" state.
 *
 * @param[in] button_number The button number being queried.
 *
 * @return int8_t The state of the button:
 *                - ::BUTTON_PRESSED if the button is pressed.
 *                - ::BUTTON_RELEASED if the button is not pressed.
 *                - ::BUTTON_STATE_INVALID if the pin does not match any known button.
 *****************************************************************************/
uint8_t sl_si91x_button_get_state(uint8_t button_number);

/***************************************************************************/
/**
 * @brief To return the current state (pressed or released) of the pin associated with a button.
 *
 * @details This function reads the actual state of the pin and can be used on startup to
 *          determine the initial position of the buttons.
 *
 * @param[in] pin The button pin being queried
 * @param[in] port The button port being queried
 *
 * @return int8_t The state of the button:
 *                - ::BUTTON_PRESSED if the button is pressed.
 *                - ::BUTTON_RELEASED if the button is not pressed.
 *****************************************************************************/
uint8_t sl_si91x_button_state(uint8_t pin, uint8_t port);

/***************************************************************************/
/**
 * @brief To toggle the value of a local variable associated with the state of a button.
 *
 * @details This function toggles the state of a local variable that tracks the state of the specified button.
 *          It must be implemented by the application to handle the functionality in response to the button state change.
 *
 * @note This function should contain the functionality to be executed in response to changes of state
 *       in each of the buttons or callbacks to the appropriate functionality.
 *
 * @param[in] button_number The button number being queried.
 *****************************************************************************/
void sl_si91x_button_toggle_state(uint8_t button_number);

/***************************************************************************/
/**
 * @brief To set the state (pressed or released) of the pin associated with a button.
 *
 * @details This function sets the state of the specified button pin. It must be implemented by the application
 *          to handle the functionality in response to changes of state in each of the buttons or callbacks to the appropriate functionality.
 *
 * @note This function should contain the functionality to be executed in response to changes of state
 *       in each of the buttons or callbacks to the appropriate functionality.
 *
 * @param[in] button_number The button number being queried.
 * @param[in] state The state of the button to be set:
 *                  - ::BUTTON_PRESSED if the button state is pressed.
 *                  - ::BUTTON_RELEASED if the button state is released.
 *****************************************************************************/
void sl_si91x_button_set_state(uint8_t button_number, int8_t state);

/***************************************************************************/
/**
 * @brief Older API - Not recommended to use.
 * 
 * @details This function returns the shadow state of the button rather than reading the actual state of the pin.
 *          It is correlated with the interrupts and their callbacks, providing a "soft" state.
 *
 * @param[in] pin The button pin being queried, either SL_BUTTON_BTN0_PIN or SL_BUTTON_BTN1_PIN as defined.
 *
 * @return int8_t The state of the button:
 *                - ::BUTTON_PRESSED if the button is pressed.
 *                - ::BUTTON_RELEASED if the button is not pressed.
 *                - ::BUTTON_STATE_INVALID if the pin does not match any known button.
 *
 * @note This function is deprecated and should be replaced with `sl_si91x_button_get_state`.
 *****************************************************************************/
int8_t sl_si91x_button_state_get(uint8_t pin) SL_DEPRECATED_API_WISECONNECT_4_0;

/***************************************************************************/
/**
 * @brief Older API - Not recommended to use.
 *
 * @details This function reads the actual state of the pin and can be used on startup to
 *          determine the initial position of the buttons.
 *
 * @param[in] pin The button pin being queried, either BUTTON0 pin or BUTTON1 pin as defined.
 *
 * @return int8_t The state of the button:
 *                - ::BUTTON_PRESSED if the button is pressed.
 *                - ::BUTTON_RELEASED if the button is not pressed.
 *
 * @note This function is deprecated and should be replaced with `sl_si91x_button_state`.
 *****************************************************************************/
int8_t sl_si91x_button_pin_state(uint8_t pin) SL_DEPRECATED_API_WISECONNECT_4_0;

/***************************************************************************/
/**
 * @brief Older API - Not recommended to use.
 *
 * @details This function toggles the state of a local variable that tracks the state of the specified button.
 *          It must be implemented by the application to handle the functionality in response to the button state change.
 *
 * @note This function should contain the functionality to be executed in response to changes of state
 *       in each of the buttons or callbacks to the appropriate functionality.
 *
 * @param[in] pin The button pin which has changed state, either BUTTON0 pin or BUTTON1 pin as defined.
 *
 * @note This function is deprecated and should be replaced with `sl_si91x_button_toggle_state`.
 *****************************************************************************/
void sl_si91x_button_state_toggle(uint8_t pin) SL_DEPRECATED_API_WISECONNECT_4_0;

/***************************************************************************/
/**
 * @brief Older API - Not recommended to use.
 *
 * @details This function sets the state of the specified button pin. It must be implemented by the application
 *          to handle the functionality in response to changes of state in each of the buttons or callbacks to the appropriate functionality.
 *
 * @note This function should contain the functionality to be executed in response to changes of state
 *       in each of the buttons or callbacks to the appropriate functionality.
 *
 * @param[in] pin The button pin which has changed state, either BUTTON0 pin or BUTTON1 pin as defined.
 * @param[in] state The state of the button to be set:
 *                  - ::BUTTON_PRESSED if the button state is pressed.
 *                  - ::BUTTON_RELEASED if the button state is released.
 *
 * @note This function is deprecated and should be replaced with `sl_si91x_button_set_state`.
 *****************************************************************************/
void sl_si91x_button_state_set(uint8_t pin, int8_t state) SL_DEPRECATED_API_WISECONNECT_4_0;

/** @} end group BUTTON */

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/** 
* @addtogroup BUTTON Button
* @{
*   @details
*    @section buttondrv_intro Introduction
*
*   The SI91x button driver is a platform-level software module designed to manage button initialization
*   and reading. It provides a wide range of functions for handling button operations, such as 
*   initialization, state monitoring, and callback management for state changes. The driver
*   supports flexible interrupt configurations, allowing detection based on button state, edge 
*   transitions, or a combination of both. All button functions are accessed via the generic driver,
*   which internally calls specific functions for the corresponding button.
*
*    @section buttondrv_config Configuration
*
*   All button instances are configured using the @ref sl_button_t struct. This struct along with a function definition for initializing Button is automatically generated when an Button is set up using Simplicity Studio's wizard.
*   This struct @ref sl_button_t struct is automatically generated into the following files sl_si91x_button_instances.h and sl_si91x_button_instances.c.
*   The samples below are for a single instance called "btn0".
*
* @code{.c}
* // sl_si91x_button_instances.c
* #include "sl_si91x_button_pin_config.h"
* #include "sl_si91x_button_instances.h"
* #include "sl_si91x_button_btn0_config.h"
* 
* sl_button_t const button_btn0 = {
*   .port = SL_BUTTON_BTN0_PORT,
*   .pin = SL_BUTTON_BTN0_PIN,
*   .button_number = SL_BUTTON_BTN0_NUMBER,
*   #ifdef SL_BUTTON_BTN0_PAD
*   .pad = SL_BUTTON_BTN0_PAD,
*   #endif
*   .interrupt_config = SL_BUTTON_CONFIG_BTN0_INTR
* };
* 
* void button_init_instances(void)
* {
*   sl_si91x_button_init(&button_btn0);
* }
* @endcode
* @note The sl_si91x_button_instances.c file is shown with only one instance, but if more were in use they would all appear in this .c file.
*  
* @code{.c}
* // sl_si91x_button_instances.h
* #ifndef SL_SI91X_BUTTON_INSTANCES_H
* #define SL_SI91X_BUTTON_INSTANCES_H
* 
* #include "sl_si91x_button.h"
* 
* extern const sl_button_t button_btn0;
* 
* 
* void button_init_instances(void);
* void sl_simple_button_init_instances(void);
* 
* #endif // BUTTON_INSTANCES_H
* @endcode
* 
* @note The sl_si91x_button_instances.h file is shown with only one instance, but if more were in use they would all appear in this .h file.
* 
* - For more information on configuring available parameters, see the respective peripheral example readme document and
*   [Peripheral Configuration](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/disable-uc-config).
*
*    @section buttondrv_usage Usage
*
*   After the button structures are defined, button functions can be called by passing an instance of 
*   sl_button_t, which will internally redirect to the type-specific version of the function.
*   The commonly used functions include the following:
*
*   1. To initialize the button: @ref sl_si91x_button_init 
*   2. To return the current state (pressed or released) of the pin associated with a button: @ref sl_si91x_button_state
*   3. To toggle the value of a local variable associated with the state of button: @ref sl_si91x_button_toggle_state
*   4. A callback that user can modify for the application: @ref sl_si91x_button_isr  
*
*   The @ref sl_si91x_button_init function is automatically invoked when the application starts.
*
*   The button driver supports interrupt mode, with or without debounce functionality.The application can
*   implement @ref sl_si91x_button_state to determine the current state of the button when needed.
*   Additionally, @ref sl_si91x_button_toggle_state can be implemented by the application to toggle the
*   "soft" state, enabling interrupts and their callbacks. The @ref sl_si91x_button_isr is a callback 
*   function triggered by a change in the button's state, and the application can implement this 
*   function to define the desired actions upon a state change.
*/
/** @} end group BUTTON */

#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_BUTTON_H
