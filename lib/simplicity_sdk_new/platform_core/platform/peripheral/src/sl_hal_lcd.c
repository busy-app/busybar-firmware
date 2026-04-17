/***************************************************************************//**
 * @file
 * @brief Liquid Crystal Display (LCD) Peripheral API
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

#include "sl_hal_lcd.h"
#if defined(LCD_COUNT) && (LCD_COUNT > 0)
#include "sl_assert.h"
#include "sl_hal_bus.h"
#include "sl_hal_gpio.h"

#include <stddef.h>

/*******************************************************************************
 *****************************   DEFINES   *************************************
 ******************************************************************************/
// Frame counter uses a maximum of 5 bits (FCTOP[5:0]).
#define LCD_FRAME_COUNTER_VAL_MAX  64

/*******************************************************************************
 **************************   LOCAL VARIABLES   ********************************
 ******************************************************************************/

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

extern __INLINE void sl_hal_lcd_wait_load_busy(void);
extern __INLINE void sl_hal_lcd_wait_ready(void);
extern __INLINE void sl_hal_lcd_enable(void);
extern __INLINE void sl_hal_lcd_disable(void);
extern __INLINE void sl_hal_lcd_reset(void);
extern __INLINE void sl_hal_lcd_animation_enable(void);
extern __INLINE void sl_hal_lcd_animation_disable(void);
extern __INLINE uint32_t sl_hal_lcd_animation_get_state(void);
extern __INLINE void sl_hal_lcd_blink_enable(void);
extern __INLINE void sl_hal_lcd_blink_disable(void);
extern __INLINE uint32_t sl_hal_lcd_blink_get_state(void);
extern __INLINE void sl_hal_lcd_enable_blank(void);
extern __INLINE void sl_hal_lcd_disable_blank(void);
extern __INLINE void sl_hal_lcd_enable_frame_counter(void);
extern __INLINE void sl_hal_lcd_disable_frame_counter(void);
extern __INLINE void sl_hal_lcd_enable_display_counter(void);
extern __INLINE void sl_hal_lcd_disable_display_counter(void);
extern __INLINE void sl_hal_lcd_start_sync(bool auto_load,
                                           sl_hal_lcd_load_address_t load_addr);
extern __INLINE void sl_hal_lcd_stop_sync(bool auto_load);
extern __INLINE uint32_t sl_hal_lcd_get_pending_interrupts(void);
extern __INLINE uint32_t sl_hal_lcd_get_enabled_pending_interrupts(void);
extern __INLINE void sl_hal_lcd_set_interrupts(uint32_t flags);
extern __INLINE void sl_hal_lcd_enable_interrupts(uint32_t flags);
extern __INLINE void sl_hal_lcd_disable_interrupts(uint32_t flags);
extern __INLINE void sl_hal_lcd_clear_interrupts(uint32_t flags);
extern __INLINE void sl_hal_lcd_enable_dsc(void);
extern __INLINE void sl_hal_lcd_disable_dsc(void);
extern __INLINE void sl_hal_lcd_set_dma_mode(sl_hal_lcd_dma_mode_t mode);
extern __INLINE void sl_hal_lcd_bias_set(sl_hal_lcd_bias_t bias);
extern __INLINE void sl_hal_lcd_set_contrast(uint8_t level);
extern __INLINE void sl_hal_lcd_set_charge_redistribution_cycle(uint8_t cycles);
extern __INLINE void sl_hal_lcd_set_mode(sl_hal_lcd_mode_t mode);

/***************************************************************************//**
 * Initialize the Liquid Crystal Display (LCD) controller.
 ******************************************************************************/
void sl_hal_lcd_init(const sl_hal_lcd_init_t *init)
{
  EFM_ASSERT(init != (void *) 0);

  uint32_t display_control  = LCD->DISPCTRL;

  // Disable the controller before reconfiguration.
  sl_hal_lcd_disable();

  // Initialize LCD registers to hardware reset state.
  sl_hal_lcd_reset();

  LCD->CTRL_CLR = _LCD_CTRL_PRESCALE_MASK;
  LCD->CTRL_SET = init->clock_prescaler << _LCD_CTRL_PRESCALE_SHIFT;

  // Configure the controller according to the initialization structure.
  display_control = (display_control & ~(_LCD_DISPCTRL_MUX_MASK
                                         | _LCD_DISPCTRL_BIAS_MASK
                                         | _LCD_DISPCTRL_WAVE_MASK
                                         | _LCD_DISPCTRL_CHGRDST_MASK))
                    | ((uint32_t)init->mux << _LCD_DISPCTRL_MUX_SHIFT)
                    | ((uint32_t)init->bias << _LCD_DISPCTRL_BIAS_SHIFT)
                    | ((uint32_t)init->wave << _LCD_DISPCTRL_WAVE_SHIFT)
                    | ((uint32_t)init->charge_redistribution << _LCD_DISPCTRL_CHGRDST_SHIFT);

  // Update the display controller.
  LCD->DISPCTRL  = display_control;

  sl_hal_lcd_set_mode(init->mode);
  LCD->FRAMERATE = init->frame_rate_divider;
  sl_hal_lcd_set_contrast(init->contrast_level);
}

/***************************************************************************//**
 * Initialize the LCD Frame Counter.
 ******************************************************************************/
void sl_hal_lcd_init_frame_counter(const sl_hal_lcd_frame_counter_init_t *init)
{
  EFM_ASSERT(init != (void *) 0);
  // Ensure LCD is disabled before writing to register.
  EFM_ASSERT(LCD->EN != LCD_EN_EN);
  // Verify that the FC Top Counter is within limits.
  EFM_ASSERT(init->top < LCD_FRAME_COUNTER_VAL_MAX);

  // Set the Blink and Animation Control Register.
  LCD->BACFG = (LCD->BACFG & ~(_LCD_BACFG_FCTOP_MASK
                               | _LCD_BACFG_FCPRESC_MASK))
               | ((uint32_t)init->top << _LCD_BACFG_FCTOP_SHIFT)
               | ((uint32_t)init->prescale << _LCD_BACFG_FCPRESC_SHIFT);
}

/***************************************************************************//**
 * Configure the LCD controller Animation feature.
 ******************************************************************************/
void sl_hal_lcd_init_animation(const sl_hal_lcd_animation_init_t *init)
{
  uint32_t bactrl = LCD->BACTRL;

  EFM_ASSERT(init != (void *) 0);

  // Set initial Animation Register Values.
  LCD->AREGA = init->A_reg;
  LCD->AREGB = init->B_reg;

  // Configure the Animation Shift and Logic.
  bactrl &= ~(_LCD_BACTRL_AREGASC_MASK
              | _LCD_BACTRL_AREGBSC_MASK
              | _LCD_BACTRL_ALOGSEL_MASK
              | _LCD_BACTRL_ALOC_MASK
              );

  bactrl |= (init->A_shift << _LCD_BACTRL_AREGASC_SHIFT);
  bactrl |= (init->B_shift << _LCD_BACTRL_AREGBSC_SHIFT);
  bactrl |= (init->animation_logic << _LCD_BACTRL_ALOGSEL_SHIFT);
  bactrl |= (init->start_segment << _LCD_BACTRL_ALOC_SHIFT);

  // Reconfigure.
  LCD->BACTRL = bactrl;
}

/***************************************************************************//**
 * Configure Update Control.
 ******************************************************************************/
void sl_hal_lcd_update_control(sl_hal_lcd_update_data_control_t method)
{
  EFM_ASSERT(LCD->EN != LCD_EN_EN);
  LCD->CTRL = (LCD->CTRL & ~_LCD_CTRL_UDCTRL_MASK) | (method << _LCD_CTRL_UDCTRL_SHIFT);
}

/***************************************************************************//**
 * Enables a given LCD segment line.
 ******************************************************************************/
void sl_hal_lcd_segment_enable(uint32_t seg_nbr)
{
  // xG23 support 20 segment lines.
  // xG26 supports up to 40 segment lines
  // xG28 supports up to 28 segment lines.
  EFM_ASSERT(seg_nbr < (uint32_t)SL_HAL_LCD_SEGMENT_LINES_MAX);

#if defined(_GPIO_LCDSEGH_MASK)
  if (seg_nbr > 31) {
    GPIO->LCDSEGH_SET = 1 << (seg_nbr - 32);
  } else {
    GPIO->LCDSEGL_SET = 1 << (seg_nbr);
  }
#else
  GPIO->LCDSEG_SET = 1 << seg_nbr;
#endif
}

/***************************************************************************//**
 * Disable a given LCD segment line.
 ******************************************************************************/
void sl_hal_lcd_segment_disable(uint32_t seg_nbr)
{
  // xG23 support 20 segment lines.
  // xG26 supports up to 40 segment lines
  // xG28 supports up to 28 segment lines.
  EFM_ASSERT(seg_nbr < (uint32_t)SL_HAL_LCD_SEGMENT_LINES_MAX);

#if defined(_GPIO_LCDSEGH_MASK)
  if (seg_nbr > 31) {
    GPIO->LCDSEGH_CLR = 1 << (seg_nbr - 32);
  } else {
    GPIO->LCDSEGL_CLR = 1 << (seg_nbr);
  }
#else
  GPIO->LCDSEG_CLR = 1 << seg_nbr;
#endif
}

/***************************************************************************//**
 * Turn on or clear a segment.
 ******************************************************************************/
void sl_hal_lcd_segment_set(uint8_t com,
                            int bit,
                            bool enable)
{
  // Series 2 parts support up to 4 COM lines except for xG26 and xG28, which supports up to 8 COM lines.
  EFM_ASSERT(com < (int)SL_HAL_LCD_COM_LINES_MAX);

  // Series 2 parts support up to 20 segment lines.
  // Except for xG26 which supports up to 40 segment lines. and xG28 which supports up to 28 segment lines.
  EFM_ASSERT(bit < (int)SL_HAL_LCD_SEGMENT_LINES_MAX);

  // Use a bitband access for atomic bit set/clear of the segment.
  switch (com) {
    case 0:
#if defined(_LCD_SEGD0H_MASK)
      if (bit < 32) {
        sl_hal_bus_reg_write_bit(&(LCD->SEGD0), bit, enable);
      } else {
        bit -= 32;
        sl_hal_bus_reg_write_bit(&(LCD->SEGD0H), bit, enable);
      }
#else
      sl_hal_bus_reg_write_bit(&(LCD->SEGD0), bit, enable);
#endif
      break;

    case 1:
#if defined(_LCD_SEGD1H_MASK)
      if (bit < 32) {
        sl_hal_bus_reg_write_bit(&(LCD->SEGD1), bit, enable);
      } else {
        bit -= 32;
        sl_hal_bus_reg_write_bit(&(LCD->SEGD1H), bit, enable);
      }
#else
      sl_hal_bus_reg_write_bit(&(LCD->SEGD1), bit, enable);
#endif
      break;

    case 2:
#if defined(_LCD_SEGD2H_MASK)
      if (bit < 32) {
        sl_hal_bus_reg_write_bit(&(LCD->SEGD2), bit, enable);
      } else {
        bit -= 32;
        sl_hal_bus_reg_write_bit(&(LCD->SEGD2H), bit, enable);
      }
#else
      sl_hal_bus_reg_write_bit(&(LCD->SEGD2), bit, enable);
#endif
      break;

    case 3:
#if defined(_LCD_SEGD3H_MASK)
      if (bit < 32) {
        sl_hal_bus_reg_write_bit(&(LCD->SEGD3), bit, enable);
      } else {
        bit -= 32;
        sl_hal_bus_reg_write_bit(&(LCD->SEGD3H), bit, enable);
      }
#else
      sl_hal_bus_reg_write_bit(&(LCD->SEGD3), bit, enable);
#endif
      break;

#if defined(_LCD_SEGD4_MASK)
    case 4:
#if defined(_LCD_SEGD4H_MASK)
      if (bit < 32) {
        sl_hal_bus_reg_write_bit(&(LCD->SEGD4), bit, enable);
      } else {
        bit -= 32;
        sl_hal_bus_reg_write_bit(&(LCD->SEGD4H), bit, enable);
      }
#else
      sl_hal_bus_reg_write_bit(&(LCD->SEGD4), bit, enable);
#endif
      break;
#endif

#if defined(_LCD_SEGD5_MASK)
    case 5:
#if defined(_LCD_SEGD5H_MASK)
      if (bit < 32) {
        sl_hal_bus_reg_write_bit(&(LCD->SEGD5), bit, enable);
      } else {
        bit -= 32;
        sl_hal_bus_reg_write_bit(&(LCD->SEGD5H), bit, enable);
      }
#else
      sl_hal_bus_reg_write_bit(&(LCD->SEGD5), bit, enable);
#endif
      break;
#endif

#if defined(_LCD_SEGD6_MASK)
    case 6:
#if defined(_LCD_SEGD6H_MASK)
      if (bit < 32) {
        sl_hal_bus_reg_write_bit(&(LCD->SEGD6), bit, enable);
      } else {
        bit -= 32;
        sl_hal_bus_reg_write_bit(&(LCD->SEGD6H), bit, enable);
      }
#else
      sl_hal_bus_reg_write_bit(&(LCD->SEGD6), bit, enable);
#endif
      break;
#endif

#if defined(_LCD_SEGD7_MASK)
    case 7:
#if defined(_LCD_SEGD7H_MASK)
      if (bit < 32) {
        sl_hal_bus_reg_write_bit(&(LCD->SEGD7), bit, enable);
      } else {
        bit -= 32;
        sl_hal_bus_reg_write_bit(&(LCD->SEGD7H), bit, enable);
      }
#else
      sl_hal_bus_reg_write_bit(&(LCD->SEGD7), bit, enable);
#endif
      break;
#endif

    default:
      EFM_ASSERT(0);
      break;
  }
}

/***************************************************************************//**
 * Update 0-31 lowest segments on a given COM-line in one operation
 * according to the bit mask.
 ******************************************************************************/
void sl_hal_lcd_segment_set_low(uint8_t com,
                                uint32_t mask,
                                uint32_t bits)
{
  uint32_t segment_data;

  // Series 2 parts support up to 4 COM lines except for xG26 and xG28, which supports up to 8 COM lines.
  EFM_ASSERT(com < (int)SL_HAL_LCD_COM_LINES_MAX);

  // Series 2 parts support up to 20 segment lines.
  // Except for xG26 which supports up to 40 segment lines. and xG28 which supports up to 28 segment lines.
  EFM_ASSERT(!(mask & (~_LCD_SEGD0_MASK)));
  EFM_ASSERT(!(bits & (~_LCD_SEGD0_MASK)));

  switch (com) {
    case 0:
      segment_data     = LCD->SEGD0;
      segment_data    &= ~(mask);
      segment_data    |= (mask & bits);
      LCD->SEGD0 = segment_data;
      break;

    case 1:
      segment_data     = LCD->SEGD1;
      segment_data    &= ~(mask);
      segment_data    |= (mask & bits);
      LCD->SEGD1 = segment_data;

      break;
    case 2:
      segment_data     = LCD->SEGD2;
      segment_data    &= ~(mask);
      segment_data    |= (mask & bits);
      LCD->SEGD2 = segment_data;
      break;

    case 3:
      segment_data     = LCD->SEGD3;
      segment_data    &= ~(mask);
      segment_data    |= (mask & bits);
      LCD->SEGD3 = segment_data;
      break;

#if defined(_LCD_SEGD4_MASK)
    case 4:
      segment_data     = LCD->SEGD4;
      segment_data    &= ~(mask);
      segment_data    |= (mask & bits);
      LCD->SEGD4 = segment_data;
      break;
#endif

#if defined(_LCD_SEGD5_MASK)
    case 5:
      segment_data     = LCD->SEGD5;
      segment_data    &= ~(mask);
      segment_data    |= (mask & bits);
      LCD->SEGD5 = segment_data;
      break;
#endif

#if defined(_LCD_SEGD6_MASK)
    case 6:
      segment_data     = LCD->SEGD6;
      segment_data    &= ~(mask);
      segment_data    |= (mask & bits);
      LCD->SEGD6 = segment_data;
      break;
#endif

#if defined(_LCD_SEGD7_MASK)
    case 7:
      segment_data     = LCD->SEGD7;
      segment_data    &= ~(mask);
      segment_data    |= (mask & bits);
      LCD->SEGD7 = segment_data;
      break;
#endif

    default:
      EFM_ASSERT(0);
      break;
  }
}

#if defined(_LCD_SEGD0H_MASK)
/***************************************************************************//**
 * Update the high (32-39) segments on a given COM-line in one operation.
 ******************************************************************************/
void sl_hal_lcd_segment_set_high(uint8_t com,
                                 uint32_t mask,
                                 uint32_t bits)
{
  uint32_t segment_data;

#if defined(_LCD_SEGD7H_MASK)
  EFM_ASSERT(com < 8);
#else
  EFM_ASSERT(com < 4);
#endif

  // A maximum number of com lines.
  switch (com) {
    case 0:
      segment_data     = LCD->SEGD0H;
      segment_data    &= ~(mask);
      segment_data    |= (mask & bits);
      LCD->SEGD0H = segment_data;
      break;
    case 1:
      segment_data     = LCD->SEGD1H;
      segment_data    &= ~(mask);
      segment_data    |= (mask & bits);
      LCD->SEGD1H = segment_data;
      break;
    case 2:
      segment_data     = LCD->SEGD2H;
      segment_data    &= ~(mask);
      segment_data    |= (mask & bits);
      LCD->SEGD2H = segment_data;
      break;
    case 3:
      segment_data     = LCD->SEGD3H;
      segment_data    &= ~(mask);
      segment_data    |= (mask & bits);
      LCD->SEGD3H = segment_data;
      break;
#if defined(_LCD_SEGD4H_MASK)
    case 4:
      segment_data     = LCD->SEGD4H;
      segment_data    &= ~(mask);
      segment_data    |= (mask & bits);
      LCD->SEGD4H = segment_data;
      break;
#endif
#if defined(_LCD_SEGD5H_MASK)
    case 5:
      segment_data     = LCD->SEGD5H;
      segment_data    &= ~(mask);
      segment_data    |= (mask & bits);
      LCD->SEGD5H = segment_data;
      break;
#endif
#if defined(_LCD_SEGD6H_MASK)
    case 6:
      segment_data     = LCD->SEGD6H;
      segment_data    &= ~(mask);
      segment_data    |= (mask & bits);
      LCD->SEGD6H = segment_data;
      break;
#endif
#if defined(_LCD_SEGD7H_MASK)
    case 7:
      segment_data     = LCD->SEGD7H;
      segment_data    &= ~(mask);
      segment_data    |= (mask & bits);
      LCD->SEGD7H = segment_data;
      break;
#endif
    default:
      break;
  }
}
#endif

/***************************************************************************//**
 * Enables a given LCD COM line.
 ******************************************************************************/
void sl_hal_lcd_enable_com_line(uint8_t com)
{
  // Series 2 parts support up to 4 COM lines except for xG28, which supports up to 8 COM lines.
  EFM_ASSERT(com < SL_HAL_LCD_COM_LINES_MAX);

  if (com < LCD_COM_NUM) {
    GPIO->LCDCOM_SET = 1 << com;
  }
#if defined(LCD_OCTAPLEX) && (LCD_OCTAPLEX == 1)
  else {
    // On xG28, SEG lines shall be configured as COM lines
    // for COM support above 4 COM lines
#if defined(_GPIO_LCDSEGH_MASK)
    if ((com - LCD_COM_NUM) + LCD_SEGASCOM_SEGSTART > 31) {
      GPIO->LCDSEGH_SET = 1 << ((com - LCD_COM_NUM) + LCD_SEGASCOM_SEGSTART - 32);
    } else {
      GPIO->LCDSEGL_SET = 1 << ((com - LCD_COM_NUM) + LCD_SEGASCOM_SEGSTART);
    }
  }
#else
    GPIO->LCDSEG_SET = 1 << ((com - LCD_COM_NUM) + LCD_SEGASCOM_SEGSTART);
  }
#endif
#endif
}

/***************************************************************************//**
 * Disable a given LCD COM line.
 ******************************************************************************/
void sl_hal_lcd_disable_com_line(uint8_t com)
{
  // Series 2 parts support up to 4 COM lines except for xG28, which supports up to 8 COM lines.
  EFM_ASSERT(com < SL_HAL_LCD_COM_LINES_MAX);

  if (com < LCD_COM_NUM) {
    GPIO->LCDCOM_CLR = 1 << com;
  }
#if defined(LCD_OCTAPLEX) && (LCD_OCTAPLEX == 1)
  else {
    // On xG28, SEG lines shall be configured as COM lines
    // for COM support above 4 COM lines
#if defined(_GPIO_LCDSEGH_MASK)
    if ((com - LCD_COM_NUM) + LCD_SEGASCOM_SEGSTART > 31) {
      GPIO->LCDSEGH_CLR = 1 << ((com - LCD_COM_NUM) + LCD_SEGASCOM_SEGSTART - 32);
    } else {
      GPIO->LCDSEGL_CLR = 1 << ((com - LCD_COM_NUM) + LCD_SEGASCOM_SEGSTART);
    }
  }
#else
    GPIO->LCDSEG_CLR = 1 << ((com - LCD_COM_NUM) + LCD_SEGASCOM_SEGSTART);
  }
#endif
#endif
}

/***************************************************************************//**
 * Configure the bias level for a specific segment line for Direct Segment Control.
 ******************************************************************************/
void sl_hal_lcd_bias_set_segment(uint8_t segment_line,
                                 uint32_t bias_level)
{
  volatile uint32_t *segment_register;
  uint32_t          bias_register;
  uint32_t          bit_shift;

  if (segment_line >= (int)LCD_SEG_NUM) {
    return;
  }

  // On Series 2, the following layout is used to configure bias when DSC
  // is active:
  //   REGISTERS | ... | 7 .. 4 | 3 .. 0 |
  //   SEGD0     | ... |  seg4  |  seg0  |
  //   SEGD1     | ... |  seg5  |  seg1  |
  //   .....

  bias_register = segment_line % 4;
  bit_shift     = (segment_line / 4) * 4;

#if defined(_GPIO_LCDSEGH_MASK)
  switch (bias_register) {
    case 0:
      if (bit_shift < 32) {
        segment_register = &LCD->SEGD0;
      } else {
        segment_register = &LCD->SEGD0H;
        bit_shift       -= 32;
      }
      break;
    case 1:
      if (bit_shift < 32) {
        segment_register = &LCD->SEGD1;
      } else {
        segment_register = &LCD->SEGD1H;
        bit_shift       -= 32;
      }
      break;
    case 2:
      if (bit_shift < 32) {
        segment_register = &LCD->SEGD2;
      } else {
        segment_register = &LCD->SEGD2H;
        bit_shift       -= 32;
      }
      break;
    case 3:
      if (bit_shift < 32) {
        segment_register = &LCD->SEGD3;
      } else {
        segment_register = &LCD->SEGD3H;
        bit_shift       -= 32;
      }
      break;
    default:
      segment_register = NULL;
      EFM_ASSERT(0);
      break;
  }

#else

  switch (bias_register) {
    case 0:
      segment_register = &LCD->SEGD0;
      break;
    case 1:
      segment_register = &LCD->SEGD1;
      break;
    case 2:
      segment_register = &LCD->SEGD2;
      break;
    case 3:
      segment_register = &LCD->SEGD3;
      break;
    default:
      segment_register = NULL;
      EFM_ASSERT(0);
      break;
  }
#endif

  // Configure a new bias setting.
  sl_hal_bus_reg_write_mask(segment_register, 0xF << bit_shift, bias_level << bit_shift);
}

/***************************************************************************//**
 * Configure the bias level for a specific segment line.
 ******************************************************************************/
void sl_hal_lcd_bias_set_com(uint8_t com_line,
                             uint32_t bias_level)
{
  volatile uint32_t *com_register;
  uint32_t bias_register;
  uint32_t bit_shift;

  if (com_line >= (int)LCD_COM_NUM) {
    return;
  }

  bias_register = (uint32_t) com_line % 2;
  bit_shift     = (com_line / 2) * 4;

  switch (bias_register) {
    case 0:
      com_register = &LCD->AREGA;
      break;
    case 1:
      com_register = &LCD->AREGB;
      break;
    default:
      com_register = NULL;
      EFM_ASSERT(0);
      break;
  }

  sl_hal_bus_reg_write_mask(com_register, 0xF << bit_shift, bias_level << bit_shift);
}

#endif /* defined(LCD_COUNT) && (LCD_COUNT > 0) */
