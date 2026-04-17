/***************************************************************************//**
 * @file
 * @brief Silicon Labs Secure Engine Manager API.
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

#include "sl_se_manager_extmem.h"
#include "sl_status.h"
#include <stdint.h>

#if defined(_SILICON_LABS_32B_SERIES_3) && defined(SLI_MAILBOX_COMMAND_SUPPORTED)

#include "sl_se_manager_util.h"
#include "sli_se_manager_internal.h"
#include "sli_se_manager_mailbox.h"
#include <string.h>

// TODO: Use offsets from local local flash base (FLASH_BASE for host) instead
// of SE flash addresses (related to SE memory map)
// https://jira.silabs.com/browse/PSEC-5391
// Base address of host flash as seen by SE.
#define QSPI_FLASH_HOST_BASE            (0x1000000)

// Code region configuration metadata bit fields.
#define MTP_QSPI_REGION_METADATA_SIZE_SHIFT    0
#define MTP_QSPI_REGION_METADATA_SIZE_MASK     0xfffUL
#define MTP_QSPI_REGION_METADATA_ENC_SHIFT     12
#define MTP_QSPI_REGION_METADATA_ENC           (0x1UL << 12)
#define MTP_QSPI_REGION_METADATA_AEAD_SHIFT    13
#define MTP_QSPI_REGION_METADATA_AEAD          (0x1UL << 13)
#define MTP_QSPI_REGION_METADATA_SWAPPED_SHIFT 14
#define MTP_QSPI_REGION_METADATA_SWAPPED       (0x1UL << 14)
#define MTP_QSPI_REGION_METADATA_LOCKED_SHIFT  15
#define MTP_QSPI_REGION_METADATA_LOCKED        (0x1UL << 15)

// -----------------------------------------------------------------------------
// Local functions

/***************************************************************************//**
 *   Execute command with flash write flag set
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
static sl_status_t extmem_execute_command(sl_se_command_context_t *cmd_ctx)
{
  // Log flash write in command context
  cmd_ctx->flash_wr = true;

  // Execute command
  sl_status_t sl_status = sli_se_execute_and_wait(cmd_ctx);

  // Clear flash write in command context before returning
  cmd_ctx->flash_wr = false;
  return sl_status;
}

// -----------------------------------------------------------------------------
// Global functions

/***************************************************************************//**
 *   Get code region configuration
 ******************************************************************************/
sl_status_t sl_se_code_region_get_config(sl_se_command_context_t *cmd_ctx,
                                         sl_se_code_region_config_t *region_array,
                                         unsigned int start_region_idx,
                                         unsigned int region_array_size)
{
  sli_se_mailbox_command_t *se_cmd;
  sl_status_t sl_status;
  uint16_t region_metadata[SL_SE_MAX_CODE_REGIONS];

  if (cmd_ctx == NULL || region_array == NULL || region_array_size == 0
      || start_region_idx >= SL_SE_MAX_CODE_REGIONS
      || region_array_size > SL_SE_MAX_CODE_REGIONS
      || start_region_idx + region_array_size > SL_SE_MAX_CODE_REGIONS ) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  se_cmd = &cmd_ctx->command;

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_GET_CODE_REGION_CONFIG);

  volatile sli_se_datatransfer_t metadata_out =
    SLI_SE_DATATRANSFER_DEFAULT(region_metadata, sizeof(region_metadata));

  sli_se_mailbox_command_add_output(se_cmd, &metadata_out);

  // Execute and wait
  sl_status = sli_se_execute_and_wait(cmd_ctx);

  if (sl_status == SL_STATUS_OK) {
    for (unsigned int i = start_region_idx; i < start_region_idx + region_array_size; i++) {
      region_array[i - start_region_idx].region_idx = i;
      if ((region_metadata[i] & MTP_QSPI_REGION_METADATA_ENC)
          && (region_metadata[i] & MTP_QSPI_REGION_METADATA_AEAD)) {
        return SL_STATUS_INVALID_STATE;
      }

      region_array[i - start_region_idx].security_level =
        region_metadata[i] & MTP_QSPI_REGION_METADATA_AEAD ? SL_SE_CODE_REGION_SECURITY_LEVEL_ENC_AUTH
        : region_metadata[i] & MTP_QSPI_REGION_METADATA_ENC ? SL_SE_CODE_REGION_SECURITY_LEVEL_ENC_ONLY
        : SL_SE_CODE_REGION_SECURITY_LEVEL_PLAINTEXT;

      region_array[i - start_region_idx].region_size =
        ((region_metadata[i] & MTP_QSPI_REGION_METADATA_SIZE_MASK)
         >> MTP_QSPI_REGION_METADATA_SIZE_SHIFT) * 32 * 1024;

      region_array[i - start_region_idx].bank_swapping_enabled =
        region_metadata[i] & MTP_QSPI_REGION_METADATA_SWAPPED ? true : false;
      region_array[i - start_region_idx].locked =
        region_metadata[i] & MTP_QSPI_REGION_METADATA_LOCKED ? true : false;
    }
  }

  return sl_status;
}

/***************************************************************************//**
 *   Apply/write code region configuration
 ******************************************************************************/
sl_status_t sl_se_code_region_apply_config(sl_se_command_context_t *cmd_ctx,
                                           sl_se_code_region_config_t *region_array,
                                           unsigned int start_region_idx,
                                           unsigned int region_array_size)
{
  sli_se_mailbox_command_t *se_cmd;
  uint16_t region_metadata[SL_SE_MAX_CODE_REGIONS];

  if (cmd_ctx == NULL || region_array == NULL || region_array_size == 0
      || start_region_idx >= SL_SE_MAX_CODE_REGIONS
      || region_array_size > SL_SE_MAX_CODE_REGIONS
      || (start_region_idx + region_array_size > SL_SE_MAX_CODE_REGIONS)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Encode region array into metadata
  for (unsigned int i = 0; i < region_array_size; i++) {
    if (region_array[i].region_idx != start_region_idx + i) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    switch (region_array[i].security_level) {
      case SL_SE_CODE_REGION_SECURITY_LEVEL_ENC_AUTH:
        region_metadata[i] = MTP_QSPI_REGION_METADATA_AEAD;
        break;
      case SL_SE_CODE_REGION_SECURITY_LEVEL_ENC_ONLY:
        region_metadata[i] = MTP_QSPI_REGION_METADATA_ENC;
        break;
      case SL_SE_CODE_REGION_SECURITY_LEVEL_PLAINTEXT:
        region_metadata[i] = 0;
        break;
      default:
        return SL_STATUS_INVALID_PARAMETER;
    }

    // Validate region size, must be a multiple of 32KiBytes
    if (region_array[i].region_size % (32 * 1024)) {
      return SL_STATUS_INVALID_PARAMETER;
    }

    // Set size
    region_metadata[i] |=
      ( (region_array[i].region_size / (32 * 1024) ) << MTP_QSPI_REGION_METADATA_SIZE_SHIFT)
      & MTP_QSPI_REGION_METADATA_SIZE_MASK;

    // Set bank swapped bit
    region_metadata[i] |=
      region_array[i].bank_swapping_enabled ? MTP_QSPI_REGION_METADATA_SWAPPED : 0;

    // Set locked bit
    region_metadata[i] |=
      region_array[i].locked ? MTP_QSPI_REGION_METADATA_LOCKED : 0;
  }

  se_cmd = &cmd_ctx->command;

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_APPLY_CODE_REGION_CONFIG);

  volatile sli_se_datatransfer_t metadata_in =
    SLI_SE_DATATRANSFER_DEFAULT(region_metadata, region_array_size * sizeof(uint16_t));

  sli_se_mailbox_command_add_input(se_cmd, &metadata_in);

  sli_se_mailbox_command_add_parameter(se_cmd, start_region_idx);
  sli_se_mailbox_command_add_parameter(se_cmd, region_array_size * sizeof(uint16_t));

  // Execute and wait
  return extmem_execute_command(cmd_ctx);
}

/***************************************************************************//**
 * Erase a host code region.
 ******************************************************************************/
sl_status_t sl_se_code_region_erase(sl_se_command_context_t *cmd_ctx,
                                    unsigned int region_idx)
{
  sli_se_mailbox_command_t *se_cmd;

  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  se_cmd = &cmd_ctx->command;

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_ERASE_CODE_REGION);

  sli_se_mailbox_command_add_parameter(se_cmd, region_idx);

  // Execute and wait
  return extmem_execute_command(cmd_ctx);
}

/***************************************************************************//**
 * Partially erase a host code region.
 ******************************************************************************/
sl_status_t sl_se_code_region_partial_erase(sl_se_command_context_t *cmd_ctx,
                                            unsigned int region_idx,
                                            uint32_t block_offset,
                                            uint32_t num_blocks)
{
  sli_se_mailbox_command_t *se_cmd;

  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (num_blocks == 0U) {
    return SL_STATUS_OK;
  }

  se_cmd = &cmd_ctx->command;

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_PARTIAL_ERASE_CODE_REGION);

  sli_se_mailbox_command_add_parameter(se_cmd, region_idx);
  sli_se_mailbox_command_add_parameter(se_cmd, block_offset);
  sli_se_mailbox_command_add_parameter(se_cmd, num_blocks);

  // Execute and wait
  return extmem_execute_command(cmd_ctx);
}

/***************************************************************************//**
 * Write data to a host code region.
 ******************************************************************************/
sl_status_t sl_se_code_region_write(sl_se_command_context_t *cmd_ctx,
                                    unsigned int region_idx,
                                    uint32_t offset,
                                    const void *data,
                                    uint32_t num_bytes,
                                    sl_se_crypto_operation_t *decryption_info,
                                    sl_se_crypto_operation_t *integrity_check)
{
  sli_se_mailbox_command_t *se_cmd;
  (void) decryption_info;
  (void) integrity_check;

  if (cmd_ctx == NULL
      || ((num_bytes != 0) && (data == NULL))
      || ((num_bytes & 0x1fU) && (data != NULL) ) ) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (num_bytes == 0U) {
    return SL_STATUS_OK;
  }

  se_cmd = &cmd_ctx->command;

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_WRITE_CODE_REGION);

  volatile sli_se_datatransfer_t in_data = SLI_SE_DATATRANSFER_DEFAULT(data, num_bytes);

  sli_se_mailbox_command_add_input(se_cmd, &in_data);

  sli_se_mailbox_command_add_parameter(se_cmd, region_idx);
  sli_se_mailbox_command_add_parameter(se_cmd, offset);
  sli_se_mailbox_command_add_parameter(se_cmd, num_bytes);

  // Execute and wait
  return extmem_execute_command(cmd_ctx);
}

/***************************************************************************//**
 *   Close code region.
 ******************************************************************************/
sl_status_t sl_se_code_region_close(sl_se_command_context_t *cmd_ctx,
                                    unsigned int region_idx,
                                    uint32_t version)
{
  sli_se_mailbox_command_t *se_cmd;

  if (cmd_ctx == NULL || region_idx >= SL_SE_MAX_CODE_REGIONS) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (version != 0) {
    uint32_t se_version = 0;
    sl_status_t sl_status = sl_se_get_se_version(cmd_ctx, &se_version);
    if ((sl_status != SL_STATUS_OK) || (se_version <= 0x000301UL)) {
      // Versions equal or below 0.3.1 does not support setting the code version
      return SL_STATUS_NOT_SUPPORTED;
    }
  }

  se_cmd = &cmd_ctx->command;

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_CLOSE_CODE_REGION);

  sli_se_mailbox_command_add_parameter(se_cmd, region_idx);
  sli_se_mailbox_command_add_parameter(se_cmd, version);

  // Execute and wait
  return extmem_execute_command(cmd_ctx);
}

/***************************************************************************//**
 *   Get version of the current content in a code region.
 ******************************************************************************/
sl_status_t sl_se_code_region_get_version(sl_se_command_context_t *cmd_ctx,
                                          unsigned int region_idx,
                                          uint32_t *version)
{
  sli_se_mailbox_command_t *se_cmd;

  if (cmd_ctx == NULL
      || region_idx >= SL_SE_MAX_CODE_REGIONS
      || version == NULL ) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  se_cmd = &cmd_ctx->command;

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_GET_CODE_REGION_VERSION);

  sli_se_mailbox_command_add_parameter(se_cmd, region_idx);

  volatile sli_se_datatransfer_t version_out = SLI_SE_DATATRANSFER_DEFAULT(version, sizeof(*version));
  sli_se_mailbox_command_add_output(se_cmd, &version_out);

  // Execute and wait
  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * @brief
 *   Get location of the data region.
 ******************************************************************************/
sl_status_t sl_se_data_region_get_location(sl_se_command_context_t *cmd_ctx,
                                           void **address,
                                           size_t *size)
{
  sli_se_mailbox_command_t *se_cmd;
  uint32_t offset;
  sl_status_t sl_status;

  if (cmd_ctx == NULL || address == NULL || size == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  se_cmd = &cmd_ctx->command;

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_GET_DATA_REGION_LOCATION);

  volatile sli_se_datatransfer_t offs_out = SLI_SE_DATATRANSFER_DEFAULT(address, sizeof(address));
  volatile sli_se_datatransfer_t size_out = SLI_SE_DATATRANSFER_DEFAULT(size, sizeof(*size));

  sli_se_mailbox_command_add_output(se_cmd, &offs_out);
  sli_se_mailbox_command_add_output(se_cmd, &size_out);

  // Execute and wait
  sl_status = sli_se_execute_and_wait(cmd_ctx);

  if (sl_status == SL_STATUS_OK) {
    // Get SE version to figure out how to proceed
    uint32_t version = 0;
    sl_status = sl_se_get_se_version(cmd_ctx, &version);
    if (sl_status != SL_STATUS_OK) {
      return sl_status;
    }
    if (version <= 0x000202UL) {
      // Versions equal or below 0.2.2 have a bug in the get_data_region_location
      // command that we need to work around to enable AXiP
      sl_se_code_region_config_t region_configs[SL_SE_MAX_CODE_REGIONS] = { 0 };
      // Get code region config, and compute AXiP overhead
      sl_status = sl_se_code_region_get_config(cmd_ctx,
                                               region_configs,
                                               0,
                                               SL_SE_MAX_CODE_REGIONS);
      if (sl_status != SL_STATUS_OK) {
        return sl_status;
      }
      uint32_t axip_overhead = 0;
      for (uint32_t i = 0; i < SL_SE_MAX_CODE_REGIONS; i++) {
        if (region_configs[i].security_level == SL_SE_CODE_REGION_SECURITY_LEVEL_ENC_AUTH) {
          axip_overhead += region_configs[i].region_size / 8;
        }
      }
      if (axip_overhead > *size) {
        return SL_STATUS_INVALID_STATE;
      }
      *size -= axip_overhead;
    }

    offset = (uint32_t)*address - QSPI_FLASH_HOST_BASE;
    *address = (void*) (FLASH_BASE + offset);
  }

  return sl_status;
}

/***************************************************************************//**
 * Erase sectors in the data region.
 ******************************************************************************/
sl_status_t sl_se_data_region_erase(sl_se_command_context_t *cmd_ctx,
                                    void *start_address,
                                    size_t num_sectors)
{
  sli_se_mailbox_command_t *se_cmd;
  uint32_t offset = (uint32_t)start_address - FLASH_BASE;

  if (cmd_ctx == NULL || num_sectors == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  se_cmd = &cmd_ctx->command;

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_ERASE_DATA_REGION);

  sli_se_mailbox_command_add_parameter(se_cmd, QSPI_FLASH_HOST_BASE + offset);
  sli_se_mailbox_command_add_parameter(se_cmd, num_sectors);

  // Execute and wait
  return extmem_execute_command(cmd_ctx);
}

/***************************************************************************//**
 * Write data to the data region.
 ******************************************************************************/
sl_status_t sl_se_data_region_write(sl_se_command_context_t *cmd_ctx,
                                    void *address,
                                    const void *data,
                                    size_t num_bytes)
{
  sli_se_mailbox_command_t *se_cmd;
  uint32_t offset = (uint32_t)address - FLASH_BASE;

  if (cmd_ctx == NULL
      || ((num_bytes != 0) && (data == NULL))
      || ((num_bytes & 0x3U) && (data != NULL) ) ) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (num_bytes == 0U) {
    return SL_STATUS_OK;
  }

  se_cmd = &cmd_ctx->command;

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_WRITE_DATA_REGION);

  volatile sli_se_datatransfer_t in_data = SLI_SE_DATATRANSFER_DEFAULT(data, num_bytes);

  sli_se_mailbox_command_add_input(se_cmd, &in_data);

  sli_se_mailbox_command_add_parameter(se_cmd, QSPI_FLASH_HOST_BASE + offset);
  sli_se_mailbox_command_add_parameter(se_cmd, num_bytes);

  // Execute and wait
  return extmem_execute_command(cmd_ctx);
}

/***************************************************************************//**
 * @brief
 *   Query the SE Manager to determine if a command that modifies the L1CACHE
 *   enabled/disabled state is in progress.
 ******************************************************************************/
bool sli_se_extmem_command_with_cachedis_in_progress(void)
{
  // Note on the use of 'extern' here:
  // The global variable has to live in sl_se_manager.c since it is protected by
  // the se thread lock implemented there. Using 'extern' prevents the need from
  // exposing an API in sl_se_manager.c to simply access the variable. The info
  // is only relevant in the context of extmem functions, and hence it belongs
  // here in this file.
  extern bool flash_wr_command_in_progress;
  return flash_wr_command_in_progress;
}

/***************************************************************************//**
 * Get flash status.
 ******************************************************************************/
sl_status_t sli_se_flash_get_status(sl_se_command_context_t *cmd_ctx,
                                    sli_se_flash_status_t *status)
{
  sli_se_flash_status_t flash_status;

  if ((cmd_ctx == NULL) || (status == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_GET_FLASH_STATUS)
  volatile sli_se_datatransfer_t flash_status_data = SLI_SE_DATATRANSFER_DEFAULT(&flash_status, sizeof(flash_status));
  sli_se_mailbox_command_add_output(se_cmd, &flash_status_data);
  sl_status_t sl_status = sli_se_execute_and_wait(cmd_ctx);

  if (sl_status == SL_STATUS_OK) {
    *status = flash_status;
  }
  return sl_status;
}

/***************************************************************************//**
 * Check if flash is busy.
 ******************************************************************************/
bool sli_se_flash_is_busy(void)
{
  sl_status_t status;
  sl_se_command_context_t cmd_ctx;
  sli_se_flash_status_t flash_status;

  status = sl_se_init_command_context(&cmd_ctx);
  if (status != SL_STATUS_OK) {
    return false;
  }

  status = sli_se_flash_get_status(&cmd_ctx, &flash_status);
  if (status != SL_STATUS_OK) {
    return false;
  }

  return flash_status.status == FLASH_BUSY ? true : false;
}

/***************************************************************************//**
 * Check if flash is idle.
 ******************************************************************************/
bool sli_se_flash_is_idle(void)
{
  sl_status_t status;
  sl_se_command_context_t cmd_ctx;
  sli_se_flash_status_t flash_status;

  status = sl_se_init_command_context(&cmd_ctx);
  if (status != SL_STATUS_OK) {
    return false;
  }

  status = sli_se_flash_get_status(&cmd_ctx, &flash_status);
  if (status != SL_STATUS_OK) {
    return false;
  }

  return flash_status.status == FLASH_IDLE ? true : false;
}

/***************************************************************************//**
 * Write data to the data region in non-blocking mode.
 ******************************************************************************/
sl_status_t sli_se_data_region_write_non_blocking(sl_se_command_context_t *cmd_ctx,
                                                  void *address,
                                                  const void *data,
                                                  size_t num_bytes)
{
  sli_se_mailbox_command_t *se_cmd;
  uint32_t offset = (uint32_t)address - FLASH_BASE;

  if (cmd_ctx == NULL
      || ((num_bytes != 0) && (data == NULL))
      || ((num_bytes & 0x3U) && (data != NULL) ) ) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (num_bytes == 0U) {
    return SL_STATUS_OK;
  }

  // The data buffer must be in SRAM.
  size_t sram_size = ((DEVINFO->EMBSIZE & _DEVINFO_EMBSIZE_RAM_MASK) >> _DEVINFO_EMBSIZE_RAM_SHIFT) * 1024;
  if (((size_t)data < SRAM_BASE) || ((size_t)data + num_bytes > SRAM_BASE + sram_size)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  se_cmd = &cmd_ctx->command;
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_WRITE_DATA_REGION | SLI_SE_COMMAND_OPTION_NON_BLOCKING)
  sli_se_mailbox_command_add_parameter(se_cmd, QSPI_FLASH_HOST_BASE + offset);
  sli_se_mailbox_command_add_parameter(se_cmd, 0);
  sli_se_mailbox_command_add_parameter(se_cmd, (uint32_t)data);
  sli_se_mailbox_command_add_parameter(se_cmd, num_bytes);

  // Execute and wait
  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Erase host data region sectors in non-blocking mode.
 ******************************************************************************/
sl_status_t sli_se_data_region_erase_non_blocking(sl_se_command_context_t *cmd_ctx,
                                                  void *start_address, size_t num_sectors)
{
  sli_se_mailbox_command_t *se_cmd;
  uint32_t offset = (uint32_t)start_address - FLASH_BASE;

  if (cmd_ctx == NULL || num_sectors == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  se_cmd = &cmd_ctx->command;
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_ERASE_DATA_REGION | SLI_SE_COMMAND_OPTION_NON_BLOCKING)
  sli_se_mailbox_command_add_parameter(se_cmd, QSPI_FLASH_HOST_BASE + offset);
  sli_se_mailbox_command_add_parameter(se_cmd, num_sectors);

  // Execute and wait
  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Pause an ongoing flash operation.
 ******************************************************************************/
sl_status_t sli_se_flash_pause(sl_se_command_context_t *cmd_ctx)
{
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_FLASH_PAUSE)
  return extmem_execute_command(cmd_ctx);
}

/***************************************************************************//**
 * Resume a paused flash operation.
 ******************************************************************************/
sl_status_t sli_se_flash_resume(sl_se_command_context_t *cmd_ctx)
{
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_FLASH_RESUME)
  return extmem_execute_command(cmd_ctx);
}

/***************************************************************************//**
 * Write data to the code region in non-blocking mode.
 ******************************************************************************/
sl_status_t sli_se_code_region_write_non_blocking(sl_se_command_context_t *cmd_ctx,
                                                  unsigned int region_idx,
                                                  uint32_t offset,
                                                  const void *data,
                                                  uint32_t num_bytes,
                                                  sl_se_crypto_operation_t *decryption_info,
                                                  sl_se_crypto_operation_t *integrity_check)
{
  sli_se_mailbox_command_t *se_cmd;
  (void) decryption_info;
  (void) integrity_check;

  if (cmd_ctx == NULL
      || ((num_bytes != 0) && (data == NULL))
      || ((num_bytes & 0x1fU) && (data != NULL) ) ) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (num_bytes == 0U) {
    return SL_STATUS_OK;
  }

  // The data buffer must be in SRAM.
  size_t sram_size = ((DEVINFO->EMBSIZE & _DEVINFO_EMBSIZE_RAM_MASK) >> _DEVINFO_EMBSIZE_RAM_SHIFT) * 1024;
  if (((size_t)data < SRAM_BASE) || ((size_t)data + num_bytes > SRAM_BASE + sram_size)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  se_cmd = &cmd_ctx->command;
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_WRITE_CODE_REGION | SLI_SE_COMMAND_OPTION_NON_BLOCKING)
  sli_se_mailbox_command_add_parameter(se_cmd, region_idx);
  sli_se_mailbox_command_add_parameter(se_cmd, offset);
  sli_se_mailbox_command_add_parameter(se_cmd, 0);
  sli_se_mailbox_command_add_parameter(se_cmd, (uint32_t)data);
  sli_se_mailbox_command_add_parameter(se_cmd, num_bytes);

  // Execute and wait
  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Erase a host code region in non-blocking mode.
 ******************************************************************************/
sl_status_t sli_se_code_region_erase_non_blocking(sl_se_command_context_t *cmd_ctx,
                                                  unsigned int region_idx)
{
  sli_se_mailbox_command_t *se_cmd;

  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  se_cmd = &cmd_ctx->command;
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_ERASE_CODE_REGION | SLI_SE_COMMAND_OPTION_NON_BLOCKING)
  sli_se_mailbox_command_add_parameter(se_cmd, region_idx);

  // Execute and wait
  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Partially erase host code region blocks in non-blocking mode.
 ******************************************************************************/
sl_status_t sli_se_code_region_partial_erase_non_blocking(sl_se_command_context_t *cmd_ctx,
                                                          unsigned int region_idx,
                                                          uint32_t block_offset,
                                                          uint32_t num_blocks)

{
  sli_se_mailbox_command_t *se_cmd;

  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (num_blocks == 0U) {
    return SL_STATUS_OK;
  }

  se_cmd = &cmd_ctx->command;
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_PARTIAL_ERASE_CODE_REGION | SLI_SE_COMMAND_OPTION_NON_BLOCKING)
  sli_se_mailbox_command_add_parameter(se_cmd, region_idx);
  sli_se_mailbox_command_add_parameter(se_cmd, block_offset);
  sli_se_mailbox_command_add_parameter(se_cmd, num_blocks);

  // Execute and wait
  return sli_se_execute_and_wait(cmd_ctx);
}

#if defined(_SILICON_LABS_32B_SERIES_3_CONFIG_301)
#define SLI_SE_COMMAND_CONFIGURE_QSPI_REF_CLOCK_OPTION_FSRCO   0x00000100;
#define SLI_SE_COMMAND_CONFIGURE_QSPI_REF_CLOCK_OPTION_FLPLL   0x00000200;

/***************************************************************************//**
 *   Selects FSRCO as QSPI clock and disables FLPLL
 ******************************************************************************/
sl_status_t sli_se_qspi_configure_clock_fsrco(sl_se_command_context_t *cmd_ctx)
{
  sl_status_t status;
  uint32_t se_version;
  status = sl_se_get_se_version(cmd_ctx, &se_version);
  if (status != SL_STATUS_OK) {
    return status;
  }

  if (se_version < 0x00000202) {
    // Command not supported prior to SEFW v0.2.2
    return SL_STATUS_NOT_SUPPORTED;
  }

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_CONFIGURE_QSPI_REF_CLOCK | SLI_SE_COMMAND_CONFIGURE_QSPI_REF_CLOCK_OPTION_FSRCO);

  // Execute and wait
  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 *   Selects the reference clock for and configures the high speed FLPLL qspi clock.
 ******************************************************************************/
sl_status_t sli_se_qspi_configure_clock_flpll(sl_se_command_context_t *cmd_ctx,
                                              const sli_se_qspi_flpll_config_t *flpll_cfg)
{
  sli_se_mailbox_command_t *se_cmd;

  uint8_t int_div = flpll_cfg->int_div;
  uint16_t frac_div = flpll_cfg->frac_div;
  uint8_t pll_clk_freq_sel = flpll_cfg->pll_clk_freq_sel;
  uint8_t clk_per_presc = flpll_cfg->clk_per_presc;
  uint8_t ref_clock_sel = flpll_cfg->ref_clock;

  sl_status_t status;
  uint32_t se_version;

  if ((int_div < 6 || int_div > 11)
      || (frac_div > 2047)
      || (pll_clk_freq_sel > 5)
      || (clk_per_presc > 3)
      || (ref_clock_sel != SLI_SE_QSPI_REF_CLOCK_HFXO && ref_clock_sel != SLI_SE_QSPI_REF_CLOCK_HFRCODPLL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  se_cmd = &cmd_ctx->command;

  // Check SEFW version
  status = sl_se_get_se_version(cmd_ctx, &se_version);
  if (status != SL_STATUS_OK) {
    return status;
  }
  if (se_version >= 0x00000202) {
    sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_CONFIGURE_QSPI_REF_CLOCK | SLI_SE_COMMAND_CONFIGURE_QSPI_REF_CLOCK_OPTION_FLPLL);

    // Set HFXO as the reference clock
    sli_se_mailbox_command_add_parameter(se_cmd, ref_clock_sel);
    sli_se_mailbox_command_add_parameter(se_cmd, (int_div << 16 | frac_div));
    sli_se_mailbox_command_add_parameter(se_cmd, pll_clk_freq_sel);
    sli_se_mailbox_command_add_parameter(se_cmd, clk_per_presc);
  } else if (se_version >= 0x00000200) {
    // Different operation for SEFW < v0.2.2. Only FRAC_DIV is handled
    sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_CONFIGURE_QSPI_REF_CLOCK);

    // Set reference clock and frac_div
    sli_se_mailbox_command_add_parameter(se_cmd, ref_clock_sel);
    sli_se_mailbox_command_add_parameter(se_cmd, frac_div);
  } else {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Execute and wait
  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 *   Retrieve FLPLL configurations currently set in the QSPI registers
 ******************************************************************************/
sl_status_t sli_se_qspi_get_flpll_config(sl_se_command_context_t *cmd_ctx,
                                         sli_se_qspi_flpll_config_t *flpll_cfg)
{
  sli_se_mailbox_command_t *se_cmd;
  sl_status_t status;
  uint32_t se_version;

  if (cmd_ctx == NULL || flpll_cfg == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Ensure version compatibility
  status = sl_se_get_se_version(cmd_ctx, &se_version);
  if (status != SL_STATUS_OK) {
    return status;
  }

  if (se_version < 0x00000300) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  se_cmd = &cmd_ctx->command;
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_GET_QSPI_FLPLL_CONFIG);
  uint8_t buffer[7] = { 0 };
  volatile sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(buffer, sizeof(buffer));

  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  status = sli_se_execute_and_wait(cmd_ctx);

  flpll_cfg->int_div = buffer[0];
  flpll_cfg->frac_div = (uint16_t)((uint16_t)buffer[2] << 8) + buffer[1];
  flpll_cfg->pll_clk_freq_sel = (sli_se_qspi_pllclkfreqsel_t)buffer[3];
  flpll_cfg->clk_per_presc = (sli_se_qspi_clkperpresc_t)buffer[5];

  return status;
}

/***************************************************************************//**
 *   Configure a QSPI/FLASH register
 ******************************************************************************/
sl_status_t sli_se_qspi_configure_reg(sl_se_command_context_t *cmd_ctx,
                                      sli_se_qspi_reg_t reg,
                                      uint32_t mask,
                                      uint32_t val,
                                      uint32_t *result)
{
  sli_se_mailbox_command_t *se_cmd;
  sl_status_t status;
  uint32_t se_version;
  uint32_t register_ret = 0;
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  se_cmd = &cmd_ctx->command;

  status = sl_se_get_se_version(cmd_ctx, &se_version);
  if (status != SL_STATUS_OK) {
    return status;
  }

  if (se_version < 0x00000300) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_CONFIGURE_QSPI_REGS | (reg << 8));
  sli_se_mailbox_command_add_parameter(se_cmd, val);
  sli_se_mailbox_command_add_parameter(se_cmd, mask);

  volatile sli_se_datatransfer_t reg_out =
    SLI_SE_DATATRANSFER_DEFAULT(&register_ret, sizeof(register_ret));

  sli_se_mailbox_command_add_output(se_cmd, &reg_out);

  status = sli_se_execute_and_wait(cmd_ctx);

  if (status != SL_STATUS_OK) {
    return status;
  }

  if (result != NULL) {
    *result = register_ret;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *   sli_se_qspi_get_reg
 ******************************************************************************/
sl_status_t sli_se_qspi_get_reg(sl_se_command_context_t *cmd_ctx,
                                sli_se_qspi_reg_t reg,
                                uint32_t *register_val)
{
  return sli_se_qspi_configure_reg(cmd_ctx, reg, 0, 0, register_val);
}

#endif // defined(_SILICON_LABS_32B_SERIES_3_CONFIG_301)

/***************************************************************************//**
 * @brief
 *   This will erase the entire host flash. Calling it from flash will break
 *   the application.
 ******************************************************************************/
sl_status_t sli_se_erase_host_region(sl_se_command_context_t *cmd_ctx)
{
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_ERASE_HOST_FLASH);

  return extmem_execute_command(cmd_ctx);
}

/***************************************************************************//**
 * Send a command to an external memory device connected to a QSPI peripheral.
 ******************************************************************************/
sl_status_t sli_se_spi_device_command(sl_se_command_context_t *cmd_ctx,
                                      uint32_t spi_instance,
                                      sli_se_spi_command_t command_type,
                                      uint16_t command_code,
                                      uint8_t  command_size,
                                      uint32_t address,
                                      uint8_t  address_size,
                                      uint8_t  *data,
                                      uint8_t  data_size)
{
  unsigned data_in_size, data_out_size;

  if ((cmd_ctx == NULL)
      || (spi_instance != 1)
      || (command_size == 0)
      || ((command_type == SLI_SE_SPI_COMMAND_READ) && (data_size == 0))
      || ((data_size != 0) && (data == NULL))
      || ((data_size == 0) && (data != NULL))
      || (address_size > 4)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;

  if (command_type == SLI_SE_SPI_COMMAND_WRITE) {
    data_out_size = 0;
    data_in_size = data_size;
    sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_SPI_DEVICE_COMMAND | SLI_SE_COMMAND_OPTION_WRITE);
    volatile sli_se_datatransfer_t in_data = SLI_SE_DATATRANSFER_DEFAULT(data, data_in_size);
    volatile sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(NULL, 0);
    sli_se_mailbox_command_add_input(se_cmd, &in_data);
    sli_se_mailbox_command_add_output(se_cmd, &out_data);
  } else {
    data_in_size = 0;
    data_out_size = data_size;
    sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_SPI_DEVICE_COMMAND | SLI_SE_COMMAND_OPTION_READ);
    volatile sli_se_datatransfer_t in_data = SLI_SE_DATATRANSFER_DEFAULT(NULL, 0);
    volatile sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(data, data_out_size);
    sli_se_mailbox_command_add_input(se_cmd, &in_data);
    sli_se_mailbox_command_add_output(se_cmd, &out_data);
  }

  sli_se_mailbox_command_add_parameter(se_cmd, spi_instance);
  sli_se_mailbox_command_add_parameter(se_cmd, command_code);
  sli_se_mailbox_command_add_parameter(se_cmd, command_size);
  sli_se_mailbox_command_add_parameter(se_cmd, address);
  sli_se_mailbox_command_add_parameter(se_cmd, address_size);
  sli_se_mailbox_command_add_parameter(se_cmd, data_in_size);
  sli_se_mailbox_command_add_parameter(se_cmd, data_out_size);

  // Execute and wait
  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Write to a single QSPI peripheral register.
 ******************************************************************************/
sl_status_t sli_se_write_spi_register(sl_se_command_context_t *cmd_ctx,
                                      uint32_t spi_instance,
                                      uint32_t offset,
                                      uint32_t value)
{
  uint32_t table[2] = { offset, value };
  return sli_se_write_spi_registers(cmd_ctx, spi_instance, table, 1);
}

/***************************************************************************//**
 * Write to a series of QSPI peripheral registers.
 ******************************************************************************/
sl_status_t sli_se_write_spi_registers(sl_se_command_context_t *cmd_ctx,
                                       uint32_t spi_instance,
                                       uint32_t *table,
                                       uint32_t count)
{
  if ((cmd_ctx == NULL) || (spi_instance != 1) || (table == NULL) || (count == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_WRITE_SPI_REGISTERS);

  volatile sli_se_datatransfer_t in_data = SLI_SE_DATATRANSFER_DEFAULT(table, 8 * count);
  sli_se_mailbox_command_add_input(se_cmd, &in_data);

  sli_se_mailbox_command_add_parameter(se_cmd, spi_instance);
  sli_se_mailbox_command_add_parameter(se_cmd, 8 * count);

  // Execute and wait
  return sli_se_execute_and_wait(cmd_ctx);
}

#endif // defined(_SILICON_LABS_32B_SERIES_3) && defined(SLI_MAILBOX_COMMAND_SUPPORTED)
