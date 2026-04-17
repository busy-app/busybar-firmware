/***************************************************************************//**
 * @file
 * @brief EZSP GPIO Activation component for Silicon Labs bootloader.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "config/btl_config.h"

#include "em_device.h"
#include "sl_hal_gpio.h"
#include "btl_ezsp_gpio_activation.h"
#include "btl_ezsp_gpio_activation_cfg.h"

// Map GPIO activation polarity settings to GPIO pin states
#define HIGH 0
#define LOW  1

bool ezsp_gpio_enterBootloader(void)
{
  bool pressed;
#if defined(_CMU_CLKEN0_MASK)
  // Enable GPIO clock
  CMU->CLKEN0_SET = CMU_CLKEN0_GPIO;
#endif
  const sl_gpio_t gpio = { .port = SL_EZSPSPI_WAKE_INT_PORT,
                           .pin  = SL_EZSPSPI_WAKE_INT_PIN };
  // Reconfigure as an input with pull(up|down) to read the nWake state
  sl_hal_gpio_set_pin_mode(&gpio,
                           SL_GPIO_MODE_INPUT_PULL,
                           SL_EZSP_GPIO_ACTIVATION_POLARITY);

  pressed = sl_hal_gpio_get_pin_input(&gpio)
            != SL_EZSP_GPIO_ACTIVATION_POLARITY;

  // Disable GPIO pin
  sl_hal_gpio_set_pin_mode(&gpio,
                           SL_GPIO_MODE_DISABLED,
                           SL_EZSP_GPIO_ACTIVATION_POLARITY);

#if defined(_CMU_CLKEN0_MASK)
  // Disable GPIO clock for Series 3
  CMU->CLKEN0_CLR = CMU_CLKEN0_GPIO;
#endif

  return pressed;
}
