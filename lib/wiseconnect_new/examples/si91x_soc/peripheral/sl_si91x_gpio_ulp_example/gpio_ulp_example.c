/*******************************************************************************
 * @file  gpio_ulp_example.c
 * @brief GPIO ULP example
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
 * This application demonstrates the toggling of ULP instance GPIO at intervals of 
 * 1 second and configures ULP pin interrupt.
 ============================================================================**/
#include "gpio_ulp_example.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_gpio_board.h"
#include "rsi_debug.h"
#include "sl_si91x_clock_manager.h"
/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define DELAY       1000 // Delay for 1sec
#define ULP_INT_CH  0    // ULP GPIO Pin interrupt 0
#define AVL_INTR_NO 0    // available interrupt number
/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/*******************************************************************************
 *************************** LOCAL VARIABLES   *********************************
 ******************************************************************************/
typedef sl_gpio_t sl_si91x_gpio_t;
// Define a configuration structure for GPIO pins that are required, specifying its port and pin number.
// Below configurations are for ULP GPIO 2. Here SL_SI91X_ULP_GPIO_2_PORT corresponds to ULP GPIO pin 2's port number : 4.
// This is defined to '4' which is ULP_PORT and SL_SI91X_ULP_GPIO_2_PIN refers to ULP GPIO pin number 2.
// Same representation followed for other port and pins.
static sl_si91x_gpio_pin_config_t sl_gpio_pin_config = { { SL_SI91X_ULP_GPIO_2_PORT, SL_SI91X_ULP_GPIO_2_PIN },
                                                         GPIO_OUTPUT };

// Define GPIO port and pin for ULP instance. Here SL_SI91X_ULP_GPIO_8_PORT / SL_SI91X_ULP_GPIO_4_PORT refers to ULP GPIO pin 8's port number : 4.
// This is defined to '4' which is ULP_PORT and SL_SI91X_ULP_GPIO_8_PIN refers to GPIO pin number 8 / SL_SI91X_ULP_GPIO_4_PIN refers to GPIO pin number 4.
// Same representation followed for other port and pins.
static sl_si91x_gpio_pin_config_t sl_gpio_pin_config1 = { { SL_SI91X_ULP_GPIO_8_PORT, SL_SI91X_ULP_GPIO_8_PIN },
                                                          GPIO_INPUT };

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void gpio_ulp_pin_interrupt_callback(uint32_t pin_intr);
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * GPIO example initialization function. It initializes clock, pin mode,
 * direction and configures ULP pin interrupt
 ******************************************************************************/
void gpio_ulp_example_init(void)
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
    // Configure ULP GPIO pin 2 using driver pin configuration API.
    // Using this API by default GPIO mode is set as MODE 0. If any other mode is selected for any GPIO use
    // corresponding API sl_gpio_driver_set_pin_mode() is for mode setting.
    status = sl_gpio_set_configuration(sl_gpio_pin_config);
    if (status != SL_STATUS_OK) {
      // Prints if pin configuration fails
      DEBUGOUT("sl_gpio_set_configuration, Error code: %lu\r\n", status);
      break; // breaks if error occurs
    }
    DEBUGOUT("GPIO driver set pin configuration is successful \r\n");
    // Configure ULP GPIO pin 8 using driver pin configuration API.
    // Using this API by default GPIO mode is set as MODE 0. If any other mode is selected for any GPIO use
    // corresponding API sl_gpio_driver_set_pin_mode() is for mode setting.
    status = sl_gpio_set_configuration(sl_gpio_pin_config1);
    if (status != SL_STATUS_OK) {
      // Prints if pin configuration fails
      DEBUGOUT("sl_gpio_set_configuration, Error code: %lu\r\n", status);
      break; // breaks if error occurs
    }
    DEBUGOUT("GPIO driver set pin configuration is successful \r\n");
    // Configure ULP GPIO pin interrupts. The pin interrupt in this application is performed using external triggering from button.
    // Connect ULP GPIO8(P15) to low/high for triggering ULP GPIO instance pin interrupt based on event selected.
    status = sl_gpio_driver_configure_interrupt(&sl_gpio_pin_config1.port_pin,
                                                ULP_INT_CH,
                                                (sl_gpio_interrupt_flag_t)SL_GPIO_INTERRUPT_FALL_EDGE,
                                                (sl_gpio_irq_callback_t)&gpio_ulp_pin_interrupt_callback,
                                                AVL_INTR_NO);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_gpio_configure_pin_interrupt, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("GPIO driver configure ulp pin interrupt is successful \r\n");
  } while (false);
}
/*******************************************************************************
 * Function will run continuously in while loop and led toggles for every 1sec
 ******************************************************************************/
void gpio_ulp_example_process_action(void)
{
  sl_status_t status;
  status = sl_gpio_driver_set_pin(&sl_gpio_pin_config.port_pin); // Set ULP GPIO pin 2
  if (status != SL_STATUS_OK) {
    // Prints if setting pin fails
    DEBUGOUT("sl_gpio_set_pin, Error code: %lu\r\n", status);
    return;
  }
  // Prints indicating successful pin setting
  DEBUGOUT("ULP GPIO driver set pin is successful \r\n");
  sl_si91x_delay_ms(DELAY);                                        // Delay of 1sec
  status = sl_gpio_driver_clear_pin(&sl_gpio_pin_config.port_pin); // Clear ULP GPIO pin 2
  if (status != SL_STATUS_OK) {
    // Prints if clearing pin fails
    DEBUGOUT("sl_gpio_clear_pin, Error code: %lu\r\n", status);
    return;
  }
  // Prints indicating successful pin clearing
  DEBUGOUT("ULP GPIO driver clear pin is successful \r\n");
  sl_si91x_delay_ms(DELAY); // Delay of 1sec
}

/*******************************************************************************
 *  This API handles ULP GPIO OR'ed pin interrupt request
 ******************************************************************************/
static void gpio_ulp_pin_interrupt_callback(uint32_t pin_intr)
{
  if (pin_intr == ULP_PIN_INTR_0) {
    // This is with respect to ISR context. Debugout might cause issues sometimes.
    DEBUGOUT("gpio ulp pin interrupt0\r\n");
  }
}
