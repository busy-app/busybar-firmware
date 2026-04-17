/***************************************************************************//**
 * @file
 * @brief Configuration for bootloader
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
#ifndef BTL_CONFIG_H
#define BTL_CONFIG_H

//
// Bootloader Version
//
#define BOOTLOADER_VERSION_MAIN_MAJOR             3
#define BOOTLOADER_VERSION_MAIN_MINOR             2

#include "core/btl_util.h"
MISRAC_DISABLE
#include "em_device.h"
MISRAC_ENABLE

//
// Bootloader configuration
#if defined(BTL_CONFIG_FILE)
  #include BTL_CONFIG_FILE
#else
  #include "btl_core_cfg.h"
#endif

#ifndef BOOTLOADER_VERSION_MAIN_CUSTOMER
#define BOOTLOADER_VERSION_MAIN_CUSTOMER          1
#endif

#define BOOTLOADER_VERSION_MAIN (BOOTLOADER_VERSION_MAIN_MAJOR   << 24 \
                                 | BOOTLOADER_VERSION_MAIN_MINOR << 16 \
                                 | BOOTLOADER_VERSION_MAIN_CUSTOMER)

//
// Option validation
//

#if (defined(BOOTLOADER_USE_SYMMETRIC_KEY_FROM_SE_STORAGE)      \
  && (BOOTLOADER_USE_SYMMETRIC_KEY_FROM_SE_STORAGE == 1))       \
  && (defined(BOOTLOADER_USE_SYMMETRIC_KEY_FROM_APP_PROPERTIES) \
  && (BOOTLOADER_USE_SYMMETRIC_KEY_FROM_APP_PROPERTIES == 1))
  #error "Choose either to store the GBL decryption key in Secure Element or Application Properties struct"
#endif // BOOTLOADER_USE_SYMMETRIC_KEY_FROM_APP_PROPERTIES && BOOTLOADER_USE_SYMMETRIC_KEY_FROM_APP_PROPERTIES

#endif // BTL_CONFIG_H
