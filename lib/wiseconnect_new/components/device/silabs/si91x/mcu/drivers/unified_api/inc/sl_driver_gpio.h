/*******************************************************************************
* @file sl_driver_gpio.h
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
#ifndef SL_DRIVER_GPIO_H
#define SL_DRIVER_GPIO_H

#if !defined(GPIO_PRESENT)
#include "sl_status.h"
#include "sl_si91x_peripheral_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/ /**
 * @addtogroup GPIO General-Purpose Input-Output
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
  ******************************************************************************/
/*******************************************************************************
 ***************************  DEFINES / MACROS ******************************* ******************************************************************************/
#define GPIO_MAX_OUTPUT_VALUE 1  ///< GPIO output maximum value
#define MAX_GROUP_INT         2  ///< Maximum number of group interrupts
#define GPIO_PORT_MAX_VALUE   5  ///< Maximum number of GPIO ports
#define MAX_UULP_INT          5  ///< Maximum number of UULP interrupts
#define ULP_MAX_MODE          10 ///< Maximum ULP mode

#define GPIO_MAX_INTR_VALUE  8    ///< Maximum number of M4 GPIO pin interrupts
#define PORTD_PIN_MAX_VALUE  9    ///< Port D maximum(0-9) number of GPIO pins
#define ULP_PIN_MAX_VALUE    11   ///< Port E maximum(0-11) number of GPIO pins
#define UULP_PIN_MAX_VALUE   5    ///< Port F maximum(0-4) number of GPIO pins
#define MAX_ULP_INTR         8    ///< Maximum number of ULP interrupts
#define MAX_MODE             15   ///< Maximum M4 GPIO mode
#define PORTA_PIN_MAX_VALUE  57   ///< GPIO pin maximum(0-63) value for SL_GPIO_PORT_A of HP instance
#define PORT_PIN_MAX_VALUE   15   ///< GPIO pin maximum(0-15) value for SL_GPIO_PORT_B, SL_GPIO_PORT_C of HP instance
#define GPIO_FLAGS_MAX_VALUE 0x0F ///< GPIO flags maximum value

#define PAD_SELECT_9       9      ///< GPIO PAD selection number
#define UULP_PORT          5      ///< Refers to port for UULP instance
#define GPIO_MAX_PORT_PINS 0xFFFF ///< Refers to maximum no. of pins port can support

/*******************************************************************************
 ********************************   ENUMS   *********************************** ******************************************************************************/
/// @brief structure to hold parameters of GPIO port and pin numbers.
typedef struct {
  sl_gpio_port_t port; ///< GPIO port
  uint8_t pin;         ///< GPIO pin number
} sl_gpio_t;

/// @brief Structure to configure GPIO pin settings.
typedef struct {
  sl_gpio_t port_pin;                  ///< The port and pin number of the GPIO
  sl_si91x_gpio_direction_t direction; ///< The direction of the GPIO pin (input or output)
} sl_si91x_gpio_pin_config_t;

/// @brief UULP GPIO pin/interrupt number
typedef enum {
  UULP_GPIO_INTERRUPT_0 = 0, /// UULP GPIO 0 pin/interrupt number
  UULP_GPIO_INTERRUPT_1 = 1, /// UULP GPIO 1 pin/interrupt number
  UULP_GPIO_INTERRUPT_2 = 2, /// UULP GPIO 2 pin/interrupt number
  UULP_GPIO_INTERRUPT_3 = 3, /// UULP GPIO 3 pin/interrupt number
  UULP_GPIO_INTERRUPT_4 = 4, /// UULP GPIO 4 pin/interrupt number
} sl_si91x_uulp_gpio_interrupt_t;

/// @brief UULP GPIO interrupt bit position
/// @note The APIs which uses this enum is deprecated, new APIs use sl_si91x_uulp_gpio_interrupt_t instead
typedef enum {
  UULP_GPIO_INTERRUPT_0_BIT = BIT(0), /// UULP GPIO 0 interrupt bit position
  UULP_GPIO_INTERRUPT_1_BIT = BIT(1), /// UULP GPIO 1 interrupt bit position
  UULP_GPIO_INTERRUPT_2_BIT = BIT(2), /// UULP GPIO 2 interrupt bit position
  UULP_GPIO_INTERRUPT_3_BIT = BIT(3), /// UULP GPIO 3 interrupt bit position
  UULP_GPIO_INTERRUPT_4_BIT = BIT(4), /// UULP GPIO 4 interrupt bit position
} sl_si91x_uulp_gpio_interrupt_bit_t;

/*******************************************************************************
 ********************************   Local Variables   ************************* ******************************************************************************/
/// @brief GPIO interrupt callback function pointer.
typedef void (*sl_gpio_irq_callback_t)(uint32_t flag);

/*******************************************************************************
 *****************************   PROTOTYPES   ********************************* ******************************************************************************/

/***************************************************************************/
/**
 * @brief  Clears one or more pending GPIO interrupts.
 * 
 * This function clears the specified GPIO interrupt sources. The `flags` 
 * parameter is a bitwise OR of the interrupt sources to be cleared.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection() must be called for HP instance to enable pad selection.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance to enable pad receiver.
 * -   \ref sl_gpio_driver_set_pin_mode() must be called to set the pin mode.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 *
 * @param[in]  flags  Bitwise logic OR of GPIO interrupt sources to clear.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK  - Success.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
STATIC __INLINE sl_status_t sl_gpio_driver_clear_interrupts(uint32_t flags)
{
  sl_gpio_clear_interrupts(flags);
  return SL_STATUS_OK;
}

/***************************************************************************/
/**
 * @brief Configures the GPIO pin interrupt.
 * 
 * This function sets up a GPIO pin to trigger an interrupt. The `gpio` parameter 
 * is a pointer to a structure of type \ref sl_gpio_t, which contains the GPIO 
 * configuration. The `int_no` parameter specifies the interrupt number to trigger 
 * (ranging from 0 to 7). The `flags` parameter contains the interrupt configuration 
 * flags of type sl_gpio_interrupt_flag_t. The `gpio_callback` parameter is a 
 * pointer to the IRQ callback function of type \ref sl_gpio_irq_callback_t. The 
 * `avl_intr_no` parameter is an output pointer to the available interrupt number.
 * 
 * @pre Pre-conditions:
 * -   sl_gpio_set_configuration() must be called to configure the GPIO.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 * 
 * @param[in] gpio Pointer to the structure of type \ref sl_gpio_t.
 * @param[in] int_no Specifies the interrupt number to trigger,
 *                  - For High Performance (HP) GPIO ports:
 *                      Valid values are (0-7). All 8 interrupts can be used simultaneously.
 *                  - For Ultra-Low Power (ULP) GPIO port:
 *                      Valid values are (0-7). (0-7) interrupts can be configured simultaneously, but only one interrupt is served at a time.
 *                  - For Ultra-Ultra Low Power (UULP) GPIO port:
 *                      Valid values are (0-4). (0-4) interrupts can be configured simultaneously, but only one interrupt is served at a time.
 * @param[in] flags Interrupt configuration flags of type sl_gpio_interrupt_flag_t.
 * @param[in] gpio_callback IRQ callback function pointer of type \ref sl_gpio_irq_callback_t.
 * @param[out] avl_intr_no Pointer to the available interrupt number. This parameter is currently
 *                         unused and will be ignored.
 *
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * -   SL_STATUS_BUSY  - Interrupt is busy and cannot carry out the requested operation.
 * -   SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * -   SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note Ensure that the GPIO clock is enabled and the pin is properly configured 
 *       before calling this function.
 * 
 ******************************************************************************/
sl_status_t sl_gpio_driver_configure_interrupt(sl_gpio_t *gpio,
                                               uint32_t int_no,
                                               sl_gpio_interrupt_flag_t flags,
                                               sl_gpio_irq_callback_t gpio_callback,
                                               uint32_t *avl_intr_no);

/***************************************************************************/
/**
 * @brief Configures the GPIO group interrupts for HP and ULP instances.
 * 
 * This function sets up group interrupts for GPIO pins in both High Performance (HP) 
 * and Ultra Low Power (ULP) instances. The `configuration` parameter is a pointer 
 * to a structure of type sl_si91x_gpio_group_interrupt_config_t, which contains 
 * the GPIO group interrupt configuration. The `gpio_callback` parameter is a pointer 
 * to the IRQ callback function of type \ref sl_gpio_irq_callback_t.
 * 
 * @pre Pre-conditions:
 * -   sl_gpio_set_configuration() must be called to configure the GPIO.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 * 
 * @param[in] configuration Pointer to the structure of type sl_si91x_gpio_group_interrupt_config_t
 * @param[in] gpio_callback IRQ callback function pointer of type \ref sl_gpio_irq_callback_t.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * -   SL_STATUS_BUSY  - Interrupt is busy and cannot carry out the requested operation.
 * -   SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * -   SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note Ensure that the GPIO clock is enabled and the pins are properly configured 
 *       before calling this function.
 * 
 ******************************************************************************/
sl_status_t sl_gpio_configure_group_interrupt(sl_si91x_gpio_group_interrupt_config_t *configuration,
                                              sl_gpio_irq_callback_t gpio_callback);

/***************************************************************************/
/**
 * @brief  Sets the pin mode for a GPIO pin in HP (or) ULP instance.
 * 
 * @details By default, mode-0 is set, and the GPIO pin acts as a normal GPIO. 
 * If a GPIO pin needs to be used for some alternate modes, the respective mode 
 * should be selected. For more information about modes present for different 
 * instances, refer to the PIN MUX section in the HRM.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_ulp_pad_receiver() must be called for ULP instance.
 * 
 * Use the corresponding pad receiver API for the corresponding GPIO instance.
 * 
 * @param[in] gpio Pointer to the structure of type \ref sl_gpio_t.
 *                 Refer to the table below for a description of each port and pins available.
 *                 |  GPIO Instance                              |    GPIO Port          |  GPIO Pin Number  |
 *                 |---------------------------------------------|-----------------------|-------------------|
 *                 |                                             |  SL_GPIO_PORT_A       |   (0-15)          |
 *                 | HP (High Power) GPIO Instance               |  SL_GPIO_PORT_B       |   (16-31)         |
 *                 |                                             |  SL_GPIO_PORT_C       |   (32-47)         |
 *                 |                                             |  SL_GPIO_PORT_D       |   (48-57)         |
 *                 |---------------------------------------------|-----------------------|-------------------|
 *                 | ULP (Ultra Low Power) GPIO Instance         |  SL_GPIO_ULP_PORT     |   (0-11)          |
 *                 |---------------------------------------------|-----------------------|-------------------|
 *                 | UULP (Ultra Ultra Low Power) GPIO Instance  | SL_GPIO_UULP_PORT     |   (0-3)           |
 *                 |---------------------------------------------|-----------------------|-------------------|
 *                 PORT_A can also be a single port to access all GPIO (0-57) pins available in the HP domain, 
 *                 instead of using PORT B, C, D. Pins (57-63) are reserved.
 * 
 * @param[in] mode The desired pin mode.
 * @param[in] output_value A value to set for the pin in the GPIO register.
 *                         The GPIO setting is important for some input mode configurations.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * -   SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * -   SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note Ensure that the GPIO clock is enabled and the pin is properly configured 
 *       before calling this function.
  ******************************************************************************/
sl_status_t sl_gpio_driver_set_pin_mode(sl_gpio_t *gpio, sl_gpio_mode_t mode, uint32_t output_value);

/***************************************************************************/
/**
 * @brief Gets the pin mode (alternate function) of a GPIO for either HP instance (or) ULP instance as per the port number.
 * 
 * This function retrieves the current pin mode (alternate function) of a specified GPIO pin. 
 * The `gpio` parameter is a pointer to a structure of type \ref sl_gpio_t, which contains 
 * the GPIO configuration. The `mode` parameter is an output parameter that will hold the 
 * current pin mode after the function executes.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_ulp_pad_receiver() must be called for ULP instance.
 * 
 * Use the corresponding pad receiver API for the corresponding GPIO instance.
 * 
 * @param[in] gpio Pointer to the structure of type \ref sl_gpio_t.
 * @param[out] mode Pointer to a variable where the current pin mode will be stored.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * -   SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * -   SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note Ensure that the GPIO clock is enabled and the pin is properly configured 
 *       before calling this function.
 * 
 ******************************************************************************/
sl_status_t sl_gpio_driver_get_pin_mode(sl_gpio_t *gpio, sl_gpio_mode_t *mode);

/*******************************************************************************/
/**
 * @brief Initializes the GPIO driver.
 * 
 * This function initializes the GPIO driver by clearing all the interrupts for 
 * High Performance (HP), Ultra Low Power (ULP), and NPSS instances. It also sets 
 * all callback function pointers to NULL.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note This function should be called before any other GPIO driver functions 
 *       to ensure proper initialization.
  ******************************************************************************/
sl_status_t sl_gpio_driver_init(void);

/*******************************************************************************/
/**
 * @brief De-Initializes the GPIO driver.
 * 
 * This function de-initializes the GPIO driver by disabling the clocks for both 
 * High Performance (HP), Ultra Low Power (ULP) and Ultra Ulta Low Power (UULP) instances. It also sets all 
 * callback function pointers to NULL.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note This function should be called to properly clean up the GPIO driver 
 *       before the application exits or when the GPIO driver is no longer needed.
  ******************************************************************************/
sl_status_t sl_gpio_driver_deinit(void);

/*******************************************************************************/
/**
 * @brief Unregisters GPIO driver.
 * 
 * This function unregisters the GPIO driver for a specified GPIO instance and interrupt. 
 * The `gpio_instance` parameter specifies the GPIO instance of type sl_si91x_gpio_instances_t. 
 * The `gpio_intr` parameter specifies the GPIO interrupt of type sl_si91x_gpio_intr_t. 
 * The `flag` parameter specifies the GPIO interrupt flag, with a maximum range of 8.
 * 
 * @param[in] gpio_instance Instances of type sl_si91x_gpio_instances_t.
 * @param[in] gpio_intr GPIO interrupts of type sl_si91x_gpio_intr_t.
 * @param[in] flag GPIO interrupt flag. Maximum range is 8.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * -   SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note Ensure that the GPIO instance and interrupt are properly configured before 
 *       calling this function.
  ******************************************************************************/
sl_status_t sl_gpio_driver_unregister(sl_si91x_gpio_instances_t gpio_instance,
                                      sl_si91x_gpio_intr_t gpio_intr,
                                      uint8_t flag);

/***************************************************************************/ /**
 * @brief
 *   Validate if the given pin is configured as SOC peripheral on ULP GPIO.
 *   This function checks if the given pin is configured as a SOC Peripheral on ULP GPIO by
 *   examining the `soc_peri_on_ulp_gpio_status` bit corresponding to the pin number.
 *
 * @param[in] pin
 *   The pin number to validate.
 *
 * @return
 *   Returns the status of the validation.
 *   - SL_STATUS_OK: The pin is valid and configured as a ULP pin.
 *   - SL_STATUS_INVALID_PARAMETER: The pin is not configured as a ULP pin.
 * 
 * * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_gpio_validate_soc_peri_on_ulp_gpio(uint8_t pin);

/*******************************************************************************/
/**
 * @brief Validates the port and pin of a GPIO.
 * 
 * This function checks whether the specified port and pin in the GPIO structure 
 * are valid. The `gpio` parameter is a pointer to a structure of type \ref sl_gpio_t, 
 * which contains the GPIO configuration.
 * 
 * @param[in] gpio Pointer to the structure of type \ref sl_gpio_t.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * -   SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note Ensure that the `gpio` structure is properly initialized before calling this function.
  ******************************************************************************/
STATIC __INLINE sl_status_t sl_gpio_validation(sl_gpio_t *gpio)
{
  // Checks if gpio port value exceeds maximum allowed value. Return error code for invalid parameter
  if (gpio->port > GPIO_PORT_MAX_VALUE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  //  Checks if the port is Port A. If true, checks if the pin value exceeds
  //  the maximum allowable value for Port A. Returns an invalid parameter status code if true
  if (gpio->port == SL_GPIO_PORT_A) {
    if (gpio->pin < GPIO_PA_PIN_0_5_VALIDATE) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    if (gpio->pin > PORTA_PIN_MAX_VALUE) {
      // If the pin value exceeds the maximum allowable value, it checks if the pin is configured as
      // SOC peripheral on ULP GPIO.
      return sl_si91x_gpio_validate_soc_peri_on_ulp_gpio(gpio->pin);
    }
  }
  // Checks if the port is either Port B or Port C. If true, checks if the pin value exceeds
  // the maximum allowable value for these ports. Returns an invalid parameter status code if true.
  if ((gpio->port == SL_GPIO_PORT_B) || (gpio->port == SL_GPIO_PORT_C)) {
    if (gpio->pin > PORT_PIN_MAX_VALUE) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  }
  // Checks if the port is Port D. If true, checks if the pin value exceeds the maximum allowable
  // value for Port D. Returns an invalid parameter status code if true.
  if (gpio->port == SL_GPIO_PORT_D) {
    if (gpio->pin > PORTD_PIN_MAX_VALUE) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  }
  // Checks if the GPIO port is the Ultra-Low Power GPIO port.
  if (gpio->port == SL_GPIO_ULP_PORT) {
    // Check if the GPIO pin exceeds the maximum allowed values.
    if (gpio->pin > ULP_PIN_MAX_VALUE) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  }
  // Checks if the GPIO port is the Ultra-Ultra Low Power GPIO port.
  if (gpio->port == SL_GPIO_UULP_PORT) {
    // Check if the GPIO pin exceeds the maximum allowed values.
    if (gpio->pin > UULP_PIN_MAX_VALUE) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  }
  return SL_STATUS_OK;
}

/***************************************************************************/
/**
 * @brief Sets a single pin in the GPIO configuration register to 1.
 * 
 * This function sets a specified GPIO pin to 1 in the GPIO configuration register. 
 * The `gpio` parameter is a pointer to a structure of type \ref sl_gpio_t, which 
 * contains the GPIO configuration.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_ulp_pad_receiver() must be called for ULP instance.
 * 
 * Use the corresponding pad receiver API for the corresponding GPIO instance.
 * -   \ref sl_gpio_driver_set_pin_mode() must be called to set the pin mode.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 * 
 * @param[in] gpio Pointer to the structure of type \ref sl_gpio_t.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * -   SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * -   SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note Ensure that the GPIO clock is enabled and the pin is properly configured 
 *       before calling this function.
  ******************************************************************************/
STATIC __INLINE sl_status_t sl_gpio_driver_set_pin(sl_gpio_t *gpio)
{
  sl_status_t status;
  // Checks if the gpio pointer is NULL
  if (gpio == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  status = sl_gpio_validation(gpio);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Sets the GPIO pin output
  sl_gpio_set_pin_output(gpio->port, gpio->pin);
  return SL_STATUS_OK;
}

/***************************************************************************/
/**
 * @brief Clears a single pin in the GPIO configuration register.
 * 
 * This function clears (sets to 0) a specified GPIO pin in the GPIO configuration register. 
 * The `gpio` parameter is a pointer to a structure of type \ref sl_gpio_t, which contains 
 * the GPIO configuration.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_ulp_pad_receiver() must be called for ULP instance.
 * 
 * Use the corresponding pad receiver API for the corresponding GPIO instance.
 * -   \ref sl_gpio_driver_set_pin_mode() must be called to set the pin mode.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 * 
 * @param[in] gpio Pointer to the structure of type \ref sl_gpio_t.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * -   SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * -   SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note Ensure that the GPIO clock is enabled and the pin is properly configured 
 *       before calling this function.
  ******************************************************************************/
STATIC __INLINE sl_status_t sl_gpio_driver_clear_pin(sl_gpio_t *gpio)
{
  sl_status_t status;
  // Checks if the gpio pointer is NULL. Returns error code for NULL pointer
  if (gpio == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  status = sl_gpio_validation(gpio);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Clears the GPIO pin output
  sl_gpio_clear_pin_output(gpio->port, gpio->pin);
  return SL_STATUS_OK;
}

/***************************************************************************/
/**
 * @brief Toggles a single pin in the GPIO port register.
 * 
 * This function toggles the state of a specified GPIO pin in the GPIO port register. 
 * The `gpio` parameter is a pointer to a structure of type \ref sl_gpio_t, which 
 * contains the GPIO configuration.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_ulp_pad_receiver() must be called for ULP instance.
 * 
 * Use the corresponding pad receiver API for the corresponding GPIO instance.
 * -   \ref sl_gpio_driver_set_pin_mode() must be called to set the pin mode.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 * 
 * @param[in] gpio Pointer to the structure of type \ref sl_gpio_t.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * -   SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * -   SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note Ensure that the GPIO clock is enabled and the pin is properly configured 
 *       before calling this function.
  ******************************************************************************/
STATIC __INLINE sl_status_t sl_gpio_driver_toggle_pin(sl_gpio_t *gpio)
{
  sl_status_t status;
  // Checks if the gpio pointer is NULL. Returns error code for NULL pointer
  if (gpio == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  status = sl_gpio_validation(gpio);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Toggles GPIO pin
  sl_gpio_toggle_pin_output(gpio->port, gpio->pin);
  return SL_STATUS_OK;
}

/***************************************************************************/
/**
 * @brief Reads the pin value for a single pin in a GPIO port.
 * 
 * This function reads the value of a specified GPIO pin. The `gpio` parameter 
 * is a pointer to a structure of type \ref sl_gpio_t, which contains the GPIO 
 * configuration. The `pin_value` parameter is an output parameter that will 
 * hold the value of the GPIO pin after the function executes.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_ulp_pad_receiver() must be called for ULP instance.
 * 
 * Use the corresponding pad receiver API for the corresponding GPIO instance.
 * -   \ref sl_gpio_driver_set_pin_mode() must be called to set the pin mode.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 * 
 * @param[in] gpio Pointer to the structure of type \ref sl_gpio_t.
 * @param[out] pin_value Pointer to a variable where the GPIO pin value will be stored.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * -   SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * -   SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note Ensure that the GPIO clock is enabled and the pin is properly configured 
 *       before calling this function.
  ******************************************************************************/
STATIC __INLINE sl_status_t sl_gpio_driver_get_pin(sl_gpio_t *gpio, uint8_t *pin_value)
{
  sl_status_t status;
  // Checks if the gpio pointer is NULL. Returns error code for NULL pointer
  if (gpio == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  status = sl_gpio_validation(gpio);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Gets the GPIO pin input
  *pin_value = sl_gpio_get_pin_input(gpio->port, gpio->pin);
  return SL_STATUS_OK;
}

/***************************************************************************/
/**
 * @brief Sets the bits of the GPIO data out register.
 * 
 * This function sets the specified GPIO pins to 1 in the GPIO data out register 
 * for a given port. The `port` parameter specifies the port to associate with the pin. 
 * The `pins` parameter specifies the GPIO pins in the port that are set to 1. 
 * The pins can be specified as a value between 0 and 65535 in decimal (or 0xFFFF in hex), 
 * allowing up to 16 pins (0-15) to be set at a time.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_ulp_pad_receiver() must be called for ULP instance.
 * 
 * Use the corresponding pad receiver API for the corresponding GPIO instance.
 * -   \ref sl_gpio_driver_set_pin_mode() must be called to set the pin mode.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 * 
 * @param[in] port The port to associate with the pin.
 *                 - HP instance: PORT A, B, C, D
 *                 - ULP instance: PORT 4
 * @param[in] pins The GPIO pins in the port that are set to 1 (0 to 65535 in decimal or 0xFFFF in hex).
 *                 If we want to set pins (maximum of 0-15 pins) in a port, it can set all pins at a time.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * -   SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note Ensure that the GPIO clock is enabled and the pins are properly configured 
 *       before calling this function.
  ******************************************************************************/
STATIC __INLINE sl_status_t sl_gpio_driver_set_port(sl_gpio_port_t port, uint32_t pins)
{
  // Checks if the gpio port value exceeds maximum allowed value. Return error code for invalid parameter
  if ((port > GPIO_PORT_MAX_VALUE) || (pins > GPIO_MAX_PORT_PINS)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Sets the GPIO port output
  sl_gpio_set_port_output(port, pins);
  return SL_STATUS_OK;
}

/***************************************************************************/
/**
 * @brief Sets the bits in the configuration register for a port to 0.
 * 
 * This function clears (sets to 0) the specified GPIO pins in the configuration 
 * register for a given port. The `port` parameter specifies the port to associate 
 * with the pin. The `pins` parameter specifies the GPIO pins in the port that are 
 * set to 0. The pins can be specified as a value between 1 and 65535 in decimal 
 * (or 0xFFFF in hex), allowing up to 16 pins (0-15) to be cleared at a time.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_ulp_pad_receiver() must be called for ULP instance.
 * 
 * Use the corresponding pad receiver API for the corresponding GPIO instance.
 * -   \ref sl_gpio_driver_set_pin_mode() must be called to set the pin mode.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 * 
 * @param[in] port The port to associate with the pin.
 *                 - HP instance: PORT A, B, C, D
 *                 - ULP instance: PORT 4
 * @param[in] pins The GPIO pins in the port that are set to 0 (0 to 65535 in decimal or 0xFFFF in hex).
 *                 If we want to clear pins (maximum of 0-15 pins) in a port, it can clear all pins at a time.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * -   SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note Ensure that the GPIO clock is enabled and the pins are properly configured 
 *       before calling this function.
  ******************************************************************************/
STATIC __INLINE sl_status_t sl_gpio_driver_clear_port(sl_gpio_port_t port, uint32_t pins)
{
  // Checks if the gpio port value exceeds maximum allowed value. Return error code for invalid parameter
  if ((port > GPIO_PORT_MAX_VALUE) || (pins > GPIO_MAX_PORT_PINS)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Clears the GPIO port output
  sl_gpio_clear_port_output(port, pins);
  return SL_STATUS_OK;
}

/***************************************************************************/
/**
 * @brief Gets the current setting for a GPIO configuration register.
 * 
 * This function retrieves the current configuration settings for a specified GPIO port. 
 * The `port` parameter specifies the port to associate with the pin. The `port_value` 
 * parameter is an output parameter that will hold the current value of the GPIO port 
 * configuration register after the function executes.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_ulp_pad_receiver() must be called for ULP instance.
 * 
 * Use the corresponding pad receiver API for the corresponding GPIO instance.
 * -   \ref sl_gpio_driver_set_pin_mode() must be called to set the pin mode.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 * 
 * @param[in] port The port to associate with the pin.
 *                 - HP instance: PORT A, B, C, D
 *                 - ULP instance: PORT 4
 * @param[out] port_value Pointer to a variable where the GPIO port value will be stored.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * -   SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note Ensure that the GPIO clock is enabled and the pins are properly configured 
 *       before calling this function.
  ******************************************************************************/
STATIC __INLINE sl_status_t sl_gpio_driver_get_port_output(sl_gpio_port_t port, uint32_t *port_value)
{
  // Checks if the gpio port value exceeds maximum allowed value. Return error code for invalid parameter
  if (port > GPIO_PORT_MAX_VALUE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Gets the GPIO port output
  *port_value = sl_gpio_get_port_output(port);
  return SL_STATUS_OK;
}

/***************************************************************************/
/**
 * @brief Gets the current setting for a pin in a GPIO configuration register.
 * 
 * This function retrieves the current value of a specified GPIO pin. The `gpio` 
 * parameter is a pointer to a structure of type \ref sl_gpio_t, which contains 
 * the GPIO configuration.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_ulp_pad_receiver() must be called for ULP instance.
 * 
 * Use the corresponding pad receiver API for the corresponding GPIO instance.
 * -   \ref sl_gpio_driver_set_pin_mode() must be called to set the pin mode.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 * 
 * @param[in] gpio Pointer to the structure of type \ref sl_gpio_t.
 * 
 * @return The GPIO pin value:
 *         - '0' - Low
 *         - '1' - High
 ******************************************************************************/
STATIC __INLINE uint8_t sl_gpio_driver_get_pin_output(sl_gpio_t *gpio)
{
  sl_status_t status;
  uint8_t pin_output = 0;
  // Checks if the gpio pointer is NULL. Returns error code for NULL pointer
  if (gpio == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  status = sl_gpio_validation(gpio);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Gets the GPIO pin output
  pin_output = sl_gpio_get_pin_output(gpio->port, gpio->pin);
  return pin_output;
}

/***************************************************************************/
/**
 * @brief Sets the GPIO port configuration register.
 * 
 * This function sets the specified bits in the GPIO port configuration register 
 * for a given port. The `port` parameter specifies the port to associate with the pin. 
 * The `val` parameter specifies the value to write to the port configuration register. 
 * The `mask` parameter indicates which bits to modify.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_ulp_pad_receiver() must be called for ULP instance.
 * 
 * Use the corresponding pad receiver API for the corresponding GPIO instance.
 * -   \ref sl_gpio_driver_set_pin_mode() must be called to set the pin mode.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 * 
 * @param[in] port The port to associate with the pin.
 *                 - HP instance: PORT A, B, C, D
 *                 - ULP instance: PORT 4
 * @param[in] val Value to write to the port configuration register.
 * @param[in] mask Mask indicating which bits to modify.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * -   SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
STATIC __INLINE sl_status_t sl_gpio_driver_set_port_output_value(sl_gpio_port_t port, uint32_t val, uint32_t mask)
{
  // Checks if the gpio port value exceeds maximum allowed value. Return error code for invalid parameter
  if (port > GPIO_PORT_MAX_VALUE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Sets the GPIO port output value
  sl_gpio_set_port_output_value(port, val, mask);
  return SL_STATUS_OK;
}

/***************************************************************************/
/**
 * @brief Sets the slewrate for pins on an HP instance GPIO port.
 * 
 * This function configures the slewrate for the pins on a specified HP instance GPIO port. 
 * The `port` parameter specifies the GPIO port to configure. The `slewrate` parameter 
 * specifies the slewrate for the pins on this GPIO port. The `slewrate_alt` parameter 
 * specifies the slewrate for the pins using alternate modes on this GPIO port.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * 
 * @param[in] port The GPIO port to configure.
 * @param[in] slewrate The slewrate to configure for the pins on this GPIO port.
 * @param[in] slewrate_alt The slewrate to configure for the pins using alternate modes on this GPIO port.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * -   SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
STATIC __INLINE sl_status_t sl_gpio_driver_set_slew_rate(sl_gpio_port_t port, uint32_t slewrate, uint32_t slewrate_alt)
{
  // Checks if the gpio port value exceeds maximum allowed value. Return error code for invalid parameter
  if (port > GPIO_PORT_MAX_VALUE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Sets the GPIO slew rate
  sl_gpio_set_slew_rate(port, slewrate, slewrate_alt);
  return SL_STATUS_OK;
}

/***************************************************************************/
/**
 * @brief Reads the port value for GPIO.
 * 
 * This function reads the value of a specified GPIO port. The `port` parameter 
 * specifies the port to associate with the pin.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_ulp_pad_receiver() must be called for ULP instance.
 * 
 * Use the corresponding pad receiver API for the corresponding GPIO instance.
 * -   \ref sl_gpio_driver_set_pin_mode() must be called to set the pin mode.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 * 
 * @param[in] port The port to associate with the pin.
 *                 - HP instance: PORT A, B, C, D
 *                 - ULP instance: PORT 4
 * 
 * @return The GPIO pin value:
 *         - '0' - Low
 *         - '1' - High
 ******************************************************************************/
STATIC __INLINE uint32_t sl_gpio_driver_get_port_input(sl_gpio_port_t port)
{
  uint32_t port_input = 0;
  // Checks if the gpio port value exceeds maximum allowed value. Return error code for invalid parameter
  if (port > GPIO_PORT_MAX_VALUE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Gets the GPIO port input
  port_input = sl_gpio_get_port_input(port);
  return port_input;
}

/***************************************************************************/
/**
 * @brief Toggles the selected pin values in the GPIO port register.
 * 
 * This function toggles the specified GPIO pins in the port register. The `port` 
 * parameter specifies the port to associate with the pin. The `pins` parameter 
 * specifies the port pins to toggle, ranging from 0 to 65535 in decimal (or 0xFFFF in hex).
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_ulp_pad_receiver() must be called for ULP instance.
 * 
 * Use the corresponding pad receiver API for the corresponding GPIO instance.
 * -   \ref sl_gpio_driver_set_pin_mode() must be called to set the pin mode.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 * 
 * @param[in] port The port to associate with the pin.
 *                 - HP instance: PORT A, B, C, D
 *                 - ULP instance: PORT 4
 * @param[in] pins Port pins to toggle. Ranges from 0 to 65535 in decimal (or 0xFFFF in hex).
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * -   SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
STATIC __INLINE sl_status_t sl_gpio_driver_toggle_port_output(sl_gpio_port_t port, uint32_t pins)
{
  // Checks if the gpio port value exceeds maximum allowed value. Return error code for invalid parameter
  if ((port > GPIO_PORT_MAX_VALUE) || (pins > GPIO_MAX_PORT_PINS)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Toggles the GPIO port output
  sl_gpio_toggle_port_output(port, pins);
  return SL_STATUS_OK;
}

/***************************************************************************/
/**
 * @brief Enables the selected GPIO pin interrupt with the configured interrupt type (Level or Edge).
 * 
 * This function enables the interrupt for the selected GPIO pin with the specified interrupt type, 
 * which can be either level-triggered or edge-triggered. The `flags` parameter specifies the GPIO 
 * interrupt sources to enable.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * -   \ref sl_gpio_driver_set_pin_mode() must be called to set the pin mode.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 * 
 * @param[in] flags GPIO interrupt sources to enable.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * -   SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
STATIC __INLINE sl_status_t sl_gpio_driver_enable_interrupts(uint32_t flags)
{
  sl_gpio_enable_interrupts(flags);
  return SL_STATUS_OK;
}

/*******************************************************************************/ /**
 * @brief Disables the selected GPIO pin interrupt with the configured interrupt type (Level or Edge).
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() 
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection(), for HP instance 
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver(), for HP instance 
 * -   \ref sl_gpio_driver_set_pin_mode() 
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() 
 *
 * @param[in]  flags  -  GPIO interrupt sources to disable.
 * @return Status code indicating the result:
 * -   SL_STATUS_OK  - Success.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
  ******************************************************************************/
STATIC __INLINE sl_status_t sl_gpio_driver_disable_interrupts(uint32_t flags)
{
  sl_gpio_disable_interrupts(flags);
  return SL_STATUS_OK;
}

/***************************************************************************/
/**
 * @brief Sets the selected GPIO pin interrupt with the configured interrupt type (Level or Edge).
 * 
 * This function sets the interrupt for the selected GPIO pin with the specified interrupt type, 
 * which can be either level-triggered or edge-triggered. The `flags` parameter specifies the GPIO 
 * interrupt sources to set to pending.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * -   \ref sl_gpio_driver_set_pin_mode() must be called to set the pin mode.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 * 
 * @param[in] flags GPIO interrupt sources to set to pending.
 * 
 * @return Status code indicating the result:
 * -   SL_STATUS_OK - Success.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
STATIC __INLINE sl_status_t sl_gpio_driver_set_interrupts(uint32_t flags)
{
  sl_gpio_set_interrupts(flags);
  return SL_STATUS_OK;
}

/***************************************************************************/
/**
 * @brief Gets the pending GPIO interrupts.
 * 
 * This function retrieves the numbers of the GPIO pins that have pending interrupts.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * -   \ref sl_gpio_driver_set_pin_mode() must be called to set the pin mode.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 * 
 * @return Pending GPIO pin interrupt numbers.
 ******************************************************************************/
STATIC __INLINE uint32_t sl_gpio_driver_get_pending_interrupts(void)
{
  uint32_t status;
  status = sl_gpio_get_pending_interrupts();
  return status;
}

/***************************************************************************/
/**
 * @brief Gets the enabled GPIO interrupts.
 * 
 * This function retrieves the numbers of the GPIO pins that have enabled interrupts.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * -   \ref sl_gpio_driver_set_pin_mode() must be called to set the pin mode.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 * 
 * @return Enabled GPIO pin interrupt numbers.
 ******************************************************************************/
STATIC __INLINE uint32_t sl_gpio_driver_get_enabled_interrupts(void)
{
  uint32_t status;
  status = sl_gpio_get_enabled_interrupts();
  return status;
}

/***************************************************************************/
/**
 * @brief Gets the enabled and pending GPIO interrupt flags.
 * 
 * This function retrieves the flags for the enabled and pending GPIO interrupts.
 * 
 * @pre Pre-conditions:
 * -   \ref sl_si91x_gpio_driver_enable_clock() must be called to enable the GPIO clock.
 * -   \ref sl_si91x_gpio_driver_enable_pad_selection() must be called for HP instance.
 * -   \ref sl_si91x_gpio_driver_enable_pad_receiver() must be called for HP instance.
 * -   \ref sl_gpio_driver_set_pin_mode() must be called to set the pin mode.
 * -   \ref sl_si91x_gpio_driver_set_pin_direction() must be called to set the pin direction.
 * 
 * @return Flags indicating the enabled and pending GPIO interrupts.
 ******************************************************************************/
STATIC __INLINE uint32_t sl_gpio_driver_get_enabled_pending_interrupts(void)
{
  uint32_t status;
  status = sl_gpio_get_enabled_pending_interrupts();
  return status;
}

/** @} (end addtogroup GPIO) */

#ifdef __cplusplus
}
#endif

#endif ///< GPIO_PRESENT
#endif ///< SL_DRIVER_GPIO_H
