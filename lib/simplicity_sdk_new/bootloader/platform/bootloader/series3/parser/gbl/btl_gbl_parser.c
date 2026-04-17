/***************************************************************************//**
 * @file
 * @brief GBL image file parser
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
#include <string.h>
#include <stdlib.h>
#include "config/btl_config.h"

#include "parser/gbl/btl_gbl_parser.h"
#include "parser/gbl/btl_gbl_format.h"
#include "parser/gbl/btl_gbl_custom_tags.h"

#include "api/btl_interface.h"

#include "security/btl_security_aes.h"
#include "security/btl_security_sha256.h"
#include "security/btl_security_ecdsa.h"
#include "security/btl_security_types.h"

#include "core/btl_util.h"
#include "core/btl_bootload.h"

MISRAC_DISABLE
#include "em_device.h"
MISRAC_ENABLE

#ifdef BOOTLOADER_SUPPORT_STORAGE
#include "sl_clock_manager_init.h"
#include "btl_storage_cfg.h"
#include "storage/btl_storage.h"
#include "debug/btl_debug.h"
#endif

int32_t parser_idle(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parser_error(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parse_v4(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parse_manifest(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parse_updateProcess(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parse_certificate(ParserContext_t *ctx, InputBuffer_t *input);
int32_t process_certificate(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parse_signature(ParserContext_t *ctx, InputBuffer_t *input);
int32_t process_signature(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parse_manifestInfo(ParserContext_t *ctx, InputBuffer_t *input);
int32_t process_manifestInfo(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parse_bundleVersion(ParserContext_t *ctx, InputBuffer_t *input);
int32_t process_bundleVersion(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parse_contentHash(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parse_updateSe(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parse_updateMemorySection(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parse_manifestStateDone(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parse_seBlob(ParserContext_t *ctx, InputBuffer_t *input, const BootloaderParserCallbacks_t* callbacks);
int32_t parse_completeSEblob(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parse_memorySection(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parse_memorySectionInfo(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parse_ignorePaddingData(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parse_memorySectionBlob(ParserContext_t* context, InputBuffer_t* buffer, const BootloaderParserCallbacks_t* callbacks);
int32_t parse_rcvBlockOfHashes(ParserContext_t *ctx, InputBuffer_t *input);
int32_t process_memorySectionBlob(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parse_completeMemorySection(ParserContext_t *ctx, InputBuffer_t *input);

void updateSHAContextBasedOnTag(ParserContext_t *ctx, const uint8_t* data, size_t length);
int32_t streamToBufferTransfer(ParserContext_t *ctx, uint8_t *dest, size_t destSize, InputBuffer_t *input, size_t numBytes);
int32_t parser_finalize(ParserContext_t *ctx, InputBuffer_t *input);
int32_t parser_stateDone(ParserContext_t *ctx, InputBuffer_t *input);

void handleManifestSHA(ParserContext_t *ctx, const uint8_t* data, size_t length);
void handleMemorySectionInfoSHA(ParserContext_t *ctx, const uint8_t* data, size_t length);
void handleMemorySectionSHA(ParserContext_t *ctx, const uint8_t* data, size_t length);

// -----------------------------------------------------------------------------
// Macros
#define GBL_PARSER_ARRAY_TO_U32(array, offset)         \
  ((uint32_t)((uint32_t)((array)[(offset) + 3]) << 24) \
   | ((uint32_t)((array)[(offset) + 2]) << 16)         \
   | ((uint32_t)((array)[(offset) + 1]) << 8)          \
   | ((uint32_t)((array)[(offset) + 0]) << 0))

#define FLASH_WRITE_BUFFER_SIZE 2048
#define BLOB_BUFFER_SIZE 2048
#define FLASH_ERASE_BLOCK_SIZE 32768
static uint8_t blobBuffer[BLOB_BUFFER_SIZE];

const HashInterface_t sha256Interface = {
  .init   = &btl_initSha256,
  .update = &btl_updateSha256,
  .verify = &btl_verifySha256,
  .final  = &btl_finalizeSha256,
};

const FlashCodeRegionInterface_t flashCodeRegion = {
  .code_region_get_config        = &sl_se_code_region_get_config,
  .code_region_apply_config      = &sl_se_code_region_apply_config,
  .code_region_get_version       = &sl_se_code_region_get_version,
};

const FlashWriteInterface_t flashWrite = {
  .code_region_partial_erase = &sl_se_code_region_partial_erase,
  .code_region_erase = &sl_se_code_region_erase,
  .code_region_write = &sl_se_code_region_write,
  .code_region_close = &sl_se_code_region_close,
};

const FlashDataRegionInterface_t flashDataRegion = {
  .data_region_get_location = &sl_se_data_region_get_location,
  .data_region_write        = &sl_se_data_region_write,
  .data_region_erase        = &sl_se_data_region_erase,
};

const FlashUpgradeFileInterface_t flashUpgradeFile = {
  .get_upgrade_file_version = &sl_se_get_upgrade_file_version,
  .set_upgrade_file_version = &sl_se_set_upgrade_file_version,
};

const AESInterface_t AESInterface = {
  .init    = &btl_initAesCcm,
  .process = &btl_processAesCtrData,
};

const EcdsaP256r1Interface_t EcdsaP256r1 = {
  .verify = btl_verifyEcdsaP256r1,
};

const SECmdCtxInterface_t seCmdCtx = {
  .init = sl_se_init_command_context,
};

const ParserStateHandlerMap_t paserStateHandlerMap[] = {
  { ParserStateIdle, { .standard = parser_idle }, false },
  { ParserStatev4, { .standard = parse_v4 }, false },
  { ParserStateManifest, { .standard = parse_manifest }, false },
  { ParserStateCertificate, { .standard = parse_certificate }, false },
  { ParserStateSignature, { .standard = parse_signature }, false },
  { ParserStateManifestInfo, { .standard = parse_manifestInfo }, false },
  { ParserStateBundleVersion, { .standard = parse_bundleVersion }, false },
  { ParserStateContentHash, { .standard = parse_contentHash }, false },
  { ParserStateUpdateProcess, { .standard = parse_updateProcess }, false },
  { ParserStateUpdateSe, { .standard = parse_updateSe }, false },
  { ParserStateUpdateMemorySection, { .standard = parse_updateMemorySection }, false },
  { ParserStateManifestDone, { .standard = parse_manifestStateDone }, false },
  { ParserStateProcessCertificate, { .standard = process_certificate }, false },
  { ParserStateProcessSignature, { .standard = process_signature }, false },
  { ParserStateProcessManifestInfo, { .standard = process_manifestInfo }, false },
  { ParserStateProcessBundleVersion, { .standard = process_bundleVersion }, false },
  { ParserStateCompleteMemorySection, { .standard = parse_completeMemorySection }, false },
  { ParserStateCompleteSeBlob, { .standard = parse_completeSEblob }, false },
  { ParserStateSeBlob, { .withCallback = parse_seBlob }, true },
  { ParserStateMemorySection, { .standard = parse_memorySection }, false },
  { ParserStateMemorySectionInfo, { .standard = parse_memorySectionInfo }, false },
  { ParserStateIgnorePaddingData, { .standard = parse_ignorePaddingData }, false },
  { ParserStateReceiveBlockOfHashes, { .standard = parse_rcvBlockOfHashes }, false },
  { ParserStateMemorySectionBlob, { .withCallback = parse_memorySectionBlob }, true },
  { ParserStateDone, { .standard = parser_stateDone }, false },
  { ParserStateError, { .standard = parser_error }, false },
  { ParserStateNone, { .standard = NULL }, false }
};

const GblTagStateMap_t tagParserStateTbl[] = {
  { GBL_TAG_ID_V4, ParserStatev4 },
  { GBL_TAG_ID_MANIFEST, ParserStateManifest },
  { GBL_TAG_ID_UPDATE_PROCESS, ParserStateUpdateProcess },
  { GBL_TAG_ID_CERTIFICATE, ParserStateCertificate },
  { GBL_TAG_ID_SIGNATURE, ParserStateSignature },
  { GBL_TAG_ID_MANIFEST_INFO, ParserStateManifestInfo },
  { GBL_TAG_ID_BUNDLE_VERSION, ParserStateBundleVersion },
  { GBL_TAG_ID_CONTENT_HASH, ParserStateContentHash },
  { GBL_TAG_ID_UPDATE_SE, ParserStateUpdateSe },
  { GBL_TAG_ID_UPDATE_MEMORY_SECTION, ParserStateUpdateMemorySection },
  { GBL_TAG_ID_MANIFEST_FINISH, ParserStateManifestDone },
  { GBL_TAG_ID_MEMORY_SECTION, ParserStateCompleteMemorySection },
  { GBL_TAG_ID_SE_BLOB, ParserStateCompleteSeBlob },
  { GBL_TAG_ID_MEMORY_SECTION_INFO, ParserStateMemorySectionInfo },
  { GBL_TAG_ID_MEMORY_SECTION_BLOB, ParserStateMemorySectionBlob },
  { GBL_TAG_ID_PAD, ParserStateIgnorePaddingData },
  { 0, ParserStateError }
};

// ID:BatchUpgrade: StreamingUpgrade
const TagShaHandlerMap_t tagShaHandlerTbl[] = {
  { GBL_TAG_ID_V4, NULL, NULL },
  { GBL_TAG_ID_MANIFEST, NULL, NULL },
  { GBL_TAG_ID_CERTIFICATE, NULL, NULL },
  { GBL_TAG_ID_SIGNATURE, NULL, NULL },

  { GBL_TAG_ID_MANIFEST_INFO, handleManifestSHA, handleManifestSHA },
  { GBL_TAG_ID_BUNDLE_VERSION, handleManifestSHA, handleManifestSHA },
  { GBL_TAG_ID_CONTENT_HASH, handleManifestSHA, handleManifestSHA },
  { GBL_TAG_ID_UPDATE_PROCESS, handleManifestSHA, handleManifestSHA },
  { GBL_TAG_ID_UPDATE_SE, handleManifestSHA, handleManifestSHA },
  { GBL_TAG_ID_UPDATE_MEMORY_SECTION, handleManifestSHA, handleManifestSHA },
  { GBL_TAG_ID_MANIFEST_FINISH, handleManifestSHA, handleManifestSHA },

  { GBL_TAG_ID_MEMORY_SECTION, handleMemorySectionSHA, handleMemorySectionSHA },
  { GBL_TAG_ID_PAD, handleMemorySectionSHA, handleMemorySectionSHA },
  { GBL_TAG_ID_SE_BLOB, handleMemorySectionSHA, handleMemorySectionSHA },
  { GBL_TAG_ID_MEMORY_SECTION_INFO, handleMemorySectionSHA, handleMemorySectionInfoSHA },
  { GBL_TAG_ID_MEMORY_SECTION_BLOB, handleMemorySectionSHA, handleMemorySectionInfoSHA },
  { 0, NULL, NULL }
};

// -----------------------------------------------------------------------------
static int32_t setUpProgrammingAddress(ParserContext_t *ctx);
static sl_status_t construct_initial_region_lookup_table(ParserContext_t *ctx);
static uint8_t get_region_id(uint32_t target_address, uint32_t *regionStartAddresses);
static uint8_t countNumOfUpgrades(uint16_t contents);
#if defined(BTL_PARSER_SUPPORT_CUSTOM_TAGS)
static int32_t parse_customTag(ParserContext_t *ctx, uint8_t *buffer, size_t length, const BootloaderParserCallbacks_t* callbacks);
static int32_t handleCompressionInit(ParserContext_t *ctx);
#endif
#if defined (BOOTLOADER_SUPPORT_STORAGE)
static int32_t parse_blobFromStorageBuf(ParserContext_t *ctx, InputBuffer_t *input, const BootloaderParserCallbacks_t* callbacks);
#else
static int32_t parse_blobFromStreamingBuf(ParserContext_t *ctx, InputBuffer_t *input, const BootloaderParserCallbacks_t* callbacks);
#endif

void handleManifestSHA(ParserContext_t *ctx, const uint8_t* data, size_t length)
{
  ctx->hashInterface.update(ctx->manifest.shaContext, &ctx->cmd_ctx, data, length);
}

void handleMemorySectionInfoSHA(ParserContext_t *ctx, const uint8_t* data, size_t length)
{
  ctx->hashInterface.update(ctx->shaContextMemInfo, &ctx->cmd_ctx, data, length);
}

void handleMemorySectionSHA(ParserContext_t *ctx, const uint8_t* data, size_t length)
{
  ctx->hashInterface.update(ctx->memorySectionForRegions.shaContext, &ctx->cmd_ctx, data, length);
}

#if defined(BTL_PARSER_SUPPORT_CUSTOM_TAGS)
static int32_t parse_customTag(ParserContext_t *ctx, uint8_t *buffer, size_t length, const BootloaderParserCallbacks_t* callbacks)
{
  int32_t retval;

  const GblCustomTag_t *customTag = NULL;

  if (ctx->flags & PARSER_FLAG_PARSE_CUSTOM_TAGS) {
    customTag = gbl_getCustomTagProperties(ctx->compression_scheme);
    if (!customTag) {
      return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
    }
  }

  if (ctx->flags & PARSER_FLAG_PARSE_CUSTOM_TAGS && customTag && customTag->parseTag) {
    retval = customTag->parseTag(ctx, buffer, length, callbacks);
    if (retval != BOOTLOADER_OK) {
      return retval;
    }
  }

  if (ctx->offsetInTLV == ctx->lengthOfTLV) {
    if (ctx->flags & PARSER_FLAG_PARSE_CUSTOM_TAGS && customTag && customTag->exitTag) {
      retval = customTag->exitTag(ctx, callbacks);
      if (retval != BOOTLOADER_OK) {
        return retval;
      }
    }
  }

  return BOOTLOADER_ERROR_PARSER_PARSED;
}
#endif // defined(BTL_PARSER_SUPPORT_CUSTOM_TAGS)

static int32_t setUpProgrammingAddress(ParserContext_t *ctx)
{
  uint32_t tagId = GBL_PARSER_ARRAY_TO_U32(ctx->tlv.tagHeaderBuf, 0);

#ifndef BOOTLOADER_SUPPORT_STORAGE
  if (tagId == GBL_TAG_ID_SE_BLOB) {
    void *address = NULL;
    size_t size;
    sl_se_command_context_t cmd_ctx = { 0u };
    ctx->seCmdCtxInterface.init(&cmd_ctx);
    ctx->flashDataRegionInterface.data_region_get_location(&cmd_ctx, &address, &size);
    ctx->programmingAddress = (uint32_t)address;
    uint32_t seUpgradeSize;
    size_t numOfSectors;
    if (bootload_checkSeUpgradeVersion(ctx->imageProperties->seUpgradeVersion)) {
      seUpgradeSize = ctx->tlv.tagHeader.length;
      if (seUpgradeSize % FLASH_PAGE_SIZE) {
        // Not sector aligned, move boundary to next page page
        numOfSectors = ((seUpgradeSize & ~(FLASH_PAGE_SIZE - 1)) + FLASH_PAGE_SIZE) / FLASH_PAGE_SIZE;
      } else {
        // sector aligned
        numOfSectors = seUpgradeSize / FLASH_PAGE_SIZE;
      }
      ctx->seCmdCtxInterface.init(&cmd_ctx);
      ctx->flashDataRegionInterface.data_region_erase(&cmd_ctx,
                                                      address,
                                                      numOfSectors);
    }
  }
#endif

  if (tagId != GBL_TAG_ID_SE_BLOB) {
    uint8_t regionId;
    regionId = get_region_id(ctx->currentUpdateMemorySection.updateMemorySection.targetAddr, ctx->regionStartAddresses);
    if (regionId == 0) {
#ifndef BOOTLOADER_SUPPORT_STORAGE
      void *address = NULL;
      size_t size;
      sl_se_command_context_t cmd_ctx = { 0u };
      ctx->seCmdCtxInterface.init(&cmd_ctx);
      ctx->flashDataRegionInterface.data_region_get_location(&cmd_ctx, &address, &size);
      ctx->programmingAddress = (uint32_t)address;
      ctx->plainBootloaderAddress = ctx->programmingAddress;
      ctx->seCmdCtxInterface.init(&cmd_ctx);
      size_t numOfSectors = 32768 / FLASH_PAGE_SIZE;
      ctx->flashDataRegionInterface.data_region_erase(&cmd_ctx,
                                                      address,
                                                      numOfSectors);
#else
      if (ctx->compression_scheme == COMPRESSION_LZMA   \
          || ctx->compression_scheme == COMPRESSION_LZ4 \
          || ctx->encryption_scheme == ENC_AES_CTR_128) {
        BootloaderStorageSlot_t slot;
        int32_t  retval = storage_get_slot_info(ctx->slotId, &slot);
        if (retval != BOOTLOADER_OK) {
          return retval;
        }
        volatile uint32_t intendedAddress = slot.address + ctx->receivedGblLength + 8;
        volatile uint32_t alignedAddress = (intendedAddress + 0xFFF) & ~0xFFF;
        if (slot.length - (alignedAddress - slot.address) < 32768) {
          return BOOTLOADER_ERROR_STORAGE_GENERIC;
        }
        ctx->plainBootloaderAddress = alignedAddress;
        ctx->programmingAddress = alignedAddress;
        retval = storage_erase_raw(ctx->programmingAddress, 32768);
        if (retval != BOOTLOADER_OK) {
          return retval;
        }
      }
#endif
    }
  }
  return BOOTLOADER_OK;
}

int32_t parser_idle(ParserContext_t *ctx, InputBuffer_t *input)
{
  size_t neededBytes = sizeof(TagHeader_t) - ctx->destTLVBufOffset;
  size_t bytesCopied = streamToBufferTransfer(ctx, ctx->tlv.tagHeaderBuf,
                                              sizeof(ctx->tlv.tagHeaderBuf),
                                              input, neededBytes);
  input->offset += bytesCopied;
  if (bytesCopied < neededBytes) {
    return BOOTLOADER_OK;
  }

  uint32_t tagId   = GBL_PARSER_ARRAY_TO_U32(ctx->tlv.tagHeaderBuf, 0);
  size_t tlvLength = GBL_PARSER_ARRAY_TO_U32(ctx->tlv.tagHeaderBuf, 4);

  ctx->lengthOfTLV = tlvLength;
  ctx->offsetInTLV = 0;
  ctx->destTLVBufOffset = 0;

  bool stateFound = false;
  for (int i = 0; tagParserStateTbl[i].tagId != 0; i++) {
    if (tagParserStateTbl[i].tagId == tagId) {
      ctx->internalState = tagParserStateTbl[i].parserState;
      stateFound = true;
      break;
    }
  }

  if (!stateFound) {
    ctx->internalState = ParserStateError;
    return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
  }

  if ( (ctx->gblByteCount == ctx->upgradeBlobPosition) \
       && (ctx->imageProperties->instructions != 0)    \
       && (ctx->upgradeBlobPosition != 0)              \
       && (tagId == GBL_TAG_ID_MEMORY_SECTION) ) {
    ctx->internalState = ParserStateMemorySection;
  }

  if ( (ctx->imageProperties->instructions != 0) \
       && (tagId == GBL_TAG_ID_SE_BLOB) ) {
    ctx->internalState = ParserStateSeBlob;
    int32_t  retval = setUpProgrammingAddress(ctx);
    if (retval != BOOTLOADER_OK) {
      return retval;
    }
  }

  updateSHAContextBasedOnTag(ctx, ctx->tlv.tagHeaderBuf, sizeof(ctx->tlv.tagHeaderBuf));
  ctx->gblByteCount += sizeof(ctx->tlv.tagHeaderBuf);

  return BOOTLOADER_ERROR_PARSER_PARSED;
}

#if defined(BTL_PARSER_SUPPORT_CUSTOM_TAGS)
static int32_t handleCompressionInit(ParserContext_t *ctx)
{
  if (ctx->compression_scheme == COMPRESSION_LZMA || ctx->compression_scheme == COMPRESSION_LZ4) {
    ctx->programmingAddress = ctx->currentUpdateMemorySection.updateMemorySection.targetAddr;
    if (gbl_isCustomTag(ctx->compression_scheme)) {
      const GblCustomTag_t *customTag = gbl_getCustomTagProperties(ctx->compression_scheme);
      if (!customTag) {
        return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
      }

      if ((ctx->flags & PARSER_FLAG_PARSE_CUSTOM_TAGS) && (customTag->enterTag)) {
        int32_t retval = customTag->enterTag(ctx);
        if (retval != BOOTLOADER_OK) {
          return retval;
        }
      }
    }
  }
  return BOOTLOADER_OK;
}
#endif

int32_t parse_certificate(ParserContext_t *ctx, InputBuffer_t *input)
{
  size_t neededBytes = sizeof(CertificateUnion_t) - ctx->destTLVBufOffset;

  size_t bytesCopied = streamToBufferTransfer(ctx, ctx->certificateUnion.tagCertificateBuf,
                                              sizeof(ctx->certificateUnion.tagCertificateBuf),
                                              input, neededBytes);

  input->offset += bytesCopied;
  ctx->gblByteCount += bytesCopied;
  if (bytesCopied < neededBytes) {
    return BOOTLOADER_OK;
  }

  ctx->offsetInTLV += ctx->destTLVBufOffset;
  ctx->hashInterface.update(ctx->manifestCertificate.shaContext, &ctx->cmd_ctx, ctx->certificateUnion.tagCertificateBuf, 72U);
  ctx->hashInterface.final(ctx->manifestCertificate.shaContext, &ctx->cmd_ctx);
  ctx->internalState = ParserStateIdle;
  ctx->certificateReceived = true;
  ctx->destTLVBufOffset = 0;
  return BOOTLOADER_ERROR_PARSER_PARSED;
}

int32_t process_certificate(ParserContext_t *ctx, InputBuffer_t *input)
{
  (void)input;
  int32_t certificateVerifyResp = BOOTLOADER_ERROR_SECURITY_REJECTED;

  // Access word 13 to read sl_app_properties of the bootloader.
  ApplicationProperties_t *blProperties = (ApplicationProperties_t *)(*(uint32_t *)(BTL_MAIN_STAGE_BASE + 52UL));

  if ((uint32_t)blProperties > ((uint32_t)mainBootloaderTable->startOfAppSpace - sizeof(ApplicationProperties_t))) {
    // Make sure that this pointer is within the bootloader space.
    return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
  }

  if (blProperties->cert == NULL) {
    return BOOTLOADER_ERROR_PARSER_SIGNATURE;
  }

  uint32_t certVersion = GBL_PARSER_ARRAY_TO_U32(ctx->certificateUnion.tagCertificateBuf, 68);
  if (blProperties->cert->version > certVersion) {
    return BOOTLOADER_ERROR_PARSER_SIGNATURE;
  }

  // Use the public key stored in the bootloader certificate to verify the manifest certificate.
  certificateVerifyResp = ctx->ecdsaP256r1Interface.verify(ctx->manifestCertificate.shaContext,
                                                           &ctx->certificateUnion.certificate.signature[0],
                                                           &ctx->certificateUnion.certificate.signature[32],
                                                           &(blProperties->cert->key[0]),
                                                           &(blProperties->cert->key[32]));
  if (certificateVerifyResp == BOOTLOADER_ERROR_SECURITY_REJECTED) {
    // Check if the OTP platform key can verify the certificate.
    certificateVerifyResp = ctx->ecdsaP256r1Interface.verify(ctx->manifestCertificate.shaContext,
                                                             &ctx->certificateUnion.certificate.signature[0],
                                                             &ctx->certificateUnion.certificate.signature[32],
                                                             NULL,
                                                             NULL);
  }

  if (certificateVerifyResp != BOOTLOADER_OK) {
    ctx->internalState = ParserStateError;
    return BOOTLOADER_ERROR_PARSER_SIGNATURE;
  }

  ctx->internalState = ParserStateProcessSignature;
  ctx->certificateVerified = true;
  return BOOTLOADER_ERROR_PARSER_PARSED;
}

int32_t parse_signature(ParserContext_t *ctx, InputBuffer_t *input)
{
  size_t neededBytes = ctx->lengthOfTLV - ctx->destTLVBufOffset;

  size_t bytesCopied = streamToBufferTransfer(ctx, ctx->hashSignatureUnion.tagHashSignatureBuf,
                                              sizeof(ctx->hashSignatureUnion.tagHashSignatureBuf),
                                              input, neededBytes);

  input->offset += bytesCopied;
  ctx->gblByteCount += bytesCopied;
  if (bytesCopied < neededBytes) {
    return BOOTLOADER_OK;
  }

  ctx->offsetInTLV += ctx->destTLVBufOffset;
  ctx->destTLVBufOffset = 0;

  if (PARSER_REQUIRE_AUTHENTICITY && (ctx->hashSignatureUnion.hashSignature.dataType != ECDSA_P256_SIGNATURE)) {
    ctx->internalState = ParserStateError;
    return BOOTLOADER_ERROR_PARSER_SIGNATURE;
  }

  uint8_t hashType = (uint8_t)ctx->hashSignatureUnion.hashSignature.dataType;
  if ((hashType != HASH_SHA256) && (hashType != ECDSA_P256_SIGNATURE)) {
    ctx->internalState = ParserStateError;
    return BOOTLOADER_ERROR_SECURITY_INVALID_OPTION;
  }

  ctx->internalState = ParserStateIdle;
  return BOOTLOADER_ERROR_PARSER_PARSED;
}

int32_t process_signature(ParserContext_t *ctx, InputBuffer_t *input)
{
  (void)input;

  uint8_t dataType = (uint8_t)ctx->hashSignatureUnion.hashSignature.dataType;
  if ((PARSER_REQUIRE_AUTHENTICITY) && (dataType != ECDSA_P256_SIGNATURE)) {
    return BOOTLOADER_ERROR_PARSER_SIGNATURE;
  }

  int32_t signatureVerifyResp = BOOTLOADER_ERROR_PARSER_REJECTED;

  //Fetch bootloader capabilities
  if (!bootloader_pointerValid(mainBootloaderTable)) {
    return BOOTLOADER_ERROR_INIT_TABLE;
  }
  bool isParserRequireAuth = (mainBootloaderTable->capabilities & BOOTLOADER_CAPABILITY_ENFORCE_UPGRADE_SIGNATURE);
  bool isParserRequireCertAuth = (mainBootloaderTable->capabilities & BOOTLOADER_CAPABILITY_ENFORCE_CERTIFICATE_SECURE_BOOT);

  // Access word 13 to read sl_app_properties of the bootloader.
  ApplicationProperties_t *blProperties = (ApplicationProperties_t *)(*(uint32_t *)(BTL_MAIN_STAGE_BASE + 52UL));
  if ((uint32_t)blProperties > ((uint32_t)mainBootloaderTable->startOfAppSpace - sizeof(ApplicationProperties_t))) {
    // Make sure that this pointer is within the bootloader space.
    return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
  }

  ctx->hashInterface.final(ctx->manifest.shaContext, &ctx->cmd_ctx);
  // Signature tag
  if ((dataType == HASH_SHA256) && (!isParserRequireAuth)) {
    signatureVerifyResp = ctx->hashInterface.verify(ctx->manifest.shaContext, ctx->hashSignatureUnion.hashSignature.data.sha256);
  } else {
    if (ctx->certificateVerified) {
      signatureVerifyResp = ctx->ecdsaP256r1Interface.verify(ctx->manifest.shaContext,
                                                             &ctx->hashSignatureUnion.hashSignature.data.ecdsaSignature[0],
                                                             &ctx->hashSignatureUnion.hashSignature.data.ecdsaSignature[32],
                                                             &(ctx->certificateUnion.certificate.key[0]),
                                                             &(ctx->certificateUnion.certificate.key[32]));
    } else {
      // Received direct signed GBL
      // Here we have two options.
      // 1. The bootloader is configured to use public key of its certificate.
      // 2. The bootloader is configured to use public key from the platform key.
      if (isParserRequireCertAuth) {
        // The bootloader is configured to use public key of its certificate.
        if (blProperties->cert == NULL) {
          return BOOTLOADER_ERROR_PARSER_SIGNATURE;
        }
        signatureVerifyResp = ctx->ecdsaP256r1Interface.verify(ctx->manifest.shaContext,
                                                               &ctx->hashSignatureUnion.hashSignature.data.ecdsaSignature[0],
                                                               &ctx->hashSignatureUnion.hashSignature.data.ecdsaSignature[32],
                                                               &(blProperties->cert->key[0]),
                                                               &(blProperties->cert->key[32]));
        if (signatureVerifyResp == BOOTLOADER_ERROR_SECURITY_REJECTED) {
          // Check if the OTP platform key can verify the certificate.
          // The OTP platform key has higher level of trust.
          signatureVerifyResp = ctx->ecdsaP256r1Interface.verify(ctx->manifest.shaContext,
                                                                 &ctx->hashSignatureUnion.hashSignature.data.ecdsaSignature[0],
                                                                 &ctx->hashSignatureUnion.hashSignature.data.ecdsaSignature[32],
                                                                 NULL,
                                                                 NULL);
        }
      } else if (isParserRequireAuth) {
        // Check if the OTP platform key can verify the certificate.
        // The OTP platform key has higher level of trust.
        signatureVerifyResp = ctx->ecdsaP256r1Interface.verify(ctx->manifest.shaContext,
                                                               &ctx->hashSignatureUnion.hashSignature.data.ecdsaSignature[0],
                                                               &ctx->hashSignatureUnion.hashSignature.data.ecdsaSignature[32],
                                                               NULL,
                                                               NULL);
      }
    }
  }
  if (signatureVerifyResp != BOOTLOADER_OK) {
    ctx->internalState = ParserStateError;
    return BOOTLOADER_ERROR_PARSER_SIGNATURE;
  }

  ctx->internalState = ParserStateProcessManifestInfo;
  ctx->hashInterface.init(ctx->memorySectionForRegions.shaContext, &ctx->cmd_ctx);
  return BOOTLOADER_ERROR_PARSER_PARSED;
}

int32_t parse_manifestInfo(ParserContext_t *ctx, InputBuffer_t *input)
{
  size_t neededBytes = sizeof(ManifestInfo_t) -  ctx->destTLVBufOffset;
  size_t bytesCopied = streamToBufferTransfer(ctx,
                                              ctx->manifestInfoUnion.tagManifestInfoBuf,
                                              sizeof(ctx->manifestInfoUnion.tagManifestInfoBuf),
                                              input,
                                              neededBytes);

  updateSHAContextBasedOnTag(ctx, input->buffer + input->offset, bytesCopied);
  input->offset += bytesCopied;
  ctx->gblByteCount += bytesCopied;
  if (bytesCopied < neededBytes) {
    return BOOTLOADER_OK;
  }

  ctx->offsetInTLV += ctx->destTLVBufOffset;
  ctx->internalState = ParserStateIdle;
  ctx->destTLVBufOffset = 0;
  return BOOTLOADER_ERROR_PARSER_PARSED;
}

int32_t process_manifestInfo(ParserContext_t *ctx, InputBuffer_t *input)
{
  (void)input;
#ifndef BTL_PARSER_NO_SUPPORT_ENCRYPTION
  if ((ctx->manifestInfoUnion.manifestInfo.features & FEATURE_ENCRYPTION_AESCCM_ON) != 0U) {
    ctx->flags |= PARSER_FLAG_ENCRYPTED;
  } else if (mainBootloaderTable->capabilities & BOOTLOADER_CAPABILITY_ENFORCE_UPGRADE_ENCRYPTION) {
    ctx->internalState = ParserStateError;
    return BOOTLOADER_ERROR_PARSER_FILETYPE;
  } else {
    // Unencrypted GBL is allowed
  }
#else
  // No encryption supported, but encryption feature is requested
  if ((ctx->manifestInfoUnion.manifestInfo.features & FEATURE_ENCRYPTION_AESCCM_ON) != 0U) {
    ctx->internalState = ParserStateError;
    return BOOTLOADER_ERROR_PARSER_FILETYPE;
  }
#endif

  ctx->internalState = ParserStateProcessBundleVersion;
  return BOOTLOADER_ERROR_PARSER_PARSED;
}

int32_t parse_bundleVersion(ParserContext_t *ctx, InputBuffer_t *input)
{
  size_t neededBytes = sizeof(BundleVersion_t) - ctx->destTLVBufOffset;

  size_t bytesCopied = streamToBufferTransfer(ctx,
                                              ctx->bundleVersionUnion.tagBundleVersionBuf,
                                              sizeof(ctx->bundleVersionUnion.tagBundleVersionBuf),
                                              input,
                                              neededBytes);

  updateSHAContextBasedOnTag(ctx, input->buffer + input->offset, bytesCopied);
  input->offset += bytesCopied;
  ctx->gblByteCount += bytesCopied;
  if (bytesCopied < neededBytes) {
    return BOOTLOADER_OK;
  }

  ctx->offsetInTLV += ctx->destTLVBufOffset;
  ctx->internalState = ParserStateIdle;
  ctx->destTLVBufOffset = 0;
  return BOOTLOADER_ERROR_PARSER_PARSED;
}

int32_t process_bundleVersion(ParserContext_t *ctx, InputBuffer_t *input)
{
  (void)input;
  memcpy(&ctx->imageProperties->application.productId, &ctx->bundleVersionUnion.bundleVersion.productId, 16);
  uint32_t currBundleVersion;
  sl_status_t status;
  bool flag = false;
  sl_se_command_context_t cmd_ctx = { 0 };
  ctx->seCmdCtxInterface.init(&cmd_ctx);
  status = ctx->flashUpgradeFileInterface.get_upgrade_file_version(&cmd_ctx, &currBundleVersion);

  if (status == BOOTLOADER_OK) {
    flag = (currBundleVersion >=  ctx->bundleVersionUnion.bundleVersion.minVersion) \
           && (currBundleVersion <= ctx->bundleVersionUnion.bundleVersion.bundleVersion);
  } else if (status == SL_STATUS_NOT_INITIALIZED) {
    flag = true;
  }

  if (flag) {
    ctx->internalState = ParserStateIdle;
    return BOOTLOADER_ERROR_PARSER_PARSED;
  } else {
    ctx->internalState = ParserStateError;
    return BOOTLOADER_ERROR_PARSER_VERSION;
  }
}

int32_t parse_contentHash(ParserContext_t *ctx, InputBuffer_t *input)
{
  size_t neededBytes = ctx->lengthOfTLV - ctx->destTLVBufOffset;

  size_t bytesCopied = streamToBufferTransfer(ctx,
                                              ctx->contentHashUnion.tagContentHashBuf,
                                              sizeof(ctx->contentHashUnion.tagContentHashBuf),
                                              input,
                                              neededBytes);

  updateSHAContextBasedOnTag(ctx, input->buffer + input->offset, bytesCopied);
  input->offset += bytesCopied;
  ctx->gblByteCount += bytesCopied;
  if (bytesCopied < neededBytes) {
    return BOOTLOADER_OK;
  }

  uint8_t hashType = (uint8_t)ctx->contentHashUnion.contentHash.hashType;
  if (hashType != HASHTYPE_SHA256) {
    ctx->internalState = ParserStateError;
    return BOOTLOADER_ERROR_SECURITY_INVALID_OPTION;
  }

  ctx->offsetInTLV += ctx->destTLVBufOffset;
  ctx->internalState = ParserStateIdle;
  ctx->destTLVBufOffset = 0;
  return BOOTLOADER_ERROR_PARSER_PARSED;
}

int32_t parse_updateSe(ParserContext_t *ctx, InputBuffer_t *input)
{
  size_t neededBytes = sizeof(UpdateSe_t) - ctx->destTLVBufOffset;

  size_t bytesCopied = streamToBufferTransfer(ctx,
                                              ctx->updateSeUnion.tagUpdateSeBuf,
                                              sizeof(ctx->updateSeUnion.tagUpdateSeBuf),
                                              input,
                                              neededBytes);

  updateSHAContextBasedOnTag(ctx, input->buffer + input->offset, bytesCopied);
  input->offset += bytesCopied;
  ctx->gblByteCount += bytesCopied;
  if (bytesCopied < neededBytes) {
    return BOOTLOADER_OK;
  }

  ctx->offsetInTLV += ctx->destTLVBufOffset;
  ctx->imageProperties->contents |= BTL_IMAGE_CONTENT_SE;
  ctx->imageProperties->seUpgradeVersion = ctx->updateSeUnion.updateSe.version;
  ctx->internalState = ParserStateIdle;
  ctx->destTLVBufOffset = 0;
  return BOOTLOADER_ERROR_PARSER_PARSED;
}

static uint8_t countNumOfUpgrades(uint16_t contents)
{
  uint8_t count = 0;
  while (contents) {
    count += contents & 0x01U;
    contents >>= 1;
  }
  return count;
}

int32_t parse_manifest(ParserContext_t *ctx, InputBuffer_t *input)
{
  (void)(input);
  // It's just a container
  // Note: Perform any necessary processing for the Manifest state.
  // After processing, transition the internal state to Idle.
  ctx->internalState = ParserStateIdle;
  ctx->destTLVBufOffset = 0;
  return BOOTLOADER_ERROR_PARSER_PARSED;
}

int32_t parse_updateProcess(ParserContext_t *ctx, InputBuffer_t *input)
{
  (void)input;
  // It's just a container TLV
  // Note: Perform any necessary processing for the  Update Process state.
  // After processing, transition the internal state to Idle.
  construct_initial_region_lookup_table(ctx);

  ctx->internalState = ParserStateIdle;
  ctx->destTLVBufOffset = 0;
  return BOOTLOADER_ERROR_PARSER_PARSED;
}

int32_t parse_v4(ParserContext_t *ctx, InputBuffer_t *input)
{
  (void)input;
  //Note: Perform any necessary processing for the v4 state.
  uint32_t temporaryWord;
  temporaryWord =  GBL_PARSER_ARRAY_TO_U32(ctx->tlv.tagHeaderBuf, 0);
  if ((temporaryWord & 0xFF000000UL) != GBL_COMPATIBILITY_MAJOR_VERSION) {
    ctx->internalState = ParserStateError;
    return BOOTLOADER_ERROR_PARSER_VERSION;
  }
  // After processing, transition the internal state to Idle.
  ctx->internalState = ParserStateIdle;
  ctx->destTLVBufOffset = 0;
  ctx->receivedGblLength = GBL_PARSER_ARRAY_TO_U32(ctx->tlv.tagHeaderBuf, 4);
  return BOOTLOADER_ERROR_PARSER_PARSED;
}

int32_t parse_manifestStateDone(ParserContext_t *ctx, InputBuffer_t *input)
{
  (void)input;
  if (ctx->certificateReceived) {
    ctx->internalState = ParserStateProcessCertificate;
  } else {
    ctx->internalState = ParserStateProcessSignature;
  }
  ctx->destTLVBufOffset = 0;
  return BOOTLOADER_ERROR_PARSER_PARSED;
}

int32_t parse_memorySection(ParserContext_t *ctx, InputBuffer_t *input)
{
  volatile uint8_t regionId;
  (void)input;

  if (ctx->imageProperties->instructions != 0) {
    sl_status_t status;
    sl_se_command_context_t cmd_ctx;
    ctx->seCmdCtxInterface.init(&cmd_ctx);

    regionId = get_region_id(ctx->currentUpdateMemorySection.updateMemorySection.targetAddr, ctx->regionStartAddresses);
    if (regionId != 0) {
      uint64_t instruction_mask = 0;
      if (regionId < 8) {
        instruction_mask = (1u << (regionId + 1));
      }

      if (ctx->imageProperties->instructions & instruction_mask) {
        // Disable LFRCO.
        CMU->CLKEN0_SET = CMU_CLKEN0_LFRCO;
        LFRCO->CTRL_SET = LFRCO_CTRL_DISONDEMAND;
        LFRCO->CTRL_CLR = LFRCO_CTRL_FORCEEN;
        while ((LFRCO->STATUS & LFRCO_STATUS_ENS) != 0U) {
        }

        // Disable High Precision Mode to prevent hangs happening
        // in the SE firmware when sending commands to the QSPI.
        LFRCO->CFG_CLR = LFRCO_CFG_HIGHPRECEN;

        // Clear disable on-demand.
        LFRCO->CTRL_CLR = LFRCO_CTRL_DISONDEMAND;
        CMU->CLKEN0_CLR = CMU_CLKEN0_LFRCO;

        uint32_t runningSEVersion = 0xFFFFFFFFUL;
        if (!bootload_getSeVersion(&runningSEVersion)) {
          return BOOTLOADER_ERROR_PARSER_UNEXPECTED; // Could not retrieve SE version
        }
        // Erase the region
        if (runningSEVersion < 0x030200UL) {
          status = ctx->flashWriteInterface.code_region_erase(&cmd_ctx, regionId);
        } else {
          uint32_t numBlocks = (ctx->appPlainImgSize + FLASH_ERASE_BLOCK_SIZE - 1) / FLASH_ERASE_BLOCK_SIZE;
          status = ctx->flashWriteInterface.code_region_partial_erase(&cmd_ctx, regionId, 0, numBlocks);
        }
        if (status != BOOTLOADER_OK) {
          ctx->internalState = ParserStateError;
          return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
        }
#ifdef BOOTLOADER_SUPPORT_STORAGE
        //Increase CPU clock frequency before proceeding with the upgrades
        if (BOOTLOADER_ENFORCE_FAST_UPGRADES) {
          //Faster upgrades are enabled. Configure CPU to run at higher frequencies
          sl_clock_manager_init();
          BTL_DEBUG_INIT();
        }
#endif
      }
      ctx->programmingAddress = 0;
    }
  }
#ifndef BOOTLOADER_SUPPORT_STORAGE
  ctx->hashInterface.init(ctx->shaContextMemInfo, &ctx->cmd_ctx);
#endif
  ctx->internalState = ParserStateIdle;
  ctx->destTLVBufOffset = 0;
  return BOOTLOADER_ERROR_PARSER_PARSED;
}

int32_t parse_memorySectionInfo(ParserContext_t *ctx, InputBuffer_t *input)
{
  MemSectionInfo_t *currentInstance = &ctx->memorySectionInfo;
  size_t neededBytes = sizeof(MemSectionPartialInfo_t) - ctx->destTLVBufOffset;

  size_t bytesCopied = streamToBufferTransfer(ctx,
                                              currentInstance->memorySection.tagMemSectionInfoBuf,
                                              sizeof(currentInstance->memorySection.tagMemSectionInfoBuf),
                                              input,
                                              neededBytes);

  updateSHAContextBasedOnTag(ctx, input->buffer + input->offset, bytesCopied);
  input->offset += bytesCopied;
  ctx->gblByteCount += bytesCopied;

  if (bytesCopied < neededBytes) {
    return BOOTLOADER_OK;
  }
  // 8(ID + LEN for MemorySection) + 8 (ID + LEN for MemorySectionInfo) + lengthOfTLV(length of memorySectionInfo) + 8(ID + LEN for blob)
  ctx->memSectionMetaDataLength = 8 + 8 + ctx->lengthOfTLV + 8;
  ctx->offsetInTLV += ctx->destTLVBufOffset;
  ctx->encryption_scheme = currentInstance->memorySection.memSectionInfo.encryptionScheme;
#ifndef BTL_PARSER_NO_SUPPORT_ENCRYPTION
  if (ctx->encryption_scheme == ENC_AES_CTR_128) {
    ctx->flags |= PARSER_FLAG_ENCRYPTED;
    ctx->aesInterface.init(ctx->aesContext,
                           0x02U,
                           currentInstance->memorySection.memSectionInfo.nonce,
                           1UL,
                           NULL,
                           128UL);
  } else if (mainBootloaderTable->capabilities & BOOTLOADER_CAPABILITY_ENFORCE_UPGRADE_ENCRYPTION) {
    ctx->internalState = ParserStateError;
    return BOOTLOADER_ERROR_PARSER_FILETYPE;
  }
#else
  if (ctx->encryption_scheme == ENC_AES_CTR_128) {
    ctx->internalState = ParserStateError;
    return BOOTLOADER_ERROR_PARSER_FILETYPE;
  }
#endif

#if defined(BTL_PARSER_SUPPORT_CUSTOM_TAGS)
  ctx->compression_scheme = currentInstance->memorySection.memSectionInfo.compressionScheme;
  handleCompressionInit(ctx);
#endif // defined(BTL_PARSER_SUPPORT_CUSTOM_TAGS)
  int32_t  retval = setUpProgrammingAddress(ctx);
  if (retval != BOOTLOADER_OK) {
    return retval;
  }

  currentInstance->totalHashBytes = currentInstance->memorySection.memSectionInfo.numBlocks * 32;

  if (currentInstance->totalHashBytes != 0) {
    currentInstance->ListOfBlockOfHashes = (uint8_t *)malloc(currentInstance->totalHashBytes);
    if (currentInstance->ListOfBlockOfHashes == NULL) {
      ctx->internalState = ParserStateError;
      return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
    }
    memset(currentInstance->ListOfBlockOfHashes, 0, currentInstance->totalHashBytes);
    ctx->internalState = ParserStateReceiveBlockOfHashes;
  } else {
#ifndef BOOTLOADER_SUPPORT_STORAGE
    ctx->hashInterface.final(ctx->shaContextMemInfo, &ctx->cmd_ctx);
    const UpdateMemorySectionUnion_t *updateMemSecInstance = &ctx->currentUpdateMemorySection;
    int32_t retVal;
    retVal =  ctx->hashInterface.verify(ctx->shaContextMemInfo, updateMemSecInstance->updateMemorySection.memSecHash.data.sha);
    if (retVal != BOOTLOADER_OK) {
      ctx->internalState = ParserStateError;
      return BOOTLOADER_ERROR_PARSER_HASH;
    }
#endif
    ctx->internalState = ParserStateIdle;
  }
  ctx->destTLVBufOffset = 0;
  return BOOTLOADER_ERROR_PARSER_PARSED;
}

int32_t parse_rcvBlockOfHashes(ParserContext_t *ctx, InputBuffer_t *input)
{
  MemSectionInfo_t *currentInstance = &ctx->memorySectionInfo;
  size_t neededBytes = currentInstance->totalHashBytes - ctx->destTLVBufOffset;

  size_t bytesCopied = streamToBufferTransfer(ctx,
                                              currentInstance->ListOfBlockOfHashes,
                                              currentInstance->totalHashBytes,
                                              input,
                                              neededBytes);

  updateSHAContextBasedOnTag(ctx, input->buffer + input->offset, bytesCopied);
  input->offset += bytesCopied;
  ctx->gblByteCount += bytesCopied;

  if (bytesCopied < neededBytes) {
    return BOOTLOADER_OK;
  }

  ctx->offsetInTLV += ctx->destTLVBufOffset;
#ifndef BOOTLOADER_SUPPORT_STORAGE
  int32_t retVal;
  if (ctx->destTLVBufOffset == currentInstance->totalHashBytes) {
    ctx->hashInterface.final(ctx->shaContextMemInfo, &ctx->cmd_ctx);
    const UpdateMemorySectionUnion_t *updateMemSecInstance = &ctx->currentUpdateMemorySection;
    retVal =  ctx->hashInterface.verify(ctx->shaContextMemInfo, updateMemSecInstance->updateMemorySection.memSecHash.data.sha);
    if (retVal != BOOTLOADER_OK) {
      ctx->internalState = ParserStateError;
      return BOOTLOADER_ERROR_PARSER_HASH;
    }
    ctx->internalState = ParserStateIdle;
    ctx->destTLVBufOffset = 0;
  }
#else
  ctx->internalState = ParserStateIdle;
  ctx->destTLVBufOffset = 0;
#endif

  return BOOTLOADER_ERROR_PARSER_PARSED;
}

int32_t parse_ignorePaddingData(ParserContext_t *ctx, InputBuffer_t *input)
{
  int32_t retVal = BOOTLOADER_OK;
  size_t neededBytes = ctx->lengthOfTLV - ctx->destTLVBufOffset;

  size_t bytesCopied = streamToBufferTransfer(ctx,
                                              ctx->padBuffer,
                                              sizeof(ctx->padBuffer),
                                              input,
                                              neededBytes);

  updateSHAContextBasedOnTag(ctx, input->buffer + input->offset, bytesCopied);
  input->offset += bytesCopied;
  ctx->gblByteCount += bytesCopied;

  if (bytesCopied < neededBytes) {
    return retVal;
  }

  ctx->offsetInTLV += ctx->destTLVBufOffset;
  ctx->internalState = ParserStateIdle;
  ctx->destTLVBufOffset = 0;

  retVal = BOOTLOADER_ERROR_PARSER_PARSED;

  if ( (ctx->gblByteCount == ctx->receivedGblLength + 8) \
       && (ctx->receivedGblLength != 0) ) {
    retVal = parser_finalize(ctx, input);
  }
  return retVal;
}

#if defined (BOOTLOADER_SUPPORT_STORAGE)
static int32_t parse_blobFromStorageBuf(ParserContext_t *ctx, InputBuffer_t *input, const BootloaderParserCallbacks_t* callbacks)
{
  size_t bytesRemainingInTag = ctx->lengthOfTLV - ctx->offsetInTLV;
  size_t neededBytes = SL_MIN((size_t)BLOB_BUFFER_SIZE, bytesRemainingInTag) - ctx->destTLVBufOffset;
  size_t bytesCopied = streamToBufferTransfer(ctx, blobBuffer, sizeof(blobBuffer), input, neededBytes);

  updateSHAContextBasedOnTag(ctx, input->buffer + input->offset, bytesCopied);

  input->offset += bytesCopied;
  ctx->gblByteCount += bytesCopied;

  if (bytesCopied < neededBytes) {
    return BOOTLOADER_OK;
  }

  ctx->offsetInTLV += ctx->destTLVBufOffset;
  ctx->regionId = get_region_id(ctx->currentUpdateMemorySection.updateMemorySection.targetAddr, ctx->regionStartAddresses);

  uint64_t instruction_mask = 0U;
  if (ctx->regionId < 8) {
    instruction_mask = (1u << (ctx->regionId + 1));
  } else if (ctx->regionId == 0xFF) {
    return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
  }

  if (ctx->imageProperties->instructions & instruction_mask) {
    if (ctx->encryption_scheme == ENC_AES_CTR_128) {
      ctx->aesInterface.process(ctx->aesContext, blobBuffer, blobBuffer, ctx->destTLVBufOffset);
    }
#if defined(BTL_PARSER_SUPPORT_CUSTOM_TAGS)
    /* Provided Compressed Image and Compression component installed */
    if (ctx->compression_scheme == COMPRESSION_LZMA || ctx->compression_scheme == COMPRESSION_LZ4) {
      parse_customTag(ctx, blobBuffer, ctx->destTLVBufOffset, callbacks);
    } else
#endif
    {
      if (ctx->regionId == 0) {
        if ((callbacks->bootloaderCallback != NULL) && (ctx->encryption_scheme != NO_ENCRYPTION)) {
          callbacks->bootloaderCallback(ctx->programmingAddress, blobBuffer, ctx->destTLVBufOffset, ctx->regionId, ctx);
          if (ctx->retCode != BOOTLOADER_OK) {
            return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
          }
        }
      } else {
        if (callbacks->applicationCallback != NULL) {
          callbacks->applicationCallback(ctx->programmingAddress, blobBuffer, ctx->destTLVBufOffset, ctx->regionId, ctx);
          if (ctx->retCode != BOOTLOADER_OK) {
            return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
          }
        }
      }
      ctx->programmingAddress += ctx->destTLVBufOffset;
    }
  }

  ctx->destTLVBufOffset = 0;
  return BOOTLOADER_ERROR_PARSER_PARSED;
}
#else
static int32_t parse_blobFromStreamingBuf(ParserContext_t *ctx, InputBuffer_t *input, const BootloaderParserCallbacks_t* callbacks)
{
  int32_t retVal = 0;

  MemSectionInfo_t *currentInstance = &ctx->memorySectionInfo;
  uint16_t signBlockSize;

  if (currentInstance->memorySection.memSectionInfo.numBlocks != 0) {
    signBlockSize = currentInstance->memorySection.memSectionInfo.signBlockSize;
  } else {
    signBlockSize = FLASH_WRITE_BUFFER_SIZE;
  }

  if (ctx->segmentBuffer == NULL) {
    ctx->segmentBuffer = (uint8_t *)malloc(signBlockSize);
    if (ctx->segmentBuffer == NULL) {
      ctx->internalState = ParserStateError;
      return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
    }
    if (currentInstance->memorySection.memSectionInfo.numBlocks != 0) {
      ctx->hashInterface.init(ctx->shaContextMemInfo, &ctx->cmd_ctx);
    }
  }

  size_t bytesRemainingInTag = ctx->lengthOfTLV - ctx->offsetInTLV;
  uint32_t neededBytes       = SL_MIN(signBlockSize, bytesRemainingInTag);
  neededBytes                = neededBytes - ctx->destTLVBufOffset;

  size_t bytesCopied = streamToBufferTransfer(ctx,
                                              ctx->segmentBuffer,
                                              signBlockSize,
                                              input,
                                              neededBytes);

  if (currentInstance->memorySection.memSectionInfo.numBlocks != 0) {
    updateSHAContextBasedOnTag(ctx, input->buffer + input->offset, bytesCopied);
  }

  input->offset += bytesCopied;
  ctx->gblByteCount += bytesCopied;

  // Buffer the data until the entire segment is received
  if (bytesCopied < neededBytes) {
    return BOOTLOADER_OK;
  }

  ctx->offsetInTLV += ctx->destTLVBufOffset;
  ctx->regionId =  get_region_id(ctx->currentUpdateMemorySection.updateMemorySection.targetAddr, ctx->regionStartAddresses);

  if (ctx->regionId == 0xFF) {
    return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
  }

  if (currentInstance->memorySection.memSectionInfo.numBlocks != 0) {
    ctx->hashInterface.final(ctx->shaContextMemInfo, &ctx->cmd_ctx);

    if (ctx->blockCount > currentInstance->memorySection.memSectionInfo.numBlocks) {
      return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
    }

    unsigned char blockHash[32];
    uint32_t index = 0;
    index = 32 * ctx->blockCount;
    for (int i = 0; i < 32; i++) {
      blockHash[i] = currentInstance->ListOfBlockOfHashes[index + i];
    }
    retVal =  ctx->hashInterface.verify(ctx->shaContextMemInfo, (void*)blockHash);

    if (retVal != BOOTLOADER_OK) {
      ctx->internalState = ParserStateError;
      // free the segment buffer incase any error
      free(ctx->segmentBuffer);
      ctx->segmentBuffer = NULL;
      ctx->imageProperties->imageVerified = false;
      return BOOTLOADER_ERROR_PARSER_HASH;
    }

    ctx->blockCount++;
  }

  if (ctx->encryption_scheme == ENC_AES_CTR_128) {
    ctx->aesInterface.process(ctx->aesContext,
                              ctx->segmentBuffer,
                              ctx->segmentBuffer,
                              ctx->destTLVBufOffset);
  }

#if defined(BTL_PARSER_SUPPORT_CUSTOM_TAGS)
  /* Provided Compressed Image and Compression component installed*/
  if (ctx->compression_scheme == COMPRESSION_LZMA || ctx->compression_scheme == COMPRESSION_LZ4) {
    parse_customTag(ctx, ctx->segmentBuffer, ctx->destTLVBufOffset, callbacks);
  } else
#endif
  {
    if (ctx->regionId == 0) {
      if (callbacks->bootloaderCallback != NULL) {
        if (currentInstance->memorySection.memSectionInfo.numBlocks != 0) {
          size_t totalBytes = ctx->destTLVBufOffset;
          size_t offset = 0;
          while (offset < totalBytes) {
            size_t chunkSize = (totalBytes - offset >= FLASH_WRITE_BUFFER_SIZE) ? FLASH_WRITE_BUFFER_SIZE : (totalBytes - offset);
            callbacks->bootloaderCallback(ctx->programmingAddress + offset,
                                          ctx->segmentBuffer + offset,
                                          chunkSize,
                                          0,
                                          ctx);
            if (ctx->retCode != BOOTLOADER_OK) {
              return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
            }
            offset += chunkSize;
          }
        } else {
          callbacks->bootloaderCallback(ctx->programmingAddress,
                                        ctx->segmentBuffer,
                                        ctx->destTLVBufOffset,
                                        0,
                                        ctx);
          if (ctx->retCode != BOOTLOADER_OK) {
            return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
          }
        }
      }
    } else {
      if (callbacks->applicationCallback != NULL) {
        if (currentInstance->memorySection.memSectionInfo.numBlocks != 0) {
          size_t totalBytes = ctx->destTLVBufOffset;
          size_t offset = 0;
          while (offset < totalBytes) {
            size_t chunkSize = (totalBytes - offset >= FLASH_WRITE_BUFFER_SIZE) ? FLASH_WRITE_BUFFER_SIZE : (totalBytes - offset);
            callbacks->applicationCallback(ctx->programmingAddress + offset,
                                           ctx->segmentBuffer + offset,
                                           chunkSize,
                                           ctx->regionId,
                                           ctx);
            if (ctx->retCode != BOOTLOADER_OK) {
              return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
            }
            offset += chunkSize;
          }
        } else {
          callbacks->applicationCallback(ctx->programmingAddress,
                                         ctx->segmentBuffer,
                                         ctx->destTLVBufOffset,
                                         ctx->regionId,
                                         ctx);
          if (ctx->retCode != BOOTLOADER_OK) {
            return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
          }
        }
      }
    }
    ctx->programmingAddress += ctx->destTLVBufOffset;
  }

  // free the segment buffer after segment processing is completed
  free(ctx->segmentBuffer);
  ctx->segmentBuffer = NULL;

  // Reset for next segment
  ctx->destTLVBufOffset = 0;

  if (ctx->offsetInTLV == ctx->lengthOfTLV) {
    ctx->imageProperties->imageVerified = true;
    ctx->blockCount = 0;
    if (currentInstance->ListOfBlockOfHashes != NULL) {
      free(currentInstance->ListOfBlockOfHashes);
      currentInstance->ListOfBlockOfHashes = NULL;
    }
  }

  return BOOTLOADER_ERROR_PARSER_PARSED;
}
#endif

int32_t parse_memorySectionBlob(ParserContext_t *ctx, InputBuffer_t *input, const BootloaderParserCallbacks_t* callbacks)
{
  int32_t retVal = BOOTLOADER_OK;

  while (input->offset < input->length && ctx->offsetInTLV < ctx->lengthOfTLV) {
#ifdef BOOTLOADER_SUPPORT_STORAGE
    retVal = parse_blobFromStorageBuf(ctx, input, callbacks);
#else
    retVal = parse_blobFromStreamingBuf(ctx, input, callbacks);
#endif
  }

  if (retVal != BOOTLOADER_ERROR_PARSER_PARSED) {
    return retVal;
  }

  if (ctx->offsetInTLV == ctx->lengthOfTLV) {
    ctx->internalState = ParserStateIdle;
    ctx->offsetInTLV = 0;
  }

  if ( (ctx->gblByteCount == ctx->receivedGblLength + 8) \
       && (ctx->receivedGblLength != 0) ) {
    retVal = parser_finalize(ctx, input);
  }
  return retVal;
}

void updateSHAContextBasedOnTag(ParserContext_t *ctx, const uint8_t* data, size_t length)
{
  uint32_t tagId = GBL_PARSER_ARRAY_TO_U32(ctx->tlv.tagHeaderBuf, 0);
  for (int i = 0; tagShaHandlerTbl[i].tagId != 0; i++) {
    if (tagShaHandlerTbl[i].tagId == tagId) {
#ifdef BOOTLOADER_SUPPORT_STORAGE
      if (tagShaHandlerTbl[i].batchUpgradeHandler != NULL) {
        tagShaHandlerTbl[i].batchUpgradeHandler(ctx, data, length);
        return;
      }
#else
      if (tagShaHandlerTbl[i].streamingUpgradeHandler != NULL) {
        tagShaHandlerTbl[i].streamingUpgradeHandler(ctx, data, length);
        return;
      }
#endif
    }
  }
  return;
}

int32_t parser_init(void *context,
                    void *decryptContext,
                    void *primaryAuthContext,
                    void *secondaryAuthContext,
                    uint8_t flags)
{
  ParserContext_t* ctx = (ParserContext_t*)context;
  ctx->hashInterface = sha256Interface;
  ctx->flashWriteInterface = flashWrite;
  ctx->flashCodeRegionInterface = flashCodeRegion;
  ctx->flashDataRegionInterface = flashDataRegion;
  ctx->flashUpgradeFileInterface = flashUpgradeFile;
  ctx->aesInterface = AESInterface;
  ctx->ecdsaP256r1Interface = EcdsaP256r1;
  ctx->seCmdCtxInterface = seCmdCtx;
  ctx->seCmdCtxInterface.init(&ctx->cmd_ctx);
  ctx->internalState = ParserStateIdle;
  ctx->lengthOfTLV = 0;
  ctx->offsetInTLV = 0;
  ctx->destTLVBufOffset = 0;
  ctx->programmingAddress = 0;
  ctx->certificateReceived = false;
  ctx->certificateVerified = false;
  ctx->blockCount = 0;
  ctx->gblByteCount = 0;
  ctx->receivedGblLength = 0;
  ctx->upgradeBlobPosition = 0;
  ctx->updateMemorySectionCount = 0;
  ctx->dataSectionUpgradeCount = 0;
  ctx->memSectionMetaDataLength = 0;
  ctx->slotId = 0;
  ctx->plainBootloaderAddress = 0;
  ctx->flags = flags & PARSER_FLAGS_PUBLIC_MASK;
  ctx->retCode = BOOTLOADER_ERROR_SECURITY_REJECTED;
#if defined (BTL_PARSER_SUPPORT_CUSTOM_TAGS)
  ctx->compression_scheme = NO_COMPRESSION;
#endif
  ctx->encryption_scheme = NO_ENCRYPTION;
  ctx->segmentBuffer = NULL;
  ctx->imageProperties = NULL;
  ctx->manifest.shaContext = primaryAuthContext;
  ctx->manifestCertificate.shaContext = secondaryAuthContext;
  ctx->memorySectionForRegions.shaContext = primaryAuthContext;
  ctx->shaContextMemInfo = secondaryAuthContext;
  ctx->aesContext = (DecryptContext_t*)decryptContext;
  ctx->hashInterface.init(ctx->manifest.shaContext, &ctx->cmd_ctx);
  ctx->hashInterface.init(ctx->manifestCertificate.shaContext, &ctx->cmd_ctx);

  memset(ctx->withheldApplicationVectors, 0xFFU, 64U);
  return BOOTLOADER_OK;
}

int32_t parser_parse(void                              *context,
                     ImageProperties_t                 *imageProperties,
                     uint8_t                           buffer[],
                     size_t                            length,
                     const BootloaderParserCallbacks_t *callbacks)
{
  int32_t retVal;
  ParserContext_t *ctx = (ParserContext_t*)context;
  ctx->imageProperties = imageProperties;

  InputBuffer_t input = {
    .buffer = buffer,
    .length = length,
    .offset = 0UL
  };

  while (input.offset < input.length) {
    TLVHandler standardHandler = NULL;
    TLVHandlerWithCallback callbackHandler = NULL;
    bool useCallback = false;

    for (int i = 0; paserStateHandlerMap[i].handler.standard != NULL; i++) {
      if (paserStateHandlerMap[i].internalState == ctx->internalState) {
        if (paserStateHandlerMap[i].useCallback) {
          callbackHandler = paserStateHandlerMap[i].handler.withCallback;
        } else {
          standardHandler = paserStateHandlerMap[i].handler.standard;
        }
        useCallback = paserStateHandlerMap[i].useCallback;
        break;
      }
    }

    if (standardHandler || callbackHandler) {
      if (useCallback) {
        retVal = callbackHandler(ctx, &input, callbacks);
      } else {
        retVal = standardHandler(ctx, &input);
      }
      if (retVal != BOOTLOADER_ERROR_PARSER_PARSED) {
        return retVal;
      }
    } else {
      ctx->internalState = ParserStateError;
      return BOOTLOADER_ERROR_PARSER_UNKNOWN_TAG;
    }
  }
  return BOOTLOADER_OK;
}

int32_t streamToBufferTransfer(ParserContext_t *ctx, uint8_t *dest, size_t destSize, InputBuffer_t *input, size_t numBytes)
{
  size_t bytesCopied = 0;

  while (bytesCopied < numBytes && input->offset + bytesCopied < input->length
         && ctx->destTLVBufOffset + bytesCopied < destSize) {
    dest[ctx->destTLVBufOffset + bytesCopied] = input->buffer[input->offset + bytesCopied];
    bytesCopied++;
  }

  ctx->destTLVBufOffset += bytesCopied;

  return bytesCopied;
}

int32_t parse_seBlob(ParserContext_t* ctx, InputBuffer_t *input, const BootloaderParserCallbacks_t* callbacks)
{
  while (input->offset < input->length && ctx->offsetInTLV < ctx->lengthOfTLV) {
    size_t bytesRemainingInTag = ctx->lengthOfTLV - ctx->offsetInTLV;
    size_t neededBytes = SL_MIN((size_t)BLOB_BUFFER_SIZE, bytesRemainingInTag) - ctx->destTLVBufOffset;
    size_t bytesCopied = streamToBufferTransfer(ctx, blobBuffer, sizeof(blobBuffer), input, neededBytes);

    updateSHAContextBasedOnTag(ctx, input->buffer + input->offset, bytesCopied);
    input->offset += bytesCopied;
    ctx->gblByteCount += bytesCopied;

    if (bytesCopied < neededBytes) {
      return BOOTLOADER_OK;
    }
    if ((ctx->imageProperties->instructions & BTL_IMAGE_INSTRUCTION_SE) && (callbacks->bootloaderCallback != NULL)) {
      callbacks->bootloaderCallback(ctx->programmingAddress, blobBuffer, ctx->destTLVBufOffset, 0, ctx);
    }

    ctx->offsetInTLV += ctx->destTLVBufOffset;
    ctx->programmingAddress += ctx->destTLVBufOffset;
    ctx->destTLVBufOffset = 0;
  }
  if (ctx->offsetInTLV == ctx->lengthOfTLV) {
    ctx->internalState = ParserStateIdle;
    ctx->offsetInTLV = 0;
  }
  if ( (ctx->gblByteCount == ctx->receivedGblLength + 8) \
       && (ctx->receivedGblLength != 0) ) {
    return parser_finalize(ctx, input);
  }
  return BOOTLOADER_ERROR_PARSER_PARSED;
}

int32_t parser_finalize(ParserContext_t *ctx, InputBuffer_t *input)
{
  (void)(input);
  int32_t retVal;
  uint8_t regionIndex = 0;
  bool isCloseRegion = false;
  bool isIntegrityCheck = false;

  ctx->imageProperties->imageCompleted = true;

  if (!bootloader_pointerValid(mainBootloaderTable)) {
    return BOOTLOADER_ERROR_INIT_TABLE;
  }

  if (mainBootloaderTable->capabilities & BOOTLOADER_CAPABILITY_STORAGE) {
    isIntegrityCheck = true;
  }

  if ((isIntegrityCheck) || (ctx->imageProperties->contents == BTL_IMAGE_CONTENT_SE)) {
    ctx->hashInterface.final(ctx->memorySectionForRegions.shaContext, &ctx->cmd_ctx);
    retVal = ctx->hashInterface.verify(ctx->memorySectionForRegions.shaContext,
                                       ctx->contentHashUnion.contentHash.data.sha256);

    if (retVal == BOOTLOADER_OK) {
      ctx->imageProperties->imageVerified = true;
    } else {
      ctx->imageProperties->imageVerified = false;
      ctx->internalState = ParserStateError;
      return BOOTLOADER_ERROR_PARSER_HASH;
    }
  }

  if (ctx->imageProperties->imageVerified) {
    if (mainBootloaderTable->capabilities & BOOTLOADER_CAPABILITY_STORAGE) {
      for (regionIndex = 2; regionIndex <= 8; regionIndex++) {
        if (ctx->imageProperties->instructions & (1 << regionIndex)) {
          isCloseRegion = true;
          break;
        }
      }
    } else {
      for (regionIndex = 2; regionIndex <= 8; regionIndex++) {
        if (ctx->imageProperties->contents & (1 << regionIndex)) {
          isCloseRegion = true;
          break;
        }
      }
    }

    if (isCloseRegion) {
      sl_status_t status;
      sl_se_command_context_t cmd_ctx = { 0u };
      ctx->seCmdCtxInterface.init(&cmd_ctx);
      UpdateMemorySectionUnion_t *currentInstance = &ctx->currentUpdateMemorySection;
#if !defined(LOCKBIT_SKIP_BOOT_CHECK)
      uint32_t version = currentInstance->updateMemorySection.version;
#endif
      uint32_t regionId = get_region_id(currentInstance->updateMemorySection.targetAddr, ctx->regionStartAddresses);
      status = ctx->flashWriteInterface.code_region_write(&cmd_ctx,
                                                          regionId,
                                                          0,
                                                          ctx->withheldApplicationVectors,
                                                          WITHHELD_VECTORS_SIZE,
                                                          NULL,
                                                          NULL);
      if (status != BOOTLOADER_OK) {
        return parser_error(ctx, input);
      }
#if !defined(LOCKBIT_SKIP_BOOT_CHECK)
      status = ctx->flashWriteInterface.code_region_close(&cmd_ctx, regionId, version);
      if (status != BOOTLOADER_OK) {
        return parser_error(ctx, input);
      }
#endif
    }
    if (!(mainBootloaderTable->capabilities & BOOTLOADER_CAPABILITY_STORAGE)) {
      sl_se_command_context_t cmd_ctx = { 0 };
      ctx->seCmdCtxInterface.init(&cmd_ctx);
      retVal = ctx->flashUpgradeFileInterface.set_upgrade_file_version(&cmd_ctx,
                                                                       ctx->bundleVersionUnion.bundleVersion.bundleVersion);
      if (retVal != BOOTLOADER_OK) {
        return BOOTLOADER_ERROR_PARSER_VERSION;
      }
    }
  }
  ctx->internalState = ParserStateDone;
  return BOOTLOADER_OK;
}

int32_t parser_stateDone(ParserContext_t *ctx, InputBuffer_t *input)
{
  (void)input;
  (void)ctx;
  return BOOTLOADER_ERROR_PARSER_EOF;
}

int32_t parser_error(ParserContext_t *ctx, InputBuffer_t *input)
{
  (void)input;
#ifndef BOOTLOADER_SUPPORT_STORAGE

  MemSectionInfo_t *currentInstance = &ctx->memorySectionInfo;
  if (currentInstance->ListOfBlockOfHashes != NULL) {
    free(currentInstance->ListOfBlockOfHashes);
    currentInstance->ListOfBlockOfHashes = NULL;
  }
  if (ctx->segmentBuffer != NULL) {
    free(ctx->segmentBuffer);
    ctx->segmentBuffer = NULL;
  }
#endif
  ctx->imageProperties->imageVerified = false;
  return BOOTLOADER_ERROR_PARSER_EOF;
}

int32_t parse_completeMemorySection(ParserContext_t *ctx, InputBuffer_t *input)
{
  int32_t retVal = BOOTLOADER_OK;

  while (input->offset < input->length && ctx->offsetInTLV < ctx->lengthOfTLV) {
    size_t dataAvailable = input->length - input->offset;
    size_t dataNeeded = ctx->lengthOfTLV - ctx->offsetInTLV;
    size_t bytesToProcess = (dataAvailable < dataNeeded) ? dataAvailable : dataNeeded;

    handleMemorySectionSHA(ctx, input->buffer + input->offset, bytesToProcess);

    ctx->offsetInTLV += bytesToProcess;
    input->offset += bytesToProcess;
    ctx->gblByteCount += bytesToProcess;
  }

  if (ctx->offsetInTLV == ctx->lengthOfTLV) {
    ctx->internalState = ParserStateIdle;
    retVal = BOOTLOADER_ERROR_PARSER_PARSED;
  }

  if ( (ctx->gblByteCount == ctx->receivedGblLength + 8) \
       && (ctx->receivedGblLength != 0) ) {
    retVal = parser_finalize(ctx, input);
  }

  return retVal;
}

int32_t parse_completeSEblob(ParserContext_t *ctx, InputBuffer_t *input)
{
  int32_t retVal = BOOTLOADER_OK;

  while (input->offset < input->length && ctx->offsetInTLV < ctx->lengthOfTLV) {
    size_t dataAvailable = input->length - input->offset;
    size_t dataNeeded = ctx->lengthOfTLV - ctx->offsetInTLV;
    size_t bytesToProcess = (dataAvailable < dataNeeded) ? dataAvailable : dataNeeded;

    updateSHAContextBasedOnTag(ctx, input->buffer + input->offset, bytesToProcess);

    ctx->offsetInTLV += bytesToProcess;
    input->offset += bytesToProcess;
    ctx->gblByteCount += bytesToProcess;
  }

  if (ctx->offsetInTLV == ctx->lengthOfTLV) {
    ctx->internalState = ParserStateIdle;
    retVal = BOOTLOADER_ERROR_PARSER_PARSED;
  }

  if ( (ctx->gblByteCount == ctx->receivedGblLength + 8) \
       && (ctx->receivedGblLength != 0) ) {
    retVal = parser_finalize(ctx, input);
  }

  return retVal;
}

static sl_status_t construct_initial_region_lookup_table(ParserContext_t *ctx)
{
  sl_status_t status;
  sl_se_command_context_t cmd_ctx = { 0u };
  ctx->seCmdCtxInterface.init(&cmd_ctx);

  sl_se_code_region_config_t region_configs[NUMBER_OF_MEMORY_SECTIONS];

  status = ctx->flashCodeRegionInterface.code_region_get_config(&cmd_ctx, region_configs, 0, NUMBER_OF_MEMORY_SECTIONS);

  if (status != BOOTLOADER_OK) {
    return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
  }

  uint32_t current_address = FLASH_BASE;
  for (uint8_t i = 0; i < NUMBER_OF_MEMORY_SECTIONS; i++) {
    ctx->regionStartAddresses[i] = current_address;
    current_address += region_configs[i].region_size;
  }
  return BOOTLOADER_OK;
}

static uint8_t get_region_id(uint32_t target_address, uint32_t *regionStartAddresses)
{
  target_address = bootloader_GetAliasedAddr(target_address);
  for (uint8_t i = 0; i < NUMBER_OF_MEMORY_SECTIONS; i++) {
    if (target_address == regionStartAddresses[i]) {
      return i;
    }
  }
  return 0xFF;
}

int32_t parse_updateMemorySection(ParserContext_t *ctx, InputBuffer_t *input)
{
  uint8_t regionId;

  if (ctx->updateMemorySectionCount > 12) {
    return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
  }

  UpdateMemorySectionUnion_t *currentInstance = &ctx->updateMemorySectionUnion[ctx->updateMemorySectionCount];
  size_t neededBytes = ctx->lengthOfTLV - ctx->destTLVBufOffset;
  size_t bytesCopied = streamToBufferTransfer(ctx,
                                              currentInstance->tagUpdateMemSectionBuf,
                                              sizeof(currentInstance->tagUpdateMemSectionBuf),
                                              input,
                                              neededBytes);

  updateSHAContextBasedOnTag(ctx, input->buffer + input->offset, bytesCopied);
  input->offset += bytesCopied;
  ctx->gblByteCount += bytesCopied;

  if (bytesCopied < neededBytes) {
    return BOOTLOADER_OK;
  }

  uint8_t hashType = (uint8_t)currentInstance->updateMemorySection.memSecHash.hashType;
  if (hashType != HASHTYPE_SHA256) {
    ctx->internalState = ParserStateError;
    return BOOTLOADER_ERROR_SECURITY_INVALID_OPTION;
  }

  ctx->offsetInTLV += ctx->destTLVBufOffset;
  if (!bootloader_pointerValid(mainBootloaderTable)) {
    return BOOTLOADER_ERROR_INIT_TABLE;
  }
  regionId = get_region_id(currentInstance->updateMemorySection.targetAddr, ctx->regionStartAddresses);
  if (regionId == 0) {
    ctx->imageProperties->bootloaderVersion = currentInstance->updateMemorySection.version;
    ctx->imageProperties->bootloaderUpgradeSize = 0;
    ctx->imageProperties->bootloaderUpgradeSize |= currentInstance->updateMemorySection.plainImageSize[0];
    ctx->imageProperties->bootloaderUpgradeSize |= currentInstance->updateMemorySection.plainImageSize[1] << 8;
    ctx->imageProperties->bootloaderUpgradeSize |= currentInstance->updateMemorySection.plainImageSize[2] << 16;
  } else if (regionId == 1) {
    ctx->imageProperties->application.type = currentInstance->updateMemorySection.type;
    ctx->imageProperties->application.version = currentInstance->updateMemorySection.version;
    ctx->imageProperties->application.capabilities = currentInstance->updateMemorySection.capabilities;
    ctx->appPlainImgSize = 0;
    ctx->appPlainImgSize |= currentInstance->updateMemorySection.plainImageSize[0];
    ctx->appPlainImgSize |= currentInstance->updateMemorySection.plainImageSize[1] << 8;
    ctx->appPlainImgSize |= currentInstance->updateMemorySection.plainImageSize[2] << 16;
    if (mainBootloaderTable->capabilities & BOOTLOADER_CAPABILITY_ROLLBACK_PROTECTION) {
      if ((ctx->imageProperties->application.type == 0)
          || (bootload_verifyApplicationVersion(ctx->imageProperties->application.version) == false)) {
        ctx->internalState = ParserStateError;
        return BOOTLOADER_ERROR_PARSER_REJECTED;
      }
    }
  }

  uint64_t instruction_mask = 0;
  if (regionId < 8) {
    ctx->imageProperties->contents |= (1u << (regionId + 1));
    instruction_mask = (1u << (regionId + 1));
  } else if (regionId == 0xFF) {
    ctx->imageProperties->contents |= (1u << (40 + ctx->dataSectionUpgradeCount));
    instruction_mask = (1u << (40 + ctx->dataSectionUpgradeCount));
    ctx->dataSectionUpgradeCount++;
  }

  if (!(mainBootloaderTable->capabilities & BOOTLOADER_CAPABILITY_STORAGE)) {
    if (countNumOfUpgrades(ctx->imageProperties->contents) > 1) {
      ctx->internalState = ParserStateError;
      return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
    }
  }

  if (ctx->imageProperties->instructions & instruction_mask) {
    ctx->upgradeBlobPosition  = currentInstance->updateMemorySection.memorySectionPos;
    memcpy(&ctx->currentUpdateMemorySection, currentInstance, sizeof(UpdateMemorySectionUnion_t));
  }

  ctx->internalState = ParserStateIdle;
  ctx->destTLVBufOffset = 0;
  ctx->updateMemorySectionCount++;
  return BOOTLOADER_ERROR_PARSER_PARSED;
}
