/***************************************************************************//**
 * @file app_custom_cli_tick.c
 * @brief Application CLI handler tick function
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <string.h>
#include <cmsis_os2.h>
#include "sl_assert.h"
#include "sl_cli.h"
#include "sl_cli_input.h"
#include "sli_cli_input.h"
#include "sli_cli_io.h"

/* Mostly a copy-paste of sli_cli_tick from SiSDK's sl_cli.c
 * with an extra osDelay(1) at the beginning to work around
 * a wake-up issue on RAIL timer expirations.
 */
bool sli_cli_tick(sl_cli_handle_t handle)
{
  int c;
  bool newline = false;
  bool no_valid_input = false;

  if (handle->tick_in_progress) {
    return false;
  }

  handle->tick_in_progress = true;

  if (handle->req_prompt) {
    handle->req_prompt = false;
    sli_cli_io_printf("%s", handle->prompt_string);
  }
#if defined(SL_CLI_ACTIVE_FLAG_EN)
  handle->active = false;
#endif

  do {
    c = sli_cli_io_getchar();

    if ((c != EOF) && ((char)c != '\0')) {
      newline = sl_cli_input_char(handle, (char)c);
    } else {
      no_valid_input = true;
    }
  } while ((c != EOF) && (!newline));

  // Works around wake-up issue on RAIL timer expirations
  EFM_ASSERT(osDelay(10) == osOK);

  if (newline) {
    handle->req_prompt = true;
    if (strlen(handle->input_buffer) > 0) {
      sli_cli_input_update_history(handle);
      sl_cli_handle_input(handle, handle->input_buffer);
      sl_cli_input_clear(handle);
    }
#if defined(SL_CLI_ACTIVE_FLAG_EN)
    handle->req_prompt = true;
    handle->active = true;
#else
    if (handle->req_prompt) {
      sli_cli_io_printf("%s", handle->prompt_string);
      handle->req_prompt = false;
    }
#endif
  } else {
    if (handle->input_len >= handle->input_size - 1) {
      sli_cli_io_printf("Input buffer is FULL%s", SL_CLI_EOL_STRING);
    }
  }
  handle->tick_in_progress = false;

  return no_valid_input;
}
