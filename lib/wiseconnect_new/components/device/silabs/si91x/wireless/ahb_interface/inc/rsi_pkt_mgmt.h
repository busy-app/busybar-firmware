/***************************************************************************/ /**
 * @file  rsi_pkt_mgmt.h
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

#ifndef RSI_PKT_MGMT_H
#define RSI_PKT_MGMT_H

#include "rsi_os.h"
#include <stdint.h>
#if ((defined RSI_SDIO_INTERFACE) && (defined SLI_SI917))
#define SIZE_OF_HEADROOM 216
#else
#define SIZE_OF_HEADROOM 0
#endif
/******************************************************
 * *                      Macros
 * ******************************************************/
/******************************************************
 * *                    Constants
 * ******************************************************/
/******************************************************
 * *                   Enumerations
 * ******************************************************/
/******************************************************
 * *                 Type Definitions
 * ******************************************************/
// driver TX/RX packet structure
typedef struct rsi_pkt_s {
  // next packet pointer
  struct rsi_pkt_s *next;

#if ((defined RSI_SDIO_INTERFACE) && (defined SLI_SI917))
  uint8_t headroom[SIZE_OF_HEADROOM];
#endif

  // host descriptor
  uint8_t desc[16];

  // payload
  uint8_t data[1];
} rsi_pkt_t;

// packet pool structure
typedef struct rsi_pkt_pool_s {
  // Pool total packets count
  uint16_t size;

  // Pool avaialble packets count
  uint16_t avail;

  // Pool pointer
  void **pool;

  rsi_semaphore_handle_t pkt_sem;

} rsi_pkt_pool_t;

/******************************************************
 * *                    Structures
 * ******************************************************/
/******************************************************
 * *                 Global Variables
 * ******************************************************/
/******************************************************
 * *               Function Declarations
 * ******************************************************/
int32_t rsi_pkt_pool_init(rsi_pkt_pool_t *pool_cb, uint8_t *buffer, uint32_t total_size, uint32_t pkt_size);
rsi_pkt_t *rsi_pkt_alloc(rsi_pkt_pool_t *pool_cb);
int32_t rsi_pkt_free(rsi_pkt_pool_t *pool_cb, rsi_pkt_t *pkt);
uint32_t rsi_is_pkt_available(rsi_pkt_pool_t *pool_cb);
#endif
