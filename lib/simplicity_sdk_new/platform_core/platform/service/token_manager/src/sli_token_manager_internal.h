/***************************************************************************//**
 * @file sli_token_manager_internal.h
 * @brief Internal API implementation.
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

#ifndef SLI_TOKEN_MANAGER_INTERNAL_H
#define SLI_TOKEN_MANAGER_INTERNAL_H

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "sl_status.h"
#include "sl_common.h"
#include "sl_enum.h"

#if defined(SL_TOKEN_MANAGER_BACKEND_INT_FLASH)

// Unlike Series1 EFR devices, Series2 devices do not have an explicit
// flash space for LockBits.  Series 2 uses the top page of main
// flash to store LockBits.  The define LOCKBITS_BASE is then used like
// all EFR devices to access the LockBits memory.
// In non-Series2 devices the top page  of flash is free for all existing prior uses.
#ifndef LOCKBITS_DATA_SIZE
#define LOCKBITS_DATA_SIZE FLASH_PAGE_SIZE
#endif
#ifndef LOCKBITS_BASE
#define LOCKBITS_BASE ((FLASH_BASE + FLASH_SIZE) - LOCKBITS_DATA_SIZE)
#endif
#elif defined(SL_TOKEN_MANAGER_BACKEND_EXT_FLASH)
// The size of the static secure token region in bytes. This region is used to store
// static secure tokens. The default size is set to 8KB.
// Any changes to this size should be made with caution, as it may affect the overall
// memory layout and token management functionality.
// In Series3, Static secure region is defined in the Host data region.
#if defined (__ICCARM__)
extern char __static_secure_token_start__;
extern char __static_secure_token_end__;
#define SLI_TOKEN_STATIC_SECURE_TOKEN_BASE                 (&__static_secure_token_start__)
#define SLI_TOKEN_STATIC_SECURE_TOKEN_END                  (&__static_secure_token_end__)
#elif defined (__GNUC__)
extern char linker_static_secure_tokens_begin;
extern char linker_static_secure_tokens_end;
#define SLI_TOKEN_STATIC_SECURE_TOKEN_BASE                 (&linker_static_secure_tokens_begin)
#define SLI_TOKEN_STATIC_SECURE_TOKEN_END                  (&linker_static_secure_tokens_end)
#else
#error "Unsupported toolchain"
#endif
#define SL_TOKEN_MANAGER_STATIC_SECURE_TOKEN_SIZE          (SLI_TOKEN_STATIC_SECURE_TOKEN_END - SLI_TOKEN_STATIC_SECURE_TOKEN_BASE)
#else
  #error The flash map of manufacturing tokens is not configured for this device
#endif

// Defines indicating the versions number these definitions work with.
#define CURRENT_MFG_CUSTOM_VERSION 0x01FE //MSB is version, LSB is complement

#if defined(SL_TOKEN_MANAGER_ENABLE_DEBUG_TRACE)
//prints debug data from the token access functions
#ifndef TOKENDBG
#include "printf.h"
#define TOKENDBG(x) x
#endif
#else
#ifndef TOKENDBG
#define TOKENDBG(x)
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
/*******************************************************************************
 *******************************  TYPEDEF   ************************************
 ******************************************************************************/
SL_ENUM(sl_token_type_t) {
  SL_TOKEN_TYPE_NVM3_DEFAULT = 0,
  SL_TOKEN_TYPE_NVM3_ZWAVE,
  SL_TOKEN_TYPE_STATIC_DEVICE_TOKEN,
  SL_TOKEN_TYPE_STATIC_SECURE_TOKEN,
  SL_TOKEN_TYPE_INVALID
};

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/
/**
 * @description: Set to 'true' if the sl_token_manager_init is successful,
 * 'false' otherwise.
 */
extern bool sl_token_manager_init_success;

/*****************************************************************************
**************************** Prototypes **************************************
*****************************************************************************/
#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_STATIC_TOKENS)
/***************************************************************************//**
 * This function initializes static tokens read and write operations.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_init_static(void);

/***************************************************************************//**
 * Read the data associated with the specified static token.
 *
 * @param[in]  token     A 32-bit token identifier for the static device
 *                       or static secure token.
 * @param[out] data      A pointer to where the token data should be placed.
 * @param[in]  offset    Indicates the number of bytes to skip from the beginning of the
 *                       token data before starting to read.
 * @param[in]  length    The size of the data (full or partial) to be read, in bytes.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_get_static_data(uint32_t token,
                                              void *data,
                                              uint32_t offset,
                                              uint32_t length);

/***************************************************************************//**
 * Writes data to a specified static token.
 *
 * @note Writing static tokens on Silicon Labs devices has different constraints
 * depending on the series:
 *
 * Silicon Labs Series 2 (EFR) devices: Only static token values that have not
 * been written since the last erase can be written. For areas of flash that
 * cannot be erased by user code, those static tokens are effectively write-once.
 *
 * Silicon Labs Series 3 (SixG3xx) devices: While static device tokens
 * support overwriting, the write operation is limited to a maximum of 100 writes.
 * The static secure tokens are effectively write-once.
 *
 * @param[in] token    A 32-bit token identifier for the static device
 *                     or static secure token.
 * @param[in] data     A pointer to the data being written.
 * @param[in] length   The size of the token data in number of bytes.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_set_static_data(uint32_t token,
                                              void *data,
                                              uint32_t length);

/***************************************************************************//**
 * The purpose of this function is to read static device tokens early,
 * ensuring they are available for use during the initial stages of the
 * system startup.
 *
 * @note The function is designed to be called before the token manager is
 * fully initialized. Once the token manager is initialized, this API will be
 * disabled and the standard token manager get/set functions should be used.
 *
 * @param[in]  token     A 32-bit token identifier for the static device token.
 * @param[out] data      A pointer to where the token data should be placed.
 * @param[in]  length    The size of the data to be read, in bytes.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_get_token_data_preinit(uint32_t token,
                                                     void *data,
                                                     uint32_t length);

/***************************************************************************//**
 * Returns the size of the static token
 *
 * @param[in]  token_type   Token type of the token key.
 * @param[in]  token        The token identifier in 32-bit token format.
 * @param[out] size_out     A pointer to where the token data size should be placed.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_get_static_token_size(sl_token_type_t token_type,
                                                    uint32_t token,
                                                    uint32_t *size_out);
#endif

#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_DYNAMIC_TOKENS)
/***************************************************************************//**
 * Initialize the NVM3 for dynamic token read and write operations.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_init_dynamic(void);

/***************************************************************************//**
 * Read the data associated with the specified dynamic token.
 *
 * @param[in]  token     The NVM3KEY for the token.
 * @param[out] data      A pointer to where the token data should be placed.
 * @param[in]  offset    Indicates the number of bytes to skip from the beginning of the
 *                       token data before starting to read.
 * @param[in]  length    The size of the data (full or partial) to be read, in bytes.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_get_dynamic_data(uint32_t token,
                                               void *data,
                                               uint32_t offset,
                                               uint32_t length);

/***************************************************************************//**
 * Writes data to a specified dynamic token.
 *
 * @param[in] token    The NVM3KEY for the token.
 * @param[in] data     A pointer to the data being written.
 * @param[in] length   The size of the token data in number of bytes.
 *
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_set_dynamic_data(uint32_t token,
                                               void *data,
                                               uint32_t length);

/***************************************************************************//**
 * For NVM3 override of a manufacturing tokens, returns NVM3 key region value
 * allocated for manufacturing tokens.
 *
 * @param[in] token The token identifier in 32-bit token format.
 *
 * @return either SL_TOKEN_NVM3_REGION_STATIC_DEVICE_TOKEN_RANGE or
 * SL_TOKEN_NVM3_REGION_STATIC_SECURE_TOKEN_RANGE.
 ******************************************************************************/
uint32_t sli_token_manager_get_nvm3_key_space_region (uint32_t token);

/***************************************************************************//**
 * Read the data associated with the specified override token.
 * To read an override token, the token manager must be initialized and
 * the override feature must be enabled.
 *
 * @param[in]  token     The manufacturing key for the override token.
 * @param[out] data      A pointer to where the token data should be placed.
 * @param[in]  offset    Indicates the number of bytes to skip from the
 *                       beginning of the token data before starting to read.
 * @param[in]  length    The size of the data (full or partial) to be read,
 *                       in bytes.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_get_override_token_data(uint32_t token,
                                                      void *data,
                                                      uint32_t offset,
                                                      uint32_t length);
#endif // (SL_COMMON_TOKEN_MANAGER_ENABLE_DYNAMIC_TOKENS)

/***************************************************************************//**
 * This function identifies the token and returns token type as enum.
 *
 * @param[in] token The token identifier in 32-bit token format.
 *
 * @return sl_token_type_t token type of the token key.
 ******************************************************************************/
sl_token_type_t sli_token_manager_decode_token_type(uint32_t token);

#ifdef __cplusplus
}
#endif

#endif //SLI_TOKEN_MANAGER_INTERNAL_H
