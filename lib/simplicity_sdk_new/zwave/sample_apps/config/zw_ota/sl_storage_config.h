/***************************************************************************//**
 * # License
 * <b> Copyright 2023 Silicon Laboratories Inc. www.silabs.com </b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of the Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * https://www.silabs.com/about-us/legal/master-software-license-agreement
 * By installing, copying or otherwise using this software, you agree to the
 * terms of the MSLA.
 *
 ******************************************************************************/

#ifndef SL_STORAGE_CONFIG_H
#define SL_STORAGE_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
// <h> Flash Storage configuration

// <o ZW_BTL_STORAGE_SIZE_RELEASE> Size of the bootloader storage in release mode. <0x0..0xFFFFFFFF> <f.h>
// <i> Default: 0x30000
// <i> Note that this value is only being used if BOOTLOADER_STORAGE_USE_DEFAULT
// <i> is set to false. This value will control how much of the flash memory
// <i> is reserved for bootloader storage when the application is built in
// <i> release mode (without the zw_debug component).
#define ZW_BTL_STORAGE_SIZE_RELEASE  0x30000

// <o ZW_BTL_STORAGE_SIZE_DEBUG> Size of the bootloader storage in debug mode. <0x0..0xFFFFFFFF> <f.h>
// <i> Default: 0x28000
// <i> Note that this value is only being used if BOOTLOADER_STORAGE_USE_DEFAULT
// <i> is set to false. This value will control how much of the flash memory
// <i> is reserved for bootloader storage when the application is built in
// <i> debug mode (with the zw_debug component).
#define ZW_BTL_STORAGE_SIZE_DEBUG  0x28000

// </h>
// <<< end of configuration section >>>

// For Z-Wave applications, the bootloader storage slot size should always be custom
#define SL_BOOTLOADER_STORAGE_USE_CUSTOM_SIZE  1

#if defined(NDEBUG)
#define SL_BOOTLOADER_STORAGE_SIZE ZW_BTL_STORAGE_SIZE_RELEASE
#else
/**
 * Decrease the bootloader storage slot size to accommodate the size increase
 * introduced by debug build
 */
#define SL_BOOTLOADER_STORAGE_SIZE ZW_BTL_STORAGE_SIZE_DEBUG
#endif

#endif
