/***************************************************************************//**
 * @file sl_connect_rail_dmp_cli.c
 * @brief Source file for the command line interface in the Silicon Labs Connect
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

#include "sl_connect_rail_dmp.h"
#include "sl_cli.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
extern volatile bool tx_requested;
extern volatile bool start_rx;
extern volatile bool stop_rx;
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/**
 * @brief Sends a packet by setting the transmission request flag and posting an OS flag.
 *
 * This function sets the `tx_requested` flag to true, indicating that a transmission
 * has been requested. It then calls the `post_os_flag()` function to signal the OS
 * that a transmission is pending.
 */
void cli_send_packet(void)
{
  tx_requested = true;
  post_os_flag();
}

/**
 * @brief Handles the CLI command to receive a packet.
 *
 * This function processes the CLI command to either start or stop
 * receiving packets based on the provided argument.
 *
 * @param arguments Pointer to the CLI command arguments.
 *                  - arguments[0]: uint8_t value indicating whether to start or stop receiving packets.
 *                    - 0: Stop receiving packets.
 *                    - 1: Start receiving packets.
 */
void cli_receive_packet(sl_cli_command_arg_t *arguments)
{
  uint8_t rxForward = sl_cli_get_argument_uint8(arguments, 0);

  if (rxForward == 0) {
    stop_rx = true;
    post_os_flag();
  } else {
    start_rx = true;
    post_os_flag();
  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
