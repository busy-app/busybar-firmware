/**************************************************************************//**
 * @file
 * @brief Device Manager DMA API Definition
 ******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories, Inc. www.silabs.com</b>
 ******************************************************************************
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
 *****************************************************************************/

#include "sl_device_dma.h"
#include "cmsis_compiler.h"

/***************************************************************************//**
 * @addtogroup device_dma Device Manager DMA
 * @{
 ******************************************************************************/

// Weak declaration for LDMAXBAR0_PRSREQ0 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_LDMAXBAR0_PRSREQ0_VALUE = 0xFFFFFFFF;
// Weak declaration for LDMAXBAR0_PRSREQ1 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_LDMAXBAR0_PRSREQ1_VALUE = 0xFFFFFFFF;

// Weak declaration for TIMER0 CC0 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER0_CC0_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER0 CC1 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER0_CC1_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER0 CC2 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER0_CC2_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER0 UFOF DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER0_UFOF_VALUE = 0xFFFFFFFF;

// Weak declaration for TIMER1 CC0 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER1_CC0_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER1 CC1 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER1_CC1_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER1 CC2 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER1_CC2_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER1 UFOF DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER1_UFOF_VALUE = 0xFFFFFFFF;

// Weak declaration for TIMER2 CC0 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER2_CC0_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER2 CC1 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER2_CC1_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER2 CC2 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER2_CC2_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER2 UFOF DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER2_UFOF_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER2 CC3 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER2_CC3_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER2 CC4 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER2_CC4_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER2 CC5 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER2_CC5_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER2 CC6 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER2_CC6_VALUE = 0xFFFFFFFF;

// Weak declaration for TIMER3 CC0 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER3_CC0_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER3 CC1 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER3_CC1_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER3 CC2 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER3_CC2_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER3 UFOF DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER3_UFOF_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER3 CC3 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER3_CC3_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER3 CC4 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER3_CC4_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER3 CC5 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER3_CC5_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER3 CC6 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER3_CC6_VALUE = 0xFFFFFFFF;

// Weak declaration for TIMER4 CC0 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER4_CC0_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER4 CC1 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER4_CC1_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER4 CC2 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER4_CC2_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER4 UFOF DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER4_UFOF_VALUE = 0xFFFFFFFF;

// Weak declaration for TIMER5 CC0 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER5_CC0_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER5 CC1 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER5_CC1_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER5 CC2 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER5_CC2_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER5 UFOF DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER5_UFOF_VALUE = 0xFFFFFFFF;

// Weak declaration for TIMER6 CC0 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER6_CC0_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER6 CC1 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER6_CC1_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER6 CC2 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER6_CC2_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER6 UFOF DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER6_UFOF_VALUE = 0xFFFFFFFF;

// Weak declaration for TIMER7 CC0 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER7_CC0_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER7 CC1 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER7_CC1_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER7 CC2 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER7_CC2_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER7 UFOF DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER7_UFOF_VALUE = 0xFFFFFFFF;

// Weak declaration for TIMER8 CC0 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER8_CC0_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER8 CC1 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER8_CC1_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER8 CC2 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER8_CC2_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER8 UFOF DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER8_UFOF_VALUE = 0xFFFFFFFF;

// Weak declaration for TIMER9 CC0 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER9_CC0_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER9 CC1 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER9_CC1_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER9 CC2 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER9_CC2_VALUE = 0xFFFFFFFF;
// Weak declaration for TIMER9 UFOF DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_TIMER9_UFOF_VALUE = 0xFFFFFFFF;

// Weak declaration for I2C0 RXDATAV DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_I2C0_RXDATAV_VALUE = 0xFFFFFFFF;
// Weak declaration for I2C0 TXBL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_I2C0_TXBL_VALUE = 0xFFFFFFFF;

// Weak declaration for I2C1 RXDATAV DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_I2C1_RXDATAV_VALUE = 0xFFFFFFFF;
// Weak declaration for I2C1 TXBL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_I2C1_TXBL_VALUE = 0xFFFFFFFF;

// Weak declaration for I2C2 RXDATAV DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_I2C2_RXDATAV_VALUE = 0xFFFFFFFF;
// Weak declaration for I2C2 TXBL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_I2C2_TXBL_VALUE = 0xFFFFFFFF;

// Weak declaration for I2C3 RXDATAV DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_I2C3_RXDATAV_VALUE = 0xFFFFFFFF;
// Weak declaration for I2C3 TXBL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_I2C3_TXBL_VALUE = 0xFFFFFFFF;

// Weak declaration for PROTIMER BOF DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_PROTIMER_BOF_VALUE = 0xFFFFFFFF;
// Weak declaration for PROTIMER CC0 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_PROTIMER_CC0_VALUE = 0xFFFFFFFF;
// Weak declaration for PROTIMER CC1 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_PROTIMER_CC1_VALUE = 0xFFFFFFFF;
// Weak declaration for PROTIMER CC2 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_PROTIMER_CC2_VALUE = 0xFFFFFFFF;
// Weak declaration for PROTIMER CC3 DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_PROTIMER_CC3_VALUE = 0xFFFFFFFF;
// Weak declaration for PROTIMER POF DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_PROTIMER_POF_VALUE = 0xFFFFFFFF;
// Weak declaration for PROTIMER WOF DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_PROTIMER_WOF_VALUE = 0xFFFFFFFF;

// Weak declaration for ADC0 SCAN DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_ADC0_SCAN_VALUE = 0xFFFFFFFF;

// Weak declaration for ADC1 SCAN DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_ADC1_SCAN_VALUE = 0xFFFFFFFF;

// Weak declaration for IADC0 SCAN DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_IADC0_SCAN_VALUE = 0xFFFFFFFF;
// Weak declaration for IADC0 SINGLE DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_IADC0_SINGLE_VALUE = 0xFFFFFFFF;

// Weak declaration for EUSART0 RXFL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_EUSART0_RXFL_VALUE = 0xFFFFFFFF;
// Weak declaration for EUSART0 TXFL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_EUSART0_TXFL_VALUE = 0xFFFFFFFF;

// Weak declaration for EUSART1 RXFL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_EUSART1_RXFL_VALUE = 0xFFFFFFFF;
// Weak declaration for EUSART1 TXFL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_EUSART1_TXFL_VALUE = 0xFFFFFFFF;

// Weak declaration for EUSART2 RXFL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_EUSART2_RXFL_VALUE = 0xFFFFFFFF;
// Weak declaration for EUSART2 TXFL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_EUSART2_TXFL_VALUE = 0xFFFFFFFF;

// Weak declaration for EUSART3 RXFL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_EUSART3_RXFL_VALUE = 0xFFFFFFFF;
// Weak declaration for EUSART3 TXFL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_EUSART3_TXFL_VALUE = 0xFFFFFFFF;

// Weak declaration for EUSART4 RXFL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_EUSART4_RXFL_VALUE = 0xFFFFFFFF;
// Weak declaration for EUSART4 TXFL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_EUSART4_TXFL_VALUE = 0xFFFFFFFF;

// Weak declaration for EUART0 RXFL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_EUART0_RXFL_VALUE = 0xFFFFFFFF;
// Weak declaration for EUART0 TXFL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_EUART0_TXFL_VALUE = 0xFFFFFFFF;

// Weak declaration for USART0 RXDATAV DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_USART0_RXDATAV_VALUE = 0xFFFFFFFF;
// Weak declaration for USART0 RXDATAVRIGHT DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_USART0_RXDATAVRIGHT_VALUE = 0xFFFFFFFF;
// Weak declaration for USART0 TXBL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_USART0_TXBL_VALUE = 0xFFFFFFFF;
// Weak declaration for USART0 TXBLRIGHT DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_USART0_TXBLRIGHT_VALUE = 0xFFFFFFFF;
// Weak declaration for USART0 TXEMPTY DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_USART0_TXEMPTY_VALUE = 0xFFFFFFFF;

// Weak declaration for USART1 RXDATAV DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_USART1_RXDATAV_VALUE = 0xFFFFFFFF;
// Weak declaration for USART1 RXDATAVRIGHT DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_USART1_RXDATAVRIGHT_VALUE = 0xFFFFFFFF;
// Weak declaration for USART1 TXBL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_USART1_TXBL_VALUE = 0xFFFFFFFF;
// Weak declaration for USART1 TXBLRIGHT DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_USART1_TXBLRIGHT_VALUE = 0xFFFFFFFF;
// Weak declaration for USART1 TXEMPTY DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_USART1_TXEMPTY_VALUE = 0xFFFFFFFF;

// Weak declaration for USART2 RXDATAV DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_USART2_RXDATAV_VALUE = 0xFFFFFFFF;
// Weak declaration for USART2 RXDATAVRIGHT DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_USART2_RXDATAVRIGHT_VALUE = 0xFFFFFFFF;
// Weak declaration for USART2 TXBL DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_USART2_TXBL_VALUE = 0xFFFFFFFF;
// Weak declaration for USART2 TXBLRIGHT DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_USART2_TXBLRIGHT_VALUE = 0xFFFFFFFF;
// Weak declaration for USART2 TXEMPTY DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_USART2_TXEMPTY_VALUE = 0xFFFFFFFF;

// Weak declaration for MSC WDATA DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_MSC_WDATA_VALUE = 0xFFFFFFFF;

// Weak declaration for MVP REQ DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_MVP_REQ_VALUE = 0xFFFFFFFF;

// Weak declaration for PDM RXDATAV DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_PDM_RXDATAV_VALUE = 0xFFFFFFFF;

// Weak declaration for PDM0 RXDATA DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_PDM0_RXDATA_VALUE = 0xFFFFFFFF;

// Weak declaration for LCD DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_LCD_VALUE = 0xFFFFFFFF;

// Weak declaration for LESENSE FIFO DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_LESENSE_FIFO_VALUE = 0xFFFFFFFF;

// Weak declaration for VDAC0 CH0 REQ DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_VDAC0_CH0REQ_VALUE = 0xFFFFFFFF;
// Weak declaration for VDAC0 CH1 REQ DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_VDAC0_CH1REQ_VALUE = 0xFFFFFFFF;

// Weak declaration for VDAC1 CH0 REQ DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_VDAC1_CH0REQ_VALUE = 0xFFFFFFFF;
// Weak declaration for VDAC1 CH1 REQ DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_VDAC1_CH1REQ_VALUE = 0xFFFFFFFF;

// Weak declaration for PIXELRZ0REQ TXF DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_PIXELRZ0REQ_TXF_VALUE = 0xFFFFFFFF;

// Weak declaration for PIXELRZ1REQ TXF DMA Signal
__WEAK const uint32_t SL_DMA_SIGNAL_PIXELRZ1REQ_TXF_VALUE = 0xFFFFFFFF;

/** @} (end addtogroup device_dma) */
