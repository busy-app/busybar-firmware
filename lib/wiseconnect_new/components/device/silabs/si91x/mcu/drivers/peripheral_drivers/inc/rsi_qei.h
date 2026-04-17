/******************************************************************************
* @file  rsi_qei.h
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

// Include Files

#include "base_types.h"
#include "rsi_ccp_common.h"
#include "rsi_pll.h"
#include "rsi_rom_clks.h"
#include "rsi_rom_egpio.h"

#ifndef RSI_QEI_H
#define RSI_QEI_H

#ifdef __cplusplus
extern "C" {
#endif

// QEI status flags
#define QEI_IDX          BIT(0) // This is a direct value from the position signal generator
#define QEI_POS_B        BIT(1) // This is a direct value from the position signal generator.Value
#define QEI_POS_A        BIT(2) // This is a direct value from the position signal generator.Value
#define QEI_POS_CTRL_ERR BIT(3) // Count Error Status Flag bit
#define QEI_POS_CTRL_DIR BIT(4) // Position Counter Direction Status bit

// QEI interrupt status flags

// Note : Same flags are used to clear the interrupts

// Interrupt status bits
#define QEI_POS_CNT_RST_INTR_LVL BIT(0) // This is raised when the position counter reaches it?s extremes.
#define QEI_IDX_CNT_MAT_INT_LVL \
  BIT(1) //	This is raised when index counter reaches max value loaded in to index_max_cnt register.
#define QEI_POS_CNT_ERR_INTR_LVL \
  BIT(2) /* Whenever number of possible positions are mismatched 
                                                     with actual positions are received between two index pulses this will raised.*/
#define QEI_VELO_LESS_THAN_INTR_LVL \
  BIT(3) // When velocity count is less than the value given in velocity_value_to_compare register, interrupt is raised.
#define QEI_POS_CNT_MAT_INTR_LVL \
  BIT(4) //  This is raised when the position counter reaches position match value, which is programmable
#define VELOCITY_COMPUTATION_OVER_INTR_LVL BIT(5) // QEI control register set

// Configuration settings
#define QEI_SFT_RST BIT(0) // Quadrature encoder soft reset. It is self reset signal
#define QEI_SWAP_PHASE_AB_B \
  BIT(1) // Phase A and Phase B Input Swap Select bit 1 = Phase A and Phase B inputs are swapped
#define QEI_POS_CNT_RST_WITH_IDX_EN \
  BIT(2) // 1 = position counter is getting reset for every index pulse 0 = position counter is getting
#define QEI_POS_CNT_DIR_CTRL    BIT(4) // 0 = position B pin defines the direction of position counter
#define QEI_POS_CNT_DIR_FRM_REG BIT(5) // Position Counter Direction indication from user
#define QEI_IDX_CNT_RST_EN      BIT(8) // 1= index counter is going to reset after reaching max count, which is mentioned in
#define QEI_DIGITAL_FILTER_BYPASS \
  BIT(9) // 1= digital filter is bypassed for all input signals (position A, position B and Index)
#define QEI_TIMER_MODE \
  BIT(10) /* 1 = timer mode. In this mode, decoded timer pulse and direction are taken from position A and 
                                               position B pins respectively */
#define QEI_START_VELOCITY_CNTR BIT(11) // Starting the velocity counter. It is self reset bit
#define QEI_STOP_IN_IDLE        BIT(12) // QEI stop in idle
#define QEI_POS_CNT_16_BIT_MDE  BIT(13) // QEI position counter 16 bit mode enable
#define QEI_POS_CNT_RST         BIT(14) // 1= position counter is going to reset
#define QEI_IDX_CNT_RST         BIT(15) // 1= index counter is going to reset

// QEI control bits
#define QEI_INDEX_CNT_INIT          BIT(12)
#define QEI_UNIDIRECTIONAL_INDEX    BIT(11)
#define QEI_UNIDIRECTIONAL_VELOCITY BIT(10)

// Interrupt un mask
#define QEI_POS_CNT_RESET_INTR_UNMASK      BIT(0)
#define QEI_IDX_CNT_MATCH_INTR_UNMASK      BIT(1)
#define QEI_POS_CNTR_ERR_INTR_UNMASK       BIT(2)
#define QEI_VEL_LESS_THAN_INTR_UNMASK      BIT(3)
#define QEI_IDX_POS_MATCH_INTR_UNMASK      BIT(4)
#define QEI_VEL_COMPUTATION_OVER_INTR_MASK BIT(5)

// Clock divider
// Digital Filter Clock Divide Select bits
#define QEI_CLK_DIV_1    0x00
#define QEI_CLK_DIV_2    0x01
#define QEI_CLK_DIV_4    0x02
#define QEI_CLK_DIV_8    0x03
#define QEI_CLK_DIV_16   0x04
#define QEI_CLK_DIV_32   0x05
#define QEI_CLK_DIV_64   0x06
#define QEI_CLK_DIV_128  0x07
#define QEI_CLK_DIV_256  0x08
#define QEI_CLK_DIV_512  0x09
#define QEI_CLK_DIV_1024 0x0A

// QEI Encoding mode
#define QEI_ENCODING_MODE_1X 0x00
#define QEI_ENCODING_MODE_2X 0x01
#define QEI_ENCODING_MODE_4X 0x02

#define INDEX_MATCH_VALUE_MASK 0x03 // Mask for the index match value (2 bits)

/** @addtogroup SOC25
* @{
*/

// Function prototypes
void RSI_QEI_Enable(QEI_Type *pstcQei);
void RSI_QEI_Disable(QEI_Type *pstcQei);
/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_SetConfiguration(volatile QEI_Type  *pstcQei, uint32_t configParms)   
 * @brief	    Sets the QEI module configurations
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @param[in]   configParms : OR'ed values configurations needs to be passed to this variable
 * @return 		none
 */
STATIC INLINE void RSI_QEI_SetConfiguration(volatile QEI_Type *pstcQei, uint32_t configParms)
{
  // QEI configuration set
  pstcQei->QEI_CTRL_REG_SET = configParms;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_ClrConfiguration(volatile QEI_Type  *pstcQei, uint32_t configParms)
 * @brief	    clears the QEI module configurations
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @param[in]   configParms : OR'ed values configurations needs to be passed to this variable
 * @return 		none
 */
STATIC INLINE void RSI_QEI_ClrConfiguration(volatile QEI_Type *pstcQei, uint32_t configParms)
{
  // QEI configuration set
  pstcQei->QEI_CTRL_REG_RESET = configParms;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_SetControls(volatile QEI_Type  *pstcQei, uint32_t cntrlParms)
 * @brief	    Sets the QEI control configuration
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @param[in]   cntrlParms : OR'ed values control field bits.
 * @return 		none
 */
STATIC INLINE void RSI_QEI_SetControls(volatile QEI_Type *pstcQei, uint32_t cntrlParms)
{
  // QEI configuration set
  pstcQei->QEI_CNTLR_INIT_REG |= cntrlParms;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_ClrControls(volatile QEI_Type  *pstcQei, uint32_t cntrlParms)
 * @brief		Clears the QEI control configuration
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @param[in]   cntrlParms : OR'ed values control field bits.
 *              \n 
 *              \ref QEI_CNTLR_INIT_REG
 * @return 	    none
 */
STATIC INLINE void RSI_QEI_ClrControls(volatile QEI_Type *pstcQei, uint32_t cntrlParms)
{
  // QEI configuration set
  pstcQei->QEI_CNTLR_INIT_REG &= ~cntrlParms;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_SetMaxPosCnt(volatile QEI_Type  *pstcQei,uint32_t maxPosition)
 * @brief       Sets the MAX value for position counter. This is a maximum count value that is
 *              allowed to increment in the position counter.If position counter reaches this value in
 *              positive direction, will get set to zero and if reaches zero in negative direction, will get
 *              set to this max count.\ref Pos_cnt_rst_with_index_en,
 *              \ref qei_ctrl_reg[2] should be reset (0) for setting the position
 *              counter when reaches max/zero value.
 *              \ref QEI_POSITION_MAX_CNT_LSW_REG
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @param[in]   maxPosition :Max position value
 * @return 		none
 */
STATIC INLINE void RSI_QEI_SetMaxPosCnt(volatile QEI_Type *pstcQei, uint32_t maxPosition)
{
  // configure least significant word (LSW)
  pstcQei->QEI_POSITION_MAX_CNT_LSW_REG = (0xFFFF & maxPosition);
  pstcQei->QEI_POSITION_MAX_CNT_LSW_REG |= (0xFFFF & (maxPosition >> 16)) << 16;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE uint32_t RSI_QEI_GetMaxPosCnt(volatile QEI_Type  *pstcQei)
 * @brief 	    Gets the maximum position count
 * @param[in]   pstcQei : Pointer to the QEI register instance
 *             \n 
 *             \ref QEI_POSITION_MAX_CNT_LSW_REG
 * @return 	returns the maximum position count
 */
STATIC INLINE uint32_t RSI_QEI_GetMaxPosCnt(volatile QEI_Type *pstcQei)
{
  return pstcQei->QEI_POSITION_MAX_CNT_LSW_REG;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_SetPosMatch(volatile QEI_Type  *pstcQei, uint32_t matchPosition)    
 * @brief	    Sets the position match value to compare the position counter. When it is matched with position
 *              counter, interrupt is raised.
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @param[in]   matchPosition : position match value.
 *              \ref QEI_POSITION_MATCH_REG
 * @return 		none
 */
STATIC INLINE void RSI_QEI_SetPosMatch(volatile QEI_Type *pstcQei, uint32_t matchPosition)
{
  // configure least significant word (LSW)
  pstcQei->QEI_POSITION_MATCH_REG = matchPosition;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE uint32_t RSI_QEI_GetPosMatch(volatile QEI_Type  *pstcQei)
 * @brief	    Gets the position match value.
 * @param[in]   pstcQei : Pointer to the QEI register instance
 *              \ref STATIC INLINE uint32_t RSI_QEI_GetPosMatch(volatile QEI_Type  *pstcQei)
 * @return 	    returns the position match value
 */
STATIC INLINE uint32_t RSI_QEI_GetPosMatch(volatile QEI_Type *pstcQei)
{
  // configure least significant word (LSW)
  return pstcQei->QEI_POSITION_MATCH_REG;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE uint32_t RSI_QEI_GetPosition(volatile QEI_Type  *pstcQei)
 * @brief       Gets the current position increment counter value to know the position of the motor.
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @return 	    position current position counter value
 *              \n
 *              \ref QEI_POSITION_CNT_REG 
 */
STATIC INLINE uint32_t RSI_QEI_GetPosition(volatile QEI_Type *pstcQei)
{
  // Return the position count register
  return pstcQei->QEI_POSITION_CNT_REG;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE uint32_t RSI_QEI_GetIndex(volatile QEI_Type  *pstcQei)   
 * @brief	    Gets the current index increment counter value to know the index count of the motor.
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @return 	    index current counter value
 *              \n
 *              \ref QEI_INDEX_CNT_REG
 */
STATIC INLINE uint32_t RSI_QEI_GetIndex(volatile QEI_Type *pstcQei)
{
  // Return the position count register
  return pstcQei->QEI_INDEX_CNT_REG;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_SetMaxIndex(volatile QEI_Type  *pstcQei , uint16_t maxIdxCntVal)
 * @brief	    configures the QEI index maximum count.This is a maximum count value that is allowed to increment in the index counter
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @param[in]   maxIdxCntVal : maximum allowed index value to be programmed.
 *              \n
 *              \ref QEI_INDEX_MAX_CNT_REG
 * @return 	    none
 */
STATIC INLINE void RSI_QEI_SetMaxIndex(volatile QEI_Type *pstcQei, uint16_t maxIdxCntVal)
{
  pstcQei->QEI_INDEX_MAX_CNT_REG = maxIdxCntVal;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE uint16_t RSI_QEI_GetMaxIndex(volatile QEI_Type  *pstcQei)
 * @brief       Gets the maximum index value.
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @return 	    returns the MAX index value programmed by user.
 *              \n
 *              \ref  QEI_INDEX_MAX_CNT_REG 
 */
STATIC INLINE uint16_t RSI_QEI_GetMaxIndex(volatile QEI_Type *pstcQei)
{
  return (uint16_t)(pstcQei->QEI_INDEX_MAX_CNT_REG);
}

/*===================================================*/
/**
 * @fn          STATIC INLINE boolean_t RSI_QEI_GetDirection(volatile QEI_Type  *pstcQei) 
 * @brief	    Gets the direction of the QEI interfaced motor
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @return 	    returns 1: (+) Positive direction \ref QEI_STATUS_REG_b 
 *                      0: (-) Negative direction \ref QEI_STATUS_REG_b
 */
STATIC INLINE boolean_t RSI_QEI_GetDirection(volatile QEI_Type *pstcQei)
{
  // return direction
  return pstcQei->QEI_STATUS_REG_b.POSITION_CNTR_DIRECTION;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE boolean_t RSI_QEI_GetDirection(volatile QEI_Type  *pstcQei)
 * @brief     Sets the direction of the QEI interfaced motor
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @return      returns 1: (+) Positive direction \ref QEI_STATUS_REG_b
 *                      0: (-) Negative direction \ref QEI_STATUS_REG_b
 */
STATIC INLINE void RSI_QEI_SetDirection(volatile QEI_Type *pstcQei, boolean_t direction)
{
  pstcQei->QEI_CTRL_REG_SET_b.POS_CNT_DIR_FRM_REG = direction;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE int32_t RSI_QEI_GetStatus(volatile QEI_Type  *pstcQei)
 * @brief	    Gets the status of the QEI module
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @return 	    returns the status of the QEI. \ref QEI_STATUS_REG
 */
STATIC INLINE int32_t RSI_QEI_GetStatus(volatile QEI_Type *pstcQei)
{
  return (int32_t)(pstcQei->QEI_STATUS_REG);
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_StartVelocityCounter(volatile QEI_Type  *pstcQei)
 * @brief       Starts the velocity counter.
 * @param[in]   pstcQei : Pointer to the QEI register instance.\ref QEI_CTRL_REG_SET 
 * @return 	    none
 */
STATIC INLINE void RSI_QEI_StartVelocityCounter(volatile QEI_Type *pstcQei)
{
  // Enable velocity counter
  pstcQei->QEI_CTRL_REG_SET = BIT(11);
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_ConfigureTimerMode(volatile QEI_Type *pstcQei, boolean_t timer_mode)
 * @brief       Configures the timer mode for the QEI module.
 * @param[in]   pstcQei : Pointer to the QEI register instance (e.g., QEI_CTRL_REG_SET).
 * @param[in]   timer_mode : Boolean value to enable (1) or disable (0) timer mode.
 * @return      none
 */
STATIC INLINE void RSI_QEI_ConfigureTimerMode(volatile QEI_Type *pstcQei, boolean_t timer_mode)
{
  if (timer_mode == TRUE) {
    // Enable timer mode by setting the appropriate bit in the control register
    pstcQei->QEI_CTRL_REG_SET |= BIT(10);
  } else {
    // Disable timer mode by clearing the appropriate bit in the control register
    pstcQei->QEI_CTRL_REG_SET &= ~BIT(10);
  }
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_Swap_PhaseAB(volatile QEI_Type *pstcQei, boolean_t swap_select)
 * @brief       Swaps the phase A and phase B signals for the QEI module.
 * @param[in]   pstcQei : Pointer to the QEI register instance (e.g., QEI_CTRL_REG_SET).
 * @param[in]   swap_select : Boolean value to enable (1) or disable (0) phase swapping.
 * @return      none
 */
STATIC INLINE void RSI_QEI_Swap_PhaseAB(volatile QEI_Type *pstcQei, boolean_t swap_select)
{
  if (swap_select == 1) {
    // Enable phase swapping by setting the appropriate bit in the control register
    pstcQei->QEI_CTRL_REG_SET |= QEI_SWAP_PHASE_AB_B;
  } else {
    // Disable phase swapping by clearing the appropriate bit in the control register
    pstcQei->QEI_CTRL_REG_SET &= ~QEI_SWAP_PHASE_AB_B;
  }
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_StopVelocityCounter(volatile QEI_Type  *pstcQei)
 * @brief       stops the velocity counter.
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @return 	    none
 */
STATIC INLINE void RSI_QEI_StopVelocityCounter(volatile QEI_Type *pstcQei)
{
  // Disable velocity counter
  pstcQei->QEI_CTRL_REG_RESET = BIT(11);
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_SetDigitalFilterClkDiv(volatile QEI_Type  *pstcQei , uint32_t div)
 * @brief       Configures the digital filter clock division selects
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @param[in]   div     : Digital filter clock division selects
 * @return 	    none
 */
STATIC INLINE void RSI_QEI_SetDigitalFilterClkDiv(volatile QEI_Type *pstcQei, uint32_t div)
{
  pstcQei->QEI_CNTLR_INIT_REG_b.DF_CLK_DIVIDE_SLT = (unsigned int)(div & 0x0F);
}

/*===================================================*/
/**
 * @fn          STATIC INLINE uint32_t RSI_QEI_GetDigitalFilterClkDiv(volatile QEI_Type  *pstcQei) 
 * @brief	    Gets the digital filter clock division selects
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @return 		returns the digital filter clock division bits
 */
STATIC INLINE uint32_t RSI_QEI_GetDigitalFilterClkDiv(volatile QEI_Type *pstcQei)
{
  return pstcQei->QEI_CNTLR_INIT_REG_b.DF_CLK_DIVIDE_SLT;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void  RSI_QEI_ConfigureDeltaTimeAndFreq(volatile QEI_Type  *pstcQei, uint32_t freq, uint32_t PeriodInUs)
 * @brief       Configures the delta time and QEI module input frequency for velocity measurements
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @param[in]   freq    : QEI module input frequency(typically soc clock) in hertz.
 * @param[in]   PeriodInUs    : Velocity measurement time in micro seconds (ex: for sec program 1000000)
 * @return 	    none
 */
STATIC INLINE void RSI_QEI_ConfigureDeltaTimeAndFreq(volatile QEI_Type *pstcQei, uint32_t freq, uint32_t PeriodInUs)
{
  // Operating frequency
  pstcQei->QEI_CLK_FREQ_REG = ((freq / 1000000) - 1);
  // configure the clock divider value
  pstcQei->QEI_DELTA_TIME_REG = PeriodInUs;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void  RSI_QEI_SetModuleFreq(volatile QEI_Type  *pstcQei, uint32_t freq)
 * @brief       Sets the QEI module frequency for velocity measurements computations.
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @param[in]   freq    : QEI module input frequency(typically soc clock) in hertz.
 *              \ref  QEI_CLK_FREQ_REG
 * @return 	    none
 */
STATIC INLINE void RSI_QEI_SetModuleFreq(volatile QEI_Type *pstcQei, uint32_t freq)
{
  // Operating frequency
  pstcQei->QEI_CLK_FREQ_REG = ((freq / 1000000) - 1);
}

/**
 * @fn          STATIC INLINE uint32_t  RSI_QEI_GetModuleFreq(volatile QEI_Type  *pstcQei)
 * @brief	Gets the QEI module frequency for velocity measurements computations.
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @return 	returns the module frequency in hertz
 *              \ref QEI_CLK_FREQ_REG
 */
STATIC INLINE uint32_t RSI_QEI_GetModuleFreq(volatile QEI_Type *pstcQei)
{
  // Operating frequency
  return (((pstcQei->QEI_CLK_FREQ_REG) + 1) * 1000000);
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void  RSI_QEI_SetDeltaTime(volatile QEI_Type  *pstcQei, uint32_t PeriodInUs)
 * @brief	    Gets the QEI module delta time for velocity computation
 * @param[in]   pstcQei : Pointer to the QEI register instance.\ref QEI_DELTA_TIME_REG 
 * @param[in]   PeriodInUs    : Velocity measurement time in micro seconds (ex: for sec program 1000000)
 * @return      none
 */
STATIC INLINE void RSI_QEI_SetDeltaTime(volatile QEI_Type *pstcQei, uint32_t PeriodInUs)
{
  // Operating frequency
  pstcQei->QEI_DELTA_TIME_REG = PeriodInUs;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE uint32_t  RSI_QEI_GetDeltaTime(volatile QEI_Type  *pstcQei)
 * @brief       Gets the QEI module delta time configured for velocity computation
 * @param[in]   pstcQei : Pointer to the QEI register instance.\ref  QEI_DELTA_TIME_REG
 * @return 	    returns the configured delta time.
 */
STATIC INLINE uint32_t RSI_QEI_GetDeltaTime(volatile QEI_Type *pstcQei)
{
  // Operating frequency
  return pstcQei->QEI_DELTA_TIME_REG;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE uint32_t RSI_QEI_GetVelocity(volatile QEI_Type  *pstcQei)
 * @brief       Get the velocity of QEI module for programmed delta time
 * @param[in]   pstcQei : Pointer to the QEI register instance.\ref QEI_VELOCITY_REG
 * @return 	    number of position counts for configured delta time from start of velocity trigger.
 */
STATIC INLINE uint32_t RSI_QEI_GetVelocity(volatile QEI_Type *pstcQei)
{
  return pstcQei->QEI_VELOCITY_REG;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_IntrUnMask(volatile QEI_Type  *pstcQei, uint32_t intrMask)
 * @brief       Un masks the QEI interrupts
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @param[in]   intrMask : OR'ed values of interrupt mask bits to be un masked
 * @return 	none
 */
STATIC INLINE void RSI_QEI_IntrUnMask(volatile QEI_Type *pstcQei, uint32_t intrMask)
{
  pstcQei->QEI_INTR_UNMASK_REg = intrMask;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_IntrMask(volatile QEI_Type  *pstcQei, uint32_t intrMask)
 * @brief	    Masks the QEI interrupts
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @param[in]   intrMask : OR'ed values of interrupt mask bits to be masked
 * @return 	    none
 */
STATIC INLINE void RSI_QEI_IntrMask(volatile QEI_Type *pstcQei, uint32_t intrMask)
{
  pstcQei->QEI_INTR_MASK_REG = intrMask;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE uint32_t RSI_QEI_GetIntrMask(volatile QEI_Type *pstcQei)
 * @brief       Get the value of the QEI interrupts Mask bits
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @return      returns the interrupt Mask bits
 */
STATIC INLINE uint32_t RSI_QEI_GetIntrMask(volatile QEI_Type *pstcQei)
{
  return (pstcQei->QEI_INTR_MASK_REG);
}

/*===================================================*/
/**
 * @fn          STATIC INLINE uint32_t RSI_QEI_GetIntrStatus(volatile QEI_Type  *pstcQei)
 * @brief       Get the interrupt status of QEI module
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @return 	    returns the interrupt status bits
 */
STATIC INLINE uint32_t RSI_QEI_GetIntrStatus(volatile QEI_Type *pstcQei)
{
  return (pstcQei->QEI_INTR_STS_REG);
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_ClrIntrStatus(volatile QEI_Type  *pstcQei, uint32_t intrMask) 
 * @brief	    Clears the QEI interrupt status
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @param[in]   intrMask : OR'ed values of interrupt mask bits to be cleared
 * @return 	    none
 */
STATIC INLINE void RSI_QEI_ClrIntrStatus(volatile QEI_Type *pstcQei, uint32_t intrMask)
{
  pstcQei->QEI_INTR_ACK_REG = intrMask;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_SetMode(volatile QEI_Type  *pstcQei , uint8_t encMode)
 * @brief       Sets the QEI encoding modes (eg:1X , 2X , 4X)
 * @param[in]   pstcQei : Pointer to the QEI register instance.\ref QEI_CNTLR_INIT_REG_b
 * @param[in]   encMode : Encoding mode
 * @return 	    none
 */
STATIC INLINE void RSI_QEI_SetMode(volatile QEI_Type *pstcQei, uint8_t encMode)
{
  pstcQei->QEI_CNTLR_INIT_REG_b.QEI_ENCODING_MODE = (unsigned int)(encMode & 0x03);
}

/*===================================================*/
/**
 * @fn          STATIC INLINE uint32_t RSI_QEI_GetMode(volatile QEI_Type  *pstcQei)
 * @brief       Gets the QEI encoding modes (eg:1X , 2X , 4X)
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @return 	    Returns the Encoding mode
 */
STATIC INLINE uint32_t RSI_QEI_GetMode(volatile QEI_Type *pstcQei)
{
  return pstcQei->QEI_CNTLR_INIT_REG_b.QEI_ENCODING_MODE;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_VelocityValueToCompare(volatile QEI_Type *pstcQei, uint32_t Velocity)
 * @brief       Sets the velocity value for comparison in the QEI.
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @param[in]   Velocity : The velocity value to set
 */
STATIC INLINE void RSI_QEI_VelocityValueToCompare(volatile QEI_Type *pstcQei, uint32_t velocity)
{
  pstcQei->QEI_VELOCITY_REG = velocity;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_SetPosition(volatile QEI_Type *pstcQei, uint32_t Position)
 * @brief       Sets the current position in the QEI.
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @param[in]   Position : The position value to set
 */
STATIC INLINE void RSI_QEI_SetPosition(volatile QEI_Type *pstcQei, uint32_t position)
{
  pstcQei->QEI_POSITION_CNT_REG = position;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_SetIndex(volatile QEI_Type *pstcQei, uint32_t Index)
 * @brief       Sets the index count value in the QEI.
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @param[in]   Index : The index value to set
 */
STATIC INLINE void RSI_QEI_SetIndex(volatile QEI_Type *pstcQei, uint32_t index)
{
  pstcQei->QEI_INDEX_CNT_REG = index;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE uint32_t RSI_QEI_GetClrIntrStatus(volatile QEI_Type *pstcQei)
 * @brief       Retrieves the interrupt acknowledgment status for the QEI.
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @return      Returns the acknowledged interrupt status
 */
STATIC INLINE uint32_t RSI_QEI_GetClrIntrStatus(volatile QEI_Type *pstcQei)
{
  return pstcQei->QEI_INTR_ACK_REG;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE void RSI_QEI_SetIndexMatchValue(volatile QEI_Type *pstcQei, uint32_t Match_Value)
 * @brief       Sets the index match value in the QEI control register.
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @param[in]   Match_Value : The index match value to set (limited to 2 bits)
 */
STATIC INLINE void RSI_QEI_SetIndexMatchValue(volatile QEI_Type *pstcQei, uint32_t match_Value)
{
  pstcQei->QEI_CNTLR_INIT_REG_b.INDEX_MATCH_VALUE = (unsigned int)(match_Value & INDEX_MATCH_VALUE_MASK);
}

/*===================================================*/
/**
 * @fn          STATIC INLINE uint32_t RSI_QEI_GetIndexMatchValue(volatile QEI_Type *pstcQei)
 * @brief       Retrieves the index match value from the QEI control register.
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @return      Returns the current index match value
 */
STATIC INLINE uint32_t RSI_QEI_GetIndexMatchValue(volatile QEI_Type *pstcQei)
{
  return pstcQei->QEI_CNTLR_INIT_REG_b.INDEX_MATCH_VALUE;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE uint32_t RSI_QEI_GetControls(volatile QEI_Type *pstcQei)
 * @brief       Retrieves the current control settings for the QEI.
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @return      Returns the current control register settings
 */
STATIC INLINE uint32_t RSI_QEI_GetControls(volatile QEI_Type *pstcQei)
{
  return pstcQei->QEI_CNTLR_INIT_REG;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE uint32_t RSI_QEI_GetSetConfiguration(volatile QEI_Type *pstcQei)
 * @brief       Retrieves the set configuration settings for the QEI.
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @return      Returns the set configuration register
 */
STATIC INLINE uint32_t RSI_QEI_GetSetConfiguration(volatile QEI_Type *pstcQei)
{
  return pstcQei->QEI_CTRL_REG_SET;
}

/*===================================================*/
/**
 * @fn          STATIC INLINE uint32_t RSI_QEI_GetClrConfiguration(volatile QEI_Type *pstcQei)
 * @brief       Retrieves the clear configuration settings for the QEI.
 * @param[in]   pstcQei : Pointer to the QEI register instance
 * @return      Returns the clear configuration register
 */
STATIC INLINE uint32_t RSI_QEI_GetClrConfiguration(volatile QEI_Type *pstcQei)
{
  return pstcQei->QEI_CTRL_REG_RESET;
}

#ifdef __cplusplus
}
#endif

#endif // RSI_QEI_H
/** @} */
