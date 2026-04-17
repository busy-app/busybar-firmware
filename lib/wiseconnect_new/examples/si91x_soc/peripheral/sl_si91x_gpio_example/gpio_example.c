/*******************************************************************************
 * @file  gpio_example.c
 * @brief GPIO example
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
/**============================================================================
 * @brief : This file contains example application for GPIO example
 * @section Description :
 * This application demonstrates the HP pin interrupt and toggling of HP GPIO pin.
 ============================================================================**/
#include "gpio_example.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_gpio_board.h"
#include "rsi_debug.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define AVL_INTR_NO 0 // available interrupt number
#define INT_CH      0 // GPIO Pin interrupt 0
/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/*******************************************************************************
 *************************** LOCAL VARIABLES   *********************************
 ******************************************************************************/
// Define GPIO port and pin for HP instance. Here SL_SI91X_GPIO_6_PORT corresponds to GPIO pin 6's port number : 0.
// This is defined to '0' which is PORT_A and SL_SI91X_GPIO_6_PIN refers to GPIO pin number 6.
// Same representation followed for other port and pins.
static sl_si91x_gpio_pin_config_t sl_gpio_pin_config = { { SL_SI91X_GPIO_6_PORT, SL_SI91X_GPIO_6_PIN }, GPIO_OUTPUT };
// Define GPIO port and pin for HP instance. Here SL_SI91X_GPIO_11_PORT corresponds to GPIO pin 11's port number : 0.
// This is defined to '0' which is PORT_A and SL_SI91X_GPIO_11_PIN refers to GPIO pin number 11.
// Same representation followed for other port and pins.
static sl_si91x_gpio_pin_config_t sl_gpio_pin_config1 = { { SL_SI91X_GPIO_11_PORT, SL_SI91X_GPIO_11_PIN }, GPIO_INPUT };
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void gpio_pin_interrupt0_callback(uint32_t pin_intr);
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * GPIO example initialization function. It initializes clock, pin mode,
 * direction and configures pin interrupt
 ******************************************************************************/
void gpio_example_init(void)
{
  sl_status_t status;

  do {
    // Initialize the GPIOs by clearing all interrupts initially
    status = sl_gpio_driver_init();
    if (status != SL_STATUS_OK) {
      // Prints GPIO initialization fails
      DEBUGOUT("sl_gpio_driver_init, Error code: %lu\r\n", status);
      break; // breaks if error occurs
    }
    DEBUGOUT("GPIO driver initialization is successful \r\n");
    // Configure GPIO pin 6 using pin configuration API.
    // Using this API by default GPIO mode is set as MODE 0. If any other mode is selected for any GPIO use
    // corresponding API sl_gpio_driver_set_pin_mode() is for mode setting.
    status = sl_gpio_set_configuration(sl_gpio_pin_config);
    if (status != SL_STATUS_OK) {
      // Prints if pin configuration fails
      DEBUGOUT("sl_gpio_set_configuration, Error code: %lu\r\n", status);
      break; // breaks if error occurs
    }
    DEBUGOUT("GPIO driver set pin configuration is successful \r\n");
    // Configure GPIO pin 11 using pin configuration API.
    // Using this API by default GPIO mode is set as MODE 0. If any other mode is selected for any GPIO use
    // corresponding API sl_gpio_driver_set_pin_mode() is for mode setting.
    status = sl_gpio_set_configuration(sl_gpio_pin_config1);
    if (status != SL_STATUS_OK) {
      // Prints if pin configuration fails
      DEBUGOUT("sl_gpio_set_configuration, Error code: %lu\r\n", status);
      break; // breaks if error occurs
    }
    DEBUGOUT("GPIO driver set pin configuration is successful \r\n");
    // Configure pin interrupt for GPIO pin. The pin interrupt in this application is performed using external triggering from button.
    // Press button1 for triggering HP GPIO instance pin interrupt
    status = sl_gpio_driver_configure_interrupt(&sl_gpio_pin_config1.port_pin,
                                                INT_CH,
                                                (sl_gpio_interrupt_flag_t)SL_GPIO_INTERRUPT_RISE_EDGE,
                                                (sl_gpio_irq_callback_t)&gpio_pin_interrupt0_callback,
                                                AVL_INTR_NO);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_gpio_configure_pin_interrupt, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("GPIO driver interrupt configure is successful \r\n");
  } while (false);
}
/*******************************************************************************
 * Function will run continuously in while loop and toggles GPIO pin 6
 ******************************************************************************/
void gpio_example_process_action(void)
{
  sl_status_t status;
  status = sl_gpio_driver_toggle_pin(&sl_gpio_pin_config.port_pin); // Toggle HP GPIO pin 6
  if (status != SL_STATUS_OK) {
    // Prints if toggling pin fails
    DEBUGOUT("sl_gpio_toggle_pin, Error code: %lu\r\n", status);
    return;
  }
  // Prints indicating successful pin toggle
  DEBUGOUT("HP GPIO driver toggle pin is successful \r\n");
}

/*******************************************************************************
 *   This API handles GPIO pin interrupt 0 request
 ******************************************************************************/
static void gpio_pin_interrupt0_callback(uint32_t pin_intr)
{
  if (pin_intr == PIN_INTR_0) {
    // This is with respect to ISR context. Debugout might cause issues sometimes.
    DEBUGOUT("gpio pin interrupt0\r\n");
  }
}
