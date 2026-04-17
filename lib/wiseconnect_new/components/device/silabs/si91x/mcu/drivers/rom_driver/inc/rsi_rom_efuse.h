/******************************************************************************
* @file  rsi_rom_efuse.h
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

//Includes

#ifndef __RSI_ROM_EFUSE_H__
#define __RSI_ROM_EFUSE_H__

/**
 * \ingroup   RSI_SPECIFIC_DRIVERS
 * \defgroup EFUSE_DRIVER
 *  @{
 *
 */
#include "rsi_efuse.h"
#include "rsi_ccp_user_config.h"
#include "rsi_packing.h"
#if defined(A11_ROM)
#include "rsi_rom_table_si91x.h"
#else
#include "rsi_rom_table_RS1xxxx.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @fn           STATIC INLINE void  RSI_EFUSE_Enable(EFUSE_Type* pstcEfuse)
 * @brief        This API Is Used To Enable The EFUSE
 * @param[in]    pstcEfuse  :Pointer to the EFUSE register instance
 * @return       None
 */
STATIC INLINE void RSI_EFUSE_Enable(EFUSE_Type *pstcEfuse)
{
#if defined(EFUSE_ROMDRIVER_PRESENT) && !defined(SLI_SI917B0)
  ROMAPI_EFUSE_API->efuse_enable(pstcEfuse);
#else
  efuse_enable(pstcEfuse);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EFUSE_Disable(EFUSE_Type* pstcEfuse)
 * @brief        This API is used to Disable the EFUSE
 * @param[in]    pstcEfuse  :Pointer to the EFUSE register instance
 * @return       None
 */
STATIC INLINE void RSI_EFUSE_Disable(EFUSE_Type *pstcEfuse)
{
#if defined(EFUSE_ROMDRIVER_PRESENT) && !defined(SLI_SI917B0)
  ROMAPI_EFUSE_API->efuse_Disable(pstcEfuse);
#else
  efuse_Disable(pstcEfuse);
#endif
}

/**
 * @fn           STATIC INLINE uint8_t  RSI_EFUSE_ReadData(EFUSE_Type* pstcEfuse)
 * @brief        This API is used to read the eFUSE data
 * @param[in]    pstcEfuse   : Pointer to the EFUSE register instance
 * @return       eFUSE data
 */
STATIC INLINE uint8_t RSI_EFUSE_ReadData(EFUSE_Type *pstcEfuse)
{
#if defined(EFUSE_ROMDRIVER_PRESENT) && !defined(SLI_SI917B0)
  return ROMAPI_EFUSE_API->efuse_read_data(pstcEfuse);
#else
  return efuse_read_data(pstcEfuse);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EFUSE_WriteAddr(EFUSE_Type* pstcEfuse , uint16_t u16Addr)
 * @brief        This API is used to set the eFUSE address for read and write operations
 * @param[in]    pstcEfuse   : Pointer to the EFUSE register instance
 * @param[in]    u16Addr     : EFUSE address
 * @return       None
 */
STATIC INLINE void RSI_EFUSE_WriteAddr(EFUSE_Type *pstcEfuse, uint16_t u16Addr)
{
#if defined(EFUSE_ROMDRIVER_PRESENT) && !defined(SLI_SI917B0)
  ROMAPI_EFUSE_API->efuse_write_addr(pstcEfuse, u16Addr);
#else
  efuse_write_addr(pstcEfuse, u16Addr);
#endif
}

/**
 * @fn           STATIC INLINE rsi_error_t RSI_EFUSE_WriteBit(EFUSE_Type *pstcEfuse , uint16_t u16Addr ,
 *                                                         uint8_t u8BitPos,uint32_t hold_time)
 * @brief        Any Bit in this macro can be programmed in any order by raising STROBE high for aperiod specified
 *               by TPGM with a proper address selected.There is only one programming scheme,which is
 *               single bit programming
 * @param[in]    pstcEfuse   : Pointer to the EFUSE register instance
 * @param[in]    u16Addr     : Address of EFUSE to the data
 * @param[in]    u8BitPos    : position of bit to be written in 8 bit location in address
 * @param[in]    hold_time   : count value depends on clock frequency of EFUSE controller
 * @return       Nonzero     : If fails
 *               \n 0        : If success
 */
STATIC INLINE rsi_error_t RSI_EFUSE_WriteBit(EFUSE_Type *pstcEfuse,
                                             uint16_t u16Addr,
                                             uint8_t u8BitPos,
                                             uint32_t hold_time)
{
#if defined(EFUSE_ROMDRIVER_PRESENT) && !defined(SLI_SI917B0)
  return ROMAPI_EFUSE_API->efuse_write_bit(pstcEfuse, u16Addr, u8BitPos, hold_time);
#else
  return efuse_write_bit(pstcEfuse, u16Addr, u8BitPos, hold_time);
#endif
}

/**
 * @fn           STATIC INLINE rsi_error_t RSI_EFUSE_FsmReadByte(EFUSE_Type *pstcEfuse , uint16_t u16Addr ,
 *                                                            uint8_t *pu8Byte ,uint32_t SocClk)
 * @brief        This API is used to read the data from 32x8 byte eFUSE memory(OTP) in fsm mode
 * @param[in]    pstcEfuse  : Pointer to the EFUSE register instance
 * @param[in]    u16Addr    : byte address to be read
 * @param[out]   pu8Byte    : pointer to hold the read byte
 * @param[in]    SocClk     : clock frequency of EFUSE controller
 * @return       Non zero   : If fails
 *               \n 0       : If success
 */
STATIC INLINE rsi_error_t RSI_EFUSE_FsmReadByte(EFUSE_Type *pstcEfuse,
                                                uint16_t u16Addr,
                                                uint8_t *pu8Byte,
                                                uint32_t SocClk)
{
#if defined(EFUSE_ROMDRIVER_PRESENT) && !defined(SLI_SI917B0)
  return ROMAPI_EFUSE_API->efuse_fsm_read_byte(pstcEfuse, u16Addr, pu8Byte, SocClk);
#else
  return efuse_fsm_read_byte(pstcEfuse, u16Addr, pu8Byte, SocClk);
#endif
}

/**
 * @fn           STATIC INLINE rsi_error_t RSI_EFUSE_MemMapReadByte(EFUSE_Type *pstcEfuse , uint16_t u16Addr ,
 *                                                             uint8_t *pu8Byte ,uint32_t SocClk)
 * @brief        This API is used to read the data from 32x8 byte eFUSE memory(OTP) in memory mapped mode
 * @param[in]    pstcEfuse : Pointer to the EFUSE register instance
 * @param[in]    u16Addr   : byte address to be read
 * @param[out]   pu8Byte   : pointer to hold the read byte
 * @param[in]    SocClk    : clock frequency of EFUSE controller
 * @return       Non zero  : If fails
 *               \n 0      : If success
 */
STATIC INLINE rsi_error_t RSI_EFUSE_MemMapReadByte(EFUSE_Type *pstcEfuse,
                                                   uint16_t u16Addr,
                                                   uint8_t *pu8Byte,
                                                   uint32_t SocClk)
{
#if defined(EFUSE_ROMDRIVER_PRESENT) && !defined(SLI_SI917B0)
  return ROMAPI_EFUSE_API->efuse_mem_map_read_byte(pstcEfuse, u16Addr, pu8Byte, SocClk);
#else
  return efuse_mem_map_read_byte(pstcEfuse, u16Addr, pu8Byte, SocClk);
#endif
}

/**
 * @fn           STATIC INLINE rsi_error_t RSI_EFUSE_MemMapReadWord(EFUSE_Type *pstcEfuse , uint16_t u16Addr ,
 *                                                            uint16_t *pu16Word ,uint32_t SocClk)
 * @brief        This API is used to Read the 1 word(16 bits) of data to EFUSE macro
 * @param[in]    pstcEfuse : Pointer to the EFUSE register instance
 * @param[in]    u16Addr   : Address of EFUSE to the data
 * @param[out]   pu16Word  : pointer to the buffer to hold the read byte
 * @param[in]    SocClk    : clock frequency of EFUSE controller
 * @return       Non zero  : If fails
 *               \n 0      : If success
 */
STATIC INLINE rsi_error_t RSI_EFUSE_MemMapReadWord(EFUSE_Type *pstcEfuse,
                                                   uint16_t u16Addr,
                                                   uint16_t *pu16Word,
                                                   uint32_t SocClk)
{
#if defined(EFUSE_ROMDRIVER_PRESENT) && !defined(SLI_SI917B0)
  return ROMAPI_EFUSE_API->efuse_mem_map_read_word(pstcEfuse, u16Addr, pu16Word, SocClk);
#else
  return efuse_mem_map_read_word(pstcEfuse, u16Addr, pu16Word, SocClk);
#endif
}

#ifdef __cplusplus
}
#endif

#endif /*__RSI_ROM_EFUSE_H__*/

/* @} end of  RSI_EFUSE_DRIVER */