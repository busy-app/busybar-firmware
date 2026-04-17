/***************************************************************************//**
 * @file
 * @brief cli_wall_controller.c
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
#include "sl_cli.h"
#include "app_log.h"
#include "ev_man.h"
#include "events.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static inline bool isKeyPressed(char* key_attribute_value);

static inline bool isKeyReleased(char* key_attribute_value);

static inline bool isKeyHold(char* key_attribute_value);
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
 * CLI - cli_send_central_scene_key1: Send Central Scene Command for the 1st key
 *****************************************************************************/
void cli_send_central_scene_key(sl_cli_command_arg_t *arguments)
{
  uint8_t key = sl_cli_get_argument_uint8(arguments, 0);
  char*   key_action = sl_cli_get_argument_string(arguments, 1);

  app_log_info("Send Central Scene Command: Key %d %s\r\n", key, key_action);
  enum EVENT_APP_WALL_CONTROLLER event = EVENT_EMPTY;

  if ((key > 3) || (key < 1)) {
    app_log_error("Invalid key number\r\n");
  }
  if (!isKeyPressed(key_action) && !isKeyReleased(key_action) && !isKeyHold(key_action)) {
    app_log_error("Invalid key action\r\n");
  }

  if (key == 1) {
    if (isKeyPressed(key_action)) {
      event = EVENT_APP_KEY01_SHORT_PRESS;
    } else if (isKeyReleased(key_action)) {
      event = EVENT_APP_KEY01_RELEASE;
    } else if (isKeyHold(key_action)) {
      event = EVENT_APP_KEY01_HOLD;
    }
  } else if (key == 2) {
    if (isKeyPressed(key_action)) {
      event = EVENT_APP_KEY02_SHORT_PRESS;
    } else if (isKeyReleased(key_action)) {
      event = EVENT_APP_KEY02_RELEASE;
    } else if (isKeyHold(key_action)) {
      event = EVENT_APP_KEY02_HOLD;
    }
  } else if (key == 3) {
    if (isKeyPressed(key_action)) {
      event = EVENT_APP_KEY03_SHORT_PRESS;
    } else if (isKeyReleased(key_action)) {
      event = EVENT_APP_KEY03_RELEASE;
    } else if (isKeyHold(key_action)) {
      event = EVENT_APP_KEY03_HOLD;
    }
  }

  zaf_event_distributor_enqueue_app_event(event);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static inline bool isKeyPressed(char* key_attribute_value)
{
  return (strcmp(key_attribute_value, "press") == 0);
}

static inline bool isKeyReleased(char* key_attribute_value)
{
  return (strcmp(key_attribute_value, "release") == 0);
}

static inline bool isKeyHold(char* key_attribute_value)
{
  return (strcmp(key_attribute_value, "hold") == 0);
}

#endif // SL_CATALOG_ZW_CLI_COMMON_PRESENT
