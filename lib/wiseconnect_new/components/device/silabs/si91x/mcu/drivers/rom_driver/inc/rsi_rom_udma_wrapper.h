/******************************************************************************
* @file  rsi_rom_udma_wrapper.h
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

#ifndef __RSI_ROM_UDMA_WRAPPER_H__
#define __RSI_ROM_UDMA_WRAPPER_H__

/**
 * \ingroup   RSI_SPECIFIC_DRIVERS
 * \defgroup RSI_UDMA_DRIVER UDMA WRAPPER 
 *  @{
 *
 */
#include "rsi_ccp_user_config.h"
#include "rsi_packing.h"
#include "rsi_udma_wrapper.h"
#include "UDMA.h"
#if defined(A11_ROM)
#include "rsi_rom_table_si91x.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

STATIC INLINE RSI_UDMA_HANDLE_T UDMAx_Initialize(const UDMA_RESOURCES *udma,
                                                 RSI_UDMA_DESC_T *UDMA_Table,
                                                 RSI_UDMA_HANDLE_T udmaHandle,
                                                 uint32_t *mem)
{
#if defined(UDMA_ROMDRIVER_PRESENT) && defined(A11_ROM)
  udmaHandle = ROMAPI_UDMA_WRAPPER_API->uDMAx_Initialize(udma, UDMA_Table, udmaHandle, mem);
#else
  udmaHandle = uDMAx_Initialize(udma, UDMA_Table, udmaHandle, mem);
#endif
  return udmaHandle;
}

STATIC INLINE int32_t UDMAx_Uninitialize(const UDMA_RESOURCES *udma)
{
#if defined(UDMA_ROMDRIVER_PRESENT) && defined(A11_ROM)
  return ROMAPI_UDMA_WRAPPER_API->uDMAx_Uninitialize(udma);
#else
  return uDMAx_Uninitialize(udma);
#endif
}

STATIC INLINE int32_t UDMAx_ChannelConfigure(const UDMA_RESOURCES *udma,
                                             uint8_t ch,
                                             uint32_t src_addr,
                                             uint32_t dest_addr,
                                             uint32_t size,
                                             RSI_UDMA_CHA_CONFIG_DATA_T control,
                                             const RSI_UDMA_CHA_CFG_T *config,
                                             UDMA_SignalEvent_t cb_event,
                                             UDMA_Channel_Info *chnl_info,
                                             RSI_UDMA_HANDLE_T udmaHandle)
{
  if (control.transferType == UDMA_SOFTWARE_TRIGG) {
    udma->desc->vsUDMAChaConfigData1.transferType = UDMA_SOFTWARE_TRIGG;
  }
  if ((control.totalNumOfDMATrans + 1UL) > size) {
    control.totalNumOfDMATrans = (unsigned int)((size - 1) & 0x03FF);
  }

#if defined(UDMA_ROMDRIVER_PRESENT) && defined(A11_ROM)
  return ROMAPI_UDMA_WRAPPER_API
    ->uDMAx_ChannelConfigure(udma, ch, src_addr, dest_addr, size, control, config, cb_event, chnl_info, udmaHandle);
#else
  return uDMAx_ChannelConfigure(udma, ch, src_addr, dest_addr, size, control, config, cb_event, chnl_info, udmaHandle);
#endif
}

STATIC INLINE int32_t UDMAx_ChannelEnable(uint8_t ch, const UDMA_RESOURCES *udma, RSI_UDMA_HANDLE_T udmaHandle)
{
#if defined(UDMA_ROMDRIVER_PRESENT) && defined(A11_ROM)

  return ROMAPI_UDMA_WRAPPER_API->uDMAx_ChannelEnable(ch, udma, udmaHandle);
#else
  return uDMAx_ChannelEnable(ch, udma, udmaHandle);
#endif
}

STATIC INLINE int32_t UDMAx_DMAEnable(const UDMA_RESOURCES *udma, RSI_UDMA_HANDLE_T udmaHandle)
{
#if defined(UDMA_ROMDRIVER_PRESENT) && defined(A11_ROM)
  return ROMAPI_UDMA_WRAPPER_API->uDMAx_DMAEnable(udma, udmaHandle);
#else
  return uDMAx_DMAEnable(udma, udmaHandle);
#endif
}

STATIC INLINE int32_t UDMAx_ChannelDisable(uint8_t ch, const UDMA_RESOURCES *udma, RSI_UDMA_HANDLE_T udmaHandle)
{
#if defined(UDMA_ROMDRIVER_PRESENT) && defined(A11_ROM)
  return ROMAPI_UDMA_WRAPPER_API->uDMAx_ChannelDisable(ch, udma, udmaHandle);
#else
  return uDMAx_ChannelDisable(ch, udma, udmaHandle);
#endif
}

STATIC INLINE uint32_t UDMAx_ChannelGetCount(uint8_t ch,
                                             RSI_UDMA_CHA_CONFIG_DATA_T control,
                                             RSI_UDMA_CHA_CFG_T config,
                                             const UDMA_RESOURCES *udma,
                                             RSI_UDMA_HANDLE_T udmaHandle)
{
#if defined(UDMA_ROMDRIVER_PRESENT) && defined(A11_ROM)
  return ROMAPI_UDMA_WRAPPER_API->uDMAx_ChannelGetCount(ch, control, config, udma, udmaHandle);
#else
  return uDMAx_ChannelGetCount(ch, control, config, udma, udmaHandle);
#endif
}
STATIC INLINE void UDMAx_IRQHandler(UDMA_RESOURCES *udma, RSI_UDMA_DESC_T *UDMA_Table, UDMA_Channel_Info *chnl_info)
{
#if defined(UDMA_ROMDRIVER_PRESENT) && defined(A11_ROM)
  ROMAPI_UDMA_WRAPPER_API->uDMAx_IRQHandler(udma, UDMA_Table, chnl_info);
#else
  uDMAx_IRQHandler(udma, UDMA_Table, chnl_info);
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* __RSI_ROM_UDMA_WRAPPER_H__*/
