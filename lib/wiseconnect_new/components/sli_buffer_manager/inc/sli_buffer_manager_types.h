/***************************************************************************/ /**
 * @file sli_buffer_manager_types.h
 * @brief
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

#ifndef SLI_WIFI_BUFFER_MANAGER_TYPES_H
#define SLI_WIFI_BUFFER_MANAGER_TYPES_H

#include "sli_mem_pool.h"
#include "stdint.h"

typedef enum {
  SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID =
    0, ///< Allocate from common pool first and then allocate from dedicated pool.
  SLI_BUFFER_MANAGER_ALLOCATION_TYPE_DEDICATED ///< Allocate from dedicated pool only.
} sli_buffer_manager_allocation_types_t;

typedef enum {
  SLI_BUFFER_MANAGER_CE_TX_POOL = 0,    ///< Command Engine Command TX Pool.
  SLI_BUFFER_MANAGER_CE_RX_POOL,        ///< Command Engine Command RX Pool.
  SLI_BUFFER_MANAGER_CE_DATA_POOL,      ///< Command Engine Data Packet Pool.
  SLI_BUFFER_MANAGER_CE_METADATA_POOL,  ///< Command Engine Meta Data Pool.
  SLI_BUFFER_MANAGER_HAL_METADATA_POOL, ///< HAL Metadata Pool.
  SLI_BUFFER_MANAGER_CP_CMD_TX_POOL,    ///< Command Parser Command TX Pool.
  SLI_BUFFER_MANAGER_CP_CMD_RX_POOL,    ///< Command Parser Command RX Pool.
  SLI_BUFFER_MANAGER_CP_DATA_TX_POOL,   ///< Command Parser Data TX Pool.
  SLI_BUFFER_MANAGER_CP_DATA_RX_POOL,   ///< Command Parser Data RX Pool.
  SLI_BUFFER_MANAGER_QUEUE_NODE_POOL,   ///< Command Parser Data RX Pool.
  SLI_BUFFER_MANAGER_MAX_POOL           ///< Maximum Pool Size.
} sli_buffer_manager_pool_types_t;

/**
 * @typedef sli_buffer_t
 */
typedef void *sli_buffer_t;

/**
 * @struct sli_buffer_manager_pool_info_t
 * @brief Structure representing the buffer manager configuration.
 */
typedef struct {
  uint32_t block_size;  ///< Size of the blocks.
  uint32_t block_count; ///< Number of blocks in the pool.
} sli_buffer_manager_pool_info_t;

/**
 * @struct sli_buffer_manager_configuration_t
 * @brief Structure representing the buffer manager configuration.
 */
typedef struct {
  sli_buffer_manager_pool_info_t *pool_info[SLI_BUFFER_MANAGER_MAX_POOL]; ///< Array of Pool Info.
  sli_buffer_manager_pool_info_t common_pool_info;                        ///< Common Pool Info.
} sli_buffer_manager_configuration_t;
#endif
