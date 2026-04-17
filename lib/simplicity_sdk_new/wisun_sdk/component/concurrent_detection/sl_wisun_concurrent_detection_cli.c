/***************************************************************************//**
 * @file sl_wisun_concurrent_detection_cli.c
 * @brief Wi-SUN Concurrent Detection CLI handler
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

#include "sl_status.h"
#include "sl_cli.h"
#include "sl_wisun_api.h"
#include "sl_wisun_cli_util.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define SL_WISUN_CONCURRENT_DETECTION_RESERVED  (0U)
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/* CLI app concurrent detection */
void app_concurrent_detection(const sl_cli_command_arg_t *arguments)
{
  sl_status_t status = SL_STATUS_OK;
  bool enable = false;

  enable = sl_cli_get_argument_uint8(arguments, 0);

  status = sl_wisun_config_concurrent_detection(enable,
                                                SL_WISUN_CONCURRENT_DETECTION_RESERVED);
  switch (status) {
    case SL_STATUS_OK:
      printf("[Concurrent detection succeeded]\n");
      break;
    case SL_STATUS_NOT_SUPPORTED:
      printf("[Concurrent detection feature not supported on this chip]\n");
      break;
    default:
      printf("[Concurrent detection failure %lu]\n", status);
      break;
  }
}
