/***************************************************************************//**
 * @file
 * @brief Storage component for Silicon Labs Bootloader (library part).
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
#include "core/btl_bootload.h"
#include "core/btl_parse.h"
#include "debug/btl_debug.h"
#include "parser/gbl/btl_gbl_parser.h"
#include "btl_storage_ext_mem.h"
#include "btl_storage.h"
#include <string.h>
/// Storage layout information for the storage implementation.
extern const BootloaderStorageLayout_t storageLayout;

const BootloaderStorageFunctions_t storageFunctions = {
  // Version number
  BOOTLOADER_STORAGE_FUNCTIONS_VERSION,
  // Get information about the storage -- capabilities, configuration
  &storage_get_info,
  // Get information about a storage slot -- size, location
  &storage_get_slot_info,
  // Read bytes from slot into buffer
  NULL,
  // Write bytes from buffer into slot
  NULL,
  // Get application minimum version
  &storage_get_app_min_version,
  // Mark a list of slots for bootload
  NULL,
  // Get list of slots marked for bootload
  NULL,
  // Append slot to list of slots marked for bootload
  NULL,
  // Start verification of image in a storage slot
  .initParseImage = NULL,
  // Continue verification of image in a storage slot
  .verifyImage = NULL,
  // Get application and bootloader upgrade metadata from storage slot
  .getImageInfo = &storage_get_slot_metadata,
  // Whether storage is busy
  .isBusy = &storage_is_busy,

  // Read raw bytes from storage
  .readRaw = NULL,
  // Write bytes to raw storage
  .writeRaw = NULL,
  // Erase storage
  .eraseRaw = NULL,
  // Get configured DMA channel
  .getDMAchannel = NULL,
};

// --------------------------------
// Internal prototypes

static void advance_parser(BootloaderParserContext_t         *ctx,
                           const BootloaderParserCallbacks_t *callbacks);
static bool bootload_from_slot(BootloaderParserContext_t         *context,
                               const BootloaderParserCallbacks_t *callbacks);
static void dummy_callback(uint32_t address,
                           uint8_t  *data,
                           size_t   length,
                           unsigned int region_idx,
                           void     *context);

// --------------------------------
// Storage Info

void storage_get_info(BootloaderStorageInformation_t *info)
{
  if (info == NULL) {
    return;
  }
  info->version = BOOTLOADER_STORAGE_INFO_VERSION;
  info->capabilities = 0UL;
  info->storageType = storageLayout.storageType;
  info->numStorageSlots = storageLayout.numSlots;
}

// --------------------------------
// Internal parsing routine

static void advance_parser(BootloaderParserContext_t         *ctx,
                           const BootloaderParserCallbacks_t *callbacks)
{
  uint8_t readBuffer[BTL_STORAGE_READ_BUFFER_SIZE];

  storage_read_slot(ctx->slotId,
                    ctx->slotOffset,
                    readBuffer,
                    BTL_STORAGE_READ_BUFFER_SIZE);
  ctx->parserContext.slotId = ctx->slotId;
  ctx->errorCode = parser_parse(&(ctx->parserContext),
                                &(ctx->imageProperties),
                                readBuffer,
                                BTL_STORAGE_READ_BUFFER_SIZE,
                                callbacks);
  ctx->slotOffset += BTL_STORAGE_READ_BUFFER_SIZE;
}

// --------------------------------
// Slot API

int32_t storage_get_slot_info(uint32_t slotId, BootloaderStorageSlot_t *slot)
{
  if (slot == NULL) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_SLOT;
  }
  if (slotId >= storageLayout.numSlots) {
    slot->address = 0UL;
    slot->length = 0UL;
    return BOOTLOADER_ERROR_STORAGE_INVALID_SLOT;
  }

  slot->address = storageLayout.slot[slotId].address;
  slot->length = storageLayout.slot[slotId].length;

  return BOOTLOADER_OK;
}

int32_t storage_read_slot(uint32_t slotId,
                          uint32_t offset,
                          uint8_t  *buffer,
                          size_t   numBytes)
{
  // Ensure slot is valid
  if (slotId >= storageLayout.numSlots) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_SLOT;
  }

  // Ensure address is within slot
  if (offset + numBytes > storageLayout.slot[slotId].length) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }

  // Address range is valid; read data
  return storage_read_raw(storageLayout.slot[slotId].address + offset,
                          buffer,
                          numBytes);
}

int32_t storage_write_slot(uint32_t slotId,
                           uint32_t offset,
                           uint8_t  *data,
                           size_t   numBytes)
{
  // Ensure slot is valid
  if (slotId >= storageLayout.numSlots) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_SLOT;
  }

  // Ensure address is within slot
  if (offset + numBytes > storageLayout.slot[slotId].length) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }

  return storage_write_raw(storageLayout.slot[slotId].address + offset,
                           data,
                           numBytes);
}

int32_t storage_erase_slot(uint32_t slotId)
{
  // Ensure slot is valid
  if (slotId >= storageLayout.numSlots) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_SLOT;
  }

  return storage_erase_raw(storageLayout.slot[slotId].address,
                           storageLayout.slot[slotId].length);
}

void storage_get_app_min_version(uint32_t *version)
{
#if defined(BOOTLOADER_ROLLBACK_PROTECTION) && (BOOTLOADER_ROLLBACK_PROTECTION == 1)
  *version = PARSER_APPLICATION_MINIMUM_VERSION_VALID;
#else
  *version = 0;
#endif
}
// --------------------------------
// Image Verification

int32_t storage_init_parse_slot(uint32_t                  slotId,
                                BootloaderParserContext_t *context,
                                size_t                    contextSize
                                )
{
  int32_t retval;
  retval = core_init_parser(context, contextSize);
  if (retval != BOOTLOADER_OK) {
    return retval;
  }

  BootloaderStorageSlot_t slot;
  retval = storage_get_slot_info(slotId, &slot);
  if (retval != BOOTLOADER_OK) {
    return retval;
  }

  context->slotId = slotId;
  context->slotSize = slot.length;

  return BOOTLOADER_OK;
}

int32_t storage_verify_slot(BootloaderParserContext_t  *context,
                            BootloaderParserCallback_t metadataCallback)
{
  const BootloaderParserCallbacks_t parseCb = {
    NULL,
    NULL,
    metadataCallback,
    NULL
  };

  if ((context->errorCode == 0) && (context->slotOffset < context->slotSize)) {
    // There is still data left to parse
    advance_parser(context, &parseCb);

    if ((context->errorCode != BOOTLOADER_OK)
        && (context->errorCode != BOOTLOADER_ERROR_PARSER_EOF)) {
      return context->errorCode;
    } else {
      return BOOTLOADER_ERROR_PARSE_CONTINUE;
    }
  } else {
    // Parsing is complete, perform verification
    if ((context->imageProperties.imageCompleted)
        && (context->imageProperties.imageVerified)) {
      return BOOTLOADER_ERROR_PARSE_SUCCESS;
    } else {
      return BOOTLOADER_ERROR_PARSE_FAILED;
    }
  }
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

int32_t storage_get_slot_metadata(BootloaderParserContext_t *context,
                                  ApplicationData_t         *appInfo,
                                  uint32_t                  *bootloaderVersion)
{
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

  while ((!receivedData)
         && (context->errorCode == BOOTLOADER_OK)
         && (context->slotOffset < context->slotSize)) {
    // There is still data left to parse
    advance_parser(context, &parseCb);
  }

  if (context->imageProperties.contents & BTL_IMAGE_CONTENT_MEM_SEC_1) {
    *bootloaderVersion = context->imageProperties.bootloaderVersion;
  }
  if (context->imageProperties.contents & BTL_IMAGE_CONTENT_MEM_SEC_2) {
    memcpy(appInfo,
           &context->imageProperties.application,
           sizeof(ApplicationData_t));
  }

  if (receivedData || (context->imageProperties.contents)) {
    return BOOTLOADER_OK;
  } else {
    return BOOTLOADER_ERROR_STORAGE_NO_IMAGE;
  }
}

// --------------------------------
// Bootloading Functions

// Generic implementation of bootload from slot
static bool bootload_from_slot(BootloaderParserContext_t         *context,
                               const BootloaderParserCallbacks_t *callbacks)
{
  parser_init(&(context->parserContext),
              &(context->decryptContext),
              &(context->primaryAuthContext),
              &(context->secondaryAuthContext),
              PARSER_FLAG_PARSE_CUSTOM_TAGS);

  // Run through the image and flash it
  while ((0 == context->errorCode)
         && (context->slotOffset < context->slotSize)) {
    advance_parser(context, callbacks);
  }

  if (!context->imageProperties.imageCompleted) {
    BTL_DEBUG_PRINT("Err ");
    BTL_DEBUG_PRINT_WORD_HEX((uint32_t)context->errorCode);
    BTL_DEBUG_PRINT_LF();
  }

  if ((context->imageProperties.imageCompleted)
      && (context->imageProperties.imageVerified)) {
    return true;
  } else {
    return false;
  }
}

// Bootload a bootloader image from slot
bool storage_bootload_bootloader_from_slot(uint32_t slotId, uint32_t version, BootloaderParserContext_t  *context)
{
  if (version <= mainBootloaderTable->header.version) {
    return false;
  }

  int32_t ret = storage_init_parse_slot(slotId,
                                        context,
                                        sizeof(BootloaderParserContext_t));

  if (ret != BOOTLOADER_OK) {
    return false;
  }

  // Only apply bootloader
  context->imageProperties.instructions = BTL_IMAGE_INSTRUCTION_MEM_SEC_1;

  const BootloaderParserCallbacks_t parseCb = {
    .context = NULL,
    .applicationCallback = NULL,
    .metadataCallback = NULL,
    .bootloaderCallback = bootload_bootloaderCallback
  };

  return bootload_from_slot(context, &parseCb);
}

// Bootload an application image from slot
bool storage_bootload_application_from_slot(uint32_t slotId, uint32_t version)
{
  (void)version;
  BootloaderParserContext_t parseCtx;

  int32_t ret = storage_init_parse_slot(slotId,
                                        &parseCtx,
                                        sizeof(BootloaderParserContext_t));

  if (ret != BOOTLOADER_OK) {
    return false;
  }

  // Only apply application
  parseCtx.imageProperties.instructions = BTL_IMAGE_INSTRUCTION_MEM_SEC_2;

  const BootloaderParserCallbacks_t parseCb = {
    .context = NULL,
    .applicationCallback = bootload_applicationCallback,
    .metadataCallback = NULL, // Metadata callback not used when bootloading app
    .bootloaderCallback = NULL
  };

  return bootload_from_slot(&parseCtx, &parseCb);
}
