/*******************************************************************************
* @file  rsi_processor_sensor.h
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

#ifndef __RSI_PROCESSOR_SENSOR_H__
#define __RSI_PROCESSOR_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \ingroup   RSI_SPECIFIC_DRIVERS
 * \defgroup RSI_PROCESSOR_SENSOR RSI:RS1xxxx PROCESSOR_SENSOR 
 *  @{
 *
 */
#include "rsi_ccp_common.h"

rsi_error_t RSI_ProSense_Enable(MCU_ProcessSensor_Type *pstcProcessSensor, boolean_t bEN);
uint32_t RSI_ProSense_Read(MCU_ProcessSensor_Type *pstcProcessSensor);
uint32_t RSI_ProSense_GetNumCycles(MCU_ProcessSensor_Type *pstcProcessSensor);
rsi_error_t RSI_ProSense_ClkEnable(MCU_ProcessSensor_Type *pstcProcessSensor, boolean_t bEN);
rsi_error_t RSI_ProSense_RingClkStart(MCU_ProcessSensor_Type *pstcProcessSensor, boolean_t bEN);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /*__RSI_PROCESSOR_SENSOR_H__*/
