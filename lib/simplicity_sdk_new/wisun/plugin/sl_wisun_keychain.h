/***************************************************************************//**
 * @file sl_wisun_keychain.h
 * @brief Wi-SUN application keychain
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef SL_WISUN_KEYCHAIN_H
#define SL_WISUN_KEYCHAIN_H

#include <stdint.h>
#include "sl_status.h"

/**************************************************************************//**
 * @addtogroup SL_WISUN_KEYCHAIN Keychain
 * @{
 *****************************************************************************/

/// Enumeration for keychain
typedef enum {
  /// Automatic keychain selection. If SL_WISUN_KEYCHAIN_NVM does not have
  /// any credentials, SL_WISUN_KEYCHAIN_BUILTIN is used instead.
  SL_WISUN_KEYCHAIN_AUTOMATIC,
  /// Built-in keychain
  SL_WISUN_KEYCHAIN_BUILTIN,
  /// NVM keychain
  SL_WISUN_KEYCHAIN_NVM
} sl_wisun_keychain_t;

/// Data structure for a keychain entry
typedef struct {
  /// Pointer to data. When freeing a keychain entry, this pointer must not
  /// be freed.
  const void *data;
  /// Amount of data in bytes
  size_t data_length;
  /// Keychain were the entry is stored
  sl_wisun_keychain_t keychain;
} sl_wisun_keychain_entry_t;

/// Enumeration for private key type
typedef enum {
  /// Private key is stored in plaintext
  SL_WISUN_KEYCHAIN_KEY_TYPE_PLAINTEXT,
  /// Private key is stored in PSA ITS
  SL_WISUN_KEYCHAIN_KEY_TYPE_ID
} sl_wisun_keychain_key_type_t;

/// Data structure for a device private key
typedef struct {
  /// Type of the private key
  sl_wisun_keychain_key_type_t type;
  /// Keychain were the private keý is stored
  sl_wisun_keychain_t keychain;
  /// Private key
  union {
    /// Private key when type is SL_WISUN_KEYCHAIN_KEY_TYPE_PLAINTEXT
    sl_wisun_keychain_entry_t plaintext;
    /// Private key when type is SL_WISUN_KEYCHAIN_KEY_TYPE_ID
    uint32_t key_id;
  } u;
} sl_wisun_keychain_key_t;

/// Data structure for a device credential
typedef struct {
  /// Device certificate
  sl_wisun_keychain_entry_t certificate;
  /// Device private key
  sl_wisun_keychain_key_t pk;
} sl_wisun_keychain_credential_t;

/**************************************************************************//**
 * Retrieve a device credential.
 *
 * @param[in] keychain Keychain to use
 *   - #SL_WISUN_KEYCHAIN_AUTOMATIC: Use NVM keychain if available, built-in otherwise
 *   - #SL_WISUN_KEYCHAIN_BUILTIN: Use built-in keychain
 *   - #SL_WISUN_KEYCHAIN_NVM: Use NVM keychain
 * @param[in] index Built-in credential index (0...max)
 * @return Pointer to the device credential on success, NULL otherwise.
 *
 * This function retrieves a device credential from the given keychain. Calling
 * function is responsible for freeing the returned entry.
 *****************************************************************************/
sl_wisun_keychain_credential_t *sl_wisun_keychain_get_credential(sl_wisun_keychain_t keychain, uint8_t index);

/**************************************************************************//**
 * Retrieve the number of trusted CA certificates.
 *
 * @return Number of trusted CA certificates, 0 if none.
 *
 * This function returns the number of trusted CA certificates in all
 * available keychains.
 *****************************************************************************/
uint8_t sl_wisun_keychain_get_trustedca_count();

/**************************************************************************//**
 * Retrieve a trusted CA certificate.
 *
 * @param[in] index Trusted CA certificate index to retrieve (0...max)
 * @return Pointer to the trusted CA certificate on success, NULL otherwise.
 *
 * This function retrieves a trusted CA certificate from one of the keychains.
 * The index must be below the value returned by sl_wisun_keychain_get_trustedca_count().
 * Calling function is responsible for freeing the returned entry.
 *****************************************************************************/
sl_wisun_keychain_entry_t *sl_wisun_keychain_get_trustedca(uint8_t index);

/** @} (end addtogroup SL_WISUN_KEYCHAIN) */

#endif /* SL_WISUN_KEYCHAIN_H */
