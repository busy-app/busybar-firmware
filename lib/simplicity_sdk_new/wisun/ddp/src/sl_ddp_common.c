/***************************************************************************//**
 * @file sl_ddp_common.c
 * @brief Common framework for DDP
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <stdio.h>
#include "sl_ddp_common.h"

// Auto-generated DDP command table
extern const sl_ddp_command_entry_t sl_ddp_default_command_table[];

/**************************************************************************//**
 * Handle a DDP command.
 *****************************************************************************/
sl_ddp_status_t sl_ddp_handle_command(uint32_t id,
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
  return SL_DDP_STATUS_CMD_UNKNOWN_ERR;
}
