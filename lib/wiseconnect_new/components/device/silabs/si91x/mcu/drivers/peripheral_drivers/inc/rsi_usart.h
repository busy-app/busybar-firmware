/******************************************************************************
* @file  rsi_usart.h
* @brief This files contains functions prototypes related to USART peripheral
* @section Description :
* This file contains the list of function prototypes for the usart and low level 
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

#include "USART.h"
#include "rsi_udma_wrapper.h"

#ifndef RSI_USART_H
#define RSI_USART_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { USART_0, UART_1, ULPUART, UARTLAST } usart_peripheral_t;

// prototypes
// prototypes
void UartIrqHandler(USART_RESOURCES *usart);
void USART_UDMA_Tx_Event(uint32_t event, uint8_t dmaCh, USART_RESOURCES *usart);
void USART_UDMA_Rx_Event(uint32_t event, uint8_t dmaCh, USART_RESOURCES *usart);
int32_t USART_SetBaudrate(uint32_t baudrate, uint32_t baseClk, USART_RESOURCES *usart);
int32_t USART_Initialize(ARM_USART_SignalEvent_t cb_event,
                         USART_RESOURCES *usart,
                         const UDMA_RESOURCES *udma,
                         RSI_UDMA_DESC_T *UDMA_Table,
                         RSI_UDMA_HANDLE_T *udmaHandle,
                         uint32_t *mem);
int32_t USART_Uninitialize(const USART_RESOURCES *usart, const UDMA_RESOURCES *udma);
int32_t USART_PowerControl(ARM_POWER_STATE state,
                           USART_RESOURCES *usart,
                           const UDMA_RESOURCES *udma,
                           RSI_UDMA_HANDLE_T udmaHandle);
int32_t USART_Send_Data(const void *data,
                        uint32_t num,
                        USART_RESOURCES *usart,
                        const UDMA_RESOURCES *udma,
                        UDMA_Channel_Info *chnl_info,
                        RSI_UDMA_HANDLE_T udmaHandle);
int32_t USART_Receive_Data(const void *data,
                           uint32_t num,
                           USART_RESOURCES *usart,
                           const UDMA_RESOURCES *udma,
                           UDMA_Channel_Info *chnl_info,
                           RSI_UDMA_HANDLE_T udmaHandle);
int32_t USART_Transfer(const void *data_out,
                       const void *data_in,
                       uint32_t num,
                       USART_RESOURCES *usart,
                       const UDMA_RESOURCES *udma,
                       UDMA_Channel_Info *chnl_info,
                       RSI_UDMA_HANDLE_T udmaHandle);
uint32_t USART_GetTxCount(const USART_RESOURCES *usart);
uint32_t USART_GetRxCount(const USART_RESOURCES *usart);
int32_t USART_Control(uint32_t control,
                      uint32_t arg,
                      uint32_t baseClk,
                      USART_RESOURCES *usart,
                      const UDMA_RESOURCES *udma,
                      RSI_UDMA_HANDLE_T udmaHandle);
ARM_USART_STATUS USART_GetStatus(const USART_RESOURCES *usart);
int32_t USART_SetModemControl(ARM_USART_MODEM_CONTROL control, USART_RESOURCES *usart);
ARM_USART_MODEM_STATUS USART_GetModemStatus(const USART_RESOURCES *usart);
uint32_t USART_GetParity_StopBit(uint8_t usart_peripheral); //API to get the parity and stop bit info
uint32_t USART_GetBaudrate(uint8_t usart_peripheral);       //API to get the USART/UART baudrate
uint8_t USART_GetInitState(uint8_t usart_peripheral);       //Get the USART Initialized state
uint8_t USART_GetMode(uint8_t usart_peripheral);            //API to get the USART/UART mode
#ifdef SL_SI91X_USART_DMA
void usart_transfer_complete_callback(uint32_t channel, void *data);
void usart_error_callback(uint32_t channel, void *data);
#endif
#ifdef __cplusplus
}
#endif

#endif // RSI_USART_H
