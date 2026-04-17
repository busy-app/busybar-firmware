/***************************************************************************//**
 * @file
 * @brief External header for the storage component of the Silicon Labs Bootloader.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef BTL_STORAGE_EXT_MEM_H
#define BTL_STORAGE_EXT_MEM_H

#include <stddef.h>
#include <stdbool.h>

#include "api/btl_interface.h"
#include "api/btl_interface_storage.h"
/// Size of the buffer used for chunked reads from storage
#define BTL_STORAGE_READ_BUFFER_SIZE            (2048UL)

/***************************************************************************//**
 * Get base address of the storage medium to use for bootloader storage.
 *
 * @return The base address of the storage medium.
 ******************************************************************************/
uint32_t storage_get_base_address(void);

#endif // BTL_STORAGE_EXT_MEM_H
