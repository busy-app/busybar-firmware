/***************************************************************************//**
 * @file
 * @brief GPIO Activation component for Silicon Labs bootloader.
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
#include "btl_gpio_activation.h"
#include "btl_gpio_activation_cfg.h"
// Map GPIO activation polarity settings to GPIO pin states
#define HIGH 0
#define LOW  1

bool gpio_enterBootloader(void)
{
  bool pressed;

#if defined(_CMU_CLKEN0_MASK)
  // Enable GPIO clock
  CMU->CLKEN0_SET = CMU_CLKEN0_GPIO;
#endif
  const sl_gpio_t gpio = { .port = SL_BTL_BUTTON_PORT,
                           .pin  = SL_BTL_BUTTON_PIN };
  // Since the button may have decoupling caps, they may not be charged
  // after a power-on and could give a false positive result. To avoid
  // this issue, drive the output as an output for a short time to charge
  // them up as quickly as possible.
  sl_hal_gpio_set_pin_mode(&gpio,
                           SL_GPIO_MODE_PUSH_PULL,
                           SL_GPIO_ACTIVATION_POLARITY);
  for (volatile int i = 0; i < 100; i++) {
    // Do nothing
  }

  // Reconfigure as an input with pull(up|down) to read the button state
  sl_hal_gpio_set_pin_mode(&gpio,
                           SL_GPIO_MODE_INPUT_PULL,
                           SL_GPIO_ACTIVATION_POLARITY);

  // We have to delay again here so that if the button is depressed the
  // cap has time to discharge again after being charged up by the above delay
  for (volatile int i = 0; i < 500; i++) {
    // Do nothing
  }

  pressed = sl_hal_gpio_get_pin_input(&gpio)
            != SL_GPIO_ACTIVATION_POLARITY;

  // Disable GPIO pin
  sl_hal_gpio_set_pin_mode(&gpio,
                           SL_GPIO_MODE_DISABLED,
                           SL_GPIO_ACTIVATION_POLARITY);

  return pressed;
}
