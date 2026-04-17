/***************************************************************************//**
 * @file
 * @brief Silicon Labs Secure Engine Manager Device Data API.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sli_se_manager_device_data.h"
#include "sli_se_manager_internal.h"
#include "sli_se_manager_mailbox.h"
#include <stdint.h>

#if defined(_SILICON_LABS_32B_SERIES_3) && defined(SLI_MAILBOX_COMMAND_SUPPORTED)

// ReadDeviceData command options
#define SLI_SE_COMMAND_OPTION_READ_ELEMENT        0x00000000
#define SLI_SE_COMMAND_OPTION_READ_ELEMENT_CHUNK  0x00000001
#define SLI_SE_COMMAND_OPTION_READ_ELEMENT_WORD   0x00000002
#define SLI_SE_COMMAND_OPTION_GET_ELEMENT_SIZE    0x00000003
#define SLI_SE_COMMAND_OPTION_GET_CC_VALUE        0x00000004

/***************************************************************************//**
 * @brief
 *   Read an entire device data section
 ******************************************************************************/
sl_status_t sli_se_device_data_read(sl_se_command_context_t *cmd_ctx,
                                    sli_se_device_data_t section,
                                    void *output_data,
                                    size_t output_data_size)
{
  if (cmd_ctx == NULL || output_data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;

  uint32_t read_command = SLI_SE_COMMAND_READ_DEVICE_DATA;
  read_command |= (section << 8);
  read_command |= SLI_SE_COMMAND_OPTION_READ_ELEMENT;

  sli_se_command_init(cmd_ctx, read_command);

  sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(output_data, output_data_size);

  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * @brief
 *   Read a chunk of a certain length of a device data section
 ******************************************************************************/
sl_status_t sli_se_device_data_read_chunk(sl_se_command_context_t *cmd_ctx,
                                          sli_se_device_data_t section,
                                          uint32_t offset,
                                          size_t chunk_size,
                                          void *output_data)
{
  if (cmd_ctx == NULL || output_data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (chunk_size % 4 != 0 || offset % 4 != 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;

  uint32_t command_word = SLI_SE_COMMAND_READ_DEVICE_DATA;
  command_word |= (section << 8);
  command_word |= SLI_SE_COMMAND_OPTION_READ_ELEMENT_CHUNK;

  // Generate command
  sli_se_command_init(cmd_ctx, command_word);

  sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(output_data, chunk_size);
  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  // Add parameters [offset] and [size]
  sli_se_mailbox_command_add_parameter(se_cmd, offset);
  sli_se_mailbox_command_add_parameter(se_cmd, chunk_size);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * @brief
 *   Read one word (4 bytes) from an address offset of a device data section
 ******************************************************************************/
sl_status_t sli_se_device_data_read_word(sl_se_command_context_t *cmd_ctx,
                                         sli_se_device_data_t section,
                                         uint32_t offset,
                                         uint32_t *output_data)
{
  if (cmd_ctx == NULL || output_data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (offset % 4 != 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  uint32_t command_word = SLI_SE_COMMAND_READ_DEVICE_DATA;
  command_word |= (section << 8);
  command_word |= (SLI_SE_COMMAND_OPTION_READ_ELEMENT_WORD);

  sli_se_command_init(cmd_ctx, command_word);

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(output_data, 4);

  sli_se_mailbox_command_add_output(se_cmd, &out_data);
  sli_se_mailbox_command_add_parameter(se_cmd, offset);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * @brief
 *   Get the size of a device data section
 ******************************************************************************/
sl_status_t sli_se_device_data_get_size(sl_se_command_context_t *cmd_ctx,
                                        sli_se_device_data_t section,
                                        uint32_t *size)
{
  if (cmd_ctx == NULL || size == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  uint32_t command_word = SLI_SE_COMMAND_READ_DEVICE_DATA;
  command_word |= (section << 8);
  command_word |= (SLI_SE_COMMAND_OPTION_GET_ELEMENT_SIZE);

  sli_se_command_init(cmd_ctx, command_word);

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(size, 4);

  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * @brief
 *   Read the value from an address in the CC section
 ******************************************************************************/
sl_status_t sli_se_device_data_get_cc_value(sl_se_command_context_t *cmd_ctx,
                                            uint32_t cc_address,
                                            uint32_t *cc_value)
{
  if (cmd_ctx == NULL || cc_value == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  uint32_t command_word = SLI_SE_COMMAND_READ_DEVICE_DATA;
  command_word |= (SLI_SE_DEVICE_DATA_CC << 8);
  command_word |= (SLI_SE_COMMAND_OPTION_GET_CC_VALUE);

  sli_se_command_init(cmd_ctx, command_word);

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(cc_value, 4);

  sli_se_mailbox_command_add_output(se_cmd, &out_data);
  sli_se_mailbox_command_add_parameter(se_cmd, cc_address);

  return sli_se_execute_and_wait(cmd_ctx);
}

#endif
