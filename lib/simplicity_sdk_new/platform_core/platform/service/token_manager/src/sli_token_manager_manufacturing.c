/***************************************************************************//**
 * @file sli_token_manufacturing.c
 * @brief Manufacturing toke API implementation. Handles static device and
 * static secure token read/write operations.
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
#include "sl_token_manager_manufacturing.h"
#include "sli_token_manager_internal.h"

#if defined(SL_TOKEN_MANAGER_BACKEND_INT_FLASH_SE) || defined(SL_TOKEN_MANAGER_BACKEND_EXT_FLASH)
#include "sl_se_manager_util.h"
#endif // (SL_TOKEN_MANAGER_BACKEND_INT_FLASH_SE) || (SL_TOKEN_MANAGER_BACKEND_EXT_FLASH)

#if defined(SL_TOKEN_MANAGER_BACKEND_EXT_FLASH)
#include "sli_token_manager_backend_nvm.h"
#include "sl_se_manager.h"
#include "sl_se_manager_extmem.h"
#include "sli_token_manager_format_klv.h"
#include "sli_token_manager_backend_se_mtp.h"
#endif //SL_TOKEN_MANAGER_BACKEND_EXT_FLASH

#if defined(SL_TOKEN_MANAGER_BACKEND_INT_FLASH) || defined(SL_TOKEN_MANAGER_BACKEND_EXT_FLASH)
#define SILABS_DEVINFO_EUI64_LOW   (DEVINFO->EUI64L)
#define SILABS_DEVINFO_EUI64_HIGH  (DEVINFO->EUI64H)
#else
  #error The flash map of manufacturing tokens is not available for this device
#endif

#if defined (SL_TOKEN_MANAGER_BACKEND_INT_FLASH)
/* Reusing the legacy file to handle manufacturing writes/read request,
   once the legacy folder is deprecated, below extern's can be removed */
extern sl_status_t sl_token_set_manufacturing_data(uint32_t token,
                                                   void *data,
                                                   uint32_t length);
#endif

static const uint8_t nullEui[] = { 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU };

#if defined(SL_TOKEN_MANAGER_BACKEND_EXT_FLASH)
// Array used for read/write operation of static device token data
// in klv object format in SE's MTP region
// make it 4 bytes aligned
uint8_t  static_device_token_data[SL_SE_USER_DATA_SIZE] __attribute__((aligned(4)));

/***************************************************************************//**
 * Reads static secure tokens from data flash region.
 *
 * @param[in]  token     The token identifier in 32-bit token format.
 * @param[out] data      A pointer to where the token data should be placed.
 * @param[in]  offset    Indicates the number of bytes to skip from the beginning of the
 *                       token data before starting to read.
 * @param[in]  length    The size of the token data in number of bytes.
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
static sl_status_t sli_token_manager_read_static_secure_token(uint32_t token,
                                                              void *data,
                                                              uint32_t offset,
                                                              uint32_t length)
{
  sl_klv_handle_t klv_handle = { token, offset, length };

  return sli_read_klv_object(&klv_handle,
                             (uint32_t *)SLI_TOKEN_STATIC_SECURE_TOKEN_BASE,
                             (uint32_t *)SLI_TOKEN_STATIC_SECURE_TOKEN_END,
                             data);
}

/***************************************************************************//**
 * Writes static secure tokens to flash region.
 *
 * @param[in]  token   The token identifier in 32-bit token format.
 * @param[in]  data    A pointer to the data being written.
 * @param[in]  length  The size of the token data in number of bytes.
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
static sl_status_t sli_token_manager_write_static_secure_token(uint32_t token,
                                                               void *data,
                                                               uint32_t length)
{
  sl_status_t status = SL_STATUS_OK;
  sl_klv_handle_t klv_handle = { token, 0, length };

  status = sli_write_klv_object(&klv_handle,
                                (uint32_t)SLI_TOKEN_STATIC_SECURE_TOKEN_BASE,
                                (uint32_t)SLI_TOKEN_STATIC_SECURE_TOKEN_END,
                                data);

  return status;
}

/***************************************************************************//**
 * Reads static device tokens from MTP region.
 *
 * @param[in]  token     The token identifier in 32-bit token format.
 * @param[out] data      A pointer to where the token data should be placed.
 * @param[in]  offset    Indicates the number of bytes to skip from the beginning of the
 *                       token data before starting to read.
 * @param[in]  length    The size of the token data in number of bytes.
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
static sl_status_t sli_token_manager_read_static_device_token(uint32_t token,
                                                              void *data,
                                                              uint32_t offset,
                                                              uint32_t length)
{
  uint32_t *klv_start_addr, *klv_end_addr;
  sl_status_t status = SL_STATUS_OK;
  sl_klv_handle_t klv_handle = { token, offset, length };

  // Read from MTP
  status = sli_hal_mtp_read_static_device_token(&static_device_token_data[0]);
  if (status == SL_STATUS_OK) {
    klv_start_addr = (uint32_t *)&static_device_token_data[0];
    klv_end_addr = (uint32_t *)((&static_device_token_data[0]) + sizeof(static_device_token_data));
    // Traverse the klv chain to read the token data
    return sli_read_klv_object(&klv_handle, klv_start_addr, klv_end_addr, data);
  }

  return status;
}

/***************************************************************************//**
 * Reads static device tokens from MTP region.
 *
 * @param[in]  token    The token identifier in 32-bit token format.
 * @param[in]  data     A pointer to the data being written.
 * @param[in]  length   The size of the token data in number of bytes.
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
static sl_status_t sli_token_manager_write_static_device_token(uint32_t token,
                                                               void *data,
                                                               uint32_t length)
{
  sl_status_t status = SL_STATUS_OK;
  uint32_t klv_start_addr, klv_end_addr;
  sl_klv_handle_t klv_handle = { token, 0, length };

  // Read from MTP
  status = sli_hal_mtp_read_static_device_token(&static_device_token_data[0]);

  if (status == SL_STATUS_OK) {
    klv_start_addr = (uint32_t)&static_device_token_data[0];
    klv_end_addr = (uint32_t)((&static_device_token_data[0]) + sizeof(static_device_token_data));
    // Create klv object
    status = sli_write_klv_object(&klv_handle, klv_start_addr, klv_end_addr, data);
    if (status == SL_STATUS_OK) {
      // Write the data to MTP
      return sli_hal_mtp_write_static_device_token(&static_device_token_data[0]);
    }
  }

  return status;
}
#endif // SL_TOKEN_MANAGER_BACKEND_EXT_FLASH

/***************************************************************************//**
 * Reads token data. Based on the token type either read static secure or static
 * device token.
 *
 * @param[out]  data     A pointer to the data being written.
 * @param[in]   token    The token identifier in 32-bit token format.
 * @param[in]   offset   Indicates the number of bytes to skip from the beginning of the
 *                       token data before starting to read.
 * @param[in]   length   The size of the token data in number of bytes.
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
static sl_status_t sli_token_manager_get_static_token_data(void *data,
                                                           uint32_t token,
                                                           uint32_t offset,
                                                           uint32_t length)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t *ram = (uint8_t*)data;

  // Get the token type
  sl_token_type_t token_type = sli_token_manager_decode_token_type(token);

  if ((token & 0xFFFF) == TOKEN_MFG_EUI_64) {
    // There are two EUI64's stored in the flash, Ember and Custom.
    // TOKEN_MFG_EUI_64 is the address used by the generic EUI64 token.
    // It is the low level routine's responsibility to pick and return EUI64 from
    // either Ember or Custom.  Return the Custom EUI64 if it is not all FF's,
    // otherwise return the Ember EUI64.
    uint8_t eui64[TOKEN_MFG_CUSTOM_EUI_64_SIZE];
    memset(&eui64[0], 0xFF, sizeof(eui64));
#if defined(SL_TOKEN_MANAGER_BACKEND_INT_FLASH)
    status = sli_token_manager_get_static_data(SL_TOKEN_GET_STATIC_DEVICE_TOKEN(TOKEN_MFG_CUSTOM_EUI_64),
                                               &eui64,
                                               offset,
                                               length);
#else
    status = sli_token_manager_read_static_device_token(SL_TOKEN_GET_STATIC_DEVICE_TOKEN(TOKEN_MFG_CUSTOM_EUI_64),
                                                        &eui64,
                                                        offset,
                                                        length);
#endif
    if (memcmp(eui64, nullEui, sizeof(eui64) /*EUI64_SIZE*/) == 0) {
      status = sli_token_manager_get_static_data(TOKEN_MFG_EMBER_EUI_64,
                                                 &eui64,
                                                 offset,
                                                 length);
    }

    if (status == SL_STATUS_OK) {
      memcpy(ram, eui64, length);
    }
  } else if ((token & 0xFFFF) == TOKEN_MFG_EMBER_EUI_64) {
    uint32_t eui64_low = SILABS_DEVINFO_EUI64_LOW;
    uint32_t eui64_high = SILABS_DEVINFO_EUI64_HIGH;
    tokTypeMfgEmberEui64 ember_eui64 = { 0 };

    if (TOKEN_MFG_EMBER_EUI_64_SIZE < (offset + length)) {
      TOKENDBG(printf("(TOKEN_MFG_EMBER_EUI_64_SIZE < (offset + length))\n"));
      return SL_STATUS_INVALID_PARAMETER;
    }

    memcpy(ember_eui64, &eui64_low, sizeof(eui64_low));
    memcpy(ember_eui64 + sizeof(eui64_low), &eui64_high, sizeof(eui64_high));
    // Copy the required length of data
    memcpy(ram, &ember_eui64[offset], length);
  } else if (token_type == SL_TOKEN_TYPE_STATIC_DEVICE_TOKEN) {
#if defined(SL_TOKEN_MANAGER_BACKEND_INT_FLASH)
    uint32_t size = 0;
    sli_token_manager_get_static_token_size(token_type, token, &size);
    if (size < (offset + length)) {
      TOKENDBG(printf("(size < (offset + length))\n"));
      return SL_STATUS_INVALID_PARAMETER;
    }
    // Read from internal flash
    uint32_t real_address = ((USERDATA_BASE + (token & 0x0FFF)) + (offset));
    uint8_t *flash = (uint8_t *)real_address;

    memcpy(ram, flash, length);
#elif defined(SL_TOKEN_MANAGER_BACKEND_EXT_FLASH)
    status = sli_token_manager_read_static_device_token(token, data, offset, length);
#endif
  } else if (token_type == SL_TOKEN_TYPE_STATIC_SECURE_TOKEN) {
#if defined(SL_TOKEN_MANAGER_BACKEND_INT_FLASH)
    uint32_t size = 0;
    sli_token_manager_get_static_token_size(token_type, token, &size);
    if (size < (offset + length)) {
      TOKENDBG(printf("(size < (offset + length))\n"));
      return SL_STATUS_INVALID_PARAMETER;
    }
    // Read from internal flash
    uint32_t real_address = ((LOCKBITS_BASE + (token & 0x0FFF)) + (offset));
    uint8_t *flash = (uint8_t *)real_address;

    memcpy(ram, flash, length);
#elif defined(SL_TOKEN_MANAGER_BACKEND_EXT_FLASH)
    status = sli_token_manager_read_static_secure_token(token, data, offset, length);
#endif
  } else {
    // Sate MISRA
    status = SL_STATUS_FAIL;
  }
  return status;
}

/***************************************************************************//**
 * Initialize flash for static tokens read and write operations.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_init_static(void)
{
  sl_status_t status = SL_STATUS_OK;
#if defined(SL_TOKEN_MANAGER_BACKEND_EXT_FLASH)
  // Initialization for static secure tokens usage.
  status = sli_token_manager_flash_open((uint32_t)SLI_TOKEN_STATIC_SECURE_TOKEN_BASE,
                                        (uint32_t)SL_TOKEN_MANAGER_STATIC_SECURE_TOKEN_SIZE);
#endif
  return status;
}

/***************************************************************************//**
 * Read the data associated with the specified manufacturing token.
 *
 * @param[in]  token     The manufacturing key for the static device or static secure token.
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
                                              uint32_t length)
{
  sl_status_t status = SL_STATUS_OK;

  if ((length == 0U) || (data == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;   // Nothing to do...
  }

  TOKENDBG(printf("get_static_data token:0x%0lx length:0x%0lx\r\n",
                  token,
                  length); )
  status = sli_token_manager_get_static_token_data(data, token, offset, length);
  return status;
}

/***************************************************************************//**
 * Writes data to a manufacturing token.
 *
 * @note Only manufacturing token values that have not been written since
 * the last erase can be written.  For areas of flash that cannot be erased
 * by user code, those manufacturing tokens are effectively write-once.
 *
 * @param[in] token    The manufacturing key for the static device or static secure token.
 * @param[in] data     A pointer to the data being written.
 * @param[in] length   The size of the token data in number of bytes.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_set_static_data(uint32_t token,
                                              void *data,
                                              uint32_t length)
{
  if ((length == 0U) || (data == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;   // Nothing to do...
  }

  TOKENDBG(printf("set_static_data token:0x%0lx length:0x%0lx\r\n",
                  token,
                  length); )
#if defined(SL_TOKEN_MANAGER_BACKEND_INT_FLASH)
  // Reusing the legacy file here to process write request,
  // once the legacy folder is deprecated, bring the definition to this file.
  return sl_token_set_manufacturing_data((token & 0xFFFF), data, length);
#elif defined(SL_TOKEN_MANAGER_BACKEND_EXT_FLASH)
  sl_token_type_t token_type = SL_TOKEN_TYPE_INVALID;
  // Get the token type
  token_type = sli_token_manager_decode_token_type(token);

  if (token_type == SL_TOKEN_TYPE_STATIC_DEVICE_TOKEN) {
    return sli_token_manager_write_static_device_token((token & 0xFFFF), data, length);
  } else if (token_type == SL_TOKEN_TYPE_STATIC_SECURE_TOKEN) {
    return sli_token_manager_write_static_secure_token((token & 0xFFFF), data, length);
  } else {
    // In coming token parameter is not valid.
    return SL_STATUS_INVALID_PARAMETER;
  }
#endif
}

/***************************************************************************//**
 * Returns the size of the static token
 *
 * @param[in]  token_type   Token type of the token key.
 * @param[in]  token        A 32-bit token identifier for the static device token.
 * @param[out] size_out     A pointer to where the token data size should be placed.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_get_static_token_size(sl_token_type_t token_type,
                                                    uint32_t token,
                                                    uint32_t *size_out)
{
#if defined(SL_TOKEN_MANAGER_BACKEND_INT_FLASH)
  (void)token_type;
  // For S2 devices the data size is retrieved from the 32-bit token identifier created.
  *size_out = ((token >> SL_TOKEN_STATIC_TOKEN_SIZE_OFFSET) & SL_TOKEN_STATIC_TOKEN_SIZE_MASK);
  return SL_STATUS_OK;
#elif defined(SL_TOKEN_MANAGER_BACKEND_EXT_FLASH)
  uint32_t klv_start_addr = 0, klv_end_addr = 0;

  // Determine KLV start and end addresses based on token type
  switch (token_type) {
    case SL_TOKEN_TYPE_STATIC_DEVICE_TOKEN:
      klv_start_addr = (uint32_t)&static_device_token_data[0];
      klv_end_addr = (uint32_t)((&static_device_token_data[0]) + sizeof(static_device_token_data));
      break;

    case SL_TOKEN_TYPE_STATIC_SECURE_TOKEN:
      klv_start_addr = (uint32_t)SLI_TOKEN_STATIC_SECURE_TOKEN_BASE;
      klv_end_addr = (uint32_t)SLI_TOKEN_STATIC_SECURE_TOKEN_END;
      break;

    default:
      return SL_STATUS_INVALID_PARAMETER; // Invalid token type
  }

  // Call KLV size retrieval function
  return sli_token_manager_get_size_from_klv(klv_start_addr,
                                             klv_end_addr,
                                             token,
                                             size_out);
#endif
}

#if defined(SL_TOKEN_MANAGER_ENABLE_OVERRIDE_TOKENS) && (SL_TOKEN_MANAGER_ENABLE_OVERRIDE_TOKENS == 1)
/***************************************************************************//**
 * Read the data associated with the specified override token created for static tokens.
 * To read an override token, the token manager must be initialized and
 * the override feature must be enabled.
 *
 * @param[in]  token     The manufacturing key for the override token.
 * @param[out] data      A pointer to where the token data should be placed.
 * @param[in]  offset    Indicates the number of bytes to skip from the beginning of the
 *                       token data before starting to read.
 * @param[in]  length    The size of the data (full or partial) to be read, in bytes.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_get_override_token_data(uint32_t token,
                                                      void *data,
                                                      uint32_t offset,
                                                      uint32_t length)
{
  sl_status_t status = SL_STATUS_OK;
  // Special handling for MFG_EUI_64 token.
  if ((token & 0xFFFF) == TOKEN_MFG_EUI_64) {
    tokTypeMfgEui64 eui64;

    memset(eui64, 0xFFU, sizeof(tokTypeMfgEui64));
    // Check if the Custom EUI64 override token is present.
    status = sli_token_manager_get_dynamic_data(SL_TOKEN_GET_DYNAMIC_OVER_RIDE_TOKEN(TOKEN_MFG_CUSTOM_EUI_64),
                                                &eui64,
                                                offset,
                                                length);
    if (status == SL_STATUS_OK) {
      // Override value is found, copy the data.
      memcpy((char*)data, eui64, length);
    }
  } else {
    // Read the override token
    status = sli_token_manager_get_dynamic_data(SL_TOKEN_GET_DYNAMIC_OVER_RIDE_TOKEN(token & 0xFFFF), data, offset, length);
  }

  return status;
}
#endif // SL_TOKEN_MANAGER_ENABLE_OVERRIDE_TOKENS
