/*******************************************************************************
 * @file  gpio_uulp_example.c
 * @brief GPIO UULP example
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
 * This application demonstrates the UULP pin interrupt.
 ============================================================================**/
#include "gpio_uulp_example.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_gpio_board.h"
#include "rsi_debug.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define UULP_GPIO_INTR_2 UULP_GPIO_INTERRUPT_2 // UULP GPIO pin interrupt 2
#define AVL_INTR_NO      0                     // available interrupt number
/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/*******************************************************************************
 *************************** LOCAL VARIABLES   *********************************
 ******************************************************************************/
// Define GPIO port and pin for UULP instance. Here SL_SI91X_UULP_GPIO_2_PORT corresponds to UULP GPIO pin 2's port number : 5
// This is defined to '5' which is UULP_PORT and SL_SI91X_UULP_GPIO_2_PIN refers to GPIO pin number 2.
// Same representation followed for other port and pins.
static sl_si91x_gpio_pin_config_t sl_gpio_pin_config1 = { { SL_SI91X_UULP_GPIO_2_PORT, SL_SI91X_UULP_GPIO_2_PIN },
                                                          GPIO_INPUT };
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void gpio_uulp_pin_interrupt_callback(uint32_t pin_intr);
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * GPIO example initialization function. It initializes clock, pin mode,
 * direction and configures UULP pin interrupt
 ******************************************************************************/
void gpio_uulp_example_init(void)
{
  sl_status_t status;
  uulp_pad_config_t uulp_pad;
  do {
    uulp_pad.gpio_padnum = SL_SI91X_UULP_GPIO_2_PIN; // UULP GPIO pin number 2 is selected
    uulp_pad.pad_select  = SET;                      // UULP GPIO PAD is selected
    uulp_pad.mode        = CLR;                      // UULP GPIO mode 0 is selected
    uulp_pad.direction   = SET;                      // UULP GPIO direction is selected
    uulp_pad.receiver    = SET;                      // UULP GPIO receiver is enabled

    // Initialize the GPIOs by clearing all interrupts initially
    status = sl_gpio_driver_init();
    if (status != SL_STATUS_OK) {
      // Prints GPIO initialization fails
      DEBUGOUT("sl_gpio_driver_init, Error code: %lu\r\n", status);
      break; // breaks if error occurs
    }
    DEBUGOUT("GPIO driver initialization is successful \r\n");
    // Configure UULP GPIO pin 2 using driver pin configuration API.
    // Using this API by default GPIO mode is set as MODE 0. If any other mode is selected for any GPIO use
    // corresponding API sl_gpio_driver_set_pin_mode() is for mode setting.
    status = sl_gpio_set_configuration(sl_gpio_pin_config1);
    if (status != SL_STATUS_OK) {
      // Prints if pin configuration fails
      DEBUGOUT("sl_gpio_set_configuration, Error code: %lu\r\n", status);
      break; // breaks if error occurs
    }
    DEBUGOUT("GPIO driver set pin configuration is successful \r\n");
    // Configure the UULP GPIO pin mode, receiver enable, direction and polarity.
    status = sl_si91x_gpio_driver_set_uulp_pad_configuration(&uulp_pad);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_set_uulp_pad_configuration, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("GPIO driver set uulp pad configuration is successful \r\n");
    // Configure uulp pin interrupt. The pin interrupt in this application is performed using external triggering from button.
    // Press button0 for triggering UULP GPIO instance pin interrupt based on event selected.
    status = sl_gpio_driver_configure_interrupt(&sl_gpio_pin_config1.port_pin,
                                                UULP_GPIO_INTR_2,
                                                (sl_gpio_interrupt_flag_t)SL_GPIO_INTERRUPT_RISE_EDGE,
                                                (sl_gpio_irq_callback_t)&gpio_uulp_pin_interrupt_callback,
                                                AVL_INTR_NO);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_gpio_configure_interrupt, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("GPIO driver configure uulp interrupt is successful \r\n");
  } while (false);
}
/*******************************************************************************
 * Function will run continuously in while loop 
 ******************************************************************************/
void gpio_uulp_example_process_action(void)
{
}

/*******************************************************************************
 *  This API handles UULP GPIO pin interrupt 0 request
 ******************************************************************************/
static void gpio_uulp_pin_interrupt_callback(uint32_t pin_intr)
{
  if (pin_intr == UULP_GPIO_INTR_2) {
    // This is with respect to ISR context. Debugout might cause issues sometimes.
    DEBUGOUT("gpio uulp pin interrupt2\r\n");
  }
}
