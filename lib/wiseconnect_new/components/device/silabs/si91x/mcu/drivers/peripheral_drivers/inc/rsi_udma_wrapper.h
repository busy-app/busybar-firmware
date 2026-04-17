/*******************************************************************************
* @file  rsi_udma_wrapper.h
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
/**===========================================================================
 * @brief This files contains functions prototypes releated to UDMA peripheral
 * @section Description :
 * This file contains the list of function prototypes for the UDMA and low level function definations 
 * Following are list of API's which need to be defined in this file.
============================================================================**/
// Include Files

#include "rsi_ccp_common.h"
#include "rsi_error.h"
#include "rsi_packing.h"
#include "UDMA.h"

#ifndef RSI_UDMA_WRAPPER_H
#define RSI_UDMA_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif // UNUSED_PARAMETER

typedef struct {
  uint32_t SrcAddr;
  uint32_t DestAddr;
  uint32_t Size;
  uint32_t Cnt;
  UDMA_SignalEvent_t cb_event;
} UDMA_Channel_Info;

RSI_UDMA_HANDLE_T uDMAx_Initialize(const UDMA_RESOURCES *udma,
                                   RSI_UDMA_DESC_T *UDMA_Table,
                                   RSI_UDMA_HANDLE_T udmaHandle,
                                   uint32_t *mem);
int32_t uDMAx_Uninitialize(const UDMA_RESOURCES *udma);
int32_t uDMAx_ChannelConfigure(const UDMA_RESOURCES *udma,
                               uint8_t ch,
                               uint32_t src_addr,
                               uint32_t dest_addr,
                               uint32_t size,
                               RSI_UDMA_CHA_CONFIG_DATA_T control,
                               const RSI_UDMA_CHA_CFG_T *config,
                               UDMA_SignalEvent_t cb_event,
                               UDMA_Channel_Info *chnl_info,
                               RSI_UDMA_HANDLE_T udmaHandle);
int32_t uDMAx_ChannelEnable(uint8_t ch, const UDMA_RESOURCES *udma, RSI_UDMA_HANDLE_T udmaHandle);
int32_t uDMAx_DMAEnable(const UDMA_RESOURCES *udma, RSI_UDMA_HANDLE_T udmaHandle);
int32_t uDMAx_ChannelDisable(uint8_t ch, const UDMA_RESOURCES *udma, RSI_UDMA_HANDLE_T udmaHandle);
uint32_t uDMAx_ChannelGetCount(uint8_t ch,
                               RSI_UDMA_CHA_CONFIG_DATA_T control,
                               RSI_UDMA_CHA_CFG_T config,
                               const UDMA_RESOURCES *udma,
                               RSI_UDMA_HANDLE_T udmaHandle);
void uDMAx_IRQHandler(UDMA_RESOURCES *udma, RSI_UDMA_DESC_T *UDMA_Table, UDMA_Channel_Info *chnl_info);

#ifdef __cplusplus
}
#endif

#endif // RSI_UDMA_WRAPPER_H
