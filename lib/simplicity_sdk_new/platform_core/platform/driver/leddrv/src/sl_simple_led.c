/***************************************************************************//**
 * @file
 * @brief Simple LED Driver
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_simple_led.h"
#include "sl_gpio.h"
#include "sl_clock_manager.h"

sl_status_t sl_simple_led_init(void *context)
{
  sl_simple_led_context_t *led = context;
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);

  sl_gpio_t gpio;
  gpio.port = led->port;
  gpio.pin = led->pin;
  sl_gpio_set_pin_mode(&gpio,
                       SL_GPIO_MODE_PUSH_PULL,
                       !led->polarity);
  return SL_STATUS_OK;
}

void sl_simple_led_turn_on(void *context)
{
  sl_simple_led_context_t *led = context;

  sl_gpio_t gpio;
  gpio.port = led->port;
  gpio.pin = led->pin;
  if (led->polarity == SL_SIMPLE_LED_POLARITY_ACTIVE_LOW) {
    sl_gpio_clear_pin(&gpio);
  } else {
    sl_gpio_set_pin(&gpio);
  }
}

void sl_simple_led_turn_off(void *context)
{
  sl_simple_led_context_t *led = context;

  sl_gpio_t gpio;
  gpio.port = led->port;
  gpio.pin = led->pin;
  if (led->polarity == SL_SIMPLE_LED_POLARITY_ACTIVE_LOW) {
    sl_gpio_set_pin(&gpio);
  } else {
    sl_gpio_clear_pin(&gpio);
  }
}

void sl_simple_led_toggle(void *context)
{
  sl_simple_led_context_t *led = context;

  sl_gpio_t gpio;
  gpio.port = led->port;
  gpio.pin = led->pin;
  sl_gpio_toggle_pin(&gpio);
}

sl_led_state_t sl_simple_led_get_state(void *context)
{
  sl_simple_led_context_t *led = context;
  sl_led_state_t value;

  sl_gpio_t gpio;
  bool pin_value;
  gpio.port = led->port;
  gpio.pin = led->pin;
  sl_gpio_get_pin_output(&gpio, &pin_value);
  value = (sl_led_state_t)pin_value;

  if (led->polarity == SL_SIMPLE_LED_POLARITY_ACTIVE_LOW) {
    return !value;
  } else {
    return value;
  }
}
