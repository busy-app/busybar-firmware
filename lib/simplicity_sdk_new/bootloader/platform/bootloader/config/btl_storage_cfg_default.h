/***************************************************************************//**
 * @file
 * @brief Header for bootloader common storage default configuration
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
#ifndef BTL_STORAGE_CFG_DEFAULT_H
#define BTL_STORAGE_CFG_DEFAULT_H

// This configuration file can be overridden by another component to provide
// default base address configuration.
// Note: Base address configuration is required only for bootloaders with
//       multiple storage slots to store the bootload list.
#error "This bootloader project doesn't support default base address config."

#endif // BTL_STORAGE_CFG_DEFAULT_H
