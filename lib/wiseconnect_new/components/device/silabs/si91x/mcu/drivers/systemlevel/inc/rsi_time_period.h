/*******************************************************************************
* @file  rsi_time_period.h
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

#ifndef __RSI_TIME_PERIOD_H__
#define __RSI_TIME_PERIOD_H__
#include "rsi_ccp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/**
  * @brief Structure to hold the calibration configuration parameters.
  */
typedef struct {
  uint32_t u32TimePeriodRefClk;  ///< Reference clock
  uint32_t u32XtalSettle;        ///< Clock settling time
  uint16_t u16ClkCnt;            ///< No. of clocks required for calibration
  boolean_t bPeriodicCalibEn;    ///< Periodic calibration enable
  uint8_t u8PeriodicCalibRate;   ///< Calibration periodic rate
  boolean_t bTemperatureCalibEn; ///< Temperature based calibration enable
  uint8_t u8TemperatureVal;      ///< Temperature value
  uint8_t u8AverageFactor;       ///< Average factor
} rsi_timeperiod_calib_config_t;

rsi_error_t RSI_TIMEPERIOD_TimerClkSel(TIME_PERIOD_Type *pstcTimePeriod, uint32_t u32TimePeriod);
rsi_error_t RSI_TIMEPERIOD_RCCalibration(TIME_PERIOD_Type *pstcTimePeriod,
                                         const rsi_timeperiod_calib_config_t *rc_calib_config);
uint32_t RSI_TIMEPERIOD_RCCalibTimePeriodRead(const TIME_PERIOD_Type *pstcTimePeriod);
uint32_t RSI_TIMEPERIOD_ROCalibTimePeriodRead(const TIME_PERIOD_Type *pstcTimePeriod);
rsi_error_t RSI_TIMEPERIOD_XTAL32KHzCalibration(TIME_PERIOD_Type *pstcTimePeriod,
                                                const rsi_timeperiod_calib_config_t *xtal32khz_calib_config);
rsi_error_t RSI_TIMEPERIOD_ROCalibration(TIME_PERIOD_Type *pstcTimePeriod,
                                         const rsi_timeperiod_calib_config_t *ro_calib_config);
rsi_error_t RSI_TIMEPERIOD_LowPwrTrigSelEn(TIME_PERIOD_Type *pstcTimePeriod, boolean_t bEn);
rsi_error_t RSI_TIMEPERIOD_VbatTrigSel(TIME_PERIOD_Type *pstcTimePeriod, uint8_t u8Time);

#ifdef __cplusplus
}
#endif

/*End of file not truncated*/
#endif /*__RSI_TIME_PERIOD_H__*/
