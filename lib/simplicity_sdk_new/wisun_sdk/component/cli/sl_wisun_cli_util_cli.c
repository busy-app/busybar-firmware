/***************************************************************************//**
 * @file sl_wisun_cli_util_cli.c
 * @brief Wi-SUN CLI utility functions
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_cli.h"
#include "sl_string.h"
#include "sl_wisun_cli_core.h"
#include "sl_wisun_cli_util.h"
#include "sl_cmsis_os2_common.h"
#include "sl_wisun_cli_settings.h"
#include "sl_wisun_cli_util_config.h"
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

/* CLI app set */
void app_set(const sl_cli_command_arg_t *arguments)
{
  const char *domain_and_key = NULL;
  const char *value = NULL;

  uint8_t argument_count = (uint8_t)sl_cli_get_argument_count(arguments);

  if (argument_count == 0 || (!sl_strcasecmp(sl_cli_get_argument_string(arguments, 0), "help") && argument_count == 1)) {
    if (app_settings_help(NULL, 0) != SL_STATUS_OK) {
      printf("[Failed: unable to get the set help]\r\n");
    }
  } else if (argument_count == 0 || (!sl_strcasecmp(sl_cli_get_argument_string(arguments, argument_count - 1), "help"))) {
    domain_and_key = sl_cli_get_argument_string(arguments, 0);
    value = NULL;
    if (app_settings_help((char *)domain_and_key, 0) != SL_STATUS_OK) {
      printf("[Failed: unable to get the get help of : %s]\r\n", domain_and_key);
    }
  } else {
    if (sl_cli_get_argument_count(arguments) != 2) {
      printf("[Failed: incorrect number of arguments]\r\n");
      return;
    }

    domain_and_key = sl_cli_get_argument_string(arguments, 0);
    value = sl_cli_get_argument_string(arguments, 1);

    if (value == NULL || domain_and_key == NULL) {
      printf("[Failed: missing %s]\r\n", value == NULL ? "value" : "key");
      return;
    }

    if (app_settings_set((char *)domain_and_key, value) != SL_STATUS_OK) {
      printf("[Failed: unable to set the key: %s]\r\n", domain_and_key);
    }

#if (WISUN_CLI_AUTO_SAVE)
    app_settings_save();
#endif
  }
}

/* CLI app get */
void app_get(const sl_cli_command_arg_t *arguments)
{
  const char *domain_and_key = NULL;

  uint8_t argument_count = (uint8_t)sl_cli_get_argument_count(arguments);

  if (argument_count == 0 || (!sl_strcasecmp(sl_cli_get_argument_string(arguments, 0), "help") && argument_count == 1)) {
    if (app_settings_help(NULL, 1) != SL_STATUS_OK) {
      printf("[Failed: unable to get the get help]\r\n");
    }
  } else if (argument_count == 0 || (!sl_strcasecmp(sl_cli_get_argument_string(arguments, argument_count - 1), "help"))) {
    domain_and_key = sl_cli_get_argument_string(arguments, 0);
    if (app_settings_help((char *)domain_and_key, 1) != SL_STATUS_OK) {
      printf("[Failed: unable to get the set help of : %s]\r\n", domain_and_key);
    }
  } else {
    if (sl_cli_get_argument_count(arguments) != 1) {
      printf("[Failed: incorrect number of arguments]\r\n");
      return;
    }

    domain_and_key = sl_cli_get_argument_string(arguments, 0);
    if (app_settings_get((char *)domain_and_key) != SL_STATUS_OK) {
      printf("[Failed: unable to get the key: %s]\r\n", domain_and_key);
    }
  }
}

/* CLI Save settings */
void app_save(const sl_cli_command_arg_t *arguments)
{
  sl_status_t ret;
  (void)arguments;

  ret = app_settings_save();
  if (ret == SL_STATUS_OK) {
    printf("[Settings saved]\r\n");
  } else {
    printf("[Failed to save settings: %lu]\r\n", ret);
  }
}

/* CLI Reset settings */
void app_reset(const sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  app_settings_reset();
  printf("[Settings reset]\r\n");
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
