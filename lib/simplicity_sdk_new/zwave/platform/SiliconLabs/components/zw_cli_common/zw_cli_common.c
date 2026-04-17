/***************************************************************************//**
 * @file
 * @brief zw_cli_common.c
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
#include <stdint.h>
#include <string.h>
#include "sl_cli.h"
#include "app_log.h"
#include "zaf_event_distributor_soc.h"
#include "MfgTokens.h"
#include "events.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
// DSK starts from the 13th byte in the QR code
#define SL_CLI_COMMON_DSK_START_INDEX   12
#define SL_CLI_COMMON_DSK_LEN_CHAR      40
#define SL_CLI_DSK_CHUNK_LEN_CHAR       5
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static const char *sli_get_region_name_as_string(zpal_radio_region_t region);

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
 * CLI - this function is used to log cli relevant events
 *****************************************************************************/
void cli_log_system_events(EVENT_SYSTEM event)
{
  switch (event) {
    case EVENT_SYSTEM_LEARNMODE_START:
      app_log_info("Start learn mode\r\n");
      break;
    case EVENT_SYSTEM_LEARNMODE_FINISHED:
      app_log_info("Finished learn mode\r\n");
      break;
    case EVENT_SYSTEM_LEARNMODE_STOP:
      app_log_info("Stop learn mode\r\n");
      break;
    default:
      break;
  }
}

/******************************************************************************
 * CLI - toggle_learn_mode: Include / exclude the device into / from a z-wave network
 *****************************************************************************/
void cli_set_learn_mode(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  zaf_event_distributor_enqueue_app_event(EVENT_SYSTEM_LEARNMODE_TOGGLE);
}

/******************************************************************************
 * CLI - reset: Reset the device
 *****************************************************************************/
void cli_factory_reset(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  app_log_info("Factory reset the device\r\n");
  zaf_event_distributor_enqueue_app_event(EVENT_SYSTEM_RESET);
}

/******************************************************************************
 * CLI - read_dsk: Read the DSK, stored in the manufacturing token
 *****************************************************************************/
void cli_get_dsk(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  char qr_code[TOKEN_MFG_ZW_QR_CODE_SIZE];
  char qr_code_print_buffer[TOKEN_MFG_ZW_QR_CODE_SIZE + 7 /*we put - after every 5th char*/];

  memset(qr_code, 0, sizeof(qr_code));
  memset(qr_code_print_buffer, 0, sizeof(qr_code_print_buffer));

  ZW_GetMfgTokenData(&qr_code, TOKEN_MFG_ZW_QR_CODE_ID, TOKEN_MFG_ZW_QR_CODE_SIZE);

  for (uint8_t i = SL_CLI_COMMON_DSK_START_INDEX; i < SL_CLI_COMMON_DSK_START_INDEX + SL_CLI_COMMON_DSK_LEN_CHAR; i += 5) {
    strncat(qr_code_print_buffer, &qr_code[i], SL_CLI_DSK_CHUNK_LEN_CHAR);

    if (i < (SL_CLI_COMMON_DSK_START_INDEX + SL_CLI_COMMON_DSK_LEN_CHAR) - SL_CLI_DSK_CHUNK_LEN_CHAR) {
      strcat(qr_code_print_buffer, "-");
    }
  }
  app_log_info("%s\r\n", qr_code_print_buffer);
}

/******************************************************************************
 * CLI - get_region: Get the used region
 *****************************************************************************/
void cli_get_region(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  zpal_radio_region_t region = zpal_radio_get_region();
  app_log_info("%s\n", sli_get_region_name_as_string(region));
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static const char* sli_get_region_name_as_string(zpal_radio_region_t region)
{
  switch (region) {
    case REGION_EU:                return "EU";
    case REGION_US:                return "US";
    case REGION_ANZ:               return "ANZ";
    case REGION_HK:                return "HK";
    case REGION_IN:                return "IN";
    case REGION_IL:                return "IL";
    case REGION_RU:                return "RU";
    case REGION_CN:                return "CN";
    case REGION_US_LR:             return "US_LR";
    case REGION_EU_LR:             return "EU_LR";
    case REGION_JP:                return "JP";
    case REGION_KR:                return "KR";
    case REGION_UNDEFINED:         return "UNDEFINED";
    case REGION_DEFAULT:           return "EU";
    default:                       return "Unknown";
  }
}
