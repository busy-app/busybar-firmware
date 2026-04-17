/***************************************************************************//**
 * @file
 * @brief cli_door_lock_keypad.c
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZW_CLI_COMMON_PRESENT

#include <string.h>
#include "zaf_event_distributor_soc.h"
#include "CC_UserCode.h"
#include "sl_cli.h"
#include "app_log.h"
#include "ev_man.h"
#include "events.h"
#include "CC_DoorLock.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static void send_user_code_for_validation(char* user_code);
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/******************************************************************************
 * CLI - battery_report: Send Battery Report
 *****************************************************************************/
void cli_battery_report(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  app_log_info("Send Battery Report\r\n");
  zaf_event_distributor_enqueue_app_event(EVENT_APP_BATTERY_REPORT);
}

/******************************************************************************
 * CLI - enter_user_code: Enter User Code
 *****************************************************************************/
void cli_enter_user_code(sl_cli_command_arg_t *arguments)
{
  char* user_code = sl_cli_get_argument_string(arguments, 0);
  uint8_t user_code_length = strlen(user_code);

  if ((user_code_length < USERCODE_MIN_LEN) || (user_code_length > USERCODE_MAX_LEN)) {
    app_log_error("User code length must be between %d and %d\r\n", USERCODE_MIN_LEN, USERCODE_MAX_LEN);
    return;
  }
  app_log_info("Enter User Code: %s\r\n", user_code);
  send_user_code_for_validation(user_code);
}

/******************************************************************************
 * CLI - set_new_user_code: Set New User Code
 *****************************************************************************/
void cli_set_new_user_code(sl_cli_command_arg_t *arguments)
{
  char* user_code = sl_cli_get_argument_string(arguments, 0);
  uint8_t user_code_length = strlen(user_code);

  if ((user_code_length < USERCODE_MIN_LEN) || (user_code_length > USERCODE_MAX_LEN)) {
    app_log_error("User code length must be between %d and %d\r\n", USERCODE_MIN_LEN, USERCODE_MAX_LEN);
    return;
  }
  CC_UserCode_set_usercode(user_code);
  app_log_info("Set New User Code: %s\r\n", user_code);
}

/******************************************************************************
 * CLI - set_doorhandle_state: Set Doorhandle State
 *****************************************************************************/
void cli_set_doorhandle_state(sl_cli_command_arg_t *arguments)
{
  char* state = sl_cli_get_argument_string(arguments, 0);
  if (strcmp(state, "activate") == 0) {
    app_log_info("Activate Doorhandle\r\n");
    zaf_event_distributor_enqueue_app_event(EVENT_APP_DOORHANDLE_ACTIVATED);
  } else if (strcmp(state, "deactivate") == 0) {
    app_log_info("Deactivate Doorhandle\r\n");
    zaf_event_distributor_enqueue_app_event(EVENT_APP_DOORHANDLE_DEACTIVATED);
  } else {
    app_log_error("Invalid argument\r\n");
  }
}

/******************************************************************************
 * CLI - get_doorhandle_state: Get Door Handle State
 *****************************************************************************/
void cli_get_doorhandle_state(__attribute__((unused)) sl_cli_command_arg_t *arguments)
{
  app_log_info("Get Door Handle State\r\n");
  char* state = door_lock_hw_handle_is_pressed() ? "pressed" : "released";
  app_log_info("Door Handle is %s\r\n", state);
}

/******************************************************************************
 * CLI - set_doorhandle_state: Set Doorhandle State
 *****************************************************************************/
void cli_get_doorbolt_state(__attribute__((unused)) sl_cli_command_arg_t *arguments)
{
  app_log_info("Get Door Bolt State\r\n");
  char* state = door_lock_hw_bolt_is_unlocked() ? "unlocked" : "locked";
  app_log_info("Door Bolt is %s\r\n", state);
}

/******************************************************************************
 * CLI - get_doorlatch_state: Get Door Latch State
 *****************************************************************************/
void cli_get_doorlatch_state(__attribute__((unused)) sl_cli_command_arg_t *arguments)
{
  app_log_info("Get Door Latch State\r\n");
  char* state = door_lock_hw_latch_is_closed() ? "closed" : "open";
  app_log_info("Door Latch is %s\r\n", state);
}

void cli_log_cc_door_lock_events(
  const uint8_t event, __attribute__((unused)) const void * const data)
{
  switch (event) {
    case CC_DOOR_LOCK_EVENT_HW_OPERATION_DONE:
      app_log_info("The door bolt is now %s\r\n",
                   door_lock_hw_bolt_is_unlocked() ? "unlocked" : "locked");
      break;
    default:
      break;
  }
}

ZAF_EVENT_DISTRIBUTOR_REGISTER_CC_EVENT_HANDLER(
  COMMAND_CLASS_DOOR_LOCK, cli_log_cc_door_lock_events);

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static void send_user_code_for_validation(char* user_code)
{
  static uint8_t user_code_buffer[USERCODE_MAX_LEN];
  static cc_user_code_event_validate_data_t user_code_event_validate_data;
  uint8_t user_code_length = strlen(user_code);

  memset(user_code_buffer, 0, sizeof(user_code_buffer));
  memcpy(user_code_buffer, user_code, user_code_length);

  user_code_event_validate_data.id = 1;
  user_code_event_validate_data.data = user_code_buffer;
  user_code_event_validate_data.length = user_code_length;

  zaf_event_distributor_enqueue_cc_event(COMMAND_CLASS_USER_CODE, CC_USER_CODE_EVENT_VALIDATE, (const void*) &user_code_event_validate_data);
}

#endif // SL_CATALOG_ZW_CLI_COMMON_PRESENT
