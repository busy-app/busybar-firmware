/******************************************************************************
* @file  rsi_pwm.c
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

// Include files

#include "rsi_ccp_user_config.h"
#if (defined(PWM_ROMDRIVER_PRESENT)) || (defined(SLI_SI917B0))
#include "rsi_rom_pwm.h"

#ifdef __cplusplus
extern "C" {
#endif
/** @addtogroup SOC12
* @{
*/
/*==============================================*/
/**
 * @fn            RSI_DRIVER_VERSION_M4 RSI_MCPWM_GetVersion(void)
 * @brief		  This API is used to get version information of the driver implementation
 * @return 		  structure of type RSI_DRIVER_VERSION_M4 and its members are as below
 *				  - \ref RSI_MCPWM_API_VERSION : Version of the CMSIS-Driver specification used to implement this driver.
 *			      - \ref RSI_MCPWM_DRV_VERSION : MCPWM peripheral source code version of the actual driver implementation. 
 */
RSI_DRIVER_VERSION_M4 RSI_MCPWM_GetVersion(void)
{
  RSI_DRIVER_VERSION_M4 vsDriverVersion;

  vsDriverVersion.api = RSI_MCPWM_API_VERSION;
  vsDriverVersion.drv = RSI_MCPWM_API_VERSION;
  return vsDriverVersion;
}

/*==============================================*/
/**
 * @fn          RSI_MCPWM_CAPABILITIES RSI_MCPWM_GetCapabilities(void)
 * @brief		The function RSI_MCPWM_GetCapabilities returns information about capabilities of this driver implementation. 
 *								The data fields of the struct RSI_MCPWM_CAPABILITIES encodes the driver capabilities.
 * @param[in]	none
 * @return 		structure of type RSI_MCPWM_CAPABILITIES and its members are as below
 *							  - \ref PWMOutputs    : Number of PWM outputs
 *							  - \ref FaultPins     : Number of fault input pins							
 */
RSI_MCPWM_CAPABILITIES_T RSI_MCPWM_GetCapabilities(void)
{
  RSI_MCPWM_CAPABILITIES_T vsDriverCapabilities;

  vsDriverCapabilities.faultPins  = 2;
  vsDriverCapabilities.pwmOutputs = 8;
  return vsDriverCapabilities;
}

/*==============================================*/
/**
 * @fn            rsi_error_t mcpwm_start(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum)
 * @brief		  This API is used to start the MCPWM operation for required channel
 * @param[in]	  pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]	  chnlNum	:  Channel number(0 to 3)
 * @return 	      \ref  ERROR_PWM_INVALID_CHNLNUM : If channel number is invalid
 *                 \n \ref RSI_OK                    : If process is done successfully
 */
rsi_error_t mcpwm_start(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum)
{
  // starts base timer operation
  switch (chnlNum) {
    case PWM_CHNL_0:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH0_b.PWM_TIME_BASE_EN_FRM_REG_CH0 = ENABLE;
      break;
    case PWM_CHNL_1:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH1_b.PWM_TIME_BASE_EN_FRM_REG_CH1 = ENABLE;
      break;
    case PWM_CHNL_2:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH2_b.PWM_TIME_BASE_EN_FRM_REG_CH2 = ENABLE;
      break;
    case PWM_CHNL_3:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH3_b.PWM_TIME_BASE_EN_FRM_REG_CH3 = ENABLE;
      break;
    default:
      return ERROR_PWM_INVALID_CHNLNUM;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn            rsi_error_t mcpwm_stop(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum)
 * @brief		  This API is used to start the MCPWM operation for required channel
 * @param[in]	  pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]	  chnlNum	:  Channel number(0 to 3)
 * @return 	      \ref  ERROR_PWM_INVALID_CHNLNUM : If channel number is invalid
 *                \n \ref RSI_OK(0)               : If process is done successfully
 */
rsi_error_t mcpwm_stop(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum)
{
  // Stops base timer operation
  switch (chnlNum) {
    case PWM_CHNL_0:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH0_b.PWM_TIME_BASE_EN_FRM_REG_CH0 = DISABLE;
      break;
    case PWM_CHNL_1:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH1_b.PWM_TIME_BASE_EN_FRM_REG_CH1 = DISABLE;
      break;
    case PWM_CHNL_2:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH2_b.PWM_TIME_BASE_EN_FRM_REG_CH2 = DISABLE;
      break;
    case PWM_CHNL_3:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH3_b.PWM_TIME_BASE_EN_FRM_REG_CH3 = DISABLE;
      break;
    default:
      return ERROR_PWM_INVALID_CHNLNUM;
  }
  return RSI_OK;
}
/*==============================================*/
/**
 * @fn          rsi_error_t mcpwm_set_time_period(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum, uint16_t period, uint16_t initVal)
 * @brief		This API is used to set time period and counter initial,value for the required MCPWM channel.
 * @param[in]	pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]	chnlNum	:  Channel number(0 to 3)
 * @param[in]   period  :  Time period value
 * @param[in]   initVal :  Update the base time counter initial value
 * @return      \ref ERROR_PWM_INVALID_CHNLNUM : If channel number is invalid
 *              \n \ref RSI_OK (0)        : If process is done successfully
 */
rsi_error_t mcpwm_set_time_period(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum, uint16_t period, uint16_t initVal)
{
  // Sets initial value and timer period for time base counter
  switch (chnlNum) {
    case PWM_CHNL_0:
      pMCPWM->PWM_TIME_PRD_CNTR_WR_REG_CH0_b.PWM_TIME_PRD_CNTR_WR_REG_CH0 = initVal;
      pMCPWM->PWM_TIME_PRD_WR_REG_CH0_b.PWM_TIME_PRD_REG_WR_VALUE_CH0     = period;
      break;
    case PWM_CHNL_1:
      pMCPWM->PWM_TIME_PRD_CNTR_WR_REG_CH1_b.PWM_TIME_PRD_CNTR_WR_REG_CH1 = initVal;
      pMCPWM->PWM_TIME_PRD_WR_REG_CH1_b.PWM_TIME_PRD_REG_WR_VALUE_CH1     = period;
      break;
    case PWM_CHNL_2:
      pMCPWM->PWM_TIME_PRD_CNTR_WR_REG_CH2_b.PWM_TIME_PRD_CNTR_WR_REG_CH2 = initVal;
      pMCPWM->PWM_TIME_PRD_WR_REG_CH2_b.PWM_TIME_PRD_REG_WR_VALUE_CH2     = period;
      break;
    case PWM_CHNL_3:
      pMCPWM->PWM_TIME_PRD_CNTR_WR_REG_CH3_b.PWM_TIME_PRD_CNTR_WR_REG_CH3 = initVal;
      pMCPWM->PWM_TIME_PRD_WR_REG_CH3_b.PWM_TIME_PRD_REG_WR_VALUE_CH3     = period;
      break;
    default:
      return ERROR_PWM_INVALID_CHNLNUM;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn        void mcpwm_special_event_trigger_config(RSI_MCPWM_T *pMCPWM, boolean_t svtDir, RSI_MCPWM_SVT_CONFIG_T *pMCPWMSVTConfig)
 * @brief	  This API is used to configure special event trigger generation for required MCPWM channel which
 *								allows the A/D converter to be synchronized to the PWM time base.
 * @param[in] pMCPWM            :  Pointer to the MCPWM instance register area
 * @param[in] svtDir            :  Special event trigger for time base direction
 *                                    -  1 : A special event trigger will occur when
 *                                           PWM time base is counting down
 *                                    -  0 : A special event trigger will occur when
 *                                           PWM time base is counting up
 * @param[in] pMCPWMSVTConfig   :  Pointer to the structure of type \ref RSI_MCPWM_SVT_CONFIG_T
 *                                   -  svtPostscalar : PWM special event trigger output postscale value
 *                                                         \n 0000 means 1:1 post scale
 *																										              \n to
 *                                                          \n 1111 means  1:16 post scale
 *                                     -  svtCompareVal : Compare value to generate special event trigger.
 *                                     -  svtChannel    : Out of four channels, we can generate special event
 *                                                       for one channel
 * @return 	 none
 */
void mcpwm_special_event_trigger_config(RSI_MCPWM_T *pMCPWM, boolean_t svtDir, RSI_MCPWM_SVT_CONFIG_T *pMCPWMSVTConfig)
{
  // sets counter direction to generate special event trigger
  if (svtDir == COUNTDOWN) {
    pMCPWM->PWM_SVT_CTRL_SET_REG = (1 << 1);
  }
  if (svtDir == COUNTUP) {
    pMCPWM->PWM_SVT_CTRL_RESET_REG = (1 << 1);
  }
  // selct the channel to generate SVT
  pMCPWM->PWM_TIME_PRD_COMMON_REG_b.PWM_TIME_PRD_COMMON_TIMER_VALUE =
    (unsigned int)(pMCPWMSVTConfig->svtChannel & 0x03);
  // sets post scalar
  pMCPWM->PWM_SVT_PARAM_REG_b.SVT_POSTSCALER_SELECT = (unsigned int)(pMCPWMSVTConfig->svtPostscalar & 0x0F);
  // compare value for SVT
  pMCPWM->PWM_SVT_COMPARE_VALUE_REG = pMCPWMSVTConfig->svtCompareVal;
}

/*==============================================*/
/**
 * @fn            rsi_error_t mcpwm_dead_time_value_set(RSI_MCPWM_T *pMCPWM, RSI_MCPWM_DT_CONFIG_T *pMCPWMDeadTimeConfig, uint8_t chnlNum)
 * @brief		  This API is used to Configure Dead time insertion parameters for MCPWM
 * @param[in]	  pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]     pMCPWMDeadTimeConfig  :  Pointer to the structure of type \ref RSI_MCPWM_DT_CONFIG_T
 *                                        - counterSelect :  Selects coutner A/B for deadtime insertion.
 *                                     preScaleA     :  Dead time prescale selection bits for unit A is below like
 *                                        - 00  :  clock period for dead time unit A is 1x input clock period
 *                                        - 01  :  clock period for dead time unit A is 2x input clock period
 *                                        - 10  :  clock period for dead time unit A is 4x input clock period
 *                                        - 11  :  clock period for dead time unit A is 8x input clock period
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
 *                 \n \ref RSI_OK        : If process is done successfully
 */
rsi_error_t mcpwm_dead_time_value_set(RSI_MCPWM_T *pMCPWM, RSI_MCPWM_DT_CONFIG_T *pMCPWMDeadTimeConfig, uint8_t chnlNum)
{
  if ((pMCPWMDeadTimeConfig->counterSelect == COUNTER_A) || (pMCPWMDeadTimeConfig->counterSelect == COUNTER_B)) {
    // Sets prescale for counter A or counter B
    switch (chnlNum) {
      case PWM_CHNL_0: {
        if (pMCPWMDeadTimeConfig->counterSelect == COUNTER_A) {
          if ((pMCPWMDeadTimeConfig->preScaleA >= 0) && (pMCPWMDeadTimeConfig->preScaleA <= 3)) {
            pMCPWM->PWM_DEADTIME_PRESCALE_SELECT_A_b.DEADTIME_PRESCALE_SELECT_A =
              (uint8_t)(pMCPWMDeadTimeConfig->preScaleA << 0);
          } else {
            return ERROR_PWM_INVALID_ARG;
          }
        }
        if (pMCPWMDeadTimeConfig->counterSelect == COUNTER_B) {
          if ((pMCPWMDeadTimeConfig->preScaleB >= 0) && (pMCPWMDeadTimeConfig->preScaleB <= 3)) {
            pMCPWM->PWM_DEADTIME_PRESCALE_SELECT_B_b.DEADTIME_PRESCALE_SELECT_B =
              (uint8_t)(pMCPWMDeadTimeConfig->preScaleB << 0);
          } else {
            return ERROR_PWM_INVALID_ARG;
          }
        }
      } break;
      case PWM_CHNL_1: {
        if (pMCPWMDeadTimeConfig->counterSelect == COUNTER_A) {
          if ((pMCPWMDeadTimeConfig->preScaleA >= 0) && (pMCPWMDeadTimeConfig->preScaleA <= 3)) {
            pMCPWM->PWM_DEADTIME_PRESCALE_SELECT_A_b.DEADTIME_PRESCALE_SELECT_A =
              (uint8_t)(pMCPWMDeadTimeConfig->preScaleA << 2);
          } else {
            return ERROR_PWM_INVALID_ARG;
          }
        }
        if (pMCPWMDeadTimeConfig->counterSelect == COUNTER_B) {
          if ((pMCPWMDeadTimeConfig->preScaleB >= 0) && (pMCPWMDeadTimeConfig->preScaleB <= 3)) {
            pMCPWM->PWM_DEADTIME_PRESCALE_SELECT_B_b.DEADTIME_PRESCALE_SELECT_B =
              (uint8_t)(pMCPWMDeadTimeConfig->preScaleB << 2);
          } else {
            return ERROR_PWM_INVALID_ARG;
          }
        }
      } break;
      case PWM_CHNL_2: {
        if (pMCPWMDeadTimeConfig->counterSelect == COUNTER_A) {
          if ((pMCPWMDeadTimeConfig->preScaleA >= 0) && (pMCPWMDeadTimeConfig->preScaleA <= 3)) {
            pMCPWM->PWM_DEADTIME_PRESCALE_SELECT_A_b.DEADTIME_PRESCALE_SELECT_A =
              (uint8_t)(pMCPWMDeadTimeConfig->preScaleA << 4);
          } else {
            return ERROR_PWM_INVALID_ARG;
          }
        }
        if (pMCPWMDeadTimeConfig->counterSelect == COUNTER_B) {
          if ((pMCPWMDeadTimeConfig->preScaleB >= 0) && (pMCPWMDeadTimeConfig->preScaleB <= 3)) {
            pMCPWM->PWM_DEADTIME_PRESCALE_SELECT_B_b.DEADTIME_PRESCALE_SELECT_B =
              (uint8_t)(pMCPWMDeadTimeConfig->preScaleB << 4);
          } else {
            return ERROR_PWM_INVALID_ARG;
          }
        }
      } break;
      case PWM_CHNL_3: {
        if (pMCPWMDeadTimeConfig->counterSelect == COUNTER_A) {
          if ((pMCPWMDeadTimeConfig->preScaleA >= 0) && (pMCPWMDeadTimeConfig->preScaleA <= 3)) {
            pMCPWM->PWM_DEADTIME_PRESCALE_SELECT_A_b.DEADTIME_PRESCALE_SELECT_A =
              (uint8_t)(pMCPWMDeadTimeConfig->preScaleA << 6);
          } else {
            return ERROR_PWM_INVALID_ARG;
          }
        }
        if (pMCPWMDeadTimeConfig->counterSelect == COUNTER_B) {
          if ((pMCPWMDeadTimeConfig->preScaleB >= 0) && (pMCPWMDeadTimeConfig->preScaleB <= 3)) {
            pMCPWM->PWM_DEADTIME_PRESCALE_SELECT_B_b.DEADTIME_PRESCALE_SELECT_B =
              (uint8_t)(pMCPWMDeadTimeConfig->preScaleB << 6);
          } else {
            return ERROR_PWM_INVALID_ARG;
          }
        }
      } break;
      default:
        return ERROR_PWM_INVALID_CHNLNUM;
    }
    // Sets DeadTime for counter A
    if (pMCPWMDeadTimeConfig->counterSelect == COUNTER_A) {
      pMCPWM->PWM_DEADTIME[chnlNum].PWM_DEADTIME_A_b.DEADTIME_A_CH =
        (volatile unsigned int)(pMCPWMDeadTimeConfig->deadTimeA & 0x3F);
    }
    // Sets DeadTime for counter B
    if (pMCPWMDeadTimeConfig->counterSelect == COUNTER_B) {
      pMCPWM->PWM_DEADTIME[chnlNum].PWM_DEADTIME_B_b.DEADTIME_B_CH =
        (volatile unsigned int)(pMCPWMDeadTimeConfig->deadTimeB & 0x3F);
    }
  } else {
    return ERROR_PWM_INVALID_ARG;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          rsi_error_t mcpwm_channel_reset(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum)
 * @brief		This API is used to reset the required channel of MCPWM.
 * @param[in]	pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]	chnlNum	:  Channel number(0 to 3)
 * @return 		\ref ERROR_PWM_INVALID_CHNLNUM : If channel is invalid
 *               \n \ref RSI_OK            : If process is done successfully.
 */
rsi_error_t mcpwm_channel_reset(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum)
{
  // Resets operation of MCPWM channel
  switch (chnlNum) {
    case PWM_CHNL_0:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH0_b.PWM_SFT_RST = ENABLE;
      break;
    case PWM_CHNL_1:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH1_b.PWM_SFT_RST = ENABLE;
      break;
    case PWM_CHNL_2:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH2_b.PWM_SFT_RST = ENABLE;
      break;
    case PWM_CHNL_3:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH3_b.PWM_SFT_RST = ENABLE;
      break;
    default:
      return ERROR_PWM_INVALID_CHNLNUM;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          rsi_error_t mcpwm_counter_reset(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum)
 * @brief		This API is used to reset the counter from required channel of MCPWM
 * @param[in]	pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]	chnlNum	:  Channel number(0 to 3)
 * @return      \ref ERROR_PWM_INVALID_CHNLNUM : If channel number is invalid
 *               \n \ref RSI_OK            : If process is done successfully
 */
rsi_error_t mcpwm_counter_reset(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum)
{
  // resets counter operations
  switch (chnlNum) {
    case PWM_CHNL_0:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH0_b.PWM_TIME_PRD_CNTR_RST_FRM_REG = ENABLE;
      break;
    case PWM_CHNL_1:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH1_b.PWM_TIME_PRD_CNTR_RST_FRM_REG = ENABLE;
      break;
    case PWM_CHNL_2:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH2_b.PWM_TIME_PRD_CNTR_RST_FRM_REG = ENABLE;
      break;
    case PWM_CHNL_3:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH3_b.PWM_TIME_PRD_CNTR_RST_FRM_REG = ENABLE;
      break;
    default:
      return ERROR_PWM_INVALID_CHNLNUM;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          rsi_error_t mcpwm_period_control_config(RSI_MCPWM_T *pMCPWM, uint32_t postScale, uint32_t preScale, uint8_t chnlNum)
 * @brief		This API is used to set base time period control for the required MCPWM channel.
 * @param[in]	pMCPWM     :  Pointer to the MCPWM instance register area
 * @param[in]	postScale  :  Time base output postscale bits ,possible values are as below
 *                             -  0000 means 1:1
 *                             -  0001 means 1:2
 *									\n To
 *                             -  1111 means 1:16
 * @param[in]	preScale  :   Base timer input clock prescale select value,possible values are as below
 *                             -  000 : 1x input clock period
 *                             -  010 : 4x input clock period
 *                             -  011 : 8x input clock period
 *                             -  100 : 16x input clock period
 *                             -  101 : 32x input clock period
 *                             -  110 : 64x input clock period
 * @param[in]	chnlNum	 :  Channel number(0 to 3)
 * @return      \ref ERROR_PWM_INVALID_CHNLNUM : If channel number is invalid
 *              \n RSI_OK                 : If process is done successfully
 */
rsi_error_t mcpwm_period_control_config(RSI_MCPWM_T *pMCPWM, uint32_t postScale, uint32_t preScale, uint8_t chnlNum)
{
  // sets prescale and post scale values
  switch (chnlNum) {
    case PWM_CHNL_0:
      pMCPWM->PWM_TIME_PRD_PARAM_REG_CH0_b.PWM_TIME_PRD_POST_SCALAR_VALUE_CH0 = (unsigned int)(postScale & 0x0F);
      pMCPWM->PWM_TIME_PRD_PARAM_REG_CH0_b.PWM_TIME_PRD_PRE_SCALAR_VALUE_CH0  = (unsigned int)(preScale & 0x07);
      break;
    case PWM_CHNL_1:
      pMCPWM->PWM_TIME_PRD_PARAM_REG_CH1_b.PWM_TIME_PRD_POST_SCALAR_VALUE_CH1 = (unsigned int)(postScale & 0x0F);
      pMCPWM->PWM_TIME_PRD_PARAM_REG_CH1_b.PWM_TIME_PRD_PRE_SCALAR_VALUE_CH1  = (unsigned int)(preScale & 0x07);
      break;
    case PWM_CHNL_2:
      pMCPWM->PWM_TIME_PRD_PARAM_REG_CH2_b.PWM_TIME_PRD_POST_SCALAR_VALUE_CH2 = (unsigned int)(postScale & 0x0F);
      pMCPWM->PWM_TIME_PRD_PARAM_REG_CH2_b.PWM_TIME_PRD_PRE_SCALAR_VALUE_CH2  = (unsigned int)(preScale & 0x07);
      break;
    case PWM_CHNL_3:
      pMCPWM->PWM_TIME_PRD_PARAM_REG_CH3_b.PWM_TIME_PRD_POST_SCALAR_VALUE_CH3 = (unsigned int)(postScale & 0x07);
      pMCPWM->PWM_TIME_PRD_PARAM_REG_CH3_b.PWM_TIME_PRD_PRE_SCALAR_VALUE_CH3  = (unsigned int)(preScale & 0x07);
      break;
    default:
      return ERROR_PWM_INVALID_CHNLNUM;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn            rsi_error_t mcpwm_fault_avalue_set(RSI_MCPWM_T *pMCPWM, uint8_t pwmOutput, uint8_t value)
 * @brief		  This API is used to set fault A pin output value to be overridden when fault condition occurs.
 * @param[in]	  pMCPWM    : Pointer to the MCPWM instance register area
 * @param[in]	  pwmOutput : PWM outputs are as below
 *                           -  0 : L0
 *                           -  1 : L1
 *                           -  2 : L2
 *                           -  3 : L3
 *                           -  4 : H0
 *                           -  5 : H1
 *                           -  6 : H2
 *                           -  7 : H3
 * @param[in]   value     : Fault input A PWM output override value below
 *												 -  1 : PWM output pin is driven active on an external fault input A event
 *												 -  0 : PWM output pin is driven inactive on an external fault input A event
 * @return      \ref ERROR_PWM_INVALID_PWMOUT : If pwmOutput is invalid
 *               \n RSI_OK                : If process is done successfully
 */
rsi_error_t mcpwm_fault_avalue_set(RSI_MCPWM_T *pMCPWM, uint8_t pwmOutput, uint8_t value)
{
  switch (pwmOutput) {
    case PWM_OUTPUT_L0:
      pMCPWM->PWM_FLT_A_OVERRIDE_VALUE_REG_b.PWM_FLT_A_OVERRIDE_VALUE_L0 = (unsigned int)(value & 0x01);
      break;
    case PWM_OUTPUT_H0:
      pMCPWM->PWM_FLT_A_OVERRIDE_VALUE_REG_b.PWM_FLT_A_OVERRIDE_VALUE_H0 = (unsigned int)(value & 0x01);
      break;
    case PWM_OUTPUT_L1:
      pMCPWM->PWM_FLT_A_OVERRIDE_VALUE_REG_b.PWM_FLT_A_OVERRIDE_VALUE_L1 = (unsigned int)(value & 0x01);
      break;
    case PWM_OUTPUT_H1:
      pMCPWM->PWM_FLT_A_OVERRIDE_VALUE_REG_b.PWM_FLT_A_OVERRIDE_VALUE_H1 = (unsigned int)(value & 0x01);
      break;
    case PWM_OUTPUT_L2:
      pMCPWM->PWM_FLT_A_OVERRIDE_VALUE_REG_b.PWM_FLT_A_OVERRIDE_VALUE_L2 = (unsigned int)(value & 0x01);
      break;
    case PWM_OUTPUT_H2:
      pMCPWM->PWM_FLT_A_OVERRIDE_VALUE_REG_b.PWM_FLT_A_OVERRIDE_VALUE_H2 = (unsigned int)(value & 0x01);
      break;
    case PWM_OUTPUT_L3:
      pMCPWM->PWM_FLT_A_OVERRIDE_VALUE_REG_b.PWM_FLT_A_OVERRIDE_VALUE_L3 = (unsigned int)(value & 0x01);
      break;
    case PWM_OUTPUT_H3:
      pMCPWM->PWM_FLT_A_OVERRIDE_VALUE_REG_b.PWM_FLT_A_OVERRIDE_VALUE_H3 = (unsigned int)(value & 0x01);
      break;
    default:
      return ERROR_PWM_INVALID_PWMOUT;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          rsi_error_t mcpwm_fault_bvalue_set(RSI_MCPWM_T *pMCPWM, uint8_t pwmOutput, uint8_t value)
 * @brief		    This API is used to set fault B pin output value to
 *								be overridden when fault condition occurs
 * @param[in]	  pMCPWM    : Pointer to the MCPWM instance register area
 * @param[in]	  pwmOutput : PWM outputs are as below
 *                           -  0 : L0
 *                           -  1 : L1
 *                           -  2 : L2
 *                           -  3 : L3
 *                           -  4 : H0
 *                           -  5 : H1
 *                           -  6 : H2
 *                           -  7 : H3
 * @param[in]   value     : Fault input B PWM output override value below
 *													  -  1 : PWM output pin is driven active on an external fault input B event
 *													  -  0 : PWM output pin is driven inactive on an external fault input B event
 * @return      \ref ERROR_PWM_INVALID_PWMOUT : If pwmOutput is invalid
 *               \n RSI_OK                : If process is done successfully
 */
rsi_error_t mcpwm_fault_bvalue_set(RSI_MCPWM_T *pMCPWM, uint8_t pwmOutput, uint8_t value)
{
  switch (pwmOutput) {
    case PWM_OUTPUT_L0:
      pMCPWM->PWM_FLT_B_OVERRIDE_VALUE_REG_b.PWM_FLT_B_OVERRIDE_VALUE_L0 = (unsigned int)(value & 0x01);
      break;
    case PWM_OUTPUT_H0:
      pMCPWM->PWM_FLT_B_OVERRIDE_VALUE_REG_b.PWM_FLT_B_OVERRIDE_VALUE_H0 = (unsigned int)(value & 0x01);
      break;
    case PWM_OUTPUT_L1:
      pMCPWM->PWM_FLT_B_OVERRIDE_VALUE_REG_b.PWM_FLT_B_OVERRIDE_VALUE_L1 = (unsigned int)(value & 0x01);
      break;
    case PWM_OUTPUT_H1:
      pMCPWM->PWM_FLT_B_OVERRIDE_VALUE_REG_b.PWM_FLT_B_OVERRIDE_VALUE_H1 = (unsigned int)(value & 0x01);
      break;
    case PWM_OUTPUT_L2:
      pMCPWM->PWM_FLT_B_OVERRIDE_VALUE_REG_b.PWM_FLT_B_OVERRIDE_VALUE_L2 = (unsigned int)(value & 0x01);
      break;
    case PWM_OUTPUT_H2:
      pMCPWM->PWM_FLT_B_OVERRIDE_VALUE_REG_b.PWM_FLT_B_OVERRIDE_VALUE_H2 = (unsigned int)(value & 0x01);
      break;
    case PWM_OUTPUT_L3:
      pMCPWM->PWM_FLT_B_OVERRIDE_VALUE_REG_b.PWM_FLT_B_OVERRIDE_VALUE_L3 = (unsigned int)(value & 0x01);
      break;
    case PWM_OUTPUT_H3:
      pMCPWM->PWM_FLT_B_OVERRIDE_VALUE_REG_b.PWM_FLT_B_OVERRIDE_VALUE_H3 = (unsigned int)(value & 0x01);
      break;
    default:
      return ERROR_PWM_INVALID_PWMOUT;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          rsi_error_t mcpwm_set_base_timer_mode(RSI_MCPWM_T *pMCPWM, uint8_t mode, uint8_t chnlNum)
 * @brief		This API is used to set the mode of base timer for required channel
 * @param[in]	pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]	mode    :  Base timer operating mode is below
 *                          -  000 : free running mode
 *                          -  001 : single event mode
 *                          -  010 : down count mode
 *                          -  100 : up/down mode
 *                          -  101 : up/dowm mode with interrupts for double PWM updates
 * @param[in]   chnlNum : Channel number(0 to 3)
 * @return      \ref ERROR_PWM_INVALID_CHNLNUM : If channel number is invalid
 *              \n \ref RSI_OK                 : If process is done successfully
 */
rsi_error_t mcpwm_set_base_timer_mode(RSI_MCPWM_T *pMCPWM, uint8_t mode, uint8_t chnlNum)
{
  switch (chnlNum) {
    case 0:
      pMCPWM->PWM_TIME_PRD_PARAM_REG_CH0_b.TMR_OPEARATING_MODE_CH0 = (unsigned int)(mode & 0x07);
      break;
    case 1:
      pMCPWM->PWM_TIME_PRD_PARAM_REG_CH1_b.TMR_OPEARATING_MODE_CH1 = (unsigned int)(mode & 0x07);
      break;
    case 2:
      pMCPWM->PWM_TIME_PRD_PARAM_REG_CH2_b.TMR_OPEARATING_MODE_CH2 = (unsigned int)(mode & 0x07);
      break;
    case 3:
      pMCPWM->PWM_TIME_PRD_PARAM_REG_CH3_b.TMR_OPEARATING_MODE_CH3 = (unsigned int)(mode & 0x07);
      break;
    default:
      return ERROR_PWM_INVALID_CHNLNUM;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          rsi_error_t mcpwm_set_output_mode(RSI_MCPWM_T *pMCPWM, boolean_t mode, uint8_t chnlNum)
 * @brief		This API is used to set output mode for the MCPWM
 * @param[in]	pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]	mode    :  If input is one then complementary output mode if zero then independent output mode.
 * @param[in]   chnlNum :  Channel number(0 to 3)
 * @return      \ref ERROR_PWM_INVALID_CHNLNUM : If channel number is invalid
 *               \n \ref RSI_OK                 : If process is done successfully
 */
rsi_error_t mcpwm_set_output_mode(RSI_MCPWM_T *pMCPWM, boolean_t mode, uint8_t chnlNum)
{
  switch (chnlNum) {
    case 0:
      if (mode) {
        pMCPWM->PWM_FLT_OVERRIDE_CTRL_SET_REG = (1 << 12);
      } else {
        pMCPWM->PWM_FLT_OVERRIDE_CTRL_RESET_REG = (1 << 12);
      }
      break;
    case 1:
      if (mode) {
        pMCPWM->PWM_FLT_OVERRIDE_CTRL_SET_REG = (1 << 13);
      } else {
        pMCPWM->PWM_FLT_OVERRIDE_CTRL_RESET_REG = (1 << 13);
      }
      break;
    case 2:
      if (mode) {
        pMCPWM->PWM_FLT_OVERRIDE_CTRL_SET_REG = (1 << 14);
      } else {
        pMCPWM->PWM_FLT_OVERRIDE_CTRL_RESET_REG = (1 << 14);
      }
      break;
    case 3:
      if (mode) {
        pMCPWM->PWM_FLT_OVERRIDE_CTRL_SET_REG = (1 << 15);
      } else {
        pMCPWM->PWM_FLT_OVERRIDE_CTRL_RESET_REG = (1 << 15);
      }
      break;
    default:
      return ERROR_PWM_INVALID_CHNLNUM;
  }
  return RSI_OK;
}

/*==============================================*/

/**
 * @fn          void mcpwm_set_output_polarity(RSI_MCPWM_T *pMCPWM, boolean_t polL, boolean_t polH)
 * @brief		This API is used to set output polarity for MCPWM
 * @param[in]	pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]   polL	  :  Ouput polarity for low side (L3, L2, L1, L0)
 * @param[in]   polH    :  Ouput polarity for high side (H3, H2, H1, H0)
 * @return 	    none
 */
void mcpwm_set_output_polarity(RSI_MCPWM_T *pMCPWM, boolean_t polL, boolean_t polH)
{
  if (polL) {
    pMCPWM->PWM_FLT_OVERRIDE_CTRL_SET_REG = (1 << 3);
  } else {
    pMCPWM->PWM_FLT_OVERRIDE_CTRL_RESET_REG = (1 << 3);
  }
  if (polH) {
    pMCPWM->PWM_FLT_OVERRIDE_CTRL_SET_REG = (1 << 2);
  } else {
    pMCPWM->PWM_FLT_OVERRIDE_CTRL_RESET_REG = (1 << 2);
  }
}
/*==============================================*/
/**
 * @fn          void mcpwm_interrupt_handler(RSI_MCPWM_T *pMCPWM, RSI_MCPWM_CALLBACK_T *pCallBack)
 * @brief		Handles all interrupt flags of MCPWM.
 * @param[in]	pMCPWM    :  Pointer to the MCPWM instance register area
 * @param[in]	pCallBack :  Pointer to the structure of type RSI_CALLBACK
 * @return      none
 */
void mcpwm_interrupt_handler(RSI_MCPWM_T *pMCPWM, RSI_MCPWM_CALLBACK_T *pCallBack)
{
  uint32_t intrStat = 0;

  intrStat = pMCPWM->PWM_INTR_STS;
  if ((pCallBack->cbFunc) != NULL) {
    if (intrStat & RSI_MCPWM_EVENT_RISE_TIME_PERIOD_MATCH_CH0) {
      RSI_MCPWM_InterruptClear(pMCPWM, RSI_MCPWM_EVENT_RISE_TIME_PERIOD_MATCH_CH0);
      pCallBack->cbFunc(RISE_TIME_PERIOD_MATCH_CH0);
    }
    if (intrStat & RSI_MCPWM_EVENT_TIME_PERIOD_MATCH_CH0) {
      RSI_MCPWM_InterruptClear(pMCPWM, RSI_MCPWM_EVENT_TIME_PERIOD_MATCH_CH0);
      pCallBack->cbFunc(TIME_PERIOD_MATCH_CH0);
    }
    if (intrStat & RSI_MCPWM_EVENT_FAULT_A) {
      RSI_MCPWM_InterruptClear(pMCPWM, RSI_MCPWM_EVENT_FAULT_A);
      pCallBack->cbFunc(FAULT_A);
    }
    if (intrStat & RSI_MCPWM_EVENT_FAULT_B) {
      RSI_MCPWM_InterruptClear(pMCPWM, RSI_MCPWM_EVENT_FAULT_B);
      pCallBack->cbFunc(FAULT_B);
    }
    if (intrStat & RSI_MCPWM_EVENT_RISE_TIME_PERIOD_MATCH_CH1) {
      RSI_MCPWM_InterruptClear(pMCPWM, RSI_MCPWM_EVENT_RISE_TIME_PERIOD_MATCH_CH1);
      pCallBack->cbFunc(RISE_TIME_PERIOD_MATCH_CH1);
    }
    if (intrStat & RSI_MCPWM_EVENT_TIME_PERIOD_MATCH_CH1) {
      RSI_MCPWM_InterruptClear(pMCPWM, RSI_MCPWM_EVENT_TIME_PERIOD_MATCH_CH1);
      pCallBack->cbFunc(TIME_PERIOD_MATCH_CH1);
    }
    if (intrStat & RSI_MCPWM_EVENT_RISE_TIME_PERIOD_MATCH_CH2) {
      RSI_MCPWM_InterruptClear(pMCPWM, RSI_MCPWM_EVENT_RISE_TIME_PERIOD_MATCH_CH2);
      pCallBack->cbFunc(RISE_TIME_PERIOD_MATCH_CH2);
    }
    if (intrStat & RSI_MCPWM_EVENT_TIME_PERIOD_MATCH_CH2) {
      RSI_MCPWM_InterruptClear(pMCPWM, RSI_MCPWM_EVENT_TIME_PERIOD_MATCH_CH2);
      pCallBack->cbFunc(TIME_PERIOD_MATCH_CH2);
    }
    if (intrStat & RSI_MCPWM_EVENT_RISE_TIME_PERIOD_MATCH_CH3) {
      RSI_MCPWM_InterruptClear(pMCPWM, RSI_MCPWM_EVENT_RISE_TIME_PERIOD_MATCH_CH3);
      pCallBack->cbFunc(RISE_TIME_PERIOD_MATCH_CH3);
    }
    if (intrStat & RSI_MCPWM_EVENT_TIME_PERIOD_MATCH_CH3) {
      RSI_MCPWM_InterruptClear(pMCPWM, RSI_MCPWM_EVENT_TIME_PERIOD_MATCH_CH3);
      pCallBack->cbFunc(TIME_PERIOD_MATCH_CH3);
    }
  } else {
  }
}
/** @} */
#ifdef __cplusplus
}
#endif
#else
typedef int dummy; // To remove empty translation unit warning.
#endif // PWM_ROMDRIVER_PRESENT
