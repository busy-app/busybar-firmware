/***************************************************************************/
/**
 * @file sl_si91x_rgb_led.h
 * @brief See @ref RGB_LED for documentation.
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
#ifndef __SL_SI91X_RGB_LED_H__
#define __SL_SI91X_RGB_LED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_si91x_rgb_led_config.h"
#include <stdint.h>

/** @addtogroup RGB_LED RGB LED
 *  @ingroup SI91X_HARDWARE_DRIVER_APIS
 *@{
 */

/**
 * @brief Structure representing a single LED configuration.
 *
 * This structure holds the configuration parameters for an individual LED,
 * including its pin number, port number, LED identifier, and pad.
 */
typedef struct {
  uint8_t pin;        /**< Pin number associated with the LED. */
  uint8_t port;       /**< Port number where the LED is connected. */
  uint8_t led_number; /**< Identifier for the LED. */
  uint8_t pad;        /**< Pad configuration for the LED. */
} sl_led_t;

/**
 * @brief Structure representing an RGB LED configuration.
 *
 * This structure contains pointers to the configurations of the red, green,
 * and blue LEDs that together form an RGB LED.
 */
typedef struct {
  sl_led_t *red;   /**< Pointer to the red LED configuration. */
  sl_led_t *green; /**< Pointer to the green LED configuration. */
  sl_led_t *blue;  /**< Pointer to the blue LED configuration. */
} sl_rgb_led_t;

/***************************************************************************/
/**
 * @brief Configures GPIOs pertaining to the control of RGB LEDs.
 * 
 * @details This function initializes the GPIOs required to control the RGB LEDs.
 * 
 * @param[in] handle
 * The pointer to the RGB LED structure that contains the specific LED information.
 * 
 * @note This function must be called before any other RGB LED control functions.
 ******************************************************************************/
void sl_si91x_simple_rgb_led_init(const sl_rgb_led_t *handle);

/***************************************************************************/
/**
 * @brief Turns on the RGB LED.
 * 
 * @details This function sets the GPIO pins connected to the specified RGB LED 
 * and starts the sleeptimer, turning it on.
 * 
 * @param[in] handle
 * The pointer to the RGB LED structure that contains the specific LED information.
 * 
 * @pre Pre-conditions:
 *      - The RGB LED structure must be properly initialized before calling this function.
 * 
 * @note Ensure that the GPIO pin is configured correctly before using this function.
 ******************************************************************************/
void sl_si91x_simple_rgb_led_on(const sl_rgb_led_t *handle);

/***************************************************************************/
/**
 * @brief Turns off (clears) the RGB LED.
 * 
 * @details This function clears the GPIO pins connected to the specified RGB LED 
 * and stops the sleeptimer, turning it off.
 * 
 * @param[in] handle
 * The pointer to the RGB LED structure that contains the specific RGB LED information.
 * 
 * @pre Pre-conditions:
 *      - The RGB LED structure must be properly initialized before calling this function.
 * 
 * @note Ensure that the GPIO pin is configured correctly before using this function.
 ******************************************************************************/
void sl_si91x_simple_rgb_led_off(const sl_rgb_led_t *handle);

/***************************************************************************/
/**
 * @brief Atomically wraps an XOR or similar operation for the RGB LED.
 * 
 * @details This function performs an atomic XOR operation on the RGB LED, toggling its state.
 * 
 * @param[in] handle
 * The pointer to the RGB LED structure that contains the specific LED information.
 * 
 * @pre Pre-conditions:
 *      - The RGB LED structure must be properly initialized before calling this function.
 * 
 * @note Ensure that the GPIO pin is configured correctly before using this function.
 ******************************************************************************/
void sl_si91x_simple_rgb_led_toggle(const sl_rgb_led_t *handle);

/***************************************************************************/
/**
 * @brief Displays the user-defined color hex code on the RGB LED.
 * 
 * @details This function sets the RGB LED to display the specified color hex code.
 * 
 * @param[in] handle
 * The pointer to the RGB LED structure that contains the specific LED information.
 * 
 * @param[in] rgb_colour
 * The color hex code to be displayed.
 * 
 * @pre Pre-conditions:
 *      - The RGB LED structure must be properly initialized before calling this function.
 * 
 * @note Ensure that the color hex code is valid and within the acceptable range.
 ******************************************************************************/
void sl_si91x_simple_rgb_led_set_colour(const sl_rgb_led_t *handle, int rgb_colour);

/***************************************************************************/
/**
 * @brief Returns the current state of the RGB LED.
 * 
 * @details This function retrieves the current state of the specified RGB LED.
 * 
 * @param[in] handle
 * The pointer to the RGB LED structure that contains the specific RGB LED information.
 * 
 * @pre Pre-conditions:
 *      - The RGB LED structure must be properly initialized before calling this function.
 * 
 * @return uint8_t
 * The current state of the RGB LED.
 * 
 * @note The returned state indicates whether the RGB LED is on or off.
 ******************************************************************************/
uint8_t sl_si91x_simple_rgb_led_get_current_state(const sl_rgb_led_t *handle);

/***************************************************************************/
/**
 * @brief Returns the current colour of the RGB LED.
 * 
 * @details This function retrieves the current color values of the specified RGB LED.
 * 
 * @param[in] handle
 * The pointer to the RGB LED structure that contains the specific RGB LED information.
 * 
 * @param[out] red
 * The pointer to the variable to store the red LED color value.
 * 
 * @param[out] green
 * The pointer to the variable to store the green LED color value.
 * 
 * @param[out] blue
 * The pointer to the variable to store the blue LED color value.
 * 
 * @pre Pre-conditions:
 *      - The RGB LED structure must be properly initialized before calling this function.
 * 
 * @note Ensure that the pointers for red, green, and blue are valid and not null.
 ******************************************************************************/
void sl_si91x_simple_rgb_led_get_colour(const sl_rgb_led_t *handle, uint16_t *red, uint16_t *green, uint16_t *blue);

/***************************************************************************/
/**
 * @brief Older API - Not recommended to use.
 * 
 * @details This API is deprecated in favor of the new `sl_si91x_simple_` series of APIs.
 * Consider using `sl_si91x_simple_rgb_led_init`.
 * 
 * @param[in] handle
 * The pointer to the RGB LED structure that contains the specific LED information.
 * 
 * @pre Pre-conditions:
 *      - The RGB LED structure must be properly initialized before calling this function.
 * 
 * @note This function is deprecated and should be replaced with `sl_si91x_simple_rgb_led_init`.
 ******************************************************************************/
void sl_si91x_rgb_led_init(const sl_led_t *handle);

/***************************************************************************/
/**
 * @brief Older API - Not recommended to use.
 * 
 * @details This API is deprecated in favor of the new `sl_si91x_simple_` series of APIs.
 * Consider using `sl_si91x_simple_rgb_led_on`.
 * 
 * @param[in] handle
 * The pointer to the RGB LED structure that contains the specific LED information.
 * 
 * @pre Pre-conditions:
 *      - The RGB LED structure must be properly initialized before calling this function.
 * 
 * @note This function is deprecated and should be replaced with `sl_si91x_simple_rgb_led_on`.
 ******************************************************************************/
void sl_si91x_rgb_led_on(const sl_led_t *handle);

/***************************************************************************/
/**
 * @brief Older API - Not recommended to use.
 * 
 * @details This API is deprecated in favor of the new `sl_si91x_simple_` series of APIs.
 * Consider using `sl_si91x_simple_rgb_led_off`.
 * 
 * @param[in] handle
 * The pointer to the RGB LED structure that contains the specific RGB LED information.
 * 
 * @pre Pre-conditions:
 *      - The RGB LED structure must be properly initialized before calling this function.
 * 
 * @note This function is deprecated and should be replaced with `sl_si91x_simple_rgb_led_off`.
 ******************************************************************************/
void sl_si91x_rgb_led_off(const sl_led_t *handle);

/***************************************************************************/
/**
 * @brief Older API - Not recommended to use.
 * 
 * @details This API is deprecated in favor of the new `sl_si91x_simple_` series of APIs.
 * Consider using `sl_si91x_simple_rgb_led_get_current_state`.
 * 
 * @param[in] handle
 * The pointer to the RGB LED structure that contains the specific LED information.
 * 
 * @pre Pre-conditions:
 *      - The RGB LED structure must be properly initialized before calling this function.
 * 
 * @return uint8_t
 * The current state of the RGB LED.
 * 
 * @note This function is deprecated and should be replaced with `sl_si91x_simple_rgb_led_get_current_state`.
 ******************************************************************************/
uint8_t sl_si91x_rgb_led_get_current_state(const sl_led_t *handle);

/***************************************************************************/
/**
 * @brief Older API - Not recommended to use.
 * 
 * @details This API is deprecated in favor of the new `sl_si91x_simple_` series of APIs.
 * Consider using `sl_si91x_simple_rgb_led_toggle`.
 * 
 * @param[in] handle
 * The pointer to the RGB LED structure that contains the specific LED information.
 * 
 * @pre Pre-conditions:
 *      - The RGB LED structure must be properly initialized before calling this function.
 * 
 * @note This function is deprecated and should be replaced with `sl_si91x_simple_rgb_led_toggle`.
 ******************************************************************************/
void sl_si91x_rgb_led_toggle(const sl_led_t *handle);

/// @} end group RGB_LED ********************************************************/

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/** @addtogroup RGB_LED RGB LED
 *  @{
 *
 *  @details
 *  The RGB LED driver provides a set of functions for controlling RGB LEDs on the SI91x platform.
 *  It allows initialization, turning on/off, toggling, setting colors, and reading the current state or color of the LED.
 *
 *  @section RGB_LED_Usage Usage
 *  - Initialize the RGB LED: @ref sl_si91x_simple_rgb_led_init
 *  - Turn the LED on/off: @ref sl_si91x_simple_rgb_led_on, @ref sl_si91x_simple_rgb_led_off
 *  - Toggle the LED: @ref sl_si91x_simple_rgb_led_toggle
 *  - Set the LED color: @ref sl_si91x_simple_rgb_led_set_colour
 *  - Get the current state or color: @ref sl_si91x_simple_rgb_led_get_current_state, @ref sl_si91x_simple_rgb_led_get_colour
 *
 *  @note Deprecated APIs are provided for backward compatibility but new designs should use the `sl_si91x_simple_` series of APIs.
 *
 *  See the function documentation above for detailed usage information.
 *
 *  @} (end addtogroup RGB_LED RGB LED)
 */

#ifdef __cplusplus
}
#endif

#endif /* __SL_SI91X_RGB_LED_H__ */
