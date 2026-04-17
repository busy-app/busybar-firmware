/***************************************************************************//**
 * @file
 * @brief Segment LCD Config for the LCD module CE322_1002
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
#define SL_SEGMENT_LCD_MODULE_CE322_1002

// <<< Use Configuration Wizard in Context Menu >>>

// <h>Frame Rate
// LCD Controller Prescaler (divide LCDCLK / 64)
// LCDCLK_pre = 512 Hz
// Set FDIV=1, means 512/2 = 256 Hz
// With quadruplex mode, 256/8 => 32 Hz Frame Rate

// <o SL_SEGMENT_LCD_CLK_PRE> LCD Controller prescaler
// <i> Default: 64
#define SL_SEGMENT_LCD_CLK_PRE         64

// <o SL_SEGMENT_LCD_FRAME_RATE_DIV> LCD frame rate divisor
// <i> Default: 1
#define SL_SEGMENT_LCD_FRAME_RATE_DIV   1
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
#define SL_SEGMENT_LCD_BOOST_CONTRAST      0x0F

// <o SL_SEGMENT_LCD_ENABLE_CONTROLLER> Enable at initialization
// <false=> Disabled
// <true=> Enabled
// <i> Default: true
#define SL_SEGMENT_LCD_ENABLE_CONTROLLER      true

// <o SL_SEGMENT_LCD_MUX_CONFIG> Mux configuration
// <SL_HAL_LCD_MUX_STATIC=> Static 
// <SL_HAL_LCD_MUX_DUPLEX=> Duplex (1/2 duty cycle)
// <SL_HAL_LCD_MUX_TRIPLEX=> Triplex (1/3 duty cycle)
// <SL_HAL_LCD_MUX_QUADRUPLEX=> Quadruplex (1/4 duty cycle)
// <SL_HAL_LCD_MUX_SEXTAPLEX=> Sextaplex (1/6 duty cycle)
// <SL_HAL_LCD_MUX_OCTAPLEX=> Octaplex (1/8 duty cycle)
// <i> Default: SL_HAL_LCD_MUX_OCTAPLEX
#define SL_SEGMENT_LCD_MUX_CONFIG             SL_HAL_LCD_MUX_QUADRUPLEX

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
// <i> Default: SL_HAL_LCD_BIAS_ONE_THIRD
#define SL_SEGMENT_LCD_BIAS_SETTING           SL_HAL_LCD_BIAS_ONE_THIRD

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

// Range of symbols available on display
typedef enum {
  SL_LCD_SYMBOL_P2, //!< SL_LCD_SYMBOL_P2
  SL_LCD_SYMBOL_P3, //!< SL_LCD_SYMBOL_P3
  SL_LCD_SYMBOL_P4, //!< SL_LCD_SYMBOL_P4
  SL_LCD_SYMBOL_P5, //!< SL_LCD_SYMBOL_P5
  SL_LCD_SYMBOL_P6, //!< SL_LCD_SYMBOL_P6
  SL_LCD_SYMBOL_DEGC//!< SL_LCD_SYMBOL_DEGC
} sl_segment_lcd_symbol_t;

#define SL_LCD_SYMBOL_P2_COM  SL_SEGMENT_LCD_COM_C00
#define SL_LCD_SYMBOL_P2_SEG  SL_SEGMENT_LCD_SEG_S03
#define SL_LCD_SYMBOL_P3_COM  SL_SEGMENT_LCD_COM_C00
#define SL_LCD_SYMBOL_P3_SEG  SL_SEGMENT_LCD_SEG_S05
#define SL_LCD_SYMBOL_P4_COM  SL_SEGMENT_LCD_COM_C00
#define SL_LCD_SYMBOL_P4_SEG  SL_SEGMENT_LCD_SEG_S08
#define SL_LCD_SYMBOL_P5_COM  SL_SEGMENT_LCD_COM_C03
#define SL_LCD_SYMBOL_P5_SEG  SL_SEGMENT_LCD_SEG_S07
#define SL_LCD_SYMBOL_P6_COM  SL_SEGMENT_LCD_COM_C03
#define SL_LCD_SYMBOL_P6_SEG  SL_SEGMENT_LCD_SEG_S01
#define SL_LCD_SYMBOL_DEGC_COM  SL_SEGMENT_LCD_COM_C03
#define SL_LCD_SYMBOL_DEGC_SEG  SL_SEGMENT_LCD_SEG_S07

// Multiplexing table: See board schematic for more details
#define SL_SEGMENT_LCD_EFM_DISPLAY_DEF {                                    \
    .number      = {                                                        \
      { /* #5 location : Segments 5A/5B/5C/5D/5E/5F/5G */                   \
        .com[0] = SL_SEGMENT_LCD_COM_C03, .com[1] = SL_SEGMENT_LCD_COM_C02, \
        .com[2] = SL_SEGMENT_LCD_COM_C01, .com[3] = SL_SEGMENT_LCD_COM_C00, \
        .bit[0] = SL_SEGMENT_LCD_SEG_S09, .bit[1] = SL_SEGMENT_LCD_SEG_S09, \
        .bit[2] = SL_SEGMENT_LCD_SEG_S09, .bit[3] = SL_SEGMENT_LCD_SEG_S09, \
        .com[4] = SL_SEGMENT_LCD_COM_C01, .com[5] = SL_SEGMENT_LCD_COM_C03, \
        .com[6] = SL_SEGMENT_LCD_COM_C02,                                   \
        .bit[4] = SL_SEGMENT_LCD_SEG_S08, .bit[5] = SL_SEGMENT_LCD_SEG_S08, \
        .bit[6] = SL_SEGMENT_LCD_SEG_S08,                                   \
      },                                                                    \
      { /* #4 location : Segments 4A/4B/4C/4D/4E/4F/4G */                   \
        .com[0] = SL_SEGMENT_LCD_COM_C03, .com[1] = SL_SEGMENT_LCD_COM_C02, \
        .com[2] = SL_SEGMENT_LCD_COM_C00, .com[3] = SL_SEGMENT_LCD_COM_C00, \
        .bit[0] = SL_SEGMENT_LCD_SEG_S06, .bit[1] = SL_SEGMENT_LCD_SEG_S07, \
        .bit[2] = SL_SEGMENT_LCD_SEG_S07, .bit[3] = SL_SEGMENT_LCD_SEG_S06, \
        .com[4] = SL_SEGMENT_LCD_COM_C01, .com[5] = SL_SEGMENT_LCD_COM_C02, \
        .com[6] = SL_SEGMENT_LCD_COM_C01,                                   \
        .bit[4] = SL_SEGMENT_LCD_SEG_S06, .bit[5] = SL_SEGMENT_LCD_SEG_S06, \
        .bit[6] = SL_SEGMENT_LCD_SEG_S07,                                   \
      },                                                                    \
      { /* #3 location : Segments 3A/3B/3C/3D/3E/3F/3G */                   \
        .com[0] = SL_SEGMENT_LCD_COM_C03, .com[1] = SL_SEGMENT_LCD_COM_C03, \
        .com[2] = SL_SEGMENT_LCD_COM_C01, .com[3] = SL_SEGMENT_LCD_COM_C00, \
        .bit[0] = SL_SEGMENT_LCD_SEG_S04, .bit[1] = SL_SEGMENT_LCD_SEG_S05, \
        .bit[2] = SL_SEGMENT_LCD_SEG_S05, .bit[3] = SL_SEGMENT_LCD_SEG_S04, \
        .com[4] = SL_SEGMENT_LCD_COM_C01, .com[5] = SL_SEGMENT_LCD_COM_C02, \
        .com[6] = SL_SEGMENT_LCD_COM_C02,                                   \
        .bit[4] = SL_SEGMENT_LCD_SEG_S04, .bit[5] = SL_SEGMENT_LCD_SEG_S04, \
        .bit[6] = SL_SEGMENT_LCD_SEG_S05,                                   \
      },                                                                    \
      { /* #2 location : Segments 2A/2B/2C/2D/2E/2F/2G */                   \
        .com[0] = SL_SEGMENT_LCD_COM_C03, .com[1] = SL_SEGMENT_LCD_COM_C03, \
        .com[2] = SL_SEGMENT_LCD_COM_C01, .com[3] = SL_SEGMENT_LCD_COM_C00, \
        .bit[0] = SL_SEGMENT_LCD_SEG_S02, .bit[1] = SL_SEGMENT_LCD_SEG_S03, \
        .bit[2] = SL_SEGMENT_LCD_SEG_S03, .bit[3] = SL_SEGMENT_LCD_SEG_S02, \
        .com[4] = SL_SEGMENT_LCD_COM_C01, .com[5] = SL_SEGMENT_LCD_COM_C02, \
        .com[6] = SL_SEGMENT_LCD_COM_C02,                                   \
        .bit[4] = SL_SEGMENT_LCD_SEG_S02, .bit[5] = SL_SEGMENT_LCD_SEG_S02, \
        .bit[6] = SL_SEGMENT_LCD_SEG_S03,                                   \
      },                                                                    \
      { /* #1 location : Segments 1A/1B/1C/1D/1E/1F/1G */                   \
        .com[0] = SL_SEGMENT_LCD_COM_C03, .com[1] = SL_SEGMENT_LCD_COM_C02, \
        .com[2] = SL_SEGMENT_LCD_COM_C00, .com[3] = SL_SEGMENT_LCD_COM_C00, \
        .bit[0] = SL_SEGMENT_LCD_SEG_S00, .bit[1] = SL_SEGMENT_LCD_SEG_S01, \
        .bit[2] = SL_SEGMENT_LCD_SEG_S01, .bit[3] = SL_SEGMENT_LCD_SEG_S00, \
        .com[4] = SL_SEGMENT_LCD_COM_C01, .com[5] = SL_SEGMENT_LCD_COM_C02, \
        .com[6] = SL_SEGMENT_LCD_COM_C01,                                   \
        .bit[4] = SL_SEGMENT_LCD_SEG_S00, .bit[5] = SL_SEGMENT_LCD_SEG_S00, \
        .bit[6] = SL_SEGMENT_LCD_SEG_S01,                                   \
      }                                                                     \
    }                                                                       \
}

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
  (1 << SL_SEGMENT_LCD_SEG_S09))

// Utility Macros
#define SL_SEGMENT_LCD_NUMBER_OFF()                                     \
  do {                                                                  \
    sl_hal_lcd_segment_set_low(0, SL_SEGMENT_LCD_ALL_SEG_BITMASK &               \
                      (~( (1 << SL_SEGMENT_LCD_SEG_S03)|                \
                          (1 << SL_SEGMENT_LCD_SEG_S05)|                \
                      (1 << SL_SEGMENT_LCD_SEG_S08))), 0x0000);         \
    sl_hal_lcd_segment_set_low(1, SL_SEGMENT_LCD_ALL_SEG_BITMASK, 0x0000);       \
    sl_hal_lcd_segment_set_low(2, SL_SEGMENT_LCD_ALL_SEG_BITMASK, 0x0000);       \
    sl_hal_lcd_segment_set_low(3, SL_SEGMENT_LCD_ALL_SEG_BITMASK &               \
                      (~( (1 << SL_SEGMENT_LCD_SEG_S01)|                \
                          (1 << SL_SEGMENT_LCD_SEG_S07)))               \
                      , 0x0000);                                        \
  } while (0)

#define SL_SEGMENT_LCD_ALL_SEGMENTS_OFF()  \
  do {                                     \
    sl_hal_lcd_segment_set_low(0, 0xFFFFF, 0x0000); \
    sl_hal_lcd_segment_set_low(1, 0xFFFFF, 0x0000); \
    sl_hal_lcd_segment_set_low(2, 0xFFFFF, 0x0000); \
    sl_hal_lcd_segment_set_low(3, 0xFFFFF, 0x0000); \
  } while (0)

#define SL_SEGMENT_LCD_ALL_SEGMENTS_ON()      \
    do {                                      \
      sl_hal_lcd_segment_set_low(0, 0xFFFFF, 0xFFFFF); \
      sl_hal_lcd_segment_set_low(1, 0xFFFFF, 0xFFFFF); \
      sl_hal_lcd_segment_set_low(2, 0xFFFFF, 0xFFFFF); \
      sl_hal_lcd_segment_set_low(3, 0xFFFFF, 0xFFFFF); \
    } while (0)

#define SL_SEGMENT_LCD_SEGMENTS_ENABLE()             \
  do {                                               \
    sl_hal_lcd_enable_com_line(SL_SEGMENT_LCD_COM_C00);     \
    sl_hal_lcd_enable_com_line(SL_SEGMENT_LCD_COM_C01);     \
    sl_hal_lcd_enable_com_line(SL_SEGMENT_LCD_COM_C02);     \
    sl_hal_lcd_enable_com_line(SL_SEGMENT_LCD_COM_C03);     \
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
    ;                                                \
  } while (0)

#define SL_SEGMENT_LCD_DISPLAY_ENABLE() \
  do {                                  \
    ;                                   \
  } while (0)


#ifdef __cplusplus
}
#endif

#endif