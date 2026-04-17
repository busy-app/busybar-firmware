/***************************************************************************//**
 * @file sl_token_manager_manufacturing.h
 * @brief Header file for static token defines. All the static tokens
 * are defined in this file. for each static token newly added a corresponding
 * size should be defined. Ex - TOKEN_MFG_EMBER_EUI_64 has its size defined as
 * TOKEN_MFG_EMBER_EUI_64_SIZE.
 *
 * For silicon labs series 2 devices, all the manufacturing token defines are
 * offset in the flash memory from the base address, where the token data is located.
 *
 * For silicon labs series 3 devices, all the manufacturing token defines represent
 * a key value.
 *
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
#ifndef SL_TOKEN_MANAGER_MANUFACTURING_H
#define SL_TOKEN_MANAGER_MANUFACTURING_H

#include "sl_token_manager_config.h"
#include "sl_token_manufacturing_generic.h"

// For series 2 devices, the static tokens need to be stored at well-defined
// locations. None of these addresses should ever change without extremely great care.

// NOTE: On the EFM32 platform the EMBER_EUI64_TOKEN is taken from the
//       DEVINFO info UNIQUEL and UNIQUEH register.

// NOTE: On series 2 devices addresses must be 4 byte aligned.
//       Any value with a size not 4 byte aligned must be followed by
//       enough buffer space to allow the next value to start at a 4 byte
//       aligned address.

// Note: In series 3, since the tokens are stored in klv chain format,
//       these key don't specify the memory offset, rather treated as a key.
//       The key is used in KLV header for identification purposes.

//--- Static device token Data ---
// Appending the static device token with 0x1000 will keep the token out of NVM and
// indicate it is a token living in static device Data space.

// In case of series 2 device, static device Data is mapped to 2kB at USERDATA_BASE (0x0FE00000-0x0FE007FF).
// 0x1000, which is appended to the token, is stripped off when computing the location of a token in the flash
// to get the correct static device Data offset.

// In case of series 3 device, static device space is located in SE MTP region. The 16 bit
// token is used as a key in KLV header for identification purposes. The token is not mapped to any
// specific address in the SE MTP region.

// MAX token length that that can be embedded as part of token is 0x1FFF

// Incase of coexistence of series2 legacy token defines and new common token manager token defines,
// the legacy token defines should be used. The new common token manager token defines should not be used.
// Note that the common token manager token values are not changed, they are kept same as the legacy token values.
#ifndef SL_TOKEN_STATIC_DEVICE_TOKENS
#define SL_TOKEN_STATIC_DEVICE_TOKENS             (0x1000)
#endif
#ifndef TOKEN_MFG_EMBER_EUI_64
#define TOKEN_MFG_EMBER_EUI_64                    (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x1f0)
#define TOKEN_MFG_EMBER_EUI_64_SIZE               (8u)                //   8 bytes
#endif
#ifndef TOKEN_MFG_CUSTOM_EUI_64
#define TOKEN_MFG_CUSTOM_EUI_64                   (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x002)
#define TOKEN_MFG_CUSTOM_EUI_64_SIZE              (8u)                //   8 bytes
#endif
#ifndef TOKEN_MFG_CUSTOM_VERSION
#define TOKEN_MFG_CUSTOM_VERSION                  (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x00C)
#define TOKEN_MFG_CUSTOM_VERSION_SIZE             (2u)                //   2 bytes
#endif
#ifndef TOKEN_MFG_STRING
#define TOKEN_MFG_STRING                          (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x010)
#define TOKEN_MFG_STRING_SIZE                     (16u)               //  16 bytes
#endif
#ifndef TOKEN_MFG_BOARD_NAME
#define TOKEN_MFG_BOARD_NAME                      (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x020)
#define TOKEN_MFG_BOARD_NAME_SIZE                 (16u)               //  16 bytes
#endif
#ifndef TOKEN_MFG_MANUF_ID
#define TOKEN_MFG_MANUF_ID                        (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x030)
#define TOKEN_MFG_MANUF_ID_SIZE                   (2u)                //   2 bytes
#endif
#ifndef TOKEN_MFG_PHY_CONFIG
#define TOKEN_MFG_PHY_CONFIG                      (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x034)
#define TOKEN_MFG_PHY_CONFIG_SIZE                 (2u)                //   2 bytes
#endif
#ifndef TOKEN_MFG_ASH_CONFIG
#define TOKEN_MFG_ASH_CONFIG                      (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x038)
#define TOKEN_MFG_ASH_CONFIG_SIZE                 (2u)                //   2 bytes
#endif
#ifndef TOKEN_MFG_SYNTH_FREQ_OFFSET
#define TOKEN_MFG_SYNTH_FREQ_OFFSET               (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x060)
#define TOKEN_MFG_SYNTH_FREQ_OFFSET_SIZE          (2u)                //   2 bytes
#endif
#ifndef TOKEN_MFG_CCA_THRESHOLD
#define TOKEN_MFG_CCA_THRESHOLD                   (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x064)
#define TOKEN_MFG_CCA_THRESHOLD_SIZE              (2u)                //   2 bytes
#endif
#ifndef TOKEN_MFG_EZSP_STORAGE
#define TOKEN_MFG_EZSP_STORAGE                    (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x068)
#define TOKEN_MFG_EZSP_STORAGE_SIZE               (8u)                //   8 bytes
#endif
#ifndef TOKEN_MFG_XO_TUNE
#define TOKEN_MFG_XO_TUNE                         (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x070)
#define TOKEN_MFG_XO_TUNE_SIZE                    (2u)                //   2 bytes
#endif
#ifndef TOKEN_MFG_ZWAVE_COUNTRY_FREQ
#define TOKEN_MFG_ZWAVE_COUNTRY_FREQ              (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x074)
#define TOKEN_MFG_ZWAVE_COUNTRY_FREQ_SIZE         (1u)                //   1 bytes
#endif
#ifndef TOKEN_MFG_ZWAVE_HW_VERSION
#define TOKEN_MFG_ZWAVE_HW_VERSION                (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x078)
#define TOKEN_MFG_ZWAVE_HW_VERSION_SIZE           (1u)                //   1 bytes
#endif
#ifndef TOKEN_MFG_ZWAVE_PSEUDO_RANDOM_NUMBER
#define TOKEN_MFG_ZWAVE_PSEUDO_RANDOM_NUMBER      (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x07C)
#define TOKEN_MFG_ZWAVE_PSEUDO_RANDOM_NUMBER_SIZE (16u)               //  16 bytes
#endif
#ifndef TOKEN_MFG_SERIAL_NUMBER
#define TOKEN_MFG_SERIAL_NUMBER                   (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x08C)
#define TOKEN_MFG_SERIAL_NUMBER_SIZE              (16u)               //  16 bytes
#endif
#ifndef TOKEN_MFG_LFXO_TUNE
#define TOKEN_MFG_LFXO_TUNE                       (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x09C)
#define TOKEN_MFG_LFXO_TUNE_SIZE                  (1u)                //   1 bytes
#endif
#ifndef TOKEN_MFG_CTUNE
#define TOKEN_MFG_CTUNE                           (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x100)
#define TOKEN_MFG_CTUNE_SIZE                      (2u)                //   2 bytes
#endif
#ifndef TOKEN_MFG_KIT_SIGNATURE
#define TOKEN_MFG_KIT_SIGNATURE                   (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x104)
#define TOKEN_MFG_KIT_SIGNATURE_SIZE              (4u)                //   4 bytes
#endif

//--- Static secure token ---
// In case of series 2 devices, the Static secure token page is physically mapped to 0x0FE04000-0x0FE047FF.
// The lock words are fixed functionality in the first 512 bytes of the
// page so all static secure data must exist above 0x0FE04200.
// Appending the location with 0x2000 will keep the token out of NVM and
// indicate it is a token living in static secure data space.
#ifndef SL_TOKEN_STATIC_SECURE_TOKENS
#define SL_TOKEN_STATIC_SECURE_TOKENS             (0x2000)
#endif
#ifndef TOKEN_MFG_LOCKBITS_PLW
#define TOKEN_MFG_LOCKBITS_PLW                    (SL_TOKEN_STATIC_SECURE_TOKENS | 0x000)
#define TOKEN_MFG_LOCKBITS_PLW_SIZE               (16u)               //  16 bytes
#endif
#ifndef TOKEN_MFG_LOCKBITS_CLW0
#define TOKEN_MFG_LOCKBITS_CLW0                   (SL_TOKEN_STATIC_SECURE_TOKENS | 0x1e8)
#define TOKEN_MFG_LOCKBITS_CLW0_SIZE              (4U)                //   4 bytes
#endif
#ifndef TOKEN_MFG_LOCKBITS_MLW
#define TOKEN_MFG_LOCKBITS_MLW                    (SL_TOKEN_STATIC_SECURE_TOKENS | 0x1f4)
#define TOKEN_MFG_LOCKBITS_MLW_SIZE               (4U)                //   4 bytes
#endif
#ifndef TOKEN_MFG_LOCKBITS_ULW
#define TOKEN_MFG_LOCKBITS_ULW                    (SL_TOKEN_STATIC_SECURE_TOKENS | 0x1f8)
#define TOKEN_MFG_LOCKBITS_ULW_SIZE               (4U)                //   4 bytes
#endif
#ifndef TOKEN_MFG_LOCKBITS_DLW
#define TOKEN_MFG_LOCKBITS_DLW                    (SL_TOKEN_STATIC_SECURE_TOKENS | 0x1fc)
#define TOKEN_MFG_LOCKBITS_DLW_SIZE               (4U)                //   4 bytes
#endif

//--- Static secure token Data    ---
// In case of series 3 device, static secure space is located in data region of flash,
// appending the token with 0x3000 will indicate that the token belongs to static secure range.
// This resulting 16-bit key will be used in KLV header for identification purposes.
// Note: Since the tokens are stored in klv chain format, these key don't specify the memory offset,
// rather treated as a key.
#ifndef SL_TOKEN_STATIC_SECURE_DATA_TOKENS
#define SL_TOKEN_STATIC_SECURE_DATA_TOKENS        (0x3000)
#endif
// Intentionally skipping the first four bytes.  The location might be
// best for a version or other type of information.
#ifndef TOKEN_MFG_CBKE_DATA
#define TOKEN_MFG_CBKE_DATA                       (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x204)
#define TOKEN_MFG_CBKE_DATA_SIZE                  (92u)               //  92 bytes
#endif
#ifndef TOKEN_MFG_SECURITY_CONFIG
#define TOKEN_MFG_SECURITY_CONFIG                 (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x260)
#define TOKEN_MFG_SECURITY_CONFIG_SIZE            (2u)                //   2 bytes
#endif
#ifndef TOKEN_MFG_INSTALLATION_CODE
#define TOKEN_MFG_INSTALLATION_CODE               (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x270)
#define TOKEN_MFG_INSTALLATION_CODE_SIZE          (20u)               //  20 bytes
#endif
#ifndef TOKEN_MFG_SECURE_BOOTLOADER_KEY
#define TOKEN_MFG_SECURE_BOOTLOADER_KEY           (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x286)
#define TOKEN_MFG_SECURE_BOOTLOADER_KEY_SIZE      (16u)               //  16 bytes
#endif
#ifndef TOKEN_MFG_CBKE_283K1_DATA
#define TOKEN_MFG_CBKE_283K1_DATA                 (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x298)
#define TOKEN_MFG_CBKE_283K1_DATA_SIZE            (148u)              // 148 bytes
#endif
#ifndef TOKEN_MFG_BOOTLOAD_AES_KEY
#define TOKEN_MFG_BOOTLOAD_AES_KEY                (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x32C)
#define TOKEN_MFG_BOOTLOAD_AES_KEY_SIZE           (16u)               //  16 bytes
#endif
#ifndef TOKEN_MFG_SIGNED_BOOTLOADER_KEY_X
#define TOKEN_MFG_SIGNED_BOOTLOADER_KEY_X         (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x34C)
#define TOKEN_MFG_SIGNED_BOOTLOADER_KEY_X_SIZE    (32u)               //  32 bytes
#endif
#ifndef TOKEN_MFG_SIGNED_BOOTLOADER_KEY_Y
#define TOKEN_MFG_SIGNED_BOOTLOADER_KEY_Y         (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x36C)
#define TOKEN_MFG_SIGNED_BOOTLOADER_KEY_Y_SIZE    (32u)               //  32 bytes
#endif
#ifndef TOKEN_MFG_THREAD_JOIN_KEY
#define TOKEN_MFG_THREAD_JOIN_KEY                 (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x38C)
#define TOKEN_MFG_THREAD_JOIN_KEY_SIZE            (34u)               //  34 bytes
#endif
#ifndef TOKEN_MFG_NVM3_CRYPTO_KEY
#define TOKEN_MFG_NVM3_CRYPTO_KEY                 (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x3B0)
#define TOKEN_MFG_NVM3_CRYPTO_KEY_SIZE            (16u)               //  16 bytes
#endif
#ifndef TOKEN_MFG_ZW_PRK
#define TOKEN_MFG_ZW_PRK                          (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x3C0)
#define TOKEN_MFG_ZW_PRK_SIZE                     (32u)               //  32 bytes
#endif
#ifndef TOKEN_MFG_ZW_PUK
#define TOKEN_MFG_ZW_PUK                          (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x3E0)
#define TOKEN_MFG_ZW_PUK_SIZE                     (32u)               //  32 bytes
#endif
#ifndef TOKEN_MFG_ZW_QR_CODE
#define TOKEN_MFG_ZW_QR_CODE                      (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x400)
#define TOKEN_MFG_ZW_QR_CODE_SIZE                 (90u)               //  90 bytes
#endif
#ifndef TOKEN_MFG_ZW_INITIALIZED
#define TOKEN_MFG_ZW_INITIALIZED                  (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x45C)
#define TOKEN_MFG_ZW_INITIALIZED_SIZE             (1u)                //   1 bytes
#endif
#ifndef TOKEN_MFG_ZW_QR_CODE_EXT
#define TOKEN_MFG_ZW_QR_CODE_EXT                  (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x460)
#define TOKEN_MFG_ZW_QR_CODE_EXT_SIZE             (16u)               //  16 bytes
#endif

//--- Virtual manufacturing Tokens ---
#ifndef TOKEN_MFG_EUI_64
#define TOKEN_MFG_EUI_64                          (0xb634)
#define TOKEN_MFG_EUI_64_SIZE                     (8u)                //   8 bytes
#endif

/* For backward compatibility */
#ifndef USERDATA_TOKENS
#define USERDATA_TOKENS                           (0x1000)
#endif
#ifndef LOCKBITS_TOKENS
#define LOCKBITS_TOKENS                           (0x2000)
#endif
#ifndef LOCKBITSDATA_TOKENS
#define LOCKBITSDATA_TOKENS                       (0x3000)
#endif

#ifdef APPLICATION_MFG_TOKEN_HEADER
  #include APPLICATION_MFG_TOKEN_HEADER
#endif

#endif // SL_TOKEN_MANAGER_MANUFACTURING_H
