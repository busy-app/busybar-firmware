/***************************************************************************/ /**
 * @file at_command_data_mode.c
 * @brief AT Command Data Mode
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

#include "console.h"
#include "sl_constants.h"
#include "at_command_data_mode.h"
#include "at_utility.h"
#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>

typedef struct {
  bool enable;
  uint8_t *buffer;
  uint32_t buff_index;
  uint32_t max_data;
  at_data_mode_handler_t handler;
  void *user_data;
} at_command_data_mode_t;

static at_command_data_mode_t data_mode = {
  .enable     = false,
  .buffer     = NULL,
  .buff_index = 0,
  .max_data   = 0,
  .handler    = NULL,
};

bool at_command_data_mode_is_enable()
{
  return data_mode.enable;
}

bool at_command_data_mode_process()
{
  if (!data_mode.enable || !data_mode.buffer)
    return false;

  uint32_t data_length = console_read_data_from_cache((char *)(data_mode.buffer + data_mode.buff_index),
                                                      data_mode.max_data - data_mode.buff_index);

  if (data_length + data_mode.buff_index < data_mode.max_data) {
    data_mode.buff_index += data_length;
  } else {
    if (data_mode.handler != NULL) {
      sl_status_t status = data_mode.handler(data_mode.buffer, data_mode.max_data, data_mode.user_data);
      if (status != SL_STATUS_OK) {
        AT_PRINTF("ERROR %" PRIi32 "", (int32_t)status);
      }
      AT_PRINTF("\r\n> \r\n");
    }

    data_mode.enable     = false;
    data_mode.buff_index = 0;
    data_mode.max_data   = 0;
    data_mode.handler    = NULL;
    SL_CLEANUP_MALLOC(data_mode.buffer);
  }

  return true;
}

void at_command_exit_data_mode()
{
  data_mode.enable     = false;
  data_mode.buff_index = 0;
  data_mode.max_data   = 0;
  data_mode.handler    = NULL;
  SL_CLEANUP_MALLOC(data_mode.buffer);
}

sl_status_t at_command_goto_data_mode(at_data_mode_handler_t handler, uint32_t max_data, void *user_data)
{
  data_mode.buffer = (uint8_t *)calloc(max_data, sizeof(uint8_t));
  if (!data_mode.buffer) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  data_mode.enable     = true;
  data_mode.buff_index = 0;
  data_mode.max_data   = max_data;
  data_mode.handler    = handler;
  data_mode.user_data  = user_data;

  return SL_STATUS_OK;
}