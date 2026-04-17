/******************************************************************************
* @file  rsi_rtc.c
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

// Includes
#include "si91x_device.h"
#include "rsi_power_save.h"
#include "rsi_rtc.h"
#include "rsi_time_period.h"

#define CENTURTY_START   0
#define BYPASS_RESET_BIT 0x02
/** @addtogroup SOC19
* @{
*/
/*==============================================*/
/**
 * @fn		    void RSI_RTC_Start(RTC_Type *Cal)
 * @brief   	This API is used to start the RTC
 * @param[in] 	Cal :pointer to the rtc register instance
 * @return      none
 */

void RSI_RTC_Start(RTC_Type *Cal)
{
#if defined(SLI_SI917)
  if (0x1 == STATIC_COMBI_RTC_PG_ENABLE) {
    /* Enable static combi rtc powergate */
    Cal->MCU_CAL_POWERGATE_REG_b.DISABLE_COMBI_DYN_PWRGATE_EN = 0x1;
    Cal->MCU_CAL_POWERGATE_REG_b.STATIC_COMBI_RTC_PG_EN       = 0x1;
  } else {
    /* Disable static combi rtc powergate*/
    Cal->MCU_CAL_POWERGATE_REG_b.STATIC_COMBI_RTC_PG_EN = 0x0;
  }
#endif
  Cal->MCU_CAL_POWERGATE_REG_b.ENABLE_CALENDER_COMBI = 1u;
  Cal->MCU_CAL_POWERGATE_REG_b.PG_EN_CALENDER        = 1u;
  return;
}

/*==============================================*/
/**
 * @fn			void RSI_RTC_Init(RTC_Type *Cal)
 * @brief    	This API is used to init the rtc block
 * @param[in] 	Cal : pointer to the rtc register instance
 * @return  	none
 */

void RSI_RTC_Init(RTC_Type *Cal)
{
  /* Power-Up RTC Domain */
  RSI_PS_NpssPeriPowerUp(SLPSS_PWRGATE_ULP_MCURTC | SLPSS_PWRGATE_ULP_TIMEPERIOD);

  /*Wait for RTC-ON pulse synchronization*/
  do {
    RTC->MCU_CAL_ALARM_PROG_1 = 6U;
  } while (RTC->MCU_CAL_ALARM_PROG_1 != 6U);
  // by using this API we programmed the RTC timer clock in SOC
  // MSB 8-bits for the Integer part &
  // LSB 17-bits for the Fractional part
  // Eg:- 32KHz = 31.25µs ==> 31.25*2^17 = 4096000 = 0x3E8000
  /* Time Period Programming */
  RSI_TIMEPERIOD_TimerClkSel(TIME_PERIOD, 0x003E7FFF);
  /*Enable power gates*/
  RSI_RTC_Start(Cal);
  return;
}

/*==============================================*/
/**
 * @fn				void RSI_RTC_Stop(RTC_Type *Cal)
 * @brief   		This API is used to stop the rtc operation
 * @param[in]		Cal : pointer to the rtc register instance
 * @return  		none
 */

void RSI_RTC_Stop(RTC_Type *Cal)
{
  Cal->MCU_CAL_POWERGATE_REG_b.ENABLE_CALENDER_COMBI = 0u;
  Cal->MCU_CAL_POWERGATE_REG_b.PG_EN_CALENDER        = 0u;
  return;
}

/*==============================================*/
/**	
 * @fn           rsi_error_t RSI_RTC_SetDateTime(RTC_Type *Cal, const RTC_TIME_CONFIG_T *date)
 * @brief   	 This API is used to set the rtc configuration
 * @param[in]    Cal   : pointer to the rtc register instance
 * @param[in]    date  : pointer to the rtc configuration structure
 * @return       RSI_OK on success 
 *               
 * - Error code on failure
 */

rsi_error_t RSI_RTC_SetDateTime(RTC_Type *Cal, const RTC_TIME_CONFIG_T *date)
{
  if ((Cal == NULL) || (date == NULL)) {
    return ERROR_CAL_INVALID_PARAMETERS;
  }
  Cal->MCU_CAL_PROG_TIME_2_b.PROG_DAY       = (unsigned int)((date->Day) & 0x1F);
  Cal->MCU_CAL_PROG_TIME_2_b.PROG_WEEK_DAY  = date->DayOfWeek;
  Cal->MCU_CAL_PROG_TIME_2_b.PROG_MONTH     = date->Month;
  Cal->MCU_CAL_PROG_TIME_2_b.PROG_YEAR      = (unsigned int)((date->Year) & 0x7F);
  Cal->MCU_CAL_PROG_TIME_2_b.PROG_CENTURY   = (unsigned int)((date->Century) & 0x03);
  Cal->MCU_CAL_PROG_TIME_1_b.PROG_MSEC      = (unsigned int)((date->MilliSeconds) & 0x03FF);
  Cal->MCU_CAL_PROG_TIME_1_b.PROG_SEC       = (unsigned int)((date->Second) & 0x3F);
  Cal->MCU_CAL_PROG_TIME_1_b.PROG_MIN       = (unsigned int)(date->Minute & 0x3F);
  Cal->MCU_CAL_PROG_TIME_1_b.PROG_HOUR      = (unsigned int)(date->Hour & 0x1F);
  Cal->MCU_CAL_PROG_TIME_2_b.PROG_TIME_TRIG = 1U;
  /*Wait for applied values to updated to registers */
  while (Cal->MCU_CAL_READ_TIME_LSB_b.MILLISECONDS_COUNT != date->MilliSeconds)
    ;
  while (Cal->MCU_CAL_READ_TIME_LSB_b.SECONDS_COUNT != date->Second)
    ;
  while (Cal->MCU_CAL_READ_TIME_LSB_b.MINS_COUNT != date->Minute)
    ;
  while (Cal->MCU_CAL_READ_TIME_LSB_b.HOURS_COUNT != date->Hour)
    ;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn              rsi_error_t RSI_RTC_GetDateTime(const RTC_Type *Cal, RTC_TIME_CONFIG_T *date)
 * @brief    	 	This API is used to Get the RTC time
 * @param[in]  	    Cal  : pointer to the rtc register instance
 * @param[in] 		date : pointer to the rtc structure to hold the current time parameters
 * @return    		RSI_OK on success 
 *                  
 * - Error code on failure
 */

rsi_error_t RSI_RTC_GetDateTime(const RTC_Type *Cal, RTC_TIME_CONFIG_T *date)
{
  if ((Cal == NULL) || (date == NULL)) {
    return ERROR_CAL_INVALID_PARAMETERS;
  }
  date->MilliSeconds = Cal->MCU_CAL_READ_TIME_LSB_b.MILLISECONDS_COUNT;
  date->Second       = Cal->MCU_CAL_READ_TIME_LSB_b.SECONDS_COUNT;
  date->Minute       = Cal->MCU_CAL_READ_TIME_LSB_b.MINS_COUNT;
  date->Hour         = Cal->MCU_CAL_READ_TIME_LSB_b.HOURS_COUNT;
  date->DayOfWeek    = (RTC_DAY_OF_WEEK_T)Cal->MCU_CAL_READ_TIME_MSB_b.WEEK_DAY;
  date->Day          = Cal->MCU_CAL_READ_TIME_LSB_b.DAYS_COUNT;
  date->Month        = (RTC_MONTH_T)Cal->MCU_CAL_READ_TIME_MSB_b.MONTHS_COUNT;
  date->Year         = Cal->MCU_CAL_READ_TIME_MSB_b.YEAR_COUNT;
  date->Century      = Cal->MCU_CAL_READ_TIME_MSB_b.CENTURY_COUNT;

  return RSI_OK;
}

/*==============================================*/
/**	
 * @fn	             rsi_error_t RSI_RTC_SetAlarmDateTime(RTC_Type *Cal, const RTC_TIME_CONFIG_T *alarm)
 * @brief    		 This API is used to Set the alarm for RTC module
 * @param[in]  		 Cal   : pointer to the rtc register instance
 * @param[in]  		 alarm : pointer to alarm configuration structure
 * @return    		 RSI_OK on success
 *                   
 * - Error code on failure
 */

rsi_error_t RSI_RTC_SetAlarmDateTime(RTC_Type *Cal, const RTC_TIME_CONFIG_T *alarm)
{
  if ((Cal == NULL) || (alarm == NULL)) {
    return ERROR_CAL_INVALID_PARAMETERS;
  }

  Cal->MCU_CAL_ALARM_PROG_1_b.PROG_ALARM_HOUR = (unsigned int)((alarm->Hour) & 0x1F);           //HH
  Cal->MCU_CAL_ALARM_PROG_1_b.PROG_ALARM_MIN  = (unsigned int)((alarm->Minute) & 0x3F);         //MM
  Cal->MCU_CAL_ALARM_PROG_1_b.PROG_ALARM_SEC  = (unsigned int)((alarm->Second) & 0x3F);         //SS
  Cal->MCU_CAL_ALARM_PROG_1_b.PROG_ALARM_MSEC = (unsigned int)((alarm->MilliSeconds) & 0x03FF); // MSEC

  Cal->MCU_CAL_ALARM_PROG_2_b.PROG_ALARM_CENTURY = (unsigned int)((alarm->Century) & 0x03); // CC
  Cal->MCU_CAL_ALARM_PROG_2_b.PROG_ALARM_DAY     = (unsigned int)((alarm->Day) & 0x1F);     // DAY
  Cal->MCU_CAL_ALARM_PROG_2_b.PROG_ALARM_MONTH   = alarm->Month;                            //MONTH
  Cal->MCU_CAL_ALARM_PROG_2_b.PROG_ALARM_YEAR    = (unsigned int)((alarm->Year) & 0x7F);    // YEAR
  /*Wait for applied values to updated to registers */
  while (Cal->MCU_CAL_ALARM_PROG_1_b.PROG_ALARM_MIN != alarm->Minute)
    ;
  while (Cal->MCU_CAL_ALARM_PROG_1_b.PROG_ALARM_SEC != alarm->Second)
    ;
  while (Cal->MCU_CAL_ALARM_PROG_1_b.PROG_ALARM_MSEC != alarm->MilliSeconds)
    ;
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn				void RSI_RTC_AlamEnable(RTC_Type *Cal , boolean_t val)
 * @brief   		This API is used to Enable alarm in RTC
 * @param[in] 		Cal : pointer to the rtc register instance
 * @param[in] 		val    : to enable or disable the alarm
 *           				- 0 : Disable the alarm
 *            			- 1 : Enable the alarm
 *
 * @return  		none
 */

void RSI_RTC_AlamEnable(RTC_Type *Cal, boolean_t val)
{
  Cal->MCU_CAL_ALARM_PROG_2_b.ALARM_EN = (unsigned int)(val & 0x01);
  return;
}

/*==============================================*/
/**
 * @fn               rsi_error_t RSI_RTC_GetAlarmDateTime(const RTC_Type *Cal, RTC_TIME_CONFIG_T *alarm)
 * @brief   		 This API is used to Get alarm configurations for RTC
 * @param[in]		 Cal 	 : pointer to the rtc register instance
 * @param[in]		 alarm : pointer to the rtc alarm configuration structure
 * @return			 RSI_OK on success
 *                   
 * - Error code on failure
 */

rsi_error_t RSI_RTC_GetAlarmDateTime(const RTC_Type *Cal, RTC_TIME_CONFIG_T *alarm)
{
  if ((Cal == NULL) || (alarm == NULL)) {
    return ERROR_CAL_INVALID_PARAMETERS;
  }

  alarm->Hour         = Cal->MCU_CAL_ALARM_PROG_1_b.PROG_ALARM_HOUR; //HH
  alarm->Minute       = Cal->MCU_CAL_ALARM_PROG_1_b.PROG_ALARM_MIN;  //MM
  alarm->Second       = Cal->MCU_CAL_ALARM_PROG_1_b.PROG_ALARM_SEC;  //SS
  alarm->MilliSeconds = Cal->MCU_CAL_ALARM_PROG_1_b.PROG_ALARM_MSEC; // MSEC

  alarm->Century = Cal->MCU_CAL_ALARM_PROG_2_b.PROG_ALARM_CENTURY;            // CC
  alarm->Day     = Cal->MCU_CAL_ALARM_PROG_2_b.PROG_ALARM_DAY;                // DAY
  alarm->Month   = (RTC_MONTH_T)Cal->MCU_CAL_ALARM_PROG_2_b.PROG_ALARM_MONTH; //MONTH
  alarm->Year    = Cal->MCU_CAL_ALARM_PROG_2_b.PROG_ALARM_YEAR;               // YEAR
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn			     void RSI_RTC_SetDayOfWeek(RTC_Type *Cal , RTC_DAY_OF_WEEK_T dayInWeek)
 * @brief   		 This API is used to set the Day in a week
 * @param[in]		 Cal 	   : pointer to the rtc register instance
 * @param[in]		 dayInWeek : enum value of days in a week
 *														- \ref Sunday      
 *   													- \ref Monday
 *   													- \ref Tuesday       
 *  													- \ref Wednesday
 *														- \ref Thursday
 *  													- \ref Friday         
 *  													- \ref Saturday     
 * @return			 none
 */

void RSI_RTC_SetDayOfWeek(RTC_Type *Cal, RTC_DAY_OF_WEEK_T dayInWeek)
{
  Cal->MCU_CAL_PROG_TIME_2_b.PROG_WEEK_DAY = dayInWeek;
  return;
}

/*==============================================*/
/**
 * @fn			     void RSI_RTC_IntrUnMask(uint32_t intr)
 * @brief   		 This API is used to enable the RTC alarm interrupts
 * @param[in]		 intr  : \b Ored value of interrupt to be enabled/Un-mask
 *                       - \ref RTC_MSEC_INTR  : Use this macro to enable msec interrupt
 *                       - \ref RTC_SEC_INTR   : Use this macro to enable sec interrupt
 *                       - \ref RTC_ALARM_INTR : Use this macro to enable alarm interrupt 
 * @return			 none
 */

void RSI_RTC_IntrUnMask(uint32_t intr)
{
  NPSS_INTR_MASK_CLR_REG = intr;
}

/*==============================================*/
/**
 * @fn			void RSI_RTC_IntrMask(uint32_t intr)
 * @brief   	This API is used to disable the RTC alarm interrupts
 * @param[in]	intr  : \b Ored value of interrupt to be disabled/Mask
 *                       - \ref RTC_MSEC_INTR  : Use this macro to disable msec interrupt
 *                       - \ref RTC_SEC_INTR   : Use this macro to disable sec interrupt
 *                       - \ref RTC_ALARM_INTR : Use this macro to disable alarm interrupt 
 * @return		none
 */

void RSI_RTC_IntrMask(uint32_t intr)
{
  NPSS_INTR_MASK_SET_REG = intr;
}

/*==============================================*/
/**
 * @fn			void RSI_RTC_IntrClear(uint32_t intr)
 * @brief   	This API is used to clear the RTC alarm interrupts
 * @param[in]	intr  : \b Ored value of interrupt to be cleared
 *                       - \ref RTC_MSEC_INTR  : Use this macro to clear msec interrupt
 *                       - \ref RTC_SEC_INTR   : Use this macro to clear sec interrupt
 *                       - \ref RTC_ALARM_INTR : Use this macro to clear alarm interrupt 
 * @return		none
 */

void RSI_RTC_IntrClear(uint32_t intr)
{
  /*Clear FSM registers */
  if (intr & RTC_MSEC_INTR) {
    NPSS_INTR_CLEAR_REG                  = NPSS_TO_MCU_MSEC_INTR;
    MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR = MILLI_SEC_BASED_STATUS_CLEAR;
  }
  if (intr & RTC_SEC_INTR) {
    NPSS_INTR_CLEAR_REG                  = NPSS_TO_MCU_SEC_INTR;
    MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR = RTC_SEC_BASED_STATUS_CLEAR;
  }
  if (intr & RTC_ALARM_INTR) {
    NPSS_INTR_CLEAR_REG                  = NPSS_TO_MCU_ALARM_INTR;
    MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR = RTC_ALARM_BASED_WAKEUP_STATUS_CLEAR;
  }
}

/*==============================================*/
/**
 * @fn			void RSI_RTC_CalibInitialization(void)
 * @brief   	This API is used to initialize RTC CALIBRATION
 * @return		none
 */

void RSI_RTC_CalibInitilization(void)
{
  ULP_SPI_MEM_MAP(0x106) |= (BIT(21) | BIT(19));
}

/*==============================================*/
/**
 * @fn			void RSI_RTC_RCCLK_Calib(TIME_PERIOD_Type *rtc, uint8_t enable,uint8_t periodic_en,uint8_t trigger_time)
 * @brief   	This API is used to rc calibration
 * @param[in]	rtc      : 
 * @param[in]	enable      : 1 to start rc calibration
 * @param[in]	periodic_en : 1 to start periodically calibrate
 * @param[in]	trigger_time : rc_trigger time
 *             - 0 :5 sec
 *             - 1 :10 sec 
 *             - 2 :15 sec
 *             - 3 :30 sec
 *             - 4 :1 minute
 *             - 5 :2 minute          
 * @return		none
 */

void RSI_RTC_RCCLK_Calib(TIME_PERIOD_Type *rtc, uint8_t enable, uint8_t periodic_en, uint8_t trigger_time)
{
  rtc->MCU_CAL_TEMP_PROG_REG_b.RTC_TIMER_PERIOD_MUX_SEL = 1;
  rtc->MCU_CAL_START_REG_b.START_CALIB_RC               = (unsigned int)(enable & 0x01);
  rtc->MCU_CAL_START_REG_b.PERIODIC_RC_CALIB_EN         = (unsigned int)(periodic_en & 0x01);
  rtc->MCU_CAL_START_REG_b.RC_TRIGGER_TIME_SEL          = (unsigned int)(trigger_time & 0x07);
  return;
}

/*==============================================*/
/**
 * @fn			void RSI_RTC_ROCLK_Calib(TIME_PERIOD_Type *rtc,
 *                       uint8_t enable,
 *                       uint8_t periodic_en,
 *                       uint8_t trigger_time,
 *                       uint8_t ro_enable,
 *                       uint8_t ro_periodic_en,
 *                       uint8_t ro_trigger_time)
 * @brief   	This API is used to rc and ro calibration
 * @param[in]	rtc      : 
 * @param[in]	enable      : 1 to start rc calibration
 * @param[in] periodic_en : 1 to start periodically calibrate
 * @param[in] trigger_time : rc_trigger time
 *              - 0 :5 sec
 *              - 1 :10 sec 
 *              - 2 :15 sec
 *              - 3 :30 sec
 *              - 4 :1 minute
 *              - 5 :2 minute   
 * @param[in] ro_enable  :1 to enavle ro calib
 * @param[in] periodic_en:1 to enable periodic ro calib
 * @param[in] trigger_time : ro_trigger time
 *               - 0 :1 time in sec
 *               - 1 :2 time in sec 
 *               - 2 :4 time in sec
 *               - 3 :8 time in sec               
 * @return		none
 */

void RSI_RTC_ROCLK_Calib(TIME_PERIOD_Type *rtc,
                         uint8_t enable,
                         uint8_t periodic_en,
                         uint8_t trigger_time,
                         uint8_t ro_enable,
                         uint8_t ro_periodic_en,
                         uint8_t ro_trigger_time)
{
  RSI_RTC_RCCLK_Calib(rtc, enable, periodic_en, trigger_time);
  rtc->MCU_CAL_TEMP_PROG_REG_b.RTC_TIMER_PERIOD_MUX_SEL = 1;
  rtc->MCU_CAL_START_REG_b.START_CALIB_RO               = (unsigned int)(ro_enable & 0x01);
  rtc->MCU_CAL_START_REG_b.PERIODIC_RO_CALIB_EN         = (unsigned int)(ro_periodic_en & 0x01);
  rtc->MCU_CAL_START_REG_b.RO_TRIGGER_TIME_SEL          = (unsigned int)(ro_trigger_time & 0x03);
  return;
}

/*==============================================*/
/**
 * @fn			uint32_t  RSI_RTC_GetIntrStatus(void)
 * @brief   	This API is used to get the RTC alarm interrupt status
 * @return      intr status  : \b Ored value of interrupt status
 *                       - \ref RTC_MSEC_INTR  : Use this macro to check the msec interrupt status
 *                       - \ref RTC_SEC_INTR   : Use this macro to check the sec interrupt status
 *                       - \ref RTC_ALARM_INTR : Use this macro to check the alarm interrupt status
 */

uint32_t RSI_RTC_GetIntrStatus(void)
{
  return NPSS_INTR_STATUS_REG;
}

/*==============================================*/
/**
 * @fn			  void  RSI_RTC_BypassReset(void)
 * @brief   	This API is used to bypass the soft reset of mcu.
 * @return    none
 */
void RSI_RTC_BypassReset(void)
{
  NWPAON_POR_CTRL_BITS |= BYPASS_RESET_BIT;
}
/** @} */
/*End of file not truncated*/
