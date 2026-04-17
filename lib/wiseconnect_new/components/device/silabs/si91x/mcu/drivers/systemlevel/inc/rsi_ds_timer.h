/******************************************************************************
* @file  rsi_ds_timer.h
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
 *
 */

/**
 * \ingroup   RSI_SPECIFIC_DRIVERS
 * \defgroup  RSI_DS_TIMER_DRIVERS  RSI:RSIxxxx DS TIMER 
 *  @brief    
 *  @{
 *
 */
#ifndef __RSI_DS_TIMER_H__
#define __RSI_DS_TIMER_H__

#include "rsi_error.h"
#include "base_types.h"
#include "rsi_power_save.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief	   This API is used to  configure the deep sleep duration of sleep timer
 * @param[in]      dsDuration : sleep duration count in micro seconds
 * @return 	   none
 */
STATIC INLINE void RSI_DST_DurationSet(uint32_t dsDuration)
{
  MCU_FSM->MCU_FSM_DEEP_SLEEP_DURATION_LSB_REG = dsDuration;
}

/**
 * @brief          This API is used to  configure the deep sleep duration of sleep timer
 * @return         none
 */
STATIC INLINE void RSI_DST_IntrUnMask(void)
{
  NPSS_INTR_MASK_CLR_REG = NPSS_TO_MCU_WAKEUP_INTR;
}

/**
 * @brief          This API is used to  mask the Deep Sleep Timer interrupt
 * @return         none
 */
STATIC INLINE void RSI_DST_IntrMask(void)
{
  NPSS_INTR_MASK_SET_REG = NPSS_TO_MCU_WAKEUP_INTR;
}

/**
 * @brief          This API is used to  configure the deep sleep duration of sleep timer
 * @return         none
 */
STATIC INLINE void RSI_DST_TimerIntrClear(void)
{
  NPSS_INTR_CLEAR_REG                = NPSS_TO_MCU_WAKEUP_INTR;
  MCU_FSM->MCU_FSM_WAKEUP_STATUS_REG = 1;
}

#ifdef __cplusplus
}
#endif

#endif /*__RSI_DS_TIMER_H__*/

/*  @} end of RSI_DS_TIMER_DRIVERS */
