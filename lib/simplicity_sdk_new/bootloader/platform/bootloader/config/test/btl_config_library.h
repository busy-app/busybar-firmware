/***************************************************************************//**
 * @file
 * @brief Configuration header for Silicon Labs Bootloader library.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef BTL_CONFIG_LIBRARY_H
#define BTL_CONFIG_LIBRARY_H

#ifndef BTL_UPGRADE_LOCATION_BASE
#define BTL_UPGRADE_LOCATION_BASE  0x8000UL
#endif

// --------------------------------
// EBL Image parser configuration

// No config -- signature/encryption enable/disable is outside library

// --------------------------------
// Debug component configuration

// Enable debug printing
//#define SL_DEBUG_PRINT

// ----

// Disable CRC32 verification support in application properties
//#define BTL_LIB_NO_SUPPORT_CRC32_SIGNATURE

#endif // BTL_CONFIG_LIBRARY_H
