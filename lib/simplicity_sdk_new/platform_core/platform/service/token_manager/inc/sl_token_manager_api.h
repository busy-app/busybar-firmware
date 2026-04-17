/***************************************************************************//**
 * @file sl_token_manager_api.h
 * @brief Common token manager APIs.
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

#ifndef SL_TOKEN_MANAGER_API_H
#define SL_TOKEN_MANAGER_API_H

#if !defined(SL_ZIGBEE_TEST)
#include "sl_common.h"
#endif
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup common_token_manager
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * Initialize the Token Manager.
 *
 * @note This function must be called before any other token manager
 * functions are called. The sl_token_manager_init() call will be automatically
 * added to your initialization.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sl_token_manager_init(void);

/***************************************************************************//**
 * Read token data.
 *
 * @param[in]  token       A 32-bit token identifier. Refer sl_token_manager_defines.h
 *                         on how to create a token.
 * @param[out] data        A pointer to where the token data should be placed.
 * @param[in]  length      The size of the data to be read, in bytes.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sl_token_manager_get_data(uint32_t token,
                                      void *data,
                                      uint32_t length);

/***************************************************************************//**
 * Write token data.
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
 * @param[in] token    A 32-bit token identifier. Refer sl_token_manager_defines.h
 *                     on how to create a token.
 * @param[in] data     A pointer to the data being written.
 * @param[in] length   The size of the token data in number of bytes.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sl_token_manager_set_data(uint32_t token,
                                      void *data,
                                      uint32_t length);

/***************************************************************************//**
 * This call supports reading both dynamic and static tokens partially.
 *
 * @param[in]  token       A 32-bit token identifier. Refer sl_token_manager_defines.h
 *                         on how to create a token.
 * @param[out] data        A pointer to where the token data should be placed.
 * @param[in]  offset      Indicates the number of bytes to skip from the beginning of the
 *                         token data before starting to read.
 * @param[in]  length      The size of the data (full or partial) to be read, in bytes.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sl_token_manager_get_partial_data(uint32_t token,
                                              void *data,
                                              uint32_t offset,
                                              uint32_t length);

/***************************************************************************//**
 * This call supports deleting a dynamic token.
 *
 * @note This call also supports deleting static tokens, if they have been
 * overridden with NVM3 override tokens. Only the override tokens are deleted.
 *
 * @param[in] token   A 32-bit token identifier. Refer sl_token_manager_defines.h
 *                    on how to create a token.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sl_token_manager_delete_dynamic_token(uint32_t token);

/***************************************************************************//**
 * Increments the value of a counter object by 1. Counter tokens are stored in NVM3.
 *
 * @param[in] token  A 32-bit token identifier. Refer sl_token_manager_defines.h
 *                   on how to create a token.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sl_token_manager_increment_counter(uint32_t token);

/***************************************************************************//**
 * This call reads the token size.
 *
 * @note This call can be used to know the token size and user can allocate buffer
 * for holding the token data at runtime.
 *
 * @param[in]  token     A 32-bit token identifier. Refer sl_token_manager_defines.h
 *                       on how to create a token.
 * @param[out] size_out  output parameter to know the size of token.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sl_token_manager_get_size(uint32_t token,
                                      uint32_t *size_out);

/** @} end token_manager */

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/***************************************************************************//**
 * Internal assert failed function.
 *
 * @note Minimal assert functionality to support token manager asserts.  In
 * the future a fully formed assert component will become available and make
 * this assert code obsolete.
 *
 * @param filename
 * @param linenumber
 ******************************************************************************/
void halInternalAssertFailed(const char * filename, int linenumber);

#endif // DOXYGEN_SHOULD_SKIP_THIS

#if defined(SL_TOKEN_MANAGER_BACKWARD_COMPATIBILITY) || defined(SL_TOKEN_MANAGER_BACKEND_INT_FLASH)
// Note: Below header is for S2 backward compatibility.
// The legacy hal API support for silicon labs EFR series 2 devices
// are planned for deprecation. However the new API's listed above
// will be supported. Plan to migrate to new common token manager API's.
#include "sl_token_manager.h"
#endif
#ifdef __cplusplus
}
#endif

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/// @addtogroup common_token_manager Common Token Manager
/// @{
/// @brief Routines for working with tokens using the Common Token Manager.
///
///   @details
///
///
/// @n @section common_token_manager Introduction
///
/// The Common Token Manager (CTM) provides a means to read and write manufacturing
/// (static device and static secure) and dynamic tokens such as manufacturing ID,
/// channel number, transmit power, and various pieces of information that the application
/// needs to be persistent between device power cycles. The token system is designed to
/// abstract implementation details and simplify interacting with differing non-volatile
/// systems. The majority of tokens are stored in NVM3 where they can be rewritten.
/// By default the CTM uses NVM3 component for storage.
///
/// The CTM API is designed to be used across different platforms, providing a unified
/// interface for token management. It supports both dynamic tokens and static tokens.
/// - The dynamic tokens are stored in NVM3, which can be modified at runtime,
/// allowing for read and write operations.
/// - The static tokens are typically used for manufacturing data and are not
///   intended to be modified after the initial write.
///   The static tokens are further divided into
///   - Static device tokens, which are stored in secure memory regions, has limited
///     storage capacity, and are mass erase protected.
///   - Static secure tokens, which are stored in a dedicated flash region, with
///     additional security measures when flash is external (uses 128-bit AES-GCM
///     mode to secure the data). The static secure region has a reserved 8k space
///     at the end of flash. This space is used for storing larger size tokens.
///
/// In CTM, tokens are identified by a 32-bit identifier that includes the token type,
/// size, and a unique key. The token type can be one of the following:
/// - SL_TOKEN_TYPE_NVM3: For dynamic tokens stored in NVM3.
/// - SL_TOKEN_TYPE_NVM3_SECONDARY: For dynamic tokens stored in a secondary NVM3 instance.
/// - SL_TOKEN_TYPE_STATIC_DEVICE: For static device tokens stored in secure memory.
/// - SL_TOKEN_TYPE_STATIC_SECURE: For static secure tokens stored in a dedicated flash region.
/// The token size is embedded in the identifier for only static tokens,
/// allowing the CTM to manage tokens of varying sizes.
///
/// To create a 32-bit token identifier, the CTM provides helper macros such as:
/// - SL_TOKEN_GET_STATIC_DEVICE_TOKEN(token): Creates a static device token.
/// - SL_TOKEN_GET_STATIC_SECURE_TOKEN(token): Creates a static secure token.
/// - SL_TOKEN_GET_DYNAMIC_TOKEN(token, IsCounterObj): Creates a dynamic token for
///   default NVM3 instance.
/// - SL_TOKEN_GET_DYNAMIC_SECONDARY_INSTANCE_TOKEN(token, IsCounterObj): Creates a
///   dynamic token for a secondary NVM3 instance.
/// - SL_TOKEN_GET_DYNAMIC_OVER_RIDE_TOKEN(token): Creates a dynamic override token
///   for static tokens.
/// For more information refer to @ref `sl_token_manager_defines.h` file.
///
/// The CTM API provides functions to initialize the token manager, read and write token data,
/// increment counter tokens, and manage token sizes. It also supports partial reads of
/// token data, allowing applications to access specific portions of token data without
/// needing to read the entire token.
///
/// ## Override Tokens
/// This feature enables applications to store copies of static manufacturing
/// tokens in NVM3, allowing read/write operations on the NVM3 version rather
/// than relying exclusively on the immutable or restricted static token regions.
/// It offers a flexible way to override factory-programmed values when necessary,
/// while preserving the integrity of the original static data.
///
/// To enable NVM3 override functionality, the override config flag
/// `SL_TOKEN_MANAGER_ENABLE_OVERRIDE_TOKENS` in your project sl_token_manager_config.h
/// configuration file should be `enabled` or set to `1`, by enabling this feature
/// the default token retrieval mechanisms can be overridden by custom implementations.
///
/// The search order for tokens read request (when the override flag is enabled) is as follows:
///
/// - By default search a static token in NVM3, when NVM3 override config flag is
/// active, if found return the token data read from NVM3 region.
/// - If the token is not found in NVM3, search in manufacturing region.
///
/// Note: Enabling the override flag may introduce latency compared to direct reads
/// from static region, particularly in time-sensitive scenarios such as device boot-up
/// where rapid access to token data is critical. Furthermore, storing override token
/// data in NVM3 consumes NVM3 memory, which can be a limiting factor on
/// resource-constrained devices.
///
/// @} end common_token_manager ******************************************************/

#endif /* SL_TOKEN_MANAGER_API_H */
