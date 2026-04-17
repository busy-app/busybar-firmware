/***************************************************************************//**
 * @file
 * @brief app_cc_event_handlers.c
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

#include <stdint.h>

#include "CC_DoorLock.h"
#include "CC_UserCode.h"
#include "CC_Supervision.h"
#include "zaf_event_distributor_soc.h"
#include "events.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Functions Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/**
 * @brief Handles user code application events.
 *
 * This function processes events related to user code applications.
 *
 * @param event The event to handle.
 * @param data Pointer to the event data.
 */
static void user_code_app_event_handler(const uint8_t event, __attribute__((unused)) const void *data)
{
  switch (event) {
    case CC_USER_CODE_EVENT_VALIDATE_VALID:
      zaf_event_distributor_enqueue_cc_event(COMMAND_CLASS_DOOR_LOCK, CC_DOOR_LOCK_CODE_EVENT_TOGGLE, NULL);
      break;
  }
}

/**
 * @brief Handles events related to the door lock application.
 *
 * This function processes various events that are specific to the door lock application.
 * It is responsible for handling events such as door handle activation, deactivation,
 * and other door lock related events.
 *
 * @param event The event to be handled. This parameter is of type `EVENT_APP_DOOR_LOCK`.
 *
 * @return void
 *
 * @note This function is specific to the door lock application and is not intended to be
 *       used for other applications.
 */
static void door_lock_app_event_handler(const uint8_t event, const void *data)
{
  switch (event) {
    case CC_DOOR_LOCK_EVENT_OPERATION_SET_DONE:
    {
      zaf_event_distributor_enqueue_cc_event(COMMAND_CLASS_SUPERVISION, CC_SUPERVISION_EVENT_SUCCESS, data);
      break;
    }
  }
}

ZAF_EVENT_DISTRIBUTOR_REGISTER_CC_EVENT_HANDLER(COMMAND_CLASS_USER_CODE, user_code_app_event_handler);
ZAF_EVENT_DISTRIBUTOR_REGISTER_CC_EVENT_HANDLER(COMMAND_CLASS_DOOR_LOCK, door_lock_app_event_handler);
