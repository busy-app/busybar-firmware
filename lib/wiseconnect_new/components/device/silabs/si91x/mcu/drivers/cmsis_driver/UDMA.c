/*******************************************************************************
* @file  UDMA.c
* @brief 
*******************************************************************************
* # License
* <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "rsi_ccp_user_config.h"

#include "UDMA.h"
#include "rsi_packing.h"

#ifdef ADC_MULTICHANNEL_WITH_EXT_DMA 
#include "rsi_adc.h"
extern adc_commn_config_t adc_commn_config;
#endif

#ifdef DAC_FIFO_MODE_EN
#include "rsi_dac.h"
extern dac_config_t dac_callback_fun;
#endif

#if defined(A11_ROM)
#include "rsi_rom_table_si91x.h"
#endif

#include "rsi_udma_wrapper.h"

//UDMA Defines////
RSI_UDMA_HANDLE_T udmaHandle0;
RSI_UDMA_HANDLE_T udmaHandle1;
uint32_t dma_rom_buff0[30];
uint32_t dma_rom_buff1[30];

// Signal Event Macro
#define SIGNAL_UDMA_EVENT(channel_info, channel, event) \
  if (channel_info[channel].cb_event != NULL) {         \
    channel_info[channel].cb_event(event, channel);     \
  }

#if  ((UDMA0_SRAM_BASE & (~0x3FF)) != UDMA0_SRAM_BASE)
#error "Invalid UDMA0 sram base address"
#endif

#if  ((UDMA1_SRAM_BASE & (~0x3FF)) != UDMA1_SRAM_BASE)
#error "Invalid UDMA1 sram base address"
#endif

/* IAR support */
#if defined(__ICCARM__)
#pragma location = UDMA0_SRAM_BASE
RSI_UDMA_DESC_T UDMA0_Table[CONTROL_STRUCT0];
#pragma location = UDMA1_SRAM_BASE
RSI_UDMA_DESC_T UDMA1_Table[CONTROL_STRUCT1];
#endif

/* DMA descriptors must be aligned to 16 bytes */
#if defined(__CC_ARM)
RSI_UDMA_DESC_T UDMA0_Table[CONTROL_STRUCT0] __attribute__ ((at(UDMA0_SRAM_BASE)));
RSI_UDMA_DESC_T UDMA1_Table[CONTROL_STRUCT1] __attribute__ ((at(UDMA1_SRAM_BASE)));
#endif /* defined (__CC_ARM) */

#if defined( __GNUC__ )
RSI_UDMA_DESC_T __attribute__ ((section(".udma_addr0"))) UDMA0_Table[CONTROL_STRUCT0];
RSI_UDMA_DESC_T __attribute__ ((section(".udma_addr1"))) UDMA1_Table[CONTROL_STRUCT1];
#endif /* defined (__GNUC__) */

UDMA_Channel_Info udma0_chnl_info[32] = { 0U } ;
UDMA_Channel_Info udma1_chnl_info[12] = { 0U } ;

#ifdef RTE_UDMA0
/* I2C0 Resources */
UDMA_RESOURCES UDMA0_Resources = {
    UDMA0,
    UDMA0_IRQn,
    UDMA0_Table          // SRAM base address
};
#endif /* RTE_UDMA0 */

#ifdef RTE_UDMA1

/* I2C1 Resources */
UDMA_RESOURCES UDMA1_Resources = {
    UDMA1,
    UDMA1_IRQn,
    UDMA1_Table          // SRAM base address
};
#endif /* RTE_UDMA1 */

/*==============================================*/
/**
 * @fn          void uDMAx_IRQHandler(UDMA_RESOURCES *udma, RSI_UDMA_DESC_T *UDMA_Table, UDMA_Channel_Info *chnl_info)
 * @brief       DMA interrupt handler
 * @return      none
 */
void uDMAx_IRQHandler(UDMA_RESOURCES *udma, RSI_UDMA_DESC_T *UDMA_Table, UDMA_Channel_Info *chnl_info)
{
  volatile uint32_t size = 0;
  volatile uint32_t intr = 0; 
  volatile uint32_t src_inc = 0;
  volatile uint32_t dst_inc = 0;
  volatile uint32_t dma_len = 0;

  // error check, invalid instance
  if ((udma->reg != UDMA0) && (udma->reg != UDMA1)) {
    return;
  }

  for (volatile uint32_t ch = 0; ch < UDMA_NUMBER_OF_CHANNELS; ch++) {
    intr = udma->reg->UDMA_DONE_STATUS_REG;
    if (intr & (1U << ch)) {
      // Clear interrupt flag
      udma->reg->UDMA_DONE_STATUS_REG = (1U << ch);      
    } else {
      // DMA error interrupt
      if (udma->reg->ERR_CLR & (1U << ch)) {
        udma->reg->ERR_CLR_b.ERR_CLR = 0x1;
        // Clear interrupt flag
        udma->reg->UDMA_DONE_STATUS_REG = (1U << ch);
        // Signal Error Event
        SIGNAL_UDMA_EVENT(chnl_info, ch, UDMA_EVENT_ERROR);
      }
      // continue check for subsequent channels
      continue;
    }

    // valid interrupt, check if data is waiting to transfer
    if (chnl_info[ch].Cnt != chnl_info[ch].Size) {
      // Data waiting to transfer
      size = chnl_info[ch].Size - chnl_info[ch].Cnt;
      // Max DMA transfer size = 4k
      if (size >= DESC_MAX_LEN) {
        size = DESC_MAX_LEN;
      }
      if (size > 0) {
        dma_len = size - 1;
        chnl_info[ch].Cnt += size;
      }
      // Source Address Increment
      src_inc = UDMA_Table[ch].vsUDMAChaConfigData1.srcInc;
      
      if (src_inc != UDMA_SRC_INC_NONE) {
        UDMA_Table[ch].pSrcEndAddr = (void *)((uint32_t)UDMA_Table[ch].pSrcEndAddr + (size << src_inc));
      }

      // Destination Address Increment
      dst_inc = UDMA_Table[ch].vsUDMAChaConfigData1.dstInc;

      if (dst_inc != UDMA_DST_INC_NONE) {
        UDMA_Table[ch].pDstEndAddr = (void *)((uint32_t)UDMA_Table[ch].pDstEndAddr + (size << dst_inc));
      }

      // Update other DMA parameters
      UDMA_Table[ch].vsUDMAChaConfigData1.totalNumOfDMATrans = (unsigned int)(dma_len & 0x03FF);
      UDMA_Table[ch].vsUDMAChaConfigData1.transferType       = UDMA_MODE_BASIC;

      // Enable DMA Channel
      udma->reg->CHNL_ENABLE_SET = (1U << ch);
    } else {
      // All Data has been transferred
      // Signal Transfer Done Event
      SIGNAL_UDMA_EVENT(chnl_info, ch, UDMA_EVENT_XFER_DONE);
    }
  }
}

void IRQ033_Handler(void)
{
  NVIC_DisableIRQ(UDMA0_IRQn);
 #if defined(A11_ROM) && defined(UDMA_ROMDRIVER_PRESENT)
  uint8_t channel;
  uint32_t int_status;
  uint8_t soft_trig_flag = 0;
  int_status = UDMA0->UDMA_DONE_STATUS_REG;
  //Identify the interrupt channel
  for (channel = 0; channel < UDMA_NUMBER_OF_CHANNELS; channel++) {
    if (int_status & (1U << channel)) {
       break;
    }
  }
  //Check if the transfer type is memory-memory
  if ((UDMA0_Table[channel].vsUDMAChaConfigData1.srcInc != UDMA_SRC_INC_NONE) &&
      (UDMA0_Table[channel].vsUDMAChaConfigData1.dstInc != UDMA_DST_INC_NONE) &&
      (udma0_chnl_info[channel].Size != udma0_chnl_info[channel].Cnt)) {
          soft_trig_flag = 1;
  }
 ROMAPI_UDMA_WRAPPER_API->uDMAx_IRQHandler (&UDMA0_Resources,UDMA0_Table,udma0_chnl_info);
 if(soft_trig_flag) {
   //Set the software trigger bit for starting next transfer
   UDMA0->CHNL_SW_REQUEST |=(1U << channel);
 }
 #else
 uDMAx_IRQHandler (&UDMA0_Resources,UDMA0_Table,udma0_chnl_info);
 #endif
 NVIC_EnableIRQ(UDMA0_IRQn);
}

void IRQ010_Handler (void) 
{
  NVIC_DisableIRQ(UDMA1_IRQn);
#if defined(DAC_FIFO_MODE_EN) || defined(ADC_MULTICHANNEL_WITH_EXT_DMA)
  volatile uint32_t intr = 0;
  intr = UDMA1_Resources.reg->UDMA_DONE_STATUS_REG;

  if((intr & BIT(DAC_UDMA_CHANNEL)) || (intr & BIT(ADC_UDMA_CHANNEL)))
  {
    if(intr & BIT(DAC_UDMA_CHANNEL))
    {
#ifdef DAC_FIFO_MODE_EN
      RSI_UDMA_InterruptClear(udmaHandle1, DAC_UDMA_CHANNEL);
      dac_callback_fun.callback_event(DAC_UDMA_PING_PONG_CONFIG);
#endif
    }
    else
    {
#ifdef ADC_MULTICHANNEL_WITH_EXT_DMA
      RSI_UDMA_InterruptClear(udmaHandle1, ADC_UDMA_CHANNEL);
      adc_commn_config.call_back_event(ADC_CHNL0_INTR , EXTERNAL_DMA_RECONFIG);
#endif
    }
    NVIC_EnableIRQ(UDMA1_IRQn);
    return;
  }
#endif
#if defined(A11_ROM) && defined(UDMA_ROMDRIVER_PRESENT)
  uint8_t channel;
  uint32_t int_status;
  uint8_t soft_trig_flag = 0;
  int_status = UDMA1->UDMA_DONE_STATUS_REG;
  //Identify the interrupt channel
  for (channel = 0; channel < 12; channel++) {
    if (int_status & (1U << channel)) {
       break;
    }
  }
  //Check if the transfer type is memory-memory
  if ((UDMA1_Table[channel].vsUDMAChaConfigData1.srcInc != UDMA_SRC_INC_NONE) &&
      (UDMA1_Table[channel].vsUDMAChaConfigData1.dstInc != UDMA_DST_INC_NONE) &&
      (udma1_chnl_info[channel].Size != udma1_chnl_info[channel].Cnt)) {
          soft_trig_flag = 1;
  }
  ROMAPI_UDMA_WRAPPER_API->uDMAx_IRQHandler (&UDMA1_Resources,UDMA1_Table,udma1_chnl_info);
  if(soft_trig_flag) {
	//Set the software trigger bit for starting next transfer
    UDMA1->CHNL_SW_REQUEST |=(1U << channel);
  }
#else
  uDMAx_IRQHandler (&UDMA1_Resources,UDMA1_Table,udma1_chnl_info);
#endif
  NVIC_EnableIRQ(UDMA1_IRQn);
}