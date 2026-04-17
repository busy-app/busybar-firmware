/******************************************************************************
* @file  rsi_rom_crc.h
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

#ifndef __RSI_ROM_CRC_H__
#define __RSI_ROM_CRC_H__

/**
 * \ingroup   RSI_SPECIFIC_DRIVERS
 * \defgroup CRC_DRIVER
 *  @{
 *
 */
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
 * @fn           STATIC INLINE void  RSI_CRC_SetGenControl(CRC_Type *pCRC)
 * @brief        This API is used to clears the FIFO and settles all the state machines to their IDLE.
 * @param[in]    pCRC : Pointer to the CRC Register set instance area.
 * @return       None
 */
STATIC INLINE void RSI_CRC_SetGenControl(CRC_Type *pCRC)
{
#if defined(CRC_ROMDRIVER_PRESENT)
  ROMAPI_CRC_API->crc_set_gen_control(pCRC);
#else
  crc_set_gen_control(pCRC);
#endif
}

/**
 * @fn           STATIC INLINE uint32_t  RSI_CRC_GetGenStatus(CRC_Type *pCRC)
 * @brief        This API is used to get the general status of the crc signals.
 * @param[in]    pCRC : Pointer to the CRC Register set instance area.
 * @return       Returns Crc status signals.
 */
STATIC INLINE uint32_t RSI_CRC_GetGenStatus(CRC_Type *pCRC)
{
#if defined(CRC_ROMDRIVER_PRESENT)
  return ROMAPI_CRC_API->crc_get_gen_status(pCRC);
#else
  return crc_get_gen_status(pCRC);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_CRC_Polynomial(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams )
 * @brief        This API is used to load the polynomial value into the polynomial reg.
 * @param[in]    pCRC       : Pointer to the CRC Register set instance area.
 * @param[in]    pCRCParams : Pointer to the CRC Parameters structure.
 *                            The possible variable is polynomial.
 * @return       None
 */
STATIC INLINE void RSI_CRC_Polynomial(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
{
#if defined(CRC_ROMDRIVER_PRESENT)
  ROMAPI_CRC_API->crc_polynomial(pCRC, pCRCParams);
#else
  crc_polynomial(pCRC, pCRCParams);
#endif
}

/**
 * @fn           STATIC INLINE uint32_t  RSI_CRC_Polynomial_Width(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams )
 * @brief        This API is used to load the width of the polynomail .Number of bits/width of the polynomial
 *               has to be written here for the computation of final CRC. If a new width has to be configured,
 *               clear the existing length first by writing 0x1f in polynomial_ctrl_reset register.
 *
 * @param[in]    pCRC       : Pointer to the CRC Register set instance area.
 * @param[in]    pCRCParams : Pointer to the CRC Parameters structure.
 *                            The possible variable is Poly_width.
 * @return       None
 */
STATIC INLINE uint32_t RSI_CRC_Polynomial_Width(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
{
#if defined(CRC_ROMDRIVER_PRESENT)
  return ROMAPI_CRC_API->crc_polynomial_width(pCRC, pCRCParams);
#else
  return crc_polynomial_width(pCRC, pCRCParams);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_CRC_LfsrInit(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams )
 * @brief        This API is used to initialize the  LFSR Value. When ever the LFSR need to be Initilized
 *               this has to be updated with the Init value.
 * @param[in]    pCRC       : Pointer to the CRC Register set instance area.
 * @param[in]    pCRCParams : Pointer to the CRC Parameters structure.
 *                            The possible variable is lfsr_val.
 * @return       None
 */
STATIC INLINE void RSI_CRC_LfsrInit(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
{
#if defined(CRC_ROMDRIVER_PRESENT)
  ROMAPI_CRC_API->crc_lfsr_init(pCRC, pCRCParams);
#else
  crc_lfsr_init(pCRC, pCRCParams);
#endif
}

/**
 * @fn           STATIC INLINE uint32_t  RSI_CRC_Use_Swapped_Init( CRC_Type *pCRC ,RSI_CRC_PARAMS_T *pCRCParams)
 * @brief        This API is used for swapped init value. If this is set bit swapped
 *               version of LFSR init value will be loaded / initialized to LFSR state.
 * @param[in]    pCRC       : Pointer to the CRC Register set instance area.
 * @param[in]    pCRCParams : Pointer to the CRC Parameters structure.
 * @return       return \ref RSI_OK on success and failure return error code.
 */
STATIC INLINE uint32_t RSI_CRC_Use_Swapped_Init(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
{
#if defined(CRC_ROMDRIVER_PRESENT)
  return ROMAPI_CRC_API->crc_use_swapped_init(pCRC, pCRCParams);
#else
  return crc_use_swapped_init(pCRC, pCRCParams);
#endif
}

/**
 * @fn           STATIC INLINE uint32_t  RSI_CRC_Set_DataWidthType(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams )
 * @brief        This API is used to set and control the data width types.
 * @param[in]    pCRC       : Pointer to the CRC Register set instance area.
 * @param[in]    pCRCParams : Pointer to the CRC Parameters structure.
 *								             The possible parameter is num_bytes. When only the data width type is DIN_WIDTH_FROM_CNT
 *                            then only uses this variable.
 * @return       return \ref RSI_OK on success and failure return error code.
 */
STATIC INLINE uint32_t RSI_CRC_Set_DataWidthType(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
{
#if defined(CRC_ROMDRIVER_PRESENT)
  return ROMAPI_CRC_API->crc_set_data_width_type(pCRC, pCRCParams);
#else
  return crc_set_data_width_type(pCRC, pCRCParams);
#endif
}

/**
 * @fn           STATIC INLINE uint32_t  RSI_CRC_SetFifoThresholds(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
 * @brief        This API is used to set the FIFO Threshold Levels.
 * @param[in]    pCRC       : Pointer to the CRC Register set instance area.
 * @param[in]    pCRCParams : Pointer to the CRC Parameters structure.
 * 							 The possible parameters are aempty and afull.
 *               Max value is 15 for both the threshold levels.
 * @return       return \ref RSI_OK on success and failure return error code.*
 */
STATIC INLINE uint32_t RSI_CRC_SetFifoThresholds(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
{
#if defined(CRC_ROMDRIVER_PRESENT)
  return ROMAPI_CRC_API->crc_set_fifo_thresholds(pCRC, pCRCParams);
#else
  return crc_set_fifo_thresholds(pCRC, pCRCParams);
#endif
}

/**
 * @fn           STATIC INLINE uint32_t  RSI_CRC_WriteData( CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
 * @brief        This API is used to set the Input data Information for CRC Calculation.
 * @param[in]    pCRC       : Pointer to the CRC Register set instance area.
 * @param[in]    pCRCParams : Pointer to the CRC Parameters structure.
 *							 The possible parameters are aempty and afull.
 * @return       return \ref RSI_OK on success and failure return error code.
 */
STATIC INLINE uint32_t RSI_CRC_WriteData(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams, uint32_t data)
{
#if defined(CRC_ROMDRIVER_PRESENT)
  return ROMAPI_CRC_API->crc_write_data(pCRC, pCRCParams, data);
#else
  return crc_write_data(pCRC, pCRCParams, data);
#endif
}

/**
 * @fn           STATIC INLINE uint32_t  RSI_Monitor_CRCcalc(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
 * @brief        This API is used to monitor the CRC Caluclation status and the returns the CRC Value.
 * @param[in]    pCRC       : Pointer to the CRC Register set instance area.
 * @param[in]    pCRCParams : Pointer to the CRC Parameters structure.
 *		                        The possible parameter is width_type. This Params always 2 when we want
 *                            Max value is 15 for both the threshold levels.
 * @return       CRC Value to be returned.
 */
STATIC INLINE uint32_t RSI_Monitor_CRCcalc(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
{
#if defined(CRC_ROMDRIVER_PRESENT)
  return ROMAPI_CRC_API->monitor_crc_calc(pCRC, pCRCParams);
#else
  return monitor_crc_calc(pCRC, pCRCParams);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_CRC_LfsrDynamicWrite(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams )
 * @brief        This API can be used for writing the LFSR state directly.
 * @param[in]    pCRC       : Pointer to the CRC Register set instance area.
 * @param[in]    pCRCParams : Pointer to the CRC Parameters structure.
                 The possible variable is lfsrState.
 * @return       None
 */
STATIC INLINE void RSI_CRC_LfsrDynamicWrite(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
{
#if defined(CRC_ROMDRIVER_PRESENT)
  ROMAPI_CRC_API->crc_lfsr_dynamic_write(pCRC, pCRCParams);
#else
  crc_lfsr_dynamic_write(pCRC, pCRCParams);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_CRC_ResetFifo(CRC_Type *pCRC)
 * @brief        This API is used to Reset fifo pointer. This clears the FIFO.When this is set
 * @param[in]    pCRC : Pointer to the CRC Register set instance area.
 * @return       None
 */
STATIC INLINE void RSI_CRC_ResetFifo(CRC_Type *pCRC)
{
#if defined(CRC_ROMDRIVER_PRESENT)
  ROMAPI_CRC_API->crc_reset_fifo(pCRC);
#else
  crc_reset_fifo(pCRC);
#endif
}

/**
 * @fn           STATIC INLINE uint32_t  RSI_CRC_GetFifoStatus(CRC_Type *pCRC)
 * @brief        This API is used to get the fifo status of the crc occupancy.
 * @param[in]    pCRC : Pointer to the CRC Register set instance area.
 * @return       Returns Crc Fifo status signals.
 */
STATIC INLINE uint32_t RSI_CRC_GetFifoStatus(CRC_Type *pCRC)
{
#if defined(CRC_ROMDRIVER_PRESENT)
  return ROMAPI_CRC_API->crc_get_fifo_status(pCRC);
#else
  return crc_get_fifo_status(pCRC);
#endif
}
#ifdef __cplusplus
}
#endif

#endif /*__RSI_ROM_CRC_H__*/

/* @} end of  RSI_CRC_DRIVER */
