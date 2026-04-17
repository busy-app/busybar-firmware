/***************************************************************************//**
 * @file
 * @brief CPC SDIO SECONDARY driver configuration file.
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

#include "sl_cpc_config.h"
#include "sl_cpc_instance_handles.h"

#ifndef SL_CPC_DRV_SDIO_SECONDARY_CONFIG_H
#define SL_CPC_DRV_SDIO_SECONDARY_CONFIG_H

// <o SLI_CPC_BUFFER_HANDLE_MAX_COUNT> Max buffer count
#define SLI_CPC_BUFFER_HANDLE_MAX_COUNT      SLI_CPC_DRV_SDIO_BUFFER_HANDLE_MAX_COUNT

// <o SLI_CPC_HDLC_HEADER_MAX_COUNT> Hdlc header max count
#define SLI_CPC_HDLC_HEADER_MAX_COUNT    SLI_CPC_DRV_SDIO_HDLC_HEADER_MAX_COUNT

// <o SLI_CPC_HDLC_REJECT_MAX_COUNT> Number hdlc reject max count
#define SLI_CPC_HDLC_REJECT_MAX_COUNT        SLI_CPC_DRV_SDIO_HDLC_REJECT_MAX_COUNT

// <o SLI_CPC_RX_BUFFER_MAX_LENGTH> Rx buffer max length
#define SLI_CPC_RX_BUFFER_MAX_LENGTH         SLI_CPC_DRV_SDIO_RX_BUFFER_MAX_LENGTH

// <o SLI_CPC_RX_BUFFER_MAX_LENGTH> Rx queue items max count
#define SLI_CPC_RX_QUEUE_ITEM_MAX_COUNT      SLI_CPC_DRV_SDIO_RX_QUEUE_ITEM_MAX_COUNT

// <o SL_CPC_DRV_SDIO_EXP_RX_QUEUE_SIZE> Number of frame that can be queued in the driver receive queue
// <i> Default: 10
#define SL_CPC_DRV_SDIO_EXP_RX_QUEUE_SIZE 10

// <o SL_CPC_DRV_SDIO_EXP_TX_QUEUE_SIZE> Number of frame that can be queued in the driver transmit queue
// <i> Default: 10
#define SL_CPC_DRV_SDIO_EXP_TX_QUEUE_SIZE 10

#define SL_CPC_DRV_SDIO_RX_QUEUE_SIZE SL_CPC_DRV_SDIO_EXP_RX_QUEUE_SIZE
#define SL_CPC_DRV_SDIO_TX_QUEUE_SIZE SL_CPC_DRV_SDIO_EXP_TX_QUEUE_SIZE

#endif /* SL_CPC_DRV_SDIO_SECONDARY_CONFIG_H */
