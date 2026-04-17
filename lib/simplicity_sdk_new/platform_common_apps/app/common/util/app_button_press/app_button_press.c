/***************************************************************************//**
 * @file
 * @brief Button Press source code
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

#include "sl_common.h"
#include "sl_sleeptimer.h"
#include "app_button_press.h"
#include "app_button_press_internal.h"
#include "app_button_press_config.h"
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"
#include "app_rta.h"
#include "sl_component_catalog.h"

#ifdef SL_CATALOG_CLI_PRESENT
#include "sl_cli.h"
#endif // SL_CATALOG_CLI_PRESENT

#ifdef SL_CATALOG_APP_ASSERT_PRESENT
#include "app_assert.h"
#endif // SL_CATALOG_APP_ASSERT_PRESENT

#ifdef SL_CATALOG_APP_EM4H_RESET_PRESENT
#include "em_rmu.h"
#include "em_gpio.h"
#include "sl_hal_gpio.h"
#endif // SL_CATALOG_APP_EM4H_RESET_PRESENT

// -----------------------------------------------------------------------------
// Definitions

#ifdef SL_CATALOG_CLI_PRESENT
// Index of button ID parameter from CLI
#define BUTTON_ID_PARAM_IDX         0
// Index of button event duration parameter from CLI
#define DURATION_PARAM_IDX          1
#endif // SL_CATALOG_CLI_PRESENT
// Queue element size
#define QUEUE_ELEMENT_SIZE          sizeof(button_event_t)

// Type for button event
typedef struct {
  uint32_t duration;
  uint8_t  index;
} button_event_t;

// Structure type for button status
typedef struct {
  uint32_t timestamp;
  uint8_t  press;
} button_state_t;

// Structure type for all button status
typedef struct {
  button_state_t buttons[SL_SIMPLE_BUTTON_COUNT];
} button_press_state_t;

// -----------------------------------------------------------------------------
// Private variables

// State of the component, default is disabled
static bool disabled = true;
// Button press state
static button_press_state_t state;
// Application Runtime Adaptor context
static app_rta_context_t ctx = APP_RTA_INVALID_CONTEXT;

#ifdef SL_CATALOG_CLI_PRESENT
static uint32_t cli_button_timestamps[SL_SIMPLE_BUTTON_COUNT];
static uint8_t cli_button_states[SL_SIMPLE_BUTTON_COUNT];
#endif // SL_CATALOG_CLI_PRESENT

#ifdef SL_CATALOG_APP_EM4H_RESET_PRESENT
static bool wakeup_buttons[SL_SIMPLE_BUTTON_COUNT] = { false };
#endif // SL_CATALOG_APP_EM4H_RESET_PRESENT

// -----------------------------------------------------------------------------
// Forward declaration of private functions

static void init_error(app_rta_error_t error, sl_status_t result);
static void calculate_press(button_event_t *evt);

// -----------------------------------------------------------------------------
// Public functions

/******************************************************************************
* Init function for button press
******************************************************************************/
void app_button_press_init(void)
{
  sl_status_t sc;

#ifdef SL_CATALOG_CLI_PRESENT
  // Iterate over buttons and clear timestamps and states
  for (uint8_t i = 0; i < SL_SIMPLE_BUTTON_COUNT; i++) {
    cli_button_timestamps[i] = 0;
    cli_button_states[i] = SL_SIMPLE_BUTTON_RELEASED;
  }
#endif // SL_CATALOG_CLI_PRESENT

  // Iterate over buttons and clear timestamps and states
  for (uint8_t i = 0; i < SL_SIMPLE_BUTTON_COUNT; i++) {
    state.buttons[i].press = APP_BUTTON_PRESS_NONE;
    state.buttons[i].timestamp = 0;
  }
  // Optimize queue size
  uint16_t queue_size = APP_BUTTON_PRESS_CONFIG_QUEUE_SIZE;
  if (queue_size < SL_SIMPLE_BUTTON_COUNT) {
    queue_size = SL_SIMPLE_BUTTON_COUNT;
  }
  app_rta_config_t config = {
    .requirement.runtime = true,
    .requirement.guard   = true,
    .requirement.signal  = false,
    .requirement.queue   = true,
    .step                = app_button_press_step,
    .priority            = APP_BUTTON_PRESS_CONFIG_PRIORITY,
    .stack_size          = APP_BUTTON_PRESS_CONFIG_STACK_SIZE,
    .error               = init_error,
    .queue_size          = queue_size,
    .queue_element_size  = QUEUE_ELEMENT_SIZE,
    .wait_for_guard      = APP_BUTTON_PRESS_CONFIG_WAIT_FOR_GUARD
  };
  sc = app_rta_create_context(&config, &ctx);
  if (sc != SL_STATUS_OK) {
    app_button_press_error(sc);
  }
}

/******************************************************************************
* Step function for button press
******************************************************************************/
void app_button_press_step(void)
{
  button_event_t evt;
  sl_status_t sc;
  size_t size;
  sc = app_rta_queue_read_and_acquire(ctx, (uint8_t *)&evt, &size);
  if (sc == SL_STATUS_OK) {
    // Calculate button press
    calculate_press(&evt);
    // Iterate over buttons
    for (uint8_t i = 0; i < SL_SIMPLE_BUTTON_COUNT; i++) {
      // If the button is pressed
      if (state.buttons[i].press != APP_BUTTON_PRESS_NONE) {
        // Call callback
        app_button_press_cb(i, state.buttons[i].press);
        // Clear state
        state.buttons[i].press = APP_BUTTON_PRESS_NONE;
      }
    }
    (void)app_rta_release(ctx);
  } else {
    app_button_press_error(sc);
  }
}

/******************************************************************************
* Enable button press
******************************************************************************/
sl_status_t app_button_press_enable(void)
{
  sl_status_t sc = app_rta_acquire(ctx);
  if (sc == SL_STATUS_OK) {
    // Check if buttons are pressed now
    for (uint8_t i = 0; i < SL_SIMPLE_BUTTON_COUNT; i++) {
      sl_button_state_t button_state = sl_button_get_state(SL_SIMPLE_BUTTON_INSTANCE(i));

      if (button_state == SL_SIMPLE_BUTTON_PRESSED) {
        // Set timestamp
        state.buttons[i].timestamp = sl_sleeptimer_get_tick_count();
      }
#ifdef SL_CATALOG_APP_EM4H_RESET_PRESENT
      else if (wakeup_buttons[i]) {
        // This button caused an EM4 wakeup but it was released in the meantime.
        // Fire a short press event.
        button_event_t evt = {
          .duration = sl_sleeptimer_get_tick_count(),
          .index = i
        };
        sl_status_t sc = app_rta_queue_push(ctx, (uint8_t *)&evt, sizeof(evt));
        if (sc != SL_STATUS_OK) {
          app_button_press_error(sc);
        }
        wakeup_buttons[i] = false;
      }
#endif // SL_CATALOG_APP_EM4H_RESET_PRESENT
    }
    // Clear disabled state
    disabled = false;
    (void)app_rta_release(ctx);
  }
  return sc;
}

/******************************************************************************
* Disable button press
******************************************************************************/
sl_status_t app_button_press_disable(void)
{
  sl_status_t sc = app_rta_acquire(ctx);
  if (sc == SL_STATUS_OK) {
    if (!disabled) {
      disabled = true;
      // Clear timestamps and states

      for (uint8_t i = 0; i < SL_SIMPLE_BUTTON_COUNT; i++) {
        state.buttons[i].press = APP_BUTTON_PRESS_NONE;
        state.buttons[i].timestamp = 0;
      }
    }
    (void)app_rta_release(ctx);
  }
  return sc;
}

#ifdef SL_CATALOG_APP_EM4H_RESET_PRESENT
/******************************************************************************
* Check if the device was woken up from EM4 by a button press
******************************************************************************/
void app_button_press_check_wakeup_cause(void)
{
  // Check if the device was woken up from EM4 by an external interrupt
  if ((RMU_ResetCauseGet() & EMU_RSTCAUSE_EM4)) {
    for (uint8_t i = 0; i < SL_SIMPLE_BUTTON_COUNT; i++) {
      // Determine the interrupt number for the button
      const sl_button_t *button = SL_SIMPLE_BUTTON_INSTANCE(i);
      sl_gpio_t gpio = {
        .port = SL_SIMPLE_BUTTON_GET_PORT(button->context),
        .pin = SL_SIMPLE_BUTTON_GET_PIN(button->context)
      };
      int32_t int_no = sl_hal_gpio_get_em4_interrupt_number(&gpio);

      // Check if the interrupt was caused by the button
      if (int_no != SL_GPIO_INTERRUPT_UNAVAILABLE) {
        wakeup_buttons[i] = !!(GPIO_EM4GetPinWakeupCause()
                               & (1 << (int_no + _GPIO_IF_EM4WU_SHIFT)));
      }
    }
  }
}
#endif // SL_CATALOG_APP_EM4H_RESET_PRESENT

// -----------------------------------------------------------------------------
// Weak implementation of callbacks

SL_WEAK void app_button_press_cb(uint8_t button, uint8_t duration)
{
  (void)button;
  (void)duration;
}

SL_WEAK void app_button_press_error(sl_status_t status)
{
#ifdef SL_CATALOG_APP_ASSERT_PRESENT
  app_assert_status(status);
#endif // SL_CATALOG_APP_ASSERT_PRESENT
}

// -----------------------------------------------------------------------------
// Callback implementations

/******************************************************************************
* This is a callback function that is invoked each time a GPIO interrupt
* in one of the pushbutton inputs occurs.
*
* @param[in] handle Pointer to button instance
*
* @note This function is called from ISR context and therefore it is
*       not possible to call any BGAPI functions directly. The button state
*       of the instance is updated based on the state change. The state is
*       updated only after button release and it depends on how long the
*       button was pressed. The button state is handled by the main loop.
******************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  button_event_t evt;
  sl_status_t sc;
  // If disabled, do nothing
  if (disabled) {
    return;
  }
  for (uint8_t i = 0; i < SL_SIMPLE_BUTTON_COUNT; i++) {
    // If the handle is applicable
    if (SL_SIMPLE_BUTTON_INSTANCE(i) == handle) {
      if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
        state.buttons[i].timestamp = sl_sleeptimer_get_tick_count();
        #if (APP_BUTTON_PRESS_DETECTION == APP_BUTTON_PRESS_DETECT_PRESS_AND_RELEASE)
        state.buttons[i].press = APP_BUTTON_PRESS_PRESSED_DOWN;
        #else
        return;
        #endif // APP_BUTTON_PRESS_DETECTION
      }
      // Push event
      evt.duration = sl_sleeptimer_get_tick_count() - state.buttons[i].timestamp;
#ifdef SL_CATALOG_APP_EM4H_RESET_PRESENT
      if (wakeup_buttons[i]) {
        // This button caused an EM4 wakeup
        evt.duration = sl_sleeptimer_get_tick_count()
                       + sl_sleeptimer_ms_to_tick(APP_BUTTON_PRESS_WAKEUP_DELAY_MS);
        wakeup_buttons[i] = false;
      }
#endif // SL_CATALOG_APP_EM4H_RESET_PRESENT
      evt.index = i;
      if ((evt.duration > sl_sleeptimer_ms_to_tick(MIN_VALID_BUTTON_PRESS_DURATION_MS))
          || (state.buttons[i].press == APP_BUTTON_PRESS_PRESSED_DOWN)) {
        sc = app_rta_queue_push(ctx, (uint8_t *)&evt, sizeof(evt));
        if (sc != SL_STATUS_OK) {
          app_button_press_error(sc);
        }
      }
      return;
    }
  }
}

// -----------------------------------------------------------------------------
// CLI related functions

#ifdef SL_CATALOG_CLI_PRESENT
/******************************************************************************
* Command Line Interface callback implementation
*
* @param[in] arguments command line argument list
******************************************************************************/
void button_press_from_cli(sl_cli_command_arg_t *arguments)
{
  uint8_t button_id;
  uint8_t duration;
  button_id = sl_cli_get_argument_uint8(arguments, BUTTON_ID_PARAM_IDX);
  duration = sl_cli_get_argument_uint8(arguments, DURATION_PARAM_IDX);
  if (duration > APP_BUTTON_PRESS_DURATION_VERYLONG) {
    duration = APP_BUTTON_PRESS_DURATION_VERYLONG;
  }
  if (button_id >= SL_SIMPLE_BUTTON_COUNT) {
    button_id = SL_SIMPLE_BUTTON_COUNT - 1;
  }
  app_button_press_cb(button_id, duration);
}

/******************************************************************************
* Command Line Interface callback implementation
*
* @param[in] arguments command line argument list
******************************************************************************/
void button_hold_from_cli(sl_cli_command_arg_t *arguments)
{
  uint8_t button_id;
  button_id = sl_cli_get_argument_uint8(arguments, BUTTON_ID_PARAM_IDX);
  if (button_id >= SL_SIMPLE_BUTTON_COUNT) {
    button_id = SL_SIMPLE_BUTTON_COUNT - 1;
  }
  if (cli_button_states[button_id] == SL_SIMPLE_BUTTON_RELEASED) {
    cli_button_timestamps[button_id] = sl_sleeptimer_get_tick_count();
    cli_button_states[button_id] = SL_SIMPLE_BUTTON_PRESSED;
    #if (APP_BUTTON_PRESS_DETECTION == APP_BUTTON_PRESS_DETECT_PRESS_AND_RELEASE)
    app_button_press_cb(button_id, APP_BUTTON_PRESS_PRESSED_DOWN);
    #endif // APP_BUTTON_PRESS_DETECTION
  }
}

/******************************************************************************
* Command Line Interface callback implementation
*
* @param[in] arguments command line argument list
******************************************************************************/
void button_release_from_cli(sl_cli_command_arg_t *arguments)
{
  uint8_t button_id;
  button_id = sl_cli_get_argument_uint8(arguments, BUTTON_ID_PARAM_IDX);
  if (button_id >= SL_SIMPLE_BUTTON_COUNT) {
    button_id = SL_SIMPLE_BUTTON_COUNT - 1;
  }
  if (cli_button_states[button_id] == SL_SIMPLE_BUTTON_PRESSED) {
    uint32_t t_diff = sl_sleeptimer_get_tick_count() - cli_button_timestamps[button_id];
    cli_button_states[button_id] = SL_SIMPLE_BUTTON_RELEASED;
    if (t_diff < sl_sleeptimer_ms_to_tick(SHORT_BUTTON_PRESS_DURATION_MS)) {
      app_button_press_cb(button_id, APP_BUTTON_PRESS_DURATION_SHORT);
    } else if (t_diff < sl_sleeptimer_ms_to_tick(MEDIUM_BUTTON_PRESS_DURATION_MS)) {
      app_button_press_cb(button_id, APP_BUTTON_PRESS_DURATION_MEDIUM);
    } else if (t_diff < sl_sleeptimer_ms_to_tick(LONG_BUTTON_PRESS_DURATION_MS)) {
      app_button_press_cb(button_id, APP_BUTTON_PRESS_DURATION_LONG);
    } else {
      app_button_press_cb(button_id, APP_BUTTON_PRESS_DURATION_VERYLONG);
    }
  }
}
#endif // SL_CATALOG_CLI_PRESENT

// -----------------------------------------------------------------------------
// Private functions

static void init_error(app_rta_error_t error, sl_status_t result)
{
  (void)error;
  app_button_press_error(result);
}

static void calculate_press(button_event_t *evt)
{
  uint8_t i = evt->index;
  #if (APP_BUTTON_PRESS_DETECTION == APP_BUTTON_PRESS_DETECT_PRESS_AND_RELEASE)
  if (state.buttons[i].press == APP_BUTTON_PRESS_PRESSED_DOWN) {
    return;
  }
  #endif // APP_BUTTON_PRESS_DETECTION
  // Set state flag according to the difference
  if (evt->duration < sl_sleeptimer_ms_to_tick(SHORT_BUTTON_PRESS_DURATION_MS)) {
    state.buttons[i].press = APP_BUTTON_PRESS_DURATION_SHORT;
  } else if (evt->duration < sl_sleeptimer_ms_to_tick(MEDIUM_BUTTON_PRESS_DURATION_MS)) {
    state.buttons[i].press = APP_BUTTON_PRESS_DURATION_MEDIUM;
  } else if (evt->duration < sl_sleeptimer_ms_to_tick(LONG_BUTTON_PRESS_DURATION_MS)) {
    state.buttons[i].press = APP_BUTTON_PRESS_DURATION_LONG;
  } else {
    state.buttons[i].press = APP_BUTTON_PRESS_DURATION_VERYLONG;
  }
}
