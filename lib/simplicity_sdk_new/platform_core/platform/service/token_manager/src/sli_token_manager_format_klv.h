/***************************************************************************//**
 * @file sli_token_manager_format_klv.h
 * @brief Header file for KLV (Key-Length-Value) format/type objects.
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

#ifndef SLI_TOKEN_MANAGER_FORMAT_KLV_H
#define SLI_TOKEN_MANAGER_FORMAT_KLV_H

#include "sl_status.h"
#include <stddef.h>
#include "sl_enum.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SLI_TOKEN_KLV_ERASE_STATE                        (0xFFFFFFFF)
//Start value is derived from Device MTP(DMTP)
#define SLI_TOKEN_KLV_STATIC_DEVICE_TOKEN_MAIN_START     (('D' << 24) | ('M' << 16) | ('T' << 8) | ('P' << 0))
//Start value is derived from static secure token chain(SSTC)
#define SLI_TOKEN_KLV_STATIC_SECURE_TOKEN_MAIN_START     (('S' << 24) | ('S' << 16) | ('T' << 8) | ('C' << 0))

// 'K' is 0x4B and 'V' is 0x56
#define SLI_TOKEN_KLV_MAGIC_NUMBER             (('K' << 8) | ('V' << 0))
#define SLI_TOKEN_KLV_HEADER_SIZE              (8) //8bytes or 2 words

#if defined(SL_TOKEN_MANAGER_SECURITY)
SL_ENUM(sl_klv_security_type_t) {
  SL_TOKEN_MANAGER_KLV_SECURITY_TYPE_NONE = 0,
  SL_TOKEN_MANAGER_KLV_SECURITY_TYPE_AEAD = 1,
  SL_TOKEN_MANAGER_KLV_SECURITY_TYPE_ENC = 2,
  SL_TOKEN_MANAGER_KLV_SECURITY_TYPE_INVALID
};
#endif

typedef struct {
  uint16_t magic_number;  // KLV magic number
  uint16_t crc;           // CRC value of the data
  uint16_t length;        // length of the data
  uint16_t key;           // unique key for a KLV object
} sl_klv_header_t;

typedef struct {
  uint32_t key;           // unique key for a KLV object
  uint32_t offset;        // offset to begin reading token data (bytes)
  uint32_t length;        // length of the data
} sl_klv_handle_t;

/***************************************************************************//**
 * @brief
 *  Reads KLV object from one of the static device or static secure chain
 * based on klv_start_address.
 *
 * @param[in]  klv_handle        A pointer to the sl_klv_handle_t,
 *                               containing length and key info
 * @param[in]  klv_start_address A pointer to the KLV start address of a KLV chain.
 * @param[in]  klv_end_address   A pointer to the KLV end address of a KLV chain.
 * @param[out] data              A pointer to where the token data should be placed.
 *
 * @return
 *   The result of the read operation.
 *   @ref SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_read_klv_object(sl_klv_handle_t const *klv_handle,
                                uint32_t *klv_start_address,
                                uint32_t *klv_end_address,
                                void *data);

/***************************************************************************//**
 * @brief
 *  Writes KLV object into one of the static device or static secure chain
 * based on klv_start_address.
 *
 * @param[in] klv_handle        A pointer to the sl_klv_handle_t,
 *                              containing length and key info
 * @param[in] klv_start_address The KLV start address of a KLV chain.
 * @param[in] klv_end_address   The KLV end address of a KLV chain.
 * @param[in] data              A pointer to the data being written.
 *
 * @return
 *   The result of the write operation.
 *   @ref SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_write_klv_object(sl_klv_handle_t const *klv_handle,
                                 uint32_t klv_start_address,
                                 uint32_t klv_end_address,
                                 void const *data);

/***************************************************************************//**
 * @brief
 *  Returns the size of a static token from the KLV header
 *
 * @param[in]  klv_start_address   The KLV start address of a KLV chain.
 * @param[in]  klv_end_address     The KLV end address of a KLV chain.
 * @param[in]  token               32-bit token identifier
 * @param[out] size_out            A pointer to where the token data size should be placed.
 *
 * @return
 * SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_get_size_from_klv(uint32_t klv_start_address,
                                                uint32_t klv_end_address,
                                                uint32_t token,
                                                uint32_t *size_out);
#ifdef __cplusplus
}
#endif

#endif //SLI_TOKEN_MANAGER_FORMAT_KLV_H
