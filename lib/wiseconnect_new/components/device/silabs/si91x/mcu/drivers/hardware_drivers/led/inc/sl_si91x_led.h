/***************************************************************************/
/**
 * @file sl_si91x_led.h
 * @brief See @ref LED for documentation.
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
#ifndef __SL_SI91X_LED_H__
#define __SL_SI91X_LED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "sl_si91x_led_instances.h"

/** 
 * @addtogroup LED LED
 * @ingroup SI91X_HARDWARE_DRIVER_APIS
 * @{
 */

/***************************************************************************/
/**
 * @brief  To configure GPIOs pertaining to the control of LEDs and initializes the selected LED GPIO and mode.
 * 
 * @details This API initializes the selected GPIOs, sets their operation modes, and configures them for controlling the LEDs.
 *          It ensures that the GPIOs are properly configured to drive the LEDs as per the specified settings in the handle.
 * 
 * @param[in] handle  The pointer to an sl_led_t structure that contains the specific LED information. 
 * 
 * @pre The GPIO peripheral must be enabled and clocked before calling this function.
 * @post The specified GPIO pin will be configured and set to the initial state as defined in the handle.
 *****************************************************************************/
void sl_si91x_led_init(const sl_led_t *handle);

/***************************************************************************/
/**
 * @brief To toggle the state of an LED connected to a specified GPIO pin.
 * 
 * @details This API toggles the state of an LED connected to a specified GPIO pin by performing an XOR operation.
 *          The function identifies the GPIO port corresponding to a given pin number for a specific development board/opn.
 * 
 * @param[in] pin GPIO pin connected to the LED to be toggled.
 * 
 * @pre The GPIO peripheral must be enabled and clocked before calling this function.
 * @post The state of the specified GPIO pin will be toggled, effectively turning the connected LED on or off.
 *****************************************************************************/
void sl_si91x_led_toggle(uint8_t pin);

/***************************************************************************/
/**
 * @brief To set the state of an LED connected to a specified GPIO pin to ON.
 * 
 * @details This API sets the specified GPIO pin to a high state, which turns on the LED connected to that pin.
 *          The function determines the GPIO port based on the pin number and the development kit being used.
 * 
 * @param[in] pin  GPIO pin connected to the LED to be turned on.
 * 
 * @pre The GPIO peripheral must be enabled and clocked before calling this function.
 * @post The specified GPIO pin will be set to the appropriate state to turn on the LED.
 *****************************************************************************/
void sl_si91x_led_set(uint8_t pin);

/***************************************************************************/
/**
 * @brief To clear the state of an LED connected to a specified GPIO pin to OFF.
 * 
 * @details This API sets the specified GPIO pin to a low state, which turns off the LED connected to that pin.
 *          The function determines the GPIO port based on the pin number and the development kit being used.
 * 
 * @param[in] pin  GPIO pin connected to the LED to be turned off.
 * 
 * @pre The GPIO peripheral must be enabled and clocked before calling this function.
 * @post The specified GPIO pin will be set to the appropriate state to turn off the LED.
 *****************************************************************************/
void sl_si91x_led_clear(uint8_t pin);

/***************************************************************************/
/**
 * @brief To indicate radio activity for both transmission and reception.
 * 
 * @details This function is called by the stack to indicate activity over the radio. It is called once with \c turnOn set to true to indicate the start of activity, and shortly thereafter with \c turnOn set to false to indicate the end of activity.
 *          Typically, this function changes the state of an LED to reflect the radio activity.
 * 
 * @param[in] turnOn  A boolean value indicating the radio activity state.
 * 
 * @pre The LED or other indicator should be properly configured before calling this function.
 * @post The state of the LED or other indicator will be updated based on the radio activity.
 *****************************************************************************/
void sl_si91x_led_StackIndicateActivity(bool turnOn);

/** @} end group LED */

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/** 
* @addtogroup LED LED
* @{
*
* @details
* 
*  @section led_intro Introduction
* 
* The SI91x LED driver is a platform-level software module designed to manage LED control.
* All LED functions are accessed through the generic driver, which then internally calls the LED-specific functions.
* 
* @section led_config Configuration
* 
* All LED instances are configured with an sl_led_t struct. This struct along with a function definition for initializing LED is automatically generated when an LED is set up using Simplicity Studio's wizard.
* This struct sl_led_t struct is automatically generated into the following files sl_si91x_led_instances.h and sl_si91x_led_instances.c.
* The samples below are for a single instance called "led0".
* 
* @code{.c}
* // sl_si91x_led_instances.c
*
* #include "sl_si91x_led.h"
* #include "sl_si91x_led_config.h"
* #include "sl_si91x_led_instances.h"
* sl_led_t const led_led0 = {
*  .port = SL_LED_LED0_PORT,
*  .pin = SL_LED_LED0_PIN,
*  .led_number = SL_LED_LED0_NUMBER,
* };
*
* void led_init_instances(void)
* {
*   sl_si91x_led_init(&led_led0);
* }
* @endcode
*
* @note The sl_si91x_led_instances.c file is shown with only one instance, but if more were in use they would all appear in this .c file.
*  
* @code{.c}
* // sl_si91x_led_instances.h
*
* #ifndef SL_SI91x_LED_INSTANCES_H
* #define SL_SI91x_LED_INSTANCES_H
* 
* #include "sl_si91x_led.h"
* 
* extern const sl_led_t led_led0;
* 
* void led_init_instances(void);
* 
* #endif // SL_SI91x_LED_INSTANCES_H
* @endcode
* 
* @note The sl_si91x_led_instances.h file is shown with only one instance, but if more were in use they would all appear in this .h file.
* 
* - For more information on configuring available parameters, see the respective peripheral example readme document and
*  [Peripheral Configuration](https://docs.silabs.com/wiseconnect/latest/wiseconnect-api-reference-guide-si91x-peripherals/disable-uc-config).
*
*  @section led_usage Usage
* 
*  After the LED structures are defined, the LED functions can be called by passing an instance of sl_led_t,
*  which will internally redirect to the type-specific version of the function.
*  The commonly used functions include the following:
* 
*  1. It configure GPIOs pertaining to the control of LEDs
*     and to initialize the selected LED GPIO and mode: @ref sl_si91x_led_init
*  2. This atomically wraps an XOR or a similar operation
*     for a single GPIO pin connected to an LED to toggle the LED: @ref sl_si91x_led_toggle
*  3. To set a GPIO pin connected to an LED to turn the LED on: @ref sl_si91x_led_set
*  4. To clear a GPIO pin connected to an LED to turn the LED off: @ref sl_si91x_led_clear
*
* @ref sl_si91x_led_init initialises the specific LED. This function is called automatically when the application is run.
*  
* @ref sl_si91x_led_toggle can be implemented by the application, if required. This function is used
* to toggle the state of the LED. @ref sl_si91x_led_set and @ref sl_si91x_led_clear both can be implemented by the application.
* These functions are used to turn the LED on and off resectively.
*
*/
/** @} end group LED */

#ifdef __cplusplus
}
#endif

#endif // __SL_SI91X_LED_H__
