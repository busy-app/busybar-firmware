/*******************************************************************************
 * @file  gpio_detailed_example.c
 * @brief  gpio detailed example
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
 * his application demonstrates the toggling of HP instance GPIO at intervals of
 * 1 second and provides usage of additional APIs.
 ============================================================================**/
#include "gpio_detailed_example.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_gpio_board.h"
#include "rsi_debug.h"
#include "sl_si91x_clock_manager.h"
/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define DELAY                1000 // Delay for 1sec
#define GPIO_PORT_GROUP_ABCD 0    // Set to 1 to enable GPIO Ports A/B/C/D configuration and control
/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/*******************************************************************************
 *************************** LOCAL VARIABLES   *********************************
 ******************************************************************************/
typedef sl_gpio_t sl_si91x_gpio_t;

// Define a configuration structure for GPIO pins that are required, specifying its port, pin number and direction.
// Below configurations is for GPIO 10. Here SL_SI91X_GPIO_10_PORT corresponds to GPIO pin 10's port number : 0. This
// is defined to '0' which is PORT_A and  SL_SI91X_GPIO_10_PIN refers to GPIO pin number 10. Same representation followed
// for other port and pins.
static sl_si91x_gpio_pin_config_t sl_gpio_pin_config = { { SL_SI91X_GPIO_10_PORT, SL_SI91X_GPIO_10_PIN }, GPIO_OUTPUT };
#if (GPIO_PORT_GROUP_ABCD == 1)
// GPIO configuration array for multiple ports
static sl_si91x_gpio_pin_config_t gpio_multi_port_configs[] = {
  // GPIO 6 - Port A, Pin 6
  { { SL_GPIO_PORT_A, GPIO_PIN_NUMBER6 }, GPIO_OUTPUT },
  // GPIO 27 - Port B, Pin 11 (Physical pin 27)
  { { SL_GPIO_PORT_B, GPIO_PIN_NUMBER11 }, GPIO_OUTPUT },
  // GPIO 47 - Port C, Pin 15 (Physical pin 47)
  { { SL_GPIO_PORT_C, GPIO_PIN_NUMBER15 }, GPIO_OUTPUT },
  // GPIO 49 - Port D, Pin 1 (Physical pin 49)
  { { SL_GPIO_PORT_D, GPIO_PIN_NUMBER1 }, GPIO_OUTPUT }
};

#define GPIO_MULTI_PORT_COUNT (sizeof(gpio_multi_port_configs) / sizeof(gpio_multi_port_configs[0]))

// Port names for debug messages
static const char gpio_port_names[] = { 'A', 'B', 'C', 'D' };
#endif
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * GPIO example initialization function. It initializes clock, pin mode,
 * direction, driver strength, disable state
 ******************************************************************************/
void gpio_detailed_example_init(void)
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
    // Configure GPIO pin 10 using driver pin configuration API.
    // Using this API by default GPIO mode is set as MODE 0. If any other mode is selected for any GPIO use
    // corresponding API sl_gpio_driver_set_pin_mode() is for mode setting.
    status = sl_gpio_set_configuration(sl_gpio_pin_config);
    if (status != SL_STATUS_OK) {
      // Prints if pin configuration fails
      DEBUGOUT("sl_gpio_set_configuration, Error code: %lu\r\n", status);
      break; // breaks if error occurs
    }
#if (GPIO_PORT_GROUP_ABCD == 1)
    // Configure multiple GPIO ports using array-based approach
    for (uint8_t i = 0; i < GPIO_MULTI_PORT_COUNT; i++) {
      status = sl_gpio_set_configuration(gpio_multi_port_configs[i]);
      if (status != SL_STATUS_OK) {
        // Prints if pin configuration fails with port identification
        DEBUGOUT("sl_gpio_set_configuration Port %c Pin %d, Error code: %lu\r\n",
                 gpio_port_names[i],
                 gpio_multi_port_configs[i].port_pin.pin,
                 status);
        break; // breaks if error occurs
      }
      DEBUGOUT("GPIO driver Port %c Pin %d configuration is successful \r\n",
               gpio_port_names[i],
               gpio_multi_port_configs[i].port_pin.pin);
    }
    if (status != SL_STATUS_OK) {
      break; // Exit if any multi-port configuration failed
    }
#endif
    DEBUGOUT("GPIO driver set pin configuration is successful \r\n");
    // Apart from above APIs, there are additional APIs that can be used as per requirement
    //  like to increase drive strength, to make pin pull up/ pull down, increase slew rate etc.
    // Set pad driver strength of GPIO pin 10 as GPIO_FOUR_MILLI_AMPS using the driver pad driver strength API.
    //  By default the pad driver strength is set to GPIO_TWO_MILLI_AMPS.
    status =
      sl_si91x_gpio_driver_select_pad_driver_strength(sl_gpio_pin_config.port_pin.pin,
                                                      (sl_si91x_gpio_driver_strength_select_t)GPIO_FOUR_MILLI_AMPS);
    if (status != SL_STATUS_OK) {
      // Prints if setting pad driver strength fails
      DEBUGOUT("sl_si91x_gpio_driver_select_pad_driver_strength, Error code: %lu\r\n", status);
      break; // breaks if error occurs
    }
    DEBUGOUT("GPIO driver pad driver strength select is successful \r\n");
    // Select pad driver disable state for GPIO pin 10. By default it is set to GPIO_HZ(High Impedance state). Change can be done as per one requirement.
    status = sl_si91x_gpio_driver_select_pad_driver_disable_state(sl_gpio_pin_config.port_pin.pin,
                                                                  (sl_si91x_gpio_driver_disable_state_t)GPIO_HZ);
    if (status != SL_STATUS_OK) {
      // Prints if setting pad driver disable state fails
      DEBUGOUT("sl_si91x_gpio_driver_select_pad_driver_disable_state, Error code: %lu\r\n", status);
      break; // breaks if error occurs
    }
    DEBUGOUT("GPIO driver pad driver disable state select is successful \r\n");
  } while (false);
}
/*******************************************************************************
 * Function will run continuously in while loop and led toggles for every 1sec
 ******************************************************************************/
void gpio_detailed_example_process_action(void)
{
  sl_status_t status;
  status = sl_gpio_driver_toggle_pin(&sl_gpio_pin_config.port_pin); // Toggle HP GPIO pin 10
  if (status != SL_STATUS_OK) {
    // Prints if toggling pin fails
    DEBUGOUT("sl_gpio_toggle_pin, Error code: %lu\r\n", status);
    return;
  }
#if (GPIO_PORT_GROUP_ABCD == 1)
  // Toggle multiple GPIO ports using array-based approach
  for (uint8_t i = 0; i < GPIO_MULTI_PORT_COUNT; i++) {
    status = sl_gpio_driver_toggle_pin(&gpio_multi_port_configs[i].port_pin);
    if (status != SL_STATUS_OK) {
      // Prints if toggling pin fails with port identification
      DEBUGOUT("sl_gpio_toggle_pin Port %c Pin %d, Error code: %lu\r\n",
               gpio_port_names[i],
               gpio_multi_port_configs[i].port_pin.pin,
               status);
      return;
    }
  }
#endif
  // Prints indicating successful pin toggle
  DEBUGOUT("HP GPIO driver toggle pin is successful \r\n");
  sl_si91x_delay_ms(DELAY); // Delay of 1sec
}
