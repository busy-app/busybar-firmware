/***************************************************************************//**
 * @file
 * @brief Application interface to the storage component of the bootloader.
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
#include "string.h"
#include "btl_interface.h"
#include "sl_se_manager.h"
#include "sl_se_manager_extmem.h"
#include "btl_storage_bootloadinfo.h"
#include "core/btl_parse.h"
#include "btl_storage_ext_mem.h"
// -----------------------------------------------------------------------------
// Defines

// Make assert no-op if not configured
#ifndef BTL_ASSERT
#define BTL_ASSERT(x)
#endif

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

static int32_t advance_parser(BootloaderParserContext_t         *ctx,
                              const BootloaderParserCallbacks_t *callbacks)
{
  uint8_t readBuffer[BTL_STORAGE_READ_BUFFER_SIZE];

  BootloaderStorageInformation_t info;
  bootloader_getStorageInfo(&info);

  // Ensure slot is valid
  if (ctx->slotId >= info.numStorageSlots) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_SLOT;
  }

  //Get slot info
  BootloaderStorageSlot_t storageSlot;
  bootloader_getStorageSlotInfo(ctx->slotId, &storageSlot);

  // Ensure address is within slot
  if (ctx->slotOffset + BTL_STORAGE_READ_BUFFER_SIZE > storageSlot.length) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }

  // Address range is valid; read data
  bootloader_readRawStorage(storageSlot.address + ctx->slotOffset,
                            readBuffer,
                            BTL_STORAGE_READ_BUFFER_SIZE);

  ctx->errorCode = parser_parse(&(ctx->parserContext),
                                &(ctx->imageProperties),
                                readBuffer,
                                BTL_STORAGE_READ_BUFFER_SIZE,
                                callbacks);

  ctx->slotOffset += BTL_STORAGE_READ_BUFFER_SIZE;

  return BOOTLOADER_OK;
}

static void dummy_callback(uint32_t       address,
                           uint8_t        *data,
                           size_t         length,
                           unsigned int   region_idx,
                           void           *context)
{
  (void) address;
#ifdef __ICCARM__
// Suppress MISRA error that casting pointer to void is disallowed
#pragma diag_suppress=Pm139
#endif
  (void) data;
  (void) length;
  (void) region_idx;
#ifdef __ICCARM__
#pragma diag_default=Pm139
#endif

  bool *receivedData = (bool *)context;
  *receivedData = true;
}

// --------------------------------
// API Functions
void bootloader_getStorageInfo(BootloaderStorageInformation_t *info)
{
  if (!bootloader_pointerValid(mainBootloaderTable)
      || !bootloader_pointerValid(mainBootloaderTable->storage)) {
    return;
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  int32_t retVal;
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return;
  }
#endif
  mainBootloaderTable->storage->getInfo(info);
#if defined(SL_CATALOG_KERNEL_PRESENT)
  retVal = btl_osal_lock_release();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return;
  }
#endif
}

int32_t bootloader_getStorageSlotInfo(uint32_t                slotId,
                                      BootloaderStorageSlot_t *slot)
{
  int32_t retVal;
  if (!bootloader_pointerValid(mainBootloaderTable)
      || !bootloader_pointerValid(mainBootloaderTable->storage)) {
    return BOOTLOADER_ERROR_INIT_TABLE;
  }

#if defined(SL_CATALOG_KERNEL_PRESENT)
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return retVal;
  }
#endif
  retVal = mainBootloaderTable->storage->getSlotInfo(slotId, slot);
#if defined(SL_CATALOG_KERNEL_PRESENT)
  int32_t releaseRetVal = btl_osal_lock_release();
  if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return releaseRetVal;
  }
#endif
  return retVal;
}

void bootloader_getAppMinVersion(uint32_t *version)
{
  if (!bootloader_pointerValid(mainBootloaderTable)
      || !bootloader_pointerValid(mainBootloaderTable->storage)) {
    return;
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  int32_t retVal;
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return;
  }
#endif
  mainBootloaderTable->storage->getAppMinVersion(version);
#if defined(SL_CATALOG_KERNEL_PRESENT)
  retVal = btl_osal_lock_release();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return;
  }
#endif
}

int32_t bootloader_readStorage(uint32_t slotId,
                               uint32_t offset,
                               uint8_t  *buffer,
                               size_t   length)
{
  int32_t retVal;
#if defined(SL_CATALOG_KERNEL_PRESENT)
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return retVal;
  }
#endif

  BootloaderStorageInformation_t info;
  bootloader_getStorageInfo(&info);

  // Ensure slot is valid
  if (slotId >= info.numStorageSlots) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_SLOT;
  }

  BootloaderStorageSlot_t storageSlot;
  bootloader_getStorageSlotInfo(slotId, &storageSlot);

  // Ensure address is within slot
  if ((offset + length > storageSlot.length) \
      || (offset > storageSlot.length)       \
      || (length > storageSlot.length)) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }

  retVal = bootloader_readRawStorage(storageSlot.address + offset,
                                     buffer,
                                     length);
#if defined(SL_CATALOG_KERNEL_PRESENT)
  int32_t releaseRetVal = btl_osal_lock_release();
  if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return releaseRetVal;
  }
#endif
  return retVal;
}

int32_t bootloader_writeStorage(uint32_t slotId,
                                uint32_t offset,
                                uint8_t  *buffer,
                                size_t   length)
{
  int32_t retVal;
#if defined(SL_CATALOG_KERNEL_PRESENT)
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return retVal;
  }
#endif

  BootloaderStorageInformation_t info;
  bootloader_getStorageInfo(&info);

  // Ensure slot is valid
  if (slotId >= info.numStorageSlots) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_SLOT;
  }

  BootloaderStorageSlot_t storageSlot;
  bootloader_getStorageSlotInfo(slotId, &storageSlot);

  // Ensure address is within slot
  if ((offset + length > storageSlot.length) \
      || (offset > storageSlot.length)       \
      || (length > storageSlot.length)) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }

  retVal = bootloader_writeRawStorage(storageSlot.address + offset,
                                      buffer,
                                      length);
#if defined(SL_CATALOG_KERNEL_PRESENT)
  int32_t releaseRetVal = btl_osal_lock_release();
  if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return releaseRetVal;
  }
#endif
  return retVal;
}

int32_t bootloader_eraseWriteStorage(uint32_t slotId,
                                     uint32_t offset,
                                     uint8_t  *buffer,
                                     size_t   length)
{
  int32_t retVal;
  uint32_t storageStartAddr;
  uint32_t eraseOffset;
  uint32_t eraseLength;
  BootloaderStorageSlot_t storageSlot;

  if (!bootloader_pointerValid(mainBootloaderTable)
      || !bootloader_pointerValid(mainBootloaderTable->storage)) {
    return BOOTLOADER_ERROR_INIT_TABLE;
  }

  if (FLASH_PAGE_SIZE == 0) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_SLOT;
  }

  retVal = bootloader_getStorageSlotInfo(slotId, &storageSlot);
  if (retVal != BOOTLOADER_OK) {
    return retVal;
  }
  storageStartAddr = storageSlot.address;

  if (offset + length > storageSlot.length) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }

  if (offset % FLASH_PAGE_SIZE) {
    // Erase from next page:
    eraseOffset = (offset & ~(FLASH_PAGE_SIZE - 1)) + FLASH_PAGE_SIZE;

    if ((offset + length) % FLASH_PAGE_SIZE) {
      // Example case for this if/else section:
      // 0    1    2    3
      // |----|----|----|
      //   ^          ^
      //   O          L
      eraseLength = ((offset + length) & ~(FLASH_PAGE_SIZE - 1)) + FLASH_PAGE_SIZE - eraseOffset;
    } else {
      // Example case for this if/else section:
      // 0    1    2    3
      // |----|----|----|
      //   ^            ^
      //   O            L
      eraseLength = length - (FLASH_PAGE_SIZE - (offset % FLASH_PAGE_SIZE));
    }
    eraseOffset = storageStartAddr + eraseOffset;
  } else {
    eraseOffset = storageStartAddr + offset;
    if (length % FLASH_PAGE_SIZE) {
      // Example case for this if/else section:
      // 0    1    2    3
      // |----|----|----|
      //      ^       ^
      //      O       L
      eraseLength = (length & ~(FLASH_PAGE_SIZE - 1)) + FLASH_PAGE_SIZE;
    } else {
      // Example case for this if/else section:
      // 0    1    2    3
      // |----|----|----|
      //      ^         ^
      //      O         L
      eraseLength = length;
    }
  }
  if (eraseLength != 0) {
    retVal = bootloader_eraseRawStorage(eraseOffset, eraseLength);
    if (retVal != BOOTLOADER_OK) {
      return retVal;
    }
  }

  retVal = bootloader_writeRawStorage(storageStartAddr + offset, buffer, length);
  if (retVal != BOOTLOADER_OK) {
    return retVal;
  }

  return BOOTLOADER_OK;
}

int32_t bootloader_eraseStorageSlot(uint32_t slotId)
{
  int32_t retVal;
#if defined(SL_CATALOG_KERNEL_PRESENT)
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return retVal;
  }
#endif

  BootloaderStorageInformation_t info;
  bootloader_getStorageInfo(&info);

  // Ensure slot is valid
  if (slotId >= info.numStorageSlots) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_SLOT;
  }

  BootloaderStorageSlot_t storageSlot;
  bootloader_getStorageSlotInfo(slotId, &storageSlot);

  retVal = bootloader_eraseRawStorage(storageSlot.address, storageSlot.length);
#if defined(SL_CATALOG_KERNEL_PRESENT)
  int32_t releaseRetVal = btl_osal_lock_release();
  if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return releaseRetVal;
  }
#endif
  return retVal;
}

int32_t bootloader_initChunkedEraseStorageSlot(uint32_t                slotId,
                                               BootloaderEraseStatus_t *eraseStat)
{
  int32_t retVal;

  retVal = bootloader_getStorageSlotInfo(slotId, &eraseStat->storageSlotInfo);
  if (retVal != BOOTLOADER_OK) {
    return retVal;
  }

  eraseStat->currentPageAddr = eraseStat->storageSlotInfo.address;
  eraseStat->pageSize = FLASH_PAGE_SIZE;

  return BOOTLOADER_OK;
}

int32_t bootloader_chunkedEraseStorageSlot(BootloaderEraseStatus_t *eraseStat)
{
  int32_t retVal;
  if (eraseStat->currentPageAddr
      == (eraseStat->storageSlotInfo.address + eraseStat->storageSlotInfo.length)) {
    return BOOTLOADER_OK;
  }

  retVal = bootloader_eraseRawStorage(eraseStat->currentPageAddr, eraseStat->pageSize);
  if (retVal != BOOTLOADER_OK) {
    return retVal;
  }

  eraseStat->currentPageAddr += eraseStat->pageSize;
  if (eraseStat->currentPageAddr
      == (eraseStat->storageSlotInfo.address + eraseStat->storageSlotInfo.length)) {
    return BOOTLOADER_OK;
  }

  return BOOTLOADER_ERROR_STORAGE_CONTINUE;
}

int32_t bootloader_setImageToBootload(int32_t slotId)
{
  int32_t retVal;
  if (!bootloader_pointerValid(mainBootloaderTable)
      || !bootloader_pointerValid(mainBootloaderTable->storage)) {
    return BOOTLOADER_ERROR_INIT_TABLE;
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return retVal;
  }
#endif
  retVal = storage_set_bootload_list(&slotId, 1);
#if defined(SL_CATALOG_KERNEL_PRESENT)
  int32_t releaseRetVal = btl_osal_lock_release();
  if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return releaseRetVal;
  }
#endif
  return retVal;
}

int32_t bootloader_setImagesToBootload(int32_t *slotIds, size_t length)
{
  int32_t retVal;
  if (!bootloader_pointerValid(mainBootloaderTable)
      || !bootloader_pointerValid(mainBootloaderTable->storage)) {
    return BOOTLOADER_ERROR_INIT_TABLE;
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return retVal;
  }
#endif
  retVal = storage_set_bootload_list(slotIds, length);
#if defined(SL_CATALOG_KERNEL_PRESENT)
  int32_t releaseRetVal = btl_osal_lock_release();
  if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return releaseRetVal;
  }
#endif
  return retVal;
}

int32_t bootloader_getImagesToBootload(int32_t *slotIds, size_t length)
{
  int32_t retVal;
  if (!bootloader_pointerValid(mainBootloaderTable)
      || !bootloader_pointerValid(mainBootloaderTable->storage)) {
    return BOOTLOADER_ERROR_INIT_TABLE;
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return retVal;
  }
#endif
  retVal = storage_get_bootload_list(slotIds, length);
#if defined(SL_CATALOG_KERNEL_PRESENT)
  int32_t releaseRetVal = btl_osal_lock_release();
  if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return releaseRetVal;
  }
#endif
  return retVal;
}

int32_t bootloader_appendImageToBootloadList(int32_t slotId)
{
  int32_t retVal;
  if (!bootloader_pointerValid(mainBootloaderTable)
      || !bootloader_pointerValid(mainBootloaderTable->storage)) {
    return BOOTLOADER_ERROR_INIT_TABLE;
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return retVal;
  }
#endif
  retVal = storage_append_bootload_list(slotId);
#if defined(SL_CATALOG_KERNEL_PRESENT)
  int32_t releaseRetVal = btl_osal_lock_release();
  if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return releaseRetVal;
  }
#endif
  return retVal;
}

int32_t bootloader_initVerifyImage(uint32_t slotId,
                                   void     *context,
                                   size_t   contextSize)
{
  int32_t retVal;

  BootloaderParserContext_t* ctx = (BootloaderParserContext_t*)context;

  #if defined(SL_CATALOG_KERNEL_PRESENT)
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return retVal;
  }
#endif

  retVal = core_init_parser(ctx, contextSize);
  if (retVal != BOOTLOADER_OK) {
#if defined(SL_CATALOG_KERNEL_PRESENT)
    int32_t releaseRetVal = btl_osal_lock_release();
    if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
      return releaseRetVal;
    }
#endif
    return retVal;
  }

  BootloaderStorageSlot_t slot;
  retVal = bootloader_getStorageSlotInfo(slotId, &slot);
  if (retVal != BOOTLOADER_OK) {
#if defined(SL_CATALOG_KERNEL_PRESENT)
    int32_t releaseRetVal = btl_osal_lock_release();
    if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
      return releaseRetVal;
    }
#endif
    return retVal;
  }

  ctx->slotId = slotId;
  ctx->slotSize = slot.length;

#if defined(SL_CATALOG_KERNEL_PRESENT)
  int32_t releaseRetVal = btl_osal_lock_release();
  if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return releaseRetVal;
  }
#endif
  return retVal;
}

int32_t bootloader_continueVerifyImage(void                       *context,
                                       BootloaderParserCallback_t metadataCallback)
{
  int32_t retVal;

  //Typecast context to BootloaderParserContext_t*
  BootloaderParserContext_t* ctx = (BootloaderParserContext_t*)context;

#if defined(SL_CATALOG_KERNEL_PRESENT)
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return retVal;
  }
#endif

  const BootloaderParserCallbacks_t parseCb = {
    NULL,
    NULL,
    metadataCallback,
    NULL
  };

  if ((ctx->errorCode == 0) && (ctx->slotOffset < ctx->slotSize)) {
    // There is still data left to parse
    advance_parser(ctx, &parseCb);

    if ((ctx->errorCode != BOOTLOADER_OK)
        && (ctx->errorCode != BOOTLOADER_ERROR_PARSER_EOF)) {
      retVal = ctx->errorCode;
    } else {
      retVal = BOOTLOADER_ERROR_PARSE_CONTINUE;
    }
  } else {
    // Parsing is complete, perform verification
    if ((ctx->imageProperties.imageCompleted)
        && (ctx->imageProperties.imageVerified)) {
      retVal = BOOTLOADER_ERROR_PARSE_SUCCESS;
    } else {
      retVal = BOOTLOADER_ERROR_PARSE_FAILED;
    }
  }

#if defined(SL_CATALOG_KERNEL_PRESENT)
  int32_t releaseRetVal = btl_osal_lock_release();
  if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return releaseRetVal;
  }
#endif
  return retVal;
}

int32_t bootloader_verifyImage(uint32_t                   slotId,
                               BootloaderParserCallback_t metadataCallback)
{
  int32_t retVal;
  uint8_t context[BOOTLOADER_STORAGE_VERIFICATION_CONTEXT_SIZE];

  retVal = bootloader_initVerifyImage(slotId,
                                      context,
                                      BOOTLOADER_STORAGE_VERIFICATION_CONTEXT_SIZE);

  if (retVal != BOOTLOADER_OK) {
    return retVal;
  }

  do {
    retVal = bootloader_continueVerifyImage(context, metadataCallback);
  } while (retVal == BOOTLOADER_ERROR_PARSE_CONTINUE);

  if (retVal == BOOTLOADER_ERROR_PARSE_SUCCESS) {
    return BOOTLOADER_OK;
  } else {
    return retVal;
  }
}

int32_t bootloader_getImageInfo(uint32_t          slotId,
                                ApplicationData_t *appInfo,
                                uint32_t          *bootloaderVersion)
{
  int32_t retVal;
  uint8_t context[BOOTLOADER_STORAGE_VERIFICATION_CONTEXT_SIZE];
  BootloaderParserContext_t *ctx = (BootloaderParserContext_t*)(&context);

  bool receivedData = false;

  const BootloaderParserCallbacks_t parseCb = {
    .context = &receivedData,
    .applicationCallback = &dummy_callback,
    .metadataCallback = &dummy_callback,
    .bootloaderCallback = &dummy_callback,
  };

  // Default versions to zero
  *bootloaderVersion = 0UL;
  memset(appInfo, 0, sizeof(ApplicationData_t));

  if (!bootloader_pointerValid(mainBootloaderTable)
      || !bootloader_pointerValid(mainBootloaderTable->storage)) {
    return BOOTLOADER_ERROR_PARSE_STORAGE;
  }

  // Check that the bootloader has image verification capability
  BTL_ASSERT(mainBootloaderTable->storage != NULL);
#if defined(SL_CATALOG_KERNEL_PRESENT)
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return retVal;
  }
#endif

  retVal = core_init_parser(ctx, BOOTLOADER_STORAGE_VERIFICATION_CONTEXT_SIZE);
  if (retVal != BOOTLOADER_OK) {
#if defined(SL_CATALOG_KERNEL_PRESENT)
    int32_t releaseRetVal = btl_osal_lock_release();
    if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
      return releaseRetVal;
    }
#endif
    return retVal;
  }

  BootloaderStorageSlot_t slot;
  retVal = bootloader_getStorageSlotInfo(slotId, &slot);
  if (retVal != BOOTLOADER_OK) {
#if defined(SL_CATALOG_KERNEL_PRESENT)
    int32_t releaseRetVal = btl_osal_lock_release();
    if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
      return releaseRetVal;
    }
#endif
    return retVal;
  }

  ctx->slotId = slotId;
  ctx->slotSize = slot.length;

  while ((!receivedData)
         && (ctx->errorCode == BOOTLOADER_OK)
         && (ctx->slotOffset < ctx->slotSize)) {
    // There is still data left to parse
    advance_parser(ctx, &parseCb);
  }

  if (ctx->imageProperties.contents & BTL_IMAGE_CONTENT_MEM_SEC_1) {
    *bootloaderVersion = ctx->imageProperties.bootloaderVersion;
  }
  if (ctx->imageProperties.contents & BTL_IMAGE_CONTENT_MEM_SEC_2) {
    memcpy(appInfo,
           &ctx->imageProperties.application,
           sizeof(ApplicationData_t));
  }

  if (receivedData || (ctx->imageProperties.contents)) {
    retVal = BOOTLOADER_OK;
  } else {
    retVal = BOOTLOADER_ERROR_STORAGE_NO_IMAGE;
  }

#if defined(SL_CATALOG_KERNEL_PRESENT)
  int32_t releaseRetVal = btl_osal_lock_release();
  if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return releaseRetVal;
  }
#endif
  return retVal;
}

bool bootloader_storageIsBusy(void)
{
  if (!bootloader_pointerValid(mainBootloaderTable)
      || !bootloader_pointerValid(mainBootloaderTable->storage)) {
    return true;
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  if (BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS != btl_osal_lock_aquire()) {
    return false;
  }
#endif
  bool retVal = mainBootloaderTable->storage->isBusy();
#if defined(SL_CATALOG_KERNEL_PRESENT)
  if (BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS != btl_osal_lock_release()) {
    return false;
  }
#endif
  return retVal;
}

int32_t bootloader_readRawStorage(uint32_t address,
                                  uint8_t  *buffer,
                                  size_t   length)
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

  memcpy(buffer, (void *)alias_address, length);

  return BOOTLOADER_OK;
}

int32_t bootloader_writeRawStorage(uint32_t address,
                                   uint8_t  *buffer,
                                   size_t   length)
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
  //Ensure address is within chip
  if (!verifyAddressRange(alias_address, length, (uint32_t) data_start, data_size)) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }

  //Ensure space is empty
  if (!verifyErased(alias_address, length)) {
    return BOOTLOADER_ERROR_STORAGE_NEEDS_ERASE;
  }

  status = sl_se_data_region_write(&cmd_ctx, (void*)alias_address, buffer, length);
  if (status != BOOTLOADER_OK) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }

  return BOOTLOADER_OK;
}

int32_t bootloader_eraseRawStorage(uint32_t address,
                                   size_t   length)
{
  // Ensure erase covers an integer number of pages
  if (length % FLASH_PAGE_SIZE) {
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
  //Ensure address is within flash
  if (!verifyAddressRange(alias_address, length, (uint32_t)data_start, data_size)) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }

  size_t num_sectors = length / FLASH_PAGE_SIZE;

  status = sl_se_data_region_erase(&cmd_ctx, (void*)alias_address, num_sectors);
  if (status != BOOTLOADER_OK) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }
  return BOOTLOADER_OK;
}
