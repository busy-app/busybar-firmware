/******************************************************************************
* @file  rsi_rom_ct.h
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

#ifndef __RSI_ROM_CT_H__
#define __RSI_ROM_CT_H__

/**
 * \ingroup   RSI_SPECIFIC_DRIVERS
 * \defgroup CT_DRIVER
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
 * @fn           STATIC INLINE void  RSI_CT_OCUHighLowToggleSelect(RSI_CT_T *pCT, boolean_t lowHigh,
																		   boolean_t counterNum, uint8_t outputSel)
 * @brief		     This API is used to toggle output state of OCU.
 * @param[in]	   pCT         : Pointer to the CT instance register area
 * @param[in]	   lowHigh     : Output low/high state
 * @param[in]    counterNum  : Counter 0/1
 * @param[in]    outputSel   : OCU possibilities(0 to 7)
 * @return 		   none
 */
STATIC INLINE void RSI_CT_OCUHighLowToggleSelect(RSI_CT_T *pCT,
                                                 boolean_t lowHigh,
                                                 boolean_t counterNum,
                                                 uint8_t outputSel)
{
#if defined(CT_ROMDRIVER_PRESENT)
  ROMAPI_CT_API->ct_ocu_high_Low_toggle_select(pCT, lowHigh, counterNum, outputSel);
#else
  ct_ocu_high_Low_toggle_select(pCT, lowHigh, counterNum, outputSel);
#endif
}
/**
 * @fn          STATIC INLINE rsi_error_t RSI_CT_WFGControlConfig( RSI_CT_T *pCT, WFG_PARAMS_T ctrlReg )
 * @brief		    This API is used to toggle output state of WFG
 * @param[in]	  pCT         : Pointer to the CT instance register area
 * @param[in]	  ctrlReg     : Variable to a structure of type \ref WFG_PARAMS_T
 * @return 		  \ref ERROR_CT_INVALID_ARG : If WFG_PARAMS_T structure parameters are invalid
 *              - \n \ref RSI_OK            : If process is done successfully
 */
STATIC INLINE rsi_error_t RSI_CT_WFGControlConfig(RSI_CT_T *pCT, WFG_PARAMS_T ctrlReg)
{
#if defined(CT_ROMDRIVER_PRESENT)
  return ROMAPI_CT_API->ct_wfg_control_config(pCT, ctrlReg);
#else
  return ct_wfg_control_config(pCT, ctrlReg);
#endif
}
/**
 * @fn          STATIC INLINE rsi_error_t RSI_CT_OCUControl(RSI_CT_T *pCT, boolean_t counterNum,boolean_t dmaEn,
                                          OCU_PARAMS_T *pOCUparams,RSI_CT_CALLBACK_T *pCB)
 * @brief		    This API controls OCU operation
 * @param[in]   pCT        	 : Pointer to the CT instance register area
 * @param[in]   counterNum   : Counter 0/1
 * @param[in]   dmaEn		     : DMA enable flag
                                - \ref 0 : not enable DMA.
                                - \ref 1 : DMA enable.
 * @param[in]   pOCUparams   : Pointer to the OCU parameters structure and its members are as below \ref OCU_PARAMS_T
 *                              - \ref  CompareVal1_0       : Threshold one of Counter 0,possible values  (0x0 to 0xFFFF)
 *                              - \ref  CompareVal2_0 	    : Threshold two of Counter 0,possible values (0x0 to 0xFFFF)
 *                              - \ref  CompareVal1_1 	    : Threshold two of Counter 1,possible values (0x0 to 0xFFFF)
 *                              - \ref  CompareVal2_1 	    : Threshold two of Counter 1,possible values (0x0 to 0xFFFF)
 *                              - \ref  CompareVal1_0 	    : Threshold two of Counter 0,possible values (0x0 to 0xFFFF)
 *                              - \ref  CompareNextVal2_0 	: next compare value of counter 0,possible values (0x0 to 0xFFFF)
 *                              - \ref  CompareNextVal1_1 	: next compare value of counter 1,possible values (0x0 to 0xFFFF)
 *                              - \ref  CompareNextVal2_1 	: next compare value of counter 1,possible values (0x0 to 0xFFFF)
 *                              - \ref  SyncValue         	: Holds the starting point of counters for synchronization purpose
 *                                                          This is logical OR of sync values for counter 0& 1,
 *                                                          possible values for two counters (0x0 to 0xFFFF).
 * @param[in]   pCB          : Pointer to CT callback structure \ref RSI_CT_CALLBACK_T
 * @return 	    \ref ERROR_CT_INVALID_ARG : If passed parameters are invalid
 *              \n \ref RSI_OK            : If process is done successfully
 */
STATIC INLINE rsi_error_t RSI_CT_OCUControl(RSI_CT_T *pCT,
                                            boolean_t counterNum,
                                            boolean_t dmaEn,
                                            OCU_PARAMS_T *pOCUparams,
                                            RSI_CT_CALLBACK_T *pCB)
{
#if defined(CT_ROMDRIVER_PRESENT)
  return ROMAPI_CT_API->ct_ocu_control(pCT, counterNum, dmaEn, pOCUparams, pCB);
#else
  return ct_ocu_control(pCT, counterNum, dmaEn, pOCUparams, pCB);
#endif
}
/**
 * @fn           STATIC INLINE rsi_error_t RSI_CT_WFGComapreValueSet( RSI_CT_T *pCT,
                                                    boolean_t counterNum,
																	                  OCU_PARAMS_T *pOCUparams)
 * @brief		     This function controls WFG operation
 * @param[in]	   pCT          : Pointer to the CT instance register area
 * @param[in]    counterNum   : Counter 0/1
 * @param[in]    pOCUparams   : Pointer to the OCU parameters structure and its members are as below \ref OCU_PARAMS_T
 *									               - \ref  CompareVal1_0   : Threshold one of Counter 0,possible values  (0x0 to 0xFFFF)
 *									               - \ref  CompareVal2_0   : Threshold two of Counter 0,possible values  (0x0 to 0xFFFF)
 *									               - \ref  CompareVal1_1   : Threshold one of Counter 1,possible values  (0x0 to 0xFFFF)
 *									               - \ref  CompareVal2_1   : Threshold one of Counter 1,possible values  (0x0 to 0xFFFF)
 *                                 - \ref  SyncValue       : Holds the starting point of counters for synchronization purpose
 *			                                                    This is logical OR of sync values for counter 0& 1
 *															                            possible values for two counters (0x0 to 0xFFFF)
 * @return 		  \ref ERROR_CT_INVALID_ARG : If passed parameters are invalid
                \n \ref RSI_OK            : If process is done successfully
 */
STATIC INLINE rsi_error_t RSI_CT_WFGComapreValueSet(RSI_CT_T *pCT, boolean_t counterNum, OCU_PARAMS_T *pOCUparams)
{
#if defined(CT_ROMDRIVER_PRESENT)
  return ROMAPI_CT_API->ct_wfg_comapre_value_set(pCT, counterNum, pOCUparams);
#else
  return ct_wfg_comapre_value_set(pCT, counterNum, pOCUparams);
#endif
}

#ifdef __cplusplus
}
#endif

#endif /*__RSI_ROM_CT_H__*/

/* @} end of  RSI_CT_DRIVER */
