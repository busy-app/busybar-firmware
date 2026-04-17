/***************************************************************************//**
 * @file
 * @brief Non-Volatile Memory Wear-Leveling driver HAL implementation
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

#include <stdbool.h>
#include <string.h>
#include "nvm3.h"
#include "nvm3_hal_flash.h"
#include "sl_hal_system.h"
#include "sl_se_manager.h"
#include "sl_se_manager_extmem.h"

/***************************************************************************//**
 * @addtogroup nvm3
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup nvm3hal
 * @{
 ******************************************************************************/

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
static bool isErased(void *adr, size_t len)
{
  size_t cnt;
  uint32_t *dat = adr;

  cnt = len / sizeof(uint32_t);
  for (size_t i = 0U; i < cnt; i++) {
    if (*dat != 0xFFFFFFFFUL) {
      return false;
    }
    dat++;
  }

  return true;
}

/** @endcond */

/***************************************************************************//**
 * Open the NVM3 HAL for usage. SE will take care of clocking the external
 * flash. This function initializes the SE lock mutex and checks if NVM3
 * start and end addresses are within the data region or not.
 ******************************************************************************/
static sl_status_t nvm3_halFlashExtOpen(nvm3_HalPtr_t nvmAdr, size_t nvmSize)
{
  sl_status_t halSta;
  void *startAdr;
  size_t regSize;
  sl_se_command_context_t cmd_ctx;

  // Initialize SE lock mutex
  halSta = sl_se_init();
  if (halSta != SL_STATUS_OK) {
    return SL_STATUS_INITIALIZATION;
  }

  sl_se_init_command_context(&cmd_ctx);
  halSta = sl_se_data_region_get_location(&cmd_ctx, &startAdr, &regSize);

  if (halSta == SL_STATUS_OK) {
    // Check if NVM3 start and end addresses are within the data region or not
    if (((size_t)nvmAdr < (size_t)startAdr) || (((size_t)nvmAdr + nvmSize) > ((size_t)startAdr + regSize))) {
      return SL_STATUS_NVM3_INVALID_ADDR;
    }
  }

  return halSta;
}

/***************************************************************************//**
 * Close the NVM3 HAL. This function de-initializes the SE lock mutex.
 ******************************************************************************/
static void nvm3_halFlashExtClose(void)
{
  // De-initialize SE lock mutex
  sl_se_deinit();
}

/***************************************************************************//**
 * Retrieve device information.
 ******************************************************************************/
static sl_status_t nvm3_halFlashExtGetInfo(nvm3_HalInfo_t *halInfo)
{
  sl_hal_system_chip_revision_t chipRev;
  sl_hal_system_get_chip_revision(&chipRev);
#if defined(_SYSCFG_CHIPREV_PARTNUMBER_MASK)
  halInfo->deviceFamilyPartNumber = chipRev.part_number;
#elif defined(_SYSCFG_CHIPREV_FAMILY_MASK)
  halInfo->deviceFamilyPartNumber = chipRev.family;
#endif
  halInfo->memoryMapped = 1;
  halInfo->writeSize = NVM3_HAL_WRITE_SIZE_32;
  halInfo->pageSize = sl_hal_system_get_flash_page_size();

  return SL_STATUS_OK;
}

static void nvm3_halFlashExtAccess(nvm3_HalNvmAccessCode_t access)
{
  (void)access;
}

/***************************************************************************//**
 * This function is used to read data from the NVM.
 ******************************************************************************/
static sl_status_t nvm3_halFlashExtReadWords(nvm3_HalPtr_t nvmAdr, void *dst, size_t wordCnt)
{
  uint32_t *pSrc = (uint32_t *)nvmAdr;
  uint32_t *pDst = dst;

  if ((((size_t)pSrc % 4) == 0) && (((size_t)pDst % 4) == 0)) {
    while (wordCnt > 0U) {
      *pDst++ = *pSrc++;
      wordCnt--;
    }
  } else {
    (void)memcpy(dst, nvmAdr, wordCnt * sizeof(uint32_t));
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * This function is used to write data to the NVM.
 ******************************************************************************/
static sl_status_t nvm3_halFlashExtWriteWords(nvm3_HalPtr_t nvmAdr, void const *src, size_t wordCnt)
{
  sl_status_t halSta;
  sl_se_command_context_t cmd_ctx;
  size_t byteCnt;

  byteCnt = wordCnt * sizeof(uint32_t);
  sl_se_init_command_context(&cmd_ctx);
  halSta = sl_se_data_region_write(&cmd_ctx, (void *)nvmAdr, src, byteCnt);

#if CHECK_DATA
  if (halSta == SL_STATUS_OK) {
    if (memcmp(nvmAdr, src, byteCnt) != 0) {
      halSta = SL_STATUS_FLASH_PROGRAM_FAILED;
    }
  }
#endif

  return halSta;
}

/***************************************************************************//**
 * This function is used to erase an NVM page.
 ******************************************************************************/
static sl_status_t nvm3_halFlashExtPageErase(nvm3_HalPtr_t nvmAdr)
{
  sl_status_t halSta;
  sl_se_command_context_t cmd_ctx;
  size_t numSectors = 1;

  sl_se_init_command_context(&cmd_ctx);
  halSta = sl_se_data_region_erase(&cmd_ctx, (void *)nvmAdr, numSectors);

#if CHECK_DATA
  if (halSta == SL_STATUS_OK) {
    if (!isErased(nvmAdr, sl_hal_system_get_flash_page_size())) {
      halSta = SL_STATUS_FLASH_ERASE_FAILED;
    }
  }
#endif

  return halSta;
}

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

const nvm3_HalHandle_t nvm3_halFlashHandle = {
  .open = nvm3_halFlashExtOpen,                    ///< Set the open function
  .close = nvm3_halFlashExtClose,                  ///< Set the close function
  .getInfo = nvm3_halFlashExtGetInfo,              ///< Set the get-info function
  .access = nvm3_halFlashExtAccess,                ///< Set the access function
  .pageErase = nvm3_halFlashExtPageErase,          ///< Set the page-erase function
  .readWords = nvm3_halFlashExtReadWords,          ///< Set the read-words function
  .writeWords = nvm3_halFlashExtWriteWords,        ///< Set the write-words function
};

/** @} (end addtogroup nvm3hal) */
/** @} (end addtogroup nvm3) */
