/***************************************************************************/ /**
 * @file  sl_wifi_host_interface.h
 * @brief This file defines the host interface for Wi-Fi operations.
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

#pragma once

#include "sl_status.h"
#include "sl_slist.h"
#include "sl_common.h"
#include <stdbool.h>
#include <stdint.h>
#include "sl_wifi_types.h"

/** \addtogroup SL_WIFI_TYPES
  * @{ */

/**
 * @enum sl_wifi_buffer_type_t
 * @brief Enumeration for types of Wi-Fi buffers.
 */
typedef enum {
  SL_WIFI_TX_FRAME_BUFFER,    ///< Buffer for sending socket data to NWP
  SL_WIFI_RX_FRAME_BUFFER,    ///< Buffer for storing response from NWP
  SL_WIFI_CONTROL_BUFFER,     ///< Buffer for sending command to NWP
  SL_WIFI_SCAN_RESULT_BUFFER, ///< Buffer for storing scan results
} sl_wifi_buffer_type_t;

/**
 * @struct sl_wifi_buffer_configuration_t
 * @brief Structure representing the Wi-Fi buffer configuration.
 */
typedef struct {
  uint8_t tx_buffer_quota;      ///< Indicates the limit on buffers used for sending the data to NWP
  uint8_t rx_buffer_quota;      ///< Indicates the limit on buffers used for storing the response from NWP
  uint8_t control_buffer_quota; ///< Indicates the limit on buffers used for sending the command to NWP
  uint32_t block_size;          ///< Indicates the block size in bytes
  void *buffer_memory;          ///< Pointer to the chunk of memory allocated on the first invocation of malloc
} sl_wifi_buffer_configuration_t;

/** @} */
