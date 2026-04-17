/***************************************************************************//**
 * @file
 * @brief GBL image file parser.
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
#ifndef BTL_GBL_PARSER_H
#define BTL_GBL_PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "security/btl_security_aes.h"
#include "security/btl_security_ecdsa.h"
#include "security/btl_security_sha256.h"
#include "security/btl_security_types.h"
#include "parser/gbl/btl_gbl_format.h"
#include "config/btl_config.h"

#include "api/btl_interface.h"
#include "api/btl_interface_parser.h"
#include "api/application_properties.h"

/***************************************************************************//**
 * @addtogroup ComponentsS3
 * @{
 * @addtogroup ImageParserS3 Image Parser
 * @brief The image parser parses the data and returns bootloader upgrade data in a callback.
 * @{
 * @addtogroup GblParserS3 GBL Parser
 * @{
 * @brief GBL parser implementation.
 * @details
 *   Image parser for GBL files. Parses GBL files based on the
 *   [GBL file format specification](@ref GblParserFormat). Callbacks are used
 *   to present data and metadata contents of the GBL file to the bootloader.
 ******************************************************************************/
#define PRODUCT_VERSION 1U          // TODO: Used to identify which product this file is intended

/// GBL file is encrypted
#define PARSER_FLAG_ENCRYPTED               (1U << 0U)
/// Parse custom tags rather than silently traversing them
#define PARSER_FLAG_PARSE_CUSTOM_TAGS       (1U << 5U)

/// Some flags are public, some are internal to the parser
#define PARSER_FLAGS_PUBLIC_MASK            PARSER_FLAG_PARSE_CUSTOM_TAGS

/// Upgrade image contains following upgrades
#define BTL_IMAGE_CONTENT_SE        (1u << 0)
#define BTL_IMAGE_CONTENT_MEM_SEC_1 (1u << 1)
#define BTL_IMAGE_CONTENT_MEM_SEC_2 (1u << 2)
#define BTL_IMAGE_CONTENT_MEM_SEC_3 (1u << 3)
#define BTL_IMAGE_CONTENT_MEM_SEC_4 (1u << 4)
#define BTL_IMAGE_CONTENT_MEM_SEC_5 (1u << 5)
#define BTL_IMAGE_CONTENT_MEM_SEC_6 (1u << 6)
#define BTL_IMAGE_CONTENT_MEM_SEC_7 (1u << 7)
#define BTL_IMAGE_CONTENT_MEM_SEC_8 (1u << 8)

#define BTL_IMAGE_INSTRUCTION_SE        (1u << 0)
#define BTL_IMAGE_INSTRUCTION_MEM_SEC_1 (1u << 1)
#define BTL_IMAGE_INSTRUCTION_MEM_SEC_2 (1u << 2)
#define BTL_IMAGE_INSTRUCTION_MEM_SEC_3 (1u << 3)
#define BTL_IMAGE_INSTRUCTION_MEM_SEC_4 (1u << 4)
#define BTL_IMAGE_INSTRUCTION_MEM_SEC_5 (1u << 5)
#define BTL_IMAGE_INSTRUCTION_MEM_SEC_6 (1u << 6)
#define BTL_IMAGE_INSTRUCTION_MEM_SEC_7 (1u << 7)
#define BTL_IMAGE_INSTRUCTION_MEM_SEC_8 (1u << 8)

/***************************************************************************//**
 * Bootloader/parser configurations
 ******************************************************************************/
/// Parser requires upgrade images to be signed, providing authenticity, if true.
#if defined(BOOTLOADER_ENFORCE_SIGNED_UPGRADE) && (BOOTLOADER_ENFORCE_SIGNED_UPGRADE == 1)
#define PARSER_REQUIRE_AUTHENTICITY (true)
#else
#define PARSER_REQUIRE_AUTHENTICITY (false)
#endif

/// Parser requires upgrade images to be encrypted, providing confidentiality, if true.
#if defined(BOOTLOADER_ENFORCE_ENCRYPTED_UPGRADE) && (BOOTLOADER_ENFORCE_ENCRYPTED_UPGRADE == 1)
#define PARSER_REQUIRE_CONFIDENTIALITY (true)
#else
#define PARSER_REQUIRE_CONFIDENTIALITY (false)
#endif

/// Parser requires upgrade images to be authenticated by the bootloader certificate, if true.
#ifndef PARSER_REQUIRE_CERTIFICATE_AUTHENTICITY
#if defined(BOOTLOADER_SUPPORT_CERTIFICATES) && (BOOTLOADER_SUPPORT_CERTIFICATES == 1)
#define PARSER_REQUIRE_CERTIFICATE_AUTHENTICITY (true)
#else
#define PARSER_REQUIRE_CERTIFICATE_AUTHENTICITY (false)
#endif
#endif

/// Parser requires rollback protection of applications, if true.
#if defined(BOOTLOADER_ROLLBACK_PROTECTION) && (BOOTLOADER_ROLLBACK_PROTECTION == 1)
#define PARSER_REQUIRE_ANTI_ROLLBACK_PROTECTION (true)
#else
#define PARSER_REQUIRE_ANTI_ROLLBACK_PROTECTION (false)
#endif

/// Defines the minimum application version that can be accepted.
#if defined(BOOTLOADER_ROLLBACK_PROTECTION_MINIMUM_VERSION) && (BOOTLOADER_ROLLBACK_PROTECTION_MINIMUM_VERSION != 0)
#define PARSER_APPLICATION_MINIMUM_VERSION_VALID  BOOTLOADER_ROLLBACK_PROTECTION_MINIMUM_VERSION
#else
#define PARSER_APPLICATION_MINIMUM_VERSION_VALID (0UL)
#endif

// Size of the withheld application vectors
#define WITHHELD_VECTORS_SIZE 64
#define PAD_SIZE 32                // Padding size for alignment
// -----------------------------------------------------------------------------
typedef struct {
  void (*init)(void *ctx, void *cmd_ctx);
  void (*update)(void *ctx, void *cmd_ctx, const void *data, size_t len);
  int32_t (*verify)(void *ctx, const void *sha256);
  void (*final)(void *ctx, void *cmd_ctx);
} HashInterface_t;

typedef struct {
  sl_status_t (*code_region_get_config)(sl_se_command_context_t *cmd_ctx,
                                        sl_se_code_region_config_t *regions_array,
                                        unsigned int start_region_idx,
                                        unsigned int region_array_size);

  sl_status_t (*code_region_apply_config)(sl_se_command_context_t *cmd_ctx,
                                          sl_se_code_region_config_t *regions_array,
                                          unsigned int start_region_idx,
                                          unsigned int region_array_size);

  sl_status_t (*code_region_get_version)(sl_se_command_context_t *cmd_ctx,
                                         unsigned int region_idx,
                                         uint32_t *version);
}FlashCodeRegionInterface_t;

typedef struct {
  sl_status_t (*code_region_partial_erase)(sl_se_command_context_t *cmd_ctx,
                                           unsigned int region_idx,
                                           uint32_t block_offset,
                                           uint32_t num_blocks);

  sl_status_t (*code_region_erase)(sl_se_command_context_t *cmd_ctx,
                                   unsigned int region_idx);

  sl_status_t (*code_region_write)(sl_se_command_context_t *cmd_ctx,
                                   unsigned int region_idx,
                                   uint32_t offset,
                                   const void *data,
                                   uint32_t num_bytes,
                                   sl_se_crypto_operation_t *decryption_info,
                                   sl_se_crypto_operation_t *integrity_check);

  sl_status_t (*code_region_close)(sl_se_command_context_t *cmd_ctx,
                                   unsigned int region_idx,
                                   uint32_t version);
}FlashWriteInterface_t;

typedef struct {
  sl_status_t (*data_region_get_location)(sl_se_command_context_t *cmd_ctx,
                                          void **address,
                                          size_t  *size);

  sl_status_t (*data_region_write)(sl_se_command_context_t *cmd_ctx,
                                   void *address,
                                   const void *data,
                                   size_t num_bytes);
  sl_status_t (*data_region_erase)(sl_se_command_context_t *cmd_ctx,
                                   void *start_address,
                                   size_t num_sectors);
}FlashDataRegionInterface_t;

typedef struct {
  sl_status_t (*get_upgrade_file_version)(sl_se_command_context_t *cmd_ctx,
                                          uint32_t *version);
  sl_status_t (*set_upgrade_file_version)(sl_se_command_context_t *cmd_ctx,
                                          uint32_t version);
}FlashUpgradeFileInterface_t;

typedef struct {
  void (*init)(void          *ctx,
               uint8_t       flags,
               uint8_t       *nonce,
               uint32_t      counter,
               const uint8_t *key,
               unsigned int  keySize);
  void (*process)(void          *ctx,
                  const uint8_t *input,
                  uint8_t       *output,
                  size_t        length);
}AESInterface_t;

typedef struct {
  int32_t (*verify)(const uint8_t *sha256,
                    const uint8_t *signatureR,
                    const uint8_t *signatureS,
                    const uint8_t *keyX,
                    const uint8_t *keyY);
}EcdsaP256r1Interface_t;

typedef struct {
  sl_status_t (*se_init)(void);
  sl_status_t (*init)(sl_se_command_context_t *cmd_ctx);
  sl_status_t (*deinit)(void);
}SECmdCtxInterface_t;

/// GBL parser input buffer
typedef struct {
  /// Pointer to a buffer
  const uint8_t *buffer;
  /// Length of the buffer
  const size_t  length;
  /// Offset of the buffer
  size_t        offset;
} InputBuffer_t;

typedef enum {
  ParserStateNone = -1,
  ParserStateIdle,
  ParserStatev4,
  ParserStateManifest,
  ParserStateUpdateProcess,
  ParserStateCertificate,
  ParserStateProcessCertificate,
  ParserStateSignature,
  ParserStateManifestInfo,
  ParserStateBundleVersion,
  ParserStateContentHash,
  ParserStateConfigureRegions,
  ParserStateUpdateSe,
  ParserStateUpdateMemorySection,
  ParserStateManifestDone,
  ParserStateProcessSignature,
  ParserStateProcessManifestInfo,
  ParserStateProcessBundleVersion,
  ParserStateCalHashForMemSection,
  ParserStateCompleteMemorySection,
  ParserStateSeBlob,
  ParserStateCompleteSeBlob,
  ParserStateProcessSeBlob,
  ParserStateMemorySection,
  ParserStateMemorySectionInfo,
  ParserStateMemorySectionBlob,
  ParserStateProcessBlobUpgrade,
  ParserStateError,
  ParserStateReceiveBlockOfHashes,
  ParserStateIgnorePaddingData,
  ParserStateDone
} ParserState_t;

typedef enum {
  HASH_SHA256,
  HASH_SHA512,
  ECDSA_P256_SIGNATURE
}signFuntion_t;

typedef enum {
  HASHTYPE_NONE,
  HASHTYPE_SHA256,
  HASHTYPE_SHA512
}hashFunction_t;

typedef enum {
  NO_ENCRYPTION,
  ENC_AES_CTR_128
}encryptionScheme_t;

typedef enum {
  NO_COMPRESSION,
  COMPRESSION_LZ4,
  COMPRESSION_LZMA
}compressionScheme_t;

/// Structure containing state of the image file processed
typedef struct {
  /// Image contents
  uint64_t                     contents;
  /// Parser instructions
  uint64_t                     instructions;
  /// Flag to indicate parsing has completed
  bool                        imageCompleted;
  /// Flag to indicate the image file has been validated
  bool                        imageVerified;
  /// Version number of main bootloader extracted from image file
  uint32_t                    bootloaderVersion;
  /// Information about the application
  ApplicationData_t           application;
  /// Size of the bootloader upgrade contained in the image file
  uint32_t                    bootloaderUpgradeSize;
  /// Version number of SE upgrade extracted from image file
  uint32_t                    seUpgradeVersion;
} ImageProperties_t;

typedef struct {
  TagHeaderUnion_t tlv;
  v4_t v4;
  Manifest_t manifest;
  ManifestCertificate_t manifestCertificate;
  CertificateUnion_t certificateUnion;
  HashSignatureUnion_t hashSignatureUnion;
  ManifestInfoUnion_t manifestInfoUnion;
  BundleVersionUnion_t bundleVersionUnion;
  ContentHashUnion_t contentHashUnion;
  UpdateSeUnion_t updateSeUnion;
  UpdateMemorySectionUnion_t updateMemorySectionUnion[12];
  UpdateMemorySectionUnion_t currentUpdateMemorySection;
  MemSectionInfo_t memorySectionInfo;
  MemorySectionForRegions_t memorySectionForRegions;

  HashInterface_t hashInterface;
  FlashWriteInterface_t flashWriteInterface;
  FlashCodeRegionInterface_t flashCodeRegionInterface;
  FlashDataRegionInterface_t flashDataRegionInterface;
  FlashUpgradeFileInterface_t flashUpgradeFileInterface;
  AESInterface_t aesInterface;
  EcdsaP256r1Interface_t ecdsaP256r1Interface;
  SECmdCtxInterface_t seCmdCtxInterface;
  sl_se_command_context_t cmd_ctx;
  ParserState_t internalState;
  uint32_t lengthOfTLV;
  uint32_t offsetInTLV;
  uint32_t destTLVBufOffset;
  uint32_t programmingAddress;
  uint32_t blockCount;
  uint8_t      flags;
  bool certificateVerified;
  bool certificateReceived;
  DecryptContext_t  *aesContext;
  uint8_t *segmentBuffer;
  ImageProperties_t *imageProperties;
  uint8_t encryption_scheme;
  uint8_t compression_scheme;
  void *shaContextMemInfo;
  uint32_t gblByteCount;
  uint32_t receivedGblLength;
  uint32_t upgradeBlobPosition;
  uint32_t regionStartAddresses[NUMBER_OF_MEMORY_SECTIONS];
  uint8_t updateMemorySectionCount;
  uint8_t dataSectionUpgradeCount;
  uint8_t regionId;
  uint8_t withheldApplicationVectors[WITHHELD_VECTORS_SIZE];
  uint32_t plainBootloaderAddress;
  uint32_t slotId;
  uint32_t memSectionMetaDataLength;
  uint32_t retCode;
  uint32_t appPlainImgSize;
  uint8_t padBuffer[3];
} ParserContext_t;

typedef void (*SHAContextHandler)(ParserContext_t *ctx, const uint8_t* data, size_t length);
typedef int32_t (*TLVHandler)(ParserContext_t*, InputBuffer_t *);
typedef int32_t (*TLVHandlerWithCallback)(ParserContext_t*, InputBuffer_t*, const BootloaderParserCallbacks_t*);

typedef struct {
  uint32_t tagId;
  SHAContextHandler batchUpgradeHandler;
  SHAContextHandler streamingUpgradeHandler;
}TagShaHandlerMap_t;

typedef struct {
  uint32_t tagId;
  ParserState_t parserState;
} GblTagStateMap_t;

typedef struct {
  ParserState_t internalState;
  union {
    TLVHandler standard;
    TLVHandlerWithCallback withCallback;
  } handler;
  bool useCallback;
} ParserStateHandlerMap_t;

/***************************************************************************//**
 * Initialize the parser's context.
 *
 * @param context         Pointer to context for the parser implementation
 * @param decryptContext  Pointer to context for decryption of parsed file
 * @param primaryAuthContext  Pointer to context for authentication of parsed file
 * @param secondaryAuthContext  Pointer to context for authentication of parsed file
 * @param flags           Flags for parser support
 *
 * @return @ref BOOTLOADER_OK if OK, error code otherwise.
 ******************************************************************************/
int32_t parser_init(void *context,
                    void *decryptContext,
                    void *primaryAuthContext,
                    void *secondaryAuthContext,
                    uint8_t flags);
/***************************************************************************//**
 * Parse an image file to extract the binary and some metadata.
 *
 * Pushes data into the image file parser to be parsed.
 *
 * @param context Pointer to the specific parser's context variable
 * @param imageProperties Pointer to the image file state variable
 * @param buffer Pointer to byte array containing data to parse
 * @param length Size in bytes of the data in buffer
 * @param callbacks Struct containing function pointers to be called by the
 *   parser to pass the extracted binary data back to BTL.
 *
 * @return @ref BOOTLOADER_OK if OK, error code otherwise.
 ******************************************************************************/
int32_t parser_parse(void                              *context,
                     ImageProperties_t                 *imageProperties,
                     uint8_t                           buffer[],
                     size_t                            length,
                     const BootloaderParserCallbacks_t *callbacks);
#endif
/** @} addtogroup GblParserS3 */
/** @} addtogroup ImageParserS3 */
/** @} addtogroup ComponentsS3 */
