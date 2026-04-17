/***************************************************************************//**
 * @file
 * @brief app_cc_door_lock_callbacks.c
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
#include "CC_DoorLock.h"
#include "CC_UserCode.h"
#include "cc_user_code_config.h"
#include "zaf_event_distributor_soc.h"
#include "AppTimer.h"
#include "sl_simple_led.h"
#include "sl_simple_led_instances.h"
#include <stdbool.h>
#include "zpal_log.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZW_CC_USER_CREDENTIAL_PRESENT
#include "CC_UserCredential.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

typedef struct {
  SSwTimer timer;
  bool is_instance_inited;
} doorlock_operation_timer;

/// Timeout simulates hardware action. Can be removed when the actual hardware is used.
#define CC_DOOR_LOCK_DEFAULT_OPERATION_SET_TIMEOUT 2000
#define HARDWARE_DELAY_SECONDS CC_DOOR_LOCK_DEFAULT_OPERATION_SET_TIMEOUT / 1000

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

static void door_lock_operation_set_callback(SSwTimer *pTimer);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static doorlock_operation_timer operation_timer = { 0 };
static door_lock_hw_data_t door_lock_hw = {
  .bolt_unlocked = true,
  .handle_pressed = false,
  .latch_closed = false
};
static door_lock_mode_t door_lock_mode = DOOR_MODE_UNSECURE;

// -----------------------------------------------------------------------------
//                          Public Functions Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/**
 * @brief Sets the state of the door handle.
 *
 * This function updates the state of the door handle and sets the corresponding LED status.
 *
 * @param[in] pressed The state of the door handle. True if the handle is pressed, false otherwise.
 */
void cc_door_lock_handle_set(bool pressed)
{
  door_lock_hw.handle_pressed = pressed;
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "Handle %s\r\n", pressed ? "pressed" : "released");
}

/**
 * @brief Sets the latch status.
 *
 * This function updates the latch status of the door lock hardware.
 *
 * @param[in] opened The latch status. True if the latch is opened, false if closed.
 */
void cc_door_lock_latch_set(bool opened)
{
  door_lock_hw.latch_closed = !opened;
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "Latch %s\r\n", opened ? "disengaged" : "engaged");
}

/**
 * @brief Sets the bolt status of the door lock hardware.
 *
 * This function updates the bolt status of the door lock hardware and prints the current status.
 *
 * @param[in] locked The desired bolt status.
 *                   - `true` to lock the bolt.
 *                   - `false` to unlock the bolt.
 *
 * @return void
 */
void cc_door_lock_bolt_set(bool locked)
{
  door_lock_hw.bolt_unlocked = !locked;
  if (locked) {
    sl_simple_led_turn_on(sl_led_led1.context);
  } else {
    sl_simple_led_turn_off(sl_led_led1.context);
  }
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "Bolt %s\r\n", locked ? "locked":"unlocked");
}

/**
 * @brief Checks if the door bolt is unlocked.
 *
 * This function returns the current state of the door bolt.
 *
 * @return true if the door bolt is unlocked, false if it is locked.
 */
bool door_lock_hw_bolt_is_unlocked(void)
{
  return door_lock_hw.bolt_unlocked;
}

/**
 * @brief Checks if the door latch is closed.
 *
 * This function returns the current state of the door latch.
 *
 * @return true if the door latch is closed, false if it is open.
 */
bool door_lock_hw_latch_is_closed(void)
{
  return door_lock_hw.latch_closed;
}

/**
 * @brief Checks if the door handle is pressed.
 *
 * This function returns the current state of the door handle.
 *
 * @return True if the door handle is pressed, false otherwise.
 */
bool door_lock_hw_handle_is_pressed(void)
{
  return door_lock_hw.handle_pressed;
}

/**
 * @brief Initiates a change on the hardware that changes the door lock mode.
 *
 * This function is called after an operation has been triggered locally or a valid Operation Set command was received.
 * It switches the door lock to the specified mode and starts a timer for the operation.
 *
 * @param[in] mode The door lock mode to be set. Possible values are:
 * - `DOOR_MODE_SECURED`: Secured mode
 * - `DOOR_MODE_UNSECURE`: Unsecured mode
 * - Other modes as defined in the `door_lock_mode_t` enum
 *
 * @return The estimated duration of the operation in seconds.
 */
uint8_t cc_door_lock_mode_hw_change(door_lock_mode_t mode)
{
  if ( operation_timer.is_instance_inited == false) {
    AppTimerRegister(&operation_timer.timer, false, door_lock_operation_set_callback);
    operation_timer.is_instance_inited = true;
  }

  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "%s(): Switch to %s mode %#02x\r\n",
                 __func__,
                 (DOOR_MODE_SECURED == mode) ? "Secured":"Unsecured",
                 mode);
  // Timed change happened - simulate timed hardware operation
  // This will also stop any active session
  door_lock_mode = mode;
  TimerStart(&operation_timer.timer,
             CC_DOOR_LOCK_DEFAULT_OPERATION_SET_TIMEOUT);

  // Return estimated duration
  return HARDWARE_DELAY_SECONDS;
}

/**
 * @brief Callback function for door lock operation set.
 *
 * This function is called when the timer for the door lock operation set expires.
 * It changes the door lock mode based on the value stored in the `door_lock_mode` variable.
 *
 * @param[in] pTimer Pointer to the timer structure that triggered the callback.
 *
 * @return void
 */
void door_lock_operation_set_callback(__attribute__((unused)) SSwTimer *pTimer)
{
  cc_door_lock_bolt_set(door_lock_mode == DOOR_MODE_SECURED);

  zaf_event_distributor_enqueue_cc_event(
    COMMAND_CLASS_DOOR_LOCK,
    CC_DOOR_LOCK_EVENT_HW_OPERATION_DONE,
    NULL);
}
