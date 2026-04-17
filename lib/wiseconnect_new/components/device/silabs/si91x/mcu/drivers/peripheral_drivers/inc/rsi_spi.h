/******************************************************************************
* @file  rsi_spi.h
* @brief   This files contains functions prototypes related to SPI peripheral
* @section Description :
* This file contains the list of function prototypes for the spi and low level 
* function definitions
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

// Include Files

#include "SPI.h"
#include "rsi_udma_wrapper.h"
#ifdef SSI_DUAL_QUAD_COMPONENT
#include "sl_si91x_ssi.h"
#endif

#ifndef RSI_GSPI_H
#define RSI_GSPI_H

#ifdef __cplusplus
extern "C" {
#endif

int32_t SPI_Initialize(ARM_SPI_SignalEvent_t cb_event,
                       const SPI_RESOURCES *spi,
                       UDMA_RESOURCES *udma,
                       RSI_UDMA_DESC_T *UDMA_Table,
                       RSI_UDMA_HANDLE_T *udmaHandle,
                       uint32_t *mem);
int32_t SPI_Uninitialize(const SPI_RESOURCES *spi, UDMA_RESOURCES *udma);
int32_t SPI_PowerControl(ARM_POWER_STATE state, const SPI_RESOURCES *spi);
int32_t SPI_Send(const void *data,
                 uint32_t num,
                 const SPI_RESOURCES *spi,
                 UDMA_RESOURCES *udma,
                 UDMA_Channel_Info *chnl_info,
                 RSI_UDMA_HANDLE_T udmaHandle);
int32_t SPI_Send_Command_Data(const void *data,
                              uint32_t num,
                              uint32_t instruction,
                              uint32_t address,
                              const SPI_RESOURCES *spi,
                              UDMA_RESOURCES *udma,
                              UDMA_Channel_Info *chnl_info,
                              RSI_UDMA_HANDLE_T udmaHandle);
int32_t SPI_Receive(void *data,
                    uint32_t num,
                    const SPI_RESOURCES *spi,
                    UDMA_RESOURCES *udma,
                    UDMA_Channel_Info *chnl_info,
                    RSI_UDMA_HANDLE_T udmaHandle);
int32_t SPI_Receive_Command_Data(void *data,
                                 uint32_t num,
                                 uint32_t instruction,
                                 uint32_t address,
                                 uint32_t wait_cycles,
                                 const SPI_RESOURCES *spi,
                                 UDMA_RESOURCES *udma,
                                 UDMA_Channel_Info *chnl_info,
                                 RSI_UDMA_HANDLE_T udmaHandle);
int32_t SPI_Transfer(const void *data_out,
                     void *data_in,
                     uint32_t num,
                     const SPI_RESOURCES *spi,
                     UDMA_RESOURCES *udma,
                     UDMA_Channel_Info *chnl_info,
                     RSI_UDMA_HANDLE_T udmaHandle);
uint32_t SPI_GetDataCount(const SPI_RESOURCES *spi);
int32_t SPI_Control(uint32_t control,
                    uint32_t arg,
                    const SPI_RESOURCES *spi,
                    uint32_t base_clock,
                    uint8_t spi_slavenumber);
ARM_SPI_STATUS SPI_GetStatus(const SPI_RESOURCES *spi);
void SPI_UDMA_Tx_Event(uint32_t event, uint8_t dmaCh, SPI_RESOURCES *spi);
void SPI_UDMA_Rx_Event(uint32_t event, uint8_t dmaCh, SPI_RESOURCES *spi);
void SPI_IRQHandler(const SPI_RESOURCES *spi);
uint32_t SSI_GetClockDivisionFactor(uint8_t ssi_instance);
uint32_t SSI_GetFrameLength(uint8_t ssi_instance);
int32_t SSI_SetFrameLength(uint8_t ssi_instance, uint8_t frame_length);
uint8_t SSI_GetInitState(uint8_t ssi_instance);
uint32_t SSI_GetTxCount(uint8_t ssi_instance);
void SSI_SetFifoThreshold(uint8_t ssi_instance);
void SSI_SetRxSamplingDelay(uint8_t ssi_instance, uint32_t sample_delay);
uint32_t SSI_GetReceiveSampleDelay(uint8_t ssi_instance);
uint32_t SSI_GetTxFifoThreshold(uint8_t ssi_instance);
uint32_t SSI_GetRxFifoThreshold(uint8_t ssi_instance);
#ifdef SSI_DUAL_QUAD_COMPONENT
int32_t SPI_Command_Configure(const SPI_RESOURCES *spi,
                              uint32_t inst_len,
                              uint32_t addr_len,
                              sl_ssi_frf_t spi_frf,
                              sl_ssi_xfer_type_t xfer_type);
int32_t SSI_Command_Configure(uint32_t inst_len, uint32_t addr_len, sl_ssi_frf_t spi_frf, sl_ssi_xfer_type_t xfer_type);
int32_t SPI_Command_Send(const SPI_RESOURCES *spi, uint32_t instruction, uint32_t address);
#endif
uint8_t RSI_SPI_GetSlaveSelectNumber(void);
void SPI_Clear_SSI_Enable_State(const SPI_RESOURCES *spi);
void SPI_Slave_Set_CS_Init_State(const SPI_RESOURCES *spi);
#ifdef SL_SI91X_SSI_DMA
void ssi_transfer_complete_callback(uint32_t channel, void *data);
void ssi_error_callback(uint32_t channel, void *data);
#endif

// Static Inline function to set the spi memory map pll
static inline void SSI_SetMemoryMapPll(uint16_t value)
{
  SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG9) = value;
}

#ifdef __cplusplus
}
#endif

#endif // RSI_SPI_H
