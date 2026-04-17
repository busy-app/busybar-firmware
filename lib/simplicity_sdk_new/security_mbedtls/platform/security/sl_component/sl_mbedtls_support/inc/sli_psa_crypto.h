/***************************************************************************//**
 * @file
 * @brief Silicon Labs internal PSA Crypto utility functions.
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_PSA_CRYPTO_H
#define SLI_PSA_CRYPTO_H

/// @cond DO_NOT_INCLUDE_WITH_DOXYGEN

#include "psa/crypto.h"

#include <stddef.h>
#include <stdbool.h>

// -----------------------------------------------------------------------------
// Pragmas

#if SL_MBEDTLS_PSA_ASSUME_EXCLUSIVE_BUFFERS == 1
#pragma message("SL_MBEDTLS_PSA_ASSUME_EXCLUSIVE_BUFFERS=1: This improves performance by avoiding extra buffer copies, reducing memory usage and allocation overhead. However, it is NOT the most secure option and should only be enabled if all buffers passed to PSA functions are exclusively accessible to PSA and never shared with untrusted code. If buffers may cross trust boundaries, set this to 0 for best security, as disabling this option ensures input and output buffers are protected by making local copies at the cost of increased memory usage and reduced performance.")
#endif

// -----------------------------------------------------------------------------
// Defines and Macros

// Persistent key ID ranges.
// Silicon Labs SDK 'Middleware' allocation
#define SLI_PSA_KEY_ID_RANGE_PLATFORM_START (0x00004200)
#define SLI_PSA_KEY_ID_RANGE_PLATFORM_END   (0x000042FF)
#define SLI_PSA_KEY_ID_RANGE_MATTER_START   (0x00004400)
#define SLI_PSA_KEY_ID_RANGE_MATTER_END     (0x000045FF)
#define SLI_PSA_KEY_ID_RANGE_FIND_START     (0x00004600)
#define SLI_PSA_KEY_ID_RANGE_FIND_END       (0x000046FF)
// Silicon Labs SDK 'Stack' allocation
#define SLI_PSA_KEY_ID_RANGE_THREAD_START   (0x00020000)
#define SLI_PSA_KEY_ID_RANGE_THREAD_END     (0x0002FFFF)
#define SLI_PSA_KEY_ID_RANGE_ZIGBEE_START   (0x00030000)
#define SLI_PSA_KEY_ID_RANGE_ZIGBEE_END     (0x0003FFFF)
#define SLI_PSA_KEY_ID_RANGE_BLE_START      (0x00040000)
#define SLI_PSA_KEY_ID_RANGE_BLE_END        (0x0004FFFF)
#define SLI_PSA_KEY_ID_RANGE_BLE_MESH_START (0x00060000)
#define SLI_PSA_KEY_ID_RANGE_BLE_MESH_END   (0x0006FFFF)
#define SLI_PSA_KEY_ID_RANGE_ZWAVE_START    (0x00070000)
#define SLI_PSA_KEY_ID_RANGE_ZWAVE_END      (0x0007FFFF)
#define SLI_PSA_KEY_ID_RANGE_WISUN_START    (0x00090000)
#define SLI_PSA_KEY_ID_RANGE_WISUN_END      (0x0009FFFF)

// -----------------------------------------------------------------------------
// KSU key usage flags (vendor-specific PSA extension)
#if defined(SLI_PSA_DRIVER_FEATURE_KSU)

// New KSU key attributes API (SLI_ prefixed, may change when PSA supports vendor attributes)
typedef uint32_t sli_psa_ksu_key_attributes_t;

#if defined(SL_PSA_KEY_LOCATION_KSU_1)
/// Allow key usage with LPWAES engine
#define SLI_PSA_KSU_KEY_ATTR_ALLOW_LPWAES      ((sli_psa_ksu_key_attributes_t) 0x04000000)
/// Allow key usage with HOSTCRYPTO engine
#define SLI_PSA_KSU_KEY_ATTR_ALLOW_HOSTCRYPTO  ((sli_psa_ksu_key_attributes_t) 0x08000000)
#endif // SL_PSA_KEY_LOCATION_KSU_1

/// Disallow psa_copy_key to copy source keys with location other than KSU (e.g. source key
/// location is SL_PSA_KEY_LOCATION_WRAPPED) to a KSU (target) location.
/// SLI_PSA_KSU_KEY_ATTR_DISALLOW_KSU is not applicable for key that have location KSU.
/// psa_import_key will return PSA_ERROR_INVALID_ARGUMENT if a key is imported with location
/// SL_PSA_KEY_LOCATION_KSU_x and usage attributes have SLI_PSA_KSU_KEY_ATTR_DISALLOW_KSU set.
#define SLI_PSA_KSU_KEY_ATTR_DISALLOW_KSU      ((sli_psa_ksu_key_attributes_t) 0x80000000)
/// Initialize KSU key attributes
#define SLI_PSA_KSU_KEY_ATTRIBUTES_INIT        ((sli_psa_ksu_key_attributes_t) 0x00000000)
#endif //SLI_PSA_DRIVER_FEATURE_KSU

// Convert a type name into an enum entry name, since enum entries and type
// names share the same C namespace.
#define SLI_PSA_CONTEXT_ENUM_NAME(NAME) \
  NAME ## _e
#define SLI_MBEDTLS_CONTEXT_ENUM_NAME(NAME) \
  NAME ## _e

// Convenience macros for getting the size of a context structure type
#define SLI_PSA_CONTEXT_GET_RUNTIME_SIZE(NAME) \
  (sli_psa_context_get_size(SLI_PSA_CONTEXT_ENUM_NAME(NAME)))
#define SLI_MBEDTLS_CONTEXT_GET_RUNTIME_SIZE(NAME) \
  (sli_mbedtls_context_get_size(SLI_MBEDTLS_CONTEXT_ENUM_NAME(NAME)))

// -----------------------------------------------------------------------------
// Type Definitions

// Type names supported by sli_psa_context_get_size.
typedef enum {
  SLI_PSA_CONTEXT_ENUM_NAME(psa_hash_operation_t),
  SLI_PSA_CONTEXT_ENUM_NAME(psa_cipher_operation_t),
  SLI_PSA_CONTEXT_ENUM_NAME(psa_pake_operation_t),
  SLI_PSA_CONTEXT_ENUM_NAME(psa_mac_operation_t),
  SLI_PSA_CONTEXT_ENUM_NAME(psa_aead_operation_t),
  SLI_PSA_CONTEXT_ENUM_NAME(psa_key_derivation_operation_t),
  SLI_PSA_CONTEXT_ENUM_NAME(psa_key_attributes_t)
} sli_psa_context_name_t;

// Type names supported by sli_mbedtls_context_get_size.
typedef enum {
  SLI_MBEDTLS_CONTEXT_ENUM_NAME(mbedtls_aes_context),
  SLI_MBEDTLS_CONTEXT_ENUM_NAME(mbedtls_ccm_context),
  SLI_MBEDTLS_CONTEXT_ENUM_NAME(mbedtls_cipher_context_t),
  SLI_MBEDTLS_CONTEXT_ENUM_NAME(mbedtls_ctr_drbg_context),
  SLI_MBEDTLS_CONTEXT_ENUM_NAME(mbedtls_entropy_context),
  SLI_MBEDTLS_CONTEXT_ENUM_NAME(mbedtls_md_context_t),
  SLI_MBEDTLS_CONTEXT_ENUM_NAME(mbedtls_nist_kw_context),
  SLI_MBEDTLS_CONTEXT_ENUM_NAME(mbedtls_pk_context),
  SLI_MBEDTLS_CONTEXT_ENUM_NAME(mbedtls_sha1_context),
  SLI_MBEDTLS_CONTEXT_ENUM_NAME(mbedtls_sha256_context),
  SLI_MBEDTLS_CONTEXT_ENUM_NAME(mbedtls_ssl_config),
  SLI_MBEDTLS_CONTEXT_ENUM_NAME(mbedtls_ssl_context),
  SLI_MBEDTLS_CONTEXT_ENUM_NAME(mbedtls_ssl_cookie_ctx),
  SLI_MBEDTLS_CONTEXT_ENUM_NAME(mbedtls_x509_crt)
} sli_mbedtls_context_name_t;

// -----------------------------------------------------------------------------
// Function Declarations

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @brief
 *   Get the size of a named PSA context structure. This is valuable for code
 *   shipping as precompiled libraries and needing to link with a source version
 *   of PSA Crypto, since the context structures can change in size based on
 *   configuration options which might not have been present at library
 *   compilation time.
 *
 * @param ctx_type
 *   Which context structure to get the size of. Use
 *   #SLI_PSA_CONTEXT_ENUM_NAME(psa_xxx_operation_t) as argument.
 *
 * @return
 *   Size (in bytes) of the context structure as expected by the current build.
 ******************************************************************************/
size_t sli_psa_context_get_size(sli_psa_context_name_t ctx_type);

/***************************************************************************//**
 * @brief
 *   Get the size of a named Mbed TLS context structure. This is valuable for
 *   code shipping as precompiled libraries and needing to link with a source
 *   version of PSA Crypto, since the context structures can change in size
 *   based on configuration options which might not have been present at library
 *   compilation time.
 *
 * @param ctx_type
 *   Which context structure to get the size of. Use
 *   #SLI_MBEDTLS_CONTEXT_ENUM_NAME(<mbed-tls-type>) as argument.
 *
 * @return
 *   Size (in bytes) of the context structure as expected by the current build.
 ******************************************************************************/
size_t sli_mbedtls_context_get_size(sli_mbedtls_context_name_t ctx_type);

/***************************************************************************//**
 * @brief
 *   Check if a key is copyable even though the key attributes do not have the
 *   PSA_KEY_USAGE_COPY flag set.
 *
 * @param key_id
 *   The key ID of the key of interest.
 *
 * @return
 *   True if the key should be unconditionally copyable, otherwise false.
 ******************************************************************************/
bool sli_psa_key_is_unconditionally_copyable(psa_key_id_t key_id);

#ifdef __cplusplus
}
#endif

/// @endcond

#endif // SLI_PSA_CRYPTO_H
