/***************************************************************************//**
 * @file sli_token_manager_hal_ext_flash.c
 * @brief HAL API implementation for handling external flash read/write operation.
 * @version 1.0.0
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

#include <string.h>
#include "sl_se_manager.h"
#include "sl_se_manager_extmem.h"
#include "sli_token_manager_hal_flash.h"
#include "sl_hal_system.h"

/******************************************************************************
 ******************************    MACROS    **********************************
 *****************************************************************************/

#define CHECK_DATA  1           ///< Macro defining if data should be checked

/******************************************************************************
 ***************************   LOCAL VARIABLES   ******************************
 *****************************************************************************/

/******************************************************************************
 ***************************   LOCAL FUNCTIONS   ******************************
 *****************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/***************************************************************************//**
 * Check if the page is erased.
 ******************************************************************************/
static bool sli_hal_external_flash_verify_erased(uint32_t flash_adr, uint32_t length)
{
  for (uint32_t i = 0; i < length; i += 4) {
    if (*(uint32_t *)(flash_adr + i) != 0xFFFFFFFF) {
      return false;
    }
  }
  return true;
}

/***************************************************************************//**
 * Check if the flash_adr range is within data region.
 ******************************************************************************/
static bool sli_hal_external_flash_verify_range(uint32_t flash_adr,
                                                size_t size,
                                                size_t data_start,
                                                size_t data_size)
{
  // Check if static secure region page's start and end addresses are within the data region or not
  if (((size_t)flash_adr < data_start) || (((size_t)flash_adr + size) > (data_start + data_size))) {
    return false;
  }
  return true;
}

/** @endcond */

/***************************************************************************//**
 * Open the flash hal for usage. SE will take care of clocking the external flash.
 * This function initializes the SE lock mutex and checks if LOCKBITS
 * start and end addresses are within the data region or not.
 ******************************************************************************/
static sl_status_t sli_hal_external_flash_open(uint32_t flash_adr, size_t flash_size)
{
  sl_status_t status;
  void *startAdr;
  size_t regSize;
  sl_se_command_context_t cmd_ctx;

  status = sl_se_init();

  if (status != SL_STATUS_OK) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sl_se_init_command_context(&cmd_ctx);
  status = sl_se_data_region_get_location(&cmd_ctx, &startAdr, &regSize);

  if (status == SL_STATUS_OK) {
    // Check if static secure region start and end addresses are within the data region or not
    if (!sli_hal_external_flash_verify_range(flash_adr, flash_size, (size_t)startAdr, regSize)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  } else {
    return SL_STATUS_INVALID_STATE;
  }

  return status;
}

/***************************************************************************//**
 * Close the flash hal. This function de-initializes the SE lock mutex.
 ******************************************************************************/
static void sli_hal_external_flash_close(void)
{
  // De-initialize SE lock mutex
  sl_se_deinit();
}

/***************************************************************************//**
 * This function is used to read data from the static secure token region.
 ******************************************************************************/
static sl_status_t sli_hal_external_flash_read(uint32_t srcAdr,
                                               void *dst,
                                               size_t length)
{
  (void)memcpy(dst, (void *)srcAdr, length);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * This function is used to write data to the static secure page, using SE API.
 ******************************************************************************/
static sl_status_t sli_hal_external_flash_write(uint32_t dstAdr,
                                                void const *src,
                                                size_t length)
{
  sl_status_t status;
  sl_se_command_context_t cmd_ctx;

  sl_se_init_command_context(&cmd_ctx);
  status = sl_se_data_region_write(&cmd_ctx, (void *)dstAdr, src, length);

#if CHECK_DATA
  if (status == SL_STATUS_OK) {
    if (memcmp((void *)dstAdr, src, length) != 0) {
      status = SL_STATUS_FLASH_VERIFY_FAILED;
    }
  }
#endif
  return status;
}

/***************************************************************************//**
 * This function is used to erase static secure page.
 ******************************************************************************/
static sl_status_t sli_hal_external_flash_page_erase(uint32_t address, size_t size)
{
  sl_status_t status = SL_STATUS_OK;
  sl_se_command_context_t cmd_ctx = { 0u };
  uint32_t flash_page_size = sl_hal_system_get_flash_page_size();
  size_t num_sectors = size / flash_page_size;

  // Ensure erase is page aligned
  if ( (address % flash_page_size) && (size % flash_page_size)) {
    return SL_STATUS_FLASH_ERASE_FAILED;
  }

  sl_se_init_command_context(&cmd_ctx);
  status = sl_se_data_region_erase(&cmd_ctx, (void *)address, num_sectors);

#if CHECK_DATA
  if (status == SL_STATUS_OK) {
    if (!sli_hal_external_flash_verify_erased(address, size)) {
      status = SL_STATUS_FLASH_ERASE_FAILED;
    }
  }
#endif

  return status;
}

const sli_token_hal_flash_handle_t sli_token_manager_hal_flash_handle = {
  .open = sli_hal_external_flash_open,
  .close = sli_hal_external_flash_close,
  .read = sli_hal_external_flash_read,
  .write = sli_hal_external_flash_write,
  .erase = sli_hal_external_flash_page_erase,
};
