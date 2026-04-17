/***************************************************************************//**
 * @file sl_token_manager_api.c
 * @brief Common token manager API implementation.
 * @version 1.0.0
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_token_manager_api.h"
#include "sl_token_manager_defines.h"
#include "sli_token_manager_internal.h"
#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_DYNAMIC_TOKENS)
#include "nvm3.h"
#endif
#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_STATIC_TOKENS)
#include "sl_token_manager_manufacturing.h"
#endif
#if defined(SL_TOKEN_MANAGER_SECURITY)
#include "sli_token_manager_hal_crypto.h"
#endif
#include "sl_token_manager_lock.h"

/**
 * @description: Set to 'true' if the sl_token_manager_init is successful,
 * 'false' otherwise.
 */
bool sl_token_manager_init_success = false;

/**************************************************************************//**
 * This function executes, if an assertion condition fails.
 * Apps can have its own implementation.
 * For example, it can be used to log the assert failure or to reset the device.
 * In this case, we will just enter an infinite loop.
 *****************************************************************************/
SL_WEAK void halInternalAssertFailed(const char * filename, int linenumber)
{
  (void)filename;
  (void)linenumber;

  while (1) {
    // Do nothing, just loop forever
  }
}

/**************************************************************************//**
 * Initialize the Token Manager.
 *****************************************************************************/
sl_status_t sl_token_manager_init(void)
{
  sl_status_t status = SL_STATUS_OK;

  sl_token_manager_lock_begin();
#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_STATIC_TOKENS)
  // Init for static token usage
  status = sli_token_manager_init_static();
  if (status != SL_STATUS_OK) {
    TOKENDBG(printf("sli_token_manager_init_static failed with status = %d\r\n", status); )
    sl_token_manager_lock_end();
    return status;
  }
#endif

#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_DYNAMIC_TOKENS)
  // Init for dynamic token usage
  status = sli_token_manager_init_dynamic();
  if (status != SL_STATUS_OK) {
    TOKENDBG(printf("sli_token_manager_init_dynamic failed with status = %d\r\n", status); )
    sl_token_manager_lock_end();
    return status;
  }
#endif

#if defined(SL_TOKEN_MANAGER_SECURITY)
  // Init for crypto usage
  status = hal_crypto_handle.init();
  if (status != SL_STATUS_OK) {
    TOKENDBG(printf("crypto init failed with status = %d\r\n", status); )
    sl_token_manager_lock_end();
    return status;
  }
#endif

  // All went well, update the init flag
  sl_token_manager_init_success = true;
  TOKENDBG(printf("Token manager init was successful\r\n"); )

  sl_token_manager_lock_end();
  return status;
}

/***************************************************************************//**
 * Increments the value of a counter token. This call does not support
 * static tokens.
 ******************************************************************************/
sl_status_t sl_token_manager_increment_counter(uint32_t token)
{
  sl_status_t status = SL_STATUS_OK;

  if (!sl_token_manager_init_success) {
    TOKENDBG(printf("Token manager not initialized"); )
    return SL_STATUS_NOT_INITIALIZED;
  }

#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_DYNAMIC_TOKENS)
  uint32_t nvm3_key = token & NVM3_KEY_MASK;
  status = nvm3_incrementCounter(nvm3_defaultHandle, nvm3_key, NULL);
  TOKENDBG(printf("sl_token_manager_increment_counter token 0x%0lx nvm3_key=0x%0lx status: 0x%0lx\r\n",
                  token,
                  nvm3_key,
                  status); )
#else
  // Avoid unused variable warning
  (void)token;
  // Dynamic token storage is disabled
  status = SL_STATUS_NOT_AVAILABLE;
  TOKENDBG(printf("Include 'common_token_manager_nvm3' to enable dynamic token storage\r\n"); )
#endif
  return status;
}

/***************************************************************************//**
 * This call supports deleting a dynamic token.
 *
 * @note This call also supports deleting manufacturing tokens,
 * which are stored as NVM3 override token in NVM3 region
 ******************************************************************************/
sl_status_t sl_token_manager_delete_dynamic_token(uint32_t token)
{
  sl_status_t status = SL_STATUS_OK;

  if (!sl_token_manager_init_success) {
    TOKENDBG(printf("Token manager not initialized"); )
    return SL_STATUS_NOT_INITIALIZED;
  }

#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_DYNAMIC_TOKENS)
  uint32_t nvm3_key = token & NVM3_KEY_MASK;

  status = nvm3_deleteObject(nvm3_defaultHandle, nvm3_key);
  TOKENDBG(printf("sl_token_manager_delete_dynamic_token token: %0lx nvm3_key=0x%0lx status: 0x%0lx\r\n",
                  token,
                  nvm3_key,
                  status); )
#else
  // Avoid unused variable warning
  (void)token;
  // Dynamic token storage is disabled
  status = SL_STATUS_NOT_AVAILABLE;
  TOKENDBG(printf("Include 'common_token_manager_nvm3' to enable dynamic token storage\r\n"); )
#endif
  return status;
}

/***************************************************************************//**
 * Read the data associated with the specified dynamic or static token.
 * To read a static or a dynamic token, the token manager must be initialized.
 ******************************************************************************/
sl_status_t sl_token_manager_get_data(uint32_t token,
                                      void *data,
                                      uint32_t length)
{
  sl_status_t status = SL_STATUS_OK;
  sl_token_type_t token_type;

  if (!sl_token_manager_init_success) {
    TOKENDBG(printf("Token manager not initialized"); )
    return SL_STATUS_NOT_INITIALIZED;
  }

  // Decode the token type
  token_type = sli_token_manager_decode_token_type(token);

  switch (token_type) {
    case SL_TOKEN_TYPE_NVM3_DEFAULT:
    case SL_TOKEN_TYPE_NVM3_ZWAVE:
    {
#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_DYNAMIC_TOKENS)
      // Offset of '0' always reads from the start.
      status = sli_token_manager_get_dynamic_data(token, data, 0, length);
#else
      // Dynamic token storage is disabled
      status = SL_STATUS_NOT_AVAILABLE;
      TOKENDBG(printf("Include 'common_token_manager_nvm3' to enable dynamic token storage\r\n"); )
#endif
    }
    break;

    case SL_TOKEN_TYPE_STATIC_DEVICE_TOKEN:
    case SL_TOKEN_TYPE_STATIC_SECURE_TOKEN:
    {
#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_STATIC_TOKENS)
#if defined(SL_TOKEN_MANAGER_ENABLE_OVERRIDE_TOKENS) && (SL_TOKEN_MANAGER_ENABLE_OVERRIDE_TOKENS == 1)
      // Since the override config is enabled, read the token from NVM3 first.
      // If the token is not found, fall back to static storage.
      // Once a token is overridden, it will always be read from NVM3.
      // The original factory value will only be used if the override token is
      // explicitly deleted from NVM3 or if NVM3 is erased. In these cases,
      // token access will fall back to the factory-programmed value.
      status = sli_token_manager_get_override_token_data(token & 0xFFFF, data, 0, length);

      if (status != SL_STATUS_NOT_FOUND) {
        break; // Override token found.
      }
#endif // SL_TOKEN_MANAGER_ENABLE_OVERRIDE_TOKENS
      // Read from static token storage
      sl_token_manager_lock_begin();
      // Offset of '0' always reads from the start.
      status = sli_token_manager_get_static_data(token, data, 0, length);
      sl_token_manager_lock_end();
#else
      // Static token storage is disabled
      status = SL_STATUS_NOT_AVAILABLE;
      TOKENDBG(printf("Include 'common_token_manager_static' to enable static token storage\r\n"); )
#endif
    }
    break;

    case SL_TOKEN_TYPE_INVALID:
    default:
      // Invalid token type
      status = SL_STATUS_INVALID_PARAMETER;
      break;
  }

  return status;
}

/***************************************************************************//**
 * Read the data (full or partial) associated with the specified dynamic or static token.
 ******************************************************************************/
sl_status_t sl_token_manager_get_partial_data(uint32_t token,
                                              void *data,
                                              uint32_t offset,
                                              uint32_t length)
{
  sl_status_t status = SL_STATUS_OK;
  sl_token_type_t token_type;

  if (!sl_token_manager_init_success) {
    TOKENDBG(printf("Token manager not initialized"); )
    return SL_STATUS_NOT_INITIALIZED;
  }

  // Decode the token type
  token_type = sli_token_manager_decode_token_type(token);

  switch (token_type) {
    case SL_TOKEN_TYPE_NVM3_DEFAULT:
    case SL_TOKEN_TYPE_NVM3_ZWAVE:
    {
#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_DYNAMIC_TOKENS)
      status = sli_token_manager_get_dynamic_data(token, data, offset, length);
#else
      // Dynamic token storage is disabled
      status = SL_STATUS_NOT_AVAILABLE;
      TOKENDBG(printf("Include 'common_token_manager_nvm3' to enable dynamic token storage\r\n"); )
#endif
    }
    break;

    case SL_TOKEN_TYPE_STATIC_DEVICE_TOKEN:
    case SL_TOKEN_TYPE_STATIC_SECURE_TOKEN:
    {
#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_STATIC_TOKENS)
#if defined(SL_TOKEN_MANAGER_ENABLE_OVERRIDE_TOKENS) && (SL_TOKEN_MANAGER_ENABLE_OVERRIDE_TOKENS == 1)
      // Since the override config is enabled, read the token from NVM3 first.
      // If the token is not found, fall back to static storage.
      // Once a token is overridden, it will always be read from NVM3.
      // The original factory value will only be used if the override token is
      // explicitly deleted from NVM3 or if NVM3 is erased. In these cases,
      // token access will fall back to the factory-programmed value.
      status = sli_token_manager_get_override_token_data(token & 0xFFFF, data, offset, length);

      if (status != SL_STATUS_NOT_FOUND) {
        break; // Override token found.
      }
#endif // SL_TOKEN_MANAGER_ENABLE_OVERRIDE_TOKENS
      // Read from static token storage
      sl_token_manager_lock_begin();
      status = sli_token_manager_get_static_data(token, data, offset, length);
      sl_token_manager_lock_end();
#else
      // Static token storage is disabled
      status = SL_STATUS_NOT_AVAILABLE;
      TOKENDBG(printf("Include 'common_token_manager_static' to enable static token storage\r\n"); )
#endif
    }
    break;

    case SL_TOKEN_TYPE_INVALID:
    default:
      //Invalid token type
      status = SL_STATUS_INVALID_PARAMETER;
      break;
  }

  return status;
}

/***************************************************************************//**
 * Writes the data to the specified dynamic or static token.
 * To write a static or a dynamic token, the token manager must be initialized.
 ******************************************************************************/
sl_status_t sl_token_manager_set_data(uint32_t token,
                                      void *data,
                                      uint32_t length)
{
  sl_status_t status = SL_STATUS_OK;
  sl_token_type_t token_type;

  if (!sl_token_manager_init_success) {
    TOKENDBG(printf("Token manager not initialized"); )
    return SL_STATUS_NOT_INITIALIZED;
  }

  // Decode the token type
  token_type = sli_token_manager_decode_token_type(token);

  switch (token_type) {
    case SL_TOKEN_TYPE_NVM3_DEFAULT:
    case SL_TOKEN_TYPE_NVM3_ZWAVE:
    {
#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_DYNAMIC_TOKENS)
      status = sli_token_manager_set_dynamic_data(token, data, length);
#else
      // Dynamic token storage is disabled
      status = SL_STATUS_NOT_AVAILABLE;
      TOKENDBG(printf("Include 'common_token_manager_nvm3' to enable dynamic token storage\r\n"); )
#endif
    }
    break;

    case SL_TOKEN_TYPE_STATIC_DEVICE_TOKEN:
    case SL_TOKEN_TYPE_STATIC_SECURE_TOKEN:
    {
#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_STATIC_TOKENS)
      sl_token_manager_lock_begin();
      status = sli_token_manager_set_static_data(token, data, length);
      sl_token_manager_lock_end();
#else
      // Static token storage is disabled
      status = SL_STATUS_NOT_AVAILABLE;
      TOKENDBG(printf("Include 'common_token_manager_static' to enable static token storage\r\n"); )
#endif
    }
    break;

    case SL_TOKEN_TYPE_INVALID:
    default:
      //Invalid token type
      status = SL_STATUS_INVALID_PARAMETER;
      break;
  }
  return status;
}

/***************************************************************************//**
 * This call returns token size.
 ******************************************************************************/
sl_status_t sl_token_manager_get_size(uint32_t token, uint32_t *size_out)
{
  if (size_out == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (!sl_token_manager_init_success) {
    TOKENDBG(printf("Token manager not initialized"); )
    return SL_STATUS_NOT_INITIALIZED;
  }

  sl_token_type_t token_type = sli_token_manager_decode_token_type(token);
  sl_status_t status = SL_STATUS_OK;

  switch (token_type) {
    case SL_TOKEN_TYPE_NVM3_DEFAULT:
    case SL_TOKEN_TYPE_NVM3_ZWAVE:
    {
#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_DYNAMIC_TOKENS)
      uint32_t type = 0;
      size_t object_size;
      status = nvm3_getObjectInfo(nvm3_defaultHandle, (token & NVM3_KEY_MASK), &type, &object_size);
      *size_out = (uint32_t)object_size;
#else
      // Dynamic token storage is disabled
      status = SL_STATUS_NOT_AVAILABLE;
      TOKENDBG(printf("Include 'common_token_manager_nvm3' to enable dynamic token storage\r\n"); )
#endif
    }
    break;

    case SL_TOKEN_TYPE_STATIC_SECURE_TOKEN:
    case SL_TOKEN_TYPE_STATIC_DEVICE_TOKEN:
    {
#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_STATIC_TOKENS)
      sl_token_manager_lock_begin();
      status = sli_token_manager_get_static_token_size(token_type, token, size_out);
      sl_token_manager_lock_end();
#else
      // Static token storage is disabled
      status = SL_STATUS_NOT_AVAILABLE;
      TOKENDBG(printf("Include 'common_token_manager_static' to enable static token storage\r\n"); )
#endif
    }
    break;

    case SL_TOKEN_TYPE_INVALID:
    default:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
  }

  return status;
}
