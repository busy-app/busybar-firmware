/***************************************************************************/ /**
 * @file sl_si91x_led.c
 * @brief LED manipulation routines; stack and example APIs
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_si91x_led.h"
#include "si91x_device.h"
#include "sl_driver_gpio.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_si91x_led_config.h"

void sl_si91x_led_init(const sl_led_t *handle)
{
#ifdef SI917Y_DEVKIT
  if (handle->led_number == 0U) {
    /*Enable clock*/
    sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)M4CLK_GPIO);
  } else {
    /*Enable clock*/
    sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)ULPCLK_GPIO);
  }
#else
  if (handle->led_number == 0U) {
    /*Enable clock*/
    sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)ULPCLK_GPIO);
  } else {
    /*Enable clock*/
    sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)M4CLK_GPIO);
  }
#endif
  sl_si91x_gpio_pin_config_t sl_gpio_pin_config = { { handle->port, handle->pin }, GPIO_OUTPUT };
  sl_gpio_set_configuration(sl_gpio_pin_config);
}

void sl_si91x_led_set(uint8_t pin)
{
  sl_gpio_t led_gpio_port_pin;
  led_gpio_port_pin.pin = pin;
  if (pin == SL_LED_LED0_PIN) {
    led_gpio_port_pin.port = SL_LED_LED0_PORT;
  } else {
    led_gpio_port_pin.port = SL_LED_LED1_PORT;
  }
  sl_gpio_driver_set_pin(&led_gpio_port_pin);
}

void sl_si91x_led_clear(uint8_t pin)
{
  sl_gpio_t led_gpio_port_pin;
  led_gpio_port_pin.pin = pin;
  if (pin == SL_LED_LED0_PIN) {
    led_gpio_port_pin.port = SL_LED_LED0_PORT;
  } else {
    led_gpio_port_pin.port = SL_LED_LED1_PORT;
  }
  sl_gpio_driver_clear_pin(&led_gpio_port_pin);
}

void sl_si91x_led_toggle(uint8_t pin)
{
  sl_gpio_t led_gpio_port_pin;
  led_gpio_port_pin.pin = pin;
  if (pin == SL_LED_LED0_PIN) {
    led_gpio_port_pin.port = SL_LED_LED0_PORT;
  } else {
    led_gpio_port_pin.port = SL_LED_LED1_PORT;
  }
  sl_gpio_driver_toggle_pin(&led_gpio_port_pin);
}
