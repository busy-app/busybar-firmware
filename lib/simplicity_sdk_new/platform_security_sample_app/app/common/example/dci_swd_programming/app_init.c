/***************************************************************************//**
 * @file app_init.c
 * @brief Application initialization functions.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "app_init.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * The function is used for some basic initialization relates to the
 * application.
 ******************************************************************************/
void app_init(void)
{
  // Enable cycle counter
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  // Initialize GPIO for SWCLK and SWDIO pin
  sl_gpio_t swclk_gpio = {
    .port = (sl_gpio_port_t)SWCLK_PORT,
    .pin = SWCLK_PIN,
  };
  sl_gpio_t swdio_gpio = {
    .port = (sl_gpio_port_t)SWDIO_PORT,
    .pin = SWDIO_PIN,
  };

  // High drive for SWCLK and SWD, both must on the same GPIO port
  sl_hal_gpio_set_slew_rate(&swclk_gpio, 6);
  sl_hal_gpio_set_slew_rate_alternate((sl_gpio_port_t)SWCLK_PORT, 6);

  sl_gpio_set_pin_mode(&swclk_gpio, SL_GPIO_MODE_PUSH_PULL, 0);
  sl_gpio_set_pin_mode(&swdio_gpio, SL_GPIO_MODE_PUSH_PULL, 1);

  // Initialize GPIO for RESET pin
  sl_gpio_t reset_gpio = {
    .port = (sl_gpio_port_t)RESET_PORT,
    .pin = RESET_PIN,
  };

  sl_gpio_set_pin_mode(&reset_gpio, SL_GPIO_MODE_WIRED_AND, 1);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
