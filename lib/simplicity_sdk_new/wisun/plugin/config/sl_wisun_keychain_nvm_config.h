/***************************************************************************//**
 * @file sl_wisun_keychain_nvm_config.h
 * @brief Wi-SUN application NVM keychain configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#ifndef SL_WISUN_KEYCHAIN_NVM_CONFIG_H
#define SL_WISUN_KEYCHAIN_NVM_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
// <h>Wi-SUN Application NVM Keychain Configuration

// <o SL_WISUN_KEYCHAIN_NVM_DEVICE_CREDENTIAL_KEY> PSA key identifier for the device private key in NVM <f.h>
// <d> 0x00000100
#define SL_WISUN_KEYCHAIN_NVM_DEVICE_CREDENTIAL_KEY 0x00000100

// <o SL_WISUN_KEYCHAIN_NVM_TRUSTEDCA_KEY_MIN> First NVM3 object identifier for trusted CAs <f.h>
// <d> 0x00000101
#define SL_WISUN_KEYCHAIN_NVM_TRUSTEDCA_KEY_MIN 0x00000101

// <o SL_WISUN_KEYCHAIN_NVM_TRUSTEDCA_KEY_MAX> Last NVM3 object identifier for trusted CAs <f.h>
// <d> 0x0000010A
#define SL_WISUN_KEYCHAIN_NVM_TRUSTEDCA_KEY_MAX 0x0000010A

// </h>
// <<< end of configuration section >>>

#endif
