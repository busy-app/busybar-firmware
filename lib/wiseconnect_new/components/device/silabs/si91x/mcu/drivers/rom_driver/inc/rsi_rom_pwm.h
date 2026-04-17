/******************************************************************************
* @file  rsi_rom_pwm.h
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

#ifndef __RSI_ROM_PWM_H_
#define __RSI_ROM_PWM_H_

/**
 * \ingroup   RSI_SPECIFIC_DRIVERS
 * \defgroup RSI_PWM_DRIVERS
 *  @{
 *
 */
#include "rsi_ccp_user_config.h"
#include "rsi_packing.h"
#include "rsi_pwm.h"
#if defined(A11_ROM)
#include "rsi_rom_table_si91x.h"
#else
#include "rsi_rom_table_RS1xxxx.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
/** @addtogroup SOC12
* @{
*/
/**
 * @fn          STATIC INLINE rsi_error_t RSI_MCPWM_Start(RSI_MCPWM_T *pMCPWM , uint8_t chnlNum)
 * @brief		    This API is used to start the MCPWM operation for required channel
 * @param[in]	  pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]	  chnlNum	:  Channel number(0 to 3)
 * @return 	   \ref  ERROR_PWM_INVALID_CHNLNUM : If channel number is invalid
                \n \ref RSI_OK                    : If process is done successfully
 */
STATIC INLINE rsi_error_t RSI_MCPWM_Start(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum)
{
#if defined(PWM_ROMDRIVER_PRESENT) && !(defined(SLI_SI917B0))
  return ROMAPI_MCPWM_API->mcpwm_start(pMCPWM, chnlNum);
#else
  return mcpwm_start(pMCPWM, chnlNum);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_MCPWM_Stop(RSI_MCPWM_T *pMCPWM , uint8_t chnlNum)
 * @brief		    This API is used to stops the MCPWM operation for required channel
 * @param[in]	  pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]	  chnlNum	:  Channel number(0 to 3)
 * @return      \ref ERROR_PWM_INVALID_CHNLNUM : If channel number is invalid
                \n \ref RSI_OK(0)              : If process is done successfully
 */
STATIC INLINE rsi_error_t RSI_MCPWM_Stop(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum)
{
#if defined(PWM_ROMDRIVER_PRESENT) && !(defined(SLI_SI917B0))
  return ROMAPI_MCPWM_API->mcpwm_stop(pMCPWM, chnlNum);
#else
  return mcpwm_stop(pMCPWM, chnlNum);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_MCPWM_SetTimePeriod( RSI_MCPWM_T *pMCPWM, uint8_t chnlNum,
																								  uint16_t period, uint16_t initVal )
 * @brief		    This API is used to set time period and counter initial,value for the required MCPWM channel.
 * @param[in]	  pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]	  chnlNum	:  Channel number(0 to 3)
 * @param[in]   period  :  Time period value
 * @param[in]   initVal :  Update the base time counter initial value
 * @return      \ref ERROR_PWM_INVALID_CHNLNUM : If channel number is invalid
                \n \ref RSI_OK (0)        : If process is done successfully
 */
STATIC INLINE rsi_error_t RSI_MCPWM_SetTimePeriod(RSI_MCPWM_T *pMCPWM,
                                                  uint8_t chnlNum,
                                                  uint16_t period,
                                                  uint16_t initVal)
{
#if defined(PWM_ROMDRIVER_PRESENT) && !(defined(SLI_SI917B0))
  return ROMAPI_MCPWM_API->mcpwm_set_time_period(pMCPWM, chnlNum, period, initVal);
#else
  return mcpwm_set_time_period(pMCPWM, chnlNum, period, initVal);
#endif
}

/**
 * @fn        STATIC INLINE void  RSI_MCPWM_SpecialEventTriggerConfig(RSI_MCPWM_T *pMCPWM, boolean_t   svtDir,
																				               RSI_MCPWM_SVT_CONFIG_T *pMCPWMSVTConfig)

 * @brief		    This API is used to configure special event trigger generation for required MCPWM channel which
								allows the A/D converter to be synchronized to the PWM time base.
 * @param[in]	  pMCPWM            :  Pointer to the MCPWM instance register area
 * @param[in]   svtDir            :  Special event trigger for time base direction
                                     -  1 : A special event trigger will occur when
                                            PWM time base is counting down
                                     -  0 : A special event trigger will occur when
                                            PWM time base is counting up
 * @param[in]	  pMCPWMSVTConfig  : Pointer to the structure of type \ref RSI_MCPWM_SVT_CONFIG_T
                                   -  svtPostscalar : PWM special event trigger output postscale value
                                                          \n 0000 means 1:1 post scale
																										              \n to
                                                          \n 1111 means  1:16 post scale
                                   -  svtCompareVal : Compare value to generate special event trigger.
                                   -  svtChannel    : Out of four channels, we can generate special event
                                                       for one channel
 * @return 		  none
 */
STATIC INLINE void RSI_MCPWM_SpecialEventTriggerConfig(RSI_MCPWM_T *pMCPWM,
                                                       boolean_t svtDir,
                                                       RSI_MCPWM_SVT_CONFIG_T *pMCPWMSVTConfig)
{
#if defined(PWM_ROMDRIVER_PRESENT) && !(defined(SLI_SI917B0))
  ROMAPI_MCPWM_API->mcpwm_special_event_trigger_config(pMCPWM, svtDir, pMCPWMSVTConfig);
#else
  mcpwm_special_event_trigger_config(pMCPWM, svtDir, pMCPWMSVTConfig);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_MCPWM_DeadTimeValueSet(RSI_MCPWM_T *pMCPWM , RSI_MCPWM_DT_CONFIG_T *pMCPWMDeadTimeConfig,uint8_t chnlNum)
 * @brief		    This API is used to set dead time value to be inserted at rise
								edge or fall edge for required MCPWM channel
 * @param[in]	  pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]   pMCPWMDeadTimeConfig  :  Pointer to the structure of type \ref RSI_MCPWM_DT_CONFIG_T
                                         - counterSelect :  Selects coutner A/B for deadtime insertion.
                                      preScaleA     :  Dead time prescale selection bits for unit A is below like
                                         - 00  :  clock period for dead time unit A is 1x input clock period
                                         - 01  :  clock period for dead time unit A is 2x input clock period
                                         - 10  :  clock period for dead time unit A is 4x input clock period
                                         - 11  :  clock period for dead time unit A is 8x input clock period
*                                     preScaleB     :  Dead time prescale selection bits for unit B is below like
*                                        - 00  :  clock period for dead time unit B is 1x input clock period
*                                        - 01  :  clock period for dead time unit B is 2x input clock period
*                                        - 10  :  clock period for dead time unit B is 4x input clock period
*                                        - 11  :  clock period for dead time unit B is 8x input clock period
*                                     - DeadTimeA     :  Dead time A value to load into deadtime counter A
*                                     - DeadTimeB     :  Dead time B value to load into deadtime counter B
*
* @param[in]   chnlNum  :Channel number(0 to 3)
* @return      \ref ERROR_PWM_INVALID_ARG : If selecetd dead time counter is invalid
                  \n \ref RSI_OK        : If process is done successfully
 */
STATIC INLINE rsi_error_t RSI_MCPWM_DeadTimeValueSet(RSI_MCPWM_T *pMCPWM,
                                                     RSI_MCPWM_DT_CONFIG_T *pMCPWMDeadTimeConfig,
                                                     uint8_t chnlNum)
{
#if defined(PWM_ROMDRIVER_PRESENT) && !(defined(SLI_SI917B0))
  return ROMAPI_MCPWM_API->mcpwm_dead_time_value_set(pMCPWM, pMCPWMDeadTimeConfig, chnlNum);
#else
  return mcpwm_dead_time_value_set(pMCPWM, pMCPWMDeadTimeConfig, chnlNum);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_MCPWM_ChannelReset(RSI_MCPWM_T *pMCPWM,uint8_t chnlNum)
 * @brief		    This API is used to reset the required channel of MCPWM.
 * @param[in]	  pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]	  chnlNum	:  Channel number(0 to 3)
 * @return 		  \ref ERROR_PWM_INVALID_CHNLNUM : If channel is invalid
                \n \ref RSI_OK            : If process is done successfully.
 */
STATIC INLINE rsi_error_t RSI_MCPWM_ChannelReset(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum)
{
#if defined(PWM_ROMDRIVER_PRESENT) && !(defined(SLI_SI917B0))
  return ROMAPI_MCPWM_API->mcpwm_channel_reset(pMCPWM, chnlNum);
#else
  return mcpwm_channel_reset(pMCPWM, chnlNum);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_MCPWM_CounterReset(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum)
 * @brief		    This API is used to reset the counter from required channel of MCPWM
 * @param[in]	  pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]	  chnlNum	:  Channel number(0 to 3)
 * @return      \ref ERROR_PWM_INVALID_CHNLNUM : If channel number is invalid
                \n \ref RSI_OK            : If process is done successfully
 */
STATIC INLINE rsi_error_t RSI_MCPWM_CounterReset(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum)
{
#if defined(PWM_ROMDRIVER_PRESENT) && !(defined(SLI_SI917B0))
  return ROMAPI_MCPWM_API->mcpwm_counter_reset(pMCPWM, chnlNum);
#else
  return mcpwm_counter_reset(pMCPWM, chnlNum);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_MCPWM_PeriodControlConfig(RSI_MCPWM_T *pMCPWM, uint32_t postScale,
                                      uint32_t preScale, uint8_t chnlNum )
 * @brief		    This API is used to set base time period control for the required MCPWM channel.
 * @param[in]	  pMCPWM     :  Pointer to the MCPWM instance register area
 * @param[in]	  postScale  :  Time base output postscale bits ,possible values are as below
                              -  0000 means 1:1
                              -  0001 means 1:2
																		\n To
                              -  1111 means 1:16
 * @param[in]	  preScale  :   Base timer input clock prescale select value,possible values are as below
                              -  000 : 1x input clock period
                              -  010 : 4x input clock period
                              -  011 : 8x input clock period
                              -  100 : 16x input clock period
                              -  101 : 32x input clock period
                              -  110 : 64x input clock period
 * @param[in]	  chnlNum	 :  Channel number(0 to 3)
 * @return      \ref ERROR_PWM_INVALID_CHNLNUM : If channel number is invalid
                \n RSI_OK                 : If process is done successfully

 */
STATIC INLINE rsi_error_t RSI_MCPWM_PeriodControlConfig(RSI_MCPWM_T *pMCPWM,
                                                        uint32_t postScale,
                                                        uint32_t preScale,
                                                        uint8_t chnlNum)
{
#if defined(PWM_ROMDRIVER_PRESENT) && !(defined(SLI_SI917B0))
  return ROMAPI_MCPWM_API->mcpwm_period_control_config(pMCPWM, postScale, preScale, chnlNum);
#else
  return mcpwm_period_control_config(pMCPWM, postScale, preScale, chnlNum);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_MCPWM_FaultAValueSet(RSI_MCPWM_T *pMCPWM,uint8_t pwmOutput,
																							   uint8_t value)
 * @brief		    This API is used to set fault A pin output value to be overridden when fault condition occurs.
 * @param[in]	  pMCPWM    : Pointer to the MCPWM instance register area
 * @param[in]	  pwmOutput : PWM outputs are as below
                            -  0 : L0
                            -  1 : L1
                            -  2 : L2
                            -  3 : L3
                            -  4 : H0
                            -  5 : H1
                            -  6 : H2
                            -  7 : H3
 * @param[in]   value     : Fault input A PWM output override value below
													 -  1 : PWM output pin is driven active on an external fault input A event
													 -  0 : PWM output pin is driven inactive on an external fault input A event
 * @return      \ref ERROR_PWM_INVALID_PWMOUT : If pwmOutput is invalid
                \n RSI_OK                : If process is done successfully
 */
STATIC INLINE rsi_error_t RSI_MCPWM_FaultAValueSet(RSI_MCPWM_T *pMCPWM, uint8_t pwmOutput, uint8_t value)
{
#if defined(PWM_ROMDRIVER_PRESENT) && !(defined(SLI_SI917B0))
  return ROMAPI_MCPWM_API->mcpwm_fault_avalue_set(pMCPWM, pwmOutput, value);
#else
  return mcpwm_fault_avalue_set(pMCPWM, pwmOutput, value);
#endif
}

/**
 * @fn          STATIC INLINE void  RSI_MCPWM_FaultBValueSet(RSI_MCPWM_T *pMCPWM,uint8_t pwmOutput,
																							 uint8_t value)
 * @brief		    This API is used to set fault B pin output value to
								be overridden when fault condition occurs
 * @param[in]	  pMCPWM    : Pointer to the MCPWM instance register area
 * @param[in]	  pwmOutput : PWM outputs are as below
                            -  0 : L0
                            -  1 : L1
                            -  2 : L2
                            -  3 : L3
                            -  4 : H0
                            -  5 : H1
                            -  6 : H2
                            -  7 : H3
 * @param[in]   value     : Fault input B PWM output override value below
													  -  1 : PWM output pin is driven active on an external fault input B event
													  -  0 : PWM output pin is driven inactive on an external fault input B event
 * @return      \ref ERROR_PWM_INVALID_PWMOUT : If pwmOutput is invalid
                \n RSI_OK                : If process is done successfully
 */
STATIC INLINE rsi_error_t RSI_MCPWM_FaultBValueSet(RSI_MCPWM_T *pMCPWM, uint8_t pwmOutput, uint8_t value)
{
#if defined(PWM_ROMDRIVER_PRESENT) && !(defined(SLI_SI917B0))
  return ROMAPI_MCPWM_API->mcpwm_fault_bvalue_set(pMCPWM, pwmOutput, value);
#else
  return mcpwm_fault_bvalue_set(pMCPWM, pwmOutput, value);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_MCPWM_SetBaseTimerMode(RSI_MCPWM_T *pMCPWM,uint8_t mode,
                                                   uint8_t chnlNum)
 * @brief		    This API is used to set the mode of base timer for required channel
 * @param[in]	  pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]	  mode    :  Base timer operating mode is below
                           -  000 : free running mode
                           -  001 : single event mode
                           -  010 : down count mode
                           -  100 : up/down mode
                           -  101 : up/dowm mode with interrupts for double PWM updates
 * @param[in]   chnlNum : Channel number(0 to 3)
 * @return      \ref ERROR_PWM_INVALID_CHNLNUM : If channel number is invalid
                \n \ref RSI_OK                 : If process is done successfully

 */
STATIC INLINE rsi_error_t RSI_MCPWM_SetBaseTimerMode(RSI_MCPWM_T *pMCPWM, uint8_t mode, uint8_t chnlNum)
{
#if defined(PWM_ROMDRIVER_PRESENT) && !(defined(SLI_SI917B0))
  return ROMAPI_MCPWM_API->mcpwm_set_base_timer_mode(pMCPWM, mode, chnlNum);
#else
  return mcpwm_set_base_timer_mode(pMCPWM, mode, chnlNum);
#endif
}

/**
 * @fn          STATIC INLINE rsi_error_t RSI_MCPWM_SetOutputMode(RSI_MCPWM_T *pMCPWM, boolean_t mode,
                                                uint8_t chnlNum)
 * @brief		    This API is used to set output mode for the MCPWM
 * @param[in]	  pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]	  mode    :  If input is one then complementary output mode if zero then independent output mode.
 * @param[in]   chnlNum :  Channel number(0 to 3)
 * @return      \ref ERROR_PWM_INVALID_CHNLNUM : If channel number is invalid
                \n \ref RSI_OK                 : If process is done successfully
 */
STATIC INLINE rsi_error_t RSI_MCPWM_SetOutputMode(RSI_MCPWM_T *pMCPWM, boolean_t mode, uint8_t chnlNum)
{
#if defined(PWM_ROMDRIVER_PRESENT) && !(defined(SLI_SI917B0))
  return ROMAPI_MCPWM_API->mcpwm_set_output_mode(pMCPWM, mode, chnlNum);
#else
  return mcpwm_set_output_mode(pMCPWM, mode, chnlNum);
#endif
}

/**
 * @fn          STATIC INLINE void  RSI_MCPWM_SetOutputPolarity(RSI_MCPWM_T *pMCPWM,
																									boolean_t polL,boolean_t polH)
 * @brief		    This API is used to set output polarity for MCPWM
 * @param[in]	  pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]	  polL	  :  Ouput polarity for low side (L3, L2, L1, L0)
 * @param[in]   polH    :  Ouput polarity for high side (H3, H2, H1, H0)
 * @return 		  none
 */
STATIC INLINE void RSI_MCPWM_SetOutputPolarity(RSI_MCPWM_T *pMCPWM, boolean_t polL, boolean_t polH)
{
#if defined(PWM_ROMDRIVER_PRESENT) && !(defined(SLI_SI917B0))
  ROMAPI_MCPWM_API->mcpwm_set_output_polarity(pMCPWM, polL, polH);
#else
  mcpwm_set_output_polarity(pMCPWM, polL, polH);
#endif
}

/**
 * @fn          STATIC INLINE void  RSI_MCPWM_InterruptHandler( RSI_MCPWM_T *pMCPWM,
                                                 RSI_MCPWM_CALLBACK_T *pCallBack)
 * @brief		    Handles all interrupt flags of MCPWM.
 * @param[in]	  pMCPWM    :  Pointer to the MCPWM instance register area
 * @param[in]	  pCallBack :  Pointer to the structure of type RSI_CALLBACK
 * @return      none
 */
STATIC INLINE void RSI_MCPWM_InterruptHandler(RSI_MCPWM_T *pMCPWM, RSI_MCPWM_CALLBACK_T *pCallBack)
{
#if defined(PWM_ROMDRIVER_PRESENT) && !(defined(SLI_SI917B0))
  ROMAPI_MCPWM_API->mcpwm_interrupt_handler(pMCPWM, pCallBack);
#else
  mcpwm_interrupt_handler(pMCPWM, pCallBack);
#endif
}

/**
 * @fn          STATIC INLINE uint16_t RSI_MCPWM_GetTimePeriod( RSI_MCPWM_T *pMCPWM,
                                                  uint8_t chnlNum,uint16_t *timePeriod)
 * @brief		    Get time period for required channel.
 * @param[in]   pMCPWM      : Pointer to the MCPWM instance register area
 * @param[in]   chnlNum     : channel number(0-3)
 * @param[out]  timePeriod  : pointer to read time period.
 * @return      \ref ERROR_PWM_INVALID_CHNLNUM : If channel number is invalid
 *               \n \ref RSI_OK                : If process is done successfully
 */
STATIC INLINE uint16_t RSI_MCPWM_GetTimePeriod(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum, uint16_t *timePeriod)
{
  switch (chnlNum) {
    case PWM_CHNL_0:
      *timePeriod = (pMCPWM->PWM_TIME_PRD_WR_REG_CH0_b.PWM_TIME_PRD_REG_WR_VALUE_CH0);
      break;
    case PWM_CHNL_1:
      *timePeriod = (pMCPWM->PWM_TIME_PRD_WR_REG_CH1_b.PWM_TIME_PRD_REG_WR_VALUE_CH1);
      break;
    case PWM_CHNL_2:
      *timePeriod = (pMCPWM->PWM_TIME_PRD_WR_REG_CH2_b.PWM_TIME_PRD_REG_WR_VALUE_CH2);
      break;
    case PWM_CHNL_3:
      *timePeriod = (pMCPWM->PWM_TIME_PRD_WR_REG_CH3_b.PWM_TIME_PRD_REG_WR_VALUE_CH3);
      break;
    default:
      return ERROR_PWM_INVALID_CHNLNUM;
  }
  return RSI_OK;
}

/**************************************************************************************
								              MCPWM ROM FUNCTION PROTOTYPES											  
		 **************************************************************************************/
rsi_error_t RSI_MCPWM_Start(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum);

rsi_error_t RSI_MCPWM_Stop(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum);

rsi_error_t RSI_MCPWM_SetTimePeriod(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum, uint16_t period, uint16_t initVal);

void RSI_MCPWM_SpecialEventTriggerConfig(RSI_MCPWM_T *pMCPWM,
                                         boolean_t svtDir,
                                         RSI_MCPWM_SVT_CONFIG_T *pMCPWMSVTConfig);

rsi_error_t RSI_MCPWM_DeadTimeValueSet(RSI_MCPWM_T *pMCPWM,
                                       RSI_MCPWM_DT_CONFIG_T *pMCPWMDeadTimeConfig,
                                       uint8_t chnlNum);

rsi_error_t RSI_MCPWM_ChannelReset(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum);

rsi_error_t RSI_MCPWM_CounterReset(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum);

rsi_error_t RSI_MCPWM_PeriodControlConfig(RSI_MCPWM_T *pMCPWM, uint32_t postScale, uint32_t preScale, uint8_t chnlNum);

rsi_error_t RSI_MCPWM_FaultAValueSet(RSI_MCPWM_T *pMCPWM, uint8_t pwmOutput, uint8_t value);

rsi_error_t RSI_MCPWM_FaultBValueSet(RSI_MCPWM_T *pMCPWM, uint8_t pwmOutput, uint8_t value);

rsi_error_t RSI_MCPWM_SetBaseTimerMode(RSI_MCPWM_T *pMCPWM, uint8_t mode, uint8_t chnlNum);

rsi_error_t RSI_MCPWM_SetOutputMode(RSI_MCPWM_T *pMCPWM, boolean_t mode, uint8_t chnlNum);

void RSI_MCPWM_SetOutputPolarity(RSI_MCPWM_T *pMCPWM, boolean_t polL, boolean_t polH);

void RSI_MCPWM_InterruptHandler(RSI_MCPWM_T *pMCPWM, RSI_MCPWM_CALLBACK_T *pCallBack);

uint16_t RSI_MCPWM_GetTimePeriod(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum, uint16_t *timePeriod);

#ifdef __cplusplus
}
#endif
#endif
/** @} */
/* @}end of  RSI_PWM_DRIVERS */
