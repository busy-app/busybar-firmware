/***************************************************************************//**
 * @file
 * @brief Universal asynchronous receiver/transmitter (EUSART) peripheral API
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

#ifndef SL_HAL_EUSART_H
#define SL_HAL_EUSART_H

#include "em_device.h"

#if defined(EUART_PRESENT) || defined(EUSART_PRESENT)

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_assert.h"
#include <stdbool.h>
#include "sl_enum.h"

/***************************************************************************//**
 * @addtogroup eusart EUSART - Enhanced USART
 * @brief Enhanced Universal Synchronous/Asynchronous Receiver/Transmitter
 * @{
 ******************************************************************************/

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

/// Check if EUSART instance is valid.
#if defined(EUART_PRESENT)
  #define SL_HAL_EUSART_REF_VALID(ref)    ((ref) == EUART0)
#elif defined(EUSART_PRESENT)
  #define SL_HAL_EUSART_REF_VALID(ref)    (EUSART_NUM(ref) != -1)
#endif

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

#if defined(EUART_PRESENT)
#define EUSART_EM2_CAPABLE(ref)  (true)
#endif

#if defined(__CM33_REV) || defined(__CM55_REV)
#define SL_HAL_EUSART_PRS_SUPPORTED
#endif

// Define EUSART FIFO Depth information
#if !defined(EUSART_FIFO_DEPTH)
#if defined(EUART_PRESENT)
#define EUSART0_FIFO_DEPTH 4
#elif defined(EUSART_PRESENT)
#define EUSART0_FIFO_DEPTH 16
#endif /* EUART_PRESENT */
#define EUSART1_FIFO_DEPTH EUSART0_FIFO_DEPTH
#define EUSART2_FIFO_DEPTH EUSART0_FIFO_DEPTH
#define EUSART3_FIFO_DEPTH EUSART0_FIFO_DEPTH
#define EUSART4_FIFO_DEPTH EUSART0_FIFO_DEPTH

#define EUSART_FIFO_DEPTH(n)            (((n) == 0) ? EUSART0_FIFO_DEPTH   \
                                         : ((n) == 1) ? EUSART1_FIFO_DEPTH \
                                         : ((n) == 2) ? EUSART2_FIFO_DEPTH \
                                         : ((n) == 3) ? EUSART3_FIFO_DEPTH \
                                         : ((n) == 4) ? EUSART4_FIFO_DEPTH \
                                         : 0x0UL)
#endif /* EUSART_FIFO_DEPTH */

/** @endcond */

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/// Data bit selection.
SL_ENUM(sl_hal_eusart_data_bits_t) {
  SL_HAL_EUSART_DATA_BITS_7 = _EUSART_FRAMECFG_DATABITS_SEVEN,     ///< 7 data bits.
  SL_HAL_EUSART_DATA_BITS_8 = _EUSART_FRAMECFG_DATABITS_EIGHT,     ///< 8 data bits.
  SL_HAL_EUSART_DATA_BITS_9 = _EUSART_FRAMECFG_DATABITS_NINE,      ///< 9 data bits.
#if defined(_EUSART_CFG0_SYNC_MASK)
  SL_HAL_EUSART_DATA_BITS_10 = _EUSART_FRAMECFG_DATABITS_TEN,      ///< 10 data bits, SPI mode only.
  SL_HAL_EUSART_DATA_BITS_11 = _EUSART_FRAMECFG_DATABITS_ELEVEN,   ///< 11 data bits, SPI mode only.
  SL_HAL_EUSART_DATA_BITS_12 = _EUSART_FRAMECFG_DATABITS_TWELVE,   ///< 12 data bits, SPI mode only.
  SL_HAL_EUSART_DATA_BITS_13 = _EUSART_FRAMECFG_DATABITS_THIRTEEN, ///< 13 data bits, SPI mode only.
  SL_HAL_EUSART_DATA_BITS_14 = _EUSART_FRAMECFG_DATABITS_FOURTEEN, ///< 14 data bits, SPI mode only.
  SL_HAL_EUSART_DATA_BITS_15 = _EUSART_FRAMECFG_DATABITS_FIFTEEN,  ///< 15 data bits, SPI mode only.
  SL_HAL_EUSART_DATA_BITS_16 = _EUSART_FRAMECFG_DATABITS_SIXTEEN,  ///< 16 data bits, SPI mode only.
#endif
};

/// Parity selection.
SL_ENUM(sl_hal_eusart_parity_t) {
  SL_HAL_EUSART_NO_PARITY   = _EUSART_FRAMECFG_PARITY_NONE,    ///< No parity.
  SL_HAL_EUSART_EVEN_PARITY = _EUSART_FRAMECFG_PARITY_EVEN,    ///< Even parity.
  SL_HAL_EUSART_ODD_PARITY  = _EUSART_FRAMECFG_PARITY_ODD      ///< Odd parity.
};

/// Stop bits selection.
SL_ENUM(sl_hal_eusart_stop_bits_t) {
  SL_HAL_EUSART_STOP_BITS_0P5 = _EUSART_FRAMECFG_STOPBITS_HALF,        ///< 0.5 stop bits.
  SL_HAL_EUSART_STOP_BITS_1P5 = _EUSART_FRAMECFG_STOPBITS_ONEANDAHALF, ///< 1.5 stop bits.
  SL_HAL_EUSART_STOP_BITS_1 = _EUSART_FRAMECFG_STOPBITS_ONE,           ///< 1 stop bits.
  SL_HAL_EUSART_STOP_BITS_2 = _EUSART_FRAMECFG_STOPBITS_TWO            ///< 2 stop bits.
};

/// Oversampling selection, used for asynchronous operation.
SL_ENUM(sl_hal_eusart_ovs_t) {
  SL_HAL_EUSART_OVS_16 = _EUSART_CFG0_OVS_X16,     ///< 16x oversampling (normal).
  SL_HAL_EUSART_OVS_8  = _EUSART_CFG0_OVS_X8,      ///< 8x oversampling.
  SL_HAL_EUSART_OVS_6  = _EUSART_CFG0_OVS_X6,      ///< 6x oversampling.
  SL_HAL_EUSART_OVS_4  = _EUSART_CFG0_OVS_X4,      ///< 4x oversampling.
  SL_HAL_EUSART_OVS_0  = _EUSART_CFG0_OVS_DISABLE  ///< Oversampling disabled.
};

/// Loopback enable.
SL_ENUM(sl_hal_eusart_loopback_t) {
  SL_HAL_EUSART_LOOPBACK_ENABLE  = _EUSART_CFG0_LOOPBK_ENABLE,  ///< Enable loopback.
  SL_HAL_EUSART_LOOPBACK_DISABLE = _EUSART_CFG0_LOOPBK_DISABLE  ///< Disable loopback.
};

/// Majority vote enable.
SL_ENUM(sl_hal_eusart_majority_vote_t) {
  SL_HAL_EUSART_MAJORITY_VOTE_ENABLE = 0, ///< Enable majority vote for 16x, 8x and 6x oversampling modes.
  SL_HAL_EUSART_MAJORITY_VOTE_DISABLE = 1 ///< Disable majority vote for 16x, 8x and 6x oversampling modes.
};

/// Hardware flow control configuration.
SL_ENUM(sl_hal_eusart_hw_flow_control_t) {
  SL_HAL_EUSART_HW_FLOW_CONTROL_NONE = 0, ///< No Hardware Flow Control.
  SL_HAL_EUSART_HW_FLOW_CONTROL_CTS,      ///< CTS Hardware Flow Control.
  SL_HAL_EUSART_HW_FLOW_CONTROL_RTS,      ///< RTS Hardware Flow Control.
  SL_HAL_EUSART_HW_FLOW_CONTROL_CTS_RTS   ///< CTS and RTS Hardware Flow Control.
};

/// IO polarity selection.
SL_ENUM(sl_hal_eusart_invert_io_t) {
  /// Disable inversion on both RX and TX signals.
  SL_HAL_EUSART_INVERT_IO_DISABLE = (EUSART_CFG0_RXINV_DISABLE | EUSART_CFG0_TXINV_DISABLE) >> _EUSART_CFG0_RXINV_SHIFT,

  /// Invert RX signal, before receiver.
  SL_HAL_EUSART_INVERT_IO_RX_ENABLE = EUSART_CFG0_RXINV_ENABLE >> _EUSART_CFG0_RXINV_SHIFT,

  /// Invert TX signal, after transmitter.
  SL_HAL_EUSART_INVERT_IO_TX_ENABLE = EUSART_CFG0_TXINV_ENABLE >> _EUSART_CFG0_RXINV_SHIFT,

  /// Enable trigger on both receive and transmit.
  SL_HAL_EUSART_INVERT_IO_RXTX_ENABLE = (EUSART_CFG0_RXINV_ENABLE | EUSART_CFG0_TXINV_ENABLE) >> _EUSART_CFG0_RXINV_SHIFT
};

/// Auto TX delay transmission.
SL_ENUM(sl_hal_eusart_auto_tx_delay_t) {
  /// Frames are transmitted immediately.
  SL_HAL_EUSART_AUTO_TX_DELAY_NONE   = _EUSART_TIMINGCFG_TXDELAY_NONE,

  /// Transmission of new frames is delayed by a single bit period.
  SL_HAL_EUSART_AUTO_TX_DELAY_SINGLE = _EUSART_TIMINGCFG_TXDELAY_SINGLE,

  /// Transmission of new frames is delayed by a two bit periods.
  SL_HAL_EUSART_AUTO_TX_DELAY_DOUBLE = _EUSART_TIMINGCFG_TXDELAY_DOUBLE,

  /// Transmission of new frames is delayed by a three bit periods.
  SL_HAL_EUSART_AUTO_TX_DELAY_TRIPPLE = _EUSART_TIMINGCFG_TXDELAY_TRIPPLE
};

/// FIFO Interrupt and Status Watermark.
SL_ENUM(sl_hal_eusart_fifo_interrupt_watermark_t) {
  /// Interrupt and status level are set after 1 frame.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_ONE           = _EUSART_CFG1_RXFIW_ONEFRAME,

  /// Interrupt and status level are set after 2 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_TWO           = _EUSART_CFG1_RXFIW_TWOFRAMES,

  /// Interrupt and status level are set after 3 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_THREE         = _EUSART_CFG1_RXFIW_THREEFRAMES,

  /// Interrupt and status level are set after 4 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_FOUR          = _EUSART_CFG1_RXFIW_FOURFRAMES,

#if defined(EUSART_PRESENT)
  /// Interrupt and status level are set after 5 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_FIVE          = _EUSART_CFG1_RXFIW_FIVEFRAMES,

  /// Interrupt and status level are set after 6 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_SIX           = _EUSART_CFG1_RXFIW_SIXFRAMES,

  /// Interrupt and status level are set after 7 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_SEVEN         = _EUSART_CFG1_RXFIW_SEVENFRAMES,

  /// Interrupt and status level are set after 8 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_EIGHT         = _EUSART_CFG1_RXFIW_EIGHTFRAMES,

  /// Interrupt and status level are set after 9 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_NINE          = _EUSART_CFG1_RXFIW_NINEFRAMES,

  /// Interrupt and status level are set after 10 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_TEN           = _EUSART_CFG1_RXFIW_TENFRAMES,

  /// Interrupt and status level are set after 11 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_ELEVEN        = _EUSART_CFG1_RXFIW_ELEVENFRAMES,

  /// Interrupt and status level are set after 12 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_TWELVE        = _EUSART_CFG1_RXFIW_TWELVEFRAMES,

  /// Interrupt and status level are set after 13 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_THIRTEEN      = _EUSART_CFG1_RXFIW_THIRTEENFRAMES,

  /// Interrupt and status level are set after 14 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_FOURTEEN      = _EUSART_CFG1_RXFIW_FOURTEENFRAMES,

  /// Interrupt and status level are set after 15 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_FIFTEEN       = _EUSART_CFG1_RXFIW_FIFTEENFRAMES,

  /// Interrupt and status level are set after 16 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_SIXTEEN       = _EUSART_CFG1_RXFIW_SIXTEENFRAMES,

#if defined(_EUSART_CFG1_RXFIW_SEVENTEENFRAMES)
  /// Interrupt and status level are set after 17 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_SEVENTEEN     = _EUSART_CFG1_RXFIW_SEVENTEENFRAMES,

  /// Interrupt and status level are set after 18 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_EIGHTEEN      = _EUSART_CFG1_RXFIW_EIGHTEENFRAMES,

  /// Interrupt and status level are set after 19 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_NINETEEN      = _EUSART_CFG1_RXFIW_NINETEENFRAMES,

  /// Interrupt and status level are set after 20 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_TWENTY        = _EUSART_CFG1_RXFIW_TWENTYFRAMES,

  /// Interrupt and status level are set after 21 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_TWENTYONE     = _EUSART_CFG1_RXFIW_TWENTYONEFRAMES,

  /// Interrupt and status level are set after 22 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_TWENTYTWO     = _EUSART_CFG1_RXFIW_TWENTYTWOFRAMES,

  /// Interrupt and status level are set after 23 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_TWENTYTHREE   = _EUSART_CFG1_RXFIW_TWENTYTHREEFRAMES,

  /// Interrupt and status level are set after 24 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_TWENTYFOUR    = _EUSART_CFG1_RXFIW_TWENTYFOURFRAMES,

  /// Interrupt and status level are set after 25 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_TWENTYFIVE    = _EUSART_CFG1_RXFIW_TWENTYFIVEFRAMES,

  /// Interrupt and status level are set after 26 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_TWENTYSIX     = _EUSART_CFG1_RXFIW_TWENTYSIXFRAMES,

  /// Interrupt and status level are set after 27 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_TWENTYSEVEN   = _EUSART_CFG1_RXFIW_TWENTYSEVENFRAMES,

  /// Interrupt and status level are set after 28 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_TWENTYEIGHT   = _EUSART_CFG1_RXFIW_TWENTYEIGHTFRAMES,

  /// Interrupt and status level are set after 29 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_TWENTYNINE    = _EUSART_CFG1_RXFIW_TWENTYNINEFRAMES,

  /// Interrupt and status level are set after 30 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_THIRTY        = _EUSART_CFG1_RXFIW_THIRTYFRAMES,

  /// Interrupt and status level are set after 31 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_THIRTYONE     = _EUSART_CFG1_RXFIW_THIRTYONEFRAMES,

  /// Interrupt and status level are set after 32 frames.
  SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_THIRTYTWO     = _EUSART_CFG1_RXFIW_THIRTYTWOFRAMES,
#endif
#endif
};

/// IrDA frequency mode (high/low frequency).
SL_ENUM(sl_hal_eusart_irda_frequency_mode_t) {
  /// High frequency mode.
  SL_HAL_EUSART_IRDA_MODE_HF,

  /// Low frequency mode.
  SL_HAL_EUSART_IRDA_MODE_LF
};

/// Pulse width selection for IrDA mode.
SL_ENUM(sl_hal_eusart_irda_pulse_width_t) {
  /// IrDA pulse width is 1/16 for OVS=X16 and 1/8 for OVS=X8
  SL_HAL_EUSART_PULSE_WIDTH_ONE  = _EUSART_IRHFCFG_IRHFPW_ONE,

  /// IrDA pulse width is 2/16 for OVS=X16 and 2/8 for OVS=X8
  SL_HAL_EUSART_PULSE_WIDTH_TWO   = _EUSART_IRHFCFG_IRHFPW_TWO,

  /// IrDA pulse width is 3/16 for OVS=X16 and 3/8 for OVS=X8
  SL_HAL_EUSART_PULSE_WIDTH_THREE = _EUSART_IRHFCFG_IRHFPW_THREE,

  /// IrDA pulse width is 4/16 for OVS=X16 and 4/8 for OVS=X8
  SL_HAL_EUSART_PULSE_WIDTH_FOUR  = _EUSART_IRHFCFG_IRHFPW_FOUR
};

#if defined(EUSART_PRESENT)
/// Chip select assertion/de-assertion time.
SL_ENUM(sl_hal_eusart_cs_time_t) {
  /// Chip select is asserted/de-asserted half or 1 baud-time before transmission.
  SL_HAL_EUSART_CS_TIME_ZERO = _EUSART_TIMINGCFG_CSSETUP_ZERO,

  /// Chip select is asserted/de-asserted 1 additional baud-time before transmission.
  SL_HAL_EUSART_CS_TIME_ONE = _EUSART_TIMINGCFG_CSSETUP_ONE,

  /// Chip select is asserted/de-asserted 2 additional baud-time before transmission.
  SL_HAL_EUSART_CS_TIME_TWO = _EUSART_TIMINGCFG_CSSETUP_TWO,

  /// Chip select is asserted/de-asserted 3 additional baud-time before transmission.
  SL_HAL_EUSART_CS_TIME_THREE = _EUSART_TIMINGCFG_CSSETUP_THREE,

  /// Chip select is asserted/de-asserted 4 additional baud-time before transmission.
  SL_HAL_EUSART_CS_TIME_FOUR = _EUSART_TIMINGCFG_CSSETUP_FOUR,

  /// Chip select is asserted/de-asserted 5 additional baud-time before transmission.
  SL_HAL_EUSART_CS_TIME_FIVE = _EUSART_TIMINGCFG_CSSETUP_FIVE,

  /// Chip select is asserted/de-asserted 6 additional baud-time before transmission.
  SL_HAL_EUSART_CS_TIME_SIX = _EUSART_TIMINGCFG_CSSETUP_SIX,

  /// Chip select is asserted/de-asserted 7 additional baud-time before transmission.
  SL_HAL_EUSART_CS_TIME_SEVEN = _EUSART_TIMINGCFG_CSSETUP_SEVEN,
};

/// Inter-character spacing after each TX frame while TX FIFO is not empty.
SL_ENUM(sl_hal_eusart_inter_character_space_t) {
  /// There is no space between characters..
  SL_HAL_EUSART_ICS_TIME_ZERO = _EUSART_TIMINGCFG_ICS_ZERO,

  /// Create a space on 1 baud-times between frames.
  SL_HAL_EUSART_ICS_TIME_ONE = _EUSART_TIMINGCFG_ICS_ONE,

  /// Create a space on 2 baud-times between frames.
  SL_HAL_EUSART_ICS_TIME_TWO = _EUSART_TIMINGCFG_ICS_TWO,

  /// Create a space on 3 baud-times between frames.
  SL_HAL_EUSART_ICS_TIME_THREE = _EUSART_TIMINGCFG_ICS_THREE,

  /// Create a space on 4 baud-times between frames.
  SL_HAL_EUSART_ICS_TIME_FOUR = _EUSART_TIMINGCFG_ICS_FOUR,

  /// Create a space on 5 baud-times between frames.
  SL_HAL_EUSART_ICS_TIME_FIVE = _EUSART_TIMINGCFG_ICS_FIVE,

  /// Create a space on 6 baud-times between frames.
  SL_HAL_EUSART_ICS_TIME_SIX = _EUSART_TIMINGCFG_ICS_SIX,

  /// Create a space on 7 baud-times between frames.
  SL_HAL_EUSART_ICS_TIME_SEVEN = _EUSART_TIMINGCFG_ICS_SEVEN,
};

/// Chip select polarity.
SL_ENUM(sl_hal_eusart_cs_polarity_t) {
  /// Chip select active low.
  SL_HAL_EUSART_CS_ACTIVE_LOW = _EUSART_CFG2_CSINV_AL,

  /// Chip select active high.
  SL_HAL_EUSART_CS_ACTIVE_HIGH = _EUSART_CFG2_CSINV_AH,
};

/// Clock polarity/phase mode.
SL_ENUM(sl_hal_eusart_clock_mode_t) {
  /// Clock idle low, sample on rising edge.
  SL_HAL_EUSART_CLOCK_MODE_0 = (EUSART_CFG2_CLKPOL_IDLELOW | EUSART_CFG2_CLKPHA_SAMPLELEADING) >> _EUSART_CFG2_CLKPOL_SHIFT,

  /// Clock idle low, sample on falling edge.
  SL_HAL_EUSART_CLOCK_MODE_1 = (EUSART_CFG2_CLKPOL_IDLELOW | EUSART_CFG2_CLKPHA_SAMPLETRAILING) >> _EUSART_CFG2_CLKPOL_SHIFT,

  /// Clock idle high, sample on falling edge.
  SL_HAL_EUSART_CLOCK_MODE_2 = (EUSART_CFG2_CLKPOL_IDLEHIGH | EUSART_CFG2_CLKPHA_SAMPLELEADING) >> _EUSART_CFG2_CLKPOL_SHIFT,

  /// Clock idle high, sample on rising edge.
  SL_HAL_EUSART_CLOCK_MODE_3 = (EUSART_CFG2_CLKPOL_IDLEHIGH | EUSART_CFG2_CLKPHA_SAMPLETRAILING) >> _EUSART_CFG2_CLKPOL_SHIFT
};

#if defined(EUSART_DALICFG_DALIEN)
/// DALI RX/TX databits (8-32).
SL_ENUM(sl_hal_eusart_dali_data_bits_t) {
  SL_HAL_EUSART_DALI_DATA_BITS_8 =  _EUSART_DALICFG_DALITXDATABITS_EIGHT,       ///< Each frame contains 8  data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_9 =  _EUSART_DALICFG_DALITXDATABITS_NINE,        ///< Each frame contains 9  data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_10 = _EUSART_DALICFG_DALITXDATABITS_TEN,         ///< Each frame contains 10 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_11 = _EUSART_DALICFG_DALITXDATABITS_ELEVEN,      ///< Each frame contains 11 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_12 = _EUSART_DALICFG_DALITXDATABITS_TWELVE,      ///< Each frame contains 12 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_13 = _EUSART_DALICFG_DALITXDATABITS_THIRTEEN,    ///< Each frame contains 13 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_14 = _EUSART_DALICFG_DALITXDATABITS_FOURTEEN,    ///< Each frame contains 14 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_15 = _EUSART_DALICFG_DALITXDATABITS_FIFTEEN,     ///< Each frame contains 15 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_16 = _EUSART_DALICFG_DALITXDATABITS_SIXTEEN,     ///< Each frame contains 16 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_17 = _EUSART_DALICFG_DALITXDATABITS_SEVENTEEN,   ///< Each frame contains 17 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_18 = _EUSART_DALICFG_DALITXDATABITS_EIGHTEEN,    ///< Each frame contains 18 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_19 = _EUSART_DALICFG_DALITXDATABITS_NINETEEN,    ///< Each frame contains 19 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_20 = _EUSART_DALICFG_DALITXDATABITS_TWENTY,      ///< Each frame contains 20 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_21 = _EUSART_DALICFG_DALITXDATABITS_TWENTYONE,   ///< Each frame contains 21 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_22 = _EUSART_DALICFG_DALITXDATABITS_TWENTYTWO,   ///< Each frame contains 22 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_23 = _EUSART_DALICFG_DALITXDATABITS_TWENTYTHREE, ///< Each frame contains 23 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_24 = _EUSART_DALICFG_DALITXDATABITS_TWENTYFOUR,  ///< Each frame contains 24 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_25 = _EUSART_DALICFG_DALITXDATABITS_TWENTYFIVE,  ///< Each frame contains 25 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_26 = _EUSART_DALICFG_DALITXDATABITS_TWENTYSIX,   ///< Each frame contains 26 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_27 = _EUSART_DALICFG_DALITXDATABITS_TWENTYSEVEN, ///< Each frame contains 27 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_28 = _EUSART_DALICFG_DALITXDATABITS_TWENTYEIGHT, ///< Each frame contains 28 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_29 = _EUSART_DALICFG_DALITXDATABITS_TWENTYNINE,  ///< Each frame contains 29 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_30 = _EUSART_DALICFG_DALITXDATABITS_THIRTY,      ///< Each frame contains 30 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_31 = _EUSART_DALICFG_DALITXDATABITS_THIRTYONE,   ///< Each frame contains 31 data bits.
  SL_HAL_EUSART_DALI_DATA_BITS_32 = _EUSART_DALICFG_DALITXDATABITS_THIRTYTWO,   ///< Each frame contains 32 data bits.
};
#endif
#endif

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// UART advanced initialization structure.
typedef struct {
  /// Enable the collision Detection feature.
  /// Internal (setting loopback_enable) or external loopback must be done to use this feature.
  bool collision_detect;

  /// If true, data will be sent with most significant bit first.
  bool msb_first;

#if defined(_EUSART_CFG1_RXDMAWU_MASK)
  /// Enable the automatic wake up from EM2 to EM1 for DMA RX operation.
  bool dma_wakeup_on_rx;
#endif

#if defined(_EUSART_CFG1_TXDMAWU_MASK)
  /// Enable the automatic wake up from EM2 to EM1 for DMA TX operation.
  bool dma_wakeup_on_tx;
#endif

  /// Enable DMA requests blocking while framing or parity errors.
  bool dma_halt_on_error;

  /// Enable automatic tristating of transmitter output when there is nothing to transmit.
  bool tx_auto_tristate;

  /// Enable Multiprocessor mode. Address and data filtering using the 9th bit.
  bool multi_processor_enable;

  /// Multiprocessor address bit value. If true, 9th bit of address frame must be 1, 0 otherwise.
  bool multi_processor_address_bit_high;

  /// Enable EUSART capability to use a PRS channel as an input data line for the receiver.
  /// PRS channel and RX GPIO must be configured separately using PRS/GPIO API.
  bool prs_rx_enable;

  /// PRS Channel used to transmit data from PRS to the EUSART.
  uint8_t prs_rx_channel;

  /// Start frame that will enable RX operation. 0x00 Disable this feature.
  uint8_t start_frame;

  /// Hardware flow control mode.
  sl_hal_eusart_hw_flow_control_t hw_flow_control_mode;

  /// Enable inversion of RX and/or TX signals.
  sl_hal_eusart_invert_io_t invert_io;

  /// Auto TX delay in baud cycles before new transfers. Frames sent back-to-back are not delayed.
  sl_hal_eusart_auto_tx_delay_t auto_tx_delay;

  /// Interrupt and status level of the Receive FIFO.
  sl_hal_eusart_fifo_interrupt_watermark_t rx_fifo_watermark;

  /// Interrupt and status level of the Transmit FIFO.
  sl_hal_eusart_fifo_interrupt_watermark_t tx_fifo_watermark;

#if defined(_EUSART_CFG1_RXTIMEOUT_MASK)
  /// Timeout in number of frames between RX frames. 0 disables this feature.
  uint8_t rx_timeout;
#endif

#if defined(_EUSART_CFG4_MASK)
  /// Delay in baud cycles before enabling TX after reception. 0 disables this feature.
  uint8_t response_delay;
#endif

#if defined(_EUSART_CFG3_MASK)
  /// Restarts the response timeout timer when a timeout is reached until EUSART_CMD_TSTOP.
  bool restart_response_timeout;

  /// Timeout in EUSART clocks for a response after a transmission. 0 disables this feature.
  uint16_t response_timeout;
#endif
} sl_hal_eusart_uart_advanced_init_t;

/// UART initialization structure.
typedef struct {
  /// Clock divider.
  uint32_t clock_div;

  /// Oversampling used.
  sl_hal_eusart_ovs_t oversampling;

  /// Number of data bits in frame.
  sl_hal_eusart_data_bits_t data_bits;

  /// Parity mode to use.
  sl_hal_eusart_parity_t   parity;

  /// Number of stop bits to use.
  sl_hal_eusart_stop_bits_t stop_bits;

  /// Majority Vote can be disabled for 16x, 8x and 6x oversampling modes.
  sl_hal_eusart_majority_vote_t majority_vote;

  /// Enable Loop Back configuration.
  sl_hal_eusart_loopback_t loopback_enable;

  /// Advanced initialization structure. It can be NULL.
  sl_hal_eusart_uart_advanced_init_t *advanced_config;
} sl_hal_eusart_uart_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_eusart_uart_advanced_init_t sl_hal_eusart_uart_advanced_config_t;
typedef sl_hal_eusart_uart_init_t sl_hal_eusart_uart_config_t;
/** @endcond */

/// IrDA Initialization structure.
typedef struct {
  /// Set to enable filter on IrDA demodulator (hf mode).
  bool hf_rx_filter_enable;

  /// Enable the IrDA in low or high frequency mode. Only RX operation is enabled in low frequency mode.
  sl_hal_eusart_irda_frequency_mode_t irda_mode;

  /// Configure the pulse width generated by the IrDA modulator as a fraction
  /// of the configured EUSART bit period.
  sl_hal_eusart_irda_pulse_width_t pulse_width;

  /// General EUSART initialization structure.
  sl_hal_eusart_uart_init_t eusart_init;
} sl_hal_eusart_irda_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_eusart_irda_init_t sl_hal_eusart_irda_config_t;
/** @endcond */

#if defined(EUSART_DALICFG_DALIEN)
/// DALI Initialization structure.
typedef struct {
  /// Enable the DALI low frequency mode.
  bool dali_low_frequency_enable;

  /// Number of TX data bits in frame.
  sl_hal_eusart_dali_data_bits_t tx_data_bits;

  /// Number of RX data bits in frame.
  sl_hal_eusart_dali_data_bits_t rx_data_bits;

  /// General EUSART initialization structure.
  sl_hal_eusart_uart_init_t eusart_init;
} sl_hal_eusart_dali_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_eusart_dali_init_t sl_hal_eusart_dali_config_t;
/** @endcond */
#endif /* defined(EUSART_DALICFG_DALIEN) */

#if defined(_EUSART_CFG0_SYNC_MASK)
/// SPI Advanced initialization structure.
typedef struct {
  /// Enable automatic chip select. CS is managed by the peripheral.
  bool auto_cs_enable;

  /// If true, data will be sent with most significant bit first.
  bool msb_first;

  /// Enable the automatic wake up from EM2 to EM1 for DMA RX operation.
  /// Only applicable to EM2 (low frequency) capable EUSART instances.
  bool dma_wakeup_on_rx;

  /// Enable AUTOTX mode. Transmits as long as the RX FIFO is not full.
  ///  Generates underflow interrupt if the TX FIFO is empty.
  bool auto_tx_enable;

  /// Force load the first FIFO value.
  bool force_load;

  /// Enable EUSART capability to use a PRS channel as an input data line for the receiver.
  /// PRS channel and RX GPIO must be configured separately using PRS/GPIO API.
  bool prs_rx_enable;

  /// Enable EUSART capability to use a PRS channel as an input SPI Clock.
  /// Slave mode only.
  bool prs_clock_enable;

  /// PRS Channel used to transmit data from PRS to the EUSART.
  uint8_t prs_rx_channel;

  /// Enable EUSART capability to use a PRS channel as an input SPI Clock.
  /// Slave mode only.
  uint8_t prs_clock_channel;

  /// Setup window in bus clock cycles before the sampling edge of SCLK at word-boundary to avoid force load error.
  uint8_t setup_window;

  /// Enable inversion of RX and/or TX signals.
  sl_hal_eusart_invert_io_t invert_io;

  /// Chip select polarity
  sl_hal_eusart_cs_polarity_t cs_polarity;

  /// Auto CS setup time (before transmission) in baud cycles.
  sl_hal_eusart_cs_time_t auto_cs_setup_time;

  /// Auto CS hold time (after transmission) in baud cycles.
  sl_hal_eusart_cs_time_t auto_cs_hold_time;

  /// Inter-frame time in baud cycles.
  sl_hal_eusart_inter_character_space_t auto_inter_frame_time;

  /// Auto TX delay in baud cycles before new transfers.
  sl_hal_eusart_auto_tx_delay_t auto_tx_delay;

  /// Default transmitted data when the TXFIFO is empty.
  uint16_t default_tx_data;

  /// Interrupt and status level of the Receive FIFO.
  sl_hal_eusart_fifo_interrupt_watermark_t rx_fifo_watermark;

  /// Interrupt and status level of the Transmit FIFO.
  sl_hal_eusart_fifo_interrupt_watermark_t tx_fifo_watermark;
} sl_hal_eusart_spi_advanced_init_t;

/// SPI Initialization structure.
typedef struct {
  /// Select to operate in master or slave mode.
  bool master;

  /// Clock Divider.
  uint32_t clock_div;

  /// Number of data bits in frame.
  sl_hal_eusart_data_bits_t data_bits;

  /// Clock polarity/phase mode.
  sl_hal_eusart_clock_mode_t clock_mode;

  /// Enable Loop Back configuration.
  sl_hal_eusart_loopback_t loopback_enable;

  /// Advanced initialization structure. It can be NULL.
  sl_hal_eusart_spi_advanced_init_t *advanced_config;
} sl_hal_eusart_spi_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_eusart_spi_advanced_init_t sl_hal_eusart_spi_advanced_config_t;
typedef sl_hal_eusart_spi_init_t sl_hal_eusart_spi_config_t;
/** @endcond */

#endif /* defined(_EUSART_CFG0_SYNC_MASK) */

#if defined(_EUSART_CFG0_SCMODE_MASK)
/// Smart Card Initialization structure.
typedef struct {
  /// General EUSART initialization structure.
  sl_hal_eusart_uart_init_t eusart_init;

  /// Enable retransmission of frame with error.
  bool retransmit_enable;
} sl_hal_eusart_smartcard_init_t;
#endif

#if defined(_EUSART_LINCFG_MASK)
/// LIN Initialization structure.
typedef struct {
  /// General EUSART initialization structure.
  sl_hal_eusart_uart_init_t eusart_init;

#if defined(_EUSART_CFG3_BREAKTIMEOUT_MASK)
  /// Timeout for break detection in baud cycles.
  uint8_t break_timeout;
#endif
} sl_hal_eusart_lin_init_t;
#endif

/// Default configuration for EUSART initialization structure in UART mode with high-frequency clock.
#define SL_HAL_EUSART_UART_INIT_DEFAULT_HF                                 \
  {                                                                        \
    0,                                  /* Clock divider default value. */ \
    SL_HAL_EUSART_OVS_16,               /* Oversampling x16. */            \
    SL_HAL_EUSART_DATA_BITS_8,          /* 8 data bits. */                 \
    SL_HAL_EUSART_NO_PARITY,            /* No parity. */                   \
    SL_HAL_EUSART_STOP_BITS_1,          /* 1 stop bit. */                  \
    SL_HAL_EUSART_MAJORITY_VOTE_ENABLE, /* Majority vote enabled. */       \
    SL_HAL_EUSART_LOOPBACK_DISABLE,     /* Loop back disabled. */          \
    NULL,                               /* Default advanced settings. */   \
  }

/// Default configuration for EUSART initialization structure in UART mode with low-frequency clock.
#define SL_HAL_EUSART_UART_INIT_DEFAULT_LF                                  \
  {                                                                         \
    0,                                   /* Clock divider default value. */ \
    SL_HAL_EUSART_OVS_0,                 /* Oversampling disabled. */       \
    SL_HAL_EUSART_DATA_BITS_8,           /* 8 data bits. */                 \
    SL_HAL_EUSART_NO_PARITY,             /* No parity. */                   \
    SL_HAL_EUSART_STOP_BITS_1,           /* 1 stop bit. */                  \
    SL_HAL_EUSART_MAJORITY_VOTE_DISABLE, /* Majority vote disabled. */      \
    SL_HAL_EUSART_LOOPBACK_DISABLE,      /* Loop back disabled. */          \
    NULL,                                /* Default advanced settings. */   \
  }

#if defined(_EUSART_CFG1_RXTIMEOUT_MASK)
#define SLI_HAL_EUSART_UART_RX_TIMEOUT_DEFAULT \
  0u,     /* No RX timeout. */
#else
#define SLI_HAL_EUSART_UART_RX_TIMEOUT_DEFAULT
#endif

#if defined(_EUSART_CFG3_MASK) && defined(_EUSART_CFG4_MASK)
#define SLI_HAL_EUSART_UART_RESPONSE_TIMER_DEFAULT \
  0u,      /* No response delay. */                \
  false,   /* Disable response timeout restart. */ \
  0u,      /* No response timeout. */
#else
#define SLI_HAL_EUSART_UART_RESPONSE_TIMER_DEFAULT
#endif

#if defined(_EUSART_CFG1_RXDMAWU_MASK) && defined(_EUSART_CFG1_TXDMAWU_MASK)
#define SLI_HAL_EUSART_UART_DMA_WAKEUP_DEFAULT \
  false,    /* No DMA wake up on reception. */ \
  false,    /* No DMA wake up on transmission. */
#else
#define SLI_HAL_EUSART_UART_DMA_WAKEUP_DEFAULT
#endif

/// Default configuration for EUSART advanced initialization structure in UART mode..
#define SL_HAL_EUSART_UART_ADVANCED_INIT_DEFAULT                                                                                      \
  {                                                                                                                                   \
    false,                                                /* Collision detection disabled. */                                         \
    false,                                                /* Data is sent with the least significant bit first. */                    \
    SLI_HAL_EUSART_UART_DMA_WAKEUP_DEFAULT                /* No DMA wake up on reception and transmission. */                         \
    false,                                                /* Halt DMA on error disabled. */                                           \
    false,                                                /* TX auto tristate disabled. */                                            \
    false,                                                /* Multiprocessor mode disabled. */                                         \
    false,                                                /* Multiprocessor address bit : 0.*/                                        \
    false,                                                /* Do not use PRS signal as RX signal.*/                                    \
    0u,                                                   /* EUSART RX connected to prs channel 0. */                                 \
    _EUSART_STARTFRAMECFG_STARTFRAME_DEFAULT,             /* No start frame.  */                                                      \
    SL_HAL_EUSART_HW_FLOW_CONTROL_NONE,                   /* Flow control disabled. */                                                \
    SL_HAL_EUSART_INVERT_IO_DISABLE,                      /* RX and TX signal active high. */                                         \
    SL_HAL_EUSART_AUTO_TX_DELAY_NONE,                     /* Frames are transmitted immediately */                                    \
    SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_ONE,    /* RXFL status/IF set when RX FIFO has at least one frame in it */          \
    SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_ONE,    /* TXFL status/IF set when TX FIFO has space for at least one more frame */ \
    SLI_HAL_EUSART_UART_RX_TIMEOUT_DEFAULT                /* No RX timeout. */                                                        \
    SLI_HAL_EUSART_UART_RESPONSE_TIMER_DEFAULT            /* No response delay and no response timeout. */                            \
  }

#if defined(_EUSART_CFG0_SYNC_MASK)
/// Default configuration for EUSART initialization structure in SPI master mode with high-frequency clock.
#define SL_HAL_EUSART_SPI_MASTER_INIT_DEFAULT_HF                                 \
  {                                                                              \
    true,                           /* Master mode enabled. */                   \
    0,                              /* Clock divider default value. */           \
    SL_HAL_EUSART_DATA_BITS_8,      /* 8 data bits. */                           \
    SL_HAL_EUSART_CLOCK_MODE_0,     /* Clock idle low, sample on rising edge. */ \
    SL_HAL_EUSART_LOOPBACK_DISABLE, /* Loop back disabled. */                    \
    NULL,                           /* Default advanced settings. */             \
  }

/// Default configuration for EUSART initialization structure in SPI slave mode with high-frequency clock.
#define SL_HAL_EUSART_SPI_SLAVE_INIT_DEFAULT_HF                                  \
  {                                                                              \
    false,                          /* Master mode disabled. */                  \
    0,                              /* Clock divider default value. */           \
    SL_HAL_EUSART_DATA_BITS_8,      /* 8 data bits. */                           \
    SL_HAL_EUSART_CLOCK_MODE_0,     /* Clock idle low, sample on rising edge. */ \
    SL_HAL_EUSART_LOOPBACK_DISABLE, /* Loop back disabled. */                    \
    NULL,                           /* Default advanced settings. */             \
  }

/// Default advanced configuration for EUSART initialization structure in SPI mode with high-frequency clock.
#define SL_HAL_EUSART_SPI_ADVANCED_INIT_DEFAULT                                                                    \
  {                                                                                                                \
    true,                                                 /* AutoCS enabled. */                                    \
    false,                                                /* Data is sent with the least significant bit first. */ \
    false,                                                /* No DMA wake up on reception. */                       \
    false,                                                /* Do not use PRS signal as RX signal. */                \
    true,                                                 /* The first byte sent by the slave won't be the default value if a byte is
                                                             made available after chip select is asserted. */ \
    false,                                                /* Do not use PRS signal as SCLK signal. */         \
    false,                                                /* AutoTX disabled. */                              \
    0u,                                                   /* EUSART RX connected to prs channel 0. */         \
    1u,                                                   /* EUSART SCLCK tied to prs channel 1. */           \
    4u,                                                   /* Setup window before the sampling edge of SCLK at word-boundary to avoid
                                                             force load error. */                                                     \
    SL_HAL_EUSART_CS_ACTIVE_LOW,                          /* CS active low. */                                                        \
    SL_HAL_EUSART_INVERT_IO_DISABLE,                      /* RX and TX signal active High. */                                         \
    SL_HAL_EUSART_CS_TIME_ZERO,                           /* CS setup time is 0 baud cycles */                                        \
    SL_HAL_EUSART_CS_TIME_ZERO,                           /* CS hold time is 0 baud cycles */                                         \
    SL_HAL_EUSART_ICS_TIME_ZERO,                          /* Inter-frame time is 0 baud cycles */                                     \
    SL_HAL_EUSART_AUTO_TX_DELAY_NONE,                     /* Frames are transmitted immediately */                                    \
    0x00,                                                 /* Default transmitted data is 0. */                                        \
    SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_ONE,    /* RXFL status/IF set when RX FIFO has at least one frame in it */          \
    SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_ONE,    /* TXFL status/IF set when TX FIFO has space for at least one more frame */ \
  }
#endif

/// Default configuration for EUSART initialization structure in IrDA mode with high-frequency clock.
#define SL_HAL_EUSART_IRDA_INIT_DEFAULT_HF                                           \
  {                                                                                  \
    false,                              /* Disable high frequency mode RX filter. */ \
    SL_HAL_EUSART_IRDA_MODE_HF,         /* IrDA in HF mode. */                       \
    SL_HAL_EUSART_PULSE_WIDTH_ONE,      /* Pulse width is set to 1/16. */            \
    SL_HAL_EUSART_UART_INIT_DEFAULT_HF, /* Default high frequency configuration. */  \
  }

/// Default configuration for EUSART initialization structure in IrDA mode with low-frequency clock.
#define SL_HAL_EUSART_IRDA_INIT_DEFAULT_LF                                           \
  {                                                                                  \
    false,                              /* Disable high frequency mode RX filter. */ \
    SL_HAL_EUSART_IRDA_MODE_LF,         /* IrDA in LF mode. */                       \
    SL_HAL_EUSART_PULSE_WIDTH_ONE,      /* Pulse width is set to 1/16. */            \
    SL_HAL_EUSART_UART_INIT_DEFAULT_LF, /* Default high frequency configuration. */  \
  }

#if defined(_EUSART_DALICFG_DALIEN_MASK)
/// Default configuration for EUSART initialization structure in DALI mode with high-frequency clock.
#define SL_HAL_EUSART_DALI_ADVANCED_INIT_DEFAULT                                                                                    \
  {                                                                                                                                 \
    false,                                              /* Collision detection disabled. */                                         \
    true,                                               /* Data is sent with the most significant bit first. */                     \
    SLI_HAL_EUSART_UART_DMA_WAKEUP_DEFAULT                                                                                          \
    false,                                              /* Halt DMA on error disabled. */                                           \
    false,                                              /* TX auto tristate disabled. */                                            \
    false,                                              /* Do not use PRS signal as RX signal.*/                                    \
    false,                                              /* Multiprocessor mode disabled. */                                         \
    false,                                              /* Multiprocessor address bit : 0.*/                                        \
    0u,                                                 /* EUSART RX connected to prs channel 0. */                                 \
    _EUSART_STARTFRAMECFG_STARTFRAME_DEFAULT,           /* No start frame.  */                                                      \
    SL_HAL_EUSART_HW_FLOW_CONTROL_NONE,                 /* Flow control disabled. */                                                \
    SL_HAL_EUSART_INVERT_IO_DISABLE,                    /* RX and TX signal active high. */                                         \
    SL_HAL_EUSART_AUTO_TX_DELAY_NONE,                   /* Frames are transmitted immediately */                                    \
    SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_ONE,  /* RXFL status/IF set when RX FIFO has at least one frame in it */          \
    SL_HAL_EUSART_FIFO_INTERRUPT_WATERMARK_FRAMES_ONE,  /* TXFL status/IF set when TX FIFO has space for at least one more frame */ \
    0u,                                                 /* No RX timeout. */                                                        \
    SLI_HAL_EUSART_UART_RESPONSE_TIMER_DEFAULT                                                                                      \
  }

/// Default configuration for EUSART initialization structure in DALI mode with high-frequency clock.
#define SL_HAL_EUSART_DALI_INIT_DEFAULT_HF                                     \
  {                                                                            \
    false,                              /* Disable DALI low frequency mode. */ \
    SL_HAL_EUSART_DALI_DATA_BITS_16,    /* TX 16 data bits. */                 \
    SL_HAL_EUSART_DALI_DATA_BITS_8,     /* RX 8 data bits. */                  \
    SL_HAL_EUSART_UART_INIT_DEFAULT_HF, /* Default advanced settings. */       \
  }                                                                            \

/// Default configuration for EUSART initialization structure in DALI mode with low-frequency clock.
#define SL_HAL_EUSART_DALI_INIT_DEFAULT_LF                                    \
  {                                                                           \
    true,                               /* Enable DALI low frequency mode. */ \
    SL_HAL_EUSART_DALI_DATA_BITS_16,    /* TX 16 data bits. */                \
    SL_HAL_EUSART_DALI_DATA_BITS_8,     /* RX 8 data bits. */                 \
    SL_HAL_EUSART_UART_INIT_DEFAULT_LF, /* Default advanced settings. */      \
  }
#endif

#if defined(_EUSART_CFG0_SCMODE_MASK)
/// Default configuration for EUSART initialization structure in Smart Card mode with high-frequency clock.
#define SL_HAL_EUSART_SMARTCARD_INIT_DEFAULT_HF                                             \
  {                                                                                         \
    SL_HAL_EUSART_UART_INIT_DEFAULT_HF,         /* Default high frequency configuration. */ \
    false,                                      /* Disable retransmission by default. */    \
  }

/// Default configuration for EUSART initialization structure in Smart Card mode with low-frequency clock.
#define SL_HAL_EUSART_SMARTCARD_INIT_DEFAULT_LF                                            \
  {                                                                                        \
    SL_HAL_EUSART_UART_INIT_DEFAULT_LF,         /* Default low frequency configuration. */ \
    false,                                      /* Disable retransmission by default. */   \
  }
#endif

#if defined(_EUSART_LINCFG_MASK)
/// Default configuration for EUSART initialization structure in LIN mode with high-frequency clock.
#define SL_HAL_EUSART_LIN_INIT_DEFAULT_HF                                                   \
  {                                                                                         \
    SL_HAL_EUSART_UART_INIT_DEFAULT_HF,         /* Default high frequency configuration. */ \
    13u,                                        /* Timeout after 13 bit periods. */         \
  }

/// Default configuration for EUSART initialization structure in LIN mode with low-frequency clock.
#define SL_HAL_EUSART_LIN_INIT_DEFAULT_LF                                                  \
  {                                                                                        \
    SL_HAL_EUSART_UART_INIT_DEFAULT_LF,         /* Default low frequency configuration. */ \
    13u,                                        /* Timeout after 13 bit periods. */        \
  }
#endif

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *    Initialize EUSART when used in UART mode with the high frequency clock.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] uart_init
 *   A pointer to the initialization structure.
 ******************************************************************************/
void sl_hal_eusart_init_uart_hf(EUSART_TypeDef *eusart,
                                const sl_hal_eusart_uart_init_t *uart_init);

/***************************************************************************//**
 * @brief
 *   Initialize EUSART when used in UART mode with the low frequency clock.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] uart_init
 *   A pointer to the initialization structure.
 *
 * @note (1) When EUSART oversampling is set to SL_HAL_EUSART_OVS_0 (Disable),
 * the peripheral clock frequency must be at least three times higher
 * than the chosen baud rate. In LF, max input clock is 32768
 * (LFXO or LFRCO), thus 32768 / 3 ~ 9600 baudrate. Number of stop
 * bits must be 1 or 2. Data bits must be less than 9 bits. Majority
 * Vote must be disabled.
 ******************************************************************************/
void sl_hal_eusart_init_uart_lf(EUSART_TypeDef *eusart,
                                const sl_hal_eusart_uart_init_t *uart_init);

/***************************************************************************//**
 * @brief
 *   Initialize EUSART when used in IrDA mode with the high or low
 *   frequency clock.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] irda_init
 *   A pointer to the initialization structure.
 ******************************************************************************/
void sl_hal_eusart_init_irda(EUSART_TypeDef *eusart,
                             const sl_hal_eusart_irda_init_t *irda_init);

#if defined(EUSART_DALICFG_DALIEN)
/***************************************************************************//**
 * @brief
 *   Initialize EUSART when used in DALI mode with the high or low
 *   frequency clock.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] dali_init
 *   A pointer to the initialization structure.
 *
 * @note
 *   When EUSART oversampling is set to SL_HAL_EUSART_OVS_0 (Disable),
 *   the peripheral clock frequency must be at least three times higher
 *   than the chosen baud rate. In LF, max input clock is 32768
 *   (LFXO or LFRCO), thus 32768 / 3 ~ 9600 baudrate.
 ******************************************************************************/
void sl_hal_eusart_init_dali(EUSART_TypeDef *eusart,
                             const sl_hal_eusart_dali_init_t *dali_init);
#endif

#if defined(_EUSART_CFG0_SYNC_MASK)
/***************************************************************************//**
 * @brief
 *   Initialize EUSART when used in SPI mode.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] spi_init
 *   A pointer to the initialization structure.
 ******************************************************************************/
void sl_hal_eusart_init_spi(EUSART_TypeDef *eusart,
                            const sl_hal_eusart_spi_init_t *spi_init);
#endif

#if defined(_EUSART_CFG0_SCMODE_MASK)
/***************************************************************************//**
 * @brief
 *   Initialize EUSART when used in Smart Card mode with the high or low
 *   frequency clock.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] smartcard_init
 *   A pointer to the initialization structure.
 *
 * @note
 *   Smart Card mode is only available in asynchronous mode (SYNC = 0).
 *   When EUSART oversampling is set to SL_HAL_EUSART_OVS_0 (Disable),
 *   the peripheral clock frequency must be at least three times higher
 *   than the chosen baud rate. In LF, max input clock is 32768
 *   (LFXO or LFRCO), thus 32768 / 3 ~ 9600 baudrate.
 ******************************************************************************/
void sl_hal_eusart_init_smartcard(EUSART_TypeDef *eusart,
                                  const sl_hal_eusart_smartcard_init_t *smartcard_init);
#endif

#if defined(_EUSART_LINCFG_MASK)
/***************************************************************************//**
 * @brief
 *   Initialize EUSART when used in LIN mode with the high or low
 *   frequency clock.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] lin_init
 *   A pointer to the initialization structure.
 ******************************************************************************/
void sl_hal_eusart_init_lin(EUSART_TypeDef *eusart,
                            const sl_hal_eusart_lin_init_t *lin_init);
#endif

/***************************************************************************//**
 * @brief
 *   Configure EUSART to its reset state.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 ******************************************************************************/
void sl_hal_eusart_reset(EUSART_TypeDef *eusart);

/***************************************************************************//**
 * @brief
 *   Receive one 8 bits frame, (or part of 9 bit frame).
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @note
 *   This function will stall if buffer is empty until data is received.
 *
 * @return
 *   Data received.
 ******************************************************************************/
uint16_t sl_hal_eusart_rx(EUSART_TypeDef *eusart);

/***************************************************************************//**
 * @brief
 *   Receive one 8-16 bit frame with extended information.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @note
 *   This function is normally used to receive one frame and additional RX
 * status information.
 *
 * @note
 *   This function will stall if buffer is empty until data is received.
 *
 * @return
 *   Data received and receive status.
 ******************************************************************************/
uint16_t sl_hal_eusart_rx_extended(EUSART_TypeDef *eusart);

/***************************************************************************//**
 * @brief
 *   Transmit one 8 bits frame.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] data
 *   Data to transmit.
 *
 * @note
 *   This function will stall if the 4 frame FIFO is full, until the buffer
 *   becomes available.
 ******************************************************************************/
void sl_hal_eusart_tx(EUSART_TypeDef *eusart,
                      uint16_t data);

/***************************************************************************//**
 * @brief
 *   Transmit one 8-9 bit frame with extended control.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] data
 *   Data to transmit.
 *
 * @note
 *   Possible parity/stop bits in asynchronous mode are not
 *   considered part of a specified frame bit length.
 *
 * @note
 *   This function will stall if buffer is full until the buffer becomes
 *   available.
 ******************************************************************************/
void sl_hal_eusart_tx_extended(EUSART_TypeDef *eusart,
                               uint16_t data);

#if defined(_EUSART_CFG0_SYNC_MASK)
/***************************************************************************//**
 * @brief
 *   Transmit one 8-16 bit frame and return received data.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] data
 *   Data to transmit.
 *
 * @return Data
 *   received and receive status.
 *
 * @note SPI
 *   master mode only.
 *
 * @note
 *   This function will stall if the TX buffer is full until the buffer becomes
 *   available.
 ******************************************************************************/
uint16_t sl_hal_eusart_spi_tx_rx(EUSART_TypeDef *eusart,
                                 uint16_t data);
#endif

#if defined(EUSART_DALICFG_DALIEN)
/***************************************************************************//**
 * @brief
 *   Transmit one DALI frame.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] data
 *   Data to transmit.
 *
 * @note
 *   Depending on the TXdatabits configuration, N (least significant) bits
 *   from @p data are transmitted.
 *
 * @note
 *   This function will stall if the 16 frame FIFO is full, until the buffer
 *   becomes available.
 ******************************************************************************/
void sl_hal_eusart_dali_tx(EUSART_TypeDef *eusart,
                           uint32_t data);

/***************************************************************************//**
 * @brief
 *   Receive one 8-32 bit DALI frame.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @note
 *   This function is normally used to receive one DALI frame.
 *
 * @note
 *   This function will stall if the 16 frame FIFO is empty until new
 *   data is received.
 *
 * @return
 *   Data received. Depending on the RXdatabits configuration, N
 *   (least significant) bits are returned.
 ******************************************************************************/
uint32_t sl_hal_eusart_dali_rx(EUSART_TypeDef *eusart);
#endif

#if defined(_EUSART_LINCTRL_MASK)
/***************************************************************************//**
 * @brief
 *   Set the number of bytes for the LIN response.
 *
 * @note
 *   This should be set after the protected identifier (PID) was received.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] length_bytes
 *   Number of bytes in the LIN response.
 ******************************************************************************/
void sl_hal_eusart_set_lin_response_length(EUSART_TypeDef *eusart,
                                           uint8_t length_bytes);
#endif

#if defined(SL_HAL_EUSART_PRS_SUPPORTED)
/***************************************************************************//**
 * @brief
 *   Enable automatic enabling of reception using the PRS as a trigger.
 *
 * @note
 *   Initialize EUSART with sl_hal_eusart_init_uart_hf() or
 *   sl_hal_eusart_init_uart_lf() before enabling the PRS trigger.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 ******************************************************************************/
void sl_hal_eusart_enable_rx_prs_trigger(EUSART_TypeDef *eusart,
                                         uint8_t channel);

/***************************************************************************//**
 * @brief
 *   Disable automatic enabling of reception using the PRS as a trigger.
 *
 * @note
 *   Initialize EUSART with sl_hal_eusart_init_uart_hf() or
 *   sl_hal_eusart_init_uart_lf() before enabling the PRS trigger.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 ******************************************************************************/
void sl_hal_eusart_disable_rx_prs_trigger(EUSART_TypeDef *eusart);

/***************************************************************************//**
 * @brief
 *   Enable automatic enabling of transmission using the PRS as a trigger.
 *
 * @note
 *   Initialize EUSART with sl_hal_eusart_init_uart_hf() or
 * sl_hal_eusart_init_uart_lf() before enabling the PRS trigger.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] channel
 *   PRS channel number.
 ******************************************************************************/
void sl_hal_eusart_enable_tx_prs_trigger(EUSART_TypeDef *eusart,
                                         uint8_t channel);

/***************************************************************************//**
 * @brief
 *   Disable automatic enabling of transmission using the PRS as a trigger.
 *
 * @note
 *   Initialize EUSART with sl_hal_eusart_init_uart_hf() or
 *   sl_hal_eusart_init_uart_lf() before enabling the PRS trigger.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 ******************************************************************************/
void sl_hal_eusart_disable_tx_prs_trigger(EUSART_TypeDef *eusart);
#endif // defined(SL_HAL_EUSART_PRS_SUPPORTED)

#if defined(_EUSART_CFG0_SYNC_MASK)
/***************************************************************************//**
 * @brief
 *   Calculate SPI clock divider using given reference frequency and baudrate.
 *
 * @param[in] ref_freq
 *   Peripheral clock frequency.
 *
 * @param[in] baudrate
 *   Transmission speed of the SPI interface.
 *
 * @return
 *   Eusart clock divider for desired baud rate.
 ******************************************************************************/
uint32_t sl_hal_eusart_spi_calculate_clock_div(uint32_t ref_freq,
                                               uint32_t baudrate);
#endif

/***************************************************************************//**
 * @brief
 *   Calculate UART clock divider using given reference frequency,
 *   oversampling and baudrate.
 *
 * @param[in] ref_freq
 *   Peripheral clock frequency.
 *
 * @param[in] baudrate
 *   Transmission speed of the UART interface.
 *
 * @param[in] ovs
 *   Oversampling rate.
 *
 * @return Eusart
 *   clock divider for desired baud rate.
 ******************************************************************************/
uint32_t sl_hal_eusart_uart_calculate_clock_div(uint32_t ref_freq,
                                                uint32_t baudrate,
                                                sl_hal_eusart_ovs_t ovs);

/***************************************************************************//**
 * @brief
 *   Get the calculated baudrate from the current EUSART configuration and clock.
 *
 * @param[in] div
 *   CLKDIV register value of the EUSART.
 *
 * @param[in] ovs
 *   Oversample value of the EUSART.
 *
 * @param[in] freq
 *   The clock branch frequency to calculate the baudrate.
 *
 * @return
 *   The baudrate
 ******************************************************************************/
uint32_t sl_hal_eusart_uart_calculate_baudrate(uint32_t div,
                                               sl_hal_eusart_ovs_t ovs,
                                               uint32_t freq);

#if defined(_EUSART_CFG0_SYNC_MASK)
/***************************************************************************//**
 * @brief
 *   Get the calculated baudrate from the current EUSART configuration and clock.
 *
 * @param[in] div
 *   clkdiv register value of the EUSART.
 *
 * @param[in] freq
 *   The clock branch frequency to calculate the baudrate.
 *
 * @return
 *   The baudrate.
 ******************************************************************************/
uint32_t sl_hal_eusart_spi_calculate_baudrate(uint32_t div,
                                              uint32_t freq);
#endif

/***************************************************************************//**
 * @brief
 *   Wait for ongoing sync of register(s) to the low-frequency domain to complete.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] mask
 *   Bitmask for SYNCBUSY register - indicates which register(s) to wait for
 *   synchronization to complete
 ******************************************************************************/
__INLINE void sl_hal_eusart_wait_sync(EUSART_TypeDef *eusart,
                                      uint32_t mask)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  // Wait for any pending previous write operation to have been completed
  // in the low-frequency domain.
  while ((eusart->EN & _EUSART_EN_EN_MASK) && (eusart->SYNCBUSY & mask)) {
  }
}
/***************************************************************************//**
 * @brief
 *   Enable EUSART.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_eusart_enable(EUSART_TypeDef *eusart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  // Wait for synchronization to complete.
  sl_hal_eusart_wait_sync(eusart, _EUSART_SYNCBUSY_MASK);
  // Enable peripheral.
  eusart->EN_SET = EUSART_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Disable EUSART.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_eusart_disable(EUSART_TypeDef *eusart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  if (eusart->EN != EUSART_EN_EN) {
    return;
  }

#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2)
  // Read data until FIFO is emptied
  // but taking care not to underflow the receiver
  while (eusart->STATUS & EUSART_STATUS_RXFL) {
    eusart->RXDATA;
  }
#endif

  // Disable EUSART
  eusart->EN_CLR = EUSART_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Wait for disabling to finish.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_eusart_wait_ready(EUSART_TypeDef *eusart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

#if defined(_EUSART_EN_DISABLING_MASK)
  while (eusart->EN & _EUSART_EN_DISABLING_MASK) {
    // Wait for disabling to finish.
  }
#endif
}

/***************************************************************************//**
 * @brief
 *   Enable EUSART receiver.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_eusart_enable_rx(EUSART_TypeDef *eusart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  // Wait for synchronization to complete.
  sl_hal_eusart_wait_sync(eusart, _EUSART_SYNCBUSY_MASK);
  eusart->CMD_SET = EUSART_CMD_RXEN;
}

/***************************************************************************//**
 * @brief
 *   Disable EUSART receiver.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_eusart_disable_rx(EUSART_TypeDef *eusart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  // Wait for synchronization to complete.
  sl_hal_eusart_wait_sync(eusart, _EUSART_SYNCBUSY_MASK);
  eusart->CMD_SET = EUSART_CMD_RXDIS;
}

/***************************************************************************//**
 * @brief
 *   Enable EUSART transmitter.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_eusart_enable_tx(EUSART_TypeDef *eusart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  // Wait for synchronization to complete.
  sl_hal_eusart_wait_sync(eusart, _EUSART_SYNCBUSY_MASK);
  eusart->CMD_SET = EUSART_CMD_TXEN;
}

/***************************************************************************//**
 * @brief
 *   Disable EUSART transmitter.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_eusart_disable_tx(EUSART_TypeDef *eusart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  // Wait for synchronization to complete.
  sl_hal_eusart_wait_sync(eusart, _EUSART_SYNCBUSY_MASK);
  eusart->CMD_SET = EUSART_CMD_TXDIS;
}

/***************************************************************************//**
 * @brief
 *   Enable reception operation until the configured start frame is
 *   received.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_eusart_enable_block_rx(EUSART_TypeDef *eusart)
{
  // Make sure that the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  sl_hal_eusart_wait_sync(eusart, EUSART_SYNCBUSY_RXBLOCKEN);
  eusart->CMD_SET = EUSART_CMD_RXBLOCKEN;
}

/***************************************************************************//**
 * @brief
 *   Disable reception operation until the configured start frame is
 * received.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_eusart_disable_block_rx(EUSART_TypeDef *eusart)
{
  // Make sure that the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  sl_hal_eusart_wait_sync(eusart, EUSART_SYNCBUSY_RXBLOCKDIS);
  eusart->CMD_SET = EUSART_CMD_RXBLOCKDIS;
}

/***************************************************************************//**
 * @brief
 *   Enable the tristating of the transmitter output.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_eusart_enable_tx_tristate(EUSART_TypeDef *eusart)
{
  // Make sure that the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  sl_hal_eusart_wait_sync(eusart, EUSART_SYNCBUSY_TXTRIEN);
  eusart->CMD_SET = EUSART_CMD_TXTRIEN;
}

/***************************************************************************//**
 * @brief
 *   Disable the tristating of the transmitter output.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_eusart_disable_tx_tristate(EUSART_TypeDef *eusart)
{
  // Make sure that the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  sl_hal_eusart_wait_sync(eusart, EUSART_SYNCBUSY_TXTRIDIS);
  eusart->CMD_SET = EUSART_CMD_TXTRIDIS;
}

/***************************************************************************//**
 * @brief
 *   Clear RX FIFO.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_eusart_clear_rx(EUSART_TypeDef *eusart)
{
  // Make sure that the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  // Make sure receiver is disabled.
  EFM_ASSERT(((eusart->STATUS & _EUSART_STATUS_RXENS_MASK) >> _EUSART_STATUS_RXENS_SHIFT) == 0);

#if defined(EUSART_CMD_CLEARRX)
  eusart->CMD_SET = EUSART_CMD_CLEARRX;

  while (((eusart->STATUS & _EUSART_STATUS_CLEARRXBUSY_MASK) >> _EUSART_STATUS_CLEARRXBUSY_SHIFT) != 0) {
    // Wait for clearing the RX FIFO to finish.
  }
#else
  while (((eusart->STATUS & _EUSART_STATUS_RXFL_MASK) >> _EUSART_STATUS_RXFL_SHIFT) != 0) {
    eusart->RXDATA;
  }
#endif
}

/***************************************************************************//**
 * @brief
 *   Clear TX FIFO.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_eusart_clear_tx(EUSART_TypeDef *eusart)
{
  // Make sure that the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  // Make sure transmitter is disabled.
  EFM_ASSERT(((eusart->STATUS & _EUSART_STATUS_TXENS_MASK) >> _EUSART_STATUS_TXENS_SHIFT) == 0);

  eusart->CMD_SET = EUSART_CMD_CLEARTX;

  while (((eusart->STATUS & _EUSART_STATUS_CLEARTXBUSY_MASK) >> _EUSART_STATUS_CLEARTXBUSY_SHIFT) != 0) {
    // Wait for clearing the TX FIFO to finish.
  }
}

#if defined(_EUSART_CMD_TSTOP_MASK)
/***************************************************************************//**
 * @brief
 *   Stop the timer when the response timeout is in restart mode.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_eusart_stop_timer(EUSART_TypeDef *eusart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  sl_hal_eusart_wait_sync(eusart, EUSART_SYNCBUSY_TSTOP);
  eusart->CMD_SET = EUSART_CMD_TSTOP;
}
#endif

/***************************************************************************//**
 * @brief
 *   Get EUSART STATUS register.
 *
 * @param[in] eusart Pointer to the EUSART peripheral register block.
 *
 * @return
 *   STATUS register value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_eusart_get_status(EUSART_TypeDef *eusart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  return eusart->STATUS;
}

/***************************************************************************//**
 * @brief
 *   Set UART mode clock divider.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] clock_divider
 *   Clock divider value.
 ******************************************************************************/
__INLINE void sl_hal_eusart_uart_set_clock_div(EUSART_TypeDef *eusart,
                                               uint32_t clock_divider)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  // Make sure clock divider value is valid.
  EFM_ASSERT(clock_divider <= (_EUSART_CLKDIV_DIV_MASK >> _EUSART_CLKDIV_DIV_SHIFT));

  sl_hal_eusart_wait_sync(eusart, _EUSART_SYNCBUSY_MASK);

  eusart->CLKDIV = ((clock_divider << _EUSART_CLKDIV_DIV_SHIFT) & _EUSART_CLKDIV_DIV_MASK);
}

/***************************************************************************//**
 * @brief
 *   Get UART mode clock divider.
 *
 * @param[in] eusart
 *   ointer to the EUSART peripheral register block.
 *
 * @return
 *   Clock divider value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_eusart_uart_get_clock_div(EUSART_TypeDef *eusart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  // Make sure module is enabled
  EFM_ASSERT(eusart->EN & _EUSART_EN_EN_MASK);

  sl_hal_eusart_wait_sync(eusart, _EUSART_SYNCBUSY_MASK);

  return ((eusart->CLKDIV & _EUSART_CLKDIV_DIV_MASK) >> _EUSART_CLKDIV_DIV_SHIFT);
}

/***************************************************************************//**
 * @brief
 *   Get UART mode oversampling value.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @return
 *   Oversampling value.
 ******************************************************************************/
__INLINE sl_hal_eusart_ovs_t sl_hal_eusart_uart_get_oversampling(EUSART_TypeDef *eusart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  sl_hal_eusart_wait_sync(eusart, _EUSART_SYNCBUSY_MASK);

  return ((eusart->CFG0 & _EUSART_CFG0_OVS_MASK) >> _EUSART_CFG0_OVS_SHIFT);
}

/***************************************************************************//**
 * @brief
 *   Clear one or more pending EUSART interrupts.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] flags
 *   Pending EUSART interrupt source to clear. Use a bitwise logic
 *   OR combination of valid interrupt flags for EUSART module
 *   (EUSART_IF_nnn).
 ******************************************************************************/
__INLINE void sl_hal_eusart_clear_interrupts(EUSART_TypeDef *eusart,
                                             uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  eusart->IF_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Disable one or more EUSART interrupts.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] flags
 *   Pending EUSART interrupt source to clear. Use a bitwise logic
 *   OR combination of valid interrupt flags for EUSART module
 *   (EUSART_IF_nnn).
 ******************************************************************************/
__INLINE void sl_hal_eusart_disable_interrupts(EUSART_TypeDef *eusart,
                                               uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  eusart->IEN_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Enable one or more EUSART interrupts.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] flags
 *   Pending EUSART interrupt source to clear. Use a bitwise logic
 *   OR combination of valid interrupt flags for EUSART module
 *   (EUSART_IF_nnn).
 ******************************************************************************/
__INLINE void sl_hal_eusart_enable_interrupts(EUSART_TypeDef *eusart,
                                              uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  eusart->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Get pending EUSART interrupt flags.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @return Pending
 *   EUSART interrupt sources.
 ******************************************************************************/
__INLINE uint32_t sl_hal_eusart_get_pending_interrupts(EUSART_TypeDef *eusart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  return eusart->IF;
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending EUSART interrupt flags.
 *
 * @note
 *   Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @param[in] eusart
 *   ointer to the EUSART peripheral register block.
 *
 * @return
 *   Pending and enabled EUSART interrupt sources.
 ******************************************************************************/
__INLINE uint32_t sl_hal_eusart_get_enabled_pending_interrupts(EUSART_TypeDef *eusart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  uint32_t tmp;

  /* Store EUSARTx->IEN in temporary variable in order to define explicit order
   * of volatile accesses. */
  tmp = eusart->IEN;

  /* Bitwise AND of pending and enabled interrupts */
  return eusart->IF & tmp;
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending EUSART interrupts from SW.
 *
 * @param[in] eusart
 *   Pointer to the EUSART peripheral register block.
 *
 * @param[in] flags
 *   Interrupt source(s) to set to pending. Use a bitwise logic OR
 *   combination of valid interrupt flags for EUSART module
 *   (EUSART_IF_nnn).
 ******************************************************************************/
__INLINE void sl_hal_eusart_set_interrupts(EUSART_TypeDef *eusart,
                                           uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_EUSART_REF_VALID(eusart));

  eusart->IF_SET = flags;
}

/** @} (end addtogroup eusart) */

#ifdef __cplusplus
}
#endif

/* *INDENT-OFF* */
/****************************************************************************//**
 * @addtogroup eusart EUSART - Enhanced USART
 * @{
 * @li @ref eusart_intro
 * @li @ref eusart_example
 *
 *@n @section eusart_intro Introduction
 *  This module contains functions to control the Enhanced Universal Synchronous
 *  / Asynchronous Receiver / Transmitter controller(s) (EUSART) peripheral of Silicon
 *  Labs' 32-bit MCUs and SoCs. EUSART can be used as a UART and can,
 *  therefore, be connected to an external transceiver to communicate with
 *  another host using the serial link.
 *
 *  It supports full duplex asynchronous UART communication as well as RS-485,
 *  SPI, MicroWire, and 3-wire. It can also interface with ISO7816 Smart-Cards,
 *  and IrDA devices.
 *
 *  EUSART has a wide selection of operating modes, frame formats, and baud rates.
 *  All features are supported through the API of this module.
 *
 * This module does not support DMA configuration. UARTDRV and SPIDRV drivers
 * provide full support for DMA and more.
 *
 *@n @section eusart_example Example
 *
 *  EUSART Asynchronous UART TX example:
 *  @code{.c}
 *  {
 *    #define GPIO_TX    PB0
 *
 *    sl_hal_eusart_uart_init_t init = SL_HAL_EUSART_UART_INIT_DEFAULT_HF;
 *
 *    // Configure the clocks.
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_EUSART0);
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
 *
 *    // Calculate and configure the eusart's clock divider.
 *    uint32_t baudrate = 38400;  // Desired baudrate.
 *    sl_clock_branch_t clock_branch = sl_device_peripheral_get_clock_branch(SL_PERIPHERAL_EUSART0);
 *    uint32_t freq;
 *    sl_clock_manager_get_clock_branch_frequency(clock_branch, &freq);
 *    init.clock_div = sl_hal_eusart_uart_calculate_clock_div(freq, baudrate, init.oversampling);
 *
 *    // Configure the EUSART port.
 *    sl_gpio_set_pin_mode(GPIO_TX, SL_GPIO_MODE_PUSH_PULL, 0);
 *    GPIO->EUSARTROUTE[EUSART_NUM(EUSART0)].TXROUTE = (GPIO_TX->port << _GPIO_EUSART_TXROUTE_PORT_SHIFT)
 *                                                   | (GPIO_TX->pin << _GPIO_EUSART_TXROUTE_PIN_SHIFT);
 *    GPIO->EUSARTROUTE[EUSART_NUM(EUSART0)].ROUTEEN = GPIO_EUSART_ROUTEEN_TXPEN;
 *
 *    // Initialize the EUSART.
 *    sl_hal_eusart_init_uart_hf(EUSART0, &init);
 *    sl_hal_eusart_enable(EUSART0);
 *    sl_hal_eusart_enable_tx(EUSART0);
 *
 *    uint16_t data = 0xFF;
 *    sl_hal_eusart_tx(EUSART0, data);
 *  }
 *
 *  @endcode
 *
 *  EUSART Synchronous SPI Initialization example:
 *  @code{.c}
 *  {
 *    #define GPIO_MOSI    PC0
 *    #define GPIO_MISO    PC1
 *    #define GPIO_SCLK    PC2
 *
 *    sl_hal_eusart_spi_init_t init = SL_HAL_EUSART_SPI_MASTER_INIT_DEFAULT_HF;
 *    // Clock idle low, sample on falling edge.
 *    init.clock_mode = SL_HAL_EUSART_CLOCK_MODE_1;
 *
 *    // Configure the clocks.
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_EUSART1);
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
 *
 *    // Calculate and configure the eusart's clock divider.
 *    uint32_t baudrate  = 1000000;  // Desired baudrate.
 *    sl_clock_branch_t clock_branch = sl_device_peripheral_get_clock_branch(SL_PERIPHERAL_EUSART1);
 *    uint32_t freq;
 *    sl_clock_manager_get_clock_branch_frequency(clock_branch, &freq);
 *    init.clock_div = sl_hal_eusart_spi_calculate_clock_div(freq, baudrate);
 *
 *    // Configure the SPI ports.
 *    sl_gpio_set_pin_mode(GPIO_MOSI, SL_GPIO_MODE_PUSH_PULL, 0);
 *    sl_gpio_set_pin_mode(GPIO_MISO, SL_GPIO_MODE_INPUT, 0);
 *    sl_gpio_set_pin_mode(GPIO_SCLK, SL_GPIO_MODE_PUSH_PULL, 0);
 *
 *    // Connect EUSART to ports.
 *    GPIO->EUSARTROUTE[EUSART_NUM(EUSART1)].TXROUTE = (GPIO_MOSI->port << _GPIO_EUSART_TXROUTE_PORT_SHIFT)
 *                                                     | (GPIO_MOSI->pin << _GPIO_EUSART_TXROUTE_PIN_SHIFT);
 *    GPIO->EUSARTROUTE[EUSART_NUM(EUSART1)].RXROUTE = (GPIO_MISO->port << _GPIO_EUSART_RXROUTE_PORT_SHIFT)
 *                                                     | (GPIO_MISO->pin << _GPIO_EUSART_RXROUTE_PIN_SHIFT);
 *    GPIO->EUSARTROUTE[EUSART_NUM(EUSART1)].SCLKROUTE = (GPIO_SCLK->port << _GPIO_EUSART_SCLKROUTE_PORT_SHIFT)
 *                                                     | (GPIO_SCLK->pin << _GPIO_EUSART_SCLKROUTE_PIN_SHIFT);
 *    GPIO->EUSARTROUTE[EUSART_NUM(EUSART1)].ROUTEEN = GPIO_EUSART_ROUTEEN_TXPEN | GPIO_EUSART_ROUTEEN_RXPEN | GPIO_EUSART_ROUTEEN_SCLKPEN;
 *
 *    // Initialize the EUSART.
 *    sl_hal_eusart_init_spi(EUSART1, &init);
 *    sl_hal_eusart_enable(EUSART1);
 *    sl_hal_eusart_enable_rx(EUSART1);
 *    sl_hal_eusart_enable_tx(EUSART1);
 *  }
 *
 *  @endcode
 *
 * @} (end addtogroup eusart)
 ******************************************************************************/
/* *INDENT-ON* */

#endif /* defined(EUSART_PRESENT) */
#endif /* #ifndef SL_HAL_EUSART_H */
