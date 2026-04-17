/***************************************************************************//**
 * @file
 * @brief Common framework for DDP
 *******************************************************************************
 * # License
 * <b>Copyright 2026 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <stdio.h>
#include "sl_ddp_common.h"

// Auto-generated DDP command table
extern const sl_ddp_command_entry_t sl_ddp_default_command_table[];

// -----------------------------------------------------------------------------
// Public functions

/******************************************************************************
 * Handle a DDP command.
 *****************************************************************************/
int sl_ddp_handle_command(uint16_t id,
                          const uint8_t *input,
                          uint16_t input_len,
                          uint8_t *output,
                          uint16_t output_size,
                          uint16_t *output_len)
{
  const sl_ddp_command_entry_t *iter;

  // No output by default
  *output_len = 0;

  iter = sl_ddp_default_command_table;
  while (iter && iter->handler) {
    if (iter->id == id) {
      return iter->handler(input, input_len, output, output_size, output_len);
    }

    iter++;
  }

  // No matching command ID found
  return SL_DDP_ERROR_COMMAND;
}
