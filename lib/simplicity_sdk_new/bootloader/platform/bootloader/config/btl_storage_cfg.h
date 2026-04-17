/***************************************************************************//**
 * @file
 * @brief Configuration header for bootloader common storage
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
#ifndef BTL_STORAGE_CONFIG_H
#define BTL_STORAGE_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
// <h> Bootloader Common Storage Configuration

// <e BTL_STORAGE_CUSTOM_BASE_ADDRESS> Custom base address config
// <i> Default: 1
#define BTL_STORAGE_CUSTOM_BASE_ADDRESS  1

#if BTL_STORAGE_CUSTOM_BASE_ADDRESS

// <o BTL_STORAGE_BASE_ADDRESS> Start address of bootload info
// <i> Default: 0
// <i> Absolute start address for the "bootload info" meta-information about which storage slot to attempt firmware update
// <i> from, in which order. Take care to ensure that any storage slots don't overlap with the location of bootload info.
#define BTL_STORAGE_BASE_ADDRESS  0

// </e>

#else // BTL_STORAGE_CUSTOM_BASE_ADDRESS

#include "btl_storage_cfg_default.h"

#endif // BTL_STORAGE_CUSTOM_BASE_ADDRESS

// </h>
// <<< end of configuration section >>>

#endif // BTL_STORAGE_CONFIG_H
