/***************************************************************************//**
 * @file
 * @brief Storage component for Silicon Labs Bootloader (library part).
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

#include "api/btl_interface.h"
#include "core/btl_bootload.h"
#include "core/btl_parse.h"
#include "debug/btl_debug.h"
#include "parser/gbl/btl_gbl_parser.h"
#include "btl_storage.h"
#include "btl_storage_internal.h"
#include "bootloadinfo/btl_storage_bootloadinfo.h"
#include "api/btl_interface_storage.h"
#include <string.h>

#include "core/flash/btl_internal_flash.h"

#if defined(BTL_PARSER_SUPPORT_DELTA_DFU)
#include "btl_delta_dfu_cfg.h"
#include "btl_crc32.h"
#include "common.h"
#include "ddfu/patch.h"
#include "core/btl_reset.h"
#include "btl_reset_info.h"
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif

const BootloaderStorageFunctions_t storageFunctions = {
  // Version number
  BOOTLOADER_STORAGE_FUNCTIONS_VERSION,
  // Get information about the storage -- capabilities, configuration
  &storage_getInfo,
  // Get information about a storage slot -- size, location
  &storage_getSlotInfo,
  // Read bytes from slot into buffer
  &storage_readSlot,
  // Write bytes from buffer into slot
  &storage_writeSlot,
  // Erase an entire slot
  &storage_eraseSlot,
  // Mark a list of slots for bootload
  &storage_setBootloadList,
  // Get list of slots marked for bootload
  &storage_getBootloadList,
  // Append slot to list of slots marked for bootload
  &storage_appendBootloadList,
  // Start verification of image in a storage slot
  .initParseImage = &storage_initParseSlot,
  // Continue verification of image in a storage slot
  .verifyImage = &storage_verifySlot,
  // Get application and bootloader upgrade metadata from storage slot
  .getImageInfo = &storage_getSlotMetadata,
  // Whether storage is busy
  .isBusy = &storage_isBusy,

  // Read raw bytes from storage
  .readRaw = &storage_readRaw,
  // Write bytes to raw storage
  .writeRaw = &storage_writeRaw,
  // Erase storage
  .eraseRaw = &storage_eraseRaw,
  // Get configured DMA channel
  .getDMAchannel = &storage_getDMAchannel,
};

#if defined(BTL_PARSER_SUPPORT_DELTA_DFU)

#define GET_PAGE_OFFSET(addr) ((uint32_t)(addr) & (FLASH_PAGE_SIZE - 1))
#define DELTA_DFU_WRITE_SIZE 32

struct callback_streams {
  BootloaderStorageInformation_t storageInfo;
  BootloaderStorageSlot_t slotInfo;
  uint32_t old_fw_base_addr;
  uint32_t old_fw_addr;
  uint32_t patch_base_addr;
  uint32_t patch_curr_addr;
  uint32_t new_fw_base_addr;
  uint32_t new_fw_addr;
  uint32_t new_fw_size;
  uint32_t patch_length;
  uint32_t data_index;
  uint8_t  data_buf[DELTA_DFU_WRITE_SIZE * 2];
};
#endif

// --------------------------------
// Internal prototypes

static void advanceParser(BootloaderParserContext_t         *ctx,
                          const BootloaderParserCallbacks_t *callbacks);
static bool bootloadFromSlot(BootloaderParserContext_t         *context,
                             const BootloaderParserCallbacks_t *callbacks);
static void dummyCallback(uint32_t address,
                          uint8_t  *data,
                          size_t   length,
                          void     *context);

// --------------------------------
// Storage Info

void storage_getInfo(BootloaderStorageInformation_t *info)
{
  if (info == NULL) {
    return;
  }
  info->version = BOOTLOADER_STORAGE_INFO_VERSION;
  info->capabilities = 0UL;
  info->storageType = storageLayout.storageType;
  info->numStorageSlots = storageLayout.numSlots;

  // Querying detailed information about attached flash device
  info->flashInfo = getDeviceInfo();
  // Update pointer to flash info
  info->info = &(info->flashInfo);
}

// --------------------------------
// Internal parsing routine

static void advanceParser(BootloaderParserContext_t         *ctx,
                          const BootloaderParserCallbacks_t *callbacks)
{
  #if defined(BOOTLOADER_SUPPORT_INTERNAL_STORAGE) && defined(_SILICON_LABS_32B_SERIES_2)
  // Only activate the check if we have a bootloader blob that will be parsed
  if (callbacks->bootloaderCallback == &bootload_bootloaderCallback) {
    uint32_t upgradeLocation = bootload_getUpgradeLocation();
    // Perform conservative check with the "worst" case upgrade size.
    uint32_t startAddr = storageLayout.slot[ctx->slotId].address + ctx->slotOffset;
    uint32_t endAddr = storageLayout.slot[ctx->slotId].address + ctx->slotOffset + BTL_STORAGE_READ_BUFFER_SIZE;
    if ((upgradeLocation >= startAddr)
        && (upgradeLocation < endAddr)) {
      ctx->errorCode = BOOTLOADER_ERROR_PARSER_OVERLAP;
      return;
    }

    if ((upgradeLocation < startAddr)
        && ((upgradeLocation + MINIMUM_REQUIRED_UPGRADE_SIZE) > startAddr)) {
      ctx->errorCode = BOOTLOADER_ERROR_PARSER_OVERLAP;
      return;
    }
  }
  #endif // BOOTLOADER_SUPPORT_INTERNAL_STORAGE

  uint8_t readBuffer[BTL_STORAGE_READ_BUFFER_SIZE];

  storage_readSlot(ctx->slotId,
                   ctx->slotOffset,
                   readBuffer,
                   BTL_STORAGE_READ_BUFFER_SIZE);
  ctx->errorCode = parser_parse(&(ctx->parserContext),
                                &(ctx->imageProperties),
                                readBuffer,
                                BTL_STORAGE_READ_BUFFER_SIZE,
                                callbacks);
  ctx->slotOffset += BTL_STORAGE_READ_BUFFER_SIZE;
}

// --------------------------------
// Slot API

int32_t storage_getSlotInfo(uint32_t slotId, BootloaderStorageSlot_t *slot)
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

int32_t storage_readSlot(uint32_t slotId,
                         uint32_t offset,
                         uint8_t  *buffer,
                         size_t   numBytes)
{
  // Ensure slot is valid
  if (slotId >= storageLayout.numSlots) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_SLOT;
  }

  // Ensure address is within slot
  if ((offset + numBytes > storageLayout.slot[slotId].length) \
      || (offset > storageLayout.slot[slotId].length)         \
      || (numBytes > storageLayout.slot[slotId].length)) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }

  // Address range is valid; read data
  return storage_readRaw(storageLayout.slot[slotId].address + offset,
                         buffer,
                         numBytes);
}

int32_t storage_writeSlot(uint32_t slotId,
                          uint32_t offset,
                          uint8_t  *data,
                          size_t   numBytes)
{
  // Ensure slot is valid
  if (slotId >= storageLayout.numSlots) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_SLOT;
  }

  // Ensure address is within slot
  if ((offset + numBytes > storageLayout.slot[slotId].length) \
      || (offset > storageLayout.slot[slotId].length)         \
      || (numBytes > storageLayout.slot[slotId].length)) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_ADDRESS;
  }

  return storage_writeRaw(storageLayout.slot[slotId].address + offset,
                          data,
                          numBytes);
}

int32_t storage_eraseSlot(uint32_t slotId)
{
  // Ensure slot is valid
  if (slotId >= storageLayout.numSlots) {
    return BOOTLOADER_ERROR_STORAGE_INVALID_SLOT;
  }

  return storage_eraseRaw(storageLayout.slot[slotId].address,
                          storageLayout.slot[slotId].length);
}

// --------------------------------
// Image Verification

int32_t storage_initParseSlot(uint32_t                  slotId,
                              BootloaderParserContext_t *context,
                              size_t                    contextSize
                              )
{
  int32_t retval;
  retval = core_initParser(context, contextSize);
  if (retval != BOOTLOADER_OK) {
    return retval;
  }

  BootloaderStorageSlot_t slot;
  retval = storage_getSlotInfo(slotId, &slot);
  if (retval != BOOTLOADER_OK) {
    return retval;
  }

  context->slotId = slotId;
  context->slotSize = slot.length;

#if defined (BTL_PARSER_SUPPORT_DELTA_DFU)
  context->parserContext.deltaPatchAddress = slot.address;
  context->parserContext.endOfStorageSlot = slot.address + slot.length;
#endif

  return BOOTLOADER_OK;
}

int32_t storage_verifySlot(BootloaderParserContext_t  *context,
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
    advanceParser(context, &parseCb);

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

static void dummyCallback(uint32_t       address,
                          uint8_t        *data,
                          size_t         length,
                          void           *context)
{
  (void) address;
#ifdef __ICCARM__
// Suppress MISRA error that casting pointer to void is disallowed
#pragma diag_suppress=Pm139
#endif
  (void) data;
  (void) length;
#ifdef __ICCARM__
#pragma diag_default=Pm139
#endif

  bool *receivedData = (bool *)context;
  *receivedData = true;
}

int32_t storage_getSlotMetadata(BootloaderParserContext_t *context,
                                ApplicationData_t         *appInfo,
                                uint32_t                  *bootloaderVersion)
{
  bool receivedData = false;

  const BootloaderParserCallbacks_t parseCb = {
    .context = &receivedData,
    .applicationCallback = &dummyCallback,
    .metadataCallback = &dummyCallback,
    .bootloaderCallback = &dummyCallback,
  };

  // Default versions to zero
  *bootloaderVersion = 0UL;
  memset(appInfo, 0, sizeof(ApplicationData_t));

  while ((!receivedData)
#if defined(BTL_PARSER_SUPPORT_CUSTOM_TAGS)
         && (context->parserContext.internalState != GblParserStateCustomTag)
#endif
         && (context->errorCode == BOOTLOADER_OK)
         && (context->slotOffset < context->slotSize)) {
    // There is still data left to parse
    advanceParser(context, &parseCb);
  }

  if (context->imageProperties.contents & BTL_IMAGE_CONTENT_BOOTLOADER) {
    *bootloaderVersion = context->imageProperties.bootloaderVersion;
  }
  if (context->imageProperties.contents & BTL_IMAGE_CONTENT_APPLICATION) {
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

#if defined (BTL_PARSER_SUPPORT_DELTA_DFU)
// Callback implementation for reading current firmware for ddfu library
static bool read_old_firmware(size_t offset, size_t nbytes, uint8_t *out_buf, void *user_ctx)
{
  if (nbytes == 0x00) {
    return false;
  }
  const struct callback_streams *ctx;
  ctx = (struct callback_streams *)user_ctx;
  uint32_t read_addr = ctx->old_fw_addr + offset;
  if (read_addr >= BTL_APPLICATION_BASE) {
    //old firmware will always be in the internal flash.
    memcpy(out_buf, (const void *)read_addr, nbytes);
  } else {
    return false;
  }
  return true;
}

//Callback implementation to write the re-created firmware for ddfu library
static bool write_new_firmware(uint8_t *buf, size_t nbyte, void *user_ctx)
{
  uint32_t ret = BOOTLOADER_OK;
  struct callback_streams *ctx = (struct callback_streams *)user_ctx;

  if (nbyte == 0x00) {
    return false;
  }

  if (ctx->new_fw_addr >= (ctx->slotInfo.address + ctx->slotInfo.length)) {
    return false;
  }

  for (uint32_t itr = 0; itr < nbyte; itr++) {
    ctx->data_buf[ctx->data_index] = buf[itr];
    ctx->data_index++;
    //Write 32 bytes at a time.
    if (ctx->data_index == DELTA_DFU_WRITE_SIZE) {
      ret = storage_writeRaw(ctx->new_fw_addr, ctx->data_buf, DELTA_DFU_WRITE_SIZE);
      if (ret != BOOTLOADER_OK) {
        return false;
      }
      ctx->new_fw_addr += DELTA_DFU_WRITE_SIZE;
      ctx->data_index = 0;
      memset(ctx->data_buf, 0xFF, DELTA_DFU_WRITE_SIZE);
    }
  }
  return true;
}

// Callback implementation to fill firmware gaps with 0xFF.
static bool seek_new_firmware(size_t nbyte, void *user_ctx)
{
  uint32_t ret = BOOTLOADER_OK;
  struct callback_streams *ctx = (struct callback_streams *)user_ctx;
  for (uint32_t itr = 0; itr < nbyte; itr++) {
    ctx->data_buf[ctx->data_index] = 0xFF;
    ctx->data_index++;
    //Write 32 bytes at a time.
    if (ctx->data_index == DELTA_DFU_WRITE_SIZE) {
      ret = storage_writeRaw((uint32_t)ctx->new_fw_addr, ctx->data_buf, DELTA_DFU_WRITE_SIZE);
      if (ret != BOOTLOADER_OK) {
        return false;
      }
      ctx->new_fw_addr += DELTA_DFU_WRITE_SIZE;
      ctx->data_index = 0;
      memset(ctx->data_buf, 0xFF, DELTA_DFU_WRITE_SIZE);
    }
  }
  return true;
}

// Callback implementation to indicate end of patch for ddfu library
static bool is_end_of_patch(void *user_ctx)
{
  struct callback_streams *ctx = (struct callback_streams *)user_ctx;

  if (ctx->patch_curr_addr < (ctx->patch_base_addr + ctx->patch_length)) {
    return false;
  } else {
    if (ctx->data_index != 0) {
      uint32_t ret = BOOTLOADER_OK;
      ret = storage_writeRaw((uint32_t)ctx->new_fw_addr, (uint8_t *)ctx->data_buf, DELTA_DFU_WRITE_SIZE);
      if (ret == BOOTLOADER_OK) {
        ctx->new_fw_addr += ctx->data_index;
        return true;
      }
    }
    return true;
  }
}

// Callback implementation to read patch for delta dfu library
static bool read_patch(size_t nbyte, void *out_buf, void *user_ctx)
{
  struct callback_streams *ctx = (struct callback_streams *)user_ctx;

  if (ctx->patch_curr_addr > (ctx->patch_base_addr + ctx->patch_length)) {
    return false;
  }


#if defined(BTL_DELTA_DFU_EXTRACT_TO_RAM) && (BTL_DELTA_DFU_EXTRACT_TO_RAM)
  // If patch is in RAM, copy directly from heap
  memcpy(out_buf, (void *)ctx->patch_curr_addr, nbyte);
#else
  storage_readRaw(ctx->patch_curr_addr, out_buf, nbyte);
#endif

  ctx->patch_curr_addr = ctx->patch_curr_addr + nbyte;

  return true;
}

// Function to init the callback stream struct
void delta_dfu_init(struct callback_streams *user_ctx)
{
  storage_getInfo(&user_ctx->storageInfo);
  storage_getSlotInfo(0, &user_ctx->slotInfo);
  user_ctx->old_fw_addr = 0x00;
  user_ctx->old_fw_base_addr = 0x00;
  user_ctx->new_fw_addr = 0x00;
  user_ctx->new_fw_base_addr = 0x00;
  user_ctx->patch_base_addr  = 0x00;
  user_ctx->patch_curr_addr  = 0x00;
  user_ctx->patch_length = 0x00;
  memset(user_ctx->data_buf, 0xFF, DELTA_DFU_WRITE_SIZE * 2);
  user_ctx->data_index = 0;
}

bool copy_image_from_slot(struct callback_streams *user_ctx)
{
  struct callback_streams *ctx = user_ctx;
  uint32_t old_fw_rewrite_addr = ctx->old_fw_base_addr;

  if ((old_fw_rewrite_addr < BTL_APPLICATION_BASE)
      || ((old_fw_rewrite_addr + ctx->new_fw_size >= ctx->slotInfo.address)
          && (ctx->storageInfo.storageType == INTERNAL_FLASH))) {
    BTL_DEBUG_PRINTLN("Write Address out of bounds!");
    return false;
  }

  uint8_t num_flash_pages = ctx->new_fw_size / FLASH_PAGE_SIZE;
  if (ctx->new_fw_size % FLASH_PAGE_SIZE) {
    num_flash_pages++;
  }

  uint8_t erase_page_count = 0;
  while (erase_page_count < num_flash_pages) {
    flash_erasePage(old_fw_rewrite_addr + FLASH_PAGE_SIZE * erase_page_count);
    erase_page_count++;
  }

  int32_t num_of_bytes = ctx->new_fw_size;
  ctx->new_fw_addr = ctx->new_fw_base_addr;
  while (num_of_bytes > 0) {
    uint8_t temp_buf[DELTA_DFU_WRITE_SIZE] = { 0xFF };
    uint32_t read_ret = storage_readRaw(ctx->new_fw_addr, temp_buf, DELTA_DFU_WRITE_SIZE);
    if (read_ret != BOOTLOADER_OK) {
      return false;
    }

    if (old_fw_rewrite_addr == ctx->old_fw_base_addr) {
      memset(temp_buf, 0xff, 8);
    }

    if (!flash_writeBuffer(old_fw_rewrite_addr, temp_buf, DELTA_DFU_WRITE_SIZE)) {
      return false;
    }

    num_of_bytes = num_of_bytes - DELTA_DFU_WRITE_SIZE;
    ctx->new_fw_addr = ctx->new_fw_addr + DELTA_DFU_WRITE_SIZE;
    old_fw_rewrite_addr = old_fw_rewrite_addr + DELTA_DFU_WRITE_SIZE;
  }

  uint8_t temp_buf[8] = { 0xff };
  uint32_t read_ret = storage_readRaw(ctx->new_fw_base_addr, temp_buf, 8);
  if (read_ret != BOOTLOADER_OK) {
    return false;
  }
  if (!flash_writeBuffer(ctx->old_fw_base_addr, temp_buf, 8)) {
    return false;
  }

  return true;
}

bool calc_CRC(uint32_t newFwCRC, struct callback_streams *user_ctx)
{
  //Check CRC32 before copying to app location
  uint32_t new_fw_crc = BTL_CRC32_START;
  if (user_ctx->storageInfo.storageType == INTERNAL_FLASH) {
    new_fw_crc = btl_crc32Stream((void *)user_ctx->new_fw_base_addr,
                                 user_ctx->new_fw_size, new_fw_crc);
    new_fw_crc = btl_crc32Stream((void *)&newFwCRC, 4, new_fw_crc);
    if (new_fw_crc != BTL_CRC32_END) {
      return false;
    }
  } else if (user_ctx->storageInfo.storageType == SPIFLASH) {
    uint32_t crc_start = BTL_CRC32_START;
    uint32_t chunks = (user_ctx->new_fw_size) / DELTA_DFU_WRITE_SIZE;
    uint32_t last_chunk = (user_ctx->new_fw_size) % DELTA_DFU_WRITE_SIZE;
    uint32_t itr = 0;
    uint32_t addr = user_ctx->new_fw_base_addr;
    while (itr < chunks) {
      uint8_t buf[DELTA_DFU_WRITE_SIZE];
      storage_readRaw(addr, buf, DELTA_DFU_WRITE_SIZE);
      new_fw_crc = btl_crc32Stream(buf, DELTA_DFU_WRITE_SIZE, crc_start);
      crc_start = new_fw_crc;
      addr = addr + DELTA_DFU_WRITE_SIZE;
      itr++;
    }
    if (last_chunk > 0) {
      uint8_t buf[DELTA_DFU_WRITE_SIZE];
      storage_readRaw(addr, buf, last_chunk);
      new_fw_crc = btl_crc32Stream(buf, last_chunk, crc_start);
    }

    new_fw_crc = btl_crc32Stream((void *)&newFwCRC, 4, new_fw_crc);
    if (new_fw_crc != BTL_CRC32_END) {
      return false;
    }
  }
  return true;
}

#endif //BTL_PARSER_SUPPORT_DELTA_DFU

// --------------------------------
// Bootloading Functions

// Generic implementation of bootload from slot
static bool bootloadFromSlot(BootloaderParserContext_t         *context,
                             const BootloaderParserCallbacks_t *callbacks)
{
#if defined(BTL_PARSER_SUPPORT_DELTA_DFU)
  struct ddfu_patch_io_buffer ddfuBuff;
  struct callback_streams user_ctx;
  uint8_t ddfu_buffer[DELTA_DFU_WRITE_SIZE];
  ddfuBuff.data = ddfu_buffer;
  ddfuBuff.size = DELTA_DFU_WRITE_SIZE;

  delta_dfu_init(&user_ctx);
  const struct ddfu_patch_io io = {
    read_old_firmware,
    write_new_firmware,
    seek_new_firmware,
    read_patch,
    is_end_of_patch
  };

  uint32_t gblLength = context->parserContext.gblLength;
  uint32_t endOfSlot = context->parserContext.endOfStorageSlot;
#endif //BTL_PARSER_SUPPORT_DELTA_DFU

  parser_init(&(context->parserContext),
              &(context->decryptContext),
              &(context->authContext),
              PARSER_FLAG_PARSE_CUSTOM_TAGS);

#if defined(BTL_PARSER_SUPPORT_DELTA_DFU)
  //Restore the value of deltaGBLLength in the parser context
  context->parserContext.gblLength = gblLength;
  //Restore the value of end of storage slot
  context->parserContext.endOfStorageSlot = endOfSlot;
#endif //BTL_PARSER_SUPPORT_DELTA_DFU
  // Run through the image and flash it
  while ((0 == context->errorCode)
         && (context->slotOffset < context->slotSize)) {
    advanceParser(context, callbacks);
  }

  if (!context->imageProperties.imageCompleted) {
    BTL_DEBUG_PRINT("Err ");
    BTL_DEBUG_PRINT_WORD_HEX((uint32_t)context->errorCode);
    BTL_DEBUG_PRINT_LF();
  }

  if ((context->imageProperties.imageCompleted)
      && (context->imageProperties.imageVerified)) {
  #if defined(BTL_PARSER_SUPPORT_DELTA_DFU)
    if ((context->parserContext.newFwCRC != 0U) && (context->imageProperties.instructions == BTL_IMAGE_INSTRUCTION_APPLICATION)) {
      user_ctx.new_fw_addr = context->parserContext.programmingAddress;
    #if (BTL_DELTA_DFU_EXTRACT_TO_RAM)
      //When extracting to RAM, the delta patch address should be in RAM.
      user_ctx.new_fw_addr = user_ctx.slotInfo.address + gblLength;
    #endif
      if (GET_PAGE_OFFSET(user_ctx.new_fw_addr)) {
        user_ctx.new_fw_addr = (uint32_t)user_ctx.new_fw_addr
                               - ((uint32_t)user_ctx.new_fw_addr &  (FLASH_PAGE_SIZE - 1))
                               + FLASH_PAGE_SIZE;
      }

      user_ctx.new_fw_base_addr = user_ctx.new_fw_addr;
      user_ctx.patch_base_addr = context->parserContext.deltaPatchAddress;
      user_ctx.patch_curr_addr = context->parserContext.deltaPatchAddress;
      user_ctx.patch_length = context->parserContext.lengthOfPatch;
      user_ctx.old_fw_addr = BTL_APPLICATION_BASE;
      user_ctx.old_fw_base_addr = BTL_APPLICATION_BASE;
      user_ctx.new_fw_size = context->parserContext.newFwSize;

      uint32_t startOfAppSpace = BTL_APPLICATION_BASE;
      uint32_t pc = *(uint32_t *)(startOfAppSpace + 4);
      enum ddfu_patch_status ddfu_stat = DDFU_PATCH_STATUS_ERR;
      if (pc != 0xFFFFFFFF) {
        //Valid app present. Reconstruct-image.
        uint32_t slot_space = (user_ctx.slotInfo.address + user_ctx.slotInfo.length)
                              - user_ctx.new_fw_base_addr;

        if (user_ctx.new_fw_size > slot_space) {
          //Not enough space in slot. Reset with appropriate reset reason
          //Free up the RAM buffer if RAM based patch extraction is enabled
        #if (BTL_DELTA_DFU_EXTRACT_TO_RAM)
          free((void*)context->parserContext.deltaPatchAddress);
          context->parserContext.deltaPatchAddress = 0U;
        #endif
          reset_resetWithReason(BOOTLOADER_RESET_REASON_NO_SLOT_SPACE);
        }
        ddfu_stat = ddfu_patch_apply(&io, &ddfuBuff, &user_ctx);
      }

      #if (BTL_DELTA_DFU_EXTRACT_TO_RAM)
        //Free up the RAM buffer once the patch is applied
        free((void*)context->parserContext.deltaPatchAddress);
        context->parserContext.deltaPatchAddress = 0U;
      #endif
      
      if (ddfu_stat != DDFU_PATCH_STATUS_OK) {
        //The re-creation failed or was not carried out. Check if there is an image present already.
        if (calc_CRC(context->parserContext.newFwCRC, &user_ctx)) {
          BTL_DEBUG_PRINTLN("Found re-created firmware in slot");
          if (copy_image_from_slot(&user_ctx)) {
            return true;
          }
        } else {
          //CRC Calculation has failed.
          reset_resetWithReason(BOOTLOADER_RESET_REASON_DDFU_FAIL);
        }
      } else {
        BTL_DEBUG_PRINTLN("Re-construction complete.");
        //Check CRC32 before copying to app location
        if (calc_CRC(context->parserContext.newFwCRC, &user_ctx)) {
          BTL_DEBUG_PRINTLN("CRC is OK");
          if (!(context->imageProperties.contents & BTL_IMAGE_CONTENT_SE)
              && !(context->imageProperties.contents & BTL_IMAGE_CONTENT_BOOTLOADER)) {
            BTL_DEBUG_PRINTLN("Copying to app area.");
            if (copy_image_from_slot(&user_ctx)) {
              return true;
            } else {
              return false;
            }
          }
        } else {
          //CRC calculation has failed.
          BTL_DEBUG_PRINTLN("CRC calculation failed.");
          reset_resetWithReason(BOOTLOADER_RESET_REASON_BADCRC);
        }
      }
    }
  #endif //BTL_PARSER_SUPPORT_DELTA_DFU
    return true;
  } else {
  #if defined(BTL_PARSER_SUPPORT_DELTA_DFU) && (BTL_DELTA_DFU_EXTRACT_TO_RAM)
  // Parsing did not complete or image is not verified; free the RAM buffer
  free((void *)context->parserContext.deltaPatchAddress);
  context->parserContext.deltaPatchAddress = 0U;
  #endif

  #if defined(BTL_PARSER_SUPPORT_DELTA_DFU)
    // Parsing did not complete. Check if we ran out of slot space.
    if (context->errorCode == BOOTLOADER_ERROR_PARSER_OOB_WRITE) {
      //Reset with BOOTLOADER_RESET_REASON_NO_SLOT_SPACE
      BTL_DEBUG_PRINTLN("BOOTLOADER_ERROR_PARSER_OOB_WRITE. Reset device!");
      reset_resetWithReason(BOOTLOADER_RESET_REASON_NO_SLOT_SPACE);
    }
  #endif //BTL_PARSER_SUPPORT_DELTA_DFU
    return false;
  }
}

bool storage_upgradeSeFromSlot(uint32_t slotId)
{
  BootloaderParserContext_t parseCtx;
  int32_t ret = storage_initParseSlot(slotId,
                                      &parseCtx,
                                      sizeof(BootloaderParserContext_t));

  if (ret != BOOTLOADER_OK) {
    return false;
  }

  // Only apply SE
  parseCtx.imageProperties.instructions = BTL_IMAGE_INSTRUCTION_SE;

  const BootloaderParserCallbacks_t parseCb = {
    .context = NULL,
    .applicationCallback = NULL,
    .metadataCallback = NULL,
    .bootloaderCallback = bootload_bootloaderCallback
  };

  return bootloadFromSlot(&parseCtx, &parseCb);
}

// Bootload a bootloader image from slot
bool storage_bootloadBootloaderFromSlot(uint32_t slotId, uint32_t version)
{
  if (version <= mainBootloaderTable->header.version) {
    return false;
  }

  BootloaderParserContext_t parseCtx;
  int32_t ret = storage_initParseSlot(slotId,
                                      &parseCtx,
                                      sizeof(BootloaderParserContext_t));

  if (ret != BOOTLOADER_OK) {
    return false;
  }

  // Only apply bootloader
  parseCtx.imageProperties.instructions = BTL_IMAGE_INSTRUCTION_BOOTLOADER;

  const BootloaderParserCallbacks_t parseCb = {
    .context = NULL,
    .applicationCallback = NULL,
    .metadataCallback = NULL,
    .bootloaderCallback = bootload_bootloaderCallback
  };

  return bootloadFromSlot(&parseCtx, &parseCb);
}

// Bootload an application image from slot
bool storage_bootloadApplicationFromSlot(uint32_t slotId, uint32_t version, uint32_t deltaGBLLen)
{
  (void)version;
  BootloaderParserContext_t parseCtx;

  int32_t ret = storage_initParseSlot(slotId,
                                      &parseCtx,
                                      sizeof(BootloaderParserContext_t));
#if defined (BTL_PARSER_SUPPORT_DELTA_DFU)
  parseCtx.parserContext.gblLength = deltaGBLLen;
#else
  (void)deltaGBLLen;
#endif//BTL_PARSER_SUPPORT_DELTA_DFU
  if (ret != BOOTLOADER_OK) {
    return false;
  }

  // Only apply application
  parseCtx.imageProperties.instructions = BTL_IMAGE_INSTRUCTION_APPLICATION;

  const BootloaderParserCallbacks_t parseCb = {
    .context = &parseCtx,
    .applicationCallback = bootload_applicationCallback,
    .metadataCallback = NULL, // Metadata callback not used when bootloading app
    .bootloaderCallback = NULL
  };

  return bootloadFromSlot(&parseCtx, &parseCb);
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
