/***************************************************************************//**
 * @file sli_token_manager_backend_se_mtp.h
 * @brief Header file for the SE backend operation, handles SE MTP read/write
 * requests.
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

#ifndef SLI_TOKEN_MANAGER_BACKEND_SE_MTP_H
#define SLI_TOKEN_MANAGER_BACKEND_SE_MTP_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/
/***************************************************************************//**
 * @brief
 *   Retrieves the data from the static device token data section in MTP.
 * @param[out] dst
 *   A pointer to where the token data should be placed.
 *   Must be large enough to contain \ref SL_SE_USER_DATA_SIZE bytes.
 * @return
 *   The result of the read operation.
 *   SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_hal_mtp_read_static_device_token(void *dst);

/***************************************************************************//**
 * @brief
 *   Writes data to static device Data section in MTP.
 * @param[in] src
 *   A pointer to the data being written.
 * @return
 *   The result of the read operation.
 *   SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_hal_mtp_write_static_device_token(void const *src);

#ifdef __cplusplus
}
#endif

#endif //SLI_TOKEN_MANAGER_BACKEND_SE_MTP_H
