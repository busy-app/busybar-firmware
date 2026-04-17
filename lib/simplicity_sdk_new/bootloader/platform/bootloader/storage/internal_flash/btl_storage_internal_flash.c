/***************************************************************************//**
 * @file
 * @brief Internal flash storage component for Silicon Labs Bootloader.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "core/flash/btl_internal_flash.h"

#include "storage/btl_storage.h"
#include "storage/internal_flash/btl_storage_internal_flash.h"

#include "debug/btl_debug.h"

#ifndef BTL_CONFIG_FILE
#include "btl_storage_slot_cfg.h"
#include "btl_internal_storage_cfg.h"
#endif

#include "core/btl_util.h"
MISRAC_DISABLE
#include "em_device.h"
MISRAC_ENABLE

#include <string.h>

// -----------------------------------------------------------------------------
// Globals

const BootloaderStorageLayout_t storageLayout = {
  INTERNAL_FLASH,
  BTL_STORAGE_NUM_SLOTS,
  BTL_STORAGE_SLOTS
};

// -----------------------------------------------------------------------------
// Statics

#if defined(_SILICON_LABS_32B_SERIES_2)
#define FLASH_ALIGNMENT   (4)
#else
#define FLASH_ALIGNMENT   (2)
#endif

static const BootloaderStorageImplementationInformation_t deviceInfo = {
  BOOTLOADER_STORAGE_IMPL_INFO_VERSION,
  (BOOTLOADER_STORAGE_IMPL_CAPABILITY_ERASE_SUPPORTED
   | BOOTLOADER_STORAGE_IMPL_CAPABILITY_PAGE_ERASE_REQUIRED),
  40, // Datasheet EFR32MG1: max 40 ms for page erase
  40, // Datasheet EFR32MG1: max 40 ms for mass erase
  FLASH_PAGE_SIZE,
  FLASH_SIZE,
  NULL,
  FLASH_ALIGNMENT,
  BOOTLOADER_STORAGE_INTERNAL_STORAGE
};

// -----------------------------------------------------------------------------
// Functions

// --------------------------------
// Internal Functions

BootloaderStorageImplementationInformation_t getDeviceInfo(void)
{
  return deviceInfo;
}
// --------------------------------
// API Functions

int32_t storage_init(void)
{
#if defined(_CMU_CLKEN1_MASK)
  CMU->CLKEN1_SET = CMU_CLKEN1_MSC;
#endif
  return BOOTLOADER_OK;
}

bool storage_isBusy(void)
{
  return MSC->STATUS & MSC_STATUS_BUSY;
}

int32_t storage_getDMAchannel(void)
{
#if (BOOTLOADER_MSC_DMA_WRITE == 1)
  return BOOTLOADER_MSC_DMA_CHANNEL;
#else
  return -1;
#endif
}

uint32_t storage_getSpiUsartPPUSATD(uint32_t *ppusatdNr)
{
  (void)ppusatdNr;
  return 0u;
}

int32_t storage_shutdown(void)
{
#if defined(_CMU_CLKEN1_MASK)
  CMU->CLKEN1_CLR = CMU_CLKEN1_MSC;
#endif
  return BOOTLOADER_OK;
}
