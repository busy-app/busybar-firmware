/***************************************************************************//**
 * @file
 * @brief Definitions for the Silicon Labs GBL format
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
#ifndef BTL_GBL_FORMAT_H
#define BTL_GBL_FORMAT_H

#include <stdint.h>
#include "api/application_properties.h"
#include "security/btl_security_sha256.h"

/***************************************************************************//**
 * @addtogroup ComponentsS3
 * @{
 * @addtogroup ImageParserS3 Image Parser
 * @{
 * @addtogroup GblParserS3 GBL Parser
 * @{
 * @addtogroup GblParserFormatS3 GBL Format
 * @{
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Defines

// -------------------------------
// GBL version

/// Magic word indicating GBL image
#define GBL_IMAGE_MAGIC_WORD                0xE45050E4UL
/// Major version of the GBL spec
#define GBL_COMPATIBILITY_MAJOR_VERSION     0x84000000UL

// -------------------------------
// Tag IDs

/// Tag ID for the GBL header tag
#define GBL_TAG_ID_V4                       0x84A617EBUL
/// Tag ID for the manifest tag
#define GBL_TAG_ID_MANIFEST                 0xAA01012AUL
/// Tag ID for the GBL ECDSA certfificate tag
#define GBL_TAG_ID_CERTIFICATE              0x2B01012BUL
/// Tag ID for the GBL ECDSA secp256r1 signature tag
#define GBL_TAG_ID_SIGNATURE                0x2B02022BUL
/// Tag ID for the manifest info tag
#define GBL_TAG_ID_MANIFEST_INFO            0x2B03032BUL
/// Tag ID for the bundle version tag
#define GBL_TAG_ID_BUNDLE_VERSION           0x2B04042BUL
/// Tag ID for the content hash tag
#define GBL_TAG_ID_CONTENT_HASH             0x2B05052BUL
/// Tag ID for the update process tag
#define GBL_TAG_ID_UPDATE_PROCESS           0xAB06062BUL
/// Tag ID for the SE Update tag
#define GBL_TAG_ID_UPDATE_SE                0x2C02022CUL
/// Tag ID for the Memory Section Update tag
#define GBL_TAG_ID_UPDATE_MEMORY_SECTION    0x2C03032CUL

#define GBL_TAG_ID_MANIFEST_FINISH          0x2C04042CUL

/// Tag ID for the GBL SE Blob
#define GBL_TAG_ID_SE_BLOB                  0x3A02023AUL
#define GBL_TAG_ID_MEMORY_SECTION           0xBA01013AUL
#define GBL_TAG_ID_MEMORY_SECTION_INFO      0x3B01013BUL
#define GBL_TAG_ID_MEMORY_SECTION_BLOB      0x3B02023BUL
#define GBL_TAG_ID_PAD                      0xEE0101EEUL
/// GBL type: Standard GBL
#define GBL_TYPE_NONE                         0x00UL
/// GBL type: Compression
#define FEATURE_COMPRESSION_ON                0x01UL
/// GBL type: AES-CCM encrypted GBL
#define FEATURE_ENCRYPTION_AESCCM_ON          0x02UL
/// GBL type: ECDSA P256-signed GBL
#define FEATURE_SIGNATURE_ECDSA_ON            0x04UL
// -----------------------------------------------------------------------------
// Structs
#define NUMBER_OF_MEMORY_SECTIONS 8U

/// GBL tag header. Must be the first element in all GBL tags.

#pragma pack(push, 1)
typedef struct {
  uint32_t tagId;               ///< Tag ID
  uint32_t length;              ///< Length (in bytes) of the rest of the tag
}TagHeader_t;
#pragma pack(pop)

typedef struct {
  TagHeader_t header;          ///< Tag ID and length
  uint8_t *data;              ///< Container and Root TLV
}v4_t;

typedef struct {
  void *shaContext;
}Manifest_t;

typedef struct {
  void *shaContext;
}ManifestCertificate_t;

typedef struct {
  void *shaContext;
}MemorySectionForRegions_t;

#pragma pack(push, 1)

typedef struct {
  uint32_t dataType;
  union {
    uint8_t sha256[32];
    uint8_t sha512[64];
    uint8_t ecdsaSignature[64];
  } data;
} HashSignature_t;

typedef struct {
  uint8_t structVersion;
  uint8_t flags[3];
  uint8_t key[64];
  uint32_t version;
  uint8_t signature[64];
} Certificate_t;

typedef union {
  HashSignature_t hashSignature;
  uint8_t tagHashSignatureBuf[68];
}HashSignatureUnion_t; // Data TLV, parent is Manifest_t

typedef struct {
  uint32_t signFunction;
  HashSignatureUnion_t signature;
}Signature_t;

typedef struct {
  uint32_t version;
  uint32_t features;
}ManifestInfo_t;

typedef struct {
  uint8_t productId[16];
  uint32_t bundleVersion;
  uint32_t minVersion;
}BundleVersion_t;

typedef struct {
  uint32_t hashType;
  union {
    uint8_t sha256[32];
    uint8_t sha512[64];
    uint8_t sha[64];
  } data;
} HashValue_t;

typedef struct {
  uint32_t version;
  uint32_t tlvPosition;
}UpdateSe_t;

typedef struct {
  uint8_t targetMemory;   ///< (0=SRAM,1=OSPI1, 2=OSPI2, custom, etc
  uint8_t plainImageSize[3];
  uint32_t targetAddr;
  uint32_t type;
  uint32_t version;
  uint32_t capabilities;
  uint32_t memorySectionPos;
  HashValue_t  memSecHash;
}UpdateMemorySection_t;

typedef struct {
  uint8_t compressionScheme;
  uint8_t encryptionScheme;
  uint8_t secureBootScheme;
  uint8_t reserved;
  uint16_t signBlockSize;
  uint16_t numBlocks;
  uint8_t nonce[12];                 ///< Nonce for encryption
  uint8_t finalImageHash[64];
  uint8_t secureBootSign[128];
}MemSectionPartialInfo_t;
#pragma pack(pop)

typedef union {
  TagHeader_t tagHeader;
  uint8_t tagHeaderBuf[8];
}TagHeaderUnion_t;

typedef union {
  Certificate_t certificate;
  uint8_t tagCertificateBuf[136];
}CertificateUnion_t; // Data TLV, parent is Manifest_t

typedef union {
  ManifestInfo_t manifestInfo;
  uint8_t tagManifestInfoBuf[8];
}ManifestInfoUnion_t; // Data TLV, parent is Manifest_t

typedef union {
  BundleVersion_t bundleVersion;
  uint8_t tagBundleVersionBuf[24];
}BundleVersionUnion_t; // Data TLV, parent is Manifest_t

typedef union {
  HashValue_t contentHash;
  uint8_t tagContentHashBuf[68];
} ContentHashUnion_t; // Data TLV, parent is Manifest_t

typedef union {
  UpdateSe_t updateSe;
  uint8_t tagUpdateSeBuf[8];
}UpdateSeUnion_t; // Data TLV, parent is UpdateProcess_t

typedef union {
  UpdateMemorySection_t updateMemorySection;
  uint8_t tagUpdateMemSectionBuf[92];
}UpdateMemorySectionUnion_t;  // Data TLV, parent is UpdateProcess_t

typedef union {
  MemSectionPartialInfo_t memSectionInfo;
  uint8_t tagMemSectionInfoBuf[212];
} MemSectionInfoUnion_t; //partial MemSectionInfo

typedef struct {
  MemSectionInfoUnion_t memorySection;
  uint8_t *ListOfBlockOfHashes;
  uint32_t totalHashBytes;
} MemSectionInfo_t;

typedef struct {
  uint8_t *data;
}MemorySectionBlob_t;

/** @} addtogroup GblParserFormat */
/** @} addtogroup GblParser */
/** @} addtogroup ImageParser */
/** @} addtogroup Components */

#endif // BTL_GBL_FORMAT_H
