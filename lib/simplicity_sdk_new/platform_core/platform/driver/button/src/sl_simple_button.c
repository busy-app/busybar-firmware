/***************************************************************************//**
 * @file
 * @brief Simple Button Driver
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

#include "sl_simple_button.h"
#include "sl_simple_button_config.h"
#include "sl_clock_manager.h"

#if (SL_SIMPLE_BUTTON_DEBOUNCE_BITS < 1U)
#undef SL_SIMPLE_BUTTON_DEBOUNCE_BITS
#define SL_SIMPLE_BUTTON_DEBOUNCE_BITS  1U
#endif
#if (SL_SIMPLE_BUTTON_DEBOUNCE_BITS > 15U)
#undef SL_SIMPLE_BUTTON_DEBOUNCE_BITS
#define SL_SIMPLE_BUTTON_DEBOUNCE_BITS  15U
#endif

static const uint16_t check_press = (uint16_t)(0xffff << SL_SIMPLE_BUTTON_DEBOUNCE_BITS);
static const uint16_t check_release = (uint16_t)(~(0x1 << SL_SIMPLE_BUTTON_DEBOUNCE_BITS));
static const uint16_t debounce_window = (uint16_t)(0xffff << (SL_SIMPLE_BUTTON_DEBOUNCE_BITS + 1));

/***************************************************************************//**
 * An internal callback called in interrupt context whenever a button changes
 * its state. (mode - SL_SIMPLE_BUTTON_MODE_INTERRUPT)
 *
 * @note The button state is updated by this function. The application callback
 * should not update it again.
 *
 * @param[in] interrupt_no      Interrupt number (pin number)
 * @param[in] ctx               Pointer to button handle
 ******************************************************************************/
static void sli_simple_button_on_change(uint8_t interrupt_no, void *ctx)
{
  (void)interrupt_no;
  sl_button_t *button = (sl_button_t *)ctx;
  sl_simple_button_context_t *simple_button = button->context;
  sl_gpio_t gpio = {
    .port = simple_button->port,
    .pin = simple_button->pin
  };
  bool pin_value;

  if (simple_button->state != SL_SIMPLE_BUTTON_DISABLED) {
    sl_gpio_get_pin_input(&gpio, &pin_value);
    simple_button->state = ((bool)pin_value == SL_SIMPLE_BUTTON_POLARITY);

    sl_button_on_change(button);
  }
}

sl_status_t sl_simple_button_init(const sl_button_t *handle)
{
  int32_t interrupt_em4, interrupt_ext;
  sl_status_t status;
  sl_button_t *button = (sl_button_t *)handle;
  sl_simple_button_context_t *simple_button = button->context;
  sl_gpio_t gpio = {
    .port = simple_button->port,
    .pin = simple_button->pin
  };
  bool pin_value;

  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);

  sl_gpio_set_pin_mode(&gpio, SL_SIMPLE_BUTTON_GPIO_MODE, SL_SIMPLE_BUTTON_GPIO_DOUT);
  sl_gpio_get_pin_input(&gpio, &pin_value);
  simple_button->state = ((bool)pin_value == SL_SIMPLE_BUTTON_POLARITY);

  if (simple_button->mode == SL_SIMPLE_BUTTON_MODE_INTERRUPT) {
    interrupt_em4 = SL_GPIO_INTERRUPT_UNAVAILABLE;
    interrupt_ext = SL_GPIO_INTERRUPT_UNAVAILABLE;
    // Try to register an EM4WU interrupt for the given pin
    status = sl_gpio_configure_wakeup_em4_interrupt(&gpio,
                                                    &interrupt_em4,
                                                    SL_SIMPLE_BUTTON_POLARITY,
                                                    (sl_gpio_irq_callback_t)sli_simple_button_on_change,
                                                    button);
    if (interrupt_em4 == SL_GPIO_INTERRUPT_UNAVAILABLE) {
      // if the pin not EM4WU-compatible, instead register a regualr interrupt
      status = sl_gpio_configure_external_interrupt(&gpio,
                                                    &interrupt_ext,
                                                    SL_GPIO_INTERRUPT_RISING_FALLING_EDGE,
                                                    (sl_gpio_irq_callback_t)sli_simple_button_on_change,
                                                    button);
      EFM_ASSERT(status == SL_STATUS_OK);
    } else {
      // If the pin is EM4WU-compatible, setup the pin as an EM4WU pin
      // Since EM4WU interrupts are level-sensitive and not edge-sensitive, also register a regular edge-sensitive interrupt to capture the other edge
      uint8_t flags;
      if (SL_SIMPLE_BUTTON_POLARITY == 0) {
        flags = SL_GPIO_INTERRUPT_RISING_EDGE;
      } else if (SL_SIMPLE_BUTTON_POLARITY == 1) {
        flags = SL_GPIO_INTERRUPT_FALLING_EDGE;
      }
      status = sl_gpio_configure_external_interrupt(&gpio,
                                                    &interrupt_ext,
                                                    flags,
                                                    (sl_gpio_irq_callback_t)sli_simple_button_on_change,
                                                    button);
      EFM_ASSERT(status == SL_STATUS_OK);
    }
  }

  return SL_STATUS_OK;
}

sl_button_state_t sl_simple_button_get_state(const sl_button_t *handle)
{
  sl_button_t *button = (sl_button_t *)handle;
  sl_simple_button_context_t *simple_button = button->context;

  return simple_button->state;
}

void sl_simple_button_poll_step(const sl_button_t *handle)
{
  sl_button_t *button = (sl_button_t *)handle;
  sl_simple_button_context_t *simple_button = button->context;
  bool button_press, pin_value;
  sl_gpio_t gpio = {
    .port = simple_button->port,
    .pin = simple_button->pin
  };

  if (simple_button->state == SL_SIMPLE_BUTTON_DISABLED) {
    return;
  }

  sl_gpio_get_pin_input(&gpio, &pin_value);
  button_press = (bool)pin_value;

  if (simple_button->mode == SL_SIMPLE_BUTTON_MODE_POLL_AND_DEBOUNCE) {
    uint16_t history = simple_button->history;
    history = (history << 1) | (button_press ^ SL_SIMPLE_BUTTON_POLARITY) | (debounce_window);

    if (history == check_press) {
      simple_button->state = SL_SIMPLE_BUTTON_PRESSED;
    }
    if (history == check_release) {
      simple_button->state = SL_SIMPLE_BUTTON_RELEASED;
    }

    simple_button->history = history;
  } else if (simple_button->mode == SL_SIMPLE_BUTTON_MODE_POLL) {
    simple_button->state = (button_press == SL_SIMPLE_BUTTON_POLARITY);
  }
}

void sl_simple_button_enable(const sl_button_t *handle)
{
  sl_button_t *button = (sl_button_t *)handle;
  sl_simple_button_context_t *simple_button = button->context;

  // Return if the button is not disabled
  if (simple_button->state != SL_SIMPLE_BUTTON_DISABLED) {
    return;
  }

  // Clear history
  simple_button->history = 0;
  // Reinit button
  sl_simple_button_init(handle);
}

void sl_simple_button_disable(const sl_button_t *handle)
{
  sl_button_t *button = (sl_button_t *)handle;
  sl_simple_button_context_t *simple_button = button->context;

  // Return if the button is disabled
  if (simple_button->state == SL_SIMPLE_BUTTON_DISABLED) {
    return;
  }
  if (simple_button->mode == SL_SIMPLE_BUTTON_MODE_INTERRUPT) {
    sl_gpio_deconfigure_external_interrupt(simple_button->pin);
  }
  // Disable the button
  simple_button->state = SL_SIMPLE_BUTTON_DISABLED;
}
