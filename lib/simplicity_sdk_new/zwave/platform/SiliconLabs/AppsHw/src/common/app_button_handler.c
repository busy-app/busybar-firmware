/***************************************************************************//**
 * @file
 * @brief app_button_handler.c
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include "app_button_handler.h"
#include "app_button_press.h"
#include "app_button_press_config.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#include "sl_simple_button_instances.h"
#include "sl_power_manager.h"
#include <events.h>
#include "zpal_log.h"
#include "SwTimer.h"
#include "AppTimer.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define BUTTON_HOLD_POWER_LOCK (SL_POWER_MANAGER_EM1)
#define BUTTON_POWER_LOCK_RELEASE_DELAY_MS (100)

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

#ifdef SL_CATALOG_ZW_SLEEPING_DEVICE_PRESENT
static bool button_power_locked = false;
static SSwTimer button_power_lock_timer = { 0 };
#endif

// -----------------------------------------------------------------------------
//                          Public Functions Declarations
// -----------------------------------------------------------------------------
ZW_WEAK void app_button_press_btn_0_handler(uint8_t duration);
ZW_WEAK void app_button_press_btn_1_handler(uint8_t duration);

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/**
 * @brief Enqueues an application event.
 *
 * This function adds the specified application event to the event queue.
 *
 * @param app_event The event to enqueue.
 * @return Returns true if the event was successfully enqueued, false otherwise.
 */
bool app_event_enqueue(uint8_t app_event)
{
  bool return_value = false;
  if (app_event != EVENT_EMPTY) {
    return_value = zaf_event_distributor_enqueue_app_event(app_event);
  }
  return return_value;
}

/**
 * @brief Enqueues a Command Class event.
 *
 * This function adds the specified Command Class event to the event queue.
 *
 * @param command_class The Command Class to receive the event.
 * @param cc_event The event to enqueue.
 * @param cc_data Pointer to the data associated with the event.
 * @return Returns true if the event was successfully enqueued, false otherwise.
 */
bool app_cc_event_enqueue(uint16_t command_class, uint8_t cc_event, void *cc_data)
{
  bool return_value = false;
  if (command_class != COMMAND_CLASS_NO_OPERATION) {
    return_value = zaf_event_distributor_enqueue_cc_event(command_class, cc_event, cc_data);
  }
  return return_value;
}

#ifdef SL_CATALOG_ZW_SLEEPING_DEVICE_PRESENT
static bool are_all_buttons_released()
{
  for (uint8_t i = 0; i < SL_SIMPLE_BUTTON_COUNT; i++) {
    if (sl_simple_button_get_state(SL_SIMPLE_BUTTON_INSTANCE(i)) != SL_SIMPLE_BUTTON_RELEASED) {
      // At least one button is in a pressed state
      return false;
    }
  }
  return true;
}

// Checks if button power lock can be removed.
// This is to add some delay to the power lock removal after the button release.
static void button_power_lock_timer_cb(__attribute__((unused)) SSwTimer* timer)
{
  if (!button_power_locked) {
    // Power lock is already removed, exit
    return;
  }
  if (are_all_buttons_released()) {
    button_power_locked = false;
    sl_power_manager_remove_em_requirement(BUTTON_HOLD_POWER_LOCK);
  }
}

/**
 * @brief Keeps the device awake until the button is released.
 *
 * This function sets a power lock to keep the device awake for the duration of the
 * button hold.
 */
void app_button_press_stay_awake_until_release(void)
{
  /* If any of the buttons are pressed down, keep the device awake.
   * This check is necessary, because if the button is released before this point
   * (which can happen for very short presses), the button release won't be detected,
   * and we stay awake indefinitely.
   */
  if (!are_all_buttons_released() && !button_power_locked) {
    sl_power_manager_add_em_requirement(BUTTON_HOLD_POWER_LOCK);
    button_power_locked = true;
  }
}
#endif

/**
 * @brief Handles button press errors.
 *
 * This function is called when an error occurs during a button press event.
 * It logs the error status.
 *
 * @param status The status code representing the error.
 */
void app_button_press_error(sl_status_t status)
{
  (void)status;
  ZPAL_LOG_ERROR(ZPAL_LOG_HW, "App button press error happened, status: %d\n", status);
}

/**
 * @brief Callback function for handling button press events.
 *
 * This function is called when a button is pressed. It determines which button
 * was pressed and the duration of the press, then calls the appropriate handler
 * function.
 *
 * @param button The button that was pressed.
 * @param duration The duration of the button press.
 */
void app_button_press_cb(uint8_t button, uint8_t duration)
{
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "Button %u pressed for duration ENUM %u\n", button, duration);
  switch (button) {
    case 0:
      app_button_press_btn_0_handler(duration);
      break;
    case 1:
      app_button_press_btn_1_handler(duration);
      break;
    default:
      break;
  }
  #ifdef SL_CATALOG_ZW_SLEEPING_DEVICE_PRESENT
  AppTimerRegister(&button_power_lock_timer, false, button_power_lock_timer_cb);
  TimerStop(&button_power_lock_timer);
  if (!button_power_locked && duration == APP_BUTTON_PRESS_PRESSED_DOWN) {
    button_power_locked = true;
    sl_power_manager_add_em_requirement(BUTTON_HOLD_POWER_LOCK);
  } else if (button_power_locked && duration != APP_BUTTON_PRESS_PRESSED_DOWN && are_all_buttons_released()) {
    TimerStart(&button_power_lock_timer, BUTTON_POWER_LOCK_RELEASE_DELAY_MS);
  }
  #endif
}

/**
 * @brief Handles button 0 press events.
 *
 * This function is called when button 0 is pressed. It determines the duration
 * of the press and performs the corresponding action. This is a weak implementation
 * and can be overridden by a user-defined function.
 *
 * @param duration The duration of the button press.
 */
ZW_WEAK void app_button_press_btn_0_handler(uint8_t duration)
{
  (void)duration;
  ZPAL_LOG_DEBUG(ZPAL_LOG_HW, "Button 0 pressed, default handler called\n");
}

/**
 * @brief Handles button 1 press events.
 *
 * This function is called when button 1 is pressed. It determines the duration
 * of the press and performs the corresponding action. This is a weak implementation
 * and can be overridden by a user-defined function.
 *
 * @param duration The duration of the button press.
 */
ZW_WEAK void app_button_press_btn_1_handler(uint8_t duration)
{
  (void)duration;
  ZPAL_LOG_DEBUG(ZPAL_LOG_HW, "Button 1 pressed, default handler called\n");
}
