/***************************************************************************//**
 * @file sl_wisun_keychain.c
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

#include <stdbool.h>
#include "psa/crypto.h"
#include "nvm3_default.h"
#include "sl_memory_manager.h"
#include "sl_wisun_keychain_nvm_config.h"
#include "sl_wisun_keychain.h"

#define KEYCHAIN_TRUSTEDCA_COUNT_MAX (SL_WISUN_KEYCHAIN_NVM_TRUSTEDCA_KEY_MAX - SL_WISUN_KEYCHAIN_NVM_TRUSTEDCA_KEY_MIN + 1)

static nvm3_ObjectKey_t key_list[KEYCHAIN_TRUSTEDCA_COUNT_MAX];
static size_t key_list_count;

extern const sl_wisun_keychain_entry_t sl_wisun_keychain_builtin_trustedca[];
extern const sl_wisun_keychain_entry_t sl_wisun_keychain_builtin_devicecert[];
extern const sl_wisun_keychain_entry_t sl_wisun_keychain_builtin_devicekey[];
extern const uint32_t sl_wisun_keychain_builtin_trustedca_count;
extern const uint32_t sl_wisun_keychain_builtin_devicecert_count;
extern const uint32_t sl_wisun_keychain_builtin_devicekey_count;

sl_wisun_keychain_credential_t *sl_wisun_keychain_get_credential(sl_wisun_keychain_t keychain, uint8_t index)
{
  psa_status_t status;
  Ecode_t ret;
  uint32_t nvm_type;
  size_t nvm_len;
  psa_key_attributes_t key_attr;
  sl_wisun_keychain_credential_t *credential = NULL;
  uint8_t* data;

  if (keychain == SL_WISUN_KEYCHAIN_NVM || keychain == SL_WISUN_KEYCHAIN_AUTOMATIC) {
    ret = nvm3_getObjectInfo(nvm3_defaultHandle,
                             SL_WISUN_KEYCHAIN_NVM_DEVICE_CREDENTIAL_KEY,
                             &nvm_type,
                             &nvm_len);
    if (ret == ECODE_OK) {
      key_attr = psa_key_attributes_init();
      status = psa_get_key_attributes(SL_WISUN_KEYCHAIN_NVM_DEVICE_CREDENTIAL_KEY, &key_attr);
      psa_reset_key_attributes(&key_attr);
    }
    if ((ret == ECODE_OK) && (status == PSA_SUCCESS)) {
      // NVM device certificate and private key exist
      credential = (sl_wisun_keychain_credential_t *)sl_malloc(sizeof(sl_wisun_keychain_credential_t) + nvm_len + 1);
      if (!credential) {
        return NULL;
      }
      data = (uint8_t *)(credential + 1);
      // Zero-terminate the data just in case
      data[nvm_len] = 0;
      ret = nvm3_readData(nvm3_defaultHandle,
                          SL_WISUN_KEYCHAIN_NVM_DEVICE_CREDENTIAL_KEY,
                          data,
                          nvm_len);
      if (ret != ECODE_OK) {
        sl_free(credential);
        return NULL;
      }
      credential->certificate.data_length = nvm_len + 1;
      credential->certificate.data = data;
      credential->certificate.keychain = SL_WISUN_KEYCHAIN_NVM;
      credential->pk.type = SL_WISUN_KEYCHAIN_KEY_TYPE_ID;
      credential->pk.keychain = SL_WISUN_KEYCHAIN_NVM;
      credential->pk.u.key_id = SL_WISUN_KEYCHAIN_NVM_DEVICE_CREDENTIAL_KEY;
    } else if (keychain == SL_WISUN_KEYCHAIN_NVM) {
      // NVM device certificate and/or private key not found
      return NULL;
    }
  }

  if (!credential && (keychain == SL_WISUN_KEYCHAIN_BUILTIN || keychain == SL_WISUN_KEYCHAIN_AUTOMATIC)) {
    if (index >= sl_wisun_keychain_builtin_devicekey_count
        || !sl_wisun_keychain_builtin_devicecert[index].data
        || !sl_wisun_keychain_builtin_devicekey[index].data) {
      // No built-in credentials
      return NULL;
    }
    credential = (sl_wisun_keychain_credential_t *)sl_malloc(sizeof(sl_wisun_keychain_credential_t));
    if (!credential)
    {
      return NULL;
    }
    credential->certificate.data_length = sl_wisun_keychain_builtin_devicecert[index].data_length;
    credential->certificate.data = sl_wisun_keychain_builtin_devicecert[index].data;
    credential->certificate.keychain = SL_WISUN_KEYCHAIN_BUILTIN;
    credential->pk.type = SL_WISUN_KEYCHAIN_KEY_TYPE_PLAINTEXT;
    credential->pk.keychain = SL_WISUN_KEYCHAIN_BUILTIN;
    credential->pk.u.plaintext.data_length = sl_wisun_keychain_builtin_devicekey[index].data_length;
    credential->pk.u.plaintext.data = sl_wisun_keychain_builtin_devicekey[index].data;
  }

  return credential;
}

uint8_t sl_wisun_keychain_get_trustedca_count()
{
  // Refresh NVM key cache
  key_list_count = nvm3_enumObjects(nvm3_defaultHandle,
                                    key_list,
                                    KEYCHAIN_TRUSTEDCA_COUNT_MAX,
                                    SL_WISUN_KEYCHAIN_NVM_TRUSTEDCA_KEY_MIN,
                                    SL_WISUN_KEYCHAIN_NVM_TRUSTEDCA_KEY_MAX);
  return key_list_count + sl_wisun_keychain_builtin_trustedca_count;
}

sl_wisun_keychain_entry_t *sl_wisun_keychain_get_trustedca(uint8_t index)
{
  sl_wisun_keychain_entry_t *cert = NULL;
  Ecode_t ret;
  uint32_t nvm_type;
  size_t nvm_len;
  uint8_t* data;

  if (index < key_list_count) {
    // NVM3 trusted CA
    ret = nvm3_getObjectInfo(nvm3_defaultHandle,
                             key_list[index],
                             &nvm_type,
                             &nvm_len);
    if (ret != ECODE_OK) {
      return NULL;
    }
    cert = (sl_wisun_keychain_entry_t *)sl_malloc(sizeof(sl_wisun_keychain_entry_t) + nvm_len + 1);
    if (!cert) {
      return NULL;
    }
    data = (uint8_t *)(cert + 1);
    // Zero-terminate the data just in case
    data[nvm_len] = 0;
    ret = nvm3_readData(nvm3_defaultHandle,
                        key_list[index],
                        data,
                        nvm_len);
    if (ret != ECODE_OK)
    {
      sl_free(cert);
      return NULL;
    }
    cert->data_length = nvm_len + 1;
    cert->data = data;
    cert->keychain = SL_WISUN_KEYCHAIN_NVM;
  } else {
    if (index >= (key_list_count + sl_wisun_keychain_builtin_trustedca_count)) {
      // No such index
      return NULL;
    }
    if (!sl_wisun_keychain_builtin_trustedca[index - key_list_count].data) {
      // No such index
      return NULL;
    }
    // Built-in trusted CA
    cert = (sl_wisun_keychain_entry_t *)sl_malloc(sizeof(sl_wisun_keychain_entry_t));
    if (!cert) {
      return NULL;
    }
    cert->data_length = sl_wisun_keychain_builtin_trustedca[index - key_list_count].data_length;
    cert->data = sl_wisun_keychain_builtin_trustedca[index - key_list_count].data;
    cert->keychain = SL_WISUN_KEYCHAIN_BUILTIN;
  }

  return cert;
}