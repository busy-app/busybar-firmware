/******************************************************************************
* @file  rsi_i2c.h
* @brief This files contains functions prototypes related to I2C peripheral
* @section Description
* This file contains the list of function prototypes for the i2c and low level 
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

#include "I2C.h"

#ifndef RSI_I2C_H
#define RSI_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

//prototypes
void I2C_EnableDisableInterrupt(I2C_RESOURCES *i2c, uint32_t flag, uint32_t intFlags);
int32_t I2Cx_Initialize(ARM_I2C_SignalEvent_t cb_event, I2C_RESOURCES *i2c);
int32_t I2Cx_Uninitialize(I2C_RESOURCES *i2c);
int32_t I2Cx_PowerControl(ARM_POWER_STATE state, I2C_RESOURCES *i2c);
int32_t I2Cx_MasterTransmit(uint32_t addr, const uint8_t *data, uint32_t num, bool xfer_pending, I2C_RESOURCES *i2c);
int32_t I2Cx_MasterReceive(uint32_t addr, uint8_t *data, uint32_t num, bool xfer_pending, I2C_RESOURCES *i2c);

int32_t I2Cx_SlaveTransmit(const uint8_t *data, uint32_t num, I2C_RESOURCES *i2c);
int32_t I2Cx_SlaveReceive(uint8_t *data, uint32_t num, I2C_RESOURCES *i2c);
int32_t I2Cx_GetDataCount(I2C_RESOURCES *i2c);
uint32_t GetI2CClockFreq(I2C_RESOURCES *i2c);
int32_t I2Cx_Control(uint32_t control, uint32_t arg, I2C_RESOURCES *i2c, uint32_t clock);
ARM_I2C_STATUS I2Cx_GetStatus(I2C_RESOURCES *i2c);
uint32_t I2Cx_MasterHandler(I2C_RESOURCES *i2c);
uint32_t I2Cx_SlaveHandler(I2C_RESOURCES *i2c);
void I2Cx_IRQHandler(I2C_RESOURCES *i2c);
int32_t SetClockRate(I2C_RESOURCES *i2c, uint8_t speed, uint32_t icClk);

#ifdef __cplusplus
}
#endif

#endif // RSI_I2C_H
