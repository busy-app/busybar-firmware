/***************************************************************************//**
 * @file
 * @brief Internal flash storage component for Silicon Labs Bootloader.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "btl_internal_flash.h"
#include "api/btl_errorcode.h"
#include "btl_interface.h"
#include "core/btl_util.h"
MISRAC_DISABLE
#include "em_device.h"
MISRAC_ENABLE

#include <string.h>
// --------------------------------
// API Functions

int32_t storage_readRaw(uint32_t address, uint8_t *data, size_t length)
{
  // Ensure address is is within flash
  if (!verifyAddressRange(address, length)) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }

  memcpy(data, (void *)address, length);

  return BOOTLOADER_OK;
}

int32_t storage_writeRaw(uint32_t address, uint8_t *data, size_t numBytes)
{
  // Ensure address is is within chip
  if (!verifyAddressRange(address, numBytes)) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }
  // Ensure space is empty
  if (!verifyErased(address, numBytes)) {
    return BOOTLOADER_ERROR_STORAGE_NEEDS_ERASE;
  }

#if defined(BOOTLOADER_MSC_DMA_WRITE) && (BOOTLOADER_MSC_DMA_WRITE == 1)
  if ((uint32_t) data & 3UL) {
    //Data address not aligned. Use normal write.
    if (flash_writeBuffer(address, data, numBytes)) {
      return BOOTLOADER_OK;
    } else {
      return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
    }
  } else if (flash_writeBuffer_dma(address, data, numBytes, BOOTLOADER_MSC_DMA_CHANNEL)) {
    return BOOTLOADER_OK;
  }
#else
  if (flash_writeBuffer(address, data, numBytes)) {
    return BOOTLOADER_OK;
  }
#endif
  else {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }
}

int32_t storage_eraseRaw(uint32_t address, size_t totalLength)
{
  // Ensure erase covers an integer number of pages
  if (totalLength % FLASH_PAGE_SIZE) {
    return BOOTLOADER_ERROR_STORAGE_NEEDS_ALIGN;
  }
  // Ensure erase is page aligned
  if (address % FLASH_PAGE_SIZE) {
    return BOOTLOADER_ERROR_STORAGE_NEEDS_ALIGN;
  }
  // Ensure address is is within flash
  if (!verifyAddressRange(address, totalLength)) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }

  bool retval = false;

  do {
    retval = flash_erasePage(address);
    address += FLASH_PAGE_SIZE;
    totalLength -= FLASH_PAGE_SIZE;
  } while (totalLength > 0 && retval);

  if (retval) {
    return BOOTLOADER_OK;
  } else {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }
}
