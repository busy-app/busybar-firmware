/******************************************************************************
* @file  rsi_efuse.h
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
#include "rsi_error.h"

#ifndef RSI_EFUSE_H
#define RSI_EFUSE_H

#ifdef __cplusplus
extern "C" {
#endif

#define TA_EFUSE_BASE_ADDR 0x40010000
#define M4_EFUSE_BASE_ADDR 0x4600C000

#define TA_EFUSE_IO_BASE_ADDR (TA_EFUSE_BASE_ADDR + 0x2000)
#define M4_EFUSE_IO_BASE_ADDR (M4_EFUSE_BASE_ADDR + 0x2000)

#define EFUSE_DA_ADDR_REG_OFFSET           0x00
#define EFUSE_DA_CTRL_SET_REG_OFFSET       0x04
#define EFUSE_DA_CTRL_CLEAR_REG_OFFSET     0x08
#define EFUSE_CTRL_REG_OFFSET              0x0C
#define EFUSE_READ_ADDR_REG_OFFSET         0x10
#define EFUSE_READ_DATA_REG_OFFSET         0x14
#define EFUSE_STATUS_REG_OFFSET            0x18
#define EFUSE_RD_TMNG_PARAM_REG_OFFSET     0x1C
#define EFUSE_DA_ADDR_REG_DEFAULT_OFFSET   0x2C
#define EFUSE_MEM_MAP_LENGTH_OFFSET        0x24
#define EFUSE_DA_CLR_STROBE_REG_OFFSET     0x34
#define EFUSE_CLK_ENABLE_SET_REG2_OFFSET   0x38
#define EFUSE_CLK_ENABLE_CLEAR_REG2_OFFSET 0x3C

#define TA_EFUSE_DA_ADDR_REG         (*((uint16_t volatile *)(TA_EFUSE_BASE_ADDR + EFUSE_DA_ADDR_REG_OFFSET)))
#define TA_EFUSE_DA_CTRL_SET_REG     (*((uint16_t volatile *)(TA_EFUSE_BASE_ADDR + EFUSE_DA_CTRL_SET_REG_OFFSET)))
#define TA_EFUSE_DA_CTRL_CLEAR_REG   (*((uint16_t volatile *)(TA_EFUSE_BASE_ADDR + EFUSE_DA_CTRL_CLEAR_REG_OFFSET)))
#define TA_EFUSE_CTRL_REG            (*((uint16_t volatile *)(TA_EFUSE_BASE_ADDR + EFUSE_CTRL_REG_OFFSET)))
#define TA_EFUSE_READ_ADDR_REG       (*((uint16_t volatile *)(TA_EFUSE_BASE_ADDR + EFUSE_READ_ADDR_REG_OFFSET)))
#define TA_EFUSE_READ_DATA_REG       (*((uint16_t volatile *)(TA_EFUSE_BASE_ADDR + EFUSE_READ_DATA_REG_OFFSET)))
#define TA_EFUSE_STATUS_REG          (*((uint16_t volatile *)(TA_EFUSE_BASE_ADDR + EFUSE_STATUS_REG_OFFSET)))
#define TA_EFUSE_RD_TMNG_PARAM_REG   (*((uint16_t volatile *)(TA_EFUSE_BASE_ADDR + EFUSE_RD_TMNG_PARAM_REG_OFFSET)))
#define TA_EFUSE_DA_ADDR_REG_DEFAULT (*((uint16_t volatile *)(TA_EFUSE_BASE_ADDR + EFUSE_DA_ADDR_REG_DEFAULT_OFFSET)))
#define TA_EFUSE_MEM_MAP_LENGTH      (*((uint16_t volatile *)(TA_EFUSE_BASE_ADDR + EFUSE_MEM_MAP_LENGTH_OFFSET)))
#define TA_EFUSE_DA_CLR_STROBE_REG   (*((uint16_t volatile *)(TA_EFUSE_BASE_ADDR + EFUSE_DA_CLR_STROBE_REG_OFFSET)))

#define M4_EFUSE_DA_ADDR_REG         (*((uint16_t volatile *)(M4_EFUSE_BASE_ADDR + EFUSE_DA_ADDR_REG_OFFSET)))
#define M4_EFUSE_DA_CTRL_SET_REG     (*((uint16_t volatile *)(M4_EFUSE_BASE_ADDR + EFUSE_DA_CTRL_SET_REG_OFFSET)))
#define M4_EFUSE_DA_CTRL_CLEAR_REG   (*((uint16_t volatile *)(M4_EFUSE_BASE_ADDR + EFUSE_DA_CTRL_CLEAR_REG_OFFSET)))
#define M4_EFUSE_CTRL_REG            (*((uint16_t volatile *)(M4_EFUSE_BASE_ADDR + EFUSE_CTRL_REG_OFFSET)))
#define M4_EFUSE_READ_ADDR_REG       (*((uint16_t volatile *)(M4_EFUSE_BASE_ADDR + EFUSE_READ_ADDR_REG_OFFSET)))
#define M4_EFUSE_READ_DATA_REG       (*((uint16_t volatile *)(M4_EFUSE_BASE_ADDR + EFUSE_READ_DATA_REG_OFFSET)))
#define M4_EFUSE_STATUS_REG          (*((uint16_t volatile *)(M4_EFUSE_BASE_ADDR + EFUSE_STATUS_REG_OFFSET)))
#define M4_EFUSE_RD_TMNG_PARAM_REG   (*((uint16_t volatile *)(M4_EFUSE_BASE_ADDR + EFUSE_RD_TMNG_PARAM_REG_OFFSET)))
#define M4_EFUSE_DA_ADDR_REG_DEFAULT (*((uint16_t volatile *)(M4_EFUSE_BASE_ADDR + EFUSE_DA_ADDR_REG_DEFAULT_OFFSET)))
#define M4_EFUSE_MEM_MAP_LENGTH      (*((uint16_t volatile *)(M4_EFUSE_BASE_ADDR + EFUSE_MEM_MAP_LENGTH_OFFSET)))
#define M4_EFUSE_DA_CLR_STROBE_REG   (*((uint16_t volatile *)(M4_EFUSE_BASE_ADDR + EFUSE_DA_CLR_STROBE_REG_OFFSET)))

#define EFUSE_CLK_BIT             BIT(5)
#define EFUSE_PCLK_BIT            BIT(19)
#define M4SS_CLK_ENABLE_SET_3_REG (*((volatile uint32_t *)(M4SS_CLK_PWR_CTRL_BASE_ADDR + 0x10)))

// EFUSE_DA_CTRL_SET_REG
#define SET_LOAD_ENABLE    BIT(3)
#define SET_CHIP_ENABLE    BIT(1)
#define SET_PROGRAM_ENABLE BIT(0)

#define READ_FSM_DONE BIT(15)

// Address range for 32x8 eFUSE for M4SS
#define ADDRESS_MIN 0x00 // Start address
#define ADDRESS_MAX 0x31 // End address

// Bit Positions range
#define BIT_POS_MIN 0x00 // Min Bit position
#define BIT_POS_MAX 0x07 // Max bit position

void efuse_enable(EFUSE_Type *pstcEfuse);

void efuse_Disable(EFUSE_Type *pstcEfuse);

uint8_t efuse_read_data(EFUSE_Type *pstcEfuse);

void efuse_write_addr(EFUSE_Type *pstcEfuse, uint16_t u16Addr);

rsi_error_t efuse_get_addr(EFUSE_Type *pstcEfuse, uint16_t *u16AddrVal);

rsi_error_t efuse_write_bit(EFUSE_Type *pstcEfuse, uint16_t u16Addr, uint8_t u8BitPos, uint32_t hold_time);

rsi_error_t efuse_fsm_read_byte(EFUSE_Type *pstcEfuse, uint16_t u16Addr, uint8_t *pu8Byte, uint32_t SocClk);

rsi_error_t efuse_mem_map_read_byte(EFUSE_Type *pstcEfuse, uint16_t u16Addr, uint8_t *pu8Byte, uint32_t SocClk);

rsi_error_t efuse_mem_map_read_word(EFUSE_Type *pstcEfuse, uint16_t u16Addr, uint16_t *pu16Word, uint32_t SocClk);

rsi_error_t efuse_direct_read_byte(EFUSE_Type *pstcEfuse, uint16_t u16Addr, uint8_t *pu8Byte, uint32_t hold_time);

void efuse_write_address(EFUSE_Type *pstcEfuse, uint16_t u16Addr);

#ifdef __cplusplus
}
#endif

#endif // RSI_EFUSE_H
