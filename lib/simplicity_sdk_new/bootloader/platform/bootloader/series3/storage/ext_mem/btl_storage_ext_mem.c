/***************************************************************************//**
 * @file
 * @brief External flash storage component for Silicon Labs Bootloader.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "config/btl_config.h"
#include "api/btl_interface.h"
#include "storage/btl_storage.h"
#include "security/btl_security_types.h"
#include "debug/btl_debug.h"

#ifndef BTL_CONFIG_FILE
#include "btl_storage_slot_cfg.h"
#endif

#include "core/btl_util.h"
MISRAC_DISABLE
#include "em_device.h"
MISRAC_ENABLE

#include <string.h>

// -----------------------------------------------------------------------------
// Globals

const BootloaderStorageLayout_t storageLayout = {
  EXTERNAL_FLASH,
  BTL_STORAGE_NUM_SLOTS,
  BTL_STORAGE_SLOTS
};

// -----------------------------------------------------------------------------
// Statics

// -----------------------------------------------------------------------------
// Functions

// --------------------------------
// Internal Functions

static bool verifyAddressRange(uint32_t address,
                               uint32_t length,
                               uint32_t data_start,
                               size_t data_size)
{
  if ((length > data_size) || (address < data_start) || (address > data_start + data_size)) {
    return false;
  }

  if ((address + length) <= data_start + data_size) {
    return true;
  } else {
    return false;
  }
}

static bool verifyErased(uint32_t address,
                         uint32_t length)
{
  for (uint32_t i = 0; i < length; i += 4) {
    if (*(uint32_t *)(address + i) != 0xFFFFFFFF) {
      return false;
    }
  }
  return true;
}

// --------------------------------
// API Functions

int32_t storage_init(void)
{
  return BOOTLOADER_OK;
}

bool storage_is_busy(void)
{
  return false;
}

int32_t storage_read_raw(uint32_t address, uint8_t *data, size_t length)
{
  sl_status_t status;
  uint32_t *data_start = NULL;
  uint32_t alias_address;
  size_t data_size;
  sl_se_command_context_t cmd_ctx = { 0u };
  sl_se_init_command_context(&cmd_ctx);
  status = sl_se_data_region_get_location(&cmd_ctx, (void**)&data_start, &data_size);

  if (status != BOOTLOADER_OK) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }
  alias_address = bootloader_GetAliasedAddr(address);
  // Ensure address is is within flash
  if (!verifyAddressRange(alias_address, length, (uint32_t)data_start, data_size)) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }

  memcpy(data, (void *)alias_address, length);

  return BOOTLOADER_OK;
}

int32_t storage_write_raw(uint32_t address, uint8_t *data, size_t numBytes)
{
  sl_status_t status;
  uint32_t *data_start = NULL;
  uint32_t alias_address;
  size_t data_size;
  sl_se_command_context_t cmd_ctx = { 0u };
  sl_se_init_command_context(&cmd_ctx);
  status = sl_se_data_region_get_location(&cmd_ctx, (void**)&data_start, &data_size);

  if (status != BOOTLOADER_OK) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }
  alias_address = bootloader_GetAliasedAddr(address);
  // Ensure address is is within chip
  if (!verifyAddressRange(alias_address, numBytes, (uint32_t) data_start, data_size)) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }
  // Ensure space is empty
  if (!verifyErased(alias_address, numBytes)) {
    return BOOTLOADER_ERROR_STORAGE_NEEDS_ERASE;
  }
  status = sl_se_data_region_write(&cmd_ctx, (void*)alias_address, data, numBytes);

  if (status != BOOTLOADER_OK) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }

  return BOOTLOADER_OK;
}

int32_t storage_erase_raw(uint32_t address, size_t totalLength)
{
  // Ensure erase covers an integer number of pages
  if (totalLength % FLASH_PAGE_SIZE) {
    return BOOTLOADER_ERROR_STORAGE_NEEDS_ALIGN;
  }
  // Ensure erase is page aligned
  if (address % FLASH_PAGE_SIZE) {
    return BOOTLOADER_ERROR_STORAGE_NEEDS_ALIGN;
  }

  sl_status_t status;
  uint32_t *data_start = NULL;
  uint32_t alias_address;
  size_t data_size;
  sl_se_command_context_t cmd_ctx = { 0u };
  sl_se_init_command_context(&cmd_ctx);
  status = sl_se_data_region_get_location(&cmd_ctx, (void**)&data_start, &data_size);

  alias_address = bootloader_GetAliasedAddr(address);
  // Ensure address is is within flash
  if (!verifyAddressRange(alias_address, totalLength, (uint32_t)data_start, data_size)) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }

  size_t num_sectors = totalLength / FLASH_PAGE_SIZE;

  status = sl_se_data_region_erase(&cmd_ctx, (void*)alias_address, num_sectors);
  if (status != SL_STATUS_OK) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }
  return BOOTLOADER_OK;
}

int32_t storage_shutdown(void)
{
  return BOOTLOADER_OK;
}
