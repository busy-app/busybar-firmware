/***************************************************************************//**
 * @file
 * @brief Abstraction of internal flash raw read, write and erase routines.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef BTL_INTERNAL_FLASH_RAW_H
#define BTL_INTERNAL_FLASH_RAW_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/***************************************************************************//**
 * @addtogroup Core Bootloader Core
 * @{
 * @addtogroup Flash
 * @brief Interface to internal flash
 * @details Used for writing/reading/erasing data to the main flash.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * Erase the raw storage.
 *
 * @param address Start address of the region to erase
 * @param length  Number of bytes to erase
 *
 * @note Some devices, such as Flash-based storages, have restrictions on
 *       the alignment and size of erased regions. The details of the
 *       limitations of a particular storage can be found by reading
 *       the BootloaderStorageInformation_t struct using @ref storage_getInfo.
 *
 * @return @ref BOOTLOADER_OK on success, else error code in
 *         @ref BOOTLOADER_ERROR_STORAGE_BASE range
 ******************************************************************************/
int32_t storage_eraseRaw(uint32_t address, size_t length);

/***************************************************************************//**
 * Write a number of words to raw storage.
 *
 * @param address  The raw address of the storage.
 * @param data     Pointer to data to write.
 * @param numBytes Length of data to write. Must be a multiple of 4.
 *
 * @return @ref BOOTLOADER_OK on success, else error code in
 *         @ref BOOTLOADER_ERROR_STORAGE_BASE range
 ******************************************************************************/
int32_t storage_writeRaw(uint32_t address,
                         uint8_t  *buffer,
                         size_t   length);

/***************************************************************************//**
 * Read number of words from raw storage.
 *
 * @param address    The raw address of the storage.
 * @param buffer     Pointer to the buffer to store read data in.
 * @param numBytes   Number of bytes to read.
 *
 * @return @ref BOOTLOADER_OK on success, else error code in
 *         @ref BOOTLOADER_ERROR_STORAGE_BASE range
 ******************************************************************************/
int32_t storage_readRaw(uint32_t address,
                        uint8_t  *buffer,
                        size_t   length);
/** @} addtogroup internal_flash*/
/** @} addtogroup core */

#endif // BTL_INTERNAL_FLASH_RAW_H
