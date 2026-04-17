/******************************************************************************
* @file  rsi_time_period.c
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

#include "rsi_time_period.h"
#include "rsi_power_save.h"

// by using these macros, we program the RTC timer clock in SOC
// MSB 8-bits for the Integer part &c
// LSB 17-bits for the Fractional part
// Ex: 32kHz clock = 31.25us ==> 31.25*2^17 = 4096000 = 0x3E8000
// Ex: 32.768kHz clock = 30.51us ==> 30.51*2^17 = 4000000 = 0x3D0900
#define RTC_TIME_PERIOD_32000HZ (0x003E7FFFUL) // Time period value for 32kHz
#define RTC_TIME_PERIOD_32768HZ (0x003D08FFUL) // Time period value for 32.768kHz

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif // UNUSED_PARAMETER

/**
  * \ingroup   RSI_SPECIFIC_DRIVERS
  * \defgroup RSI_TIME_PERIOD  RSI:RS1xxxx TIME PERIOD 
  *  @{
  *
  */

/*==============================================*/
/**
  * @fn        rsi_error_t RSI_TIMEPERIOD_RCCalibration(TIME_PERIOD_Type *pstcTimePeriod,
  *                                    const rsi_timeperiod_calib_config_t rc_calib_config)
  * @brief     This API is calibrate the RC
  * @param[in] pstcTimePeriod       : pointer to the timeperiod calibration registration instance
  * @param[in] rc_calib_config      : Pointer to calibration configuration structure (rsi_timeperiod_calib_config_t)
  *            u32TimePeriodRefClk  : timeperiod of reference clk with each bit corresponding to 10ps granularity
  *            u32XtalSettle        : no of 32khz clocks for xtal 40mhz clk to settle
  *            u16ClkCnt            : 2^no_of_rc_clocks = no of rc clocks used in calibration
  *            bPeriodicCalibEn     : to enable the periodic calibration
  *              1 : periodic calibration mode enable
  *              0 : periodic calibration mode disable
  *            u8PeriodicCalibRate : periodic calibration rate
  *              0 : Every 30 secs
  *              1 : every 15 secs,
  *              2 : every 10 secs,
  *              3 : every 5  secs
  *            bTemperatureCalibEn : to enable the temperature based calibration
  *              1 : Temperature based calibration enable
  *              0 : Temperature based calibration disable
  *            u8TemperatureVal : maximum temperature change after which rc calibration must be trigger
  *              default 5 : 5 Degree C.
  *            u8AverageFactor : average factor
  * @return    RSI_OK on success
  *            Error code on failure
  */

rsi_error_t RSI_TIMEPERIOD_RCCalibration(TIME_PERIOD_Type *pstcTimePeriod,
                                         const rsi_timeperiod_calib_config_t *rc_calib_config)
{
  /*Parameter validation*/
  if (pstcTimePeriod == NULL) {
    return ERROR_TIME_PERIOD_PARAMETERS;
  }
  /*Refernce clock time period*/
  pstcTimePeriod->MCU_CAL_REF_CLK_TIEMPERIOD_REG_b.TIMEPERIOD_REF_CLK =
    (unsigned int)(rc_calib_config->u32TimePeriodRefClk & 0x0FFFFFF);
  /*Clock settling time configuration */
  pstcTimePeriod->MCU_CAL_REF_CLK_SETTLE_REG_b.XTAL_SETTLE = (unsigned int)(rc_calib_config->u32XtalSettle & 0x07);
  /*RC Calib MUX select*/
  pstcTimePeriod->MCU_CAL_START_REG_b.RC_XTAL_MUX_SEL = 0;
  /*Enable the RC calib*/
  pstcTimePeriod->MCU_CAL_START_REG_b.START_CALIB_RC = 1;
  /*average factor */
  pstcTimePeriod->MCU_CAL_START_REG_b.ALPHA_RC = (unsigned int)(rc_calib_config->u8AverageFactor & 0x03);
  /*Number of clock required for calibration */
  pstcTimePeriod->MCU_CAL_START_REG_b.NO_OF_RC_CLKS = (unsigned int)(rc_calib_config->u16ClkCnt & 0x03);

  /*Periodic calibration is selected */
  if (rc_calib_config->bPeriodicCalibEn) {
    /*Periodic calibration enable*/
    pstcTimePeriod->MCU_CAL_START_REG_b.PERIODIC_RC_CALIB_EN = 1;
    /*Update periodic rate at which calibration has to happen */
    pstcTimePeriod->MCU_CAL_START_REG_b.RC_TRIGGER_TIME_SEL =
      (unsigned int)(rc_calib_config->u8PeriodicCalibRate & 0x03);
  }

  /*Periodic calibration is selected */
  if (rc_calib_config->bTemperatureCalibEn) {
    /*Enable tempearature based calibrarion */
    pstcTimePeriod->MCU_CAL_TEMP_PROG_REG_b.PERIODIC_TEMP_CALIB_EN = 1;
    /*Update at which temperature calibration has to happen*/
    pstcTimePeriod->MCU_CAL_TEMP_PROG_REG_b.MAX_TEMP_CHANGE = (unsigned int)(rc_calib_config->u8TemperatureVal & 0x03);
  }
  return RSI_OK;
}

/*==============================================*/
/**
  * @fn        uint32_t RSI_TIMEPERIOD_RCCalibTimePeriodRead(const TIME_PERIOD_Type *pstcTimePeriod)
  * @brief     This API is used to read calibrated timeperiod of RC
  * @param[in] pstcTimePeriod is pointer to the timperiod calibration registration instance
  * @return    Returns the time period on success
  */
uint32_t RSI_TIMEPERIOD_RCCalibTimePeriodRead(const TIME_PERIOD_Type *pstcTimePeriod)
{
  if (pstcTimePeriod == NULL) {
    return ERROR_TIME_PERIOD_PARAMETERS;
  }
  /*Wait for valid bit set */
  while (pstcTimePeriod->MCU_CAL_REF_CLK_SETTLE_REG_b.VALID_RC_TIMEPERIOD != 1)
    ;
  /*Return the time period value */
  return (pstcTimePeriod->MCU_CAL_RC_TIMEPERIOD_READ_b.TIMEPERIOD_RC);
}

/*==============================================*/
/**
  * @fn        uint32_t RSI_TIMEPERIOD_ROCalibTimePeriodRead(const TIME_PERIOD_Type *pstcTimePeriod)
  * @brief     This API is used to read calibrated timeperiod of RO
  * @param[in] pstcTimePeriod : pointer to the timperiod calibration registration instance
  * @return    Returns the time period on success
  */

uint32_t RSI_TIMEPERIOD_ROCalibTimePeriodRead(const TIME_PERIOD_Type *pstcTimePeriod)
{
  if (pstcTimePeriod == NULL) {
    return ERROR_TIME_PERIOD_PARAMETERS;
  }
  /*Wait for valid bit set */
  while (pstcTimePeriod->MCU_CAL_REF_CLK_SETTLE_REG_b.VALID_RO_TIMEPERIOD != 1)
    ;
  /*Return the time period value */
  return (pstcTimePeriod->MCU_CAL_RO_TIMEPERIOD_READ_b.TIMEPERIOD_RO);
}

/*==============================================*/
/**
  * @fn        rsi_error_t RSI_TIMEPERIOD_XTAL32KHzCalibration(TIME_PERIOD_Type *pstcTimePeriod,
  *                                           const rsi_timeperiod_calib_config_t *xtal32khz_calib_config)
  * @brief      This API is calibrate the External 32KHz oscilator
  * @param[in]  pstcTimePeriod         : pointer to the timperiod calibration registration instance
  * @param[in]  xtal32khz_calib_config : Pointer to calibration configuration structure (rsi_timeperiod_calib_config_t)
  *             u32TimePeriodRefClk : timeperiod of reference clk with each bit corresponding to 10ps granularity
  *             u32XtalSettle       : Not used
  *             u16ClkCnt           : 2^no_of_rc_clocks = no of rc clocks used in calibration
  *             bPeriodicCalibEn    : to enable the periodic calibration
  *              1 : periodic calibration mode enable
  *              0 : periodic calibration mode disable
  *             u8PeriodicCalibRate : periodic calibration rate
  *              0 : Every 30secs
  *              1 : every 15 secs,
  *              2 : every 10 secs,
  *              3 : every 5 secs
  *             bTemperatureCalibEn : to enable the temperature based calibration
  *              1 : Temperature based calibration enable
  *              0 : Temperature based calibration disable
  *             u8TemperatureVal : maximum temperature change after which rc calibration must be trigger
  *              default 5 : 5 Degree C.
  *             u8AverageFactor : average factor
  * @return     RSI_OK on success
  */

rsi_error_t RSI_TIMEPERIOD_XTAL32KHzCalibration(TIME_PERIOD_Type *pstcTimePeriod,
                                                const rsi_timeperiod_calib_config_t *xtal32khz_calib_config)

{
  UNUSED_PARAMETER(xtal32khz_calib_config->u32XtalSettle);
  if (pstcTimePeriod == NULL) {
    return ERROR_TIME_PERIOD_PARAMETERS;
  }
  pstcTimePeriod->MCU_CAL_REF_CLK_TIEMPERIOD_REG_b.TIMEPERIOD_REF_CLK =
    (unsigned int)(xtal32khz_calib_config->u32TimePeriodRefClk & 0x0FFFFFF);
  pstcTimePeriod->MCU_CAL_START_REG_b.ALPHA_RC        = (unsigned int)(xtal32khz_calib_config->u8AverageFactor & 0x07);
  pstcTimePeriod->MCU_CAL_START_REG_b.NO_OF_RC_CLKS   = (unsigned int)(xtal32khz_calib_config->u16ClkCnt & 0x07);
  pstcTimePeriod->MCU_CAL_START_REG_b.RC_XTAL_MUX_SEL = 1;
  pstcTimePeriod->MCU_CAL_START_REG_b.START_CALIB_RC  = 1;

  /*Periodic calibration is selected */
  if (xtal32khz_calib_config->bPeriodicCalibEn) {
    /*Periodic calibration enable*/
    pstcTimePeriod->MCU_CAL_START_REG_b.PERIODIC_RC_CALIB_EN = 1;
    /*Update periodic rate at which calibration has to happen */
    pstcTimePeriod->MCU_CAL_START_REG_b.RC_TRIGGER_TIME_SEL =
      (unsigned int)(xtal32khz_calib_config->u8PeriodicCalibRate & 0x07);
  }
  /*Periodic calibration is selected */
  if (xtal32khz_calib_config->bTemperatureCalibEn) {
    /*Enable tempearature based calibrarion */
    pstcTimePeriod->MCU_CAL_TEMP_PROG_REG_b.PERIODIC_TEMP_CALIB_EN = 1;
    /*Update at which temperature calibration has to happen*/
    pstcTimePeriod->MCU_CAL_TEMP_PROG_REG_b.MAX_TEMP_CHANGE =
      (unsigned int)(xtal32khz_calib_config->u8TemperatureVal & 0x1F);
  }
  return RSI_OK;
}

/*==============================================*/
/**
  * @fn         rsi_error_t RSI_TIMEPERIOD_ROCalibration(TIME_PERIOD_Type *pstcTimePeriod,
  *                                    const rsi_timeperiod_calib_config_t *ro_calib_config)
  * @brief      This API is calibrate the RO
  * @param[in]  pstcTimePeriod  : pointer to the timperiod calibration registration instance
  * @param[in]  ro_calib_config : Pointer to calibration configuration structure (rsi_timeperiod_calib_config_t)
  *             u32TimePeriodRefClk    : clock source selection
  *              0 - RC clock calibration happens
  *              1 - XTAL 32khz clock timeperiod calibration occurs with reference clock as 40mhz xtal
  *             This should not be changed in the middle of process. Must be changed only once.
  *             u32XtalSettle    : no of clocks of RO for the RC clk to settle when enabled
  *             u16ClkCnt        : 2^no_of_ro_clks no of clocks of ro clock counts for no of rc clocks in that time to measure timeperiod
  *             bPeriodicCalibEn : to enable the periodic calibration
  *              1 : periodic calibration mode enable
  *              0 : periodic calibration mode disable
  *             u8PeriodicCalibRate : periodic calibration duration
  *              3 : 8 times in a second
  *              2 : 4 times in a second
  *              1 : 2 times in a second
  *              0 : 1 time in a second
  *             bTemperatureCalibEn : Not used
  *             u8TemperatureVal : Not used
  *             u8AverageFactor : average factor
  * @return     RSI_OK on success
  *             Error code on failure
  */

rsi_error_t RSI_TIMEPERIOD_ROCalibration(TIME_PERIOD_Type *pstcTimePeriod,
                                         const rsi_timeperiod_calib_config_t *ro_calib_config)
{
  uint32_t u32ValidTimePeriodRc = 0;

  if (pstcTimePeriod == NULL) {
    return ERROR_TIME_PERIOD_PARAMETERS;
  }

  // Temperature based in not applicable
  UNUSED_PARAMETER(ro_calib_config->bTemperatureCalibEn);
  UNUSED_PARAMETER(ro_calib_config->u8TemperatureVal);

  /*Check RC calibration is done or not */
  u32ValidTimePeriodRc = RSI_TIMEPERIOD_RCCalibTimePeriodRead(pstcTimePeriod);

  if (!u32ValidTimePeriodRc) {
    /* If RC calibration is not done before return error */
    /* NOTE: Before calibrating RO at least RC should be calibrated once */
    return ERROR_TIME_PERIOD_RC_CALIB_NOT_DONE;
  }
  /*Xtal 32Khz clock is used for calibration */
  if (ro_calib_config->u32TimePeriodRefClk) {
    pstcTimePeriod->MCU_CAL_START_REG_b.RC_XTAL_MUX_SEL = 1;
  }
  /*RC 32Khz clock is used for calibration */
  else {
    pstcTimePeriod->MCU_CAL_START_REG_b.RC_XTAL_MUX_SEL = 0;
  }
  pstcTimePeriod->MCU_CAL_START_REG_b.NO_OF_RO_CLKS  = (unsigned int)(ro_calib_config->u16ClkCnt & 0x0F);
  pstcTimePeriod->MCU_CAL_START_REG_b.START_CALIB_RO = 1;
  pstcTimePeriod->MCU_CAL_START_REG_b.ALPHA_RO       = (unsigned int)(ro_calib_config->u8AverageFactor & 0x07);
  pstcTimePeriod->MCU_CAL_START_REG_b.RC_SETTLE_TIME = (unsigned int)(ro_calib_config->u32XtalSettle & 0x07);
  /*Periodic calibration is selected */
  if (ro_calib_config->bPeriodicCalibEn) {
    /*Periodic calibration enable*/
    pstcTimePeriod->MCU_CAL_START_REG_b.PERIODIC_RO_CALIB_EN = 1;
    /*Update periodic rate at which calibration has to happen */
    pstcTimePeriod->MCU_CAL_START_REG_b.RO_TRIGGER_TIME_SEL =
      (unsigned int)(ro_calib_config->u8PeriodicCalibRate & 0x03);
  } else {
    /*Periodic calibration enable*/
    pstcTimePeriod->MCU_CAL_START_REG_b.PERIODIC_RO_CALIB_EN = 0;
  }
  return RSI_OK;
}

/*==============================================*/
/**
  * @fn        rsi_error_t RSI_TIMEPERIOD_TimerClkSel(TIME_PERIOD_Type *pstcTimePeriod, uint32_t u32TimePeriod)
  * @brief     This API is select the RTC clock
  * @param[in] pstcTimePeriod is pointer to the timperiod calibration registration instance
  * @param[in] u32TimePeriod is RTC time period in pico seconds as 32Khz as reference
  * @return    RSI_OK on success
  *            Error code on failure
  */

rsi_error_t RSI_TIMEPERIOD_TimerClkSel(TIME_PERIOD_Type *pstcTimePeriod, uint32_t u32TimePeriod)
{
  uint32_t rtc_time_period = 0;
  uint32_t clock_type      = 0;

  UNUSED_PARAMETER(u32TimePeriod);
  /*Check for the NULL parameter*/
  if (pstcTimePeriod == NULL) {
    return ERROR_TIME_PERIOD_PARAMETERS;
  }

  // read the currently configured FSM low frequency clock
  clock_type = MCU_AON->MCUAON_KHZ_CLK_SEL_POR_RESET_STATUS_b.AON_KHZ_CLK_SEL;
  if (clock_type == KHZ_XTAL_CLK_SEL) {
    rtc_time_period = RTC_TIME_PERIOD_32768HZ;
  } else if (clock_type == KHZ_RC_CLK_SEL) {
    rtc_time_period = RTC_TIME_PERIOD_32000HZ;
  }

  /*Update the timer period */
  pstcTimePeriod->MCU_CAL_TIMER_CLOCK_PERIOD_b.RTC_TIMER_CLK_PERIOD = (unsigned int)(rtc_time_period & 0x01FFFFFF);
  /*Indicated SOC programmed rtc_timer clock period is applied at KHz clock domain*/
  while (!pstcTimePeriod->MCU_CAL_TIMER_CLOCK_PERIOD_b.SPI_RTC_TIMER_CLK_PERIOD_APPLIED_b)
    ;

  return RSI_OK;
}

/*==============================================*/
/**
  * @fn         rsi_error_t RSI_TIMEPERIOD_LowPwrTrigSelEn(TIME_PERIOD_Type *pstcTimePeriod, boolean_t bEn)
  * @brief      This API is select the low power trigger
  * @param[in]  pstcTimePeriod : pointer to the timperiod calibration registration instance
  * @param[in]  bEn 1: seperate counter runs based 2^15 clocks of 32KHz clock = 1sec
  *                 0: rtc runs and triggers are generated based on rtc
  * @return     RSI_OK  on success
  *             Error code on failure
  */

rsi_error_t RSI_TIMEPERIOD_LowPwrTrigSelEn(TIME_PERIOD_Type *pstcTimePeriod, boolean_t bEn)
{
  if (pstcTimePeriod == NULL) {
    return ERROR_TIME_PERIOD_PARAMETERS;
  }
  pstcTimePeriod->MCU_CAL_START_REG_b.LOW_POWER_TRIGGER_SEL = (unsigned int)(bEn & 0x01);
  return RSI_OK;
}

/*==============================================*/
/**
  * @fn         rsi_error_t RSI_TIMEPERIOD_VbatTrigSel(TIME_PERIOD_Type *pstcTimePeriod, uint8_t u8Time)
  * @brief      This API is select the vbat trigger selection
  * @param[in]  pstcTimePeriod : pointer to the timperiod calibration registration instance
  * @param[in]  u8Time : time period
  *             0 : Every minute
  *             1 : Every 30secs
  *             2 : every 15 secs,
  *             3 : every 10 secs,
  *             4 : every 5 secs
  *             5 : every second
  * @return    RSI_OK  on success
  *            Error code on failure
  */

rsi_error_t RSI_TIMEPERIOD_VbatTrigSel(TIME_PERIOD_Type *pstcTimePeriod, uint8_t u8Time)
{
  if (pstcTimePeriod == NULL) {
    return ERROR_TIME_PERIOD_PARAMETERS;
  }
  pstcTimePeriod->MCU_CAL_START_REG_b.VBATT_TRIGGER_TIME_SEL = (unsigned int)(u8Time & 0x07);
  return RSI_OK;
}

/*
  * @}
  */
/*End of file not truncated */
