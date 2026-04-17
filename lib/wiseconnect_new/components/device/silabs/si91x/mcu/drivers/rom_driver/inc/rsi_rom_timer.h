/******************************************************************************
* @file  rsi_rom_timer.h
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

#ifndef __RSI_ROM_TIMER_H__
#define __RSI_ROM_TIMER_H__

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
 * \ingroup   RSI_SPECIFIC_DRIVERS
 * \defgroup TIMER
 *  @{
 *
 */

/**
 * @fn          STATIC INLINE rsi_error_t  RSI_TIMERS_MicroSecTimerConfig(RSI_TIMERS_T *pTIMER, uint8_t timerNum,
 *																				uint16_t integer,uint8_t fractional,uint8_t mode)
 * @brief		    This API is used to configure timer mode as a 1 micro second or 256 micro second mode
 * @param[in]		pTIMER			: Pointer to the TIMERS instance register area
 * @param[in]   timerNum    : Timer number(0 to 3)
 * @param[in]   integer     : Integer part of number of clock cycles
 *														\n per microsecond of the system clock being used
 * @param[in]   fractional  : Fractional part of clock cycles per microsecond of the system clock being used
 * @param[in]   mode        : timer mode
 *                               - \ref MICRO_SEC_MODE
 *                               - \ref _256_MICRO_SEC_MODE
 *                               - \ref COUNTER_DOWN_MODE
 * @return 		  - \ref TIMERS_INVAL_TIMER_MODE       : If timer mode is invalid
 *              - \ref TIMERS_INVAL_TIMER_NUM_ERROR  : If timer number is invalid
 *              - \ref RSI_OK                        : If process is done
 */
STATIC INLINE rsi_error_t RSI_TIMERS_MicroSecTimerConfig(RSI_TIMERS_T *pTIMER,
                                                         uint8_t timerNum,
                                                         uint16_t integer,
                                                         uint8_t fractional,
                                                         uint8_t mode)
{
#if defined(TIMER_ROMDRIVER_PRESENT)
  return ROMAPI_TIMER_API->timers_microsec_timer_config(pTIMER, timerNum, integer, fractional, mode);
#else
  return timers_microsec_timer_config(pTIMER, timerNum, integer, fractional, mode);
#endif
}

/**
 * @fn          STATIC INLINE uint32_t  RSI_TIMERS_ReadTimer(RSI_TIMERS_T *pTIMER, uint8_t timerNum,boolean_t countDir)
 * @brief		    This API is used to get the count of the required timer
 * @param[in]   pTIMER     : Pointer to the TIMERS instance register area
 * @param[in]   timerNum   : Timer number(0 to 3)
 * @param[in]   countDir   : For reading/tracking counter in up counting this bit has to be set.
 * @return 		  Timer count value
 */
STATIC INLINE uint32_t RSI_TIMERS_ReadTimer(RSI_TIMERS_T *pTIMER, uint8_t timerNum, boolean_t countDir)
{
#if defined(TIMER_ROMDRIVER_PRESENT)
  return ROMAPI_TIMER_API->timers_read_timer(pTIMER, timerNum, countDir);
#else
  return timers_read_timer(pTIMER, timerNum, countDir);
#endif
}

//prototypes
rsi_error_t RSI_TIMERS_MicroSecTimerConfig(RSI_TIMERS_T *pTIMER,
                                           uint8_t timerNum,
                                           uint16_t integer,
                                           uint8_t fractional,
                                           uint8_t mode);

uint32_t RSI_TIMERS_ReadTimer(RSI_TIMERS_T *pTIMER, uint8_t timerNum, boolean_t countDir);

#ifdef __cplusplus
}
#endif

#endif /* __RSI_ROM_TIMER_H__ */

/* @} end of RSI_TIMER  */
