/***************************************************************************//**
 * @file
 * @brief main() function for host examples
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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "sl_main_init.h"
#include "sl_main_process_action.h"
#include "sl_common.h"
#include "app_signal.h"

SL_WEAK void app_cli_init(int argc, char *argv[])
{
  // Default implementation is empty, can be overridden in the user application
  (void)argc;
  (void)argv;
}

SL_WEAK void app_deinit(void)
{
  // Default implementation is empty, can be overridden in the user application
}

// Main loop execution status
static volatile bool run = true;

// Custom signal handler
static void signal_handler(int sig)
{
  (void)sig;
  if (run) {
    run = false;
  } else {
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[])
{
  // Set up custom signal handler for user interrupt and termination request
  app_signal(SIGINT, signal_handler);
  app_signal(SIGTERM, signal_handler);
  // Disable buffering for stdout to ensure immediate output
  setvbuf(stdout, NULL, _IONBF, 0);

  // Initialize components
  sl_main_init();

  // Parse command line arguments and initialize the application
  app_cli_init(argc, argv);
  app_init();

  while (run) {
    // Step function for the components
    sl_main_process_action();

    // Step function for the application
    app_process_action();
  }

  // Deinitialize the application
  app_deinit();

  return EXIT_SUCCESS;
}
