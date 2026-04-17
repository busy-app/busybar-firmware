/***************************************************************************//**
 * @file
 * @brief Storage component for the Gecko Bootloader.
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
#include "btl_storage.h"
#ifndef BTL_CONFIG_FILE
#include "btl_storage_slot_cfg.h"
#include "btl_storage_cfg.h"
#endif

#include "core/btl_reset.h"
#include "core/btl_parse.h"
#include "core/btl_bootload.h"
#include "debug/btl_debug.h"
// --------------------------------
// Prototypes

static int32_t install_image_from_slot(int32_t slotId);

// --------------------------------
// Function implementations

uint32_t storage_get_base_address(void)
{
#ifdef BTL_STORAGE_BASE_ADDRESS
  return BTL_STORAGE_BASE_ADDRESS;
#else
  return 0;
#endif
}

int32_t storage_main(void)
{
#if BTL_STORAGE_BOOTLOAD_LIST_LENGTH == 1
  return install_image_from_slot(0);
#else
  int32_t ret;

  int32_t slotIds[BTL_STORAGE_BOOTLOAD_LIST_LENGTH] = { -1 };

  ret = storage_get_bootload_list(slotIds, BTL_STORAGE_BOOTLOAD_LIST_LENGTH);
  if ((ret != BOOTLOADER_OK)
      && (ret != BOOTLOADER_ERROR_BOOTLOAD_LIST_NO_LIST)) {
    BTL_DEBUG_PRINTLN("BI err");
    return ret;
  } else if (ret == BOOTLOADER_ERROR_BOOTLOAD_LIST_NO_LIST) {
    // Try to bootload in an sequential order
    for (uint8_t i = 0; i < BTL_STORAGE_BOOTLOAD_LIST_LENGTH; i++) {
      slotIds[i] = i;
    }
  }

  // Attempt to bootload given images in sequence
  for (size_t id = 0; id < BTL_STORAGE_BOOTLOAD_LIST_LENGTH; id++) {
    if (slotIds[id] == -1) {
      // Invalid slot ID; try the next one
      continue;
    }

    ret = install_image_from_slot(slotIds[id]);
    if (ret == BOOTLOADER_OK) {
      break;
    }
  }

  return ret;
#endif
}

static int32_t install_image_from_slot(int32_t slotId)
{
  BootloaderParserContext_t parseContext;
  int32_t ret;

  BTL_DEBUG_PRINT("Slot: ");
  BTL_DEBUG_PRINT_WORD_HEX(slotId);
  BTL_DEBUG_PRINT_LF();

  // Get info about the image marked for bootloading
  storage_init_parse_slot(slotId,
                          &parseContext,
                          sizeof(BootloaderParserContext_t));

  do {
    ret = storage_verify_slot(&parseContext, NULL);
  } while (ret == BOOTLOADER_ERROR_PARSE_CONTINUE);

  if (ret != BOOTLOADER_ERROR_PARSE_SUCCESS) {
    // Image in slot is non-existant or corrupt.
    // Continue to next image
    BTL_DEBUG_PRINTLN("Verify fail");
    return BOOTLOADER_ERROR_STORAGE_BOOTLOAD;
  }

  if ((parseContext.imageProperties.contents & BTL_IMAGE_CONTENT_SE)
      && bootload_checkSeUpgradeVersion(parseContext.imageProperties.seUpgradeVersion)) {
    // SE upgrade should be applied
    const BootloaderStorageSlot_t storageSlots[] = BTL_STORAGE_SLOTS;
    const uint32_t upgradeAddress = storageSlots[slotId].address
                                    + parseContext.parserContext.updateSeUnion.updateSe.tlvPosition;
    if (!bootload_commitSeUpgrade(upgradeAddress)) {
      BTL_DEBUG_PRINTLN("SE upgrade commit fail");
      return BOOTLOADER_ERROR_STORAGE_BOOTLOAD;
    }
  }

  if ((parseContext.imageProperties.contents & BTL_IMAGE_CONTENT_MEM_SEC_1)) {
    BTL_DEBUG_PRINT("BL upg ");
    BTL_DEBUG_PRINT_WORD_HEX(mainBootloaderTable->header.version);
    BTL_DEBUG_PRINT(" -> ");
    BTL_DEBUG_PRINT_WORD_HEX(parseContext.imageProperties.bootloaderVersion);
    BTL_DEBUG_PRINT_LF();
  }

  if ((parseContext.imageProperties.contents & BTL_IMAGE_CONTENT_MEM_SEC_1)
      && (parseContext.imageProperties.bootloaderVersion
          > mainBootloaderTable->header.version)) {
    // This is a bootloader upgrade, and we also have an application
    // available for after the bootloader upgrade is complete
    if (storage_bootload_bootloader_from_slot(
          parseContext.slotId,
          parseContext.imageProperties.bootloaderVersion, &parseContext)) {
      uint32_t upgradeAddress;
      if (parseContext.parserContext.compression_scheme == COMPRESSION_LZMA   \
          || parseContext.parserContext.compression_scheme == COMPRESSION_LZ4 \
          || parseContext.parserContext.encryption_scheme == ENC_AES_CTR_128) {
        upgradeAddress =  parseContext.parserContext.plainBootloaderAddress;
      } else {
        const BootloaderStorageSlot_t storageSlots[] = BTL_STORAGE_SLOTS;
        upgradeAddress = storageSlots[slotId].address
                         + parseContext.parserContext.currentUpdateMemorySection.updateMemorySection.memorySectionPos
                         + parseContext.parserContext.memSectionMetaDataLength;
      }

      if (!bootload_commitBootloaderUpgrade(upgradeAddress,
                                            parseContext.imageProperties.bootloaderUpgradeSize)) {
        // Bootloader upgrade failed; not a valid image
        BTL_DEBUG_PRINTLN("Btl upgrade commit fail");
      }
    } else {
      // Bootloader upgrade failed; not a valid image
      BTL_DEBUG_PRINTLN("Btl upgrade fail");
    }
    return BOOTLOADER_ERROR_STORAGE_BOOTLOAD;
  }

  if ((parseContext.imageProperties.contents & BTL_IMAGE_CONTENT_MEM_SEC_2)
      && !storage_bootload_application_from_slot(
        parseContext.slotId,
        parseContext.imageProperties.application.version)) {
    // App upgrade failed.
    BTL_DEBUG_PRINTLN("App upgrade fail");
    // Continue to next image
    return BOOTLOADER_ERROR_STORAGE_BOOTLOAD;
  }

  // Application was updated. Reboot into new image.
  sl_se_command_context_t cmd_ctx = { 0 };
  parseContext.parserContext.seCmdCtxInterface.init(&cmd_ctx);
  uint32_t version =  parseContext.parserContext.bundleVersionUnion.bundleVersion.bundleVersion;
  ret = parseContext.parserContext.flashUpgradeFileInterface.set_upgrade_file_version(&cmd_ctx, version);
  if (ret != BOOTLOADER_OK) {
    return BOOTLOADER_ERROR_PARSER_VERSION;
  }
  return BOOTLOADER_OK;
}
