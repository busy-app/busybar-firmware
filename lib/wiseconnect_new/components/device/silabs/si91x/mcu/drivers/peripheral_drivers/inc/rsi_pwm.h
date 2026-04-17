/***************************************************************************/ /**
* @file  rsi_pwm.h
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

// Includes Files

#include "rsi_ccp_common.h"
#include "base_types.h"
#include "rsi_error.h"

#ifndef RSI_PWM_H
#define RSI_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

#define RSI_MCPWM_API_VERSION RSI_DRIVER_VERSION_MAJOR_MINOR(2, 00)  // API version 0.1
#define RSI_MCPWM_DRV_VERSION RSI_DRIVER_VERSION_MAJOR_MINOR(00, 01) // driver version 0.1

// MCPWM Interrupt Flags
#define RSI_MCPWM_EVENT_RISE_TIME_PERIOD_MATCH_CH0 \
  (1UL << 0) // Time base match for 0th channel without considering postscaler
#define RSI_MCPWM_EVENT_TIME_PERIOD_MATCH_CH0 (1UL << 1) // Time base match for 0th channel which considering postscaler
#define RSI_MCPWM_EVENT_FAULT_A               (1UL << 2) // Fault A pin match
#define RSI_MCPWM_EVENT_FAULT_B               (1UL << 3) // Fault B pin match
#define RSI_MCPWM_EVENT_RISE_TIME_PERIOD_MATCH_CH1 \
  (1UL << 4) // Time base match for 1st channel without considering postscaler
#define RSI_MCPWM_EVENT_TIME_PERIOD_MATCH_CH1 \
  (1UL << 5) // Time base match for  1st channel which considering postscaler
#define RSI_MCPWM_EVENT_RISE_TIME_PERIOD_MATCH_CH2 \
  (1UL << 6) // Time base match for 2nd channel without considering postscaler
#define RSI_MCPWM_EVENT_TIME_PERIOD_MATCH_CH2 (1UL << 7) // Time base match for 2nd channel which considering postscaler
#define RSI_MCPWM_EVENT_RISE_TIME_PERIOD_MATCH_CH3 \
  (1UL << 8) // Time base match for 3rd channel without considering postscaler
#define RSI_MCPWM_EVENT_TIME_PERIOD_MATCH_CH3 \
  (1UL << 9) // Time base match for 3rd channel without considering postscaler

#define FLTA_MODE   (1UL << 0)
#define FLTB_MODE   (1UL << 1)
#define FLTA_ENABLE (1UL << 4)
#define FLTB_ENABLE (1UL << 8)

// MCPWM Events
#define RISE_TIME_PERIOD_MATCH_CH0 0x0 // Event for 0th channel without considering postscaler
#define TIME_PERIOD_MATCH_CH0      0x1 // Event for 0th channel which considering postscaler
#define FAULT_A                    0x2 // Fault A pin Event
#define FAULT_B                    0x3 // Fault B pin Event
#define RISE_TIME_PERIOD_MATCH_CH1 0x4 // Event for 1st channel without considering postscaler
#define TIME_PERIOD_MATCH_CH1      0x5 // Event for 1st channel which considering postscaler
#define RISE_TIME_PERIOD_MATCH_CH2 0x6 // Event for 2nd channel without considering postscaler
#define TIME_PERIOD_MATCH_CH2      0x7 // Event for 2nd channel which considering postscaler
#define RISE_TIME_PERIOD_MATCH_CH3 0x8 // Event for 3rd channel without considering postscaler
#define TIME_PERIOD_MATCH_CH3      0x9 // Event for 3rd channel which considering postscaler

#define COUNTER_A 0
#define COUNTER_B 1

// PWM Example defines
#define PWM_CHNL_0 0
#define PWM_CHNL_1 1
#define PWM_CHNL_2 2
#define PWM_CHNL_3 3

#define TMR_FREE_RUN_MODE         0x0
#define TMR_SINGLE_EVENT_MODE     0x1
#define TMR_DOWN_COUNT_MODE       0x2
#define TMR_UP_DOWN_MODE          0x3
#define TMR_UP_DOWN_DOUBLE_UPDATE 0x4

#define ONE_TIMER_FOR_EACH_CHNL 0
#define ONE_TIMER_FOR_ALL_CHNL  1

#define TIME_PERIOD_PRESCALE_1  0x0
#define TIME_PERIOD_PRESCALE_2  0x1
#define TIME_PERIOD_PRESCALE_4  0x2
#define TIME_PERIOD_PRESCALE_8  0x3
#define TIME_PERIOD_PRESCALE_16 0x4
#define TIME_PERIOD_PRESCALE_32 0x5
#define TIME_PERIOD_PRESCALE_64 0x6

#define TIME_PERIOD_POSTSCALE_1_1  0x0
#define TIME_PERIOD_POSTSCALE_1_2  0x1
#define TIME_PERIOD_POSTSCALE_1_3  0x2
#define TIME_PERIOD_POSTSCALE_1_4  0x3
#define TIME_PERIOD_POSTSCALE_1_5  0x4
#define TIME_PERIOD_POSTSCALE_1_6  0x5
#define TIME_PERIOD_POSTSCALE_1_7  0x6
#define TIME_PERIOD_POSTSCALE_1_8  0x7
#define TIME_PERIOD_POSTSCALE_1_9  0x8
#define TIME_PERIOD_POSTSCALE_1_10 0x9
#define TIME_PERIOD_POSTSCALE_1_11 0xA
#define TIME_PERIOD_POSTSCALE_1_12 0xB
#define TIME_PERIOD_POSTSCALE_1_13 0xC
#define TIME_PERIOD_POSTSCALE_1_14 0xD
#define TIME_PERIOD_POSTSCALE_1_15 0xE
#define TIME_PERIOD_POSTSCALE_1_16 0xF

#define TMR0_PERIOD 0x1500
#define TMR1_PERIOD 0x1500
#define TMR2_PERIOD 0x1500
#define TMR3_PERIOD 0x1500

#define COUNTER_INIT_VAL 0

#define DUTYCYCLE_UPDATE_DIS0 (1 << 4)
#define DUTYCYCLE_UPDATE_DIS1 (1 << 5)
#define DUTYCYCLE_UPDATE_DIS2 (1 << 6)
#define DUTYCYCLE_UPDATE_DIS3 (1 << 7)

#define IMD_UPDATE_EN0 (1 << 0)
#define IMD_UPDATE_EN1 (1 << 1)
#define IMD_UPDATE_EN2 (1 << 2)
#define IMD_UPDATE_EN3 (1 << 3)

#define DUTYCYCLE_UPDATE_EN0 (1 << 4)
#define DUTYCYCLE_UPDATE_EN1 (1 << 5)
#define DUTYCYCLE_UPDATE_EN2 (1 << 6)
#define DUTYCYCLE_UPDATE_EN3 (1 << 7)

#define IMD_UPDATE_DIS0 (1 << 0)
#define IMD_UPDATE_DIS1 (1 << 1)
#define IMD_UPDATE_DIS2 (1 << 2)
#define IMD_UPDATE_DIS3 (1 << 3)

#define TMR0_DUTYCYCLE 0xA80
#define TMR1_DUTYCYCLE 0xA80
#define TMR2_DUTYCYCLE 0xA80
#define TMR3_DUTYCYCLE 0xA80

#define COMPLEMENT  1
#define INDEPENDENT 0

#define OUT_POL_H_HIGH (1 << 2)
#define OUT_POL_H_LOW  (0 << 2)

#define OUT_POL_L_HIGH (1 << 3)
#define OUT_POL_L_LOW  (0 << 3)

#define DT_SELECT_INACTIVE_CH0 (1 << 4)
#define DT_SELECT_INACTIVE_CH1 (1 << 5)
#define DT_SELECT_INACTIVE_CH2 (1 << 6)
#define DT_SELECT_INACTIVE_CH3 (1 << 7)

#define DT_SELECT_ACTIVE_CH0 (1 << 0)
#define DT_SELECT_ACTIVE_CH1 (1 << 1)
#define DT_SELECT_ACTIVE_CH2 (1 << 2)
#define DT_SELECT_ACTIVE_CH3 (1 << 3)

#define COUNTUP       0
#define COUNTDOWN     1
#define PWM_OUTPUT_L0 0
#define PWM_OUTPUT_L1 1
#define PWM_OUTPUT_L2 2
#define PWM_OUTPUT_L3 3
#define PWM_OUTPUT_H0 4
#define PWM_OUTPUT_H1 5
#define PWM_OUTPUT_H2 6
#define PWM_OUTPUT_H3 7

#define PWM_OUTPUT_MIN 0
#define PWM_OUTPUT_MAX 7

#define OVERRIDE_SYNC_EN  1
#define OVERRIDE_SYNC_DIS 0

#define OV_VALUE_0 1
#define OV_VALUE_1 0

#define DT_EN_CH0 (1 << 8)
#define DT_EN_CH1 (1 << 9)
#define DT_EN_CH2 (1 << 10)
#define DT_EN_CH3 (1 << 11)

#define DT_DIS_CH0 (1 << 8)
#define DT_DIS_CH1 (1 << 9)
#define DT_DIS_CH2 (1 << 10)
#define DT_DIS_CH3 (1 << 11)

typedef MCPWM_Type RSI_MCPWM_T;

// brief MCPWM Status
typedef struct {
  uint32_t counterDir; // Current Counter direction
  uint32_t counterVal; //  Counter current value
} RSI_MCPWM_STATUS_T;

// brief MCPWM Driver Capabilities.
typedef struct {
  uint32_t pwmOutputs : 4; // Number of PWM outputs
  uint32_t faultPins : 2;  // Number of fault input pins
} RSI_MCPWM_CAPABILITIES_T;

// brief MCPWM Callback structure
typedef struct {
  void (*cbFunc)(uint16_t flag); // Call back function pointer

} RSI_MCPWM_CALLBACK_T;

// brief  MCPWM Special Event trigger configuration parameters.
typedef struct {
  uint16_t svtPostscalar; // SVT PostScalar value
  uint16_t svtCompareVal; // SVT compare value to generate trigger for A/D
  uint8_t svtChannel;     // channel to generate SVT
} RSI_MCPWM_SVT_CONFIG_T;

// brief MCPWM DeadTime Configuration parameters.
typedef struct {
  uint8_t counterSelect; // Selects coutner A/B for deadtime insertion
  int8_t preScaleA;      // PreScale for counter A
  int8_t preScaleB;      // PreScale for counter B
  int8_t deadTimeA;      // Deadtime for counter A
  int8_t deadTimeB;      // Deadtime for counter B
} RSI_MCPWM_DT_CONFIG_T;

/*===================================================*/
/**
 * @fn            rsi_error_t RSI_MCPWM_ReadCounter(RSI_MCPWM_T *pMCPWM,uint16_t *counterVal,
                                              uint8_t chnlNum)
 * @brief     This API is used to read the counter current value
 * @param[in]   pMCPWM     : Pointer to the MCPWM instance register area
 * @param[in]     counterVal : counter value
 * @param[in]   chnlNum    : Channel number(0 to 3)
 * @return      \ref ERROR_PWM_INVALID_CHNLNUM : If channel is invalid.
                  \n \ref RSI_OK                 : If process is done successfully.  
 */
STATIC INLINE rsi_error_t RSI_MCPWM_ReadCounter(RSI_MCPWM_T *pMCPWM, uint16_t *counterVal, uint8_t chnlNum)
{
  // Gets Time period counter current value
  switch (chnlNum) {
    case PWM_CHNL_0:
      *counterVal = (uint16_t)(pMCPWM->PWM_TIME_PRD_CNTR_VALUE_CH0);
      break;
    case PWM_CHNL_1:
      *counterVal = (uint16_t)(pMCPWM->PWM_TIME_PRD_CNTR_VALUE_CH1);
      break;
    case PWM_CHNL_2:
      *counterVal = (uint16_t)(pMCPWM->PWM_TIME_PRD_CNTR_VALUE_CH2);
      break;
    case PWM_CHNL_3:
      *counterVal = (uint16_t)(pMCPWM->PWM_TIME_PRD_CNTR_VALUE_CH3);
      break;
    default:
      return ERROR_PWM_INVALID_CHNLNUM;
  }
  return RSI_OK;
}

/*===================================================*/
/**
 * @fn            rsi_error_t RSI_MCPWM_GetCounterDir(RSI_MCPWM_T *pMCPWM,uint8_t *counterDir,
                                                 uint8_t chnlNum)
 * @brief     This API is used to get time period counter direction status of required MCPWM channel
 * @param[in]   pMCPWM     :  Pointer to the MCPWM instance register area
 * @param[in]   chnlNum    :  Channel number(0 to 3)
 * @param[out]    counterDir :  Counter direction as up/down counter.
 * @return      \ref ERROR_PWM_INVALID_CHNLNUM : If channel number is invalid.
                \n \ref RSI_OK(0)              : If process is done successfully.
 */
STATIC INLINE rsi_error_t RSI_MCPWM_GetCounterDir(RSI_MCPWM_T *pMCPWM, uint8_t *counterDir, uint8_t chnlNum)
{
  // Gets the counter direction
  switch (chnlNum) {
    case PWM_CHNL_0:
      *counterDir = (uint8_t)(pMCPWM->PWM_TIME_PRD_STS_REG_CH0);
      break;
    case PWM_CHNL_1:
      *counterDir = (uint8_t)(pMCPWM->PWM_TIME_PRD_STS_REG_CH1);
      break;
    case PWM_CHNL_2:
      *counterDir = (uint8_t)(pMCPWM->PWM_TIME_PRD_STS_REG_CH2);
      break;
    case PWM_CHNL_3:
      *counterDir = (uint8_t)(pMCPWM->PWM_TIME_PRD_STS_REG_CH3);
      break;
    default:
      return ERROR_PWM_INVALID_CHNLNUM;
  }
  return RSI_OK;
}

/*===================================================*/
/**
 * @fn            void RSI_MCPWM_DeadTimeEnable(RSI_MCPWM_T *pMCPWM ,uint32_t flag)
 * @brief     Enables dead time insertion at rise edge or fall edge of any four channels
 * @param[in]   pMCPWM :  Pointer to the MCPWM instance register area
 * @param[in]   flag   :  This can be ORing of below values
                          -  DT_EN_CH0
                          -  DT_EN_CH1
                          -  DT_EN_CH2
                          -  DT_EN_CH3
 * @return      none
 */
STATIC INLINE void RSI_MCPWM_DeadTimeEnable(RSI_MCPWM_T *pMCPWM, uint32_t flag)
{
  pMCPWM->PWM_DEADTIME_CTRL_RESET_REG = flag;
}

/*===================================================*/
/**
 * @fn            void RSI_MCPWM_DeadTimeDisable(RSI_MCPWM_T *pMCPWM ,uint32_t flag)
 * @brief     This API is used to disable the dead time mode for the specified MCPWM generator.
 * @param[in]   pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]   flag    :  This can be ORing of below values
                           -  DT_DIS_CH0
                           -  DT_DIS_CH1
                           -  DT_DIS_CH2
                           -  DT_DIS_CH3
 * @return      none
 */
STATIC INLINE void RSI_MCPWM_DeadTimeDisable(RSI_MCPWM_T *pMCPWM, uint32_t flag)
{
  pMCPWM->PWM_DEADTIME_CTRL_SET_REG = flag;
}

/*===================================================*/
/**
 * @fn            void RSI_MCPWM_Dead_Time_Enable(RSI_MCPWM_T *pMCPWM ,uint32_t flag)
 * @brief     Enables dead time insertion at rise edge or fall edge of any four channels
 * @param[in]   pMCPWM :  Pointer to the MCPWM instance register area
 * @param[in]   flag   :  This can be ORing of below values
                          -  DT_EN_CH0
                          -  DT_EN_CH1
                          -  DT_EN_CH2
                          -  DT_EN_CH3
 * @return      none
 */
STATIC INLINE void RSI_MCPWM_Dead_Time_Enable(RSI_MCPWM_T *pMCPWM, uint32_t flag)
{
  pMCPWM->PWM_DEADTIME_CTRL_SET_REG = flag;
}

/*===================================================*/
/**
 * @fn            void RSI_MCPWM_Dead_Time_Disable(RSI_MCPWM_T *pMCPWM ,uint32_t flag)
 * @brief     This API is used to disable the dead time mode for the specified MCPWM generator.
 * @param[in]   pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]   flag    :  This can be ORing of below values
                           -  DT_DIS_CH0
                           -  DT_DIS_CH1
                           -  DT_DIS_CH2
                           -  DT_DIS_CH3
 * @return      none
 */
STATIC INLINE void RSI_MCPWM_Dead_Time_Disable(RSI_MCPWM_T *pMCPWM, uint32_t flag)
{
  pMCPWM->PWM_DEADTIME_CTRL_RESET_REG = flag;
}

/*===================================================*/
/**
 * @fn            void RSI_MCPWM_InterruptClear(RSI_MCPWM_T *pMCPWM, uint32_t clrFlag)
 * @brief     This API is used to clear the interrupts of MCPWM
 * @param[in]   pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]   clrFlag :  This can be logical OR of the below parameters
                           -  RISE_PWM_TIME_PERIOD_MATCH_CH0_ACK : Time base interrupt for 0th channel without considering postscaler 
                                          \n if bit is one then pwm time period match interrupt for 0th channel will 
                                            be cleared if bit zero then no effect.
                           -  PWM_TIME_PRD_MATCH_INTR_CH0_ACK  :  Time base interrupt for 0th channel which 
                                                                      considering postscaler 
                                          \n if bit is one then pwm time period match interrupt for 0th channel will 
                                            be cleared if bit zero then no effect.
                           -  FLT_A_INTR_ACK  :  Fault A pin interrupt  
                                          \n if bit is set pwm faultA interrupt will be cleared,if zero then ,no effect. 
                           -  FLT_A_INTR_ACK  :  Fault B pin interrupt  
                                          \n if bit is set pwm faultB interrupt will be cleared,if zero then ,no effect. 
                           -  RISE_PWM_TIME_PERIOD_MATCH_CH1_ACK  :  Time base interrupt for 
                                                                       1th channel without considering postscaler 
                                          \n if bit is one then pwm time period match interrupt for 1th channel will 
                                            be cleared if bit zero then no effect.
                           -  PWM_TIME_PRD_MATCH_INTR_CH1_ACK  :  Time base interrupt for 1th channel which 
                                                                      considering postscaler 
                                          \n if bit is one then pwm time period match interrupt for 1th channel will 
                                            be cleared if bit zero then no effect.
                           -  RISE_PWM_TIME_PERIOD_MATCH_CH2_ACK  :  Time base interrupt for 
                                                                       2nd channel without considering postscaler 
                                          \n if bit is one then pwm time period match interrupt for 1th channel will 
                                            be cleared if bit zero then no effect.
                           -  PWM_TIME_PRD_MATCH_INTR_CH2_ACK  :  Time base interrupt for 2nd channel which 
                                                                      considering postscaler 
                                          \n if bit is one then pwm time period match interrupt for 2nd channel will 
                                            be cleared if bit zero then no effect.
                           -  RISE_PWM_TIME_PERIOD_MATCH_CH3_ACK  :  Time base interrupt for 
                                                                       3rd channel without considering postscaler 
                                          \n if bit is one then pwm time period match interrupt for 3rd channel will 
                                            be cleared if bit zero then no effect.
                           - PWM_TIME_PRD_MATCH_INTR_CH3_ACK  :  Time base interrupt for 3rd channel which 
                                                                      considering postscaler 
                                          \n if bit is one then pwm time period match interrupt for 3rd channel will 
                                            be cleared if bit zero then no effect.
                         
 * @return      none
 */
STATIC INLINE void RSI_MCPWM_InterruptClear(RSI_MCPWM_T *pMCPWM, uint32_t clrFlag)
{
  pMCPWM->PWM_INTR_ACK = clrFlag;
}

/*===================================================*/
/**
 * @fn            void RSI_MCPWM_InterruptEnable(RSI_MCPWM_T *pMCPWM, uint16_t flag)
 * @brief     This API is used to enable the interrupts of MCPWM
 * @param[in]   pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]   flag    :  flag value can be logical OR of the below parameters
                           -  RISE_PWM_TIME_PERIOD_MATCH_CH0 : Time base interrupt for 0th channel without 
                                                                  considering postscaler 
                                                    \n if bit is 1 then pwm time period match interrupt for 0th channel 
                                                     will be cleared,if zero then no effect.
                           -  PWM_TIME_PRD_MATCH_INTR_CH0   :  Time base interrupt for 0th channel which 
                                                                   considering postscaler 
                                          \n if bit is 1 then pwm time period match interrupt for 0th channel
                                             will be cleared if zero then no effect.
                           -  FLT_A_INTR  :  Fault A pin interrupt
                                                 \n if bit is one then pwm faultA interrupt will be cleared if zero 
                                                     then no effect. 
                           -  FLT_B_INTR  :  Fault B pin interrupt
                                                 \n if bit is one pwm faultB interrupt will be cleared if zero
                                                    then no effect.
                           -  RISE_PWM_TIME_PERIOD_MATCH_CH1 : Time base interrupt for 1st channel without
                                                                   considering postscaler 
                                                       \n if bit is one pwm time period match interrupt for 1st channel
                                                          will be cleared if bit is zero no effect
                           -  PWM_TIME_PRD_MATCH_INTR_CH1   :  Time base interrupt for 1st channel which 
                                                                   considering postscaler 
                                          \n if bit is 1 then pwm time period match interrupt for 1st channel
                                             will be cleared if zero then no effect.
                           -  RISE_PWM_TIME_PERIOD_MATCH_CH2 : Time base interrupt for 2nd channel without 
                                                                   considering postscaler
                                                       \n if bit is one pwm time period match interrupt for 2nd channel
                                                          will be cleared if bit is zero no effect
                           -  PWM_TIME_PRD_MATCH_INTR_CH2  :   Time base interrupt for 2nd channel which 
                                                                   considering postscaler
                                                       \n if bit is one pwm time period match interrupt for 2nd channel
                                                          will be cleared if bit is zero no effect
                           -  RISE_PWM_TIME_PERIOD_MATCH_CH3 : Time base interrupt for 3rd channel without 
                                                                   considering postscaler
                                                       \n if bit is one pwm time period match interrupt for 3rd channel
                                                          will be cleared if bit is zero no effect
                           -  PWM_TIME_PRD_MATCH_INTR_CH3   :  Time base interrupt for 3rd channel which considering
                                                                   postscaler
                                                       \n if bit is one pwm time period match interrupt for 3rd channel
                                                          will be cleared if bit is zero no effect
 * @return      none
 */
STATIC INLINE void RSI_MCPWM_InterruptEnable(RSI_MCPWM_T *pMCPWM, uint16_t flag)
{
  pMCPWM->PWM_INTR_UNMASK = flag;
}

/*===================================================*/
/**
 * @fn          void RSI_MCPWM_InterruptDisable(RSI_MCPWM_T *pMCPWM, uint16_t flag)
 * @brief       This API is used to disable the interrupts of MCPWM
 * @param[in]   pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]   flag    :  flag value can be logical OR of the below parameters
                           -  RISE_PWM_TIME_PERIOD_MATCH_CH0 : Time base interrupt for 0th channel without 
                                                                  considering postscaler 
                                                    \n if bit is 1 then pwm time period match interrupt for 0th channel 
                                                     will be cleared,if zero then no effect.
                           -  PWM_TIME_PRD_MATCH_INTR_CH0   :  Time base interrupt for 0th channel which 
                                                                   considering postscaler 
                                          \n if bit is 1 then pwm time period match interrupt for 0th channel
                                             will be cleared if zero then no effect.
                           -  FLT_A_INTR  :  Fault A pin interrupt
                                                 \n if bit is one then pwm faultA interrupt will be cleared if zero 
                                                     then no effect. 
                           -  FLT_B_INTR  :  Fault B pin interrupt
                                                 \n if bit is one pwm faultB interrupt will be cleared if zero
                                                    then no effect.
                           -  RISE_PWM_TIME_PERIOD_MATCH_CH1 : Time base interrupt for 1st channel without
                                                                   considering postscaler 
                                                       \n if bit is one pwm time period match interrupt for 1st channel
                                                          will be cleared if bit is zero no effect
                           -  PWM_TIME_PRD_MATCH_INTR_CH1   :  Time base interrupt for 1st channel which 
                                                                   considering postscaler 
                                          \n if bit is 1 then pwm time period match interrupt for 0th channel
                                             will be cleared if zero then no effect.
                           -  RISE_PWM_TIME_PERIOD_MATCH_CH2 : Time base interrupt for 2nd channel without 
                                                                   considering postscaler
                                                       \n if bit is one pwm time period match interrupt for 2nd channel
                                                          will be cleared if bit is zero no effect
                           -  PWM_TIME_PRD_MATCH_INTR_CH2  :   Time base interrupt for 2nd channel which 
                                                                   considering postscaler
                                                       \n if bit is one pwm time period match interrupt for 2nd channel
                                                          will be cleared if bit is zero no effect
                           -  RISE_PWM_TIME_PERIOD_MATCH_CH3 : Time base interrupt for 3rd channel without 
                                                                   considering postscaler
                                                       \n if bit is one pwm time period match interrupt for 3rd channel
                                                          will be cleared if bit is zero no effect
                           -  PWM_TIME_PRD_MATCH_INTR_CH3   :  Time base interrupt for 3rd channel which considering
                                                                   postscaler
                                                       \n if bit is one pwm time period match interrupt for 3rd channel
                                                          will be cleared if bit is zero no effect
 * @return      none
 */
STATIC INLINE void RSI_MCPWM_InterruptDisable(RSI_MCPWM_T *pMCPWM, uint16_t flag)
{
  pMCPWM->PWM_INTR_MASK = flag;
}

/*===================================================*/
/**
 * @fn            uint16_t RSI_PWM_GetInterruptStatus(RSI_MCPWM_T *pMCPWM, uint16_t flag)
 * @brief     This API is used to get the interrupt status of interrupt flags of MCPWM.
 * @param[in]   pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]   flag    :  Flag value
 * @return      Interrupt status of required interrupt flag
 */
STATIC INLINE uint16_t RSI_PWM_GetInterruptStatus(RSI_MCPWM_T *pMCPWM, uint16_t flag)
{
  return (uint16_t)(pMCPWM->PWM_INTR_STS & flag);
}

/*===================================================*/
/**
 * @fn          rsi_error_t RSI_MCPWM_DutyCycleControlSet(RSI_MCPWM_T *pMCPWM,uint32_t value,uint8_t chnlNum)
 * @brief		    This API is used to set duty cycle control parameters for the required MCPWM channel
 * @param[in]	  pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]	  value   :  This can be logical OR of below parameters
							 - IMDT_DUTYCYCLE_UPDATE_EN : Enable to update the duty cycle immediately
							 - DUTYCYCLE_UPDATE_DISABLE : Duty cycle register updation disable
 * @param[in]	  chnlNum	:  Channel number(0 to 3)
 * @return 		  ERROR_PWM_INVALID_CHNLNUM : If channel is invalid.
                \n RSI_OK                 : If process is done successfully.
 */
STATIC INLINE rsi_error_t RSI_MCPWM_DutyCycleControlSet(RSI_MCPWM_T *pMCPWM, uint32_t value, uint8_t chnlNum)
{
  if (chnlNum <= PWM_CHNL_3) {
    pMCPWM->PWM_DUTYCYCLE_CTRL_SET_REG = value;
  } else {
    return ERROR_PWM_INVALID_CHNLNUM;
  }
  return RSI_OK;
}

/*===================================================*/
/**
 * @fn            rsi_error_t RSI_MCPWM_DutyCycleControlReset(RSI_MCPWM_T *pMCPWM,uint32_t value, uint8_t chnlNum)
 * @brief		  This API is used to reset the duty cycle control parameters of required MCPWM channel 
 * @param[in]	  pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]	  value   :  This can be logical OR of below parameters
                           - IMDT_DUTYCYCLE_UPDATE_EN :  Enable to update the duty cycle immediately
                           - DUTYCYCLE_UPDATE_DISABLE :  Duty cycle register updation disable
 * @param[in]   chnlNum  Channel number(0 to 3)
 * @return        \ref ERROR_PWM_INVALID_CHNLNUM : If channel number is invalid
                  \n \ref RSI_OK                 : If process is done successfully.
 */
STATIC INLINE rsi_error_t RSI_MCPWM_DutyCycleControlReset(RSI_MCPWM_T *pMCPWM, uint32_t value, uint8_t chnlNum)
{
  if (chnlNum <= PWM_CHNL_3) {
    pMCPWM->PWM_DUTYCYCLE_CTRL_RESET_REG = value;
  } else {
    return ERROR_PWM_INVALID_CHNLNUM;
  }
  return RSI_OK;
}

/*===================================================*/
/**
 * @fn            rsi_error_t RSI_MCPWM_OutputOverrideEnable(RSI_MCPWM_T *pMCPWM, uint8_t pwmOutput)
 * @brief		  This API is used to enable the output override operation of MCPWM
 * @param[in]	  pMCPWM    :  Pointer to the MCPWM instance register area
 * @param[in]	  pwmOutput :  Pwm output over ride,possible values are as below
                             - 0 : L0
                             - 1 : L1
                             - 2 : L2
                             - 3 : L3
                             - 4 : H0
                             - 5 : H1
                             - 6 : H2
                             - 7 : H3
  * @return     \ref ERROR_PWM_INVALID_ARG : If pwmOutput value is invalid
  *               \n \ref RSI_OK             : If process is done successfully.
 */
STATIC INLINE rsi_error_t RSI_MCPWM_OutputOverrideEnable(RSI_MCPWM_T *pMCPWM, uint8_t pwmOutput)
{
  if (pwmOutput <= PWM_OUTPUT_MAX) {
    pMCPWM->PWM_OP_OVERRIDE_ENABLE_SET_REG = (1 << pwmOutput);
  } else {
    return ERROR_PWM_INVALID_ARG;
  }
  return RSI_OK;
}

/*===================================================*/
/**
 * @fn            rsi_error_t RSI_MCPWM_OutputOverrideDisable(RSI_MCPWM_T *pMCPWM,uint8_t pwmOutput)
 * @brief		  This API is used to disable the output override operation of MCPWM
 * @param[in]	  pMCPWM    :  Pointer to the MCPWM instance register area
 * @param[in]	  pwmOutput :  Pwm output over ride enable,possible values are as below
                             - 0 : L0
                             - 1 : L1
                             - 2 : L2
                             - 3 : L3
                             - 4 : H0
                             - 5 : H1
                             - 6 : H2
                             - 7 : H3
  * @return     \ref ERROR_PWM_INVALID_ARG : If pwmOutput value is invalid
  *               \n \ref RSI_OK             : If process is done successfully.
*/
STATIC INLINE rsi_error_t RSI_MCPWM_OutputOverrideDisable(RSI_MCPWM_T *pMCPWM, uint8_t pwmOutput)
{
  if (pwmOutput <= PWM_OUTPUT_MAX) {
    pMCPWM->PWM_OP_OVERRIDE_ENABLE_RESET_REG = (1 << pwmOutput);
  } else {
    return ERROR_PWM_INVALID_ARG;
  }
  return RSI_OK;
}

/*===================================================*/
/**
 * @fn            void RSI_MCPWM_OverrideControlSet(RSI_MCPWM_T *pMCPWM,uint32_t value)
 * @brief     This API is used to set the override control parameter,output is synced with pwm time period depending on operating mode
 * @param[in]   pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]   value   :  if value is 1 then Output override is synced with pwm time period depending
                             \n on operating mode, if 0 then no effect. 
 * @return      none
 */
STATIC INLINE void RSI_MCPWM_OverrideControlSet(RSI_MCPWM_T *pMCPWM, uint32_t value)
{
  pMCPWM->PWM_OP_OVERRIDE_CTRL_SET_REG = value;
}

/*===================================================*/
/**
 * @fn            void RSI_MCPWM_OverrideControlReSet(RSI_MCPWM_T *pMCPWM,uint32_t value)
 * @brief     This API is used to reset the output override sync control parameter.
 * @param[in]   pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]   value   :  if value is 1 then Output override is synced with pwm time period depending
                             \n on operating mode, if 0 then no effect. 
 * @return      none
 */
STATIC INLINE void RSI_MCPWM_OverrideControlReSet(RSI_MCPWM_T *pMCPWM, uint32_t value)
{
  pMCPWM->PWM_OP_OVERRIDE_CTRL_RESET_REG = value;
}

/*===================================================*/
/**
 * @fn            void RSI_MCPWM_OverrideValueSet(RSI_MCPWM_T *pMCPWM,uint8_t pwmOutput,uint8_t value)
 * @brief     This API is used to set override value for the required output of MCPWM.
 * @param[in]   pMCPWM    :  Pointer to the MCPWM instance register area
 * @param[in]   pwmOutput :  PWM outputs are as below
                             -  0 : L0
                             -  1 : L1
                             -  2 : L2
                             -  3 : L3
                             -  4 : H0
                             -  5 : H1
                             -  6 : H2
                             -  7 : H3
 * @param[in]   value     :  override value can be 0 or 1
 * @return      \ref ERROR_PWM_INVALID_ARG : If pwmOutput value is invalid
 *              \n \ref RSI_OK             : If process is done successfully.
 */
STATIC INLINE rsi_error_t RSI_MCPWM_OverrideValueSet(RSI_MCPWM_T *pMCPWM, uint8_t pwmOutput, uint8_t value)
{
  if (pwmOutput <= PWM_OUTPUT_MAX) {
    pMCPWM->PWM_OP_OVERRIDE_VALUE_SET_REG = (value << pwmOutput);
  } else {
    return ERROR_PWM_INVALID_ARG;
  }
  return RSI_OK;
}

/*===================================================*/
/**
 * @fn            rsi_error_t RSI_MCPWM_OverrideValueReSet(RSI_MCPWM_T *pMCPWM,uint8_t pwmOutput,uint8_t value)
 * @brief		  This API is used to reset override value for the required output of MCPWM. 
 * @param[in]	  pMCPWM    :  Pointer to the MCPWM instance register area
 * @param[in]	  pwmOutput :  PWM outputs are as below
                             -  0 : L0
                             -  1 : L1
                             -  2 : L2
                             -  3 : L3
                             -  4 : H0
                             -  5 : H1
                             -  6 : H2
                             -  7 : H3
 * @param[in]   value     :  override value can be 0 or 1
 * @return      \ref ERROR_PWM_INVALID_ARG : If pwmOutput value is invalid
                \n \ref RSI_OK             : If process is done successfully.
 */
STATIC INLINE rsi_error_t RSI_MCPWM_OverrideValueReSet(RSI_MCPWM_T *pMCPWM, uint8_t pwmOutput, uint8_t value)
{
  if (pwmOutput <= PWM_OUTPUT_MAX) {
    pMCPWM->PWM_OP_OVERRIDE_VALUE_RESET_REG = (value << pwmOutput);
  } else {
    return ERROR_PWM_INVALID_ARG;
  }
  return RSI_OK;
}

/*===================================================*/
/**
 * @fn            void RSI_MCPWM_FaultControlSet(RSI_MCPWM_T *pMCPWM,uint32_t value)
 * @brief     This API is used to set output fault override control parameters for required PWM output
 * @param[in]   pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]   value   :  This can be logical OR of below parameters
                           -  FLT_A_MODE      : if bit one then cycle by cycle by mode and zero then latched mode                                            
                           -  FLT_B_MODE      : if bit one then cycle by cycle by mode and zero then latched mode
                           -  OP_POLARITY_H Ouput polarity for high (H3, H2, H1, H0) side signals. 
                           \n if bit 0 then in active low mode and 1 then active high mode. 
                           -  OP_POLARITY_L Ouput polarity for low (L3, L2, L1, L0) side signals. 
                           \n if bit 0 then in active low mode and 1 then active high mode.
                           -  FLT_A_ENABLE    :  enable fault A
                           -  FLT_B_ENABLE    :  enable fault B
                           -  COMPLEMENT_MODE :  PWM I/O pair mode
                           \n if bit is 1 then PWM I/O pin pair is in the complementary output mode
                           \n if bit is 0 then PWM I/O pin pair is in the independent output mode
 * @return      none
 */
STATIC INLINE void RSI_MCPWM_FaultControlSet(RSI_MCPWM_T *pMCPWM, uint32_t value)
{
  pMCPWM->PWM_FLT_OVERRIDE_CTRL_SET_REG = value;
}

/*===================================================*/
/**
 * @fn            void RSI_MCPWM_FaultControlReSet(RSI_MCPWM_T *pMCPWM, uint32_t value)
 * @brief     This API is used to reset output fault override control parameters for required PWM output.
 * @param[in]   pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]   value   :  This can be logical OR of below parameters
                           -  FLT_A_MODE      : if bit one then cycle by cycle by mode and zero then latched mode                                            
                           -  FLT_B_MODE      : if bit one then cycle by cycle by mode and zero then latched mode
                           -  OP_POLARITY_H Ouput polarity for high (H3, H2, H1, H0) side signals. 
                           \n if bit 0 then in active low mode and 1 then active high mode. 
                           -  OP_POLARITY_L Ouput polarity for low (L3, L2, L1, L0) side signals. 
                           \n if bit 0 then in active low mode and 1 then active high mode.
                           -  FLT_A_ENABLE    :  enable fault A
                           -  FLT_B_ENABLE    :  enable fault B
                           -  COMPLEMENT_MODE :  PWM I/O pair mode
                           \n if bit is 1 then PWM I/O pin pair is in the complementary output mode
                           \n if bit is 0 then PWM I/O pin pair is in the independent output mode
 * @return      none
 */
STATIC INLINE void RSI_MCPWM_FaultControlReSet(RSI_MCPWM_T *pMCPWM, uint32_t value)
{
  pMCPWM->PWM_FLT_OVERRIDE_CTRL_RESET_REG = value;
}

/*===================================================*/
/**
 * @fn            void RSI_MCPWM_SpecialEventTriggerEnable(RSI_MCPWM_T *pMCPWM )
 * @brief     This API is used to enable generation of special event trigger for required channel of MCPWM
 * @param[in]   pMCPWM  :  Pointer to the MCPWM instance register area
 * @return      none
 */
STATIC INLINE void RSI_MCPWM_SpecialEventTriggerEnable(RSI_MCPWM_T *pMCPWM)
{
  pMCPWM->PWM_SVT_CTRL_SET_REG = (1 << 0);
}

/*===================================================*/
/**
 * @fn            void RSI_MCPWM_SpecialEventTriggerDisable(RSI_MCPWM_T *pMCPWM )
 * @brief     This API is used to disable generation of special event trigger for required channel of MCPWM
 * @param[in]   pMCPWM  :  Pointer to the MCPWM instance register area
 * @return      none
 */
STATIC INLINE void RSI_MCPWM_SpecialEventTriggerDisable(RSI_MCPWM_T *pMCPWM)
{
  pMCPWM->PWM_SVT_CTRL_RESET_REG = (1 << 0);
}

/*===================================================*/
/**
 * @fn            void RSI_MCPWM_DeadTimeControlSet(RSI_MCPWM_T *pMCPWM, uint32_t value )
 * @brief     This API is used to set dead time control parameters for the reqired channel.
 * @param[in]   pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]   value   :  This can be logical OR of below parameters
               - DEADTIME_SELECT_ACTIVE  : Deadtime select bits for PWM going active
                             Possible values are as below if bit zero then use counter A , if one then use counter B
               - DEADTIME_SELECT_INACTIVE : Deadtime select bits for PWM going inactive
                             Possible values are as below if bit zero then use counter A , if one then use counter B
 * @return      none
 */
STATIC INLINE void RSI_MCPWM_DeadTimeControlSet(RSI_MCPWM_T *pMCPWM, uint32_t value)
{
  pMCPWM->PWM_DEADTIME_CTRL_SET_REG = value;
}

/*===================================================*/
/**
 * @fn            void RSI_MCPWM_DeadTimeControlReSet(RSI_MCPWM_T *pMCPWM, uint32_t value )
 * @brief     This API is used to reset dead time control for the MCPWM.
 * @param[in]   pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]   value   :  This can be logical OR of below parameters
                            - DEADTIME_SELECT_ACTIVE   : Deadtime select bits for PWM going active
                               Possible values are as below if bit zero then use counter A , if one then use counter B
                            - DEADTIME_SELECT_INACTIVE : Deadtime select bits for PWM going inactive
                             Possible values are as below if bit zero then use counter A , if one then use counter B
 * @return      none
 */
STATIC INLINE void RSI_MCPWM_DeadTimeControlReSet(RSI_MCPWM_T *pMCPWM, uint32_t value)
{
  pMCPWM->PWM_DEADTIME_CTRL_RESET_REG = value;
}

/*===================================================*/
/**
 * @fn          rsi_error_t RSI_MCPWM_SetDutyCycle(RSI_MCPWM_T *pMCPWM, uint16_t dutyCycle, uint8_t chnlNum)
 * @brief		This API is used to set duty cycle for the required MCPWM channel. 
 * @param[in]	pMCPWM    :  Pointer to the MCPWM instance register area
 * @param[in]	dutyCycle :  Duty cycle value 
 * @param[in]   chnlNum   :  channel number(0 to 3)
 * @return      \ref ERROR_PWM_INVALID_ARG : If channel number is invalid
                \n \ref RSI_OK             : If process is done successfully.
 */
STATIC INLINE rsi_error_t RSI_MCPWM_SetDutyCycle(RSI_MCPWM_T *pMCPWM, uint16_t dutyCycle, uint8_t chnlNum)
{
  if (chnlNum <= PWM_CHNL_3) {
    pMCPWM->PWM_DUTYCYCLE_REG_WR_VALUE_b[chnlNum].PWM_DUTYCYCLE_REG_WR_VALUE_CH = dutyCycle;
  } else {
    return ERROR_PWM_INVALID_ARG;
  }
  return RSI_OK;
}

/*===================================================*/
/**
 * @fn            void RSI_MCPWM_ExternalTriggerControl(RSI_MCPWM_T *pMCPWM,boolean_t enable)
 * @brief     This API is used to enable to use external trigger for base time counter increment or decrement of MCPWM
 * @param[in]   pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in]   enable  :  If 0 then disable external trigger
                           \n If 1 then enable external trigger.
 * @return      none
 */
STATIC INLINE void RSI_MCPWM_ExternalTriggerControl(RSI_MCPWM_T *pMCPWM, boolean_t enable)
{
  pMCPWM->PWM_TIME_PRD_COMMON_REG_b.USE_EXT_TIMER_TRIG_FRM_REG = (unsigned int)(enable & 0x01);
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_PWM_Channel_Reset_Disable(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum)
 * @brief   This API is used to disable the reset for required channel of MCPWM.
 * @param[in] pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in] chnlNum :  Channel number(0 to 3)
 * @return    \ref ERROR_PWM_INVALID_CHNLNUM : If channel is invalid
 *               \n \ref RSI_OK            : If process is done successfully.
 */
STATIC INLINE rsi_error_t RSI_PWM_Channel_Reset_Disable(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum)
{
  // Resets operation of MCPWM channel
  switch (chnlNum) {
    case PWM_CHNL_0:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH0_b.PWM_SFT_RST = DISABLE;
      break;
    case PWM_CHNL_1:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH1_b.PWM_SFT_RST = DISABLE;
      break;
    case PWM_CHNL_2:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH2_b.PWM_SFT_RST = DISABLE;
      break;
    case PWM_CHNL_3:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH3_b.PWM_SFT_RST = DISABLE;
      break;
    default:
      return ERROR_PWM_INVALID_CHNLNUM;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_PWM_Counter_Reset_Disable(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum)
 * @brief   This API is used to disable the counter reset for required channel of MCPWM
 * @param[in] pMCPWM  :  Pointer to the MCPWM instance register area
 * @param[in] chnlNum :  Channel number(0 to 3)
 * @return      \ref ERROR_PWM_INVALID_CHNLNUM : If channel number is invalid
 *               \n \ref RSI_OK            : If process is done successfully
 */
STATIC INLINE rsi_error_t RSI_PWM_Counter_Reset_Disable(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum)
{
  // resets counter operations
  switch (chnlNum) {
    case PWM_CHNL_0:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH0_b.PWM_TIME_PRD_CNTR_RST_FRM_REG = DISABLE;
      break;
    case PWM_CHNL_1:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH1_b.PWM_TIME_PRD_CNTR_RST_FRM_REG = DISABLE;
      break;
    case PWM_CHNL_2:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH2_b.PWM_TIME_PRD_CNTR_RST_FRM_REG = DISABLE;
      break;
    case PWM_CHNL_3:
      pMCPWM->PWM_TIME_PRD_CTRL_REG_CH3_b.PWM_TIME_PRD_CNTR_RST_FRM_REG = DISABLE;
      break;
    default:
      return ERROR_PWM_INVALID_CHNLNUM;
  }
  return RSI_OK;
}

/*===================================================*/
/**
 * @fn            void RSI_MCPWM_BaseTimerSelect(RSI_MCPWM_T *pMCPWM,uint8_t baseTime)
 * @brief     This API is used to select number of base timers as four base timers for four channels or one base timer for all channels of MCPWM
 * @param[in]   pMCPWM   :  Pointer to the MCPWM instance register area
 * @param[in]   baseTime :  if 0 then one base timer for each channel
                            \n if 1 then only one base timer for all channels                                    
 * @return      none
 */
STATIC INLINE void RSI_MCPWM_BaseTimerSelect(RSI_MCPWM_T *pMCPWM, uint8_t baseTime)
{
  pMCPWM->PWM_TIME_PRD_COMMON_REG_b.PWM_TIME_PRD_USE_0TH_TIMER_ONLY = (unsigned int)(baseTime & 0x01);
}
//  PWM FUNCTION PROTOTYPES
RSI_DRIVER_VERSION_M4 RSI_MCPWM_GetVersion(void);

RSI_MCPWM_CAPABILITIES_T RSI_MCPWM_GetCapabilities(void);

rsi_error_t mcpwm_counter_reset(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum);

rsi_error_t mcpwm_channel_reset(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum);

rsi_error_t mcpwm_start(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum);

rsi_error_t mcpwm_stop(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum);

rsi_error_t mcpwm_set_time_period(RSI_MCPWM_T *pMCPWM, uint8_t chnlNum, uint16_t period, uint16_t initVal);

void mcpwm_special_event_trigger_config(RSI_MCPWM_T *pMCPWM, boolean_t svtDir, RSI_MCPWM_SVT_CONFIG_T *pMCPWMSVTConfig);

rsi_error_t mcpwm_dead_time_value_set(RSI_MCPWM_T *pMCPWM,
                                      RSI_MCPWM_DT_CONFIG_T *pMCPWMDeadTimeConfig,
                                      uint8_t chnlNum);
rsi_error_t mcpwm_fault_avalue_set(RSI_MCPWM_T *pMCPWM, uint8_t pwmOutput, uint8_t value);

rsi_error_t mcpwm_fault_bvalue_set(RSI_MCPWM_T *pMCPWM, uint8_t pwmOutput, uint8_t value);

rsi_error_t mcpwm_set_base_timer_mode(RSI_MCPWM_T *pMCPWM, uint8_t mode, uint8_t chnlNum);

rsi_error_t mcpwm_set_output_mode(RSI_MCPWM_T *pMCPWM, boolean_t mode, uint8_t chnlNum);

void mcpwm_set_output_polarity(RSI_MCPWM_T *pMCPWM, boolean_t polL, boolean_t polH);

void mcpwm_interrupt_handler(RSI_MCPWM_T *pMCPWM, RSI_MCPWM_CALLBACK_T *pCallBack);

rsi_error_t mcpwm_period_control_config(RSI_MCPWM_T *pMCPWM, uint32_t postScale, uint32_t preScale, uint8_t chnlNum);
void SysTick_Handler(void);
uint32_t MCPWM_PercentageToTicks(uint8_t percent, uint8_t chnl_num);
void MCPWM_SetChannelPeriod(uint32_t freq);
void RSI_MCPWM_PinMux();

#ifdef __cplusplus
}
#endif
#endif // RSI_PWM_H
