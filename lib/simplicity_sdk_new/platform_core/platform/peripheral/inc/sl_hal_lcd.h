/***************************************************************************//**
 * @file
 * @brief Liquid Crystal Display (LCD) peripheral API
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

#ifndef SL_HAL_LCD_H
#define SL_HAL_LCD_H

#include "em_device.h"

#if defined(LCD_COUNT) && (LCD_COUNT > 0)
#include "sl_enum.h"
#include "sl_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/***************************************************************************//**
 * @addtogroup lcd LCD - Liquid Crystal Display
 * @brief Liquid Crystal Display (LCD) Peripheral API
 * @details
 *  This module contains functions to control the LCD peripheral of Silicon
 *  Labs 32-bit MCUs and SoCs. The LCD driver can drive up to 8x36 segmented
 *  LCD directly. The animation feature makes it possible to have active
 *  animations without the CPU intervention.
 * @{
 ******************************************************************************/

/*******************************************************************************
 ********************************   DEFINES   **********************************
 ******************************************************************************/

/// Default Clock Prescaler.
#define SL_HAL_LCD_DEFAULT_CLOCK_PRESCALER 64
/// Default LCD Frame Rate Divisor.
#define SL_HAL_LCD_DEFAULT_FRAME_RATE_DIV  4
/// Default LCD Contrast.
#define SL_HAL_LCD_DEFAULT_CONTRAST        15

/// Maximum common lines of LCD.
#if defined(LCD_OCTAPLEX) && (LCD_OCTAPLEX == 1)
#define SL_HAL_LCD_COM_LINES_MAX           (LCD_COM_NUM + LCD_SEGASCOM_NUM)
#else
#define SL_HAL_LCD_COM_LINES_MAX           LCD_COM_NUM
#endif
/// Maximum segment lines of LCD.
#define SL_HAL_LCD_SEGMENT_LINES_MAX       LCD_SEG_NUM

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/// MUX setting.
SL_ENUM(sl_hal_lcd_multiplexer_t) {
  /// Static (segments can be multiplexed with LCD_COM[0]).
  SL_HAL_LCD_MUX_STATIC     = _LCD_DISPCTRL_MUX_STATIC,     ///< Static (segments can be multiplexed with LCD_COM[0]).
  SL_HAL_LCD_MUX_DUPLEX     = _LCD_DISPCTRL_MUX_DUPLEX,     ///< Duplex: 1/2 Duty cycle (segments can be multiplexed with LCD_COM[0:1]).
  SL_HAL_LCD_MUX_TRIPLEX    = _LCD_DISPCTRL_MUX_TRIPLEX,    ///< Triplex: 1/3 Duty cycle (segments can be multiplexed with LCD_COM[0:2]).
  SL_HAL_LCD_MUX_QUADRUPLEX = _LCD_DISPCTRL_MUX_QUADRUPLEX, ///< Quadruplex: 1/4 Duty cycle (segments can be multiplexed with LCD_COM[0:3]).
#if defined(LCD_DISPCTRL_MUX_SEXTAPLEX)
  SL_HAL_LCD_MUX_SEXTAPLEX  = _LCD_DISPCTRL_MUX_SEXTAPLEX,  ///< Sextaplex: 1/6 Duty cycle (segments can be multiplexed with LCD_COM[0:5]).
  SL_HAL_LCD_MUX_OCTAPLEX   = _LCD_DISPCTRL_MUX_OCTAPLEX,   ///< Octaplex: 1/8 Duty cycle (segments can be multiplexed with LCD_COM[0:7]).
#endif
};

/// Wave type.
SL_ENUM(sl_hal_lcd_wave_t) {
  SL_HAL_LCD_WAVE_LOW_POWER = _LCD_DISPCTRL_WAVE_TYPEB,     ///< Low power optimized waveform output.
  SL_HAL_LCD_WAVE_NORMAL    = _LCD_DISPCTRL_WAVE_TYPEA      ///< Regular waveform output.
};

/// Bias setting.
SL_ENUM(sl_hal_lcd_bias_t) {
  SL_HAL_LCD_BIAS_STATIC     = _LCD_DISPCTRL_BIAS_STATIC,     ///< Static (2 levels).
  SL_HAL_LCD_BIAS_ONE_HALF   = _LCD_DISPCTRL_BIAS_ONEHALF,    ///< 1/2 Bias (3 levels).
  SL_HAL_LCD_BIAS_ONE_THIRD  = _LCD_DISPCTRL_BIAS_ONETHIRD,   ///< 1/3 Bias (4 levels).
  SL_HAL_LCD_BIAS_ONE_FOURTH = _LCD_DISPCTRL_BIAS_ONEFOURTH,  ///< 1/4 Bias (5 levels).
};

/// Mode of operation.
SL_ENUM(sl_hal_lcd_mode_t) {
  SL_HAL_LCD_MODE_STEP_DOWN   = _LCD_BIASCTRL_MODE_STEPDOWN,   ///< External cap with resistor string.
  SL_HAL_LCD_MODE_CHARGE_PUMP = _LCD_BIASCTRL_MODE_CHARGEPUMP, ///< External cap and internal oscillator.
};

/// Frame Counter Clock Prescaler, FC-CLK = FrameRate (Hz) / this factor.
SL_ENUM(sl_hal_lcd_frame_counter_prescale_t) {
  SL_HAL_LCD_FRAME_COUNTER_PRESCALE_DIV1 = _LCD_BACFG_FCPRESC_DIV1,     ///< Prescale Div 1.
  SL_HAL_LCD_FRAME_COUNTER_PRESCALE_DIV2 = _LCD_BACFG_FCPRESC_DIV2,     ///< Prescale Div 2.
  SL_HAL_LCD_FRAME_COUNTER_PRESCALE_DIV4 = _LCD_BACFG_FCPRESC_DIV4,     ///< Prescale Div 4.
  SL_HAL_LCD_FRAME_COUNTER_PRESCALE_DIV8 = _LCD_BACFG_FCPRESC_DIV8      ///< Prescale Div 8.
};

/// Update Data Control.
SL_ENUM(sl_hal_lcd_update_data_control_t) {
  SL_HAL_LCD_UPDATE_CONTROL_REGULAR               = _LCD_CTRL_UDCTRL_REGULAR,          ///< Regular update, data transfer done immediately.
  SL_HAL_LCD_UPDATE_CONTROL_FRAME_COUNTER_EVENT   = _LCD_CTRL_UDCTRL_FCEVENT,          ///< Data transfer done at Frame Counter event.
  SL_HAL_LCD_UPDATE_CONTROL_FRAME_START           = _LCD_CTRL_UDCTRL_FRAMESTART,       ///< Data transfer done at Frame Start.
  SL_HAL_LCD_UPDATE_CONTROL_DISPLAY_EVENT         = _LCD_CTRL_UDCTRL_DISPLAYEVENT      ///< Data transfer done at Display Counter event.
};

/// Auto Load address which will start the synchronization from CLK_BUS to CLK_PER.
SL_ENUM(sl_hal_lcd_load_address_t) {
  SL_HAL_LCD_LOAD_ADDRESS_NONE   = 0,                                     ///< Starts synchronizing registers after a write to BACTRL.
  SL_HAL_LCD_LOAD_ADDRESS_BACTRL = _LCD_UPDATECTRL_LOADADDR_BACTRLWR,     ///< Starts synchronizing registers after a write to BACTRL.
  SL_HAL_LCD_LOAD_ADDRESS_AREGA  = _LCD_UPDATECTRL_LOADADDR_AREGAWR,      ///< Starts synchronizing registers after a write to AREGA.
  SL_HAL_LCD_LOAD_ADDRESS_AREGB  = _LCD_UPDATECTRL_LOADADDR_AREGBWR,      ///< Starts synchronizing registers after a write to AREGB.
  SL_HAL_LCD_LOAD_ADDRESS_SEGD0  = _LCD_UPDATECTRL_LOADADDR_SEGD0WR,      ///< Starts synchronizing registers after a write to SEGD0.
  SL_HAL_LCD_LOAD_ADDRESS_SEGD1  = _LCD_UPDATECTRL_LOADADDR_SEGD1WR,      ///< Starts synchronizing registers after a write to SEGD1.
  SL_HAL_LCD_LOAD_ADDRESS_SEGD2  = _LCD_UPDATECTRL_LOADADDR_SEGD2WR,      ///< Starts synchronizing registers after a write to SEGD2.
  SL_HAL_LCD_LOAD_ADDRESS_SEGD3  = _LCD_UPDATECTRL_LOADADDR_SEGD3WR,      ///< Starts synchronizing registers after a write to SEGD3.
#if defined(LCD_UPDATECTRL_LOADADDR_SEGD4WR)
  SL_HAL_LCD_LOAD_ADDRESS_SEGD4  = _LCD_UPDATECTRL_LOADADDR_SEGD4WR,      ///< Starts synchronizing registers after a write to SEGD4.
#endif
#if defined(LCD_UPDATECTRL_LOADADDR_SEGD5WR)
  SL_HAL_LCD_LOAD_ADDRESS_SEGD5  = _LCD_UPDATECTRL_LOADADDR_SEGD5WR,      ///< Starts synchronizing registers after a write to SEGD5.
#endif
#if defined(LCD_UPDATECTRL_LOADADDR_SEGD6WR)
  SL_HAL_LCD_LOAD_ADDRESS_SEGD6  = _LCD_UPDATECTRL_LOADADDR_SEGD6WR,      ///< Starts synchronizing registers after a write to SEGD6.
#endif
#if defined(LCD_UPDATECTRL_LOADADDR_SEGD7WR)
  SL_HAL_LCD_LOAD_ADDRESS_SEGD7  = _LCD_UPDATECTRL_LOADADDR_SEGD7WR,      ///< Starts synchronizing registers after a write to SEGD7.
#endif
};

/// Animation Shift operation; none, left or right.
SL_ENUM(sl_hal_lcd_animation_shift_t) {
  SL_HAL_LCD_ANIMATION_SHIFT_NONE  = _LCD_BACTRL_AREGASC_NOSHIFT,       ///< No shift.
  SL_HAL_LCD_ANIMATION_SHIFT_LEFT  = _LCD_BACTRL_AREGASC_SHIFTLEFT,     ///< Shift segment bits left.
  SL_HAL_LCD_ANIMATION_SHIFT_RIGHT = _LCD_BACTRL_AREGASC_SHIFTRIGHT     ///< Shift segment bits right.
};

/// Animation Logic Control, how AReg and BReg should be combined.
SL_ENUM(sl_hal_lcd_animation_logic_t) {
  SL_HAL_LCD_ANIMATION_LOGIC_AND = _LCD_BACTRL_ALOGSEL_AND,  ///< Use bitwise logic AND to mix animation register A (AREGA) and B (AREGB).
  SL_HAL_LCD_ANIMATION_LOGIC_OR  = _LCD_BACTRL_ALOGSEL_OR    ///< Use bitwise logic OR to mix animation register A (AREGA) and B (AREGB).
};

/// Animation Location, set the LCD segments which animation applies to.
SL_ENUM(sl_hal_lcd_animation_location_t) {
  SL_HAL_LCD_ANIMATION_LOCATION_SEG0_TO_7  = _LCD_BACTRL_ALOC_SEG0TO7,       ///< Animation appears on segments 0 to 7.
  SL_HAL_LCD_ANIMATION_LOCATION_SEG8_TO_15 = _LCD_BACTRL_ALOC_SEG8TO15       ///< Animation appears on segments 8 to 15.
};

/// Charge redistribution control.
SL_ENUM(sl_hal_lcd_charge_redistribution_t) {
  SL_HAL_LCD_CHARGE_REDISTRIBUTION_DISABLE     = _LCD_DISPCTRL_CHGRDST_DISABLE,  ///< Disable charge redistribution.
  SL_HAL_LCD_CHARGE_REDISTRIBUTION_ENABLE      = _LCD_DISPCTRL_CHGRDST_ONE,      ///< Use 1 prescaled low frequency clock cycle for charge redistribution.
  SL_HAL_LCD_CHARGE_REDISTRIBUTION_TWO_CYCLE   = _LCD_DISPCTRL_CHGRDST_TWO,      ///< Use 2 prescaled low frequency clock cycle for charge redistribution.
  SL_HAL_LCD_CHARGE_REDISTRIBUTION_THREE_CYCLE = _LCD_DISPCTRL_CHGRDST_THREE,    ///< Use 3 prescaled low frequency clock cycle for charge redistribution.
  SL_HAL_LCD_CHARGE_REDISTRIBUTION_FOUR_CYCLE  = _LCD_DISPCTRL_CHGRDST_FOUR      ///< Use 4 prescaled low frequency clock cycle for charge redistribution.
};

/// DMA mode of operation.
SL_ENUM(sl_hal_lcd_dma_mode_t) {
  SL_HAL_LCD_DMA_MODE_DISABLE               = _LCD_BIASCTRL_DMAMODE_DMADISABLE,   ///< No DMA requests are generated.
  SL_HAL_LCD_DMA_MODE_FRAME_COUNTER_EVENT   = _LCD_BIASCTRL_DMAMODE_DMAFC,        ///< DMA request on frame counter event.
  SL_HAL_LCD_DMA_MODE_DISPLAY_EVENT         = _LCD_BIASCTRL_DMAMODE_DMADISPLAY    ///< DMA request on display counter event.
};

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// LCD Animation Configuration.
typedef struct {
  uint32_t                          A_reg;              ///< Initial Animation Register A Value.
  sl_hal_lcd_animation_shift_t      A_shift;            ///< Shift operation of Animation Register A.
  uint32_t                          B_reg;              ///< Initial Animation Register B Value.
  sl_hal_lcd_animation_shift_t      B_shift;            ///< Shift operation of Animation Register B.
  sl_hal_lcd_animation_logic_t      animation_logic;    ///< A and B Logical Operation to use for mixing and outputting resulting segments.
  sl_hal_lcd_animation_location_t   start_segment;      ///< Number of first segment to animate.
} sl_hal_lcd_animation_init_t;

/// LCD Frame Control Initialization.
typedef struct {
  uint32_t                              top;           ///< Frame Counter top value.
  sl_hal_lcd_frame_counter_prescale_t   prescale;      ///< Frame Counter clock prescaler.
} sl_hal_lcd_frame_counter_init_t;

/// LCD Controller Initialization structure.
typedef struct {
  sl_hal_lcd_multiplexer_t              mux;                   ///< Mux configuration.
  sl_hal_lcd_bias_t                     bias;                  ///< Bias configuration.
  sl_hal_lcd_wave_t                     wave;                  ///< Wave configuration.
  sl_hal_lcd_mode_t                     mode;                  ///< Mode of operation.
  sl_hal_lcd_charge_redistribution_t    charge_redistribution; ///< Charge redistribution cycles.
  uint8_t                               frame_rate_divider;    ///< Frame rate divider.
  int                                   contrast_level;        ///< Contrast level.
  uint32_t                              clock_prescaler;       ///< Clock Prescaler.
} sl_hal_lcd_init_t;

/// Default configuration for LCD initialization structure, enables 160 segments.
#define SL_HAL_LCD_INIT_DEFAULT              \
  {                                          \
    SL_HAL_LCD_MUX_QUADRUPLEX,               \
    SL_HAL_LCD_BIAS_ONE_FOURTH,              \
    SL_HAL_LCD_WAVE_LOW_POWER,               \
    SL_HAL_LCD_MODE_STEP_DOWN,               \
    SL_HAL_LCD_CHARGE_REDISTRIBUTION_ENABLE, \
    SL_HAL_LCD_DEFAULT_FRAME_RATE_DIV,       \
    SL_HAL_LCD_DEFAULT_CONTRAST,             \
    SL_HAL_LCD_DEFAULT_CLOCK_PRESCALER       \
  }

/// Default configuration for LCD Animation initialization structure.
#define SL_HAL_LCD_ANIMATION_INIT_DEFAULT                                                          \
  {                                                                                                \
    0x00000000,                               /* Turn off all segments, default value for AREGA */ \
    SL_HAL_LCD_ANIMATION_SHIFT_NONE,                                                               \
    0x00000000,                               /* Turn off all segments, default value for AREGB */ \
    SL_HAL_LCD_ANIMATION_SHIFT_NONE,                                                               \
    SL_HAL_LCD_ANIMATION_LOGIC_OR,                                                                 \
    SL_HAL_LCD_ANIMATION_LOCATION_SEG8_TO_15                                                       \
  }

/// Default configuration for LCD Frame Counter initialization structure.
#define SL_HAL_LCD_FRAME_COUNTER_INIT_DEFAULT \
  {                                           \
    0,                                        \
    SL_HAL_LCD_FRAME_COUNTER_PRESCALE_DIV8    \
  }

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * Initialize the Liquid Crystal Display (LCD) controller.
 *
 * @details Configures the LCD controller. You must enable
 *          it afterwards, potentially configuring Frame Control and interrupts first
 *          according to requirements.
 *
 * @param[in] init A pointer to the initialization structure which configures the LCD controller.
 ******************************************************************************/
void sl_hal_lcd_init(const sl_hal_lcd_init_t *init);

/***************************************************************************//**
 * Initialize the LCD Frame Counter.
 *
 * @note The LCD controller need to be disabled before running this function.
 *
 * @param[in] init A pointer to the Frame Counter initialization structure.
 ******************************************************************************/
void sl_hal_lcd_init_frame_counter(const sl_hal_lcd_frame_counter_init_t *init);

/***************************************************************************//**
 * Configure the LCD controller Animation feature.
 *
 * @param[in] init A pointer to the LCD Animation initialization structure.
 ******************************************************************************/
void sl_hal_lcd_init_animation(const sl_hal_lcd_animation_init_t *init);

/***************************************************************************//**
 * Configure Update Control.
 *
 * @note The LCD controller need to be disabled before running this function.
 *
 * @param[in] method Configures the LCD update method.
 ******************************************************************************/
void sl_hal_lcd_update_control(sl_hal_lcd_update_data_control_t method);

/***************************************************************************//**
 * Enables a given LCD segment line.
 *
 * @param[in] segment_number Segment line number.
 ******************************************************************************/
void sl_hal_lcd_segment_enable(uint32_t segment_number);

/***************************************************************************//**
 * Disable a given LCD segment line.
 *
 * @param[in] segment_number Segment line number.
 ******************************************************************************/
void sl_hal_lcd_segment_disable(uint32_t segment_number);

/***************************************************************************//**
 * Turn on or clear a segment.
 *
 * @note For the Series 2 Family, the maximum configuration is 4x20.
 *       For the Series 2 xG28, the maximum configuration is 8x24 or 4x28.
 *
 * @param[in] com A COM line to change.
 * @param[in] bit A bit index indicating which field to change.
 * @param[in] enable A boolean indicating whether to set or clear the segment.
 ******************************************************************************/
void sl_hal_lcd_segment_set(uint8_t com,
                            int bit,
                            bool enable);

/***************************************************************************//**
 * Update 0-31 lowest segments on a given COM-line in one operation according
 * to the bit mask.
 *
 * @param[in] com Indicates a COM line to update.
 * @param[in] mask A bit mask for segments 0-31.
 * @param[in] bits A bit pattern for segments 0-31.
 ******************************************************************************/
void sl_hal_lcd_segment_set_low(uint8_t com,
                                uint32_t mask,
                                uint32_t bits);

#if defined(_LCD_SEGD0H_MASK)
/***************************************************************************//**
 * Update the high (32-39) segments on a given COM-line in one operation.
 *
 * @param[in] com Indicates a COM line to update.
 * @param[in] mask A bit mask for segments 32-39.
 * @param[in] bits A bit pattern for segments 32-39.
 ******************************************************************************/
void sl_hal_lcd_segment_set_high(uint8_t com,
                                 uint32_t mask,
                                 uint32_t bits);
#endif

/***************************************************************************//**
 * Enables a given LCD COM line.
 *
 * @param[in] com COM line number.
 ******************************************************************************/
void sl_hal_lcd_enable_com_line(uint8_t com);

/***************************************************************************//**
 * Disable a given LCD COM line.
 *
 * @param[in] com COM line number.
 ******************************************************************************/
void sl_hal_lcd_disable_com_line(uint8_t com);

/***************************************************************************//**
 * Configure the bias level for a specific segment line for Direct Segment Control.
 *
 * @note When DSC is active, each configuration takes up 4 bits in the corresponding
 *       Segment Registers (SEGDx/SEGDxH) which defines the bias level.
 *       For optimal use of this feature, the entire SEGD-registers should be set
 *       at once in an optimized routine. Therefore, this function shows how to
 *       correctly configure the bias levels and should be used with care.
 *
 * @param[in] segment_line A segment line number.
 * @param[in] bias_level The bias configuration level. This value must be within
 *                       the constraints defined by the LCD_DISPCTRL bias settings.
 *                       For more information, see the applicable Reference Manual
 *                       and data sheet.
 ******************************************************************************/
void sl_hal_lcd_bias_set_segment(uint8_t segment_line,
                                 uint32_t bias_level);

/***************************************************************************//**
 * Configure the bias level for a specific segment line.
 *
 * @note When DSC is active, each configuration takes up 4 bits in the corresponding
 *       Segment Registers (AREGA/AREGB) which defines bias level.
 *       For optimal use of this feature, the entire register set should be set
 *       at once in a optimized routine. Therefore, this function shows how to
 *       correctly configure the bias levels and should be used with care.
 *
 * @param[in] com_line Max is LCD_COM_NUM, defined in device-specific
 *                     headers.
 * @param[in] bias_level The bias configuration level. This value must be within
 *                       the constraints defined by the LCD_DISPCTRL bias settings.
 *                       For more information, see the appropriate Reference Manual
 *                       and data sheet.
 ******************************************************************************/
void sl_hal_lcd_bias_set_com(uint8_t com_line,
                             uint32_t bias_level);

/***************************************************************************//**
 * Wait for load synchronization completion.
 *
 * @note Doing any writes to HV registers will not go through and will cause a
 *       bus fault.
 ******************************************************************************/
__INLINE void sl_hal_lcd_wait_load_busy(void)
{
  while (LCD->STATUS & _LCD_STATUS_LOADBUSY_MASK) {
    // Wait for load synchronization completion.
  }
}

/***************************************************************************//**
 * Wait for the LCD to complete resetting or disabling procedure.
 ******************************************************************************/
__INLINE void sl_hal_lcd_wait_ready(void)
{
  while ((LCD->SWRST & _LCD_SWRST_RESETTING_MASK)
         || (LCD->EN & _LCD_EN_DISABLING_MASK)
         || (LCD->STATUS & _LCD_STATUS_LOADBUSY_MASK)) {
    // Wait for resetting, for disabling or for all synchronizations to finish
  }
}

/***************************************************************************//**
 * Enable LCD controller with current configuration.
 ******************************************************************************/
__INLINE void sl_hal_lcd_enable(void)
{
  LCD->EN_SET = LCD_EN_EN;
}

/***************************************************************************//**
 * Disable LCD controller.
 *
 * @note This function will disables LCD controller.
 *       Remember to enable CMU clock for LCD for correct operation.
 ******************************************************************************/
__INLINE void sl_hal_lcd_disable(void)
{
  // Wait for internal synchronization completion.
  sl_hal_lcd_wait_load_busy();

  LCD->EN_CLR = LCD_EN_EN;
  sl_hal_lcd_wait_ready();
}

/***************************************************************************//**
 * Reset the LCD.
 ******************************************************************************/
__INLINE void sl_hal_lcd_reset(void)
{
  LCD->SWRST_SET = LCD_SWRST_SWRST;

  // Wait for reset to complete.
  sl_hal_lcd_wait_ready();
}

/***************************************************************************//**
 * Enable LCD Animation feature.
 ******************************************************************************/
__INLINE void sl_hal_lcd_animation_enable(void)
{
  LCD->BACTRL_SET = LCD_BACTRL_AEN;
}

/***************************************************************************//**
 * Disable LCD Animation feature.
 ******************************************************************************/
__INLINE void sl_hal_lcd_animation_disable(void)
{
  LCD->BACTRL_CLR = LCD_BACTRL_AEN;
}

/***************************************************************************//**
 * Return the current animation state.
 *
 * @return Animation state, in range 0-15.
 ******************************************************************************/
__INLINE uint32_t sl_hal_lcd_animation_get_state(void)
{
  return (uint32_t)(LCD->STATUS & _LCD_STATUS_ASTATE_MASK) >> _LCD_STATUS_ASTATE_SHIFT;
}

/***************************************************************************//**
 * Enable the LCD blink.
 ******************************************************************************/
__INLINE void sl_hal_lcd_blink_enable(void)
{
  LCD->BACTRL_SET = LCD_BACTRL_BLINKEN;
}

/***************************************************************************//**
 * Disable the LCD blink.
 ******************************************************************************/
__INLINE void sl_hal_lcd_blink_disable(void)
{
  LCD->BACTRL_CLR = LCD_BACTRL_BLINKEN;
}

/***************************************************************************//**
 * Return the current blink state.
 *
 * @return Return value is 1 if segments are enabled, 0 if disabled.
 ******************************************************************************/
__INLINE uint32_t sl_hal_lcd_blink_get_state(void)
{
  return (uint32_t)(LCD->STATUS & _LCD_STATUS_BLINK_MASK) >> _LCD_STATUS_BLINK_SHIFT;
}

/***************************************************************************//**
 * Clears all segments while keeping segment state.
 ******************************************************************************/
__INLINE void sl_hal_lcd_enable_blank(void)
{
  LCD->BACTRL_SET = LCD_BACTRL_BLANK;
}

/***************************************************************************//**
 * Restores all segment lines.
 ******************************************************************************/
__INLINE void sl_hal_lcd_disable_blank(void)
{
  LCD->BACTRL_CLR = LCD_BACTRL_BLANK;
}

/***************************************************************************//**
 * Enable or disable LCD Frame counter.
 ******************************************************************************/
__INLINE void sl_hal_lcd_enable_frame_counter(void)
{
  // Ensure no internal sync is in progress.
  sl_hal_lcd_wait_load_busy();

  LCD->BACTRL_SET = LCD_BACTRL_FCEN;
}

/***************************************************************************//**
 * Disable LCD Frame counter.
 ******************************************************************************/
__INLINE void sl_hal_lcd_disable_frame_counter(void)
{
  // Ensure no internal sync is in progress.
  sl_hal_lcd_wait_load_busy();

  LCD->BACTRL_CLR = LCD_BACTRL_FCEN;
}

/***************************************************************************//**
 * Enable the LCD Display counter.
 ******************************************************************************/
__INLINE void sl_hal_lcd_enable_display_counter(void)
{
  // Ensure no internal sync is in progress.
  sl_hal_lcd_wait_load_busy();

  LCD->BACTRL_SET = LCD_BACTRL_DISPLAYCNTEN;
}

/***************************************************************************//**
 * Disable the LCD Display counter.
 ******************************************************************************/
__INLINE void sl_hal_lcd_disable_display_counter(void)
{
  // Ensure no internal sync is in progress.
  sl_hal_lcd_wait_load_busy();

  LCD->BACTRL_CLR = LCD_BACTRL_DISPLAYCNTEN;
}

/***************************************************************************//**
 * Start the synchronization process.
 *
 * @param[in] auto_load Flag indicating if the synchronization is started manually
 *                      with CMD.LOAD (false) or if the synchronization is managed
 *                      automatically by Auto Load (true).
 * @param[in] load_addr Address which will start the synchronization from
 *                      CLK_BUS to CLK_PER when Auto Load is selected.
 *                      This argument has no effect if 'autoload' is false.
 *
 * @note This function will disables LCD controller.
 ******************************************************************************/
__INLINE void sl_hal_lcd_start_sync(bool auto_load,
                                    sl_hal_lcd_load_address_t load_addr)
{
  // Ensure no synchronization in progress.
  sl_hal_lcd_wait_load_busy();

  if (auto_load) {
    sl_hal_lcd_disable(); // Ensure LCD disabled before writing to register.
    sl_hal_lcd_wait_ready();
    LCD->UPDATECTRL_CLR = _LCD_UPDATECTRL_LOADADDR_MASK;
    LCD->UPDATECTRL |= load_addr << _LCD_UPDATECTRL_LOADADDR_SHIFT;
    LCD->UPDATECTRL_SET = LCD_UPDATECTRL_AUTOLOAD;
    sl_hal_lcd_enable();
  } else {
    // Start synchronization from HV registers to CLK_PER domain.
    LCD->CMD_SET = LCD_CMD_LOAD;
  }
}

/***************************************************************************//**
 * Stop the synchronization process.
 *
 * @param[in] auto_load Flag indicating if the synchronization is stopped
 *                      manually with CMD.CLEAR (false) or if the synchronization
 *                      managed by Auto Load is disabled (true).
 ******************************************************************************/
__INLINE void sl_hal_lcd_stop_sync(bool auto_load)
{
  if (auto_load) {
    // Autoload cannot be disabled if synchronization in progress.
    sl_hal_lcd_wait_load_busy();

    LCD->UPDATECTRL_CLR = LCD_UPDATECTRL_AUTOLOAD;
    LCD->UPDATECTRL_CLR = _LCD_UPDATECTRL_LOADADDR_MASK;
  } else {
    LCD->CMD_SET = LCD_CMD_CLEAR;
  }
}

/***************************************************************************//**
 * Get pending LCD interrupt flags.
 *
 * @return Pending LCD interrupt sources. Returns a set of interrupt flags OR-ed
 *         together for multiple interrupt sources in the LCD module (LCD_IFS_nnn).
 ******************************************************************************/
__INLINE uint32_t sl_hal_lcd_get_pending_interrupts(void)
{
  return LCD->IF;
}

/***************************************************************************//**
 * Get enabled and pending LCD interrupt flags.
 *
 * @details Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @note Event bits are not cleared by the use of this function.
 *
 * @return Pending and enabled LCD interrupt sources.
 *         Return value is the bitwise AND combination of
 *         - the OR combination of enabled interrupt sources in LCD_IEN_nnn
 *         register (LCD_IEN_nnn) and
 *         - the bitwise OR combination of valid interrupt flags of LCD module
 *         (LCD_IF_nnn).
 ******************************************************************************/
__INLINE uint32_t sl_hal_lcd_get_enabled_pending_interrupts(void)
{
  uint32_t ien;

  // Store the LCD->IEN in temporary variable to define the explicit order
  // of volatile accesses.
  ien = LCD->IEN;

  // Bitwise AND of pending and enabled interrupts.
  return LCD->IF & ien;
}

/***************************************************************************//**
 * Set one or more pending LCD interrupts from SW.
 *
 * @param[in] flags LCD interrupt sources to set to pending. Use a set of interrupt flags
 *                  OR-ed together to set multiple interrupt sources for the LCD module
 *                  (LCD_IFS_nnn).
 ******************************************************************************/
__INLINE void sl_hal_lcd_set_interrupts(uint32_t flags)
{
  LCD->IF_SET = flags;
}

/***************************************************************************//**
 * Enable LCD interrupts.
 *
 * @param[in] flags LCD interrupt sources to enable. Use a set of interrupt flags OR-ed
 *                  together to set multiple interrupt sources for LCD module (LCD_IFS_nnn).
 ******************************************************************************/
__INLINE void sl_hal_lcd_enable_interrupts(uint32_t flags)
{
  LCD->IEN_SET = flags;
}

/***************************************************************************//**
 * Disable LCD interrupts.
 *
 * @param[in] flags LCD interrupt sources to disable. Use a set of interrupt flags
 *                  OR-ed together to disable multiple interrupt sources for LCD module
 *                  (LCD_IFS_nnn).
 ******************************************************************************/
__INLINE void sl_hal_lcd_disable_interrupts(uint32_t flags)
{
  LCD->IEN_CLR = flags;
}

/***************************************************************************//**
 * Clear one or more interrupt flags.
 *
 * @param[in] flags LCD interrupt sources to clear. Use a set of interrupt flags OR-ed
 *                  together to clear multiple interrupt sources for LCD module
 *                  (LCD_IFS_nnn).
 ******************************************************************************/
__INLINE void sl_hal_lcd_clear_interrupts(uint32_t flags)
{
  LCD->IF_CLR = flags;
}

/***************************************************************************//**
 * Enable LCD Direct Segment Control.
 *
 * @note This function will disable the LCD controller before writing to register.
 *
 * @note LCD controller Direct Segment Control Segment and COM line bias levels
 *       need to be set explicitly with sl_hal_lcd_set_bias_segment() and
 *       sl_hal_lcd_set_bias_com() function calls respectively.
 ******************************************************************************/
__INLINE void sl_hal_lcd_enable_dsc(void)
{
  sl_hal_lcd_disable(); // Ensure LCD disabled before writing to register.

  LCD->CTRL_SET = LCD_CTRL_DSC;
}

/***************************************************************************//**
 * Disable LCD Direct Segment Control.
 *
 * @note This function will disable the LCD controller before writing to register.
 ******************************************************************************/
__INLINE void sl_hal_lcd_disable_dsc(void)
{
  sl_hal_lcd_disable(); // Ensure LCD disabled before writing to register.

  LCD->CTRL_CLR = LCD_CTRL_DSC;
}

/***************************************************************************//**
 * Set a given DMA mode operation.
 *
 * @param[in] mode DMA mode.
 ******************************************************************************/
__INLINE void sl_hal_lcd_set_dma_mode(sl_hal_lcd_dma_mode_t mode)
{
  LCD->BIASCTRL_CLR = _LCD_BIASCTRL_DMAMODE_MASK;
  LCD->BIASCTRL_SET = mode << _LCD_BIASCTRL_DMAMODE_SHIFT;
}

/***************************************************************************//**
 * Configure the bias level on the LCD panel.
 *
 * @param[in] bias The bias level.
 ******************************************************************************/
__INLINE void sl_hal_lcd_bias_set(sl_hal_lcd_bias_t bias)
{
  EFM_ASSERT(LCD->EN != LCD_EN_EN);

  LCD->DISPCTRL = (LCD->DISPCTRL & ~_LCD_DISPCTRL_BIAS_MASK) | (bias << _LCD_DISPCTRL_BIAS_SHIFT);
}

/***************************************************************************//**
 * Configure the contrast level on the LCD panel.
 *
 * @param[in] level The contrast level in range 0-31.
 ******************************************************************************/
__INLINE void sl_hal_lcd_set_contrast(uint8_t level)
{
  EFM_ASSERT(level < 32);

  LCD->BIASCTRL = (LCD->BIASCTRL & ~_LCD_BIASCTRL_VLCD_MASK)
                  | (level << _LCD_BIASCTRL_VLCD_SHIFT);
}

/***************************************************************************//**
 * Configure the charge redistribution cycles for the LCD panel.
 *
 * @note The LCD controller need to be disabled before running this function.
 *
 * @param[in] cycles Charge redistribution cycles, range 0-4.
 ******************************************************************************/
__INLINE void sl_hal_lcd_set_charge_redistribution_cycle(uint8_t cycles)
{
  EFM_ASSERT(LCD->EN != LCD_EN_EN);
  EFM_ASSERT(cycles <= 4);

  LCD->DISPCTRL = (LCD->DISPCTRL & ~_LCD_DISPCTRL_CHGRDST_MASK)
                  | ((uint32_t)cycles << _LCD_DISPCTRL_CHGRDST_SHIFT);
}

/***************************************************************************//**
 * Configure the mode for the LCD panel.
 *
 * @param[in] mode A mode.
 ******************************************************************************/
__INLINE void sl_hal_lcd_set_mode(sl_hal_lcd_mode_t mode)
{
  LCD->BIASCTRL = (LCD->BIASCTRL & ~_LCD_BIASCTRL_MODE_MASK) | (mode << _LCD_BIASCTRL_MODE_SHIFT);
}

/** @} (end addtogroup lcd) */

#ifdef __cplusplus
}
#endif

/***************************************************************************//**
 * @addtogroup lcd LCD - Liquid Crystal Display
 * @{
 *
 * @n @section lcd_example Example
 *  This example demonstrates initialization, basic configuration, and usage of the
 *  LCD peripheral. It shows how to:
 *  - Initialize the LCD with default settings
 *  - Show a blink animation of segment 0-7 of com line 0.
 *
 * @code{.c}
 * #include "sl_hal_lcd.h"
 * #include "sl_clock_manager.h"
 * void lcd_example(void)
 * {
 *   // Enable clock to LCD module.
 *   sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LCD);
 *   // Disable interrupts.
 *   sl_hal_lcd_disable_interrupts(_LCD_IF_MASK);
 *   // Initialize lcd configuration structure.
 *   sl_hal_lcd_init_t init = {
 *     .mux = SL_HAL_LCD_MUX_QUADRUPLEX,
 *     .bias = SL_HAL_LCD_BIAS_ONE_THIRD,
 *     .wave = SL_HAL_LCD_WAVE_LOW_POWER,
 *     .mode = SL_HAL_LCD_MODE_CHARGE_PUMP,
 *     .charge_redistribution = SL_HAL_LCD_CHARGE_REDISTRIBUTION_THREE_CYCLE,
 *     .frame_rate_divider = 8,
 *     .contrast_level = 25,
 *     .clock_prescaler = 128
 *   };
 *   sl_hal_lcd_frame_counter_init_t fc_init = {
 *     .top = 48,
 *     .prescale = SL_HAL_LCD_FRAME_COUNTER_PRESCALE_DIV1
 *   };
 *
 *   // Disable LCD before config.
 *   sl_hal_lcd_disable();
 *   sl_hal_lcd_wait_ready();
 *   // Initialize LCD.
 *   sl_hal_lcd_init(&init);
 *   // Initialize frame counter.
 *   sl_hal_lcd_init_frame_counter(&fc_init);
 *   // Enable the LCD.
 *   sl_hal_lcd_enable();
 *   // Enable frame counter.
 *   sl_hal_lcd_enable_frame_counter();
 *
 *   // Enable display segment.
 *   sl_hal_lcd_enable_com_line(0);
 *   for(uint8_t segment_num = 0; segment_num < 8; segment_num++) {
 *     sl_hal_lcd_segment_enable(segment_num);
 *   }
 *   
 *   // Initialize lcd blink animation configuration structure.
 *   sl_hal_lcd_animation_init_t animation_init = {
 *       .A_reg = 0x80,
 *       .A_shift = SL_HAL_LCD_ANIMATION_SHIFT_LEFT,
 *       .B_reg = 0x00,
 *       .B_shift = SL_HAL_LCD_ANIMATION_SHIFT_NONE,
 *       .animation_logic = SL_HAL_LCD_ANIMATION_LOGIC_OR,
 *       .start_segment = SL_HAL_LCD_ANIMATION_LOCATION_SEG0_TO_7
 *   };
 *   sl_hal_lcd_init_animation(&animation_init);
 *   sl_hal_lcd_animation_enable();
 * }
 * @endcode
 *
 * @} (end addtogroup lcd)
 ******************************************************************************/

#endif /* defined(LCD_COUNT) && (LCD_COUNT > 0) */
#endif /* SL_HAL_LCD_H */
