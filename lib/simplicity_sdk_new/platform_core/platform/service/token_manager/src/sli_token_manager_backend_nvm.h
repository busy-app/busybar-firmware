/***************************************************************************//**
 * @file sli_token_manager_backed_nvm.h
 * @brief Header file for the flash backend operation, handles flash read/write
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

#ifndef SLI_TOKEN_MANAGER_BACKEND_NVM_H
#define SLI_TOKEN_MANAGER_BACKEND_NVM_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *  Open the flash hal for usage.
 *
 * @details
 *   This function must be run at initialization, before any other functions
 *   are called. It is used to call necessary startup routines before the
 *   flash can be accessed.
 *
 * @param[in] flash_adr
 *   A pointer to the destination in flash.
 *
 * @param[in] flash_size
 *   The flash size.
 *
 * @return
 *   The result of the open call.
 *   @ref SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_flash_open(uint32_t flash_adr, uint32_t flash_size);

/***************************************************************************//**
 * @brief
 *  Close the flash hal.
 *
 * @details
 *   This function should be called at program termination.
 *   Should be done before any graceful halts.
 ******************************************************************************/
void sli_token_manager_flash_close(void);

/***************************************************************************//**
 * @brief
 *  Write data to the flash.
 *
 * @details
 *   This function is used to write data to the flash. This is a blocking
 *   function.
 *
 * @param[in] flash_adr
 *   A pointer to the destination in flash.
 *
 * @param[in] data
 *   A pointer to the source data.
 *
 * @param[in] size
 *   The number of bytes of data to write.
 *
 * @return
 *   The result of the write operation.
 *   @ref SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_flash_write(uint32_t flash_adr, void const *data, uint32_t size);

/***************************************************************************//**
 * @brief
 *  Read data from flash.
 *
 * @details
 *   This function is used to read data from the flash. It will be a
 *   blocking call, since the thread asking for data to be read cannot continue
 *   without the data.
 *
 * @param[in] flash_adr
 *   A pointer to the destination in flash.
 *
 * @param[out] data
 *   A pointer to the destination buffer.
 *
 * @param[in] size
 *   The number of bytes of data to read.
 *
 * @return
 *   The result of the read operation.
 *   @ref SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_flash_read(uint32_t flash_adr, void *data, uint32_t size);

/***************************************************************************//**
 * @brief
 *   Erase a page in the flash.
 *
 * @details
 *   This function is used to erase a flash page.
 *
 * @param[in] flash_adr
 *   A memory address pointing to the start of the page to erase.
 *
 * @param[in] flash_size
 *   The flash size, the value should be multiple of flash page size.
 *
 * @return
 *   The result of the erase operation.
 *   @ref SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_flash_erase(uint32_t flash_adr, uint32_t flash_size);

#if defined(SL_TOKEN_MANAGER_BACKEND_INT_FLASH)
/***************************************************************************//**
 * @brief
 *  Write first 2 bytes of token data to the flash.
 *
 * @param[in] flash_adr
 *   A pointer to the destination in flash.
 *
 * @param[in] data
 *   A pointer to the source data.
 *
 * @param[in] size
 *   The number of bytes of data to write.
 *
 * @return
 *   The result of the write operation.
 *   @ref SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_flash_write_end_word(uint32_t flash_adr, void const *data, uint32_t size);

/***************************************************************************//**
 * @brief
 *  Write last 2 bytes of token data to the flash.
 *
 * @param[in] flash_adr
 *   A pointer to the destination in flash.
 *
 * @param[in] data
 *   A pointer to the source data.
 *
 * @return
 *   The result of the write operation.
 *   @ref SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_flash_write_start_word(uint32_t flash_adr, void const *data);
#endif

#ifdef __cplusplus
}
#endif

#endif //SLI_TOKEN_MANAGER_BACKEND_NVM_H
