/***************************************************************************/ /**
* @file  rsi_crc.h
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

#include "rsi_ccp_common.h"
#include "base_types.h"

#ifndef RSI_CRC_H
#define RSI_CRC_H

#ifdef __cplusplus
extern "C" {
#endif

#define DIN_WIDTH_32 31
#define DIN_WIDTH_16 15
#define DIN_WIDTH_8  7

// brief: Structure of CRC Calculation
typedef struct {
  uint32_t crc;        //Calculated CRC Value
  uint32_t polynomial; //Polynomial Value for CRC Calculation
  unsigned int
    polyWidth : 5;  //Number of bits/width of the polynomial has to be written here for the computation of final CRC
  uint32_t lfsrVal; //lfsr Initialization value for CRC Calculation
  unsigned int widthType : 3; //Data Width taken Variable.
                              //When width_type - 0 :Take the data width from either reg programmed or from cnt
                              //width_type -1 :Take the data width from Reg.
                              //width_type-2 : Take the data width from CNT.
  unsigned int dinWidth : 5;  //Valid number of bits in the input data in din_width_from_reg set mode
  uint32_t numBytes;          //Input data number of bytes
  unsigned int aempty : 4;    //Almost empty Threshold value. Max value is 15
  unsigned int afull : 4;     //Almost Full Threshold value. Max value is 15
  uint32_t InputData;
  uint32_t swapLfsr;
  uint32_t swapDin;
  uint32_t useUdma;
  uint32_t inputSize;
  uint32_t dataIn;
  uint32_t bitWidth;
  uint32_t lfsrState;
} RSI_CRC_PARAMS_T;

// CRC FUNCTION PROTOTYPES
void crc_set_gen_control(CRC_Type *pCRC);

uint32_t crc_get_gen_status(CRC_Type *pCRC);

void crc_polynomial(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams);

uint32_t crc_polynomial_width(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams);

void crc_lfsr_init(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams);

uint32_t crc_use_swapped_init(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams);

uint32_t crc_set_data_width_type(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams);

uint32_t crc_set_fifo_thresholds(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams);

uint32_t crc_write_data(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams, uint32_t data);

uint32_t monitor_crc_calc(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams);

void crc_lfsr_dynamic_write(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams);

void crc_reset_fifo(CRC_Type *pCRC);

uint32_t crc_get_fifo_status(CRC_Type *pCRC);
#ifdef __cplusplus
}
#endif

#endif // RSI_CRC_H
