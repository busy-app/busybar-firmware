/******************************************************************************
* @file  rsi_crc.c
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

#include "rsi_ccp_user_config.h"

#ifndef CRC_ROMDRIVER_PRESENT
#include "rsi_rom_crc.h"
#include "si91x_device.h"

/*==============================================*/
/**
 * @fn           void crc_set_gen_control(CRC_Type *pCRC)
 * @brief        This API is used to clears the FIFO and settles all the state machines to their IDLE.
 * @param[in]    pCRC : Pointer to the CRC Register set instance area.
 * @return       None
 */
void crc_set_gen_control(CRC_Type *pCRC)
{
  pCRC->CRC_GEN_CTRL_SET_REG = BIT(0);
}

/*==============================================*/
/**
 * @fn           uint32_t crc_get_gen_status(CRC_Type *pCRC)
 * @brief        This API is used to get the general status of the crc signals.
 * @param[in]    pCRC : Pointer to the CRC Register set instance area.
 * @return       Returns Crc status signals.
 */

uint32_t crc_get_gen_status(CRC_Type *pCRC)
{
  return pCRC->CRC_GEN_STS;
}

/*==============================================*/

/**
 * @fn           void crc_polynomial(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
 * @brief        This API is used to load the polynomial value into the polynomial reg.
 * @param[in]    pCRC       : Pointer to the CRC Register set instance area.
 * @param[in]    pCRCParams : Pointer to the CRC Parameters structure.
 *                            The possible variable is polynomial.
 * @return       None
 */
void crc_polynomial(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
{
  pCRC->CRC_POLYNOMIAL = pCRCParams->polynomial;
}

/*==============================================*/
/**
 * @fn           uint32_t crc_polynomial_width(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
 * @brief        This API is used to load the width of the polynomail .Number of bits/width of the polynomial
 *               has to be written here for the computation of final CRC. If a new width has to be configured,
 *               clear the existing length first by writing 0x1f in polynomial_ctrl_reset register.
 * @param[in]    pCRC       : Pointer to the CRC Register set instance area.
 * @param[in]    pCRCParams : Pointer to the CRC Parameters structure.
 *                            The possible variable is Poly_width.
 * @return       None
 */
uint32_t crc_polynomial_width(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
{
  if (pCRCParams->polyWidth <= 32) {
    pCRC->CRC_POLYNOMIAL_CTRL_RESET = 0x1F;
    pCRC->CRC_POLYNOMIAL_CTRL_SET   = pCRCParams->polyWidth & 0x1F;
  } else {
    return ERROR_CRC_INVALID_ARG;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           void crc_lfsr_init(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
 * @brief        This API is used to initialize the  LFSR Value. When ever the LFSR need to be Initilized
 *               this has to be updated with the Init value.
 * @param[in]    pCRC       : Pointer to the CRC Register set instance area.
 * @param[in]    pCRCParams : Pointer to the CRC Parameters structure.
 *                            The possible variable is lfsr_val.
 * @return       None
 */
void crc_lfsr_init(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
{
  pCRC->CRC_LFSR_INIT_CTRL_SET = BIT(0);

  pCRC->CRC_LFSR_INIT_VAL = pCRCParams->lfsrVal;

  pCRC->CRC_LFSR_INIT_CTRL_SET = BIT(1);
}

/*==============================================*/
/**
 * @fn           uint32_t crc_use_swapped_init(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
 * @brief        This API is used for swapped init value. If this is set bit swapped
 *               version of LFSR init value will be loaded / initialized to LFSR state.
 * @param[in]    pCRC       : Pointer to the CRC Register set instance area.
 * @param[in]    pCRCParams : Pointer to the CRC Parameters structure.
 * @return       return \ref RSI_OK on success and failure return error code.
 */
uint32_t crc_use_swapped_init(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
{
  if (pCRCParams->swapLfsr == 0 || pCRCParams->swapLfsr == 1) {
    if (pCRCParams->swapLfsr) {
      pCRC->CRC_LFSR_INIT_CTRL_RESET = BIT(2);
      pCRC->CRC_LFSR_INIT_CTRL_SET   = BIT(2);
    } else {
      pCRC->CRC_LFSR_INIT_CTRL_RESET = BIT(2);
    }
  } else {
    return ERROR_CRC_INVALID_ARG;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           uint32_t crc_set_data_width_type(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
 * @brief        This API is used to set and control the data width types.
 * @param[in]    pCRC       : Pointer to the CRC Register set instance area.
 * @param[in]    pCRCParams : Pointer to the CRC Parameters structure.
 *							  The possible parameter is num_bytes. When only the data width type is DIN_WIDTH_FROM_CNT
 *                            then only uses this variable.
 * @return       return \ref RSI_OK on success and failure return error code.
 */
uint32_t crc_set_data_width_type(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
{
  if (pCRCParams->widthType <= 2) {
    switch (pCRCParams->widthType) {
      case 0: //Width From ULI
        /* Set DIN_CTRL_RESET [din_width_from_reg] and DIN_CTR5L_RESET [din_width_from_cnt]
     	   to get the engine into this ULI width mode */

        pCRC->CRC_DIN_CTRL_RESET_REG = BIT(5);
        pCRC->CRC_DIN_CTRL_RESET_REG = BIT(6);
        break;
      case 1: //Width from REG
        /* Set DIN_CTRL_SET [din_width_from_reg] and DIN_CTRL_RESET [din_width_from_cnt]
		   to get the engine into this Width from reg mode*/

        pCRC->CRC_DIN_CTRL_SET       = BIT(5);
        pCRC->CRC_DIN_CTRL_RESET_REG = BIT(6);
        // In this mode data length also programmed in din_width reg.
        pCRC->CRC_DIN_CTRL_RESET_REG = 0x1F;
        pCRC->CRC_DIN_CTRL_SET       = (pCRCParams->dinWidth & 0x1F);
        break;
      case 2: //Width from CNT
        /* Set DIN_CTRL_SET [din_width_from_cnt] and DIN_CTRL_RESET [din_width_from_reg]
			    to get the engine into this Width from cnt mode*/
        pCRC->CRC_DIN_CTRL_SET       = BIT(6);
        pCRC->CRC_DIN_CTRL_RESET_REG = BIT(5);
        // Write the number of bytes that the packet/data has in total into DIN_NUM_BYTES register.
        pCRC->CRC_DIN_NUM_BYTES = pCRCParams->numBytes;
        break;
      default:
        pCRC->CRC_DIN_CTRL_RESET_REG = BIT(6);
        pCRC->CRC_DIN_CTRL_RESET_REG = BIT(5);
        break;
    }
  } else {
    return ERROR_CRC_INVALID_ARG;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           uint32_t crc_set_fifo_thresholds(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
 * @brief        This API is used to set the FIFO Threshold Levels.
 * @param[in]    pCRC       : Pointer to the CRC Register set instance area.
 * @param[in]    pCRCParams : Pointer to the CRC Parameters structure.
 * 							 The possible parameters are aempty and afull.
 *               Max value is 15 for both the threshold levels.
 * @return       return \ref RSI_OK on success and failure return error code.*
 */
uint32_t crc_set_fifo_thresholds(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
{
  if (pCRCParams->aempty <= 16) {
    // Before Set the FIFO Thresholds Reset the Threshold levels.
    pCRC->CRC_DIN_CTRL_RESET_REG = 0xFF000000;

    // Write the Almost empty threshold value
    pCRC->CRC_DIN_CTRL_SET = ((uint32_t)(pCRCParams->aempty & 0xF)) << 24;

    // Write the Almost full Threshold level value
    pCRC->CRC_DIN_CTRL_SET = ((uint32_t)(pCRCParams->afull & 0xF)) << 28;
  } else {
    return ERROR_CRC_INVALID_ARG;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           uint32_t crc_write_data(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams, uint32_t data)
 * @brief        This API is used to set the Input data Information for CRC Calculation.
 * @param[in]    pCRC       : Pointer to the CRC Register set instance area.
 * @param[in]    pCRCParams : Pointer to the CRC Parameters structure.
 *							 The possible parameters are aempty and afull.
 * @return       return \ref RSI_OK on success and failure return error code.
 */
uint32_t crc_write_data(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams, uint32_t data)
{
  if (pCRCParams->swapDin == 0 || pCRCParams->swapDin == 1) {
    if (pCRCParams->swapDin) {
      pCRC->CRC_DIN_CTRL_SET = BIT(7);
    } else {
      pCRC->CRC_DIN_CTRL_RESET_REG = BIT(7);
    }
    // Wait for the FIFO occupancy is less than AFULL Threshold
    while ((pCRC->CRC_DIN_STS & BIT(2)) == BIT(2))
      ;

    if (pCRCParams->dinWidth == DIN_WIDTH_32) {
      pCRC->CRC_DIN_FIFO = (uint32_t)data;
    }
    if (pCRCParams->dinWidth == DIN_WIDTH_16) {
      pCRC->CRC_DIN_FIFO = (uint16_t)data;
    }
    if (pCRCParams->dinWidth == DIN_WIDTH_8) {
      pCRC->CRC_DIN_FIFO = (uint8_t)data;
    }
  } else {
    return ERROR_CRC_INVALID_ARG;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           uint32_t monitor_crc_calc(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
 * @brief        This API is used to monitor the CRC Caluclation status and the returns the CRC Value.
 * @param[in]    pCRC       : Pointer to the CRC Register set instance area.
 * @param[in]    pCRCParams : Pointer to the CRC Parameters structure.
 *		                      The possible parameter is width_type. This Params always 2 when we want
 *                            Max value is 15 for both the threshold levels.
 * @return       CRC Value to be returned.
 */
uint32_t monitor_crc_calc(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
{
  uint32_t crc = 0;

  if (pCRCParams->widthType == 2) {
    // Wait for the no.of bytes done.
    while ((pCRC->CRC_GEN_STS & 0x02) != 0x02)
      ;
  }
  // Wait for the CRC Cal Done
  while ((pCRC->CRC_GEN_STS & 0x01) != 0x01)
    ;

  // Read LFSR_STATE for final CRC
  crc = pCRC->CRC_LFSR_STATE;

  // Soft reset
  pCRC->CRC_GEN_CTRL_SET_REG = BIT(0);

  return crc;
}

/*==============================================*/
/**
 * @fn           void crc_lfsr_dynamic_write(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams )
 * @brief        This API can be used for writing the LFSR state directly.
 * @param[in]    pCRC - Pointer to the CRC Register set instance area.
 * @param[in]    pCRCParams - Pointer to the CRC Parameters structure.
                 The possible variable is lfsrState.
 * @return       None
 */
void crc_lfsr_dynamic_write(CRC_Type *pCRC, RSI_CRC_PARAMS_T *pCRCParams)
{
  pCRC->CRC_LFSR_STATE = pCRCParams->lfsrState;
}

/*==============================================*/
/**
 * @fn           void crc_reset_fifo(CRC_Type *pCRC)
 * @brief        This API is used to Reset fifo pointer. This clears the FIFO.When this is set
 * @param[in]    pCRC : Pointer to the CRC Register set instance area.
 * @return       None
 */
void crc_reset_fifo(CRC_Type *pCRC)
{
  pCRC->CRC_DIN_CTRL_SET = BIT(8);
}

/*==============================================*/
/**
 * @fn           uint32_t crc_get_fifo_status(CRC_Type *pCRC)
 * @brief        This API is used to get the fifo status of the crc occupancy.
 * @param[in]    pCRC : Pointer to the CRC Register set instance area.
 * @return       Returns Crc Fifo status signals.
 */
uint32_t crc_get_fifo_status(CRC_Type *pCRC)
{
  return pCRC->CRC_DIN_STS;
}

/*ROM API Structure
#if defined( A11_ROM ) && !defined (CRC_ROMDRIVER_PRESENT)
const ROM_CRC_API_T crc_api = {
		&crc_set_gen_control,
		&crc_get_gen_status,
		&crc_polynomial,
		&crc_polynomial_width,
		&crc_lfsr_init,
		&crc_use_swapped_init,
		&crc_set_data_width_type,
		&crc_set_fifo_thresholds,
		&crc_write_data,
		&monitor_crc_calc,
		&crc_lfsr_dynamic_write,
		&crc_reset_fifo,
		&crc_get_fifo_status
};
#endif
*/
#endif //CRC_ROMDRIVER_PRESENT
