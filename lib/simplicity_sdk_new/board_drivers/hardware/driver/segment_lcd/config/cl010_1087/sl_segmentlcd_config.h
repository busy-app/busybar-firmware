/***************************************************************************//**
 * @file
 * @brief Segment LCD Config for the LCD module CL010_1087
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SEGMENTLCD_CONFIG_H
#define SL_SEGMENTLCD_CONFIG_H

#include "sl_hal_lcd.h"
#include "sl_segmentlcd_pin_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// Define the LCD module type
#define SL_SEGMENT_LCD_MODULE_CL010_1087
// *********************************************
//
// Board/device specific config
//
// *********************************************

// <<< Use Configuration Wizard in Context Menu >>>

// <h>Frame Rate
// LCD Controller Prescaler (divide LCDCLK / 64)
// LCDCLK_pre = 512 Hz
// Set FDIV=0, means 512/1 = 512 Hz
// With octaplex mode, 512/16 => 32 Hz Frame Rate

// <o SL_SEGMENT_LCD_CLK_PRE> LCD Controller prescaler
// <i> Default: 64
#define SL_SEGMENT_LCD_CLK_PRE          64

// <o SL_SEGMENT_LCD_FRAME_RATE_DIV> LCD frame rate divisor
// <i> Default: 0
#define SL_SEGMENT_LCD_FRAME_RATE_DIV   0
// </h> end Frame Rate

// <h> LDMA settings
// <o SL_SEGMENT_LCD_LDMA_MODE> LDMA mode of operation
// <SL_HAL_LCD_DMA_MODE_DISABLE=> No DMA requests are generated 
// <SL_HAL_LCD_DMA_MODE_FRAME_COUNTER_EVENT=> DMA request on frame counter event
// <SL_HAL_LCD_DMA_MODE_DISPLAY_EVENT=> DMA request on display counter event
// <i> Default: SL_HAL_LCD_DMA_MODE_DISABLE
#define SL_SEGMENT_LCD_LDMA_MODE      SL_HAL_LCD_DMA_MODE_DISABLE

// <o SL_SEGMENT_LCD_BACFG_FCPRESC> Frame Counter Clock Prescaler
// FC-CLK = FrameRate (Hz) / this factor.
// <SL_HAL_LCD_FC_PRESC_DIV1=> Prescale Div 1 
// <SL_HAL_LCD_FC_PRESC_DIV2=> Prescale Div 2
// <SL_HAL_LCD_FC_PRESC_DIV4=> Prescale Div 4
// <SL_HAL_LCD_FC_PRESC_DIV8=> Prescale Div 8
// <i> Default: SL_HAL_LCD_FC_PRESC_DIV1
#define SL_SEGMENT_LCD_BACFG_FCPRESC   SL_HAL_LCD_FRAME_COUNTER_PRESCALE_DIV1
// </h> end LDMA settings

// <o SL_SEGMENT_LCD_CONTRAST> LCD contrast
// <i> Default: 15
#define SL_SEGMENT_LCD_CONTRAST   15

// LCD boost contrast
#define SL_SEGMENT_LCD_BOOST_CONTRAST   0x2


// <o SL_SEGMENT_LCD_MUX_CONFIG> Mux configuration
// <SL_HAL_LCD_MUX_STATIC=> Static 
// <SL_HAL_LCD_MUX_DUPLEX=> Duplex (1/2 duty cycle)
// <SL_HAL_LCD_MUX_TRIPLEX=> Triplex (1/3 duty cycle)
// <SL_HAL_LCD_MUX_QUADRUPLEX=> Quadruplex (1/4 duty cycle)
// <SL_HAL_LCD_MUX_SEXTAPLEX=> Sextaplex (1/6 duty cycle)
// <SL_HAL_LCD_MUX_OCTAPLEX=> Octaplex (1/8 duty cycle)
// <i> Default: SL_HAL_LCD_MUX_OCTAPLEX
#define SL_SEGMENT_LCD_MUX_CONFIG             SL_HAL_LCD_MUX_OCTAPLEX

// <o SL_SEGMENT_LCD_WAVE_TYPE> Wave type
// <SL_HAL_LCD_WAVE_LOW_POWER=> Low power
// <SL_HAL_LCD_WAVE_NORMAL=> Regular
// <i> Default: SL_HAL_LCD_WAVE_LOW_POWER
#define SL_SEGMENT_LCD_WAVE_TYPE              SL_HAL_LCD_WAVE_LOW_POWER

// <o SL_SEGMENT_LCD_BIAS_SETTING> Bias setting
// <SL_HAL_LCD_BIAS_STATIC=> Static (2 levels)
// <SL_HAL_LCD_BIAS_ONE_HALF=> 1/2 bias (3 levels)
// <SL_HAL_LCD_BIAS_ONE_THIRD=> 1/3 bias (4 levels)
// <SL_HAL_LCD_BIAS_ONE_FOURTH=> 1/4 bias (5 levels)
// <i> Default: SL_HAL_LCD_BIAS_ONE_FOURTH
#define SL_SEGMENT_LCD_BIAS_SETTING           SL_HAL_LCD_BIAS_ONE_FOURTH

// <o SL_SEGMENT_LCD_MODE_OPERATION> Mode of operation
// <SL_HAL_LCD_MODE_STEP_DOWN=> Mode step down
// <SL_HAL_LCD_MODE_CHARGE_PUMP=> Mode charge pump
// <i> Default: SL_HAL_LCD_MODE_CHARGE_PUMP
#define SL_SEGMENT_LCD_MODE_OPERATION         SL_HAL_LCD_MODE_CHARGE_PUMP

// <o SL_SEGMENT_LCD_CHARGE_REDIS_CYCL> Charge redistribution cycles
// <SL_HAL_LCD_CHARGE_REDISTRIBUTION_DISABLE=> Disabled
// <SL_HAL_LCD_CHARGE_REDISTRIBUTION_ENABLE=> 1 prescaled low frequency
// <SL_HAL_LCD_CHARGE_REDISTRIBUTION_TWO_CYCLE=> 2 prescaled low frequency
// <SL_HAL_LCD_CHARGE_REDISTRIBUTION_THREE_CYCLE=> 3 prescaled low frequency
// <SL_HAL_LCD_CHARGE_REDISTRIBUTION_FOUR_CYCLE=> 4 prescaled low frequency
// <i> Default: SL_HAL_LCD_CHARGE_REDISTRIBUTION_DISABLE
#define SL_SEGMENT_LCD_CHARGE_REDIS_CYCL       SL_HAL_LCD_CHARGE_REDISTRIBUTION_DISABLE

// <<< end of configuration section >>>

// LCD initialization structure
#define SL_SEGMENT_LCD_INIT_DEF             \
  { SL_SEGMENT_LCD_MUX_CONFIG,              \
    SL_SEGMENT_LCD_BIAS_SETTING,            \
    SL_SEGMENT_LCD_WAVE_TYPE,               \
    SL_SEGMENT_LCD_MODE_OPERATION,          \
    SL_SEGMENT_LCD_CHARGE_REDIS_CYCL,       \
    SL_SEGMENT_LCD_FRAME_RATE_DIV,          \
    SL_SEGMENT_LCD_CONTRAST,                \
    SL_SEGMENT_LCD_CLK_PRE                  \
  }

// *********************************************
//
// General LCD_MODULE_CL010_1087 config
//
// *********************************************

/** Range of symbols available on display */
typedef enum {
  SL_LCD_SYMBOL_GECKO,
  SL_LCD_SYMBOL_ANT,
  SL_LCD_SYMBOL_PAD0,
  SL_LCD_SYMBOL_PAD1,
  SL_LCD_SYMBOL_EFM32,
  SL_LCD_SYMBOL_MINUS,
  SL_LCD_SYMBOL_COL3,
  SL_LCD_SYMBOL_COL5,
  SL_LCD_SYMBOL_COL10,
  SL_LCD_SYMBOL_DEGC,
  SL_LCD_SYMBOL_DEGF,
  SL_LCD_SYMBOL_DP2,
  SL_LCD_SYMBOL_DP3,
  SL_LCD_SYMBOL_DP4,
  SL_LCD_SYMBOL_DP5,
  SL_LCD_SYMBOL_DP6,
  SL_LCD_SYMBOL_DP10,
} sl_segment_lcd_symbol_t;

#define SL_LCD_SYMBOL_GECKO_COM    SL_SEGMENT_LCD_COM_C06
#define SL_LCD_SYMBOL_GECKO_SEG    SL_SEGMENT_LCD_SEG_S00
#define SL_LCD_SYMBOL_ANT_COM      SL_SEGMENT_LCD_COM_C07
#define SL_LCD_SYMBOL_ANT_SEG      SL_SEGMENT_LCD_SEG_S12
#define SL_LCD_SYMBOL_PAD0_COM     SL_SEGMENT_LCD_COM_C04
#define SL_LCD_SYMBOL_PAD0_SEG     SL_SEGMENT_LCD_SEG_S19
#define SL_LCD_SYMBOL_PAD1_COM     SL_SEGMENT_LCD_COM_C05
#define SL_LCD_SYMBOL_PAD1_SEG     SL_SEGMENT_LCD_SEG_S00
#define SL_LCD_SYMBOL_EFM32_COM    SL_SEGMENT_LCD_COM_C07
#define SL_LCD_SYMBOL_EFM32_SEG    SL_SEGMENT_LCD_SEG_S08
#define SL_LCD_SYMBOL_MINUS_COM    SL_SEGMENT_LCD_COM_C04
#define SL_LCD_SYMBOL_MINUS_SEG    SL_SEGMENT_LCD_SEG_S00
#define SL_LCD_SYMBOL_COL3_COM     SL_SEGMENT_LCD_COM_C03
#define SL_LCD_SYMBOL_COL3_SEG     SL_SEGMENT_LCD_SEG_S00
#define SL_LCD_SYMBOL_COL5_COM     SL_SEGMENT_LCD_COM_C07
#define SL_LCD_SYMBOL_COL5_SEG     SL_SEGMENT_LCD_SEG_S10
#define SL_LCD_SYMBOL_COL10_COM    SL_SEGMENT_LCD_COM_C02
#define SL_LCD_SYMBOL_COL10_SEG    SL_SEGMENT_LCD_SEG_S19
#define SL_LCD_SYMBOL_DEGC_COM     SL_SEGMENT_LCD_COM_C07
#define SL_LCD_SYMBOL_DEGC_SEG     SL_SEGMENT_LCD_SEG_S14
#define SL_LCD_SYMBOL_DEGF_COM     SL_SEGMENT_LCD_COM_C07
#define SL_LCD_SYMBOL_DEGF_SEG     SL_SEGMENT_LCD_SEG_S15
#define SL_LCD_SYMBOL_DP2_COM      SL_SEGMENT_LCD_COM_C00
#define SL_LCD_SYMBOL_DP2_SEG      SL_SEGMENT_LCD_SEG_S00
#define SL_LCD_SYMBOL_DP3_COM      SL_SEGMENT_LCD_COM_C02
#define SL_LCD_SYMBOL_DP3_SEG      SL_SEGMENT_LCD_SEG_S00
#define SL_LCD_SYMBOL_DP4_COM      SL_SEGMENT_LCD_COM_C01
#define SL_LCD_SYMBOL_DP4_SEG      SL_SEGMENT_LCD_SEG_S00
#define SL_LCD_SYMBOL_DP5_COM      SL_SEGMENT_LCD_COM_C00
#define SL_LCD_SYMBOL_DP5_SEG      SL_SEGMENT_LCD_SEG_S09
#define SL_LCD_SYMBOL_DP6_COM      SL_SEGMENT_LCD_COM_C00
#define SL_LCD_SYMBOL_DP6_SEG      SL_SEGMENT_LCD_SEG_S11
#define SL_LCD_SYMBOL_DP10_COM     SL_SEGMENT_LCD_COM_C03
#define SL_LCD_SYMBOL_DP10_SEG     SL_SEGMENT_LCD_SEG_S19

#define SL_SEGMENT_LCD_EFM_DISPLAY_TEXT    \
    .text        = {                                                                                                                                      \
          { /* 1 */                                                                                                                                       \
            .com[0] = SL_SEGMENT_LCD_COM_C06, .com[1] = SL_SEGMENT_LCD_COM_C06, .com[2] = SL_SEGMENT_LCD_COM_C02, .com[3] = SL_SEGMENT_LCD_COM_C00,       \
            .bit[0] = SL_SEGMENT_LCD_SEG_S01, .bit[1] = SL_SEGMENT_LCD_SEG_S02, .bit[2] = SL_SEGMENT_LCD_SEG_S02, .bit[3] = SL_SEGMENT_LCD_SEG_S02,       \
            .com[4] = SL_SEGMENT_LCD_COM_C00, .com[5] = SL_SEGMENT_LCD_COM_C04, .com[6] = SL_SEGMENT_LCD_COM_C03, .com[7] = SL_SEGMENT_LCD_COM_C05,       \
            .bit[4] = SL_SEGMENT_LCD_SEG_S01, .bit[5] = SL_SEGMENT_LCD_SEG_S01, .bit[6] = SL_SEGMENT_LCD_SEG_S01, .bit[7] = SL_SEGMENT_LCD_SEG_S01,       \
            .com[8] = SL_SEGMENT_LCD_COM_C04, .com[9] = SL_SEGMENT_LCD_COM_C05, .com[10] = SL_SEGMENT_LCD_COM_C03, .com[11] = SL_SEGMENT_LCD_COM_C01,     \
            .bit[8] = SL_SEGMENT_LCD_SEG_S02, .bit[9] = SL_SEGMENT_LCD_SEG_S02, .bit[10] = SL_SEGMENT_LCD_SEG_S02, .bit[11] = SL_SEGMENT_LCD_SEG_S02,     \
            .com[12] = SL_SEGMENT_LCD_COM_C02, .com[13] = SL_SEGMENT_LCD_COM_C01,                                                                         \
            .bit[12] = SL_SEGMENT_LCD_SEG_S01, .bit[13] = SL_SEGMENT_LCD_SEG_S01                                                                          \
          },                                                                                                                                              \
          { /* 2 */                                                                                                                                       \
            .com[0] = SL_SEGMENT_LCD_COM_C06, .com[1] = SL_SEGMENT_LCD_COM_C06, .com[2] = SL_SEGMENT_LCD_COM_C02, .com[3] = SL_SEGMENT_LCD_COM_C00,       \
            .bit[0] = SL_SEGMENT_LCD_SEG_S03, .bit[1] = SL_SEGMENT_LCD_SEG_S04, .bit[2] = SL_SEGMENT_LCD_SEG_S04, .bit[3] = SL_SEGMENT_LCD_SEG_S04,       \
            .com[4] = SL_SEGMENT_LCD_COM_C00, .com[5] = SL_SEGMENT_LCD_COM_C04, .com[6] = SL_SEGMENT_LCD_COM_C03, .com[7] = SL_SEGMENT_LCD_COM_C05,       \
            .bit[4] = SL_SEGMENT_LCD_SEG_S03, .bit[5] = SL_SEGMENT_LCD_SEG_S03, .bit[6] = SL_SEGMENT_LCD_SEG_S03, .bit[7] = SL_SEGMENT_LCD_SEG_S03,       \
            .com[8] = SL_SEGMENT_LCD_COM_C04, .com[9] = SL_SEGMENT_LCD_COM_C05, .com[10] = SL_SEGMENT_LCD_COM_C03, .com[11] = SL_SEGMENT_LCD_COM_C01,     \
            .bit[8] = SL_SEGMENT_LCD_SEG_S04, .bit[9] = SL_SEGMENT_LCD_SEG_S04, .bit[10] = SL_SEGMENT_LCD_SEG_S04, .bit[11] = SL_SEGMENT_LCD_SEG_S04,     \
            .com[12] = SL_SEGMENT_LCD_COM_C02, .com[13] = SL_SEGMENT_LCD_COM_C01,                                                                         \
            .bit[12] = SL_SEGMENT_LCD_SEG_S03, .bit[13] = SL_SEGMENT_LCD_SEG_S03                                                                          \
          },                                                                                                                                              \
          { /* 3 */                                                                                                                                       \
            .com[0] = SL_SEGMENT_LCD_COM_C06, .com[1] = SL_SEGMENT_LCD_COM_C06, .com[2] = SL_SEGMENT_LCD_COM_C02, .com[3] = SL_SEGMENT_LCD_COM_C00,       \
            .bit[0] = SL_SEGMENT_LCD_SEG_S05, .bit[1] = SL_SEGMENT_LCD_SEG_S06, .bit[2] = SL_SEGMENT_LCD_SEG_S06, .bit[3] = SL_SEGMENT_LCD_SEG_S06,       \
            .com[4] = SL_SEGMENT_LCD_COM_C00, .com[5] = SL_SEGMENT_LCD_COM_C04, .com[6] = SL_SEGMENT_LCD_COM_C03, .com[7] = SL_SEGMENT_LCD_COM_C05,       \
            .bit[4] = SL_SEGMENT_LCD_SEG_S05, .bit[5] = SL_SEGMENT_LCD_SEG_S05, .bit[6] = SL_SEGMENT_LCD_SEG_S05, .bit[7] = SL_SEGMENT_LCD_SEG_S05,       \
            .com[8] = SL_SEGMENT_LCD_COM_C04, .com[9] = SL_SEGMENT_LCD_COM_C05, .com[10] = SL_SEGMENT_LCD_COM_C03, .com[11] = SL_SEGMENT_LCD_COM_C01,     \
            .bit[8] = SL_SEGMENT_LCD_SEG_S06, .bit[9] = SL_SEGMENT_LCD_SEG_S06, .bit[10] = SL_SEGMENT_LCD_SEG_S06, .bit[11] = SL_SEGMENT_LCD_SEG_S06,     \
            .com[12] = SL_SEGMENT_LCD_COM_C02, .com[13] = SL_SEGMENT_LCD_COM_C01,                                                                         \
            .bit[12] = SL_SEGMENT_LCD_SEG_S05, .bit[13] = SL_SEGMENT_LCD_SEG_S05                                                                          \
          },                                                                                                                                              \
          { /* 4 */                                                                                                                                       \
            .com[0] = SL_SEGMENT_LCD_COM_C06, .com[1] = SL_SEGMENT_LCD_COM_C06, .com[2] = SL_SEGMENT_LCD_COM_C02, .com[3] = SL_SEGMENT_LCD_COM_C00,       \
            .bit[0] = SL_SEGMENT_LCD_SEG_S07, .bit[1] = SL_SEGMENT_LCD_SEG_S08, .bit[2] = SL_SEGMENT_LCD_SEG_S08, .bit[3] = SL_SEGMENT_LCD_SEG_S08,       \
            .com[4] = SL_SEGMENT_LCD_COM_C00, .com[5] = SL_SEGMENT_LCD_COM_C04, .com[6] = SL_SEGMENT_LCD_COM_C03, .com[7] = SL_SEGMENT_LCD_COM_C05,       \
            .bit[4] = SL_SEGMENT_LCD_SEG_S07, .bit[5] = SL_SEGMENT_LCD_SEG_S07, .bit[6] = SL_SEGMENT_LCD_SEG_S07, .bit[7] = SL_SEGMENT_LCD_SEG_S07,       \
            .com[8] = SL_SEGMENT_LCD_COM_C04, .com[9] = SL_SEGMENT_LCD_COM_C05, .com[10] = SL_SEGMENT_LCD_COM_C03, .com[11] = SL_SEGMENT_LCD_COM_C01,     \
            .bit[8] = SL_SEGMENT_LCD_SEG_S08, .bit[9] = SL_SEGMENT_LCD_SEG_S08, .bit[10] = SL_SEGMENT_LCD_SEG_S08, .bit[11] = SL_SEGMENT_LCD_SEG_S08,     \
            .com[12] = SL_SEGMENT_LCD_COM_C02, .com[13] = SL_SEGMENT_LCD_COM_C01,                                                                         \
            .bit[12] = SL_SEGMENT_LCD_SEG_S07, .bit[13] = SL_SEGMENT_LCD_SEG_S07                                                                          \
          },                                                                                                                                              \
          { /* 5 */                                                                                                                                       \
            .com[0] = SL_SEGMENT_LCD_COM_C07, .com[1] = SL_SEGMENT_LCD_COM_C06, .com[2] = SL_SEGMENT_LCD_COM_C02, .com[3] = SL_SEGMENT_LCD_COM_C00,       \
            .bit[0] = SL_SEGMENT_LCD_SEG_S09, .bit[1] = SL_SEGMENT_LCD_SEG_S10, .bit[2] = SL_SEGMENT_LCD_SEG_S10, .bit[3] = SL_SEGMENT_LCD_SEG_S10,       \
            .com[4] = SL_SEGMENT_LCD_COM_C01, .com[5] = SL_SEGMENT_LCD_COM_C05, .com[6] = SL_SEGMENT_LCD_COM_C04, .com[7] = SL_SEGMENT_LCD_COM_C06,       \
            .bit[4] = SL_SEGMENT_LCD_SEG_S09, .bit[5] = SL_SEGMENT_LCD_SEG_S09, .bit[6] = SL_SEGMENT_LCD_SEG_S09, .bit[7] = SL_SEGMENT_LCD_SEG_S09,       \
            .com[8] = SL_SEGMENT_LCD_COM_C04, .com[9] = SL_SEGMENT_LCD_COM_C05, .com[10] = SL_SEGMENT_LCD_COM_C03, .com[11] = SL_SEGMENT_LCD_COM_C01,     \
            .bit[8] = SL_SEGMENT_LCD_SEG_S10, .bit[9] = SL_SEGMENT_LCD_SEG_S10, .bit[10] = SL_SEGMENT_LCD_SEG_S10, .bit[11] = SL_SEGMENT_LCD_SEG_S10,     \
            .com[12] = SL_SEGMENT_LCD_COM_C03, .com[13] = SL_SEGMENT_LCD_COM_C02,                                                                         \
            .bit[12] = SL_SEGMENT_LCD_SEG_S09, .bit[13] = SL_SEGMENT_LCD_SEG_S09                                                                          \
          },                                                                                                                                              \
          { /* 6 */                                                                                                                                       \
            .com[0] = SL_SEGMENT_LCD_COM_C07, .com[1] = SL_SEGMENT_LCD_COM_C06, .com[2] = SL_SEGMENT_LCD_COM_C02, .com[3] = SL_SEGMENT_LCD_COM_C00,       \
            .bit[0] = SL_SEGMENT_LCD_SEG_S11, .bit[1] = SL_SEGMENT_LCD_SEG_S12, .bit[2] = SL_SEGMENT_LCD_SEG_S12, .bit[3] = SL_SEGMENT_LCD_SEG_S12,       \
            .com[4] = SL_SEGMENT_LCD_COM_C01, .com[5] = SL_SEGMENT_LCD_COM_C05, .com[6] = SL_SEGMENT_LCD_COM_C04, .com[7] = SL_SEGMENT_LCD_COM_C06,       \
            .bit[4] = SL_SEGMENT_LCD_SEG_S11, .bit[5] = SL_SEGMENT_LCD_SEG_S11, .bit[6] = SL_SEGMENT_LCD_SEG_S11, .bit[7] = SL_SEGMENT_LCD_SEG_S11,       \
            .com[8] = SL_SEGMENT_LCD_COM_C04, .com[9] = SL_SEGMENT_LCD_COM_C05, .com[10] = SL_SEGMENT_LCD_COM_C03, .com[11] = SL_SEGMENT_LCD_COM_C01,     \
            .bit[8] = SL_SEGMENT_LCD_SEG_S12, .bit[9] = SL_SEGMENT_LCD_SEG_S12, .bit[10] = SL_SEGMENT_LCD_SEG_S12, .bit[11] = SL_SEGMENT_LCD_SEG_S12,     \
            .com[12] = SL_SEGMENT_LCD_COM_C03, .com[13] = SL_SEGMENT_LCD_COM_C02,                                                                         \
            .bit[12] = SL_SEGMENT_LCD_SEG_S11, .bit[13] = SL_SEGMENT_LCD_SEG_S11                                                                          \
          },                                                                                                                                              \
          { /* 7 */                                                                                                                                       \
            .com[0] = SL_SEGMENT_LCD_COM_C06, .com[1] = SL_SEGMENT_LCD_COM_C06, .com[2] = SL_SEGMENT_LCD_COM_C02, .com[3] = SL_SEGMENT_LCD_COM_C00,       \
            .bit[0] = SL_SEGMENT_LCD_SEG_S13, .bit[1] = SL_SEGMENT_LCD_SEG_S14, .bit[2] = SL_SEGMENT_LCD_SEG_S14, .bit[3] = SL_SEGMENT_LCD_SEG_S14,       \
            .com[4] = SL_SEGMENT_LCD_COM_C00, .com[5] = SL_SEGMENT_LCD_COM_C04, .com[6] = SL_SEGMENT_LCD_COM_C03, .com[7] = SL_SEGMENT_LCD_COM_C05,       \
            .bit[4] = SL_SEGMENT_LCD_SEG_S13, .bit[5] = SL_SEGMENT_LCD_SEG_S13, .bit[6] = SL_SEGMENT_LCD_SEG_S13, .bit[7] = SL_SEGMENT_LCD_SEG_S13,       \
            .com[8] = SL_SEGMENT_LCD_COM_C04, .com[9] = SL_SEGMENT_LCD_COM_C05, .com[10] = SL_SEGMENT_LCD_COM_C03, .com[11] = SL_SEGMENT_LCD_COM_C01,     \
            .bit[8] = SL_SEGMENT_LCD_SEG_S14, .bit[9] = SL_SEGMENT_LCD_SEG_S14, .bit[10] = SL_SEGMENT_LCD_SEG_S14, .bit[11] = SL_SEGMENT_LCD_SEG_S14,     \
            .com[12] = SL_SEGMENT_LCD_COM_C02, .com[13] = SL_SEGMENT_LCD_COM_C01,                                                                         \
            .bit[12] = SL_SEGMENT_LCD_SEG_S13, .bit[13] = SL_SEGMENT_LCD_SEG_S13                                                                          \
          },                                                                                                                                              \
        }

#define SL_SEGMENT_LCD_EFM_DISPLAY_NUM    \
          .number      = {                                                                                                                                      \
                {                                                                                                                                               \
                  .com[0] = SL_SEGMENT_LCD_COM_C00, .com[1] = SL_SEGMENT_LCD_COM_C02, .com[2] = SL_SEGMENT_LCD_COM_C05, .com[3] = SL_SEGMENT_LCD_COM_C06,       \
                  .bit[0] = SL_SEGMENT_LCD_SEG_S15, .bit[1] = SL_SEGMENT_LCD_SEG_S15, .bit[2] = SL_SEGMENT_LCD_SEG_S15, .bit[3] = SL_SEGMENT_LCD_SEG_S15,       \
                  .com[4] = SL_SEGMENT_LCD_COM_C04, .com[5] = SL_SEGMENT_LCD_COM_C01, .com[6] = SL_SEGMENT_LCD_COM_C03,                                         \
                  .bit[4] = SL_SEGMENT_LCD_SEG_S15, .bit[5] = SL_SEGMENT_LCD_SEG_S15, .bit[6] = SL_SEGMENT_LCD_SEG_S15,                                         \
                },                                                                                                                                              \
                {                                                                                                                                               \
                  .com[0] = SL_SEGMENT_LCD_COM_C00, .com[1] = SL_SEGMENT_LCD_COM_C02, .com[2] = SL_SEGMENT_LCD_COM_C05, .com[3] = SL_SEGMENT_LCD_COM_C06,       \
                  .bit[0] = SL_SEGMENT_LCD_SEG_S16, .bit[1] = SL_SEGMENT_LCD_SEG_S16, .bit[2] = SL_SEGMENT_LCD_SEG_S16, .bit[3] = SL_SEGMENT_LCD_SEG_S16,       \
                  .com[4] = SL_SEGMENT_LCD_COM_C04, .com[5] = SL_SEGMENT_LCD_COM_C01, .com[6] = SL_SEGMENT_LCD_COM_C03,                                         \
                  .bit[4] = SL_SEGMENT_LCD_SEG_S16, .bit[5] = SL_SEGMENT_LCD_SEG_S16, .bit[6] = SL_SEGMENT_LCD_SEG_S16,                                         \
                },                                                                                                                                              \
                {                                                                                                                                               \
                  .com[0] = SL_SEGMENT_LCD_COM_C00, .com[1] = SL_SEGMENT_LCD_COM_C02, .com[2] = SL_SEGMENT_LCD_COM_C05, .com[3] = SL_SEGMENT_LCD_COM_C06,       \
                  .bit[0] = SL_SEGMENT_LCD_SEG_S17, .bit[1] = SL_SEGMENT_LCD_SEG_S17, .bit[2] = SL_SEGMENT_LCD_SEG_S17, .bit[3] = SL_SEGMENT_LCD_SEG_S17,       \
                  .com[4] = SL_SEGMENT_LCD_COM_C04, .com[5] = SL_SEGMENT_LCD_COM_C01, .com[6] = SL_SEGMENT_LCD_COM_C03,                                         \
                  .bit[4] = SL_SEGMENT_LCD_SEG_S17, .bit[5] = SL_SEGMENT_LCD_SEG_S17, .bit[6] = SL_SEGMENT_LCD_SEG_S17,                                         \
                },                                                                                                                                              \
                {                                                                                                                                               \
                  .com[0] = SL_SEGMENT_LCD_COM_C00, .com[1] = SL_SEGMENT_LCD_COM_C02, .com[2] = SL_SEGMENT_LCD_COM_C05, .com[3] = SL_SEGMENT_LCD_COM_C06,       \
                  .bit[0] = SL_SEGMENT_LCD_SEG_S18, .bit[1] = SL_SEGMENT_LCD_SEG_S18, .bit[2] = SL_SEGMENT_LCD_SEG_S18, .bit[3] = SL_SEGMENT_LCD_SEG_S18,       \
                  .com[4] = SL_SEGMENT_LCD_COM_C04, .com[5] = SL_SEGMENT_LCD_COM_C01, .com[6] = SL_SEGMENT_LCD_COM_C03,                                         \
                  .bit[4] = SL_SEGMENT_LCD_SEG_S18, .bit[5] = SL_SEGMENT_LCD_SEG_S18, .bit[6] = SL_SEGMENT_LCD_SEG_S18,                                         \
                },                                                                                                                                              \
              }

#define SL_SEGMENT_LCD_EFM_DISPLAY_SYMBOL    \
            .emode       = {                                                                            \
              .com[0] = SL_SEGMENT_LCD_COM_C07, .bit[0] = SL_SEGMENT_LCD_SEG_S19,                       \
              .com[1] = SL_SEGMENT_LCD_COM_C06, .bit[1] = SL_SEGMENT_LCD_SEG_S19,                       \
              .com[2] = SL_SEGMENT_LCD_COM_C00, .bit[2] = SL_SEGMENT_LCD_SEG_S19,                       \
              .com[3] = SL_SEGMENT_LCD_COM_C05, .bit[3] = SL_SEGMENT_LCD_SEG_S19,                       \
              .com[4] = SL_SEGMENT_LCD_COM_C01, .bit[4] = SL_SEGMENT_LCD_SEG_S19,                       \
            },                                                                                          \
            .aring       = {                                                                            \
              .com[0] = SL_SEGMENT_LCD_COM_C07, .bit[0] = SL_SEGMENT_LCD_SEG_S07,                       \
              .com[1] = SL_SEGMENT_LCD_COM_C07, .bit[1] = SL_SEGMENT_LCD_SEG_S06,                       \
              .com[2] = SL_SEGMENT_LCD_COM_C07, .bit[2] = SL_SEGMENT_LCD_SEG_S05,                       \
              .com[3] = SL_SEGMENT_LCD_COM_C07, .bit[3] = SL_SEGMENT_LCD_SEG_S04,                       \
              .com[4] = SL_SEGMENT_LCD_COM_C07, .bit[4] = SL_SEGMENT_LCD_SEG_S03,                       \
              .com[5] = SL_SEGMENT_LCD_COM_C07, .bit[5] = SL_SEGMENT_LCD_SEG_S02,                       \
              .com[6] = SL_SEGMENT_LCD_COM_C07, .bit[6] = SL_SEGMENT_LCD_SEG_S01,                       \
              .com[7] = SL_SEGMENT_LCD_COM_C07, .bit[7] = SL_SEGMENT_LCD_SEG_S00,                       \
            },                                                                                          \
            .battery     = {                                                                            \
              .com[0] = SL_SEGMENT_LCD_COM_C07, .bit[0] = SL_SEGMENT_LCD_SEG_S13,                       \
              .com[1] = SL_SEGMENT_LCD_COM_C07, .bit[1] = SL_SEGMENT_LCD_SEG_S17,                       \
              .com[2] = SL_SEGMENT_LCD_COM_C07, .bit[2] = SL_SEGMENT_LCD_SEG_S16,                       \
              .com[3] = SL_SEGMENT_LCD_COM_C07, .bit[3] = SL_SEGMENT_LCD_SEG_S18,                       \
            }

#define SL_SEGMENT_LCD_EFM_DISPLAY_TOP_BLOCK    \
          .top_blocks   = {                                                                                                                                     \
                { /* 1 */                                                                                                                                       \
                  .com[0] = SL_SEGMENT_LCD_COM_C06, .com[1] = SL_SEGMENT_LCD_COM_C06, .com[2] = SL_SEGMENT_LCD_COM_C04, .com[3] = SL_SEGMENT_LCD_COM_C03,       \
                  .bit[0] = SL_SEGMENT_LCD_SEG_S01, .bit[1] = SL_SEGMENT_LCD_SEG_S02, .bit[2] = SL_SEGMENT_LCD_SEG_S01, .bit[3] = SL_SEGMENT_LCD_SEG_S01,       \
                  .com[4] = SL_SEGMENT_LCD_COM_C03, .com[5] = SL_SEGMENT_LCD_COM_C05, .com[6] = SL_SEGMENT_LCD_COM_C04, .com[7] = SL_SEGMENT_LCD_COM_C05,       \
                  .bit[4] = SL_SEGMENT_LCD_SEG_S02, .bit[5] = SL_SEGMENT_LCD_SEG_S01, .bit[6] = SL_SEGMENT_LCD_SEG_S02, .bit[7] = SL_SEGMENT_LCD_SEG_S02        \
                },                                                                                                                                              \
                { /* 2 */                                                                                                                                       \
                  .com[0] = SL_SEGMENT_LCD_COM_C06, .com[1] = SL_SEGMENT_LCD_COM_C06, .com[2] = SL_SEGMENT_LCD_COM_C04, .com[3] = SL_SEGMENT_LCD_COM_C03,       \
                  .bit[0] = SL_SEGMENT_LCD_SEG_S03, .bit[1] = SL_SEGMENT_LCD_SEG_S04, .bit[2] = SL_SEGMENT_LCD_SEG_S03, .bit[3] = SL_SEGMENT_LCD_SEG_S03,       \
                  .com[4] = SL_SEGMENT_LCD_COM_C03, .com[5] = SL_SEGMENT_LCD_COM_C05, .com[6] = SL_SEGMENT_LCD_COM_C04, .com[7] = SL_SEGMENT_LCD_COM_C05,       \
                  .bit[4] = SL_SEGMENT_LCD_SEG_S04, .bit[5] = SL_SEGMENT_LCD_SEG_S03, .bit[6] = SL_SEGMENT_LCD_SEG_S04, .bit[7] = SL_SEGMENT_LCD_SEG_S04        \
                },                                                                                                                                              \
                { /* 3 */                                                                                                                                       \
                  .com[0] = SL_SEGMENT_LCD_COM_C06, .com[1] = SL_SEGMENT_LCD_COM_C06, .com[2] = SL_SEGMENT_LCD_COM_C04, .com[3] = SL_SEGMENT_LCD_COM_C03,       \
                  .bit[0] = SL_SEGMENT_LCD_SEG_S05, .bit[1] = SL_SEGMENT_LCD_SEG_S06, .bit[2] = SL_SEGMENT_LCD_SEG_S05, .bit[3] = SL_SEGMENT_LCD_SEG_S05,       \
                  .com[4] = SL_SEGMENT_LCD_COM_C03, .com[5] = SL_SEGMENT_LCD_COM_C05, .com[6] = SL_SEGMENT_LCD_COM_C04, .com[7] = SL_SEGMENT_LCD_COM_C05,       \
                  .bit[4] = SL_SEGMENT_LCD_SEG_S06, .bit[5] = SL_SEGMENT_LCD_SEG_S05, .bit[6] = SL_SEGMENT_LCD_SEG_S06, .bit[7] = SL_SEGMENT_LCD_SEG_S06        \
                },                                                                                                                                              \
                { /* 4 */                                                                                                                                       \
                  .com[0] = SL_SEGMENT_LCD_COM_C06, .com[1] = SL_SEGMENT_LCD_COM_C06, .com[2] = SL_SEGMENT_LCD_COM_C04, .com[3] = SL_SEGMENT_LCD_COM_C03,       \
                  .bit[0] = SL_SEGMENT_LCD_SEG_S07, .bit[1] = SL_SEGMENT_LCD_SEG_S08, .bit[2] = SL_SEGMENT_LCD_SEG_S07, .bit[3] = SL_SEGMENT_LCD_SEG_S07,       \
                  .com[4] = SL_SEGMENT_LCD_COM_C03, .com[5] = SL_SEGMENT_LCD_COM_C05, .com[6] = SL_SEGMENT_LCD_COM_C04, .com[7] = SL_SEGMENT_LCD_COM_C05,       \
                  .bit[4] = SL_SEGMENT_LCD_SEG_S08, .bit[5] = SL_SEGMENT_LCD_SEG_S07, .bit[6] = SL_SEGMENT_LCD_SEG_S08, .bit[7] = SL_SEGMENT_LCD_SEG_S08        \
                },                                                                                                                                              \
                { /* 5 */                                                                                                                                       \
                  .com[0] = SL_SEGMENT_LCD_COM_C07, .com[1] = SL_SEGMENT_LCD_COM_C06, .com[2] = SL_SEGMENT_LCD_COM_C05, .com[3] = SL_SEGMENT_LCD_COM_C04,       \
                  .bit[0] = SL_SEGMENT_LCD_SEG_S09, .bit[1] = SL_SEGMENT_LCD_SEG_S10, .bit[2] = SL_SEGMENT_LCD_SEG_S09, .bit[3] = SL_SEGMENT_LCD_SEG_S09,       \
                  .com[4] = SL_SEGMENT_LCD_COM_C03, .com[5] = SL_SEGMENT_LCD_COM_C06, .com[6] = SL_SEGMENT_LCD_COM_C04, .com[7] = SL_SEGMENT_LCD_COM_C05,       \
                  .bit[4] = SL_SEGMENT_LCD_SEG_S10, .bit[5] = SL_SEGMENT_LCD_SEG_S09, .bit[6] = SL_SEGMENT_LCD_SEG_S10, .bit[7] = SL_SEGMENT_LCD_SEG_S10        \
                },                                                                                                                                              \
                { /* 6 */                                                                                                                                       \
                  .com[0] = SL_SEGMENT_LCD_COM_C07, .com[1] = SL_SEGMENT_LCD_COM_C06, .com[2] = SL_SEGMENT_LCD_COM_C05, .com[3] = SL_SEGMENT_LCD_COM_C04,       \
                  .bit[0] = SL_SEGMENT_LCD_SEG_S11, .bit[1] = SL_SEGMENT_LCD_SEG_S12, .bit[2] = SL_SEGMENT_LCD_SEG_S11, .bit[3] = SL_SEGMENT_LCD_SEG_S11,       \
                  .com[4] = SL_SEGMENT_LCD_COM_C03, .com[5] = SL_SEGMENT_LCD_COM_C06, .com[6] = SL_SEGMENT_LCD_COM_C04, .com[7] = SL_SEGMENT_LCD_COM_C05,       \
                  .bit[4] = SL_SEGMENT_LCD_SEG_S12, .bit[5] = SL_SEGMENT_LCD_SEG_S11, .bit[6] = SL_SEGMENT_LCD_SEG_S12, .bit[7] = SL_SEGMENT_LCD_SEG_S12        \
                },                                                                                                                                              \
                { /* 7 */                                                                                                                                       \
                  .com[0] = SL_SEGMENT_LCD_COM_C06, .com[1] = SL_SEGMENT_LCD_COM_C06, .com[2] = SL_SEGMENT_LCD_COM_C04, .com[3] = SL_SEGMENT_LCD_COM_C03,       \
                  .bit[0] = SL_SEGMENT_LCD_SEG_S13, .bit[1] = SL_SEGMENT_LCD_SEG_S14, .bit[2] = SL_SEGMENT_LCD_SEG_S13, .bit[3] = SL_SEGMENT_LCD_SEG_S13,       \
                  .com[4] = SL_SEGMENT_LCD_COM_C03, .com[5] = SL_SEGMENT_LCD_COM_C05, .com[6] = SL_SEGMENT_LCD_COM_C04, .com[7] = SL_SEGMENT_LCD_COM_C05,       \
                  .bit[4] = SL_SEGMENT_LCD_SEG_S14, .bit[5] = SL_SEGMENT_LCD_SEG_S13, .bit[6] = SL_SEGMENT_LCD_SEG_S14, .bit[7] = SL_SEGMENT_LCD_SEG_S14        \
                },                                                                                                                                              \
              }

#define SL_SEGMENT_LCD_EFM_DISPLAY_BOT_BLOCK    \
          .bot_blocks   = {                                                                                                                                     \
                { /* 1 */                                                                                                                                       \
                  .com[0] = SL_SEGMENT_LCD_COM_C02, .com[1] = SL_SEGMENT_LCD_COM_C00, .com[2] = SL_SEGMENT_LCD_COM_C00, .com[3] = SL_SEGMENT_LCD_COM_C03,       \
                  .bit[0] = SL_SEGMENT_LCD_SEG_S02, .bit[1] = SL_SEGMENT_LCD_SEG_S02, .bit[2] = SL_SEGMENT_LCD_SEG_S01, .bit[3] = SL_SEGMENT_LCD_SEG_S01,       \
                  .com[4] = SL_SEGMENT_LCD_COM_C03, .com[5] = SL_SEGMENT_LCD_COM_C01, .com[6] = SL_SEGMENT_LCD_COM_C02, .com[7] = SL_SEGMENT_LCD_COM_C01,       \
                  .bit[4] = SL_SEGMENT_LCD_SEG_S02, .bit[5] = SL_SEGMENT_LCD_SEG_S02, .bit[6] = SL_SEGMENT_LCD_SEG_S01, .bit[7] = SL_SEGMENT_LCD_SEG_S01        \
                },                                                                                                                                              \
                { /* 2 */                                                                                                                                       \
                  .com[0] = SL_SEGMENT_LCD_COM_C02, .com[1] = SL_SEGMENT_LCD_COM_C00, .com[2] = SL_SEGMENT_LCD_COM_C00, .com[3] = SL_SEGMENT_LCD_COM_C03,       \
                  .bit[0] = SL_SEGMENT_LCD_SEG_S04, .bit[1] = SL_SEGMENT_LCD_SEG_S04, .bit[2] = SL_SEGMENT_LCD_SEG_S03, .bit[3] = SL_SEGMENT_LCD_SEG_S03,       \
                  .com[4] = SL_SEGMENT_LCD_COM_C03, .com[5] = SL_SEGMENT_LCD_COM_C01, .com[6] = SL_SEGMENT_LCD_COM_C02, .com[7] = SL_SEGMENT_LCD_COM_C01,       \
                  .bit[4] = SL_SEGMENT_LCD_SEG_S04, .bit[5] = SL_SEGMENT_LCD_SEG_S04, .bit[6] = SL_SEGMENT_LCD_SEG_S03, .bit[7] = SL_SEGMENT_LCD_SEG_S03        \
                },                                                                                                                                              \
                { /* 3 */                                                                                                                                       \
                  .com[0] = SL_SEGMENT_LCD_COM_C02, .com[1] = SL_SEGMENT_LCD_COM_C00, .com[2] = SL_SEGMENT_LCD_COM_C00, .com[3] = SL_SEGMENT_LCD_COM_C03,       \
                  .bit[0] = SL_SEGMENT_LCD_SEG_S06, .bit[1] = SL_SEGMENT_LCD_SEG_S06, .bit[2] = SL_SEGMENT_LCD_SEG_S05, .bit[3] = SL_SEGMENT_LCD_SEG_S05,       \
                  .com[4] = SL_SEGMENT_LCD_COM_C03, .com[5] = SL_SEGMENT_LCD_COM_C01, .com[6] = SL_SEGMENT_LCD_COM_C02, .com[7] = SL_SEGMENT_LCD_COM_C01,       \
                  .bit[4] = SL_SEGMENT_LCD_SEG_S06, .bit[5] = SL_SEGMENT_LCD_SEG_S06, .bit[6] = SL_SEGMENT_LCD_SEG_S05, .bit[7] = SL_SEGMENT_LCD_SEG_S05        \
                },                                                                                                                                              \
                { /* 4 */                                                                                                                                       \
                  .com[0] = SL_SEGMENT_LCD_COM_C02, .com[1] = SL_SEGMENT_LCD_COM_C00, .com[2] = SL_SEGMENT_LCD_COM_C00, .com[3] = SL_SEGMENT_LCD_COM_C03,       \
                  .bit[0] = SL_SEGMENT_LCD_SEG_S08, .bit[1] = SL_SEGMENT_LCD_SEG_S08, .bit[2] = SL_SEGMENT_LCD_SEG_S07, .bit[3] = SL_SEGMENT_LCD_SEG_S07,       \
                  .com[4] = SL_SEGMENT_LCD_COM_C03, .com[5] = SL_SEGMENT_LCD_COM_C01, .com[6] = SL_SEGMENT_LCD_COM_C02, .com[7] = SL_SEGMENT_LCD_COM_C01,       \
                  .bit[4] = SL_SEGMENT_LCD_SEG_S08, .bit[5] = SL_SEGMENT_LCD_SEG_S08, .bit[6] = SL_SEGMENT_LCD_SEG_S07, .bit[7] = SL_SEGMENT_LCD_SEG_S07        \
                },                                                                                                                                              \
                { /* 5 */                                                                                                                                       \
                  .com[0] = SL_SEGMENT_LCD_COM_C02, .com[1] = SL_SEGMENT_LCD_COM_C00, .com[2] = SL_SEGMENT_LCD_COM_C01, .com[3] = SL_SEGMENT_LCD_COM_C04,       \
                  .bit[0] = SL_SEGMENT_LCD_SEG_S10, .bit[1] = SL_SEGMENT_LCD_SEG_S10, .bit[2] = SL_SEGMENT_LCD_SEG_S09, .bit[3] = SL_SEGMENT_LCD_SEG_S09,       \
                  .com[4] = SL_SEGMENT_LCD_COM_C03, .com[5] = SL_SEGMENT_LCD_COM_C01, .com[6] = SL_SEGMENT_LCD_COM_C03, .com[7] = SL_SEGMENT_LCD_COM_C02,       \
                  .bit[4] = SL_SEGMENT_LCD_SEG_S10, .bit[5] = SL_SEGMENT_LCD_SEG_S10, .bit[6] = SL_SEGMENT_LCD_SEG_S09, .bit[7] = SL_SEGMENT_LCD_SEG_S09        \
                },                                                                                                                                              \
                { /* 6 */                                                                                                                                       \
                  .com[0] = SL_SEGMENT_LCD_COM_C02, .com[1] = SL_SEGMENT_LCD_COM_C00, .com[2] = SL_SEGMENT_LCD_COM_C01, .com[3] = SL_SEGMENT_LCD_COM_C04,       \
                  .bit[0] = SL_SEGMENT_LCD_SEG_S12, .bit[1] = SL_SEGMENT_LCD_SEG_S12, .bit[2] = SL_SEGMENT_LCD_SEG_S11, .bit[3] = SL_SEGMENT_LCD_SEG_S11,       \
                  .com[4] = SL_SEGMENT_LCD_COM_C03, .com[5] = SL_SEGMENT_LCD_COM_C01, .com[6] = SL_SEGMENT_LCD_COM_C03, .com[7] = SL_SEGMENT_LCD_COM_C02,       \
                  .bit[4] = SL_SEGMENT_LCD_SEG_S12, .bit[5] = SL_SEGMENT_LCD_SEG_S12, .bit[6] = SL_SEGMENT_LCD_SEG_S11, .bit[7] = SL_SEGMENT_LCD_SEG_S11        \
                },                                                                                                                                              \
                { /* 7 */                                                                                                                                       \
                  .com[0] = SL_SEGMENT_LCD_COM_C02, .com[1] = SL_SEGMENT_LCD_COM_C00, .com[2] = SL_SEGMENT_LCD_COM_C00, .com[3] = SL_SEGMENT_LCD_COM_C03,       \
                  .bit[0] = SL_SEGMENT_LCD_SEG_S14, .bit[1] = SL_SEGMENT_LCD_SEG_S14, .bit[2] = SL_SEGMENT_LCD_SEG_S13, .bit[3] = SL_SEGMENT_LCD_SEG_S13,       \
                  .com[4] = SL_SEGMENT_LCD_COM_C03, .com[5] = SL_SEGMENT_LCD_COM_C01, .com[6] = SL_SEGMENT_LCD_COM_C02, .com[7] = SL_SEGMENT_LCD_COM_C01,       \
                  .bit[4] = SL_SEGMENT_LCD_SEG_S14, .bit[5] = SL_SEGMENT_LCD_SEG_S14, .bit[6] = SL_SEGMENT_LCD_SEG_S13, .bit[7] = SL_SEGMENT_LCD_SEG_S13        \
                },                                                                                                                                              \
              }

#define SL_SEGMENT_LCD_EFM_DISPLAY_DEF {          \
            SL_SEGMENT_LCD_EFM_DISPLAY_TEXT,      \
            SL_SEGMENT_LCD_EFM_DISPLAY_NUM,       \
            SL_SEGMENT_LCD_EFM_DISPLAY_SYMBOL,    \
            SL_SEGMENT_LCD_EFM_DISPLAY_TOP_BLOCK, \
            SL_SEGMENT_LCD_EFM_DISPLAY_BOT_BLOCK, \
          }

// Utility Macros
#define SL_SEGMENT_LCD_ALL_SEG_BITMASK ( \
  (1 << SL_SEGMENT_LCD_SEG_S00) | \
  (1 << SL_SEGMENT_LCD_SEG_S01) | \
  (1 << SL_SEGMENT_LCD_SEG_S02) | \
  (1 << SL_SEGMENT_LCD_SEG_S03) | \
  (1 << SL_SEGMENT_LCD_SEG_S04) | \
  (1 << SL_SEGMENT_LCD_SEG_S05) | \
  (1 << SL_SEGMENT_LCD_SEG_S06) | \
  (1 << SL_SEGMENT_LCD_SEG_S07) | \
  (1 << SL_SEGMENT_LCD_SEG_S08) | \
  (1 << SL_SEGMENT_LCD_SEG_S09) | \
  (1 << SL_SEGMENT_LCD_SEG_S10) | \
  (1 << SL_SEGMENT_LCD_SEG_S11) | \
  (1 << SL_SEGMENT_LCD_SEG_S12) | \
  (1 << SL_SEGMENT_LCD_SEG_S13) | \
  (1 << SL_SEGMENT_LCD_SEG_S14) | \
  (1 << SL_SEGMENT_LCD_SEG_S15) | \
  (1 << SL_SEGMENT_LCD_SEG_S16) | \
  (1 << SL_SEGMENT_LCD_SEG_S17) | \
  (1 << SL_SEGMENT_LCD_SEG_S18) | \
  (1 << SL_SEGMENT_LCD_SEG_S19))

#define SL_LCD_ALL_SEG_NUM_BITMASK ( \
  (1 << SL_SEGMENT_LCD_SEG_S15) | \
  (1 << SL_SEGMENT_LCD_SEG_S16) | \
  (1 << SL_SEGMENT_LCD_SEG_S17) | \
  (1 << SL_SEGMENT_LCD_SEG_S18))

#define SL_LCD_ALL_SEG_ALPHA_BITMASK ( \
  (1 << SL_SEGMENT_LCD_SEG_S01) | \
  (1 << SL_SEGMENT_LCD_SEG_S02) | \
  (1 << SL_SEGMENT_LCD_SEG_S03) | \
  (1 << SL_SEGMENT_LCD_SEG_S04) | \
  (1 << SL_SEGMENT_LCD_SEG_S05) | \
  (1 << SL_SEGMENT_LCD_SEG_S06) | \
  (1 << SL_SEGMENT_LCD_SEG_S07) | \
  (1 << SL_SEGMENT_LCD_SEG_S08) | \
  (1 << SL_SEGMENT_LCD_SEG_S09) | \
  (1 << SL_SEGMENT_LCD_SEG_S10) | \
  (1 << SL_SEGMENT_LCD_SEG_S11) | \
  (1 << SL_SEGMENT_LCD_SEG_S12) | \
  (1 << SL_SEGMENT_LCD_SEG_S13) | \
  (1 << SL_SEGMENT_LCD_SEG_S14))

//  LCD_NUMBER_OFF
#define SL_SEGMENT_LCD_NUMBER_OFF()                               \
  do {                                                 \
    sl_hal_lcd_segment_set_low(0, SL_LCD_ALL_SEG_NUM_BITMASK, 0);  \
    sl_hal_lcd_segment_set_low(1, SL_LCD_ALL_SEG_NUM_BITMASK, 0);  \
    sl_hal_lcd_segment_set_low(2, SL_LCD_ALL_SEG_NUM_BITMASK, 0);  \
    sl_hal_lcd_segment_set_low(3, SL_LCD_ALL_SEG_NUM_BITMASK, 0);  \
    sl_hal_lcd_segment_set_low(4, SL_LCD_ALL_SEG_NUM_BITMASK, 0);  \
    sl_hal_lcd_segment_set_low(5, SL_LCD_ALL_SEG_NUM_BITMASK, 0);  \
    sl_hal_lcd_segment_set_low(6, SL_LCD_ALL_SEG_NUM_BITMASK, 0);  \
  } while (0)

//  LCD_ALPHA_NUMBER_OFF
#define SL_LCD_ALPHA_NUMBER_OFF()                                \
  do {                                                        \
    sl_hal_lcd_segment_set_low(0, SL_LCD_ALL_SEG_ALPHA_BITMASK &          \
                      (~((1 << SL_SEGMENT_LCD_SEG_S09)|(1 << SL_SEGMENT_LCD_SEG_S11))), 0); \
    sl_hal_lcd_segment_set_low(1, SL_LCD_ALL_SEG_ALPHA_BITMASK, 0);       \
    sl_hal_lcd_segment_set_low(2, SL_LCD_ALL_SEG_ALPHA_BITMASK, 0);       \
    sl_hal_lcd_segment_set_low(3, SL_LCD_ALL_SEG_ALPHA_BITMASK, 0);       \
    sl_hal_lcd_segment_set_low(4, SL_LCD_ALL_SEG_ALPHA_BITMASK, 0);       \
    sl_hal_lcd_segment_set_low(5, SL_LCD_ALL_SEG_ALPHA_BITMASK, 0);       \
    sl_hal_lcd_segment_set_low(6, SL_LCD_ALL_SEG_ALPHA_BITMASK, 0);       \
    sl_hal_lcd_segment_set_low(7, ((1 << SL_SEGMENT_LCD_SEG_S09)|(1 << SL_SEGMENT_LCD_SEG_S11)), 0); \
  } while (0)

#define SL_SEGMENT_LCD_ALL_SEGMENTS_OFF()                     \
  do {                                             \
    sl_hal_lcd_segment_set_low(0, SL_SEGMENT_LCD_ALL_SEG_BITMASK, 0);  \
    sl_hal_lcd_segment_set_low(1, SL_SEGMENT_LCD_ALL_SEG_BITMASK, 0);  \
    sl_hal_lcd_segment_set_low(2, SL_SEGMENT_LCD_ALL_SEG_BITMASK, 0);  \
    sl_hal_lcd_segment_set_low(3, SL_SEGMENT_LCD_ALL_SEG_BITMASK, 0);  \
    sl_hal_lcd_segment_set_low(4, SL_SEGMENT_LCD_ALL_SEG_BITMASK, 0);  \
    sl_hal_lcd_segment_set_low(5, SL_SEGMENT_LCD_ALL_SEG_BITMASK, 0);  \
    sl_hal_lcd_segment_set_low(6, SL_SEGMENT_LCD_ALL_SEG_BITMASK, 0);  \
    sl_hal_lcd_segment_set_low(7, SL_SEGMENT_LCD_ALL_SEG_BITMASK, 0);  \
  } while (0)

#define SL_SEGMENT_LCD_ALL_SEGMENTS_ON()                                    \
  do {                                                           \
    sl_hal_lcd_segment_set_low(0, SL_SEGMENT_LCD_ALL_SEG_BITMASK, SL_SEGMENT_LCD_ALL_SEG_BITMASK);  \
    sl_hal_lcd_segment_set_low(1, SL_SEGMENT_LCD_ALL_SEG_BITMASK, SL_SEGMENT_LCD_ALL_SEG_BITMASK);  \
    sl_hal_lcd_segment_set_low(2, SL_SEGMENT_LCD_ALL_SEG_BITMASK, SL_SEGMENT_LCD_ALL_SEG_BITMASK);  \
    sl_hal_lcd_segment_set_low(3, SL_SEGMENT_LCD_ALL_SEG_BITMASK, SL_SEGMENT_LCD_ALL_SEG_BITMASK);  \
    sl_hal_lcd_segment_set_low(4, SL_SEGMENT_LCD_ALL_SEG_BITMASK, SL_SEGMENT_LCD_ALL_SEG_BITMASK);  \
    sl_hal_lcd_segment_set_low(5, SL_SEGMENT_LCD_ALL_SEG_BITMASK, SL_SEGMENT_LCD_ALL_SEG_BITMASK);  \
    sl_hal_lcd_segment_set_low(6, SL_SEGMENT_LCD_ALL_SEG_BITMASK, SL_SEGMENT_LCD_ALL_SEG_BITMASK);  \
    sl_hal_lcd_segment_set_low(7, SL_SEGMENT_LCD_ALL_SEG_BITMASK, SL_SEGMENT_LCD_ALL_SEG_BITMASK);  \
  } while (0)

#define SL_SEGMENT_LCD_SEGMENTS_ENABLE()         \
  do {                                \
    sl_hal_lcd_enable_com_line(SL_SEGMENT_LCD_COM_C00);     \
    sl_hal_lcd_enable_com_line(SL_SEGMENT_LCD_COM_C01);     \
    sl_hal_lcd_enable_com_line(SL_SEGMENT_LCD_COM_C02);     \
    sl_hal_lcd_enable_com_line(SL_SEGMENT_LCD_COM_C03);     \
    sl_hal_lcd_enable_com_line(SL_SEGMENT_LCD_COM_C04);     \
    sl_hal_lcd_enable_com_line(SL_SEGMENT_LCD_COM_C05);     \
    sl_hal_lcd_enable_com_line(SL_SEGMENT_LCD_COM_C06);     \
    sl_hal_lcd_enable_com_line(SL_SEGMENT_LCD_COM_C07);     \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S00);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S01);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S02);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S03);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S04);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S05);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S06);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S07);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S08);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S09);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S10);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S11);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S12);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S13);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S14);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S15);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S16);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S17);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S18);      \
    sl_hal_lcd_segment_enable(SL_SEGMENT_LCD_SEG_S19);      \
  } while (0)

#define SL_LCD_SEGMENTS_ALPHA_DIS()      \
  do {                                \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S00); \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S01); \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S02); \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S03); \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S04); \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S05); \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S06); \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S07); \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S08); \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S09); \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S10); \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S11); \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S12); \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S13); \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S14); \
  } while (0)

#define SL_LCD_SEGMENTS_NUM_DIS()         \
  do {                                 \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S15); \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S16); \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S17); \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S18); \
    sl_hal_lcd_segment_disable(SL_SEGMENT_LCD_SEG_S19); \
  } while (0)

#define SL_SEGMENT_LCD_DISPLAY_ENABLE() \
  do {                       \
    ;                        \
  } while (0)


#ifdef __cplusplus
}
#endif

#endif