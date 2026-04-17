/***************************************************************************/ /**
* @file  rsi_ct.h
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

#include "rsi_ccp_common.h"
#include "base_types.h"
#include "rsi_error.h"

#ifndef RSI_CT_H
#define RSI_CT_H

#ifdef __cplusplus
extern "C" {
#endif

#define RSI_CT_API_VERSION RSI_DRIVER_VERSION_MAJOR_MINOR(2, 00)  // API version 0.1
#define RSI_CT_DRV_VERSION RSI_DRIVER_VERSION_MAJOR_MINOR(00, 01) // driver version 0.1

#define CLK_ENABLE_SET_3_REG (*((uint32_t volatile *)(0x46000000 + 0x10)))
#define M4SS_CT_INTR_SEL     (*((uint32_t volatile *)(0x46110000 + 0x10)))

#define ENABLE  1
#define DISABLE 0

#define LOW  0
#define HIGH 1

#define INPUT_CNT    4
#define OUTPUT_CNT   8
#define COUNTERS_CNT 2
#define EVENT_CNT    38
#define DMA_MODE     1

//CT Interrupt Flags
#define RSI_CT_EVENT_INTR_0_l            (1UL << 0)  // Counter 0 Interrupt event interrupt flag
#define RSI_CT_EVENT_FIFO_0_FULL_l       (1UL << 1)  // FIFO full signal of Counter 0 interrupt flag
#define RSI_CT_EVENT_COUNTER_0_IS_ZERO_l (1UL << 2)  // Counter 0 hit zero in active mode interrupt flag
#define RSI_CT_EVENT_COUNTER_0_IS_PEAK_l (1UL << 3)  // Counter 0 hit peak (MATCH) in active mode interrupt flag
#define RSI_CT_EVENT_INTR_1_l            (1UL << 16) // Counter 1 Interrupt event interrupt flag
#define RSI_CT_EVENT_FIFO_1_FULL_l       (1UL << 17) // FIFO full signal of Counter 1 interrupt flag
#define RSI_CT_EVENT_COUNTER_1_IS_ZERO_l (1UL << 18) // Counter 1 hit zero in active mode interrupt flag
#define RSI_CT_EVENT_COUNTER_1_IS_PEAK_l (1UL << 19) // Counter 1 hit peak (MATCH) in active mode interrupt flag

#define COUNTER_0   0
#define COUNTER_1   1
#define COUNTER_0_1 2

// CT Control Set Register Bits
#define COUNTER0_UP      (0x1 << 4)
#define COUNTER0_DOWN    (0x2 << 4)
#define COUNTER0_UP_DOWN (0x3 << 4)

#define COUNTER1_UP      (0x1 << 20)
#define COUNTER1_DOWN    (0x2 << 20)
#define COUNTER1_UP_DOWN (0x3 << 20)

#define COUNTER32_BITMODE    (1UL << 0)  // 32 bit Counter mode
#define SOFTRESET_COUNTER_0  (1UL << 1)  // soft reset counter 0
#define PERIODIC_ENCOUNTER_0 (1UL << 2)  // Periodic/One shot counter 0
#define COUNTER0_TRIG        (1UL << 3)  // trigger Counter 0
#define COUNTER0_SYNC_TRIG   (1UL << 6)  // Sync trigger for counter 0
#define BUF_REG0EN           (1UL << 7)  // Buffer enable for counter 0
#define SOFTRESET_COUNTER_1  (1UL << 17) // soft reset counter 1
#define PERIODIC_ENCOUNTER_1 (1UL << 18) // Periodic/One shot counter 1
#define COUNTER1_TRIG        (1UL << 19) // trigger Counter 1
#define COUNTER1_SYNC_TRIG   (1UL << 22) // Sync trigger for counter 1
#define BUF_REG1EN           (1UL << 23) // Buffer enable for counter 1

#define RISE_EDGE_MASK0 (1UL << 0)
#define RISE_EDGE_MASK1 (1UL << 1)
#define RISE_EDGE_MASK2 (1UL << 2)
#define RISE_EDGE_MASK3 (1UL << 3)

#define FALL_EDGE_MASK0 (1UL << 4)
#define FALL_EDGE_MASK1 (1UL << 5)
#define FALL_EDGE_MASK2 (1UL << 6)
#define FALL_EDGE_MASK3 (1UL << 7)

#define RFE_EDGE_MASK0 (1UL << 8)
#define RFE_EDGE_MASK1 (1UL << 9)
#define RFE_EDGE_MASK2 (1UL << 10)
#define RFE_EDGE_MASK3 (1UL << 11)

#define LEVEL_LOW_MASK0 (1UL << 12)
#define LEVEL_LOW_MASK1 (1UL << 13)
#define LEVEL_LOW_MASK2 (1UL << 14)
#define LEVEL_LOW_MASK3 (1UL << 15)

#define LEVEL_HIGH_MASK0 (1UL << 16)
#define LEVEL_HIGH_MASK1 (1UL << 17)
#define LEVEL_HIGH_MASK2 (1UL << 18)
#define LEVEL_HIGH_MASK3 (1UL << 19)

// CT example defines
#define OUTPUT_OCU_0             (1UL << 0)  // Counter 0 output OCU
#define OCU_DMA_0                (1UL << 4)  // Counter 0 OCU DMA enable
#define OCU_8_MODE_0             (1UL << 5)  // OCU using 16 bit mode
#define OUTPUT_OCU_1             (1UL << 16) // Counter 1output OCU
#define OCU_DMA_1                (1UL << 20) // Counter 1 OCU DMA enable
#define OCU_8_MODE_1             (1UL << 21) // OCU using 16 bit mode
#define MAKE_OUTPUT_0_HIGH_SEL_0 (0x2 << 6)
#define MAKE_OUTPUT_0_LOW_SEL_0  (0x3 << 9)
#define MAKE_OUTPUT_1_HIGH_SEL_1 (0x3 << 22)
#define MAKE_OUTPUT_1_LOW_SEL_1  (0x2 << 25)
#define OCU_SYNC_WITH_0          (0x1 << 1)
#define OCU_SYNC_WITH_1          (0x1 << 17)

#define OUTPUT_0_TGL_0_MAX 7    // Counter 0 output OCU
#define OUTPUT_0_TGL_1_MAX 7    // Counter 0 OCU DMA enable
#define TGL_CNT_0_PEAK     0xFF // OCU using 16 bit mode
#define OUTPUT_1_TGL_0_MAX 7    // Counter 0 output OCU
#define OUTPUT_1_TGL_1_MAX 7    // Counter 0 OCU DMA enable
#define TGL_CNT_1_PEAK     0xFF

#define MAX_PEAK_VALUE_0 0x1000
#define MAX_PEAK_VLAUE_1 0x2000

#define UDMA_CHNL_0 0
#define UDMA_CHNL_1 1
#define UDMA_CHNL_2 2
#define UDMA_CHNL_3 3
#define UDMA_CHNL_4 4
#define UDMA_CHNL_5 5
#define UDMA_CHNL_6 6
#define UDMA_CHNL_7 7

#define UDMA_CONTROL_TABLE_BASE_CHNL0 0x00000000
#define UDMA_CONTROL_TABLE_BASE_CHNL1 0x00000010
#define WFG_ON_PERIOD                 0xFF

#define USE_DMA 1
#define NO_DMA  0

#define PERIODIC_EN_COUNTER_0_FRM_REG (1UL << 2)
#define PERIODIC_EN_COUNTER_1_FRM_REG (1UL << 18)
#define COUNTER_0_TRIG                (1UL << 3)
#define COUNTER_1_TRIG                (1UL << 19)
#define COUNTER_0_UP                  (0x1 << 4)
#define COUNTER_1_UP                  (0x1 << 20)

#define MAKE_OUTPUT_0_TGL_SEL_1 1

#define AND_COUNTER0(valid, event) ((event << 0) | (valid << 8))
#define AND_COUNTER1(valid, event) ((event << 16) | (valid << 24))

#define OR_COUNTER0(valid, event) ((event << 0) | (valid << 8))
#define OR_COUNTER1(valid, event) ((event << 16) | (valid << 24))

typedef CT0_Type RSI_CT_T;
typedef CT_MUX_REG_Type RSI_CT_MUX_REG_T;

/**============================================================================
 * @brief CT Driver Capabilities.
  ============================================================================**/

typedef struct {
  unsigned int NoOfInputs : 3;    /*!< Number of Input signals                           */
  unsigned int NoOfOutputs : 4;   /*!< Number of Output signlas                          */
  unsigned int NumOfCounters : 4; /*!< Number of Counters present in CT module           */
  unsigned int NoOfEvents : 6;    /*!< Number of Events can be generated by input events */
  unsigned int DMAIntf : 1;       /*!< Supports DMA interface or not                     */
} RSI_CT_CAPABILITIES_T;
/****** CT Events ******/
#define CT_EVT_0  1
#define CT_EVT_1  2
#define CT_EVT_2  3
#define CT_EVT_3  4
#define CT_EVT_4  5
#define CT_EVT_5  6
#define CT_EVT_6  7
#define CT_EVT_7  8
#define CT_EVT_8  9
#define CT_EVT_9  10
#define CT_EVT_10 11
#define CT_EVT_11 12
#define CT_EVT_12 13
#define CT_EVT_13 14
#define CT_EVT_14 15
#define CT_EVT_15 16
#define CT_EVT_16 17
#define CT_EVT_17 18
#define CT_EVT_18 19
#define CT_EVT_19 20
#define CT_EVT_20 21
#define CT_EVT_21 22
#define CT_EVT_22 23
#define CT_EVT_23 24
#define CT_EVT_24 25
#define CT_EVT_25 26
#define CT_EVT_26 27
#define CT_EVT_27 28
#define CT_EVT_28 29
#define CT_EVT_29 30
#define CT_EVT_30 31
#define CT_EVT_31 32
#define CT_EVT_32 33
#define CT_EVT_33 34
#define CT_EVT_34 35
#define CT_EVT_35 36
#define CT_EVT_36 37
#define CT_EVT_37 38

// CT Callback Flags
#define CT_INTR_0_l_CB            0x01 // Callback ID for Counter 0 Interrupt event
#define CT_FIFO_0_FULL_l_CB       0x02 // Callback ID for FIFO full signal of Counter 0
#define CT_COUNTER_0_IS_ZERO_l_CB 0x03 // Callback ID for Counter 0 hit zero in active mode
#define CT_COUNTER_0_IS_PEAK_l_CB 0x04 // Callback ID for Counter 0 hit peak (MATCH) in active mode
#define CT_INTR_1_l_CB            0x05 // Callback ID for Counter 1 Interrupt event
#define CT_FIFO_1_FULL_l_CB       0x06 // Callback ID for FIFO full signal of Counter 1
#define CT_COUNTER_1_IS_ZERO_l_CB 0x07 // Callback ID for Counter 1 hit zero in active mode
#define CT_COUNTER_1_IS_PEAK_l_CB 0x08 // Callback ID for Counter 1 hit peak (MATCH) in active mode
#define CT_OCU_DMA_EN_COUNTER0    0x09 // OCU DMA call back
#define CT_OCU_DMA_EN_COUNTER1    0x0A

// OCU parameter structure
typedef struct OCU_PARAMS {
  uint16_t CompareVal1_0; /*!< two thresholds for counter 0 */
  uint16_t CompareVal2_0;
  uint16_t CompareVal1_1;
  uint16_t CompareVal2_1;
  uint16_t CompareNextVal1_0; /*!< two next thresholds for counter 0 */
  uint16_t CompareNextVal2_0;
  uint16_t CompareNextVal1_1; // next threshold for counter 1
  uint16_t CompareNextVal2_1; // next threshold for counter 1
  uint16_t SyncWith0;         // Sync Value
  uint16_t SyncWith1;
} OCU_PARAMS_T;

// WFG control parameters structure
typedef struct WFG_PARAMS {
  uint32_t output0_tgl0_sel;
  uint32_t output0_tgl1_sel;
  uint32_t tgl_cnt0_peak;
  uint32_t output1_tgl0_sel;
  uint32_t output1_tgl1_sel;
  uint32_t tgl_cnt1_peak;
} WFG_PARAMS_T;

// brief MCPWM Callback structure
typedef struct {
  void (*cbFunc)(uint8_t event, uint8_t counterNum); // Call back function pointer
} RSI_CT_CALLBACK_T;

// brief CT handle type
typedef void *RSI_CT_HANDLE_T;

/*===================================================*/
/**
 * @fn            void RSI_CT_Config(RSI_CT_T *pCT, boolean_t cfg)
 * @brief     This API is used to set the 32bit/ 16bit coutners 
 * @param[in]   pCT  : Pointer to the CT instance register area
 * @param[in]   cfg  : if cfg = 0 32bit Counter, cfg = 1 16bit counter
 * @return      none
 */
STATIC INLINE void RSI_CT_Config(RSI_CT_T *pCT, boolean_t cfg)
{
  // 32bit/16bit Counters
  if (cfg == 1) {
    pCT->CT_GEN_CTRL_SET_REG = (1 << 0);
  } else {
    pCT->CT_GEN_CTRL_RESET_REG = (1 << 0);
  }
}

/*===================================================*/
/**
 * @fn            void RSI_CT_SetControl(RSI_CT_T *pCT, uint32_t value)
 * @brief     Programs General control register bit fields
 * @param[in]   pCT   :  Pointer to the CT instance register area
 * @param[in]   value :  This parameter can be the logical OR of the below parameters
               - \ref COUNTER32_BITMODE    : sets 32bit mode 
               - \ref SOFTRESET_COUNTER_0  : Resets coutner 0
               - \ref PERIODIC_ENCOUNTER_0 : sets periodic mode
               - \ref COUNTER0_TRIG        : starts counter 0
               - \ref COUNTER0_UP_DOWN     : Counter 0 direction (0,1,2,3)
                 - \ref COUNTER0_SYNC_TRIG   : This enables the counter 0 to run/active when sync is found.
               - \ref BUF_REG0EN       : Buffer will be enabled and in path for Counter 0
               - \ref SOFTRESET_COUNTER_1  : Resets Counter 1
               - \ref PERIODIC_ENCOUNTER_1 : sets periodic mode
                 - \ref COUNTER1_TRIG        : starts counter 1
               - \ref COUNTER1_UP_DOWN     : Counter 1 direction (0,1,2,3)
               - \ref COUNTER1_SYNC_TRIG   : This enables the counter 1 to run/active when sync is found.
               - \ref BUF_REG1EN       : Buffer will be enabled and in path for counter 1.                    
 * @return      none
*/
STATIC INLINE void RSI_CT_SetControl(RSI_CT_T *pCT, uint32_t value)
{
  // Sets required control bits
  pCT->CT_GEN_CTRL_SET_REG = value;
}

/*===================================================*/
/**
 * @fn            void RSI_CT_ClearControl(RSI_CT_T *pCT, uint32_t value)
 * @brief     This API is used to Reset the required control bits in general control set register
 * @param[in]   pCT    : Pointer to the CT instance register area
 * @param[in]   value  : This parameter can be the logical OR of the required bit ,fields in CT Reset Control register as below.
             - \ref COUNTER32_BITMODE    : Sets 16bit mode 
             - \ref PERIODIC_ENCOUNTER_0 : Sets Counter_0 will be in single count mode.
             - \ref COUNTER0_UP_DOWN     : counter 0 to run in up/down/up-down/down-up directions (0,1,2,3)
             - \ref BUF_REG0EN       : Buffer will be disabled and in path and in path for Counter 0
             - \ref PERIODIC_ENCOUNTER_1 : sets Counter 1 will be in single count mode
             - \ref COUNTER1_UP_DOWN     : Counter 1 to run in up/down/up-down/down-up directions(0,1,2,3)
             - \ref BUF_REG1EN       : Buffer will be disabled and in path for counter 1.   
 * @return      none 
 */
STATIC INLINE void RSI_CT_ClearControl(RSI_CT_T *pCT, uint32_t value)
{
  // Resets required control bits
  pCT->CT_GEN_CTRL_RESET_REG = value;
}

/*===================================================*/
/**
 * @fn            void RSI_CT_StartEventSelect(RSI_CT_T *pCT,uint32_t value)
 * @brief     This API is used to select the input event to start any counter
 * @param[in]   pCT    : Pointer to the CT instance register area
 * @param[in]   value  : ORed value of Events for counter0 and counter1
             \n This parameter can be the logical OR of the event number for counter 0 and counter 1
             \n possible event numbers are 1 to 38 if 0 : No event is selected
 * @return      none
 */
STATIC INLINE void RSI_CT_StartEventSelect(RSI_CT_T *pCT, uint32_t value)
{
  pCT->CT_START_COUNTER_EVENT_SEL = value;
}

/*===================================================*/
/**
 * @fn            void RSI_CT_StartEventConfig(RSI_CT_T *pCT,uint32_t andValue,uint32_t orValue)  
 * @brief     This API is used to Configure AND/OR event for start counter operation
 * @param[in]   pCT      : Pointer to the CT instance register area
 * @param[in]   andValue : This parameter can be the logical OR of the below parameters.
               -  START_COUNTER_0_AND_EVENT : AND expression for AND event in start Counter_0 event, 
                              possible values are 0x0 to 0xF.
               -  START_COUNTER_0_AND_VLD   : Indicates which bits valid for considering AND event,
                              possible values are 0x0 to 0xF.
               -  START_COUNTER_1_AND_EVENT : AND expression for AND event in start Counter_1 event, 
                              possible values are 0x0 to 0xF.
               -  START_COUNTER_1_AND_VLD   : Indicates which bits valid for considering AND event,
                              possible values are 0x0 to 0xF. 
 * @param[in]   orValue  : This parameter can be the logical OR of the below parameters
              -  START_COUNTER_0_OR_EVENT : OR expression for OR event in start Counter_0 event, 
                              possible values are 0x0 to 0xF.
              -  START_COUNTER_0_OR_VLD   : Indicates which bits valid for considering OR event,
                              possible values are 0x0 to 0xF. 
              -  START_COUNTER_1_OR_EVENT : OR expression for OR event in start Counter_1 event, 
                              possible values are 0x0 to 0xF.
              -  START_COUNTER_1_OR_VLD   : Indicates which bits valid for considering OR event,
                              possible values are 0x0 to 0xF. 
 * @return      none
 */
STATIC INLINE void RSI_CT_StartEventConfig(RSI_CT_T *pCT, uint32_t andValue, uint32_t orValue)
{
  // AND Event configuration for counter start
  pCT->CT_START_COUNTER_AND_EVENT |= andValue;
  // OR Event configuration for counter start
  pCT->CT_START_COUNTER_OR_EVENT |= orValue;
}

/*===================================================*/
/**
 * @fn            void RSI_CT_StopEventConfig(RSI_CT_T *pCT,uint32_t andValue,uint32_t orValue)
 * @brief     This API is used to Configure AND/OR event for stop counter operation
 * @param[in]   pCT      :  Pointer to the CT instance register area
 * @param[in]   andValue :  Stop Counter AND Event register value, this parameter can be the logical OR of the 
                -  STOP_COUNTER_0_AND_EVENT : AND expression for AND event in stop Counter_0 event,
                              possible values are 0x0 to 0xF,
                -  STOP_COUNTER_0_AND_VLD   : Indicates which bits valid for considering AND event,
                              possible values are 0x0 to 0xF  
                -  STOP_COUNTER_1_AND_EVENT : AND expression for AND event in stop Counter_1 event,
                              possible values are 0x0 to 0xF,
                -  STOP_COUNTER_1_AND_VLD   : Indicates which bits valid for considering AND event,
                              possible values are 0x0 to 0xF
 * @param[in]   orValue  :  Stop Counter OR Event register value,this parameter can be the logical OR of the 
                -  STOP_COUNTER_0_OR_EVENT : OR expression for OR event in stop Counter_0 event,
                               possible values are 0x0 to 0xF,
                -  STOP_COUNTER_0_OR_VLD   : Indicates which bits valid for considering OR event,
                               possible values are 0x0 to 0xF
                -  STOP_COUNTER_1_OR_EVENT : OR expression for OR event in stop Counter_1 event,
                               possible values are 0x0 to 0xF,
                -  STOP_COUNTER_1_OR_VLD   : Indicates which bits valid for considering OR event,
                               possible values are 0x0 to 0xF
 * @return      none
 */
STATIC INLINE void RSI_CT_StopEventConfig(RSI_CT_T *pCT, uint32_t andValue, uint32_t orValue)
{
  // AND Event configuration for counter stop
  pCT->CT_STOP_COUNTER_AND_EVENT |= andValue;
  // OR Event configuration for counter stop
  pCT->CT_STOP_COUNTER_OR_EVENT |= orValue;
}

/*===================================================*/
/**
 * @fn            void RSI_CT_ContinueEventConfig(RSI_CT_T *pCT,uint32_t andValue,uint32_t orValue) 
 * @brief     This API is used to Configure AND/OR event for Continue operation
 * @param[in]   pCT      : Pointer to the CT instance register area
 * @param[in]   andValue : Continue Counter AND Event register value,this parameter can be the logical OR of the 
                -  CONTINUE_COUNTER_0_AND_EVENT : AND expression for AND event in continue Counter_0 event,
                                possible values are 0x0 to 0xF,
                -  CONTINUE_COUNTER_0_AND_VLD   : Indicates which bits valid for considering AND event,
                                 possible values are 0x0 to 0xF
                -  CONTINUE_COUNTER_1_AND_EVENT : AND expression for AND event in continue Counter_1 event,
                                possible values are 0x0 to 0xF,
                -  CONTINUE_COUNTER_1_AND_VLD   : Indicates which bits valid for considering AND event,
                                possible values are 0x0 to 0xF
 * @param[in]   orValue  :  Continue Counter OR Event register value,this parameter can be the logical OR of the 
              -  CONTINUE_COUNTER_0_OR_EVENT : OR expression for OR event in continue Counter_0 event,
                               possible values are 0x0 to 0xF,
              -  CONTINUE_COUNTER_0_OR_VLD   : Indicates which bits valid for considering OR event,
                               possible values are 0x0 to 0xF
              -  CONTINUE_COUNTER_1_OR_EVENT : OR expression for OR event in continue Counter_1 event,
                               possible values are 0x0 to 0xF,
              -  CONTINUE_COUNTER_1_OR_VLD   : Indicates which bits valid for considering OR event,
                               possible values are 0x0 to 0xF
 * @return      none
 */
STATIC INLINE void RSI_CT_ContinueEventConfig(RSI_CT_T *pCT, uint32_t andValue, uint32_t orValue)
{
  // AND Event configuration for counter continue
  pCT->CT_CONTINUE_COUNTER_AND_EVENT |= andValue;
  // OR Event configuration for counter continue
  pCT->CT_CONTINUE_COUNTER_OR_EVENT |= orValue;
}

/*===================================================*/
/**
 * @fn            void RSI_CT_HaltEventConfig(RSI_CT_T *pCT,uint32_t andValue,uint32_t orValue) 
 * @brief     This API is used to Configure AND/OR event for Halt operation
 * @param[in]   pCT      : Pointer to the CT instance register area
 * @param[in]   andValue :  Halt Counter AND Event register value,This parameter can be the logical OR of the 
                -  HALT_COUNTER_0_AND_EVENT : AND expression for AND event in halt Counter_0 event,
                                possible values are 0x0 to 0xF,
                -  HALT_COUNTER_0_AND_VLD   : Indicates which bits valid for considering AND event,
                                possible values are 0x0 to 0xF
                -  HALT_COUNTER_1_AND_EVENT : AND expression for AND event in halt Counter_1 event,
                                possible values are 0x0 to 0xF,
                -  HALT_COUNTER_1_AND_VLD   : Indicates which bits valid for considering AND event,
                              possible values are 0x0 to 0xF  
 * @param[in]   orValue  :  Halt Counter OR Event register value
               \n This parameter can be the logical OR of the 
              -  HALT_COUNTER_0_OR_EVENT : OR expression for OR event in halt Counter_0 event,
                             possible values are 0x0 to 0xF,
              -  HALT_COUNTER_0_OR_VLD   : Indicates which bits valid for considering OR event,
                             possible values are 0x0 to 0xF
              -  HALT_COUNTER_1_OR_EVENT : OR expression for OR event in halt Counter_1 event,
                             possible values are 0x0 to 0xF,
              -  HALT_COUNTER_1_OR_VLD   : Indicates which bits valid for considering OR event,
                             possible values are 0x0 to 0xF
 * @return      none
 */
STATIC INLINE void RSI_CT_HaltEventConfig(RSI_CT_T *pCT, uint32_t andValue, uint32_t orValue)
{
  // AND Event configuration for counter Halt
  pCT->CT_HALT_COUNTER_AND_EVENT |= andValue;
  // OR Event configuration for counter Halt
  pCT->CT_HALT_COUNTER_OR_EVENT |= orValue;
}

/*===================================================*/
/**
 * @fn        void RSI_CT_IncrementEventConfig(RSI_CT_T *pCT,uint32_t andValue, uint32_t orValue)
 * @brief     This API is used to Configure AND/OR event for increment operation
 * @param[in]   pCT      : Pointer to the CT instance register area
 * @param[in]   andValue : Increment Counter AND Event register value,this parameter can be the logical OR of the 
               -  INCREMENT_COUNTER_0_AND_EVENT : AND expression for AND event in increment Counter_0 event,
                                  possible values are 0x0 to 0xF,
               -  INCREMENT_COUNTER_0_AND_VLD   : Indicates which bits valid for considering AND event,
                                possible values are 0x0 to 0xF
               -  INCREMENT_COUNTER_1_AND_EVENT : AND expression for AND event in increment Counter_1 event,
                                possible values are 0x0 to 0xF,
               -  INCREMENT_COUNTER_1_AND_VLD   : Indicates which bits valid for considering AND event,
                                possible values are 0x0 to 0xF
 * @param[in]   orValue  :  Increment Counter OR Event register value,this parameter can be the logical OR of the 
               -  INCREMENT_COUNTER_0_OR_EVENT :  OR expression for OR event in increment Counter_0 event,
                                possible values are 0x0 to 0xF,
               -  INCREMENT_COUNTER_0_OR_VLD   :  Indicates which bits valid for considering OR event,
                                possible values are 0x0 to 0xF
               -  INCREMENT_COUNTER_1_OR_EVENT :  OR expression for OR event in increment Counter_1 event,
                                possible values are 0x0 to 0xF,
               -  INCREMENT_COUNTER_1_OR_VLD   :  Indicates which bits valid for considering OR event,
                                  possible values are 0x0 to 0xF,
 * @return      none
 */
STATIC INLINE void RSI_CT_IncrementEventConfig(RSI_CT_T *pCT, uint32_t andValue, uint32_t orValue)
{
  // AND Event configuration for counter Increment
  pCT->CT_INCREMENT_COUNTER_AND_EVENT |= andValue;
  // OR Event configuration for counter Increment
  pCT->CT_INCREMENT_COUNTER_OR_EVENT |= orValue;
}

/*===================================================*/
/**
 * @fn            void RSI_CT_CaptureEventConfig(RSI_CT_T *pCT,uint32_t andValue,uint32_t orValue)
 * @brief     This API is used to Configure AND/OR event for capture operation
 * @param[in]   pCT      : Pointer to the CT instance register area
 * @param[in]   andValue : Capture Counter AND Event register value,This parameter can be the logical OR of the 
                -  CAPTURE_COUNTER_0_AND_EVENT : AND expression for AND event in capture Counter_0 event,
                                 possible values are 0x0 to 0xF,
                -  CAPTURE_COUNTER_0_AND_VLD   : Indicates which bits valid for considering AND event,
                                 possible values are 0x0 to 0xF
                -  CAPTURE_COUNTER_1_AND_EVENT : AND expression for AND event in capture Counter_1 event,
                                 possible values are 0x0 to 0xF,
                -  CAPTURE_COUNTER_1_AND_VLD   : Indicates which bits valid for considering AND event,
                                 possible values are 0x0 to 0xF
 * @param[in]   orValue  :  Capture Counter OR Event register value,This parameter can be the logical OR of the 
              -  CAPTURE_COUNTER_0_OR_EVENT : OR expression for OR event in capture Counter_0 event,
                              possible values are 0x0 to 0xF,
              -  CAPTURE_COUNTER_0_OR_VLD   : Indicates which bits valid for considering OR event,
                              possible values are 0x0 to 0xF
              -  CAPTURE_COUNTER_1_OR_EVENT : OR expression for OR event in capture Counter_1 event,
                              possible values are 0x0 to 0xF,
              -  CAPTURE_COUNTER_1_OR_VLD   : Indicates which bits valid for considering OR event,
                              possible values are 0x0 to 0xF
 * @return      none
*/
STATIC INLINE void RSI_CT_CaptureEventConfig(RSI_CT_T *pCT, uint32_t andValue, uint32_t orValue)
{

  // AND Event configuration for counter Capture
  pCT->CT_CAPTURE_COUNTER_AND_EVENT |= andValue;
  // OR Event configuration for counter Capture
  pCT->CT_CAPTURE_COUNTER_OR_EVENT |= orValue;
}

/*===================================================*/
/**
 * @fn            void RSI_CT_InterruptEventConfig(RSI_CT_T *pCT,uint32_t andValue,uint32_t orValue)
 * @brief     This API is used to Configure AND/OR event for interrupt operation.
 * @param[in]   pCT      :  Pointer to the CT instance register area
 * @param[in]   andValue :  Interrupt Counter AND Event register value,this parameter can be the logical OR of the 
                -  INTR_COUNTER_0_AND_EVENT : AND expression for AND event in interrupt Counter_0 event,
                              possible values are 0x0 to 0xF,
                -  INTR_COUNTER_0_AND_VLD   : Indicates which bits valid for considering AND event,
                              possible values are 0x0 to 0xF
                -  INTR_COUNTER_1_AND_EVENT : AND expression for AND event in capture Counter_1 event,
                              possible values are 0x0 to 0xF,
                -  INTR_COUNTER_1_AND_VLD   : Indicates which bits valid for considering AND event,
                              possible values are 0x0 to 0xF
 * @param[in]   orValue  :  Capture Counter OR Event register value,this parameter can be the logical OR of the 
                -  CAPTURE_COUNTER_0_OR_EVENT : OR expression for OR event in interrupt Counter_0 event,
                                possible values are 0x0 to 0xF,
                -  CAPTURE_COUNTER_0_OR_VLD   : Indicates which bits valid for considering OR event,
                                possible values are 0x0 to 0xF
                -  CAPTURE_COUNTER_1_OR_EVENT : OR expression for OR event in interrupt Counter_1 event,
                                possible values are 0x0 to 0xF,
                -  CAPTURE_COUNTER_1_OR_VLD   : Indicates which bits valid for considering OR event,
                                possible values are 0x0 to 0xF
 * @return      none
 */
STATIC INLINE void RSI_CT_InterruptEventConfig(RSI_CT_T *pCT, uint32_t andValue, uint32_t orValue)
{
  // AND Event configuration for counter Interrupt
  pCT->CT_INTR_AND_EVENT |= andValue;
  // OR Event configuration for counter Interrupt
  pCT->CT_INTR_OR_EVENT_REG |= orValue;
}

/*===================================================*/
/**
 * @fn            void RSI_CT_OutputEventConfig( RSI_CT_T *pCT,uint32_t andValue,uint32_t orValue)
 * @brief     This API is used to Configure AND/OR event for output operation.
 * @param[in]     pCT   Pointer to the CT instance register area
 * @param[in]   andValue : Output Counter AND Event register value,this parameter can be the logical OR of the 
               -  OUTPUT_COUNTER_0_AND_EVENT : AND expression for AND event in output Counter_0 event,
                               possible values are 0x0 to 0xF,
               -  OUTPUT_COUNTER_0_AND_VLD   : Indicates which bits valid for considering AND event,
                               possible values are 0x0 to 0xF
               -  OUTPUT_COUNTER_1_AND_EVENT : AND expression for AND event in output Counter_1 event,
                               possible values are 0x0 to 0xF,
               -  OUTPUT_COUNTER_1_AND_VLD   : Indicates which bits valid for considering AND event,
                                 possible values are 0x0 to 0xF
 * @param[in]   orValue  : Output Counter OR Event register value,this parameter can be the logical OR of the 
               -  OUTPUT_COUNTER_0_OR_EVENT : OR expression for OR event in output Counter_0 event,
                              possible values are 0x0 to 0xF,
               -  OUTPUT_COUNTER_0_OR_VLD   : Indicates which bits valid for considering OR event,
                                possible values are 0x0 to 0xF
               -  OUTPUT_COUNTER_1_OR_EVENT : OR expression for OR event in output Counter_1 event,
                                possible values are 0x0 to 0xF,
               -  OUTPUT_COUNTER_1_OR_VLD   : Indicates which bits valid for considering OR event,
                                possible values are 0x0 to 0xF
 * @return      none
 */
STATIC INLINE void RSI_CT_OutputEventConfig(RSI_CT_T *pCT, uint32_t andValue, uint32_t orValue)
{
  // AND Event configuration for counter Output
  pCT->CT_OUTPUT_AND_EVENT_REG |= andValue;
  // OR Event configuration for counter Output
  pCT->CT_OUTPUT_OR_EVENT |= orValue;
}
/*===================================================*/
/**
 * @fn        void RSI_CT_StopEventSelect(RSI_CT_T *pCT,uint32_t value)
 * @brief     This API is used to select the input event to stop counter
 * @param[in]   pCT   :  Pointer to the CT instance register area
 * @param[in]   value :  Stop Event select register value,this parameter can be the logical OR of the 
              -  STOP_EVENT_SEL_0 :event number for counter 0 
              -  STOP_EVENT_SEL_1 :event number for counter 1
              -  possible event numbers are 1 to 38 if 0 : No event is selected
 * @return      none
 */
STATIC INLINE void RSI_CT_StopEventSelect(RSI_CT_T *pCT, uint32_t value)
{
  // Event number to stop
  pCT->CT_STOP_COUNTER_EVENT_SEL = value;
}

/*===================================================*/
/**
 * @fn            void RSI_CT_ContinueEventSelect(RSI_CT_T *pCT,uint32_t value) 
 * @brief     Configures event for Continue operation of Counter
 * @param[in]   pCT    : Pointer to the CT instance register area
 * @param[in]   value  : Continue Event select register value,this parameter can be the logical OR of the 
              -  CONTINUE_EVENT_SEL_0 :event number for counter 0 
              -  CONTINUE_EVENT_SEL_1 :event number for counter 1
              -  possible event numbers are 1 to 38 if 0 : No event is selected
 * @return      none
 */
STATIC INLINE void RSI_CT_ContinueEventSelect(RSI_CT_T *pCT, uint32_t value)
{
  // Event number to continue
  pCT->CT_CONTINUE_COUNTER_EVENT_SEL = value;
}

/*===================================================*/
/**
 * @fn            void RSI_CT_HaltEventSelect(RSI_CT_T *pCT,uint32_t value) 
 * @brief     Configures event for HALT operation of Counter
 * @param[in]   pCT    :  Pointer to the CT instance register area
 * @param[in]   value  :  Halt Event select register value,this parameter can be the logical OR of the 
              -  HALT_EVENT_SEL_0 :event number for counter 0 
              -  HALT_EVENT_SEL_1 :event number for counter 1
              -  possible event numbers are 1 to 38 if 0 : No event is selected
 * @return      none
 */
STATIC INLINE void RSI_CT_HaltEventSelect(RSI_CT_T *pCT, uint32_t value)
{
  // Event number to Halt
  pCT->CT_HALT_COUNTER_EVENT_SEL = value;
}

/*===================================================*/
/**
 * @fn            void RSI_CT_IncrementEventSelect(RSI_CT_T *pCT,uint32_t value)
 * @brief     Configures event for Increment operation of Counter
 * @param[in]   pCT    : Pointer to the CT instance register area
 * @param[in]   value  : Increment Event select register value,this parameter can be the logical OR of the 
              -  INCREMENT_EVENT_SEL_0 :event number for counter 0 
              -  INCREMENT_EVENT_SEL_1 :event number for counter 1
              -  possible event numbers are 1 to 38 if 0 : No event is selected
 * @return      none 
 */
STATIC INLINE void RSI_CT_IncrementEventSelect(RSI_CT_T *pCT, uint32_t value)
{
  // Event number to Increment
  pCT->CT_INCREMENT_COUNTER_EVENT_SEL = value;
}

/*===================================================*/
/**
 * @fn            void RSI_CT_CaptureEventSelect(RSI_CT_T *pCT,uint32_t value)
 * @brief     This API is used to select the input event to capture counter value
 * @param[in]   pCT    : Pointer to the CT instance register area
 * @param[in]   value  : Capture Event select register value,this parameter can be the logical OR of the 
                 -  CAPTURE_EVENT_SEL_0 :event number for counter 0 
               -  CAPTURE_EVENT_SEL_1 :event number for counter 1
               -  possible event numbers are 1 to 38 if 0 : No event is selected
 * @return      none
 */
STATIC INLINE void RSI_CT_CaptureEventSelect(RSI_CT_T *pCT, uint32_t value)
{
  // Event number to capture
  pCT->CT_CAPTURE_COUNTER_EVENT_SEL = value;
}

/*===================================================*/
/**
 * @fn            void RSI_CT_OutputEventSelect(RSI_CT_T *pCT,uint32_t value)
 * @brief     This API is used to select the input event to output counter value
 * @param[in]   pCT    :  Pointer to the CT instance register area
 * @param[in]   value  :  Output Event select register value,this parameter can be the logical OR of the 
                -  OUTPUT_EVENT_SEL_0 :event number for counter 0 
                -  OUTPUT_EVENT_SEL_1 :event number for counter 1
                -  possible event numbers are 1 to 38 if 0 : No event is selected
 * @return      none 
 */
STATIC INLINE void RSI_CT_OutputEventSelect(RSI_CT_T *pCT, uint32_t value)
{
  // Event number to output event
  pCT->CT_OUTPUT_EVENT_SEL = value;
}

/*===================================================*/
/**
 * @fn        void RSI_CT_InterruptEventSelect(RSI_CT_T *pCT,uint32_t value)  
 * @brief     This API is used to select the input event for interrupt
 * @param[in]   pCT     :  Pointer to the CT instance register area
 * @param[in]   value   :  Output Event select register value,this parameter can be the logical OR of the 
                 -  INTR_EVENT_SEL_0 :event number for counter 0 
               -  INTR_EVENT_SEL_1 :event number for counter 1
               -  possible event numbers are 1 to 38 if 0 : No event is selected
 * @return      none 
 */
STATIC INLINE void RSI_CT_InterruptEventSelect(RSI_CT_T *pCT, uint32_t value)
{
  // Event number to interrupt
  pCT->CT_INTR_EVENT_SEL = value;
}

/*===================================================*/
/**
 * @fn            void RSI_CT_OutputEventADCTrigger(RSI_CT_MUX_REG_T *pCTmux,uint8_t output1,uint8_t output2)
 * @brief     This API is used to select one of the ADC trigger output
 * @param[in]   pCTmux  :  Pointer to the CT instance register area
 * @param[in]   output1 :  output event for ADC trigger (0 to 31)
 * @param[in]     output2 :  output event for ADC trigger (0 to 31)
 * @return      none
 */
STATIC INLINE void RSI_CT_OutputEventADCTrigger(RSI_CT_MUX_REG_T *pCTmux, uint8_t output1, uint8_t output2)
{
  // Sets the output pin number for ADC
  pCTmux->CT_OUTPUT_EVENT1_ADC_SEL_b.OUTPUT_EVENT_ADC_SEL = (unsigned int)(output1 & 0x0F);
  // Sets the output pin number for ADC
  pCTmux->CT_OUTPUT_EVENT2_ADC_SEL_b.OUTPUT_EVENT_ADC_SEL = (unsigned int)(output2 & 0x0F);
}

/*===================================================*/
/**
 * @fn            void RSI_CT_SetCount(RSI_CT_T *pCT, uint32_t count)
 * @brief     Sets the Counter Initial value.
 * @param[in]   pCT   : Pointer to the CT instance register area
 * @param[in]   count : 32 bit Counter initial value,this parameter can be the logical OR of the 
              - \ref COUNTER_0 : Counter 0 load value ( 0x0 to 0xFFFF)
              - \ref COUNTER_1 : Counter 1 load value ( 0x0 to 0xFFFF)
                          -  possible values are 0x0 to 0xFFFFFFFF
 * @return      none
 */
STATIC INLINE void RSI_CT_SetCount(RSI_CT_T *pCT, uint32_t count)
{
  // Sets the Coutner start value
  pCT->CT_COUNTER_REG = count;
}

/*===================================================*/
/** 
 * @fn            void RSI_CT_OCUConfigSet(RSI_CT_T *pCT, uint32_t value)
 * @brief     This API is used to set OCU control parameters
 * @param[in]   pCT   :    Pointer to the CT instance register area 
 * @param[in]   value :  OCU control register value,this parameter can be the logical OR of the below parameters
                -  OUTPUT_IS_OCU_0 : sets Counter 0 output in OCU mode
                -  SYNC_WITH_0   : Indicates whether the other channel is in sync with this counter(0,1,2,3)
                -  OCU_DMA_MODE_0  : OCU DMA mode is active or not for counter-0
                -  OCU_8_16_MODE_0 : 16 bits or only 8-bits of the counter-0 are used in OCU mode
                -  OUTPUT_IS_OCU_1 : sets Counter 1 output in OCU mode
                -  SYNC_WITH_1   : Indicates whether the other channel is in sync with this counter(0,1,2,3)
                -  OCU_DMA_MODE_1  : OCU DMA mode is active or not for counter-1
                -  OCU_8_16_MODE_1 : 16 bits or only 8-bits of the counter-1 are used in OCU mode
 * @return      none
 */
STATIC INLINE void RSI_CT_OCUConfigSet(RSI_CT_T *pCT, uint32_t value)
{
  // OCU control parameters
  pCT->CT_OCU_CTRL_REG |= value;
}

/*===================================================*/
/** 
 * @fn            void RSI_CT_OCUConfigReset(RSI_CT_T *pCT, uint32_t value)
 * @brief     This API is used to set OCU control parameters
 * @param[in]   pCT   :  Pointer to the CT instance register area 
 * @param[in]   value :  OCU control register value,this parameter can be the logical OR required bit.
 * @return      none
 */
STATIC INLINE void RSI_CT_OCUConfigReset(RSI_CT_T *pCT, uint32_t value)
{
  // OCU control parameters
  pCT->CT_OCU_CTRL_REG &= ~value;
}

/*===================================================*/
/**
 * @fn            void RSI_CT_InterruptEnable(RSI_CT_T *pCT, uint32_t unmaskFlags)   
 * @brief     Enable the interrupts in State Configurable Timer.
 * @param[in]   pCT         : Pointer to the CT instance register area
 * @param[in]   unmaskFlags : CT Interrupt unmask register value,this parameter can be the logical OR of the
                 -  INTR_0_l        : interrupt event flag for counter 0
                 -  COUNTER_0_IS_ZERO_L : counter hit zero for counter 0
                 -  COUNTER_0_IS_PEAK_L : counter hit peak for counter 0
                 -  INTR_1_L        : interrupt event flag for counter 1        
                 -  COUNTER_0_IS_ZERO_L : counter hit zero for counter 1
                 -  COUNTER_0_IS_PEAK_L : counter hit peak for counter 1  
 * @return      none 
 */
STATIC INLINE void RSI_CT_InterruptEnable(RSI_CT_T *pCT, uint32_t unmaskFlags)
{
  // multi channel VIC enable
  M4SS_CT_INTR_SEL = 0xFFFFFFFF;

  // unmasks required interrupt flags
  pCT->CT_INTER_UNMASK = unmaskFlags;
}

/*===================================================*/
/**
 * @fn            void RSI_CT_InterruptDisable(RSI_CT_T *pCT, uint32_t maskFlags)
 * @brief     Disable the interrupts in State Configurable Timer
 * @param[in]   pCT        :  Pointer to the CT instance register area
 * @param[in]   maskFlags  :  CT Interrupt mask register value,this parameter can be the logical OR of the
                 -  INTR_0_l        : interrupt event flag for counter 0
                 -  COUNTER_0_IS_ZERO_L : counter hit zero for counter 0
                   -  COUNTER_0_IS_PEAK_L : counter hit peak for counter 0
                   -  INTR_1_L      : interrupt event flag for counter 1        
                 -  COUNTER_0_IS_ZERO_L : counter hit zero for counter 1
                 -  COUNTER_0_IS_PEAK_L : counter hit peak for counter 1      
 * @return      none    
 */
STATIC INLINE void RSI_CT_InterruptDisable(RSI_CT_T *pCT, uint32_t maskFlags)
{
  // unmasks required interrupt flags
  pCT->CT_INTR_MASK = maskFlags;
}

/*===================================================*/
/**
 * @fn            void RSI_CT_InterruptClear(RSI_CT_T *pCT,uint32_t clr_flags)
 * @brief     Clear the specified interrupt flag in State Configurable Timer
 * @param[in]   pCT        :  Pointer to the CT instance register area
 * @param[in]   clr_flags  :  CT Interrupt Ack register value,this parameter can be the logical OR of the
                -  INTR_0_l        : interrupt event flag for counter 0
                -  COUNTER_0_IS_ZERO_L : counter hit zero for counter 0
                -  COUNTER_0_IS_PEAK_L : counter hit peak for counter 0
                -  INTR_1_L        : interrupt event flag for counter 1       
                -  COUNTER_0_IS_ZERO_L : counter hit zero for counter 1
                -  COUNTER_0_IS_PEAK_L : counter hit peak for counter 1                 
 * @return      none      
 */
STATIC INLINE void RSI_CT_InterruptClear(RSI_CT_T *pCT, uint32_t clrFlags)
{
  // Clears required interrupt flags
  pCT->CT_INTR_ACK = clrFlags;
}

/*===================================================*/
/**
 * @fn           uint32_t RSI_CT_GetInterruptStatus(const RSI_CT_T *pCT)
 * @brief    Clear the specified interrupt flag in State Configurable Timer
 * @param[in]  pCT      :   Pointer to the CT instance register area
 * @return     CT Interrupt  status value  
 */
STATIC INLINE uint32_t RSI_CT_GetInterruptStatus(const RSI_CT_T *pCT)
{
  return (pCT->CT_INTR_STS);
}

/*===================================================*/
/**
 * @fn          void RSI_CT_EdgeLevelEventControl(const RSI_CT_T *pCT,uint32_t value)
 * @brief   This API is used to control the input event generation to CT
 * @param[in]   pSCT    :  Pointer to the SCT instance register area
 * @param[in]   value   :  Mask value
 * @return    none               
 */
STATIC INLINE void RSI_CT_EdgeLevelEventControl(const RSI_CT_T *pCT, uint32_t value)
{
  //pCT ->RE_FE_RFE_LEV0_LEV1_EVENT_ENABLE_REG = value;
  (void)pCT;
  (void)value;
}

/*===================================================*/
/**
 * @fn          void RSI_CT_SetTimerMuxSelect( RSI_CT_MUX_REG_T *pCTMux,uint8_t timerIns )
 * @brief   To Select Timer using mux.
 * @param[in]   pCTMux       :  Pointer to the CT Mux instance register area
 * @param[in]   timerIns     :  If 0 - Timer0 instance,if 1 - Timer1 instance
 * @return    none               
 */
STATIC INLINE void RSI_CT_SetTimerMuxSelect(RSI_CT_MUX_REG_T *pCTMux, uint8_t timerIns)
{
  pCTMux->CT_MUX_SEL_0_REG = timerIns;
  pCTMux->CT_MUX_SEL_1_REG = timerIns;
}

/*===================================================*/
/**
 * @fn          void RSI_CT_ResumeHaltEvent(RSI_CT_T *pCT,boolean_t counterNum)
 * @brief   To Resume the HALT operation of counter with I/O events
 * @param[in]   pCT         :  Pointer to the CT instance register area
 * @param[in]   counterNum  :  Counter 0/1
 * @return    none               
 */
STATIC INLINE void RSI_CT_ResumeHaltEvent(RSI_CT_T *pCT, boolean_t counterNum)
{
  if (counterNum) {
    pCT->CT_HALT_COUNTER_EVENT_SEL_b.RESUME_FROM_HALT_COUNTER_1 = 0x1;
  } else {
    pCT->CT_HALT_COUNTER_EVENT_SEL_b.RESUME_FROM_HALT_COUNTER_0 = 0x1;
  }
}

/*===================================================*/
/** 
 * @fn            rsi_error_t RSI_CT_PeripheralReset(RSI_CT_T *pCT,boolean_t counterNum)
 * @brief     This API is used to Reset any counter.
 * @param[in]   pCT         :  Pointer to the CT instance register area
 * @param[in]     counterNum  :  Counter 0/1
 * @return      none
 */
STATIC INLINE void RSI_CT_PeripheralReset(RSI_CT_T *pCT, boolean_t counterNum)
{
  if (counterNum) {
    // Counter 1 reset
    pCT->CT_GEN_CTRL_SET_REG = (1 << 17);
  } else {
    // Counter 0 reset
    pCT->CT_GEN_CTRL_SET_REG = (1 << 1);
  }
}

/*===================================================*/
/**
 * @fn            rsi_error_t RSI_CT_StartSoftwareTrig(RSI_CT_T *pCT,boolean_t counterNum)
 * @brief     Starts the Counter form software register
 * @param[in]   pCT         :  Pointer to the CT instance register area
 * @param[in]   counterNum  :  Coutner 0/1
 * @return      none
 */
STATIC INLINE void RSI_CT_StartSoftwareTrig(RSI_CT_T *pCT, boolean_t counterNum)
{
  // starts the required counter */
  if (counterNum) {
    pCT->CT_GEN_CTRL_SET_REG = (1 << 19);
  } else {
    pCT->CT_GEN_CTRL_SET_REG = (1 << 3);
  }
}

/*===================================================*/
/**
 * @fn            rsi_error_t RSI_CT_OCUModeSet(RSI_CT_T *pCT, boolean_t counterNum)
 * @brief     OCU mode configuration setting
 * @param[in]   pCT         :  Pointer to the CT instance register area
 * @param[in]   counterNum    :  Coutner 0/1
 * @return      none
 */
STATIC INLINE void RSI_CT_OCUModeSet(RSI_CT_T *pCT, boolean_t counterNum)
{
  if (counterNum) {
    // set the output in OCU mode for counter 1
    pCT->CT_OCU_CTRL_REG_b.OUTPUT_1_IS_OCU = 1;
  } else {
    // set the output in OCU mode for counter 0
    pCT->CT_OCU_CTRL_REG_b.OUTPUT_IS_OCU_0 = 1;
  }
}

/*===================================================*/
/** 
 * @fn            void RSI_CT_SetMatchCount(RSI_CT_T *pCT, uint32_t value,boolean_t counterMode, boolean_t counterNum) 
 * @brief     Sets the Match load value for counter 0 and counter 1
 * @param[in]   pCT        :  Pointer to the SCT instance register area 
 * @param[in]   value      :  Match register or match buffer register value,this parameter 
                               \n can be the logical OR of the 
                 -  COUNTER_0_MATCH : Counter 0 match value ( 0x0 to 0xFFFF)
                 -  COUNTER_1_MATCH : Counter 1 match value ( 0x0 to 0xFFFF)
 * @param[in]   counterMode :  counterMode 0/1 
 * @param[in]     counterNum  :  Counter 0/1
 * @return      none
 */
STATIC INLINE void RSI_CT_SetMatchCount(RSI_CT_T *pCT, uint32_t value, boolean_t counterMode, boolean_t counterNum)
{
  if (counterMode == 0) {
    if (counterNum) {
      pCT->CT_MATCH_REG_b.COUNTER_1_MATCH = (uint16_t)value;
    } else {
      pCT->CT_MATCH_REG_b.COUNTER_0_MATCH = (uint16_t)value;
    }
  } else {
    pCT->CT_MATCH_REG = (uint16_t)value;
  }
}

/*===================================================*/
/**
 * @fn          uint16_t RSI_CT_CaptureRead(const RSI_CT_T *pCT, boolean_t counterNum)
 * @brief   Gets the captured counter value 
 * @param[in]   pCT         :  Pointer to the CT instance register area
 * @param[in]   counterNum  :  Counter 0/1
 * @return    Return counter value at the time of capture event occurs
 */
STATIC INLINE uint16_t RSI_CT_CaptureRead(const RSI_CT_T *pCT, boolean_t counterNum)
{
  if (counterNum) {
    return (pCT->CT_CAPTURE_REG_b.COUNTER_1_CAPTURE);
  } else {
    return (pCT->CT_CAPTURE_REG_b.COUNTER_0_CAPTURE);
  }
}

/*===================================================*/
/**
 * @fn          uint32_t RSI_CT_GetCounter(const RSI_CT_T *pCT,boolean_t counterNum,boolean_t mode )
 * @brief   Gets the captured counter value 
 * @param[in]   pCT         : Pointer to the CT instance register area
 * @param[in]   counterNum  : Counter 0/1
 * @param[in]   mode        : mode 0/1
 * @return    Return the counter value
 */
STATIC INLINE uint32_t RSI_CT_GetCounter(const RSI_CT_T *pCT, boolean_t counterNum, boolean_t mode)
{
  if (mode) {
    return (pCT->CT_COUNTER_REG);
  } else {
    if (counterNum) {
      return (pCT->CT_COUNTER_REG_b.COUNTER1);
    } else {
      return (pCT->CT_COUNTER_REG_b.COUNTER0);
    }
  }
}

/*===================================================*/
/**
 * @fn          void RSI_CT_SetCounerSync( RSI_CT_T *pCT, uint8_t syncCounter,boolean_t counterNum)
 * @brief   Sets the captured counter value. 
 * @param[in]   pCT          : Pointer to the CT instance register area
 * @param[in]   syncCounter  : set the counter number to be in sync
 * @param[in]   counterNum   : Counter 0/1
 * @return    none 
 */
STATIC INLINE void RSI_CT_SetCounerSync(RSI_CT_T *pCT, uint8_t syncCounter, boolean_t counterNum)
{
  if (counterNum) {
    pCT->CT_OCU_CTRL_REG_b.SYNC_WITH_1 = (unsigned int)(syncCounter & 0x07);
  } else {
    pCT->CT_OCU_CTRL_REG_b.SYNC_WITH_0 = (unsigned int)(syncCounter & 0x07);
  }
}

// CT FUNCTION PROTOTYPES
void RSI_CT_Init(void);
void ct_ocu_high_Low_toggle_select(RSI_CT_T *pCT, boolean_t lowHigh, boolean_t counterNum, uint8_t outputSel);

rsi_error_t ct_ocu_control(RSI_CT_T *pCT,
                           boolean_t counterNum,
                           boolean_t dmaEn,
                           OCU_PARAMS_T *pOCUparams,
                           RSI_CT_CALLBACK_T *pCB);

rsi_error_t ct_wfg_control_config(RSI_CT_T *pCT, WFG_PARAMS_T ctrlReg);

rsi_error_t ct_wfg_comapre_value_set(RSI_CT_T *pCT, boolean_t counterNum, OCU_PARAMS_T *pOCUparams);

#ifdef __cplusplus
}
#endif

#endif // RSI_CT_H
