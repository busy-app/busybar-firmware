/***************************************************************************//**
 * @file
 * @brief cli_switch_on_off.c
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

#include "zaf_event_distributor_soc.h"
#include "sl_cli.h"
#include "app_log.h"
#include "ev_man.h"
#include "events.h"
#include "CC_BinarySwitch.h"

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
 * CLI - toggle_led: Toggle the LED1
 *****************************************************************************/
void cli_toggle_led(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  app_log_info("Toggle the LED1\r\n");
  zaf_event_distributor_enqueue_app_event(EVENT_APP_TOGGLE_LED);
}

/******************************************************************************
 * CLI - get_led_state: Get the state of the LED1
 *****************************************************************************/
void cli_get_led_state(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  app_log_info("Get the state of the LED1\r\n");
  cc_binary_switch_t * p_switches = cc_binary_switch_get_config();
  char* state = cc_binary_switch_get_current_value(&p_switches[0]) > 0 ? "on" : "off";
  app_log_info("LED1 state: %s\r\n", state);
}

/******************************************************************************
 * CLI - send_nif: Send Node Information Frame
 *****************************************************************************/
void cli_send_nif(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  app_log_info("Send Node Information Frame\r\n");
  zaf_event_distributor_enqueue_app_event(EVENT_APP_SEND_NIF);
}

#endif // SL_CATALOG_ZW_CLI_COMMON_PRESENT
