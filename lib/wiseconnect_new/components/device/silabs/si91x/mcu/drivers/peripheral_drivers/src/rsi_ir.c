/******************************************************************************
* @file  rsi_ir.c
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

// Include files

#include "rsi_ir.h"

/** @addtogroup SOC23
* @{
*/
/*==============================================*/
/**
 * @fn           uint16_t RSI_IR_ReadData(IR_Type* pIr,uint16_t memory_address)
 * @brief        This API is used read IR address.
 * @param[in]    pIr  : IR type pointer
 * @param[in]    memory_address      : memory address value (0 .. 128).
 * @return       16-Bit IR data received (BIT[15] in received data will indicate the polarity of pulse)
 *               \n remaining bit will contain the incremented counter value of the pulse.
 */
uint16_t RSI_IR_ReadData(IR_Type *pIr, uint16_t memory_address)
{
  if (memory_address > MAX_MEMORY_ADDRESS) {
    return INVALID_PARAMETERS;
  }
  pIr->IR_MEM_ADDR_ACCESS_b.IR_MEM_ADDR  = (unsigned int)(memory_address & 0x007F);
  pIr->IR_MEM_ADDR_ACCESS_b.IR_MEM_RD_EN = 1U;
  return pIr->IR_MEM_READ_b.IR_MEM_DATA_OUT;
}

/*==============================================*/
/**
 * @fn           void RSI_IR_SoftwareRestart(IR_Type* pIr)
 * @brief        This API restart the IR operation. 
 * @param[in]    pIr  : IR type pointer
 */
void RSI_IR_SoftwareRestart(IR_Type *pIr)
{
  uint8_t i;
  pIr->IR_CONFIG_b.SREST_IR_CORE = 1U;
  for (i = 0; i < 10; i++) {
    __ASM("nop");
  }
  pIr->IR_CONFIG_b.SREST_IR_CORE = 0U;
}
/** @} */