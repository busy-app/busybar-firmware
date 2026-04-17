/***************************************************************************//**
 * @file sl_wisun_br_agent_cli.c
 * @brief Wi-SUN Border Router Agent CLI source file
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stdio.h>
#include "sl_memory_manager.h"
#include "sl_wisun_br_agent_cli.h"
#include "sl_wisun_br_agent_service.h"

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
//                          Static Function Definitions
// -----------------------------------------------------------------------------

void app_set_br_agent_bridge_remote_addr(const sl_cli_command_arg_t *arguments)
{
  const char *bridge_address = NULL;

  bridge_address = sl_cli_get_argument_string(arguments, 0);
  if (bridge_address == NULL) {
    printf("[Failed: BR Bridge Agent address is invalid]\n");
    return;
  }
  if (sl_wisun_br_agent_service_set_bridge_agent_addr(bridge_address) != SL_STATUS_OK) {
    printf("[Failed: unable to set the BR Bridge Agent address: %s]\n", bridge_address);
    return;
  }

  printf("[BR Bridge Agent address is set to: %s]\n", bridge_address);
}

void app_get_br_agent_bridge_addr(const sl_cli_command_arg_t *arguments)
{
  const char *remote_address = NULL;

  (void) arguments;

  remote_address = sl_wisun_br_agent_service_get_bridge_agent_addr();
  if (remote_address == NULL) {
    printf("[Failed: unable to get the BR Bridge Agent address]\n");
    return;
  }
  printf("[%s]\n", remote_address);
  sl_free((void *)remote_address);
}
