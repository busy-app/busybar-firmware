/*******************************************************************************
 * @file  gpio_group_example.c
 * @brief GPIO group example
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
 * This application demonstrates the usage of HP, ULP group interrupt.
 ============================================================================**/
#include "gpio_group_example.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_gpio_board.h"
#include "rsi_debug.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define PIN_COUNT      2 // Number of interrupts needed
#define GRP_COUNT      2 // Count of group interrupt pins
#define NUMBER_OF_PINS 4 //Total number of pins for pin configuration set
/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/*******************************************************************************
 *************************** LOCAL VARIABLES   *********************************
 ******************************************************************************/
// Define GPIO port and pin. Here SL_SI91X_GPIO_8_PORT corresponds to GPIO pin 8's port number : 0.
// This is defined to '0' which is PORT_A and SL_SI91X_GPIO_8_PIN refers to GPIO pin number 8.
// Same representation followed for other port and pins.
static sl_si91x_gpio_pin_config_t sl_gpio_pin_config[NUMBER_OF_PINS] = {
  { { SL_SI91X_GPIO_8_PORT, SL_SI91X_GPIO_8_PIN }, GPIO_INPUT },
  { { SL_SI91X_GPIO_9_PORT, SL_SI91X_GPIO_9_PIN }, GPIO_INPUT },
  { { SL_SI91X_ULP_GPIO_8_PORT, SL_SI91X_ULP_GPIO_8_PIN }, GPIO_INPUT },
#ifndef SL_SI91X_ACX_MODULE
  { { SL_SI91X_ULP_GPIO_10_PORT, SL_SI91X_ULP_GPIO_10_PIN }, GPIO_INPUT },
#else
  { { SL_SI91X_ULP_GPIO_4_PORT, SL_SI91X_ULP_GPIO_4_PIN }, GPIO_INPUT },
#endif
};
static sl_si91x_gpio_group_interrupt_config_t config_grp_int, ulp_config_grp_int;
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void gpio_group_interrupt0_callback(uint32_t grp_intr);
static void gpio_ulp_group_interrupt_callback(uint32_t pin_intr);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * GPIO example initialization function. It initializes clock, pin mode,
 * direction and configures pin interrupt
 ******************************************************************************/
void gpio_group_example_init(void)
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
    // Configure GPIO pin 11 using pin configuration API.
    // Using this API by default GPIO mode is set as MODE 0. If any other mode is selected for any GPIO use
    // corresponding API sl_gpio_driver_set_pin_mode() is for mode setting.
    for (uint8_t i = 0; i < (sizeof(sl_gpio_pin_config) / sizeof(sl_gpio_pin_config[0])); i++) {
      status = sl_gpio_set_configuration(sl_gpio_pin_config[i]);
      if (status != SL_STATUS_OK) {
        // Prints if pin configuration fails
        DEBUGOUT("sl_gpio_set_configuration, Error code: %lu\r\n", status);
        break; // breaks if error occurs
      }
      DEBUGOUT("GPIO driver set pin configuration is successful \r\n");
    }
    uint8_t group_pins[PIN_COUNT] = { sl_gpio_pin_config[0].port_pin.pin,
                                      sl_gpio_pin_config[1].port_pin.pin }; // pins for group interrupt
    uint8_t group_port[PIN_COUNT] = { sl_gpio_pin_config[0].port_pin.port,
                                      sl_gpio_pin_config[1].port_pin.port }; // port selected for group interrupt
    uint8_t group_pol[PIN_COUNT]  = { GPIO_POLARITY_0, GPIO_POLARITY_0 };    // polarity selected for group interrupt
    // Configure group interrupt parameters
    config_grp_int.grp_interrupt     = GROUP_INT_1; // Set group interrupt 1
    config_grp_int.grp_interrupt_cnt = GRP_COUNT;   // Count of group interrupt pins
    config_grp_int.and_or            = GPIO_AND;    // AND/OR of group interrupt
    config_grp_int.level_edge        = GPIO_LEVEL;  // Level/Edge of group interrupt
    memcpy(config_grp_int.grp_interrupt_pin, group_pins, PIN_COUNT);
    memcpy(config_grp_int.grp_interrupt_port, group_port, PIN_COUNT);
    memcpy(config_grp_int.grp_interrupt_pol, group_pol, PIN_COUNT);

    // Configure group interrupt for grp_config_int structure. The group interrupt in this application is performed using external triggering.
    // Connect GPIO8(F8) and GPIO9(F9) to low/high. When AND event is selected then interrupt occurs when both pins are high/low at a time.
    // When OR event is selected then interrupt occurs when either one pin is low/high based on level/edge.
    status = sl_si91x_gpio_driver_configure_group_interrupt(&config_grp_int,
                                                            (sl_gpio_irq_callback_t)&gpio_group_interrupt0_callback);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_configure_group_interrupt, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("GPIO driver configure group interrupt is successful \r\n");

    uint8_t ulp_group_port[PIN_COUNT] = { sl_gpio_pin_config[2].port_pin.port,
                                          sl_gpio_pin_config[3].port_pin.port }; // port selected for group interrupt
    uint8_t ulp_group_pins[PIN_COUNT] = { sl_gpio_pin_config[2].port_pin.pin,
                                          sl_gpio_pin_config[3].port_pin.pin }; // pins for group interrupt
    uint8_t ulp_group_pol[PIN_COUNT]  = { GPIO_POLARITY_0, GPIO_POLARITY_0 };   // polarity selected for group interrupt

    // Configure group interrupt parameters
    ulp_config_grp_int.grp_interrupt     = GROUP_INT_1; // Set group interrupt 1
    ulp_config_grp_int.grp_interrupt_cnt = GRP_COUNT;   // Count of group interrupt pins
    ulp_config_grp_int.and_or            = GPIO_AND;    // AND/OR of group interrupt
    ulp_config_grp_int.level_edge        = GPIO_LEVEL;  // Level/Edge of group interrupt
    memcpy(ulp_config_grp_int.grp_interrupt_pin, ulp_group_pins, PIN_COUNT);
    memcpy(ulp_config_grp_int.grp_interrupt_port, ulp_group_port, PIN_COUNT);
    memcpy(ulp_config_grp_int.grp_interrupt_pol, ulp_group_pol, PIN_COUNT);

    // Configure group interrupt for grp_config_int structure. The group interrupt in this application is performed using external triggering.
    // Connect ULP GPIO8(P15) and ULP GPIO10(P17) to low/high. When AND event is selected then interrupt occurs when both pins are high/low at a time.
    // When OR event is selected then interrupt occurs when either one pin is low/high based on level/edge.
    status =
      sl_si91x_gpio_driver_configure_ulp_group_interrupt(&ulp_config_grp_int,
                                                         (sl_gpio_irq_callback_t)&gpio_ulp_group_interrupt_callback);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_configure_ulp_group_interrupt, Error code: %lu\r\n", status);
      break;
    }
    DEBUGOUT("GPIO driver configure ulp group interrupt is successful \r\n");
  } while (false);
}
/*******************************************************************************
 * Function will run continuously in while loop 
 ******************************************************************************/
void gpio_group_example_process_action(void)
{
}

/*******************************************************************************
 *  This API handles GPIO Group interrupt 0 request
 ******************************************************************************/
static void gpio_group_interrupt0_callback(uint32_t grp_intr)
{
  if (grp_intr == GROUP_INT_1) {
    // This is with respect to ISR context. Debugout might cause issues sometimes.
    DEBUGOUT("gpio group interrupt1\r\n");
  }
}

/*******************************************************************************
 *  This API handles ULP GPIO group interrupt request
 ******************************************************************************/
static void gpio_ulp_group_interrupt_callback(uint32_t pin_intr)
{
  if (pin_intr == ULP_GROUP_INTR_0) {
    // This is with respect to ISR context. Debugout might cause issues sometimes.
    DEBUGOUT("gpio ulp group interrupt0\r\n");
  }
}
