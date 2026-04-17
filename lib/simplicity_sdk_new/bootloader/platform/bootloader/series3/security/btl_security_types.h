/***************************************************************************//**
 * @file
 * @brief AES decryption functionality for Silicon Labs bootloader
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
#ifndef BTL_SECURITY_TYPES_H
#define BTL_SECURITY_TYPES_H

#include "config/btl_config.h"
#include "core/btl_util.h"
#include "em_device.h"

MISRAC_DISABLE
#include "sl_se_manager.h"
#include "sl_se_manager_cipher.h"
#include "sl_se_manager_extmem.h"
#include "sl_se_manager_util.h"
#include "sl_se_manager_hash.h"
#include "sl_se_manager_signature.h"
MISRAC_ENABLE

/***************************************************************************//**
 * @addtogroup ComponentsS3
 * @{
 * @addtogroup SecurityS3
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup AESS3
 * @{
 ******************************************************************************/
/// Context variable type for AES-CTR (and AES-CCM)
typedef struct AesCtrContext {
  sl_se_key_descriptor_t  aesKeyDesc;       ///< SE Manager Key descriptor

  size_t                  offsetInBlock;    ///< @brief Position in block of last
                                            ///< byte en/decrypted
  uint8_t                 streamBlock[16];  ///< Current CTR encrypted block
  uint8_t                 counter[16];      ///< Current counter/CCM value
} AesCtrContext_t;

/** @} addtogroup AESS3 */

/***************************************************************************//**
 * @addtogroup SHA_256S3
 * @{
 ******************************************************************************/

/// Context type for SHA algorithm
typedef union Sha256Context {
  sl_se_sha256_multipart_context_t       shaContext;      ///<  SHA256 context struct
  uint8_t                  sha[32];         ///< resulting SHA hash
} Sha256Context_t;

/** @} addtogroup SHA_256 */

/***************************************************************************//**
 * @addtogroup DecryptionS3
 * @{
 * @brief Generic decryption functionality for bootloader
 * @details
 ******************************************************************************/

/// Generic decryption context
typedef union {
  AesCtrContext_t aesCtr; ///< Context for AES-CTR-128 decryption
} DecryptContext_t;

/// Generic authentication context
typedef union {
  Sha256Context_t sha256; ///< Context for SHA-256 digest
} AuthContext_t;

/** @} addtogroup DecryptionS3 */

/** @} addtogroup SecurityS3 */
/** @} addtogroup ComponesS3 */

#endif // BTL_SECURITY_TYPES_H
