/******************************************************************************
* @file  rsi_i2s.h
* @brief This files contains functions prototypes related to i2s peripheral
* @section Description
* This file contains the list of function prototypes for the i2s and low level 
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

// Includes Files

#include "SAI.h"
#include "rsi_udma_wrapper.h"

#ifndef RSI_I2S_H
#define RSI_I2S_H

#ifdef __cplusplus
extern "C" {
#endif

//Prototypes
void RSI_I2S_InterruptEnableDisable(I2S_RESOURCES *i2s, boolean_t chnlNum, uint32_t maskFlag, boolean_t mask);
void I2S0_Chnl0_PinMux(I2S_RESOURCES *i2s);
void I2S0_Chnl1_PinMux(I2S_RESOURCES *i2s);
void I2S1_PinMux(I2S_RESOURCES *i2s);
void i2s_chnl_Init(ARM_SAI_SignalEvent_t cb_event, I2S_RESOURCES *i2s);

int32_t I2S_Initialize(ARM_SAI_SignalEvent_t cb_event,
                       I2S_RESOURCES *i2s,
                       UDMA_RESOURCES *udma,
                       RSI_UDMA_DESC_T *UDMA_Table,
                       RSI_UDMA_HANDLE_T *udmaHandle,
                       uint32_t *mem);
int32_t I2S_Uninitialize(I2S_RESOURCES *i2s, UDMA_RESOURCES *udma);
int32_t I2S_PowerControl(ARM_POWER_STATE state, I2S_RESOURCES *i2s, UDMA_RESOURCES *udma, RSI_UDMA_HANDLE_T udmaHandle);
int32_t I2S_Send(const void *data,
                 uint32_t num,
                 I2S_RESOURCES *i2s,
                 UDMA_RESOURCES *udma,
                 UDMA_Channel_Info *chnl_info,
                 RSI_UDMA_HANDLE_T udmaHandle);
int32_t I2S_Receive(void *data,
                    uint32_t num,
                    I2S_RESOURCES *i2s,
                    UDMA_RESOURCES *udma,
                    UDMA_Channel_Info *chnl_info,
                    RSI_UDMA_HANDLE_T udmaHandle);
uint32_t I2S_GetTxCount(I2S_RESOURCES *i2s);
uint32_t I2S_GetRxCount(I2S_RESOURCES *i2s);
int32_t I2S_Control(uint32_t control,
                    uint32_t arg1,
                    uint32_t arg2,
                    I2S_RESOURCES *i2s,
                    UDMA_RESOURCES *udma,
                    RSI_UDMA_HANDLE_T udmaHandle);
ARM_SAI_STATUS I2S_GetStatus(I2S_RESOURCES *i2s);
void I2S_IRQHandler(I2S_RESOURCES *i2s);
void I2S_UDMA_Tx_Event(uint32_t event, uint8_t dmaCh, I2S_RESOURCES *i2s);
void I2S_UDMA_Rx_Event(uint32_t event, uint8_t dmaCh, I2S_RESOURCES *i2s);
uint8_t I2S_GetInitState(uint8_t i2s_instance);

#ifdef __cplusplus
}
#endif

#endif // RSI_I2S_H
