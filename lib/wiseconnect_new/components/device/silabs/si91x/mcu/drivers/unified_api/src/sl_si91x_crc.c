/******************************************************************************
* @file sl_si91x_crc.c
* @brief CRC API implementation
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
#include "data_types.h"
#include "clock_update.h"
#include "rsi_bod.h"
#include "rsi_rom_crc.h"
#include "rsi_rom_clks.h"
#include "sl_si91x_crc.h"
#include "rsi_rom_crc.h"
/**
 * @brief CRC instance.
 */
#define pCRC CRC
/*******************************************************************************
 * @fn           sl_status_t sl_si91x_crc_set_config(sl_crc_params_t *params)
 * @brief        This API is used to set the CRC configuration parameters.
 *               It initializes the CRC module with the provided parameters and
 *               returns the calculated CRC value.
 *******************************************************************************/
sl_status_t sl_si91x_crc_set_config(sl_crc_params_t *params)
{
  sl_status_t status  = SL_STATUS_OK;
  uint32_t reg_status = 0;

  // Reset all state machines to IDLE state
  status = sl_si91x_crc_set_gen_control();
  if (status != SL_STATUS_OK) {
    return SL_STATUS_INVALID_STATE;
  }
  // Enable CRC peripheral clock
  status = sl_si91x_crc_enable();
  if (status != SL_STATUS_OK) {
    return SL_STATUS_NOT_READY;
  }
  // Configure CRC polynomial value
  status = sl_si91x_crc_polynomial(params);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_INITIALIZATION;
  }
  // Set width of the polynomial
  status = sl_si91x_crc_polynomial_width(params, &reg_status);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  // Initialize LFSR with initial value
  status = sl_si91x_crc_lfsr_init(params);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  // Configure bit-swapped initialization if needed
  status = sl_si91x_crc_use_swapped_init(params, &reg_status);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_NOT_AVAILABLE;
  }
  // Set data width types for CRC calculation
  status = sl_si91x_crc_set_data_width_type(params, &reg_status);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_BUSY;
  }
  // Configure FIFO threshold levels
  status = sl_si91x_crc_set_fifo_thresholds(params, &reg_status);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_ABORT;
  }
  return SL_STATUS_OK;
}

/*******************************************************************************
 * @fn           sl_status_t sl_si91x_crc_set_gen_control(
 * uint32_t* reg_status)
 * @brief        This API is used to clear the FIFO and reset all the
 * state machines to their IDLE.
 *******************************************************************************/
sl_status_t sl_si91x_crc_set_gen_control(void)
{
  sl_status_t status = SL_STATUS_OK;
  if (pCRC != NULL)
    RSI_CRC_SetGenControl(pCRC);
  else
    status = SL_STATUS_NULL_POINTER;
  return status;
}
/*******************************************************************************
 * @fn           sl_status_t sl_si91x_crc_get_general_status(
 * uint32_t* reg_status)
 * @brief        This API is used to get the general status of the crc signals.
 *******************************************************************************/
sl_status_t sl_si91x_crc_get_general_status(uint32_t *reg_status)
{
  sl_status_t status = SL_STATUS_OK;
  if ((pCRC != NULL) && (reg_status != NULL)) {
    *reg_status = RSI_CRC_GetGenStatus(pCRC);
  } else
    status = SL_STATUS_NULL_POINTER;
  return status;
}

/*******************************************************************************
 * @fn           sl_status_t sl_si91x_crc_polynomial(,
 *                                   sl_crc_params_t *pCRCParams);
 * @brief        This API is used to load the polynomial value into the
 *polynomial reg.
 *******************************************************************************/
sl_status_t sl_si91x_crc_polynomial(sl_crc_params_t *pCRCParams)
{
  sl_status_t status = SL_STATUS_OK;
  if (pCRC != NULL && pCRCParams != NULL) {
    RSI_CRC_Polynomial(pCRC, pCRCParams);
  } else {
    status = SL_STATUS_NULL_POINTER;
  }
  return status;
}

/*******************************************************************************
 * @fn           sl_status_t sl_si91x_crc_polynomial_width(
 *                                        sl_crc_params_t *pCRCParams,
 *                                        uint32_t* reg_status)
 * @brief        This API is used to load the width of the polynomial. Number of
 *bits/width of the polynomial has to be written here for the computation of
 *final CRC. If a new width has to be configured, clear the existing length
 *first by writing 0x1f in polynomial_ctrl_reset register.
 *******************************************************************************/
sl_status_t sl_si91x_crc_polynomial_width(sl_crc_params_t *pCRCParams, uint32_t *reg_status)
{
  sl_status_t status = SL_STATUS_OK;
  if (pCRC != NULL && pCRCParams != NULL)
    *reg_status = RSI_CRC_Polynomial_Width(pCRC, pCRCParams);
  else
    status = SL_STATUS_NULL_POINTER;
  return status;
}
/*******************************************************************************
 * @fn           sl_status_t sl_si91x_crc_lfsr_init(
 *                                 sl_crc_params_t *pCRCParams)
 * @brief        This API is used to initialize the  LFSR Value. When ever the
 * LFSR need to be Initialized this has to be updated with the Init value.
 ********************************************************************************/
sl_status_t sl_si91x_crc_lfsr_init(sl_crc_params_t *pCRCParams)
{
  sl_status_t status = SL_STATUS_OK;
  if (pCRC != NULL && pCRCParams != NULL)
    RSI_CRC_LfsrInit(pCRC, pCRCParams);
  else
    status = SL_STATUS_NULL_POINTER;
  return status;
}
/*******************************************************************************
 * @fn           sl_status_t sl_si91x_crc_use_swapped_init(sl_crc_params_t *pCRCParams,
 *                                         uint32_t* reg_status)
 * @brief        This API is used for swapped init value. If this is set bit
 * swapped version of LFSR init value will be loaded / initialized to LFSR state.
 *******************************************************************************/
sl_status_t sl_si91x_crc_use_swapped_init(sl_crc_params_t *pCRCParams, uint32_t *reg_status)
{
  sl_status_t status = SL_STATUS_OK;
  if ((pCRC != NULL) && (pCRCParams != NULL) && (reg_status != NULL)) {
    *reg_status = RSI_CRC_Use_Swapped_Init(pCRC, pCRCParams);
  } else {
    status = SL_STATUS_NULL_POINTER;
  }
  return status;
}
/*******************************************************************************
 * @fn           sl_status_t sl_si91x_crc_set_data_width_type(
 *                                            sl_crc_params_t *pCRCParams,
 *                                            uint32_t oper_status)
 * @brief        This API is used to set and control the data width types.
 *******************************************************************************/
sl_status_t sl_si91x_crc_set_data_width_type(sl_crc_params_t *pCRCParams, uint32_t *oper_status)
{
  sl_status_t status = SL_STATUS_OK;
  if ((pCRC != NULL) && (pCRCParams != NULL) && (oper_status != NULL)) {
    *oper_status = RSI_CRC_Set_DataWidthType(pCRC, pCRCParams);
  } else {
    status = SL_STATUS_NULL_POINTER;
  }
  return status;
}
/*******************************************************************************
 * @fn           sl_status_t sl_si91x_crc_set_fifo_thresholds(sl_crc_params_t *pCRCParams,
 *                                            uint32_t oper_status)
 * @brief        This API is used to set the FIFO Threshold Levels.
 *******************************************************************************/
sl_status_t sl_si91x_crc_set_fifo_thresholds(sl_crc_params_t *pCRCParams, uint32_t *oper_status)
{
  sl_status_t status = SL_STATUS_OK;
  if ((pCRC != NULL) && (pCRCParams != NULL) && (oper_status != NULL)) {
    *oper_status = RSI_CRC_SetFifoThresholds(pCRC, pCRCParams);
  } else {
    status = SL_STATUS_NULL_POINTER;
  }

  return status;
}

/*******************************************************************************
 * @fn           sl_status_t sl_si91x_crc_write_data(sl_crc_params_t *pCRCParams,
 *                                   uint32_t data,
 *                                   uint32_t oper_status)
 * @brief        This API is used to set the Input data Information for CRC
 * Calculation.
 *******************************************************************************/
sl_status_t sl_si91x_crc_write_data(sl_crc_params_t *pCRCParams, uint32_t data, uint32_t *oper_status)
{
  sl_status_t status = SL_STATUS_OK;
  if (pCRC != NULL && pCRCParams != NULL && oper_status != NULL) {
    *oper_status = RSI_CRC_WriteData(pCRC, pCRCParams, data);
  } else {
    status = SL_STATUS_NULL_POINTER;
  }
  return status;
}
/*******************************************************************************
 * @fn           sl_status_t sl_si91x_crc_monitor_crc_calc(
 *                                         sl_crc_params_t *pCRCParams,
 *                                         uint32_t crc)
 * @brief        This API is used to monitor the CRC Calculation status and the
 * returns the CRC Value.
 *******************************************************************************/
sl_status_t sl_si91x_crc_monitor_crc_calc(sl_crc_params_t *pCRCParams, uint32_t *crc)
{
  sl_status_t status = SL_STATUS_OK;
  if (pCRC != NULL && pCRCParams != NULL && crc != NULL) {
    *crc = RSI_Monitor_CRCcalc(pCRC, pCRCParams);
  } else {
    status = SL_STATUS_NULL_POINTER;
  }
  return status;
}
/*******************************************************************************
 * @fn           sl_status_t sl_si91x_crc_lfsr_dynamic_write(
 *                                                  sl_crc_params_t *pCRCParams)
 * @brief        This API can be used for writing the LFSR state directly.
*******************************************************************************/
sl_status_t sl_si91x_crc_lfsr_dynamic_write(sl_crc_params_t *pCRCParams)
{
  sl_status_t status = SL_STATUS_OK;
  if (pCRC != NULL && pCRCParams != NULL)
    RSI_CRC_LfsrDynamicWrite(pCRC, pCRCParams);
  else
    status = SL_STATUS_NULL_POINTER;
  return status;
}
/*******************************************************************************
 * @fn           sl_status_t sl_si91x_crc_reset_fifo(void)
 * @brief        This API is used to Reset fifo pointer. This clears the
 *FIFO.When this is set
 * @return       None
 *******************************************************************************/
sl_status_t sl_si91x_crc_reset_fifo(void)
{
  sl_status_t status = SL_STATUS_OK;
  if (pCRC != NULL) {
    RSI_CRC_ResetFifo(pCRC);
  } else {
    status = SL_STATUS_NULL_POINTER;
  }
  return status;
}
/*******************************************************************************
 * @fn           sl_status_t sl_si91x_crc_get_fifo_status(
 *                                        uint32_t* reg_status)
 * @brief        This API is used to get the FIFO status of the CRC occupancy.
 *******************************************************************************/
sl_status_t sl_si91x_crc_get_fifo_status(uint32_t *reg_status)
{
  sl_status_t status = SL_STATUS_OK;
  if ((pCRC != NULL) && (reg_status != NULL)) {
    *reg_status = RSI_CRC_GetFifoStatus(pCRC);
  } else {
    status = SL_STATUS_NULL_POINTER;
  }
  return status;
}
/*******************************************************************************
 * @fn sl_status_t sl_si91x_crc_enable(void)
 * @brief Enables the CRC peripheral.
 *
 * This function enables the CRC peripheral by enabling its clock.
 ******************************************************************************/
sl_status_t sl_si91x_crc_enable(void)
{
  sl_status_t status = SL_STATUS_OK;
  status             = RSI_CLK_PeripheralClkEnable1(M4CLK, CRC_CLK_ENABLE_M4);
  return status;
}

/*******************************************************************************
 * @fn sl_status_t sl_si91x_crc_disable(void)
 * @brief Disables the CRC peripheral.
 *
 * This function disable the CRC peripheral by disabling its clock.
 ******************************************************************************/
sl_status_t sl_si91x_crc_disable(void)
{
  sl_status_t status = SL_STATUS_OK;
  status             = RSI_CLK_PeripheralClkDisable1(M4CLK, CRC_CLK_ENABLE_M4);
  return status;
}
