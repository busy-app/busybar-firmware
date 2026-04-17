/******************************************************************************
* @file  rsi_gspi.h
* @brief This files contains functions prototypes related to GSPI peripheral
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

// Includes Files

#include "GSPI.h"
#include "rsi_udma_wrapper.h"

#ifndef RSI_GSPI_H
#define RSI_GSPI_H

#ifdef __cplusplus
extern "C" {
#endif

int32_t GSPI_Initialize(ARM_SPI_SignalEvent_t cb_event,
                        const GSPI_RESOURCES *gspi,
                        UDMA_RESOURCES *udma,
                        RSI_UDMA_DESC_T *UDMA_Table,
                        RSI_UDMA_HANDLE_T *udmaHandle,
                        uint32_t *mem);
int32_t GSPI_Uninitialize(const GSPI_RESOURCES *gspi, UDMA_RESOURCES *udma);
int32_t GSPI_PowerControl(ARM_POWER_STATE state, const GSPI_RESOURCES *gspi);
int32_t GSPI_Control(uint32_t control,
                     uint32_t arg,
                     const GSPI_RESOURCES *gspi,
                     uint32_t base_clock,
                     uint8_t gspi_slavenumber);
int32_t GSPI_Send(const void *data,
                  uint32_t num,
                  const GSPI_RESOURCES *gspi,
                  UDMA_RESOURCES *udma,
                  UDMA_Channel_Info *chnl_info,
                  RSI_UDMA_HANDLE_T udmaHandle);
int32_t GSPI_Receive(void *data,
                     uint32_t num,
                     const GSPI_RESOURCES *gspi,
                     UDMA_RESOURCES *udma,
                     UDMA_Channel_Info *chnl_info,
                     RSI_UDMA_HANDLE_T udmaHandle);
int32_t GSPI_Transfer(const void *data_out,
                      void *data_in,
                      uint32_t num,
                      const GSPI_RESOURCES *gspi,
                      UDMA_RESOURCES *udma,
                      UDMA_Channel_Info *chnl_info,
                      RSI_UDMA_HANDLE_T udmaHandle);
uint32_t GSPI_GetDataCount(const GSPI_RESOURCES *gspi);
ARM_SPI_STATUS GSPI_GetStatus(const GSPI_RESOURCES *gspi);
void GSPI_UDMA_Tx_Event(uint32_t event, uint8_t dmaCh, GSPI_RESOURCES *gspi);
void GSPI_UDMA_Rx_Event(uint32_t event, uint8_t dmaCh, GSPI_RESOURCES *gspi);
void GSPI_IRQHandler(const GSPI_RESOURCES *gspi);
uint32_t GSPI_GetFrameLength(void);
int32_t GSPI_SwapReadWriteByte(boolean_t read, boolean_t write);
uint32_t GSPI_GetTxCount(void);
uint8_t GSPI_GetInitState(void);
ARM_SPI_STATUS GSPI_GetStatus(const GSPI_RESOURCES *gspi);
void GSPI_Write_Dummy_Byte(const GSPI_RESOURCES *gspi);
// Static Inline function to set the spi memory map pll
static inline void GSPI_SetMemoryMapPll(uint16_t value)
{
  SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG9) = value;
}
#ifdef SL_SI91X_GSPI_DMA
void gspi_transfer_complete_callback(uint32_t channel, void *data);
void gspi_error_callback(uint32_t channel, void *data);
#endif
#ifdef __cplusplus
}
#endif

#endif // RSI_GSPI_H
