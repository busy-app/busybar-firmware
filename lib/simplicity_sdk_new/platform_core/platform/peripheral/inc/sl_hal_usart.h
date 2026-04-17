/***************************************************************************//**
 * @file
 * @brief Universal synchronous/asynchronous receiver/transmitter (USART/UART)
 *   peripheral API
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

#ifndef SL_HAL_USART_H
#define SL_HAL_USART_H

#include "em_device.h"

#if defined(USART_PRESENT)

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_assert.h"
#include <stdbool.h>
#include "sl_enum.h"

/***************************************************************************//**
 * @addtogroup usart USART - Synchronous/Asynchronous Serial
 * @brief Universal Synchronous/Asynchronous Receiver/Transmitter
 *   Peripheral API
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/// Validation of USART register block pointer reference for assert statements.
#define SL_HAL_USART_REF_VALID(ref)    (USART_NUM(ref) != -1)

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/// Databit selection.
SL_ENUM(sl_hal_usart_data_bits_t) {
  SL_HAL_USART_DATA_BITS_4  = _USART_FRAME_DATABITS_FOUR,     ///< 4 data bits (not available for UART).
  SL_HAL_USART_DATA_BITS_5  = _USART_FRAME_DATABITS_FIVE,     ///< 5 data bits (not available for UART).
  SL_HAL_USART_DATA_BITS_6  = _USART_FRAME_DATABITS_SIX,      ///< 6 data bits (not available for UART).
  SL_HAL_USART_DATA_BITS_7  = _USART_FRAME_DATABITS_SEVEN,    ///< 7 data bits (not available for UART).
  SL_HAL_USART_DATA_BITS_8  = _USART_FRAME_DATABITS_EIGHT,    ///< 8 data bits.
  SL_HAL_USART_DATA_BITS_9  = _USART_FRAME_DATABITS_NINE,     ///< 9 data bits.
  SL_HAL_USART_DATA_BITS_10 = _USART_FRAME_DATABITS_TEN,      ///< 10 data bits (not available for UART).
  SL_HAL_USART_DATA_BITS_11 = _USART_FRAME_DATABITS_ELEVEN,   ///< 11 data bits (not available for UART).
  SL_HAL_USART_DATA_BITS_12 = _USART_FRAME_DATABITS_TWELVE,   ///< 12 data bits (not available for UART).
  SL_HAL_USART_DATA_BITS_13 = _USART_FRAME_DATABITS_THIRTEEN, ///< 13 data bits (not available for UART).
  SL_HAL_USART_DATA_BITS_14 = _USART_FRAME_DATABITS_FOURTEEN, ///< 14 data bits (not available for UART).
  SL_HAL_USART_DATA_BITS_15 = _USART_FRAME_DATABITS_FIFTEEN,  ///< 15 data bits (not available for UART).
  SL_HAL_USART_DATA_BITS_16 = _USART_FRAME_DATABITS_SIXTEEN   ///< 16 data bits (not available for UART).
};

/// Oversampling selection, used for asynchronous operation.
SL_ENUM(sl_hal_usart_ovs_t) {
  SL_HAL_USART_OVS_16 = _USART_CTRL_OVS_X16,     ///< 16x oversampling (normal).
  SL_HAL_USART_OVS_8  = _USART_CTRL_OVS_X8,      ///< 8x oversampling.
  SL_HAL_USART_OVS_6  = _USART_CTRL_OVS_X6,      ///< 6x oversampling.
  SL_HAL_USART_OVS_4  = _USART_CTRL_OVS_X4       ///< 4x oversampling.
};

/// Parity selection, mainly used for asynchronous operation.
SL_ENUM(sl_hal_usart_parity_t) {
  SL_HAL_USART_NO_PARITY   = _USART_FRAME_PARITY_NONE,    ///< No parity.
  SL_HAL_USART_EVEN_PARITY = _USART_FRAME_PARITY_EVEN,    ///< Even parity.
  SL_HAL_USART_ODD_PARITY  = _USART_FRAME_PARITY_ODD      ///< Odd parity.
};

/// Stop bits selection, used for asynchronous operation.
SL_ENUM(sl_hal_usart_stop_bits_t) {
  SL_HAL_USART_STOP_BITS_0P5 = _USART_FRAME_STOPBITS_HALF,        ///< 0.5 stop bits.
  SL_HAL_USART_STOP_BITS_1   = _USART_FRAME_STOPBITS_ONE,         ///< 1 stop bits.
  SL_HAL_USART_STOP_BITS_1P5 = _USART_FRAME_STOPBITS_ONEANDAHALF, ///< 1.5 stop bits.
  SL_HAL_USART_STOP_BITS_2   = _USART_FRAME_STOPBITS_TWO          ///< 2 stop bits.
};

/// Majority vote enable.
SL_ENUM(sl_hal_usart_majority_vote_t) {
  SL_HAL_USART_MAJORITY_VOTE_ENABLE  = 0, ///< Enable majority vote for 16x, 8x and 6x oversampling modes.
  SL_HAL_USART_MAJORITY_VOTE_DISABLE = 1  ///< Disable majority vote for 16x, 8x and 6x oversampling modes.
};

/// Hardware Flow Control Selection.
SL_ENUM(sl_hal_usart_hw_flow_control_t) {
  SL_HAL_USART_HW_FLOW_CONTROL_NONE = 0,  ///< No hardware flow control.
  SL_HAL_USART_HW_FLOW_CONTROL_CTS,       ///< CTS signal is enabled for TX flow control.
  SL_HAL_USART_HW_FLOW_CONTROL_RTS,       ///< RTS signal is enabled for RX flow control.
  SL_HAL_USART_HW_FLOW_CONTROL_CTS_RTS    ///< CTS and RTS signals are enabled for TX and RX flow control.
};

/// Clock polarity/phase mode.
SL_ENUM_GENERIC(sl_hal_usart_clock_mode_t, uint32_t) {
  /// Clock idle low, sample on rising edge.
  SL_HAL_USART_CLOCK_MODE_0 = USART_CTRL_CLKPOL_IDLELOW | USART_CTRL_CLKPHA_SAMPLELEADING,

  /// Clock idle low, sample on falling edge.
  SL_HAL_USART_CLOCK_MODE_1 = USART_CTRL_CLKPOL_IDLELOW | USART_CTRL_CLKPHA_SAMPLETRAILING,

  /// Clock idle high, sample on falling edge.
  SL_HAL_USART_CLOCK_MODE_2 = USART_CTRL_CLKPOL_IDLEHIGH | USART_CTRL_CLKPHA_SAMPLELEADING,

  /// Clock idle high, sample on rising edge.
  SL_HAL_USART_CLOCK_MODE_3 = USART_CTRL_CLKPOL_IDLEHIGH | USART_CTRL_CLKPHA_SAMPLETRAILING
};

/// Pulse width selection for IrDA mode.
SL_ENUM(sl_hal_usart_irda_pulse_width_t) {
  SL_HAL_USART_PULSE_WIDTH_ONE   = _USART_IRCTRL_IRPW_ONE,    ///< IrDA pulse width is 1/16 for OVS=0 and 1/8 for OVS=1
  SL_HAL_USART_PULSE_WIDTH_TWO   = _USART_IRCTRL_IRPW_TWO,    ///< IrDA pulse width is 2/16 for OVS=0 and 2/8 for OVS=1
  SL_HAL_USART_PULSE_WIDTH_THREE = _USART_IRCTRL_IRPW_THREE,  ///< IrDA pulse width is 3/16 for OVS=0 and 3/8 for OVS=1
  SL_HAL_USART_PULSE_WIDTH_FOUR  = _USART_IRCTRL_IRPW_FOUR    ///< IrDA pulse width is 4/16 for OVS=0 and 4/8 for OVS=1
};

/// I2S format selection.
SL_ENUM(sl_hal_usart_i2s_format_t) {
  SL_HAL_USART_I2S_FORMAT_W32D32  = _USART_I2SCTRL_FORMAT_W32D32,   ///< 32-bit word, 32-bit data.
  SL_HAL_USART_I2S_FORMAT_W32D24M = _USART_I2SCTRL_FORMAT_W32D24M,  ///< 32-bit word, 32-bit data with 8 lsb masked.
  SL_HAL_USART_I2S_FORMAT_W32D24  = _USART_I2SCTRL_FORMAT_W32D24,   ///< 32-bit word, 24-bit data.
  SL_HAL_USART_I2S_FORMAT_W32D16  = _USART_I2SCTRL_FORMAT_W32D16,   ///< 32-bit word, 16-bit data.
  SL_HAL_USART_I2S_FORMAT_W32D8   = _USART_I2SCTRL_FORMAT_W32D8,    ///< 32-bit word, 8-bit data.
  SL_HAL_USART_I2S_FORMAT_W16D16  = _USART_I2SCTRL_FORMAT_W16D16,   ///< 16-bit word, 16-bit data.
  SL_HAL_USART_I2S_FORMAT_W16D8   = _USART_I2SCTRL_FORMAT_W16D8,    ///< 16-bit word, 8-bit data.
  SL_HAL_USART_I2S_FORMAT_W8D8    = _USART_I2SCTRL_FORMAT_W8D8      ///<  8-bit word, 8-bit data.
};

/// I2S frame data justify.
SL_ENUM(sl_hal_usart_i2s_justify_t) {
  SL_HAL_USART_JUSTIFY_LEFT  = _USART_I2SCTRL_JUSTIFY_LEFT,  ///< Data is left-justified within the frame.
  SL_HAL_USART_JUSTIFY_RIGHT = _USART_I2SCTRL_JUSTIFY_RIGHT  ///< Data is right-justified within the frame.
};

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// Asynchronous mode initialization structure.
typedef struct {
  /// Enable CS invert. By default, chip select is active low.
  /// Set to true to make chip select active high.
  bool cs_invert;

  /// Auto CS enabling.
  bool auto_cs_enable;

  /// Enable USART Rx via PRS.
  bool prs_rx_enable;

  /// Select PRS channel for USART Rx. (Only valid if prs_rx_enable is true).
  uint8_t prs_rx_channel;

  /// Auto CS hold time in baud cycles.
  uint8_t auto_cs_hold;

  /// Auto CS setup time in baud cycles.
  uint8_t auto_cs_setup;

  /// Clock divider.
  uint32_t clock_div;

  /// Oversampling used.
  sl_hal_usart_ovs_t oversampling;

  /// Number of data bits in frame. Notice that UART modules only support
  /// 8 or 9 data bits.
  sl_hal_usart_data_bits_t data_bits;

  /// Parity mode to use.
  sl_hal_usart_parity_t parity;

  /// Number of stop bits to use.
  sl_hal_usart_stop_bits_t stop_bits;

  /// Majority Vote Disable for 16x, 8x and 6x oversampling modes.
  sl_hal_usart_majority_vote_t majority_vote;

  /// Hardware flow control mode.
  sl_hal_usart_hw_flow_control_t hw_flow_control;
} sl_hal_usart_async_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_usart_async_init_t sl_hal_usart_async_config_t;
/** @endcond */

/// Default configuration for USART asynchronous initialization structure.
#define SL_HAL_USART_INIT_ASYNC_DEFAULT                                                    \
  {                                                                                        \
    false,                               /* No CS invert. */                               \
    false,                               /* Auto CS functionality enable/disable switch */ \
    false,                               /* Not USART PRS input mode. */                   \
    0,                                   /* PRS channel 0. */                              \
    0,                                   /* Auto CS Hold cycles. */                        \
    0,                                   /* Auto CS Setup cycles. */                       \
    0,                                   /* Clock divider default value. */                \
    SL_HAL_USART_OVS_16,                 /* 16x oversampling. */                           \
    SL_HAL_USART_DATA_BITS_8,            /* 8 data bits. */                                \
    SL_HAL_USART_NO_PARITY,              /* No parity. */                                  \
    SL_HAL_USART_STOP_BITS_1,            /* 1 stop bit. */                                 \
    SL_HAL_USART_MAJORITY_VOTE_DISABLE,  /* Do not disable majority vote. */               \
    SL_HAL_USART_HW_FLOW_CONTROL_NONE,   /* No HW flow control. */                         \
  }

/// Synchronous mode initialization structure.
typedef struct {
  /// Select if to operate in master or slave mode.
  bool master;

  /// Select if to send most or least significant bit first.
  bool msb_first;

  /// Enable CS invert. By default, chip select is active low.
  /// Set to true to make chip select active high.
  bool cs_invert;

  /// Enable AUTOTX mode. Transmits as long as RX is not full.
  /// Generates underflows if TX is empty.
  bool auto_tx;

  /// Auto CS enabling.
  bool auto_cs_enable;

  /// Enable USART Rx via PRS.
  bool prs_rx_enable;

  /// Select PRS channel for USART Rx. (Only valid if prs_rx_enable is true).
  uint8_t prs_rx_channel;

  /// Auto CS hold time in baud cycles.
  uint8_t auto_cs_hold;

  /// Auto CS setup time in baud cycles.
  uint8_t auto_cs_setup;

  /// Desired Clock Divider.
  uint32_t clock_div;

  /// Number of data bits in frame.
  sl_hal_usart_data_bits_t data_bits;

  /// Clock polarity/phase mode.
  sl_hal_usart_clock_mode_t clock_mode;
} sl_hal_usart_sync_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_usart_sync_init_t sl_hal_usart_sync_config_t;
/** @endcond */

/// Default configuration for USART sync initialization structure.
#define SL_HAL_USART_INIT_SYNC_DEFAULT                                      \
  {                                                                         \
    true,                      /* Master mode. */                           \
    false,                     /* Send least significant bit first. */      \
    false,                     /* No CS invert. */                          \
    false,                     /* No AUTOTX mode. */                        \
    false,                     /* No AUTOCS mode. */                        \
    false,                     /* Not USART PRS input mode. */              \
    0,                         /* PRS channel 0. */                         \
    0,                         /* Auto CS Hold cycles. */                   \
    0,                         /* Auto CS Setup cycles. */                  \
    0,                         /* Clock divider default value. */           \
    SL_HAL_USART_DATA_BITS_8,  /* 8 databits. */                            \
    SL_HAL_USART_CLOCK_MODE_0, /* Clock idle low, sample on rising edge. */ \
  }

/// IrDA mode initialization structure. Inherited from asynchronous mode initialization structure.
typedef struct {
  /// Set to invert Rx signal before IrDA demodulator.
  bool ir_rx_inv;

  /// Set to enable filter on IrDA demodulator.
  bool ir_filter;

  /// Configure the pulse width generated by the IrDA modulator as a fraction
  /// of the configured USART bit period.
  sl_hal_usart_irda_pulse_width_t ir_pulse_width;

  /// General Asynchronous initialization structure.
  sl_hal_usart_async_init_t async;
} sl_hal_usart_irda_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_usart_irda_init_t sl_hal_usart_irda_config_t;
/** @endcond */

/// Default configuration for USART IRDA initialization structure.
#define SL_HAL_USART_INIT_IRDA_DEFAULT                                                       \
  {                                                                                          \
    false,                                 /* Rx invert disabled. */                         \
    false,                                 /* Filtering disabled. */                         \
    SL_HAL_USART_PULSE_WIDTH_THREE,        /* Pulse width is set to THREE. */                \
    {                                                                                        \
      false,                               /* No CS invert. */                               \
      false,                               /* Auto CS functionality enable/disable switch */ \
      false,                               /* Not USART PRS input mode. */                   \
      0,                                   /* PRS channel 0. */                              \
      0,                                   /* Auto CS Hold cycles. */                        \
      0,                                   /* Auto CS Setup cycles. */                       \
      0,                                   /* Clock divider default value. */                \
      SL_HAL_USART_OVS_16,                 /* 16x oversampling. */                           \
      SL_HAL_USART_DATA_BITS_8,            /* 8 data bits. */                                \
      SL_HAL_USART_NO_PARITY,              /* No parity. */                                  \
      SL_HAL_USART_STOP_BITS_1,            /* 1 stop bit. */                                 \
      SL_HAL_USART_MAJORITY_VOTE_DISABLE,  /* Do not disable majority vote. */               \
      SL_HAL_USART_HW_FLOW_CONTROL_NONE,   /* No HW flow control. */                         \
    },                                                                                       \
  }

/// I2S mode initialization structure. Inherited from synchronous mode initialization structure.
typedef struct {
  /// Delay on I2S data. Set to add a one-cycle delay between a transition
  /// on the word-clock and the start of the I2S word.
  /// Should be set for standard I2S format.
  bool delay;

  /// Separate DMA Request For Left/Right Data.
  bool dma_split;

  /// Stereo or Mono, set to true for mono.
  bool mono;

  /// I2S mode.
  sl_hal_usart_i2s_format_t format;

  /// Justification of I2S data within the frame.
  sl_hal_usart_i2s_justify_t justify;

  /// General Synchronous initialization structure.
  sl_hal_usart_sync_init_t sync;
} sl_hal_usart_i2s_init_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Typedef for configuration structure used for backward compatibility purposes.
typedef sl_hal_usart_i2s_init_t sl_hal_usart_i2s_config_t;
/** @endcond */

/// Default USART Sync configuration for series 2 devices
#define SL_HAL_USART_INIT_I2S_DEFAULT                                             \
  {                                                                               \
    true,                           /* Delay on I2S data. */                      \
    false,                          /* No DMA split. */                           \
    false,                          /* Stereo mode. */                            \
    SL_HAL_USART_I2S_FORMAT_W16D16, /* 16-bit word, 16-bit data */                \
    SL_HAL_USART_JUSTIFY_LEFT,      /* Data is left-justified within the frame */ \
    {                                                                             \
      true,                         /* Master mode. */                            \
      false,                        /* Send least significant bit first. */       \
      false,                        /* No CS invert. */                           \
      false,                        /* No AUTOTX mode. */                         \
      false,                        /* No AUTOCS mode. */                         \
      false,                        /* Not USART PRS input mode. */               \
      0,                            /* PRS channel 0. */                          \
      0,                            /* Auto CS Hold cycles. */                    \
      0,                            /* Auto CS Setup cycles. */                   \
      0,                            /* Clock divider default value. */            \
      SL_HAL_USART_DATA_BITS_8,     /* 8 databits. */                             \
      SL_HAL_USART_CLOCK_MODE_0,    /* Clock idle low, sample on rising edge. */  \
    },                                                                            \
  }

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initialize USART/UART for normal asynchronous mode.
 *
 * @details
 *   This function will configure basic settings to operate in normal
 *   asynchronous mode.
 *
 *   A special control setup not covered by this function must be done after
 *   using this function by direct modification of the CTRL register.
 *
 *   Notice that pins used by the USART/UART module must be properly configured
 *   by the user explicitly for the USART/UART to work as intended.
 *   (When configuring pins, remember to consider the sequence of
 *   configuration to avoid unintended pulses/glitches on output
 *   pins.)
 *
 * @param[in] usart
 *   A pointer to the USART/UART peripheral register block.
 *
 * @param[in] init
 *   A pointer to the initialization structure used to configure
 *   the basic async setup.
 ******************************************************************************/
void sl_hal_usart_init_async(USART_TypeDef *usart,
                             const sl_hal_usart_async_init_t *init);

/***************************************************************************//**
 * @brief
 *   Calculate baudrate for USART/UART given reference frequency, clock division,
 *   and oversampling rate.
 *
 * @details
 *   This function returns the baudrate that a USART/UART module will use
 *   if configured with the given frequency, clock divider, and mode. Notice that
 *   this function will not use the hardware configuration. It can be used
 *   to determine if a given configuration is sufficiently accurate for the
 *   application.
 *
 * @param[in] ref_freq
 *   USART/UART HF peripheral frequency used.
 *
 * @param[in] clk_div
 *   A clock division factor to be used.
 *
 * @param[in] ovs
 *   Oversample value of the USART.
 *
 * @return
 *   Baudrate with given settings.
 ******************************************************************************/
uint32_t sl_hal_usart_async_calculate_baudrate(uint32_t ref_freq,
                                               uint32_t clk_div,
                                               sl_hal_usart_ovs_t ovs);

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
 * @return
 *    USART clock divider for desired baud rate.
 ******************************************************************************/
uint32_t sl_hal_usart_async_calculate_clock_div(uint32_t ref_freq,
                                                uint32_t baudrate,
                                                sl_hal_usart_ovs_t ovs);

/***************************************************************************//**
 * @brief
 *   Initialize USART for synchronous mode.
 *
 * @details
 *   This function will configure basic settings to operate in
 *   synchronous mode.
 *
 *   A special control setup not covered by this function must be done after
 *   using this function by direct modification of the CTRL register.
 *
 *   Notice that pins used by the USART module must be properly configured
 *   by the user explicitly for the USART to work as intended.
 *   (When configuring pins remember to consider the sequence of
 *   configuration to avoid unintended pulses/glitches on output
 *   pins.)
 *
 * @param[in] usart
 *   A pointer to the USART peripheral register block.
 *   (UART does not support this mode.)
 *
 * @param[in] init
 *   A pointer to the initialization structure used to configure
 *   basic async setup.
 ******************************************************************************/
void sl_hal_usart_init_sync(USART_TypeDef *usart,
                            const sl_hal_usart_sync_init_t *init);

/***************************************************************************//**
 * @brief
 *   Calculate baudrate for USART/UART (sync mode) given reference frequency,
 *   clock division.
 *
 * @details
 *   This function returns the baudrate that a USART/UART module will use
 *   if configured with the given frequency, clock divider, and mode. Notice that
 *   this function will not use the hardware configuration. It can be used
 *   to determine if a given configuration is sufficiently accurate for the
 *   application.
 *
 * @param[in] ref_freq
 *   USART/UART HF peripheral frequency used.
 *
 * @param[in] clk_div
 *   A clock division factor to be used.
 *
 * @return
 *   Baudrate with given settings.
 ******************************************************************************/
uint32_t sl_hal_usart_sync_calculate_baudrate(uint32_t ref_freq, uint32_t clk_div);

/***************************************************************************//**
 * @brief
 *   Calculate UART (sync mode) clock divider using given reference frequency
 *   and baudrate.
 *

 *  @details
 *  The configuration will be set to use a baudrate less than or equal to the desired baudrate
 *  to ensure that the baudrate does not exceed the requested value.
 *
 *  For USART peripherals, fractional clock division is suppressed
 *  in synchronous mode by this function, even though the hardware allows it.
 *  This restriction prevents timing violations causing framing errors and data corruption at high frequencies.
 *  The fractional clock division modifies the clock cycles to average out around the desired baudrate,
 *  adjusting some half periods clock cycle count.
 *  Fractional divider affects the duty cycle of the SCLK, impact is greater at high baudrates where there are few clock cycle count per
 *  half period, and can cause timing issues like MISO setup time violations.
 *  The clock divider will be set to an integer value, so the reference clock
 *  will only be divided by 2N (where N is a positive integer).
 *
 *  For example, if the reference clock is 39 MHz, the maximum baudrate would be 19.5 MHz (RefClock/2).
 *  If the baudrate is set to 19.5 MHz, the clock divider will be set to 0 and the baudrate will be 19.5 MHz.
 *  If the baudrate is [9.75 - 19.5[ MHz, the clock divider will be set to 1, and the actual baudrate will be (RefClock/4) = 9.75 MHz.
 *  If the baudrate is [6.5 - 9.75[ MHz, the clock divider will be set to 2, and the actual baudrate will be (RefClock/6) = 6.5 MHz.
 *  If the baudrate is [4.875 - 6.5[ MHz, the clock divider will be set to 3, and the actual baudrate will be (RefClock/8) = 4.875 MHz.
 *
 *  @warning
 *  If you require fractional clock division, you may manually modify the CLKDIV register.
 *  Users should ensure compliance with USART synchronous mode timing requirements
 *  and consider using integer division ratios of the reference clock for reliable
 *  operation at high baudrates.
 *
 * @param[in] ref_freq
 *   Peripheral clock frequency.
 *
 * @param[in] baudrate
 *   Transmission speed of the USART/UART interface.
 *
 * @return
 * USART clock divider for desired baud rate.
 ******************************************************************************/
uint32_t sl_hal_usart_sync_calculate_clock_div(uint32_t ref_freq, uint32_t baudrate);

/***************************************************************************//**
 * @brief
 *   Initialize USART for asynchronous IrDA mode.
 *
 * @details This function will configure basic settings to operate in
 *   asynchronous IrDA mode.
 *
 *   A special control setup not covered by this function must be done after
 *   using this function by direct modification of the CTRL and IRCTRL
 *   registers.
 *
 *   Notice that pins used by the USART/UART module must be properly configured
 *   by the user explicitly for the USART/UART to work as intended.
 *   (When configuring pins, remember to consider the sequence of
 *   configuration to avoid unintended pulses/glitches on output
 *   pins.)
 *
 * @param[in] usart
 *   A pointer to the USART peripheral register block.
 *
 * @param[in] init
 *   A pointer to the initialization structure used to configure
 *   async IrDA setup.
 *
 * @note
 *   Not all USART instances support IrDA.
 ******************************************************************************/
void sl_hal_usart_init_irda(USART_TypeDef *usart,
                            const sl_hal_usart_irda_init_t *init);

/***************************************************************************//**
 * @brief
 *   Initialize USART for I2S mode.
 *
 * @details This function will configure basic settings to operate in I2S
 *   mode.
 *
 *   A special control setup not covered by this function must be done after
 *   using this function by direct modification of the CTRL and I2SCTRL
 *   registers.
 *
 *   Notice that pins used by the USART module must be properly configured
 *   by the user explicitly for the USART to work as intended.
 *   (When configuring pins, remember to consider the sequence of
 *   configuration to avoid unintended pulses/glitches on output
 *   pins.)
 *
 * @param[in] usart
 *   A pointer to the USART peripheral register block.
 *   (UART does not support this mode.)
 *
 * @param[in] init
 *   A pointer to the initialization structure used to
 *   configure the basic I2S setup.
 *
 * @note
 *   This function does not apply to all USART's.
 ******************************************************************************/
void sl_hal_usart_init_i2s(USART_TypeDef *usart,
                           const sl_hal_usart_i2s_init_t *init);

/***************************************************************************//**
 * Reset USART/UART to the same state that it was in after a hardware reset.
 *
 * @param[in] usart
 *   A pointer to USART/UART peripheral register block.
 ******************************************************************************/
void sl_hal_usart_reset(USART_TypeDef *usart);

/***************************************************************************//**
 * @brief
 *   Transmit one 4-9 bit frame.
 *
 * @details Depending on the frame length configuration, 4-8 (least significant)
 *   bits from @p data are transmitted. If the frame length is 9, 8 bits are
 *   transmitted from @p data and one bit as specified by CTRL register, BIT8DV
 *   field. See sl_hal_usart_tx_ext() for transmitting 9 bit frame with full
 *   control of all 9 bits.
 *
 *   Notice that possible parity/stop bits in asynchronous mode are not
 *   considered part of a specified frame bit length.
 *
 * @note This function will stall if the buffer is full until the buffer becomes
 *       available.
 *
 * @param[in] usart
 *   A pointer to the USART/UART peripheral register block.
 *
 * @param[in] data
 *   Data to transmit. See details above for more information.
 ******************************************************************************/
__INLINE void sl_hal_usart_tx(USART_TypeDef *usart, uint8_t data)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  while (!(usart->STATUS & USART_STATUS_TXBL)) {
    // Wait for TX buffer to be empty or half full.
  }
  usart->TXDATA = (uint32_t)data;
}

/***************************************************************************//**
 * @brief
 *   Perform one 8 bit frame SPI transfer.
 *
 * @note This function will stall if the transmit buffer is full. When a transmit
 *   buffer becomes available, data is written and the function will wait until
 *   data is fully transmitted. The SPI return value is then read out and
 *   returned.
 *
 * @param[in] usart
 *   A pointer to the USART peripheral register block.
 *
 * @param[in] data
 *   Data to transmit.
 *
 * @return
 *   Data received.
 ******************************************************************************/
__INLINE uint8_t sl_hal_usart_spi_transfer(USART_TypeDef *usart, uint8_t data)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  while (!(usart->STATUS & USART_STATUS_TXBL)) {
    // Wait for TX buffer to be empty or half full.
  }
  usart->TXDATA = (uint32_t)data;
  while (!(usart->STATUS & USART_STATUS_TXC)) {
    // Wait until transmission complete.
  }
  return (uint8_t)usart->RXDATA;
}

/***************************************************************************//**
 * @brief
 *   Transmit two 4-9 bit frames or one 10-16 bit frame.
 *
 * @details Depending on the frame length configuration, 4-8 (least significant)
 *   bits from each byte in @p data are transmitted. If frame length is 9, 8 bits
 *   are transmitted from each byte in @p data adding one bit as specified by the
 *   CTRL register, BIT8DV field, to each byte. See sl_hal_usart_tx_double_ext()
 *   for transmitting two 9 bit frames with full control of all 9 bits.
 *
 *   If the frame length is 10-16, 10-16 (least significant) bits from @p data
 *   are transmitted.
 *
 *   Notice that possible parity/stop bits in asynchronous mode are not
 *   considered part of a specified frame bit length.
 *
 * @note This function will stall if the buffer is full until the buffer becomes
 *       available.
 *
 * @param[in] usart
 *   A pointer to the USART/UART peripheral register block.
 *
 * @param[in] data
 *   Data to transmit, the least significant byte holds the frame
 *   transmitted first. See details above for more info.
 ******************************************************************************/
__INLINE void sl_hal_usart_tx_double(USART_TypeDef *usart, uint16_t data)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  while (!(usart->STATUS & USART_STATUS_TXBL)) {
    // Wait for TX buffer to be empty or half full.
  }
  usart->TXDOUBLE = (uint32_t)data;
}

/***************************************************************************//**
 * @brief
 *   Transmit two 4-9 bit frames or one 10-16 bit frame with extended control.
 *
 * @details
 *   Notice that possible parity/stop bits in asynchronous mode are not
 *   considered part of a specified frame bit length.
 *
 * @note
 *   This function will stall if the buffer is full until the buffer becomes
 *   available.
 *
 * @param[in] usart
 *   A pointer to the USART/UART peripheral register block.
 *
 * @param[in] data
 *   Data to transmit with extended control. Contains two 16 bit
 *   words concatenated. Least significant word holds the frame
 *   transmitted first. If the frame length is 4-9, two frames
 *   with 4-9 least significant bits from each 16 bit word are
 *   transmitted.
 *
 * @par
 *   If the frame length is 10-16 bits, 8 data bits are taken from the least
 *   significant 16 bit word and the remaining bits from the other 16 bit word.
 *
 * @par
 *   Additional control bits are available as documented in the reference
 *   manual (set to 0 if not used). For 10-16 bit frame length, these control
 *   bits are taken from the most significant 16 bit word.
 ******************************************************************************/
__INLINE void sl_hal_usart_tx_double_ext(USART_TypeDef *usart, uint32_t data)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  while (!(usart->STATUS & USART_STATUS_TXBL)) {
    // Wait for TX buffer to be empty or half full.
  }
  usart->TXDOUBLEX = data;
}

/***************************************************************************//**
 * @brief
 *   Transmit one 4-9 bit frame with extended control.
 *
 * @details
 *   Notice that possible parity/stop bits in asynchronous mode are not
 *   considered part of a specified frame bit length.
 *
 * @note
 *   This function will stall if the buffer is full until the buffer becomes
 *   available.
 *
 * @param[in] usart
 *   A pointer to the USART/UART peripheral register block.
 *
 * @param[in] data
 *   Data to transmit with extended control. Least significant bit
 *   contains frame bits. Additional control bits are available as
 *   documented in the reference manual (set to 0 if not used).
 ******************************************************************************/
__INLINE void sl_hal_usart_tx_ext(USART_TypeDef *usart, uint16_t data)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  while (!(usart->STATUS & USART_STATUS_TXBL)) {
    // Wait for TX buffer to be empty or half full.
  }
  usart->TXDATAX = (uint32_t)data;
}

/***************************************************************************//**
 * @brief
 *   Enable automatic enabling of reception using the PRS as a trigger.
 *
 * @param[in] usart
 *   Pointer to the USART peripheral register block.
 *
 * @param[in] channel
 *   PRS channel to use.
 ******************************************************************************/
void sl_hal_usart_enable_rx_prs_trigger(USART_TypeDef *usart,
                                        uint8_t channel);

/***************************************************************************//**
 * @brief
 *   Disable automatic enabling of reception using the PRS as a trigger.
 *
 * @param[in] usart
 *   Pointer to the USART peripheral register block.
 ******************************************************************************/
void sl_hal_usart_disable_rx_prs_trigger(USART_TypeDef *usart);

/***************************************************************************//**
 * @brief
 *   Enable automatic enabling of transmission using the PRS as a trigger.
 *
 * @param[in] usart
 *   Pointer to the USART peripheral register block.
 *
 * @param[in] channel
 *   PRS channel number.
 ******************************************************************************/
void sl_hal_usart_enable_tx_prs_trigger(USART_TypeDef *usart, uint8_t channel);

/***************************************************************************//**
 * @brief
 *   Disable automatic enabling of transmission using the PRS as a trigger.
 *
 * @param[in] usart
 *   Pointer to the USART peripheral register block.
 ******************************************************************************/
void sl_hal_usart_disable_tx_prs_trigger(USART_TypeDef *usart);

/***************************************************************************//**
 * @brief
 *   Set UART mode clock divider.
 *
 * @param[in] usart
 *   Pointer to the USART peripheral register block.
 *
 * @param[in] clock_div
 *   Clock divider value.
 ******************************************************************************/
__INLINE void sl_hal_usart_async_set_clock_div(USART_TypeDef *usart,
                                               uint32_t clock_div)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  // Make sure clock divider value is valid.
  EFM_ASSERT(clock_div <= (_USART_CLKDIV_DIV_MASK >> _USART_CLKDIV_DIV_SHIFT));

  usart->CLKDIV = ((clock_div << _USART_CLKDIV_DIV_SHIFT) & _USART_CLKDIV_DIV_MASK);
}

/***************************************************************************//**
 * @brief
 *   Get UART mode clock divider.
 *
 * @param[in] usart
 *   Pointer to the USART peripheral register block.
 *
 * @return
 *   Clock divider value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_usart_async_get_clock_div(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  // Make sure module is enabled
  EFM_ASSERT(usart->EN & _USART_EN_EN_MASK);

  return ((usart->CLKDIV & _USART_CLKDIV_DIV_MASK) >> _USART_CLKDIV_DIV_SHIFT);
}

/***************************************************************************//**
 * @brief
 *   Enable USART.
 *
 * @param[in] usart
 *   A pointer to the USART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_usart_enable(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  // Enable peripheral.
  usart->EN_SET = USART_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Disable USART.
 *
 * @param[in] usart
 *   A pointer to the USART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_usart_disable(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  if (usart->EN != USART_EN_EN) {
    return;
  }

  // Disable USART
  usart->EN_CLR = USART_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Enable USART receiver.
 *
 * @param[in] usart
 *   A pointer to the USART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_usart_enable_rx(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  usart->CMD_SET = USART_CMD_RXEN;
}

/***************************************************************************//**
 * @brief
 *   Disable USART receiver.
 *
 * @param[in] usart
 *   A pointer to the USART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_usart_disable_rx(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  usart->CMD_SET = USART_CMD_RXDIS;
}

/***************************************************************************//**
 * @brief
 *   Enable USART transmitter.
 *
 * @param[in] usart
 *   A pointer to the USART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_usart_enable_tx(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  usart->CMD_SET = USART_CMD_TXEN;
}

/***************************************************************************//**
 * @brief
 *   Disable USART transmitter.
 *
 * @param[in] usart
 *   A pointer to the USART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_usart_disable_tx(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  usart->CMD_SET = USART_CMD_TXDIS;
}

/***************************************************************************//**
 * @brief
 *   Clear RX FIFO.
 *
 * @param[in] usart
 *   Pointer to the USART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_usart_clear_rx(USART_TypeDef *usart)
{
  // Make sure that the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  // Make sure receiver is disabled.
  EFM_ASSERT(((usart->STATUS & _USART_STATUS_RXENS_MASK) >> _USART_STATUS_RXENS_SHIFT) == 0);

  usart->CMD_SET = USART_CMD_CLEARRX;
}

/***************************************************************************//**
 * @brief
 *   Clear TX FIFO.
 *
 * @param[in] usart
 *   Pointer to the USART peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_usart_clear_tx(USART_TypeDef *usart)
{
  // Make sure that the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  // Make sure transmitter is disabled.
  EFM_ASSERT(((usart->STATUS & _USART_STATUS_TXENS_MASK) >> _USART_STATUS_TXENS_SHIFT) == 0);

  usart->CMD_SET = USART_CMD_CLEARTX;
}

/***************************************************************************//**
 * @brief
 *   Receive one 4-8 bit frame, (or part of 10-16 bit frame).
 *
 * @details
 *   This function is normally used to receive one frame when operating
 *   with frame length 4-8 bits. See @ref sl_hal_usart_rx_ext() for reception of
 *   9 bit frames.
 *
 *   Notice that possible parity/stop bits in asynchronous mode are not
 *   considered part of a specified frame bit length.
 *
 * @note
 *   This function will stall if the buffer is empty until data is received.
 *   Alternatively, the user can explicitly check whether data is available.
 *   If data is available, call @ref sl_hal_usart_rx_data_get() to read the
 *   RXDATA register directly.
 *
 * @param[in] usart
 *   A pointer to the USART/UART peripheral register block.
 *
 * @return
 *   Data received.
 ******************************************************************************/
__INLINE uint8_t sl_hal_usart_rx(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  while (!(usart->STATUS & USART_STATUS_RXDATAV)) {
    // Wait for data available in RX buffer.
  }

  return (uint8_t)usart->RXDATA;
}

/***************************************************************************//**
 * @brief
 *   Receive two 4-8 bit frames or one 10-16 bit frame.
 *
 * @details
 *   This function is normally used to receive one frame when operating
 *   with frame length 10-16 bits. See @ref sl_hal_usart_rx_double_ext() for
 *   reception of two 9 bit frames.
 *
 *   Notice that possible parity/stop bits in asynchronous mode are not
 *   considered part of a specified frame bit length.
 *
 * @note
 *   This function will stall if the buffer is empty until data is received.
 *   Alternatively, the user can explicitly check whether data is available.
 *   If data is available, call @ref sl_hal_usart_rx_double_get() to read the
 *   RXDOUBLE register directly.
 *
 * @param[in] usart
 *   A pointer to the USART/UART peripheral register block.
 *
 * @return
 *   Data received.
 ******************************************************************************/
__INLINE uint16_t sl_hal_usart_rx_double(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  while (!(usart->STATUS & USART_STATUS_RXFULL)) {
    // Wait for RX buffer to be full.
  }

  return (uint16_t)usart->RXDOUBLE;
}

/***************************************************************************//**
 * @brief
 *   Receive two 4-9 bit frames, or one 10-16 bit frame with extended
 *   information.
 *
 * @details
 *   This function is normally used to receive one frame when operating
 *   with frame length 10-16 bits and additional RX status information is required.
 *
 *   Notice that possible parity/stop bits in asynchronous mode are not
 *   considered part of a specified frame bit length.
 *
 * @note
 *   This function will stall if buffer is empty until data is received.
 *   Alternatively, the user can explicitly check whether data is available.
 *   If data is available, call @ref sl_hal_usart_rx_double_x_get() to read the
 *   RXDOUBLEX register directly.
 *
 * @param[in] usart
 *   A pointer to the USART/UART peripheral register block.
 *
 * @return
 *   Data received.
 ******************************************************************************/
__INLINE uint32_t sl_hal_usart_rx_double_ext(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  while (!(usart->STATUS & USART_STATUS_RXFULL)) {
    // Wait for RX buffer to be full.
  }

  return usart->RXDOUBLEX;
}

/***************************************************************************//**
 * @brief
 *   Receive one 4-9 bit frame (or part of 10-16 bit frame) with extended
 *   information.
 *
 * @details
 *   This function is normally used to receive one frame when operating
 *   with frame length 4-9 bits and additional RX status information is required.
 *
 *   Notice that possible parity/stop bits in asynchronous mode are not
 *   considered part of a specified frame bit length.
 *
 * @note
 *   This function will stall if the buffer is empty until data is received.
 *   Alternatively, the user can explicitly check whether data is available.
 *   If data is available, call @ref sl_hal_usart_rx_data_get() to read the
 *   RXDATAX register directly.
 *
 * @param[in] usart
 *   A pointer to the USART/UART peripheral register block.
 *
 * @return
 *   Data received.
 ******************************************************************************/
__INLINE uint16_t sl_hal_usart_rx_ext(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  while (!(usart->STATUS & USART_STATUS_RXDATAV)) {
    // Wait for data available in RX buffer.
  }

  return (uint16_t)usart->RXDATAX;
}

/***************************************************************************//**
 * @brief
 *   Receive one 4-8 bit frame, (or part of 10-16 bit frame).
 *
 * @details
 *   This function is used to quickly receive one 4-8 bits frame by reading
 *   the RXDATA register directly, without checking the STATUS register for the
 *   RXDATAV flag. This can be useful from the RXDATAV interrupt handler,
 *   i.e., waiting is superfluous, in order to quickly read the received data.
 *   Please refer to @ref sl_hal_usart_rx_ext() for reception of 9 bit frames.
 *
 * @note
 *   Because this function does not check whether the RXDATA register actually
 *   holds valid data, it should only be used in situations when it is certain
 *   that there is valid data, ensured by some external program routine, e.g.,
 *   when handling an RXDATAV interrupt. The @ref sl_hal_usart_rx() is normally
 *   a better choice if the validity of the RXDATA register is not certain.
 *
 * @note
 *   Notice that possible parity/stop bits in asynchronous mode are not
 *   considered part of specified frame bit length.
 *
 * @param[in] usart
 *   Pointer to USART/UART peripheral register block.
 *
 * @return
 *   Data received.
 ******************************************************************************/
__INLINE uint8_t sl_hal_usart_rx_data_get(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  return (uint8_t)usart->RXDATA;
}

/***************************************************************************//**
 * @brief
 *   Receive two 4-8 bit frames, or one 10-16 bit frame.
 *
 * @details
 *   This function is used to quickly receive one 10-16 bits frame or
 *   two 4-8 bit frames by reading the RXDOUBLE register directly, without
 *   checking the STATUS register for the RXDATAV flag. This can be useful from
 *   the RXDATAV interrupt handler, i.e., waiting is superfluous, in order to
 *   quickly read the received data.
 *   This function is normally used to receive one frame when operating with
 *   frame length 10-16 bits. Please refer to @ref sl_hal_usart_rx_double_x_get()
 *   for reception of two 9 bit frames.
 *
 * @note
 *   Because this function does not check whether the RXDOUBLE register
 *   actually holds valid data, it should only be used in situations when it is
 *   certain that there is valid data, ensured by some external program routine, e.g.,
 *   when handling an RXDATAV interrupt. The @ref sl_hal_usart_rx_double() is
 *   normally a better choice if the validity of the RXDOUBLE register is not
 *   certain.
 *
 * @note
 *   Notice that possible parity/stop bits in asynchronous mode are not
 *   considered part of specified frame bit length.
 *
 * @param[in] usart
 *   Pointer to USART/UART peripheral register block.
 *
 * @return
 *   Data received.
 ******************************************************************************/
__INLINE uint16_t sl_hal_usart_rx_double_get(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  return (uint16_t)usart->RXDOUBLE;
}

/***************************************************************************//**
 * @brief
 *   Receive two 4-9 bit frames, or one 10-16 bit frame with extended
 *   information.
 *
 * @details
 *   This function is used to quickly receive one 10-16 bits frame or
 *   two 4-9 bit frames by reading the RXDOUBLEX register directly, without
 *   checking the STATUS register for the RXDATAV flag. This can be useful
 *   from the RXDATAV interrupt handler, i.e., waiting is superfluous, in order
 *   to quickly read the received data.
 *
 * @note
 *   Because this function does not check whether the RXDOUBLEX register actually
 *   holds valid data, it should only be used in situations when it is certain
 *   that there is valid data, ensured by some external program routine, e.g.,
 *   when handling an RXDATAV interrupt. The @ref sl_hal_usart_rx_double_ext() is
 *   normally a better choice if the validity of the RXDOUBLEX register is not
 *   certain.
 *
 * @note
 *   Notice that possible parity/stop bits in asynchronous mode are not
 *   considered part of specified frame bit length.
 *
 * @param[in] usart
 *   Pointer to USART/UART peripheral register block.
 *
 * @return
 *   Data received.
 ******************************************************************************/
__INLINE uint32_t sl_hal_usart_rx_double_x_get(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  return usart->RXDOUBLEX;
}

/***************************************************************************//**
 * @brief
 *   Receive one 4-9 bit frame, (or part of 10-16 bit frame) with extended
 *   information.
 *
 * @details
 *   This function is used to quickly receive one 4-9 bit frame, (or part of
 *   10-16 bit frame) with extended information by reading the RXDATAX register
 *   directly, without checking the STATUS register for the RXDATAV flag. This
 *   can be useful from the RXDATAV interrupt handler, i.e., waiting is
 *   superfluous, in order to quickly read the received data.
 *
 * @note
 *   Because this function does not check whether the RXDATAX register actually
 *   holds valid data, it should only be used in situations when it is certain
 *   that there is valid data, ensured by some external program routine, e.g.,
 *   when handling an RXDATAV interrupt. The @ref sl_hal_usart_rx_ext() is normally
 *   a better choice if the validity of the RXDATAX register is not certain.
 *
 * @note
 *   Notice that possible parity/stop bits in asynchronous mode are not
 *   considered part of specified frame bit length.
 *
 * @param[in] usart
 *   Pointer to USART/UART peripheral register block.
 *
 * @return
 *   Data received.
 ******************************************************************************/
__INLINE uint16_t sl_hal_usart_rx_data_x_get(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  return (uint16_t)usart->RXDATAX;
}

/***************************************************************************//**
 * @brief
 *   Clear one or more pending USART interrupts.
 *
 * @param[in] usart
 *   Pointer to the USART/UART peripheral register block.
 *
 * @param[in] flags
 *   Pending USART/UART interrupt source(s) to clear. Use one or
 *   more valid interrupt flags for the USART module (USART_IF_nnn)
 *   OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_usart_clear_interrupts(USART_TypeDef *usart, uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  usart->IF_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Disable one or more USART interrupts.
 *
 * @param[in] usart
 *   Pointer to the USART/UART peripheral register block.
 *
 * @param[in] flags
 *   USART/UART interrupt source(s) to disable. Use one or
 *   more valid interrupt flags for the USART module (USART_IF_nnn)
 *   OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_usart_disable_interrupts(USART_TypeDef *usart, uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  usart->IEN_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Enable one or more USART interrupts.
 *
 * @note
 *   Depending on the use, a pending interrupt may already be set prior to
 *   enabling the interrupt. To ignore a pending interrupt, consider using
 *   USART_IntClear() prior to enabling the interrupt.
 *
 * @param[in] usart
 *   Pointer to the USART/UART peripheral register block.
 *
 * @param[in] flags
 *   USART/UART interrupt source(s) to enable. Use one or
 *   more valid interrupt flags for the USART module (USART_IF_nnn)
 *   OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_usart_enable_interrupts(USART_TypeDef *usart, uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  usart->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Get pending USART interrupt flags.
 *
 * @note
 *   The event bits are not cleared by the use of this function.
 *
 * @param[in] usart
 *   Pointer to the USART/UART peripheral register block.
 *
 * @return
 *   USART/UART interrupt source(s) pending. Returns one or more valid
 *   interrupt flags for the USART module (USART_IF_nnn) OR'ed together.
 ******************************************************************************/
__INLINE uint32_t sl_hal_usart_get_pending_interrupts(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  return usart->IF;
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending USART interrupt flags.
 *   Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @param[in] usart
 *   Pointer to the USART/UART peripheral register block.
 *
 * @note Interrupt
 *   flags are not cleared by the use of this function.
 *
 * @return Pending and enabled USART interrupt sources.
 *   The return value is the bitwise AND combination of
 *   - the OR combination of enabled interrupt sources in USARTx_IEN_nnn
 *     register (USARTx_IEN_nnn) and
 *   - the OR combination of valid interrupt flags of the USART module
 *     (USARTx_IF_nnn).
 ******************************************************************************/
__INLINE uint32_t sl_hal_usart_get_enabled_pending_interrupts(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  uint32_t ien;

  // Store USARTx->IEN in temporary variable in order to define explicit order
  // of volatile accesses.
  ien = usart->IEN;

  // Bitwise AND of pending and enabled interrupts.
  return usart->IF & ien;
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending USART interrupts from SW.
 *
 * @param[in] usart
 *   A pointer to the USART/UART peripheral register block.
 *
 * @param[in] flags
 *   USART/UART interrupt source(s) to set to pending. Use one or
 *   more valid interrupt flags for the USART module(USART_IF_nnn)
 *   OR'ed together.
 ******************************************************************************/
__INLINE void sl_hal_usart_set_interrupts(USART_TypeDef *usart, uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  usart->IF_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Get USART STATUS register.
 *
 * @param[in] usart
 *   A pointer to the USART/UART peripheral register block.
 *
 * @return
 *   STATUS register value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_usart_get_status(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  return usart->STATUS;
}

/** @} (end addtogroup usart) */

#ifdef __cplusplus
}
#endif

/* *INDENT-OFF* */
/****************************************************************************//**
 * @addtogroup usart USART - Synchronous/Asynchronous Serial
 * @{
 *
 * @li @ref usart_intro
 * @li @ref usart_example
 *
 *@n @section usart_intro Introduction
 *  The Universal Synchronous/Asynchronous Receiver/Transmitter (USART)
 *  is a very flexible serial I/O module. It supports full duplex asynchronous UART
 *  communication as well as RS-485, SPI, MicroWire, and 3-wire. It can also interface
 *  with ISO7816 Smart-Cards, and IrDA devices.
 *
 *  The USART has a wide selection of operating modes, frame formats, and baud rates.
 *  All features are supported through the API of this module.
 *
 *  Triple buffering and DMA support makes high data-rates possible with minimal
 *  CPU intervention. It is possible to transmit and receive large frames while
 *  the MCU remains in EM1 Sleep.
 *
 *  This module does not support DMA configuration. The UARTDRV and SPIDRV drivers
 *  provide full support for DMA and more.
 *
 *   The following steps are necessary for basic operation:
 *
 *   @note UARTDRV supports all types of UART flow control. Software assisted
 *         hardware flow control is available for parts without true UART hardware
 *         flow control.
 *
 *@n @section usart_example Example
 *
 *  USART Asynchronous UART TX example:
 *  @code{.c}
 *  {
 *    #define GPIO_TX    PB0
 *
 *    sl_hal_usart_async_init_t init = SL_HAL_USART_INIT_ASYNC_DEFAULT;
 *
 *    // Configure the clocks.
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_USART0);
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
 *
 *    // Calculate and configure the usart's clock divider.
 *    uint32_t baudrate = 38400;  // Desired baudrate.
 *    sl_clock_branch_t clock_branch = sl_device_peripheral_get_clock_branch(SL_PERIPHERAL_USART0);
 *    uint32_t freq;
 *    sl_clock_manager_get_clock_branch_frequency(clock_branch, &freq);
 *    init.clock_div = sl_hal_usart_async_calculate_clock_div(freq, baudrate, init.oversampling);
 *
 *    // Configure the USART port.
 *    sl_gpio_set_pin_mode(GPIO_TX, SL_GPIO_MODE_PUSH_PULL, 0);
 *    GPIO->USARTROUTE[USART_NUM(USART0)].TXROUTE = (GPIO_TX->port << _GPIO_USART_TXROUTE_PORT_SHIFT)
 *                                                | (GPIO_TX->pin << _GPIO_USART_TXROUTE_PIN_SHIFT);
 *    GPIO->USARTROUTE[USART_NUM(USART0)].ROUTEEN = GPIO_USART_ROUTEEN_TXPEN;
 *
 *    // Initialize the USART.
 *    sl_hal_usart_init_async(USART0, &init);
 *    sl_hal_usart_enable(USART0);
 *    sl_hal_usart_enable_tx(USART0);
 *
 *    uint16_t data = 0xFF;
 *    sl_hal_usart_tx(USART0, data);
 *  }
 *
 *  @endcode
 *
 *  USART Synchronous SPI Initialization example:
 *  @code{.c}
 *  {
 *    #define GPIO_MOSI    PC0
 *    #define GPIO_MISO    PC1
 *    #define GPIO_SCLK    PC2
 *
 *    sl_hal_usart_sync_init_t init = SL_HAL_USART_INIT_SYNC_DEFAULT;
 *    // Operate as SPI master.
 *    init.master = true;
 *    // Clock idle low, sample on falling edge.
 *    init.clock_mode = SL_HAL_USART_CLOCK_MODE_1;
 *
 *    // Configure the clocks.
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_USART1);
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
 *
 *    // Calculate and configure the usart's clock divider.
 *    uint32_t baudrate  = 1000000;  // Desired baudrate.
 *    sl_clock_branch_t clock_branch = sl_device_peripheral_get_clock_branch(SL_PERIPHERAL_USART1);
 *    uint32_t freq;
 *    sl_clock_manager_get_clock_branch_frequency(clock_branch, &freq);
 *    init.clock_div = sl_hal_usart_sync_calculate_clock_div(freq, baudrate);
 *
 *    // Configure the SPI ports.
 *    sl_gpio_set_pin_mode(GPIO_MOSI, SL_GPIO_MODE_PUSH_PULL, 0);
 *    sl_gpio_set_pin_mode(GPIO_MISO, SL_GPIO_MODE_INPUT, 0);
 *    sl_gpio_set_pin_mode(GPIO_SCLK, SL_GPIO_MODE_PUSH_PULL, 0);
 *
 *    // Connect USART to ports.
 *    GPIO->USARTROUTE[USART_NUM(USART1)].TXROUTE = (GPIO_MOSI->port << _GPIO_USART_TXROUTE_PORT_SHIFT)
 *                                                | (GPIO_MOSI->pin << _GPIO_USART_TXROUTE_PIN_SHIFT);
 *    GPIO->USARTROUTE[USART_NUM(USART1)].RXROUTE = (GPIO_MISO->port << _GPIO_USART_RXROUTE_PORT_SHIFT)
 *                                                | (GPIO_MISO->pin << _GPIO_USART_RXROUTE_PIN_SHIFT);
 *    GPIO->USARTROUTE[USART_NUM(USART1)].CLKROUTE = (GPIO_SCLK->port << _GPIO_USART_CLKROUTE_PORT_SHIFT)
 *                                                 | (GPIO_SCLK->pin << _GPIO_USART_CLKROUTE_PIN_SHIFT);
 *    GPIO->USARTROUTE[USART_NUM(USART1)].ROUTEEN = GPIO_USART_ROUTEEN_TXPEN | GPIO_USART_ROUTEEN_RXPEN | GPIO_USART_ROUTEEN_CLKPEN;
 *
 *    // Initialize the USART.
 *    sl_hal_usart_init_sync(USART1, &init);
 *    sl_hal_usart_enable(USART1);
 *    sl_hal_usart_enable_rx(USART1);
 *    sl_hal_usart_enable_tx(USART1);
 *  }
 *
 *  @endcode
 *
 * @} (end addtogroup usart)
 *******************************************************************************/
/* *INDENT-ON* */

#endif /* defined(USART_PRESENT)*/
#endif /* SL_HAL_USART_H */
