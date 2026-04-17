/***************************************************************************//**
 * @file
 * @brief cli_sensor_pir.c
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

#include "sl_cli.h"
#include "app_log.h"
#include "ev_man.h"
#include "events.h"
#include "zaf_event_distributor_soc.h"
#include <string.h>
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
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/******************************************************************************
 * CLI - vattery_report: Send out the battery report
 *****************************************************************************/
void cli_battery_report(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  app_log_info("Send out the battery report\r\n");
  zaf_event_distributor_enqueue_app_event(EVENT_APP_BATTERY_REPORT);
}

/******************************************************************************
 * CLI - motion_detected: Emulate "motion detected" and wake up the device from EM4
 *****************************************************************************/
void cli_motion_detected(sl_cli_command_arg_t *arguments)
{
  char* state = sl_cli_get_argument_string(arguments, 0);
  if (strcmp(state, "activate") == 0) {
    app_log_info("Activate PIR event\r\n");
    zaf_event_distributor_enqueue_app_event(EVENT_APP_TRANSITION_TO_ACTIVE);
  } else if (strcmp(state, "deactivate") == 0) {
    app_log_info("Deactivate PIR event\r\n");
    zaf_event_distributor_enqueue_app_event(EVENT_APP_TRANSITION_TO_DEACTIVE);
  } else {
    app_log_error("Invalid argument\r\n");
  }
}

#endif // SL_CATALOG_ZW_CLI_COMMON_PRESENT
