/******************************************************************************
* @file  rsi_processor_sensor.c
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
#include "rsi_error.h"
#include "si91x_device.h"

/*==============================================*/
/** 
 * @fn        rsi_error_t RSI_ProSense_Enable(MCU_ProcessSensor_Type *pstcProcessSensor, boolean_t bEN)
 * @brief     This API is used to Enable / Disable the process sensor
 * @param[in] pstcProcessSensor : pointer to the processor sensor register instance
 * @param[in] bEN : to Enable / Disble of process sensor
 *            0: Disable process sensor
 *            1: Enable process sensor
 * @return    RSI_OK on success
 *            Error code on failure
 */

rsi_error_t RSI_ProSense_Enable(MCU_ProcessSensor_Type *pstcProcessSensor, boolean_t bEN)
{
  pstcProcessSensor->PROCESS_SENSOR_ENABLE_AND_READ_b.PROCESS_SENSOR_EN = (unsigned int)(bEN & 0x01);
  return RSI_OK;
}

/*==============================================*/
/** 
 * @fn        uint32_t RSI_ProSense_Read(MCU_ProcessSensor_Type *pstcProcessSensor)
 * @brief     This API is used to read the PS count from the process sensor
 * @param[in] pstcProcessSensor : pointer to the processor sensor register instance
 * @return    returns the PS count values
 */

uint32_t RSI_ProSense_Read(MCU_ProcessSensor_Type *pstcProcessSensor)
{
  return pstcProcessSensor->PROCESS_SENSOR_ENABLE_AND_READ_b.PS_COUNT;
}

/*==============================================*/
/** 
 * @fn        uint32_t RSI_ProSense_GetNumCycles(MCU_ProcessSensor_Type *pstcProcessSensor)
 * @brief     This API is used to read the Num cycle count from the process sensor
 * @param[in] pstcProcessSensor : pointer to the processor sensor register instance
 * @return    returns the PS count values
 */

uint32_t RSI_ProSense_GetNumCycles(MCU_ProcessSensor_Type *pstcProcessSensor)
{
  return pstcProcessSensor->PROCESS_SENSOR_ENABLE_AND_READ_b.NUM_CYCLES;
}

/*==============================================*/
/** 
 * @fn         rsi_error_t RSI_ProSense_ClkEnable(MCU_ProcessSensor_Type *pstcProcessSensor, boolean_t bEN)
 * @brief      This API is used to Enable / Disable the process sensor clock
 * @param[in]  pstcProcessSensor : pointer to the processor sensor register instance
 * @param[in]  bEN : to Enable / Disble of process sensor
 *             0: Disable process sensor clock
 *             1: Enable process sensor clock
 * @return     RSI_OK on success
 *             Error code on failure
 */

rsi_error_t RSI_ProSense_ClkEnable(MCU_ProcessSensor_Type *pstcProcessSensor, boolean_t bEN)
{
  if (bEN) {
    pstcProcessSensor->PROCESS_SENSOR_ENABLE_AND_READ_b.PS_CLK_SW_ON = 1;
  } else {
    pstcProcessSensor->PROCESS_SENSOR_ENABLE_AND_READ_b.PS_CLK_SW_OFF = 1;
  }
  return RSI_OK;
}

/*==============================================*/
/** 
 * @fn        rsi_error_t RSI_ProSense_RingClkStart(MCU_ProcessSensor_Type *pstcProcessSensor, boolean_t bEN)
 * @brief     This API is used to start / Stop the Ring-Oscillator clock for estimating process corner.
 * @param[in] pstcProcessSensor  : pointer to the processor sensor register instance
 * @param[in] bEN  : to Enable / Disble of process sensor
 *            0: STOP  Ring-Oscillator clock for estimating process corner.
 *            1: START Ring-Oscillator clock for estimating process corner.
 * @return    RSI_OK on success
 *            Error code on failure
 */

rsi_error_t RSI_ProSense_RingClkStart(MCU_ProcessSensor_Type *pstcProcessSensor, boolean_t bEN)
{
  pstcProcessSensor->PROCESS_SENSOR_ENABLE_AND_READ_b.PS_RING_CLK_START = (unsigned int)(bEN & 0x01);
  return RSI_OK;
}
