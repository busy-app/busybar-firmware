/***************************************************************************//**
 * @file
 * @brief Application interface provided to main().
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef APP_H
#define APP_H

#include <stdint.h>
#include "sl_bt_api.h"

/// GATT
#define UUID_16_LEN                   2
#define UUID_128_LEN                  16
#define HANDLE_NOT_INITIALIZED        0

#define GAP_INCOMPLETE_16B_UUID       0x02 // Incomplete List of 16-bit Service Class UUIDs
#define GAP_COMPLETE_16B_UUID         0x03 // Complete List of 16-bit Service Class UUIDs
#define GAP_INCOMPLETE_128B_UUID      0x06 // Incomplete List of 128-bit Service Class UUIDs
#define GAP_COMPLETE_128B_UUID        0x07 // Complete List of 128-bit Service Class UUIDs

/// Data lengths
#define CHAIN_LINK_DATA_LEN           192  // Length of an NVM3 chunk
#define CHAIN_LINK_DATA_NUM           4    // Number of how many chunks needed for a certificate

#define OOB_SIGNATURE_LEN             64
#define CERT_IND_CHUNK_LEN            100
#define EC_PUB_KEY_LEN                65
#define PUB_KEY_OFFSET                26

#define OOB_RANDOM_LEN                (sizeof(aes_key_128))
#define OOB_DATA_LEN                  (2 * OOB_RANDOM_LEN)
#define SIGNED_OOB_DATA_LEN           (OOB_DATA_LEN + OOB_SIGNATURE_LEN)

// -----------------------------------------------------------------------------
// Type definitions

/// Connection properties
typedef struct {
  uint8_t connection_handle;
  bd_addr address;
} conn_properties_t;

/// Characteristic properties
typedef struct characteristic_128_ref_s {
  uint16_t handle;
  uint8_t uuid[UUID_128_LEN];
} characteristic_128_ref_t;

/// GATT UUIDs
#define CBAP_SERVICE_UUID                         \
  0x10, 0x56, 0x28, 0xd0, 0x40, 0xdd, 0x8e, 0x91, \
  0x4d, 0x41, 0x47, 0x81, 0xc6, 0x8c, 0x81, 0xd8
#define CENTRAL_CERT_CHAR_UUID                    \
  0xca, 0x25, 0xf3, 0xa2, 0xf6, 0xda, 0xbd, 0xb6, \
  0x69, 0x4a, 0xaa, 0x08, 0xf9, 0xd0, 0x5f, 0x31
#define PERIPHERAL_CERT_CHAR_UUID                 \
  0x66, 0x50, 0xfd, 0x84, 0x4d, 0xad, 0xa2, 0x99, \
  0xc9, 0x4f, 0xf5, 0x16, 0x9e, 0xda, 0xf6, 0x0c
#define CENTRAL_OOB_CHAR_UUID                     \
  0x2c, 0x19, 0xf1, 0xeb, 0x85, 0xcd, 0xb6, 0x8a, \
  0x2c, 0x4e, 0x7d, 0x89, 0x51, 0x57, 0xd3, 0xe8
#define PERIPHERAL_OOB_CHAR_UUID                  \
  0xab, 0x69, 0xb2, 0x5e, 0xb8, 0x41, 0xb3, 0x8b, \
  0x82, 0x41, 0x51, 0xa2, 0x41, 0xcb, 0x91, 0x69

typedef enum {
  CHAR_CENTRAL_CERT,
  CHAR_PERIPHERAL_CERT,
  CHAR_CENTRAL_OOB,
  CHAR_PERIPHERAL_OOB,
  CHAR_NUM
} characteristics_t;

/// Central device states
typedef enum {
  CENTRAL_SCANNING,
  CENTRAL_DISCOVER_SERVICES,
  CENTRAL_DISCOVER_CHARACTERISTICS,
  CENTRAL_GET_PERIPHERAL_CERT,
  CENTRAL_SEND_CENTRAL_CERT,
  CENTRAL_GET_PERIPHERAL_OOB,
  CENTRAL_SEND_OOB,
  CENTRAL_INCREASE_SECURITY,
  CENTRAL_DONE,
  CENTRAL_STATE_NUM
} central_state_t;

/// Peripheral device states
typedef enum {
  PERIPHERAL_IDLE,
  PERIPHERAL_CENTRAL_CERT_OK,
  PERIPHERAL_CENTRAL_OOB_OK,
  PERIPHERAL_DONE,
  PERIPHERAL_STATE_NUM
} peripheral_state_t;

#endif // APP_H
