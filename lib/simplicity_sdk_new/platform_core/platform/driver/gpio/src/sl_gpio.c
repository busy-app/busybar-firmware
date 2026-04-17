/***************************************************************************//**
 * @file
 * @brief General Purpose IO (GPIO) driver API
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <stddef.h>
#include "sl_core.h"
#include "sl_common.h"
#include "sl_interrupt_manager.h"
#include "sl_clock_manager.h"
#include "sl_hal_gpio.h"
#include "sl_gpio.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/// Define for supporting gpiointerrupt porting
#define SL_GPIO_PORT_INTERRUPT  (0xFF)

/// Pin direction validation.
#define SL_GPIO_DIRECTION_IS_VALID(direction)  (direction <= SL_GPIO_PIN_DIRECTION_OUT)

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

typedef struct {
  void *callback;   // Pointer to callback function.
  void *context;    // Pointer to callback context.
} sl_gpio_callback_desc_t;

typedef struct {
  // An array of user callbacks for external interrupts.
  // We have external interrupts configured from 0 to 15 bits.
  sl_gpio_callback_desc_t callback_ext[SL_HAL_GPIO_INTERRUPT_MAX];
  // An array of user callbacks for EM4 interrupts.
  // We have EM4 interrupts configured from 16 to 31 bits.
  sl_gpio_callback_desc_t callback_em4[SL_HAL_GPIO_INTERRUPT_MAX];
} sl_gpio_callbacks_t;

/*******************************************************************************
 ********************************   GLOBALS   **********************************
 ******************************************************************************/

// Variable to manage and organize the callback functions for External and EM4 interrupts.
static sl_gpio_callbacks_t gpio_interrupts = { 0 };

/*******************************************************************************
 ******************************   LOCAL FUCTIONS   *****************************
 ******************************************************************************/
static void sl_gpio_dispatch_interrupt(uint32_t iflags);

/***************************************************************************//**
 *   Driver GPIO Initialization.
 ******************************************************************************/
sl_status_t sl_gpio_init()
{
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);

#if defined(SL_HAL_GPIO_IRQN_SUPPORTED)
  if (sl_interrupt_manager_is_irq_disabled(GPIO_ODD_IRQn)) {
    sl_interrupt_manager_clear_irq_pending(GPIO_ODD_IRQn);
    sl_interrupt_manager_enable_irq(GPIO_ODD_IRQn);
  }
  if (sl_interrupt_manager_is_irq_disabled(GPIO_EVEN_IRQn)) {
    sl_interrupt_manager_clear_irq_pending(GPIO_EVEN_IRQn);
    sl_interrupt_manager_enable_irq(GPIO_EVEN_IRQn);
  }
#endif

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *   Sets the pin direction for GPIO pin.
 ******************************************************************************/
sl_status_t sl_gpio_set_pin_direction(const sl_gpio_t *gpio,
                                      sl_gpio_pin_direction_t pin_direction)
{
  CORE_DECLARE_IRQ_STATE;

  if (gpio == NULL) {
    EFM_ASSERT(false);
    return SL_STATUS_NULL_POINTER;
  }
  EFM_ASSERT(SL_HAL_GPIO_PORT_PIN_IS_VALID(gpio->port, gpio->pin));
  EFM_ASSERT(SL_GPIO_DIRECTION_IS_VALID(pin_direction));

  CORE_ENTER_ATOMIC();

  if (pin_direction == SL_GPIO_PIN_DIRECTION_OUT) {
    sl_hal_gpio_set_pin_mode(gpio, SL_GPIO_MODE_PUSH_PULL, 1);
  } else if (pin_direction == SL_GPIO_PIN_DIRECTION_IN) {
    sl_hal_gpio_set_pin_mode(gpio, SL_GPIO_MODE_INPUT, 0);
  }

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *   Sets the mode for GPIO pin and pin direction.
 ******************************************************************************/
sl_status_t sl_gpio_set_pin_mode(const sl_gpio_t *gpio,
                                 sl_gpio_mode_t mode,
                                 bool output_value)
{
  CORE_DECLARE_IRQ_STATE;

  if (gpio == NULL) {
    EFM_ASSERT(false);
    return SL_STATUS_NULL_POINTER;
  }
  EFM_ASSERT(SL_HAL_GPIO_MODE_IS_VALID(mode));
  EFM_ASSERT(SL_HAL_GPIO_PORT_PIN_IS_VALID(gpio->port, gpio->pin));

  CORE_ENTER_ATOMIC();

  sl_hal_gpio_set_pin_mode(gpio, mode, output_value);

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Gets the current configuration selected pin on selected port.
 ******************************************************************************/
sl_status_t sl_gpio_get_pin_config(const sl_gpio_t *gpio,
                                   sl_gpio_pin_config_t *pin_config)
{
  CORE_DECLARE_IRQ_STATE;

  if (gpio == NULL || pin_config == NULL) {
    EFM_ASSERT(false);
    return SL_STATUS_NULL_POINTER;
  }
  EFM_ASSERT(SL_HAL_GPIO_PORT_PIN_IS_VALID(gpio->port, gpio->pin));

  CORE_ENTER_ATOMIC();

  pin_config->mode = sl_hal_gpio_get_pin_mode(gpio);
  switch (pin_config->mode) {
    case SL_GPIO_MODE_INPUT:
    case SL_GPIO_MODE_INPUT_PULL:
    case SL_GPIO_MODE_INPUT_PULL_FILTER:
      pin_config->direction = SL_GPIO_PIN_DIRECTION_IN;
      break;

    case SL_GPIO_MODE_DISABLED:
    case SL_GPIO_MODE_PUSH_PULL:
    case SL_GPIO_MODE_PUSH_PULL_ALTERNATE:
    case SL_GPIO_MODE_WIRED_OR:
    case SL_GPIO_MODE_WIRED_OR_PULL_DOWN:
    case SL_GPIO_MODE_WIRED_AND:
    case SL_GPIO_MODE_WIRED_AND_FILTER:
    case SL_GPIO_MODE_WIRED_AND_PULLUP:
    case SL_GPIO_MODE_WIRED_AND_PULLUP_FILTER:
    case SL_GPIO_MODE_WIRED_AND_ALTERNATE:
    case SL_GPIO_MODE_WIRED_AND_ALTERNATE_FILTER:
    case SL_GPIO_MODE_WIRED_AND_ALTERNATE_PULLUP:
    case SL_GPIO_MODE_WIRED_AND_ALTERNATE_PULLUP_FILTER:
      pin_config->direction = SL_GPIO_PIN_DIRECTION_OUT;
      break;

    default:
      CORE_EXIT_ATOMIC();
      EFM_ASSERT(false);
      return SL_STATUS_INVALID_MODE;
  }

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Sets the DOUT of selected pin on selected port.
 ******************************************************************************/
sl_status_t sl_gpio_set_pin(const sl_gpio_t *gpio)
{
  if (gpio == NULL) {
    EFM_ASSERT(false);
    return SL_STATUS_NULL_POINTER;
  }
  EFM_ASSERT(SL_HAL_GPIO_PORT_PIN_IS_VALID(gpio->port, gpio->pin));

  sl_hal_gpio_set_pin(gpio);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Clears the DOUT of selected pin on selected port.
 ******************************************************************************/
sl_status_t sl_gpio_clear_pin(const sl_gpio_t *gpio)
{
  if (gpio == NULL) {
    EFM_ASSERT(false);
    return SL_STATUS_NULL_POINTER;
  }
  EFM_ASSERT(SL_HAL_GPIO_PORT_PIN_IS_VALID(gpio->port, gpio->pin));

  sl_hal_gpio_clear_pin(gpio);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Toggles the DOUT of selected pin on selected port.
 ******************************************************************************/
sl_status_t sl_gpio_toggle_pin(const sl_gpio_t *gpio)
{
  if (gpio == NULL) {
    EFM_ASSERT(false);
    return SL_STATUS_NULL_POINTER;
  }
  EFM_ASSERT(SL_HAL_GPIO_PORT_PIN_IS_VALID(gpio->port, gpio->pin));

  sl_hal_gpio_toggle_pin(gpio);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Gets the output state of selected pin on selected port.
 ******************************************************************************/
sl_status_t sl_gpio_get_pin_output(const sl_gpio_t *gpio,
                                   bool *pin_value)
{
  CORE_DECLARE_IRQ_STATE;

  if (gpio == NULL || pin_value == NULL) {
    EFM_ASSERT(false);
    return SL_STATUS_NULL_POINTER;
  }
  EFM_ASSERT(SL_HAL_GPIO_PORT_PIN_IS_VALID(gpio->port, gpio->pin));

  CORE_ENTER_ATOMIC();

  *pin_value = sl_hal_gpio_get_pin_output(gpio);

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Gets the input state of selected pin on selected port.
 ******************************************************************************/
sl_status_t sl_gpio_get_pin_input(const sl_gpio_t *gpio,
                                  bool *pin_value)
{
  CORE_DECLARE_IRQ_STATE;

  if (gpio == NULL || pin_value == NULL) {
    EFM_ASSERT(false);
    return SL_STATUS_NULL_POINTER;
  }
  EFM_ASSERT(SL_HAL_GPIO_PORT_PIN_IS_VALID(gpio->port, gpio->pin));

  CORE_ENTER_ATOMIC();

  *pin_value = sl_hal_gpio_get_pin_input(gpio);

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Sets the selected pin(s) on selected port.
 ******************************************************************************/
sl_status_t sl_gpio_set_port(sl_gpio_port_t port,
                             uint32_t pins)
{
  EFM_ASSERT(SL_HAL_GPIO_PORT_IS_VALID(port));

  sl_hal_gpio_set_port(port, pins);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Clears the selected pin on selected port.
 ******************************************************************************/
sl_status_t sl_gpio_clear_port(sl_gpio_port_t port,
                               uint32_t pins)
{
  EFM_ASSERT(SL_HAL_GPIO_PORT_IS_VALID(port));

  sl_hal_gpio_clear_port(port, pins);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Gets the output state of pins of selected port.
 ******************************************************************************/
sl_status_t sl_gpio_get_port_output(sl_gpio_port_t port,
                                    uint32_t *port_value)
{
  CORE_DECLARE_IRQ_STATE;

  EFM_ASSERT(SL_HAL_GPIO_PORT_IS_VALID(port));
  if (port_value == NULL) {
    EFM_ASSERT(false);
    return SL_STATUS_NULL_POINTER;
  }

  CORE_ENTER_ATOMIC();

  *port_value = sl_hal_gpio_get_port_output(port);

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Gets the input state of pins of selected port.
 ******************************************************************************/
sl_status_t sl_gpio_get_port_input(sl_gpio_port_t port,
                                   uint32_t *port_value)
{
  CORE_DECLARE_IRQ_STATE;

  EFM_ASSERT(SL_HAL_GPIO_PORT_IS_VALID(port));
  if (port_value == NULL) {
    EFM_ASSERT(false);
    return SL_STATUS_NULL_POINTER;
  }

  CORE_ENTER_ATOMIC();

  *port_value = sl_hal_gpio_get_port_input(port);

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Configuring the GPIO external pin interrupt.
 *  This API can be used to configure interrupt and to register the callback.
 ******************************************************************************/
sl_status_t sl_gpio_configure_external_interrupt(const sl_gpio_t *gpio,
                                                 int32_t *int_no,
                                                 sl_gpio_interrupt_flag_t flags,
                                                 sl_gpio_irq_callback_t gpio_callback,
                                                 void *context)
{
  uint32_t enabled_interrupts;
  CORE_DECLARE_IRQ_STATE;

  if (gpio == NULL || int_no == NULL) {
    EFM_ASSERT(false);
    return SL_STATUS_NULL_POINTER;
  }
  EFM_ASSERT(SL_HAL_GPIO_PORT_PIN_IS_VALID(gpio->port, gpio->pin) || (gpio->port == SL_GPIO_PORT_INTERRUPT));
  EFM_ASSERT(SL_GPIO_FLAG_IS_VALID(flags));

  CORE_ENTER_ATOMIC();

  if (gpio->port != SL_GPIO_PORT_INTERRUPT) {
    *int_no = sl_hal_gpio_configure_external_interrupt(gpio, *int_no, flags);
  }

  if (*int_no == SL_GPIO_INTERRUPT_UNAVAILABLE && gpio->port == SL_GPIO_PORT_INTERRUPT) {
    enabled_interrupts = sl_hal_gpio_get_enabled_interrupts();
    *int_no = sl_hal_gpio_get_external_interrupt_number(gpio->pin, enabled_interrupts);
  }

  if (*int_no != SL_GPIO_INTERRUPT_UNAVAILABLE) {
    // Callback registration.
    gpio_interrupts.callback_ext[*int_no].callback = (void *)gpio_callback;
    gpio_interrupts.callback_ext[*int_no].context = context;

    if (gpio->port != SL_GPIO_PORT_INTERRUPT) {
      sl_hal_gpio_enable_interrupts(1 << *int_no);
    }
  } else {
    CORE_EXIT_ATOMIC();
    return SL_STATUS_NOT_FOUND;
  }

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Deconfigures the GPIO external pin interrupt.
 *  This API can be used to deconfigure the interrupt and to unregister the callback.
 ******************************************************************************/
sl_status_t sl_gpio_deconfigure_external_interrupt(int32_t int_no)
{
  CORE_DECLARE_IRQ_STATE;

  if ((int_no < 0) || (int_no > SL_HAL_GPIO_INTERRUPT_MAX) || (int_no == SL_GPIO_INTERRUPT_UNAVAILABLE)) {
    EFM_ASSERT(false);
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();

  // Clear pending interrupt.
  sl_hal_gpio_clear_interrupts(1 << int_no);
  sl_hal_gpio_disable_interrupts(1 << int_no);

  // Callback deregistration.
  gpio_interrupts.callback_ext[int_no].callback = NULL;
  gpio_interrupts.callback_ext[int_no].context = NULL;

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Enables one or more GPIO interrupts.
 ******************************************************************************/
sl_status_t sl_gpio_enable_interrupts(uint32_t flags)
{
  sl_hal_gpio_enable_interrupts(flags);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Disables one or more GPIO interrupts.
 ******************************************************************************/
sl_status_t sl_gpio_disable_interrupts(uint32_t flags)
{
  sl_hal_gpio_disable_interrupts(flags);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Configures the EM4WU pin as external level interrupts for waking up from EM mode.
 *  Registering/unregistering the callbacks and Configuring the EM4 interrupts to enable/disable
 ******************************************************************************/
sl_status_t sl_gpio_configure_wakeup_em4_interrupt(const sl_gpio_t *gpio,
                                                   int32_t *em4_int_no,
                                                   bool polarity,
                                                   sl_gpio_irq_callback_t gpio_callback,
                                                   void *context)
{
  CORE_DECLARE_IRQ_STATE;

  if (gpio == NULL || em4_int_no == NULL) {
    EFM_ASSERT(false);
    return SL_STATUS_NULL_POINTER;
  }
  EFM_ASSERT(SL_HAL_GPIO_PORT_PIN_IS_VALID(gpio->port, gpio->pin) || (gpio->port == SL_GPIO_PORT_INTERRUPT));

  CORE_ENTER_ATOMIC();

  if (gpio->port != SL_GPIO_PORT_INTERRUPT) {
    *em4_int_no = sl_hal_gpio_configure_wakeup_em4_external_interrupt(gpio, *em4_int_no, polarity);
  }

  if (*em4_int_no != SL_GPIO_INTERRUPT_UNAVAILABLE) {
    // Callback registration.
    gpio_interrupts.callback_em4[*em4_int_no].callback = (void *)gpio_callback;
    gpio_interrupts.callback_em4[*em4_int_no].context = context;

    if (gpio->port != SL_GPIO_PORT_INTERRUPT) {
      sl_hal_gpio_enable_interrupts(1 << (*em4_int_no + SL_HAL_GPIO_EM4WUEN_SHIFT));
    }
  } else {
    CORE_EXIT_ATOMIC();
    return SL_STATUS_NOT_FOUND;
  }

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Deconfigures the EM4 GPIO pin interrupt.
 *  Unregisters a callback, disable/clear interrupt and clear em4 wakeup source
 ******************************************************************************/
sl_status_t sl_gpio_deconfigure_wakeup_em4_interrupt(int32_t em4_int_no)
{
  CORE_DECLARE_IRQ_STATE;

  if (em4_int_no == SL_GPIO_INTERRUPT_UNAVAILABLE || em4_int_no < 0 || em4_int_no > SL_HAL_GPIO_INTERRUPT_MAX) {
    EFM_ASSERT(false);
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();

  // Clear any pending interrupt.
  sl_hal_gpio_clear_interrupts(1 << (em4_int_no + SL_HAL_GPIO_EM4WUEN_SHIFT));
  sl_hal_gpio_disable_pin_em4_wakeup(1 << (em4_int_no + SL_HAL_GPIO_EM4WUEN_SHIFT));
  sl_hal_gpio_disable_interrupts(1 << (em4_int_no + SL_HAL_GPIO_EM4WUEN_SHIFT));

  /* Callback deregistration */
  gpio_interrupts.callback_em4[em4_int_no].callback = NULL;
  gpio_interrupts.callback_em4[em4_int_no].context = NULL;

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Sets GPIO EM4 Wake up interrupt to Enable and EM4 Wake up interrupt polarity
 ******************************************************************************/
sl_status_t sl_gpio_enable_pin_em4_wakeup(uint32_t em4_int_mask,
                                          uint32_t em4_polarity_mask)
{
  uint32_t int_mask = 0;
  uint32_t polarity_mask = 0;

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  // Enable EM4WU function and set polarity.
  int_mask |= (em4_int_mask << _GPIO_EM4WUEN_EM4WUEN_SHIFT);
  polarity_mask |= (em4_polarity_mask << _GPIO_EM4WUEN_EM4WUEN_SHIFT);
  sl_hal_gpio_enable_pin_em4_wakeup(int_mask, polarity_mask);

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Clears GPIO EM4 Wake up enable
 ******************************************************************************/
sl_status_t sl_gpio_disable_pin_em4_wakeup(uint32_t em4_int_mask)
{
  uint32_t int_mask = 0;

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  // Disable EM4WU function.
  int_mask |= (em4_int_mask << _GPIO_EM4WUEN_EM4WUEN_SHIFT);
  sl_hal_gpio_disable_pin_em4_wakeup(int_mask);

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Enable GPIO pin retention of output enable, output value, pull direction, pull enable in EM4
 ******************************************************************************/
sl_status_t sl_gpio_set_pin_em4_retention(bool enable)
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  sl_hal_gpio_set_pin_em4_retention(enable);

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Sets slewrate for selected port.
 ******************************************************************************/
sl_status_t sl_gpio_set_slew_rate(const sl_gpio_t *gpio,
                                  uint8_t slewrate)
{
  sl_status_t status = SL_STATUS_OK;
  CORE_DECLARE_IRQ_STATE;

  if (gpio == NULL) {
    EFM_ASSERT(false);
    return SL_STATUS_NULL_POINTER;
  }

  CORE_ENTER_ATOMIC();

  status = sl_hal_gpio_set_slew_rate(gpio, slewrate);

  CORE_EXIT_ATOMIC();
  return status;
}

/***************************************************************************//**
 * Gets slewrate for selected port.
 ******************************************************************************/
sl_status_t sl_gpio_get_slew_rate(const sl_gpio_t *gpio,
                                  uint8_t *slewrate)
{
  sl_status_t status = SL_STATUS_OK;
  CORE_DECLARE_IRQ_STATE;

  if (gpio == NULL || slewrate == NULL) {
    EFM_ASSERT(false);
    status = SL_STATUS_NULL_POINTER;
  }

  CORE_ENTER_ATOMIC();

  status = sl_hal_gpio_get_slew_rate(gpio, slewrate);

  CORE_EXIT_ATOMIC();

  return status;
}

/***************************************************************************//**
 *  Locks the GPIO Configuration
 ******************************************************************************/
sl_status_t sl_gpio_lock(void)
{
  sl_hal_gpio_lock();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Unlocks the GPIO Configuration
 ******************************************************************************/
sl_status_t sl_gpio_unlock(void)
{
  sl_hal_gpio_unlock();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Gets the GPIO State
 ******************************************************************************/
sl_status_t sl_gpio_is_locked(bool *state)
{
  uint32_t status;
  CORE_DECLARE_IRQ_STATE;

  if (state == NULL) {
    EFM_ASSERT(false);
    return SL_STATUS_NULL_POINTER;
  }

  CORE_ENTER_ATOMIC();

  status =  sl_hal_gpio_get_lock_status();
  if (status) {
    // true - GPIO configuration registers are locked.
    *state = true;
  } else {
    // false - GPIO configuration registers are unlocked.
    *state = false;
  }

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Function calls users callback for registered pin interrupts.
 *
 * @details This function is called when GPIO interrupts are handled by the IRQHandlers.
 *          Function gets even or odd interrupt flags and calls user callback
 *          registered for that pin. Function iterates on flags starting from MSB.
 *
 * @param iflags Interrupt flags which shall be handled by the dispatcher.
 ******************************************************************************/
static void sl_gpio_dispatch_interrupt(uint32_t iflags)
{
  uint32_t irq_idx;
  sl_gpio_callback_desc_t *callback;
  sl_gpio_irq_callback_t func;

  // Check for flags set in IF register.
  while (iflags != 0) {
    irq_idx = SL_CTZ(iflags);
    iflags &= ~(1UL << irq_idx);

    if (irq_idx <= SL_HAL_GPIO_INTERRUPT_MAX) {
      callback = &gpio_interrupts.callback_ext[irq_idx];
    } else {
      callback = &gpio_interrupts.callback_em4[irq_idx - SL_HAL_GPIO_EM4WUEN_SHIFT];
      irq_idx = irq_idx - SL_HAL_GPIO_EM4WUEN_SHIFT;
    }
    // Call user callback.
    if (callback->callback) {
      func = (sl_gpio_irq_callback_t)(callback->callback);
      func((uint8_t)irq_idx, callback->context);
    }
  }
}

/***************************************************************************//**
 *   GPIO EVEN interrupt handler. Interrupt handler clears all IF even flags and
 *   call the dispatcher passing the flags which triggered the interrupt.
 ******************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  uint32_t even_flags;

  // Gets all enabled and pending even interrupts.
  even_flags = sl_hal_gpio_get_enabled_pending_interrupts() & SL_HAL_GPIO_INT_IF_EVEN_MASK;
  // Clears only even interrupts.
  sl_hal_gpio_clear_interrupts(even_flags);

  sl_gpio_dispatch_interrupt(even_flags);
}

/***************************************************************************//**
 * @brief
 *   GPIO ODD interrupt handler. Interrupt handler clears all IF odd flags and
 *   call the dispatcher passing the flags which triggered the interrupt.
 ******************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
  uint32_t odd_flags;

  // Gets all enabled and pending odd interrupts.
  odd_flags = sl_hal_gpio_get_enabled_pending_interrupts() & SL_HAL_GPIO_INT_IF_ODD_MASK;
  // Clears only odd interrupts.
  sl_hal_gpio_clear_interrupts(odd_flags);

  sl_gpio_dispatch_interrupt(odd_flags);
}
