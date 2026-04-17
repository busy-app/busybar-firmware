/***************************************************************************//**
 * @file
 * @brief Device Manager DMA (device independent enumerations)
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories, Inc. www.silabs.com</b>
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
 ******************************************************************************/

#ifndef SL_DEVICE_DMA_H
#define SL_DEVICE_DMA_H

#include <stdint.h>
#include <stdbool.h>

#include "sl_enum.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup device_dma Device Manager DMA
 * @brief Portable (device independent) DMA configuration enums.
 * @details
 * ## Overview
 * This header provides abstract, device-agnostic enumerations for common
 * DMA configuration parameters. They intentionally avoid binding their
 * enumerator values to any specific device register encodings. A mapping
 * layer (typically within the HAL or driver implementation) is expected to
 * translate these portable values to device-specific register fields.
 *
 * The goal is to allow higher-level components, code generators and
 * configuration tooling to reference DMA attributes without pulling in
 * device headers or conditional compilation based on device series.
 *
 * The enums defined here SHOULD NOT be assumed to match any hardware
 * bit patterns. Only their relative meaning is guaranteed.
 *
 * @{
 ******************************************************************************/

// -----------------------------------------------------------------------------
// ENUMS

/// DMA transfer unit size (portable semantic values)
SL_ENUM(sl_dma_ctrl_size_t) {
  SL_DMA_CTRL_SIZE_BYTE = 0,  ///< Unit transfer size is 1 byte.
  SL_DMA_CTRL_SIZE_HALF,      ///< Unit transfer size is 2 bytes (half-word).
  SL_DMA_CTRL_SIZE_WORD       ///< Unit transfer size is 4 bytes (word).
};

/// Number of unit transfers granted per arbitration opportunity.
/// Values are semantic ordering indices; NOT literal hardware encodings.
SL_ENUM(sl_dma_ctrl_block_size_t) {
  SL_DMA_CTRL_BLOCK_SIZE_UNIT_1 = 0,   ///< One unit transfer per arbitration.
  SL_DMA_CTRL_BLOCK_SIZE_UNIT_2,       ///< Two unit transfers per arbitration.
  SL_DMA_CTRL_BLOCK_SIZE_UNIT_3,       ///< Three unit transfers per arbitration.
  SL_DMA_CTRL_BLOCK_SIZE_UNIT_4,       ///< Four unit transfers per arbitration.
  SL_DMA_CTRL_BLOCK_SIZE_UNIT_6,       ///< Six unit transfers per arbitration.
  SL_DMA_CTRL_BLOCK_SIZE_UNIT_8,       ///< Eight unit transfers per arbitration.
  SL_DMA_CTRL_BLOCK_SIZE_UNIT_16,      ///< 16 unit transfers per arbitration.
  SL_DMA_CTRL_BLOCK_SIZE_UNIT_32,      ///< 32 unit transfers per arbitration.
  SL_DMA_CTRL_BLOCK_SIZE_UNIT_64,      ///< 64 unit transfers per arbitration.
  SL_DMA_CTRL_BLOCK_SIZE_UNIT_128,     ///< 128 unit transfers per arbitration.
  SL_DMA_CTRL_BLOCK_SIZE_UNIT_256,     ///< 256 unit transfers per arbitration.
  SL_DMA_CTRL_BLOCK_SIZE_UNIT_512,     ///< 512 unit transfers per arbitration.
  SL_DMA_CTRL_BLOCK_SIZE_UNIT_1024,    ///< 1024 unit transfers per arbitration.
  SL_DMA_CTRL_BLOCK_SIZE_ALL           ///< Entire remaining transfer (locks arbitration) if supported.
};

// -----------------------------------------------------------------------------
// VALIDATION MACROS

/// Validate transfer unit size value.
#define SL_DMA_CTRL_SIZE_IS_VALID(v) \
  ((v) == SL_DMA_CTRL_SIZE_BYTE || (v) == SL_DMA_CTRL_SIZE_HALF || (v) == SL_DMA_CTRL_SIZE_WORD)

/// Validate block size enum value.
#define SL_DMA_CTRL_BLOCK_SIZE_IS_VALID(v) \
  ((v) >= SL_DMA_CTRL_BLOCK_SIZE_UNIT_1 && (v) <= SL_DMA_CTRL_BLOCK_SIZE_ALL)

/// Validate peripheral signal selection (excluding COUNT sentinel).
#define SL_DMA_PERIPHERAL_SIGNAL_IS_VALID(v) \
  ((v) > SL_DMA_PERIPHERAL_SIGNAL_NONE && (v) < SL_DMA_PERIPHERAL_SIGNAL_COUNT)

/***************************************************************************//**
 * @name DMA Signal Defines
 * Those defines can be used as constant of type sl_dma_signal_t
 * The values of those defines are device specific.
 * @{
 ******************************************************************************/
/// Define for No Signal.
#define SL_DMA_SIGNAL_NONE (0)

/// Define for LDMAXBAR0_PRSREQ0 DMA Signal.
#define SL_DMA_SIGNAL_LDMAXBAR0_PRSREQ0 (&SL_DMA_SIGNAL_LDMAXBAR0_PRSREQ0_VALUE)
/// Define for LDMAXBAR0_PRSREQ1 DMA Signal.
#define SL_DMA_SIGNAL_LDMAXBAR0_PRSREQ1 (&SL_DMA_SIGNAL_LDMAXBAR0_PRSREQ1_VALUE)

/// Define for TIMER0 CC0 DMA Signal.
#define SL_DMA_SIGNAL_TIMER0_CC0 (&SL_DMA_SIGNAL_TIMER0_CC0_VALUE)
/// Define for TIMER0 CC1 DMA Signal.
#define SL_DMA_SIGNAL_TIMER0_CC1 (&SL_DMA_SIGNAL_TIMER0_CC1_VALUE)
/// Define for TIMER0 CC2 DMA Signal.
#define SL_DMA_SIGNAL_TIMER0_CC2 (&SL_DMA_SIGNAL_TIMER0_CC2_VALUE)
/// Define for TIMER0 UFOF DMA Signal.
#define SL_DMA_SIGNAL_TIMER0_UFOF (&SL_DMA_SIGNAL_TIMER0_UFOF_VALUE)

/// Define for TIMER1 CC0 DMA Signal.
#define SL_DMA_SIGNAL_TIMER1_CC0 (&SL_DMA_SIGNAL_TIMER1_CC0_VALUE)
/// Define for TIMER1 CC1 DMA Signal.
#define SL_DMA_SIGNAL_TIMER1_CC1 (&SL_DMA_SIGNAL_TIMER1_CC1_VALUE)
/// Define for TIMER1 CC2 DMA Signal.
#define SL_DMA_SIGNAL_TIMER1_CC2 (&SL_DMA_SIGNAL_TIMER1_CC2_VALUE)
/// Define for TIMER1 UFOF DMA Signal.
#define SL_DMA_SIGNAL_TIMER1_UFOF (&SL_DMA_SIGNAL_TIMER1_UFOF_VALUE)

/// Define for TIMER2 CC0 DMA Signal.
#define SL_DMA_SIGNAL_TIMER2_CC0 (&SL_DMA_SIGNAL_TIMER2_CC0_VALUE)
/// Define for TIMER2 CC1 DMA Signal.
#define SL_DMA_SIGNAL_TIMER2_CC1 (&SL_DMA_SIGNAL_TIMER2_CC1_VALUE)
/// Define for TIMER2 CC2 DMA Signal.
#define SL_DMA_SIGNAL_TIMER2_CC2 (&SL_DMA_SIGNAL_TIMER2_CC2_VALUE)
/// Define for TIMER2 UFOF DMA Signal.
#define SL_DMA_SIGNAL_TIMER2_UFOF (&SL_DMA_SIGNAL_TIMER2_UFOF_VALUE)
/// Define for TIMER2 CC3 DMA Signal.
#define SL_DMA_SIGNAL_TIMER2_CC3 (&SL_DMA_SIGNAL_TIMER2_CC3_VALUE)
/// Define for TIMER2 CC4 DMA Signal.
#define SL_DMA_SIGNAL_TIMER2_CC4 (&SL_DMA_SIGNAL_TIMER2_CC4_VALUE)
/// Define for TIMER2 CC5 DMA Signal.
#define SL_DMA_SIGNAL_TIMER2_CC5 (&SL_DMA_SIGNAL_TIMER2_CC5_VALUE)
/// Define for TIMER2 CC6 DMA Signal.
#define SL_DMA_SIGNAL_TIMER2_CC6 (&SL_DMA_SIGNAL_TIMER2_CC6_VALUE)

/// Define for TIMER3 CC0 DMA Signal.
#define SL_DMA_SIGNAL_TIMER3_CC0 (&SL_DMA_SIGNAL_TIMER3_CC0_VALUE)
/// Define for TIMER3 CC1 DMA Signal.
#define SL_DMA_SIGNAL_TIMER3_CC1 (&SL_DMA_SIGNAL_TIMER3_CC1_VALUE)
/// Define for TIMER3 CC2 DMA Signal.
#define SL_DMA_SIGNAL_TIMER3_CC2 (&SL_DMA_SIGNAL_TIMER3_CC2_VALUE)
/// Define for TIMER3 UFOF DMA Signal.
#define SL_DMA_SIGNAL_TIMER3_UFOF (&SL_DMA_SIGNAL_TIMER3_UFOF_VALUE)
/// Define for TIMER3 CC3 DMA Signal.
#define SL_DMA_SIGNAL_TIMER3_CC3 (&SL_DMA_SIGNAL_TIMER3_CC3_VALUE)
/// Define for TIMER3 CC4 DMA Signal.
#define SL_DMA_SIGNAL_TIMER3_CC4 (&SL_DMA_SIGNAL_TIMER3_CC4_VALUE)
/// Define for TIMER3 CC5 DMA Signal.
#define SL_DMA_SIGNAL_TIMER3_CC5 (&SL_DMA_SIGNAL_TIMER3_CC5_VALUE)
/// Define for TIMER3 CC6 DMA Signal.
#define SL_DMA_SIGNAL_TIMER3_CC6 (&SL_DMA_SIGNAL_TIMER3_CC6_VALUE)

/// Define for TIMER4 CC0 DMA Signal.
#define SL_DMA_SIGNAL_TIMER4_CC0 (&SL_DMA_SIGNAL_TIMER4_CC0_VALUE)
/// Define for TIMER4 CC1 DMA Signal.
#define SL_DMA_SIGNAL_TIMER4_CC1 (&SL_DMA_SIGNAL_TIMER4_CC1_VALUE)
/// Define for TIMER4 CC2 DMA Signal.
#define SL_DMA_SIGNAL_TIMER4_CC2 (&SL_DMA_SIGNAL_TIMER4_CC2_VALUE)
/// Define for TIMER4 UFOF DMA Signal.
#define SL_DMA_SIGNAL_TIMER4_UFOF (&SL_DMA_SIGNAL_TIMER4_UFOF_VALUE)

/// Define for TIMER5 CC0 DMA Signal.
#define SL_DMA_SIGNAL_TIMER5_CC0 (&SL_DMA_SIGNAL_TIMER5_CC0_VALUE)
/// Define for TIMER5 CC1 DMA Signal.
#define SL_DMA_SIGNAL_TIMER5_CC1 (&SL_DMA_SIGNAL_TIMER5_CC1_VALUE)
/// Define for TIMER5 CC2 DMA Signal.
#define SL_DMA_SIGNAL_TIMER5_CC2 (&SL_DMA_SIGNAL_TIMER5_CC2_VALUE)
/// Define for TIMER5 UFOF DMA Signal.
#define SL_DMA_SIGNAL_TIMER5_UFOF (&SL_DMA_SIGNAL_TIMER5_UFOF_VALUE)

/// Define for TIMER6 CC0 DMA Signal.
#define SL_DMA_SIGNAL_TIMER6_CC0 (&SL_DMA_SIGNAL_TIMER6_CC0_VALUE)
/// Define for TIMER6 CC1 DMA Signal.
#define SL_DMA_SIGNAL_TIMER6_CC1 (&SL_DMA_SIGNAL_TIMER6_CC1_VALUE)
/// Define for TIMER6 CC2 DMA Signal.
#define SL_DMA_SIGNAL_TIMER6_CC2 (&SL_DMA_SIGNAL_TIMER6_CC2_VALUE)
/// Define for TIMER6 UFOF DMA Signal.
#define SL_DMA_SIGNAL_TIMER6_UFOF (&SL_DMA_SIGNAL_TIMER6_UFOF_VALUE)

/// Define for TIMER7 CC0 DMA Signal.
#define SL_DMA_SIGNAL_TIMER7_CC0 (&SL_DMA_SIGNAL_TIMER7_CC0_VALUE)
/// Define for TIMER7 CC1 DMA Signal.
#define SL_DMA_SIGNAL_TIMER7_CC1 (&SL_DMA_SIGNAL_TIMER7_CC1_VALUE)
/// Define for TIMER7 CC2 DMA Signal.
#define SL_DMA_SIGNAL_TIMER7_CC2 (&SL_DMA_SIGNAL_TIMER7_CC2_VALUE)
/// Define for TIMER7 UFOF DMA Signal.
#define SL_DMA_SIGNAL_TIMER7_UFOF (&SL_DMA_SIGNAL_TIMER7_UFOF_VALUE)

/// Define for TIMER8 CC0 DMA Signal.
#define SL_DMA_SIGNAL_TIMER8_CC0 (&SL_DMA_SIGNAL_TIMER8_CC0_VALUE)
/// Define for TIMER8 CC1 DMA Signal.
#define SL_DMA_SIGNAL_TIMER8_CC1 (&SL_DMA_SIGNAL_TIMER8_CC1_VALUE)
/// Define for TIMER8 CC2 DMA Signal.
#define SL_DMA_SIGNAL_TIMER8_CC2 (&SL_DMA_SIGNAL_TIMER8_CC2_VALUE)
/// Define for TIMER8 UFOF DMA Signal.
#define SL_DMA_SIGNAL_TIMER8_UFOF (&SL_DMA_SIGNAL_TIMER8_UFOF_VALUE)

/// Define for TIMER9 CC0 DMA Signal.
#define SL_DMA_SIGNAL_TIMER9_CC0 (&SL_DMA_SIGNAL_TIMER9_CC0_VALUE)
/// Define for TIMER9 CC1 DMA Signal.
#define SL_DMA_SIGNAL_TIMER9_CC1 (&SL_DMA_SIGNAL_TIMER9_CC1_VALUE)
/// Define for TIMER9 CC2 DMA Signal.
#define SL_DMA_SIGNAL_TIMER9_CC2 (&SL_DMA_SIGNAL_TIMER9_CC2_VALUE)
/// Define for TIMER9 UFOF DMA Signal.
#define SL_DMA_SIGNAL_TIMER9_UFOF (&SL_DMA_SIGNAL_TIMER9_UFOF_VALUE)

/// Define for PROTIMER BOF DMA Signal.
#define SL_DMA_SIGNAL_PROTIMER_BOF (&SL_DMA_SIGNAL_PROTIMER_BOF_VALUE)
/// Define for PROTIMER CC0 DMA Signal.
#define SL_DMA_SIGNAL_PROTIMER_CC0 (&SL_DMA_SIGNAL_PROTIMER_CC0_VALUE)
/// Define for PROTIMER CC1 DMA Signal.
#define SL_DMA_SIGNAL_PROTIMER_CC1 (&SL_DMA_SIGNAL_PROTIMER_CC1_VALUE)
/// Define for PROTIMER CC2 DMA Signal.
#define SL_DMA_SIGNAL_PROTIMER_CC2 (&SL_DMA_SIGNAL_PROTIMER_CC2_VALUE)
/// Define for PROTIMER CC3 DMA Signal.
#define SL_DMA_SIGNAL_PROTIMER_CC3 (&SL_DMA_SIGNAL_PROTIMER_CC3_VALUE)
/// Define for PROTIMER POF DMA Signal.
#define SL_DMA_SIGNAL_PROTIMER_POF (&SL_DMA_SIGNAL_PROTIMER_POF_VALUE)
/// Define for PROTIMER WOF DMA Signal.
#define SL_DMA_SIGNAL_PROTIMER_WOF (&SL_DMA_SIGNAL_PROTIMER_WOF_VALUE)

/// Define for I2C0 RXDATAV DMA Signal.
#define SL_DMA_SIGNAL_I2C0_RXDATAV (&SL_DMA_SIGNAL_I2C0_RXDATAV_VALUE)
/// Define for I2C0 TXBL DMA Signal.
#define SL_DMA_SIGNAL_I2C0_TXBL (&SL_DMA_SIGNAL_I2C0_TXBL_VALUE)

/// Define for I2C1 RXDATAV DMA Signal.
#define SL_DMA_SIGNAL_I2C1_RXDATAV (&SL_DMA_SIGNAL_I2C1_RXDATAV_VALUE)
/// Define for I2C1 TXBL DMA Signal.
#define SL_DMA_SIGNAL_I2C1_TXBL (&SL_DMA_SIGNAL_I2C1_TXBL_VALUE)

/// Define for I2C2 RXDATAV DMA Signal.
#define SL_DMA_SIGNAL_I2C2_RXDATAV (&SL_DMA_SIGNAL_I2C2_RXDATAV_VALUE)
/// Define for I2C2 TXBL DMA Signal.
#define SL_DMA_SIGNAL_I2C2_TXBL (&SL_DMA_SIGNAL_I2C2_TXBL_VALUE)

/// Define for I2C3 RXDATAV DMA Signal.
#define SL_DMA_SIGNAL_I2C3_RXDATAV (&SL_DMA_SIGNAL_I2C3_RXDATAV_VALUE)
/// Define for I2C3 TXBL DMA Signal.
#define SL_DMA_SIGNAL_I2C3_TXBL (&SL_DMA_SIGNAL_I2C3_TXBL_VALUE)

/// Define for ADC0 SCAN DMA Signal.
#define SL_DMA_SIGNAL_ADC0_SCAN (&SL_DMA_SIGNAL_ADC0_SCAN_VALUE)

/// Define for ADC1 SCAN DMA Signal.
#define SL_DMA_SIGNAL_ADC1_SCAN (&SL_DMA_SIGNAL_ADC1_SCAN_VALUE)

/// Define for IADC0 SCAN DMA Signal.
#define SL_DMA_SIGNAL_IADC0_SCAN (&SL_DMA_SIGNAL_IADC0_SCAN_VALUE)
/// Define for IADC0 SINGLE DMA Signal.
#define SL_DMA_SIGNAL_IADC0_SINGLE (&SL_DMA_SIGNAL_IADC0_SINGLE_VALUE)

/// Define for EUSART0 RXFL DMA Signal.
#define SL_DMA_SIGNAL_EUSART0_RXFL (&SL_DMA_SIGNAL_EUSART0_RXFL_VALUE)
/// Define for EUSART0 TXFL DMA Signal.
#define SL_DMA_SIGNAL_EUSART0_TXFL (&SL_DMA_SIGNAL_EUSART0_TXFL_VALUE)

/// Define for EUSART1 RXFL DMA Signal.
#define SL_DMA_SIGNAL_EUSART1_RXFL (&SL_DMA_SIGNAL_EUSART1_RXFL_VALUE)
/// Define for EUSART1 TXFL DMA Signal.
#define SL_DMA_SIGNAL_EUSART1_TXFL (&SL_DMA_SIGNAL_EUSART1_TXFL_VALUE)

/// Define for EUSART2 RXFL DMA Signal.
#define SL_DMA_SIGNAL_EUSART2_RXFL (&SL_DMA_SIGNAL_EUSART2_RXFL_VALUE)
/// Define for EUSART2 TXFL DMA Signal.
#define SL_DMA_SIGNAL_EUSART2_TXFL (&SL_DMA_SIGNAL_EUSART2_TXFL_VALUE)

/// Define for EUSART3 RXFL DMA Signal.
#define SL_DMA_SIGNAL_EUSART3_RXFL (&SL_DMA_SIGNAL_EUSART3_RXFL_VALUE)
/// Define for EUSART3 TXFL DMA Signal.
#define SL_DMA_SIGNAL_EUSART3_TXFL (&SL_DMA_SIGNAL_EUSART3_TXFL_VALUE)

/// Define for EUSART4 RXFL DMA Signal.
#define SL_DMA_SIGNAL_EUSART4_RXFL (&SL_DMA_SIGNAL_EUSART4_RXFL_VALUE)
/// Define for EUSART4 TXFL DMA Signal.
#define SL_DMA_SIGNAL_EUSART4_TXFL (&SL_DMA_SIGNAL_EUSART4_TXFL_VALUE)

/// Define for USART0 RXDATAV DMA Signal.
#define SL_DMA_SIGNAL_USART0_RXDATAV (&SL_DMA_SIGNAL_USART0_RXDATAV_VALUE)
/// Define for USART0 RXDATAVRIGHT DMA Signal.
#define SL_DMA_SIGNAL_USART0_RXDATAVRIGHT (&SL_DMA_SIGNAL_USART0_RXDATAVRIGHT_VALUE)
/// Define for USART0 TXBL DMA Signal.
#define SL_DMA_SIGNAL_USART0_TXBL (&SL_DMA_SIGNAL_USART0_TXBL_VALUE)
/// Define for USART0 TXBLRIGHT DMA Signal.
#define SL_DMA_SIGNAL_USART0_TXBLRIGHT (&SL_DMA_SIGNAL_USART0_TXBLRIGHT_VALUE)
/// Define for USART0 TXEMPTY DMA Signal.
#define SL_DMA_SIGNAL_USART0_TXEMPTY (&SL_DMA_SIGNAL_USART0_TXEMPTY_VALUE)

/// Define for USART1 RXDATAV DMA Signal.
#define SL_DMA_SIGNAL_USART1_RXDATAV (&SL_DMA_SIGNAL_USART1_RXDATAV_VALUE)
/// Define for USART1 RXDATAVRIGHT DMA Signal.
#define SL_DMA_SIGNAL_USART1_RXDATAVRIGHT (&SL_DMA_SIGNAL_USART1_RXDATAVRIGHT_VALUE)
/// Define for USART1 TXBL DMA Signal.
#define SL_DMA_SIGNAL_USART1_TXBL (&SL_DMA_SIGNAL_USART1_TXBL_VALUE)
/// Define for USART1 TXBLRIGHT DMA Signal.
#define SL_DMA_SIGNAL_USART1_TXBLRIGHT (&SL_DMA_SIGNAL_USART1_TXBLRIGHT_VALUE)
/// Define for USART1 TXEMPTY DMA Signal.
#define SL_DMA_SIGNAL_USART1_TXEMPTY (&SL_DMA_SIGNAL_USART1_TXEMPTY_VALUE)

/// Define for USART2 RXDATAV DMA Signal.
#define SL_DMA_SIGNAL_USART2_RXDATAV (&SL_DMA_SIGNAL_USART2_RXDATAV_VALUE)
/// Define for USART2 RXDATAVRIGHT DMA Signal.
#define SL_DMA_SIGNAL_USART2_RXDATAVRIGHT (&SL_DMA_SIGNAL_USART2_RXDATAVRIGHT_VALUE)
/// Define for USART2 TXBL DMA Signal.
#define SL_DMA_SIGNAL_USART2_TXBL (&SL_DMA_SIGNAL_USART2_TXBL_VALUE)
/// Define for USART2 TXBLRIGHT DMA Signal.
#define SL_DMA_SIGNAL_USART2_TXBLRIGHT (&SL_DMA_SIGNAL_USART2_TXBLRIGHT_VALUE)
/// Define for USART2 TXEMPTY DMA Signal.
#define SL_DMA_SIGNAL_USART2_TXEMPTY (&SL_DMA_SIGNAL_USART2_TXEMPTY_VALUE)

/// Define for EUART0 RXFL DMA Signal.
#define SL_DMA_SIGNAL_EUART0_RXFL (&SL_DMA_SIGNAL_EUART0_RXFL_VALUE)
/// Define for EUART0 TXFL DMA Signal.
#define SL_DMA_SIGNAL_EUART0_TXFL (&SL_DMA_SIGNAL_EUART0_TXFL_VALUE)

/// Define for MSC WDATA DMA Signal.
#define SL_DMA_SIGNAL_MSC_WDATA (&SL_DMA_SIGNAL_MSC_WDATA_VALUE)

/// Define for MVP REQ DMA Signal.
#define SL_DMA_SIGNAL_MVP_REQ (&SL_DMA_SIGNAL_MVP_REQ_VALUE)

/// Define for PIXELRZ0REQ TXF DMA Signal.
#define SL_DMA_SIGNAL_PIXELRZ0REQ_TXF (&SL_DMA_SIGNAL_PIXELRZ0REQ_TXF_VALUE)

/// Define for PIXELRZ1REQ TXF DMA Signal.
#define SL_DMA_SIGNAL_PIXELRZ1REQ_TXF (&SL_DMA_SIGNAL_PIXELRZ1REQ_TXF_VALUE)

/// Define for PDM RXDATAV DMA Signal.
#define SL_DMA_SIGNAL_PDM_RXDATAV (&SL_DMA_SIGNAL_PDM_RXDATAV_VALUE)

/// Define for PDM0 RXDATA DMA Signal.
#define SL_DMA_SIGNAL_PDM0_RXDATA (&SL_DMA_SIGNAL_PDM0_RXDATA_VALUE)

/// Define for LCD DMA Signal.
#define SL_DMA_SIGNAL_LCD (&SL_DMA_SIGNAL_LCD_VALUE)

/// Define for LESENSE FIFO Signal.
#define SL_DMA_SIGNAL_LESENSE_FIFO (&SL_DMA_SIGNAL_LESENSE_FIFO_VALUE)

/// Define for VDAC0 CH0 REQ Signal.
#define SL_DMA_SIGNAL_VDAC0_CH0REQ (&SL_DMA_SIGNAL_VDAC0_CH0REQ_VALUE)
/// Define for VDAC0 CH1 REQ Signal.
#define SL_DMA_SIGNAL_VDAC0_CH1REQ (&SL_DMA_SIGNAL_VDAC0_CH1REQ_VALUE)

/// Define for VDAC1 CH0 REQ Signal.
#define SL_DMA_SIGNAL_VDAC1_CH0REQ (&SL_DMA_SIGNAL_VDAC1_CH0REQ_VALUE)
/// Define for VDAC1 CH1 REQ Signal.
#define SL_DMA_SIGNAL_VDAC1_CH1REQ (&SL_DMA_SIGNAL_VDAC1_CH1REQ_VALUE)

/// @} (end dma_signal_defines)

// ----------------------------------------------------------------------------
// TYPEDEFS

/// The DMA signal typedef.
typedef const uint32_t* sl_dma_signal_t;

// ----------------------------------------------------------------------------
// EXTERNS

/// @cond DO_NOT_INCLUDE_WITH_DOXYGEN

// External declaration for LDMAXBAR0_PRSREQ0 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_LDMAXBAR0_PRSREQ0_VALUE;
// External declaration for LDMAXBAR0_PRSREQ1 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_LDMAXBAR0_PRSREQ1_VALUE;

// External declaration for TIMER0 CC0 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER0_CC0_VALUE;
// External declaration for TIMER0 CC1 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER0_CC1_VALUE;
// External declaration for TIMER0 CC2 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER0_CC2_VALUE;
// External declaration for TIMER0 UFOF DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER0_UFOF_VALUE;

// External declaration for TIMER1 CC0 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER1_CC0_VALUE;
// External declaration for TIMER1 CC1 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER1_CC1_VALUE;
// External declaration for TIMER1 CC2 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER1_CC2_VALUE;
// External declaration for TIMER1 UFOF DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER1_UFOF_VALUE;

// External declaration for TIMER2 CC0 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER2_CC0_VALUE;
// External declaration for TIMER2 CC1 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER2_CC1_VALUE;
// External declaration for TIMER2 CC2 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER2_CC2_VALUE;
// External declaration for TIMER2 UFOF DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER2_UFOF_VALUE;
// External declaration for TIMER2 CC3 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER2_CC3_VALUE;
// External declaration for TIMER2 CC4 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER2_CC4_VALUE;
// External declaration for TIMER2 CC5 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER2_CC5_VALUE;
// External declaration for TIMER2 CC6 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER2_CC6_VALUE;

// External declaration for TIMER3 CC0 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER3_CC0_VALUE;
// External declaration for TIMER3 CC1 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER3_CC1_VALUE;
// External declaration for TIMER3 CC2 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER3_CC2_VALUE;
// External declaration for TIMER3 UFOF DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER3_UFOF_VALUE;
// External declaration for TIMER3 CC3 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER3_CC3_VALUE;
// External declaration for TIMER3 CC4 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER3_CC4_VALUE;
// External declaration for TIMER3 CC5 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER3_CC5_VALUE;
// External declaration for TIMER3 CC6 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER3_CC6_VALUE;

// External declaration for TIMER4 CC0 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER4_CC0_VALUE;
// External declaration for TIMER4 CC1 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER4_CC1_VALUE;
// External declaration for TIMER4 CC2 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER4_CC2_VALUE;
// External declaration for TIMER4 UFOF DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER4_UFOF_VALUE;

// External declaration for TIMER5 CC0 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER5_CC0_VALUE;
// External declaration for TIMER5 CC1 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER5_CC1_VALUE;
// External declaration for TIMER5 CC2 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER5_CC2_VALUE;
// External declaration for TIMER5 UFOF DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER5_UFOF_VALUE;

// External declaration for TIMER6 CC0 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER6_CC0_VALUE;
// External declaration for TIMER6 CC1 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER6_CC1_VALUE;
// External declaration for TIMER6 CC2 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER6_CC2_VALUE;
// External declaration for TIMER6 UFOF DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER6_UFOF_VALUE;

// External declaration for TIMER7 CC0 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER7_CC0_VALUE;
// External declaration for TIMER7 CC1 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER7_CC1_VALUE;
// External declaration for TIMER7 CC2 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER7_CC2_VALUE;
// External declaration for TIMER7 UFOF DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER7_UFOF_VALUE;

// External declaration for TIMER8 CC0 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER8_CC0_VALUE;
// External declaration for TIMER8 CC1 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER8_CC1_VALUE;
// External declaration for TIMER8 CC2 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER8_CC2_VALUE;
// External declaration for TIMER8 UFOF DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER8_UFOF_VALUE;

// External declaration for TIMER9 CC0 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER9_CC0_VALUE;
// External declaration for TIMER9 CC1 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER9_CC1_VALUE;
// External declaration for TIMER9 CC2 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER9_CC2_VALUE;
// External declaration for TIMER9 UFOF DMA Signal
extern const uint32_t SL_DMA_SIGNAL_TIMER9_UFOF_VALUE;

// External declaration for I2C0 RXDATAV DMA Signal
extern const uint32_t SL_DMA_SIGNAL_I2C0_RXDATAV_VALUE;
// External declaration for I2C0 TXBL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_I2C0_TXBL_VALUE;

// External declaration for I2C1 RXDATAV DMA Signal
extern const uint32_t SL_DMA_SIGNAL_I2C1_RXDATAV_VALUE;
// External declaration for I2C1 TXBL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_I2C1_TXBL_VALUE;

// External declaration for I2C2 RXDATAV DMA Signal
extern const uint32_t SL_DMA_SIGNAL_I2C2_RXDATAV_VALUE;
// External declaration for I2C2 TXBL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_I2C2_TXBL_VALUE;

// External declaration for I2C3 RXDATAV DMA Signal
extern const uint32_t SL_DMA_SIGNAL_I2C3_RXDATAV_VALUE;
// External declaration for I2C3 TXBL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_I2C3_TXBL_VALUE;

// External declaration for PROTIMER BOF DMA Signal
extern const uint32_t SL_DMA_SIGNAL_PROTIMER_BOF_VALUE;
// External declaration for PROTIMER CC0 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_PROTIMER_CC0_VALUE;
// External declaration for PROTIMER CC1 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_PROTIMER_CC1_VALUE;
// External declaration for PROTIMER CC2 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_PROTIMER_CC2_VALUE;
// External declaration for PROTIMER CC3 DMA Signal
extern const uint32_t SL_DMA_SIGNAL_PROTIMER_CC3_VALUE;
// External declaration for PROTIMER POF DMA Signal
extern const uint32_t SL_DMA_SIGNAL_PROTIMER_POF_VALUE;
// External declaration for PROTIMER WOF DMA Signal
extern const uint32_t SL_DMA_SIGNAL_PROTIMER_WOF_VALUE;

// External declaration for ADC0 SCAN DMA Signal
extern const uint32_t SL_DMA_SIGNAL_ADC0_SCAN_VALUE;

// External declaration for ADC1 SCAN DMA Signal
extern const uint32_t SL_DMA_SIGNAL_ADC1_SCAN_VALUE;

// External declaration for IADC0 SCAN DMA Signal
extern const uint32_t SL_DMA_SIGNAL_IADC0_SCAN_VALUE;
// External declaration for IADC0 SINGLE DMA Signal
extern const uint32_t SL_DMA_SIGNAL_IADC0_SINGLE_VALUE;

// External declaration for EUSART0 RXFL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_EUSART0_RXFL_VALUE;
// External declaration for EUSART0 TXFL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_EUSART0_TXFL_VALUE;

// External declaration for EUSART1 RXFL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_EUSART1_RXFL_VALUE;
// External declaration for EUSART1 TXFL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_EUSART1_TXFL_VALUE;

// External declaration for EUSART2 RXFL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_EUSART2_RXFL_VALUE;
// External declaration for EUSART2 TXFL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_EUSART2_TXFL_VALUE;

// External declaration for EUSART3 RXFL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_EUSART3_RXFL_VALUE;
// External declaration for EUSART3 TXFL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_EUSART3_TXFL_VALUE;

// External declaration for EUSART4 RXFL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_EUSART4_RXFL_VALUE;
// External declaration for EUSART4 TXFL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_EUSART4_TXFL_VALUE;

// External declaration for EUART0 RXFL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_EUART0_RXFL_VALUE;
// External declaration for EUART0 TXFL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_EUART0_TXFL_VALUE;

// External declaration for USART0 RXDATAV DMA Signal
extern const uint32_t SL_DMA_SIGNAL_USART0_RXDATAV_VALUE;
// External declaration for USART0 RXDATAVRIGHT DMA Signal
extern const uint32_t SL_DMA_SIGNAL_USART0_RXDATAVRIGHT_VALUE;
// External declaration for USART0 TXBL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_USART0_TXBL_VALUE;
// External declaration for USART0 TXBLRIGHT DMA Signal
extern const uint32_t SL_DMA_SIGNAL_USART0_TXBLRIGHT_VALUE;
// External declaration for USART0 TXEMPTY DMA Signal
extern const uint32_t SL_DMA_SIGNAL_USART0_TXEMPTY_VALUE;

// External declaration for USART1 RXDATAV DMA Signal
extern const uint32_t SL_DMA_SIGNAL_USART1_RXDATAV_VALUE;
// External declaration for USART1 RXDATAVRIGHT DMA Signal
extern const uint32_t SL_DMA_SIGNAL_USART1_RXDATAVRIGHT_VALUE;
// External declaration for USART1 TXBL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_USART1_TXBL_VALUE;
// External declaration for USART1 TXBLRIGHT DMA Signal
extern const uint32_t SL_DMA_SIGNAL_USART1_TXBLRIGHT_VALUE;
// External declaration for USART1 TXEMPTY DMA Signal
extern const uint32_t SL_DMA_SIGNAL_USART1_TXEMPTY_VALUE;

// External declaration for USART2 RXDATAV DMA Signal
extern const uint32_t SL_DMA_SIGNAL_USART2_RXDATAV_VALUE;
// External declaration for USART2 RXDATAVRIGHT DMA Signal
extern const uint32_t SL_DMA_SIGNAL_USART2_RXDATAVRIGHT_VALUE;
// External declaration for USART2 TXBL DMA Signal
extern const uint32_t SL_DMA_SIGNAL_USART2_TXBL_VALUE;
// External declaration for USART2 TXBLRIGHT DMA Signal
extern const uint32_t SL_DMA_SIGNAL_USART2_TXBLRIGHT_VALUE;
// External declaration for USART2 TXEMPTY DMA Signal
extern const uint32_t SL_DMA_SIGNAL_USART2_TXEMPTY_VALUE;

// External declaration for MSC WDATA DMA Signal
extern const uint32_t SL_DMA_SIGNAL_MSC_WDATA_VALUE;

// External declaration for MVP REQ DMA Signal
extern const uint32_t SL_DMA_SIGNAL_MVP_REQ_VALUE;

// External declaration for PDM RXDATAV DMA Signal
extern const uint32_t SL_DMA_SIGNAL_PDM_RXDATAV_VALUE;

// External declaration for PDM0 RXDATA DMA Signal
extern const uint32_t SL_DMA_SIGNAL_PDM0_RXDATA_VALUE;

// External declaration for LCD DMA Signal
extern const uint32_t SL_DMA_SIGNAL_LCD_VALUE;

// External declaration for LESENSE FIFO DMA Signal
extern const uint32_t SL_DMA_SIGNAL_LESENSE_FIFO_VALUE;

// External declaration for VDAC0 CH0 REQ DMA Signal
extern const uint32_t SL_DMA_SIGNAL_VDAC0_CH0REQ_VALUE;
// External declaration for VDAC0 CH1 REQ DMA Signal
extern const uint32_t SL_DMA_SIGNAL_VDAC0_CH1REQ_VALUE;

// External declaration for VDAC1 CH0 REQ DMA Signal
extern const uint32_t SL_DMA_SIGNAL_VDAC1_CH0REQ_VALUE;
// External declaration for VDAC1 CH1 REQ DMA Signal
extern const uint32_t SL_DMA_SIGNAL_VDAC1_CH1REQ_VALUE;

// External declaration for PIXELRZ0REQ TXF DMA Signal
extern const uint32_t SL_DMA_SIGNAL_PIXELRZ0REQ_TXF_VALUE;

// External declaration for PIXELRZ1REQ TXF DMA Signal
extern const uint32_t SL_DMA_SIGNAL_PIXELRZ1REQ_TXF_VALUE;

/// @endcond

/** @} (end addtogroup device_dma) */

#ifdef __cplusplus
}
#endif

#endif // SL_DEVICE_DMA_H
