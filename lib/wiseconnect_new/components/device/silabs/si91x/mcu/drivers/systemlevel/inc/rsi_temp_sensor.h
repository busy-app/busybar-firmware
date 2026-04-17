/*******************************************************************************
* @file  rsi_temp_sensor.h
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

/**
 * Includes
 */
#ifndef __RSI_TEMP_SENSOR_H__
#define __RSI_TEMP_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \ingroup   RSI_SPECIFIC_DRIVERS
 * \defgroup RSI_TEMP_SENSORS RSI:RS1xxxx TS
 *  @brief   
 *  @{
 *
 */
#include "rsi_ccp_common.h"

void RSI_TS_SetCntFreez(MCU_TEMP_Type *pstcTempSens, uint32_t u32CountFreez);
void RSI_TS_RefClkSel(MCU_TEMP_Type *pstcTempSens, boolean_t bRefClk);
void RSI_TS_Enable(MCU_TEMP_Type *pstcTempSens, boolean_t bEn);

/**
 * @}
 */
void RSI_TS_Config(MCU_TEMP_Type *pstcTempSens, uint32_t u32Nomial);
/**
 * \ingroup   RSI_SPECIFIC_DRIVERS
 * \defgroup RSI_TEMP_SENSORS RSI:RS1xxxx TEMPERARTURE SENSOR
 *  @brief   
 *  @{
 *
 */
uint32_t RSI_TS_ReadTemp(const MCU_TEMP_Type *pstcTempSens);
uint32_t RSI_TS_GetRefClkCnt(const MCU_TEMP_Type *pstcTempSens);
uint32_t RSI_TS_GetPtatClkCnt(const MCU_TEMP_Type *pstcTempSens);
void RSI_TS_LoadBjt(MCU_TEMP_Type *pstcTempSens, uint8_t temp);
void RSI_TS_RoBjtEnable(MCU_TEMP_Type *pstcTempSens, boolean_t enable);
void RSI_Periodic_TempUpdate(TIME_PERIOD_Type *temp, uint8_t enable, uint8_t trigger_time);

/**
 * @}
 */
#ifdef __cplusplus
}
#endif

/*End of file not truncated */
#endif /**__RSI_TEMP_SENSOR_H__*/

/* @}end of group RSI_TEMP_SENSORS */
