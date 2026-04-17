/***************************************************************************//**
 * @file
 * @brief Universal synchronous/asynchronous receiver/transmitter (USART/UART)
 *   Peripheral API
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

#include "sl_hal_usart.h"
#if defined(USART_PRESENT)

#include "sl_hal_prs.h"

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/
static void usart_prs_input(USART_TypeDef *usart, uint8_t channel);
static uint8_t usart_map_oversampling(uint32_t ref_freq,
                                      uint32_t baudrate,
                                      sl_hal_usart_ovs_t ovs);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
extern __INLINE void sl_hal_usart_enable(USART_TypeDef *usart);
extern __INLINE void sl_hal_usart_disable(USART_TypeDef *usart);
extern __INLINE void sl_hal_usart_enable_rx(USART_TypeDef *usart);
extern __INLINE void sl_hal_usart_disable_rx(USART_TypeDef *usart);
extern __INLINE void sl_hal_usart_enable_tx(USART_TypeDef *usart);
extern __INLINE void sl_hal_usart_disable_tx(USART_TypeDef *usart);
extern __INLINE void sl_hal_usart_clear_interrupts(USART_TypeDef *usart, uint32_t flags);
extern __INLINE void sl_hal_usart_disable_interrupts(USART_TypeDef *usart, uint32_t flags);
extern __INLINE void sl_hal_usart_enable_interrupts(USART_TypeDef *usart, uint32_t flags);
extern __INLINE void sl_hal_usart_set_interrupts(USART_TypeDef *usart, uint32_t flags);
extern __INLINE uint32_t sl_hal_usart_get_pending_interrupts(USART_TypeDef *usart);
extern __INLINE uint32_t sl_hal_usart_get_enabled_pending_interrupts(USART_TypeDef *usart);
extern __INLINE uint32_t sl_hal_usart_get_status(USART_TypeDef *usart);
extern __INLINE void sl_hal_usart_async_set_clock_div(USART_TypeDef *usart, uint32_t clock_div);
extern __INLINE uint32_t sl_hal_usart_async_get_clock_div(USART_TypeDef *usart);
extern __INLINE void sl_hal_usart_clear_rx(USART_TypeDef *usart);
extern __INLINE void sl_hal_usart_clear_tx(USART_TypeDef *usart);
extern __INLINE void sl_hal_usart_tx(USART_TypeDef *usart, uint8_t data);
extern __INLINE uint8_t sl_hal_usart_spi_transfer(USART_TypeDef *usart, uint8_t data);
extern __INLINE void sl_hal_usart_tx_double(USART_TypeDef *usart, uint16_t data);
extern __INLINE void sl_hal_usart_tx_double_ext(USART_TypeDef *usart, uint32_t data);
extern __INLINE void sl_hal_usart_tx_ext(USART_TypeDef *usart, uint16_t data);
extern __INLINE uint8_t sl_hal_usart_rx(USART_TypeDef *usart);
extern __INLINE uint16_t sl_hal_usart_rx_ext(USART_TypeDef *usart);
extern __INLINE uint16_t sl_hal_usart_rx_double(USART_TypeDef *usart);
extern __INLINE uint32_t sl_hal_usart_rx_double_ext(USART_TypeDef *usart);
extern __INLINE uint8_t sl_hal_usart_rx_data_get(USART_TypeDef *usart);
extern __INLINE uint16_t sl_hal_usart_rx_data_x_get(USART_TypeDef *usart);
extern __INLINE uint16_t sl_hal_usart_rx_double_get(USART_TypeDef *usart);
extern __INLINE uint32_t sl_hal_usart_rx_double_x_get(USART_TypeDef *usart);

/***************************************************************************//**
 * Initialize USART/UART for normal asynchronous mode.
 ******************************************************************************/
void sl_hal_usart_init_async(USART_TypeDef *usart,
                             const sl_hal_usart_async_init_t *init)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  // Initialize USART registers to hardware reset state.
  sl_hal_usart_reset(usart);
  sl_hal_usart_enable(usart);

  // Configure majority vote, auto CS mode, CS invert mode.
  usart->CTRL |= ((uint32_t)init->majority_vote << _USART_CTRL_MVDIS_SHIFT)
                 | ((uint32_t)init->auto_cs_enable << _USART_CTRL_AUTOCS_SHIFT)
                 | ((uint32_t)init->cs_invert << _USART_CTRL_CSINV_SHIFT);

  // Configure the PRS input mode.
  if (init->prs_rx_enable) {
    usart_prs_input(usart, init->prs_rx_channel);
  }

  // Configure databits, stopbits, and parity.
  usart->FRAME = ((uint32_t)init->data_bits << _USART_FRAME_DATABITS_SHIFT)
                 | ((uint32_t)init->stop_bits << _USART_FRAME_STOPBITS_SHIFT)
                 | ((uint32_t)init->parity << _USART_FRAME_PARITY_SHIFT);

  usart->CTRL_SET = (uint32_t)init->oversampling << _USART_CTRL_OVS_SHIFT;

  usart->CLKDIV = (init->clock_div << _USART_CLKDIV_DIV_SHIFT) & _USART_CLKDIV_DIV_MASK;

  usart->TIMING = (((uint32_t)init->auto_cs_hold << _USART_TIMING_CSHOLD_SHIFT)
                   & _USART_TIMING_CSHOLD_MASK)
                  | (((uint32_t)init->auto_cs_setup << _USART_TIMING_CSSETUP_SHIFT)
                     & _USART_TIMING_CSSETUP_MASK);

  if ((init->hw_flow_control == SL_HAL_USART_HW_FLOW_CONTROL_RTS)
      || (init->hw_flow_control == SL_HAL_USART_HW_FLOW_CONTROL_CTS_RTS)) {
    GPIO->USARTROUTE_SET[USART_NUM(usart)].ROUTEEN = GPIO_USART_ROUTEEN_RTSPEN;
  } else {
    GPIO->USARTROUTE_CLR[USART_NUM(usart)].ROUTEEN = GPIO_USART_ROUTEEN_RTSPEN;
  }

  if ((init->hw_flow_control == SL_HAL_USART_HW_FLOW_CONTROL_CTS)
      || (init->hw_flow_control == SL_HAL_USART_HW_FLOW_CONTROL_CTS_RTS)) {
    usart->CTRLX_SET = USART_CTRLX_CTSEN;
  } else {
    usart->CTRLX_CLR = USART_CTRLX_CTSEN;
  }
}

/***************************************************************************//**
 * Initialize USART for synchronous mode.
 ******************************************************************************/
void sl_hal_usart_init_sync(USART_TypeDef *usart,
                            const sl_hal_usart_sync_init_t *init)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  // Initialize USART registers to hardware reset state.
  sl_hal_usart_reset(usart);
  sl_hal_usart_enable(usart);

  // Set bits for synchronous mode.
  usart->CTRL |= USART_CTRL_SYNC
                 | init->clock_mode
                 | ((uint32_t)init->msb_first << _USART_CTRL_MSBF_SHIFT)
                 | ((uint32_t)init->auto_tx << _USART_CTRL_AUTOTX_SHIFT)
                 | ((uint32_t)init->auto_cs_enable << _USART_CTRL_AUTOCS_SHIFT)
                 | ((uint32_t)init->cs_invert << _USART_CTRL_CSINV_SHIFT);

  // Configure the PRS input mode.
  if (init->prs_rx_enable) {
    usart_prs_input(usart, init->prs_rx_channel);
  }

  // Configure databits, leave stopbits and parity at reset default (not used).
  usart->FRAME = ((uint32_t)init->data_bits << _USART_FRAME_DATABITS_SHIFT)
                 | USART_FRAME_STOPBITS_DEFAULT
                 | USART_FRAME_PARITY_DEFAULT;

  // Finally, enable (as specified).
  usart->CMD = ((uint32_t)init->master << _USART_CMD_MASTEREN_SHIFT);

  usart->CLKDIV = (init->clock_div << _USART_CLKDIV_DIV_SHIFT) & _USART_CLKDIV_DIV_MASK;
  usart->TIMING = (((uint32_t)init->auto_cs_hold << _USART_TIMING_CSHOLD_SHIFT)
                   & _USART_TIMING_CSHOLD_MASK)
                  | (((uint32_t)init->auto_cs_setup << _USART_TIMING_CSSETUP_SHIFT)
                     & _USART_TIMING_CSSETUP_MASK);
}

/***************************************************************************//**
 * Calculate baudrate for USART/UART (async mode) given reference frequency,
 * clock division, and oversampling rate.
 ******************************************************************************/
uint32_t sl_hal_usart_async_calculate_baudrate(uint32_t ref_freq,
                                               uint32_t clk_div,
                                               sl_hal_usart_ovs_t ovs)
{
  uint32_t oversample;
  uint64_t divisor;
  uint64_t factor;
  uint64_t remainder;
  uint64_t quotient;
  uint32_t br;

  // Make sure clock divider value is valid.
  clk_div <<= _USART_CLKDIV_DIV_SHIFT;
  EFM_ASSERT(clk_div <= _USART_CLKDIV_DIV_MASK);

  // Use integer division to avoid forcing in float division
  // utils and yet keep rounding effect errors to a minimum.
  switch (ovs) {
    case SL_HAL_USART_OVS_16:
      oversample = 1;
      factor     = 256 / 16;
      break;

    case SL_HAL_USART_OVS_8:
      oversample = 1;
      factor     = 256 / 8;
      break;

    case SL_HAL_USART_OVS_6:
      oversample = 3;
      factor     = 256 / 2;
      break;

    default:
      oversample = 1;
      factor     = 256 / 4;
      break;
  }

  // The basic problem with integer division in the above formula is that
  // the dividend (factor * fHFPERCLK) may become larger than a 32 bit
  // integer. Yet we want to evaluate the dividend first before dividing
  // to get as small rounding effects as possible. Too harsh restrictions
  // should not be made on the maximum fHFPERCLK value either.
  //
  // For division a/b,
  //
  // a = qb + r
  //
  // where q is the quotient and r is the remainder, both integers.
  //
  // The original baudrate formula can be rewritten as
  //
  // br = xa / b = x(qb + r)/b = xq + xr/b
  //
  // where x is 'factor', a is 'ref_freq' and b is 'divisor', referring to
  // variable names.

  // The divisor will never exceed max 32 bit value since
  // div <= _USART_CLKDIV_DIV_MASK (currently 0x1FFFC0 or 0x7FFFF8)
  // and 'oversample' has been reduced to <= 3.

  divisor = (uint64_t)(oversample * (256 + clk_div));

  quotient  = ref_freq / divisor;
  remainder = ref_freq % divisor;

  // The factor <= 128 and since divisor >= 256, the below cannot exceed the maximum
  // 32 bit value. However, factor * remainder can become larger than 32-bit
  // because of the size of _USART_CLKDIV_DIV_MASK on some families.
  br = (uint32_t)(factor * quotient);

  // The factor <= 128 and remainder < (oversample*(256 + div)), which
  // means dividend (factor * remainder) worst case is
  // 128 * (3 * (256 + _USART_CLKDIV_DIV_MASK)) = 0x1_8001_7400.

  br += (uint32_t)((factor * remainder) / divisor);

  return br;
}

/***************************************************************************//**
 * Calculate UART (async mode) clock divider using given reference frequency,
 * baudrate and oversampling.
 ******************************************************************************/
uint32_t sl_hal_usart_async_calculate_clock_div(uint32_t ref_freq,
                                                uint32_t baudrate,
                                                sl_hal_usart_ovs_t ovs)
{
  uint32_t clk_div;
  uint32_t oversample = 0;

  // Inhibit divide by 0
  EFM_ASSERT(baudrate);

  // Map oversampling.
  oversample = usart_map_oversampling(ref_freq, baudrate, ovs);

  // Calculate and set CLKDIV with fractional bits.
  // The added (oversample*baudrate)/2 in the first line is to round the
  // divisor to the nearest fractional divisor.
  // Devices with 5 fractional bits. CLKDIV[7:3]
  clk_div  = 32 * ref_freq + (oversample * baudrate) / 2;
  clk_div /= oversample * baudrate;
  clk_div -= 32;
  clk_div *= 8;

  // Verify that the resulting clock divider is within limits.
  EFM_ASSERT(clk_div <= _USART_CLKDIV_DIV_MASK);

  // If the EFM_ASSERT is not enabled, make sure not to write to reserved bits.
  clk_div = (clk_div & _USART_CLKDIV_DIV_MASK) >> _USART_CLKDIV_DIV_SHIFT;

  return clk_div;
}

/***************************************************************************//**
 * Calculate baudrate for USART/UART (sync mode) given reference frequency,
 * clock division.
 ******************************************************************************/
uint32_t sl_hal_usart_sync_calculate_baudrate(uint32_t ref_freq,
                                              uint32_t clk_div)
{
  uint32_t br;

  // Make sure clock divider value is valid.
  clk_div <<= _USART_CLKDIV_DIV_SHIFT;
  EFM_ASSERT(clk_div <= _USART_CLKDIV_DIV_MASK);

  // Baudrate calculation for synchronous mode.
  // This function expects clk_div to have integer-only division (fractional bits cleared)
  // for proper timing compliance in USART synchronous mode.
  // Baudrate is given by:
  // br = fHFPERCLK/(2 * (1 + (CLKDIV / 256)))
  // which can be rewritten to
  // br = (128 * fHFPERCLK)/(256 + CLKDIV)
  br = (uint32_t)(((uint64_t)128 * ref_freq) / (256 + clk_div));

  return br;
}

/***************************************************************************//**
 * Calculate SPI (sync mode) clock divider using given reference frequency and
 * baudrate.
 ******************************************************************************/
uint32_t sl_hal_usart_sync_calculate_clock_div(uint32_t ref_freq,
                                               uint32_t baudrate)
{
  uint32_t clk_div;

  // Inhibit divide by 0
  EFM_ASSERT(baudrate);

  // The clock divider computation is done by using unsigned integer.
  // The goal is to force integer-only division by clearing the fractional part.
  // Note: The divider field of the USART->CLKDIV register is of the following form:
  // xxxxxxxxxxxxxxx.yyyyy where x is the 15 bits integral part of the divider
  // and y is the 5 bits fractional part.

  // For USART Sync, the clock divider is rounded up to the nearest integer.
  // The clock divider is calculated as follows:
  // CLKDIV = ceil(ref_freq / (2 * baudrate)) - 1
  // where ceil(N / D) - 1 = (N - 1) / D according to integer division of positive numbers.

  clk_div  = (ref_freq - 1) / (2 * baudrate);
  clk_div = clk_div << 8;

  // Verify that resulting clock divider is within limits.
  EFM_ASSERT(clk_div <= _USART_CLKDIV_DIV_MASK);

  // If the EFM_ASSERT is not enabled, make sure not to write to reserved bits.
  clk_div = (clk_div & _USART_CLKDIV_DIV_MASK) >> _USART_CLKDIV_DIV_SHIFT;

  return clk_div;
}

/***************************************************************************//**
 * Initialize USART for asynchronous IrDA mode.
 *
 * @note Not all USART instances support IrDA. See the data sheet for your device.
 ******************************************************************************/
void sl_hal_usart_init_irda(USART_TypeDef *usart,
                            const sl_hal_usart_irda_init_t *init)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  // Initialize USART as an async device.
  sl_hal_usart_init_async(usart, &(init->async));

  // Set IrDA modulation to RZI (return-to-zero-inverted).
  usart->CTRL |= USART_CTRL_TXINV;

  // Invert the Rx signal before the demodulator if enabled.
  usart->CTRL |= ((uint32_t)init->ir_rx_inv << _USART_CTRL_RXINV_SHIFT);

  // Configure IrDA.
  usart->IRCTRL = ((uint32_t)init->ir_pulse_width << _USART_IRCTRL_IRPW_SHIFT)
                  | ((init->ir_filter ? 1UL : 0UL) << _USART_IRCTRL_IRFILT_SHIFT);

  // Enable IrDA.
  usart->IRCTRL_SET = USART_IRCTRL_IREN;
}

/***************************************************************************//**
 * Initialize USART for I2S mode.
 *
 * @note This function does not apply to all USART's. See the chip Reference Manual.
 ******************************************************************************/
void sl_hal_usart_init_i2s(USART_TypeDef *usart,
                           const sl_hal_usart_i2s_init_t *init)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  // Disable TX before init I2S
  sl_hal_usart_disable_tx(usart);

  // Initialize USART as a sync device.
  sl_hal_usart_init_sync(usart, &init->sync);

  // Configure and enable I2CCTRL register according to the selected mode.
  usart->I2SCTRL = ((uint32_t)init->format << _USART_I2SCTRL_FORMAT_SHIFT)
                   | ((uint32_t)init->justify << _USART_I2SCTRL_JUSTIFY_SHIFT)
                   | (init->delay    ? USART_I2SCTRL_DELAY     : 0)
                   | (init->dma_split ? USART_I2SCTRL_DMASPLIT : 0)
                   | (init->mono     ? USART_I2SCTRL_MONO      : 0)
                   | USART_I2SCTRL_EN;
}

/***************************************************************************//**
 * Reset USART/UART to the same state that it was in after a hardware reset.
 ******************************************************************************/
void sl_hal_usart_reset(USART_TypeDef *usart)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  usart->EN_SET = USART_EN_EN;

  // Make sure disabled first, before resetting other registers.
  usart->CMD = USART_CMD_RXDIS | USART_CMD_TXDIS | USART_CMD_MASTERDIS
               | USART_CMD_RXBLOCKDIS | USART_CMD_TXTRIDIS | USART_CMD_CLEARTX
               | USART_CMD_CLEARRX;

  usart->CTRL      = _USART_CTRL_RESETVALUE;
  usart->CTRLX     = _USART_CTRLX_RESETVALUE;
  usart->FRAME     = _USART_FRAME_RESETVALUE;
  usart->TRIGCTRL  = _USART_TRIGCTRL_RESETVALUE;
  usart->CLKDIV    = _USART_CLKDIV_RESETVALUE;
  usart->IEN       = _USART_IEN_RESETVALUE;
  usart->IF_CLR    = _USART_IF_MASK;
  usart->TIMING    = _USART_TIMING_RESETVALUE;

  usart->IRCTRL  = _USART_IRCTRL_RESETVALUE;
  usart->I2SCTRL = _USART_I2SCTRL_RESETVALUE;

  usart->EN_CLR = USART_EN_EN;
}

/***************************************************************************//**
 * Enable automatic enabling of reception using the PRS as a trigger.
 ******************************************************************************/
void sl_hal_usart_enable_rx_prs_trigger(USART_TypeDef *usart,
                                        uint8_t channel)
{
  // Make sure that the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  // The peripheral must be enabled to configure the PRS trigger.
  EFM_ASSERT(usart->EN == USART_EN_EN);

  if (usart == USART0) {
    sl_hal_prs_connect_channel_consumer(channel,
                                        SL_HAL_PRS_TYPE_ASYNC,
                                        SL_HAL_PRS_CONSUMER_USART0_TRIGGER);
  }
#if defined(USART1)
  else if (usart == USART1) {
    sl_hal_prs_connect_channel_consumer(channel,
                                        SL_HAL_PRS_TYPE_ASYNC,
                                        SL_HAL_PRS_CONSUMER_USART1_TRIGGER);
  }
#endif
#if defined(USART2)
  else if (usart == USART2) {
    sl_hal_prs_connect_channel_consumer(channel,
                                        SL_HAL_PRS_TYPE_ASYNC,
                                        SL_HAL_PRS_CONSUMER_USART2_TRIGGER);
  }
#endif
  else {
    EFM_ASSERT(false);
  }
  usart->TRIGCTRL_SET = USART_TRIGCTRL_RXTEN;
}

/***************************************************************************//**
 * Disable automatic enabling of reception using the PRS as a trigger.
 ******************************************************************************/
void sl_hal_usart_disable_rx_prs_trigger(USART_TypeDef *usart)
{
  // Make sure that the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  // The peripheral must be enabled to configure the PRS trigger.
  EFM_ASSERT(usart->EN == USART_EN_EN);

  usart->TRIGCTRL_CLR = USART_TRIGCTRL_RXTEN;
}

/***************************************************************************//**
 * Enable automatic enabling of transmission using the PRS as a trigger.
 ******************************************************************************/
void sl_hal_usart_enable_tx_prs_trigger(USART_TypeDef *usart,
                                        uint8_t channel)
{
  // Make sure that the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  // The peripheral must be enabled to configure the PRS trigger.
  EFM_ASSERT(usart->EN == USART_EN_EN);

  if (usart == USART0) {
    sl_hal_prs_connect_channel_consumer(channel,
                                        SL_HAL_PRS_TYPE_ASYNC,
                                        SL_HAL_PRS_CONSUMER_USART0_TRIGGER);
  }
#if defined(USART1)
  else if (usart == USART1) {
    sl_hal_prs_connect_channel_consumer(channel,
                                        SL_HAL_PRS_TYPE_ASYNC,
                                        SL_HAL_PRS_CONSUMER_USART1_TRIGGER);
  }
#endif
#if defined(USART2)
  else if (usart == USART2) {
    sl_hal_prs_connect_channel_consumer(channel,
                                        SL_HAL_PRS_TYPE_ASYNC,
                                        SL_HAL_PRS_CONSUMER_USART2_TRIGGER);
  }
#endif
  else {
    EFM_ASSERT(false);
  }
  usart->TRIGCTRL_SET = USART_TRIGCTRL_TXTEN;
}

/***************************************************************************//**
 * Disable automatic enabling of transmission using the PRS as a trigger.
 ******************************************************************************/
void sl_hal_usart_disable_tx_prs_trigger(USART_TypeDef *usart)
{
  // Make sure that the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_USART_REF_VALID(usart));

  // The peripheral must be enabled to configure the PRS trigger.
  EFM_ASSERT(usart->EN == USART_EN_EN);

  usart->TRIGCTRL_CLR = USART_TRIGCTRL_TXTEN;
}

/***************************************************************************//**
 * Configure a PRS channel as USART Rx input
 ******************************************************************************/
static void usart_prs_input(USART_TypeDef *usart,
                            uint8_t channel)
{
  if (usart == USART0) {
    sl_hal_prs_connect_channel_consumer(channel,
                                        SL_HAL_PRS_TYPE_ASYNC,
                                        SL_HAL_PRS_CONSUMER_USART0_RX);
  }
#if defined(USART1)
  else if (usart == USART1) {
    sl_hal_prs_connect_channel_consumer(channel,
                                        SL_HAL_PRS_TYPE_ASYNC,
                                        SL_HAL_PRS_CONSUMER_USART1_RX);
  }
#endif
#if defined(USART2)
  else if (usart == USART2) {
    sl_hal_prs_connect_channel_consumer(channel,
                                        SL_HAL_PRS_TYPE_ASYNC,
                                        SL_HAL_PRS_CONSUMER_USART2_RX);
  }
#endif
  else {
    EFM_ASSERT(false);
  }
  usart->CTRLX_SET = USART_CTRLX_RXPRSEN;
}

/***************************************************************************//**
 *  Map oversampling enum value to ovesampling real value.
 *
 * @param[in] ref_freq Peripheral clock frequency.
 * @param[in] baudrate Transmission speed.
 * @param[in] ovs Oversampling rate.
 *
 * @return Real ovesampling value.
 ******************************************************************************/
static uint8_t usart_map_oversampling(uint32_t ref_freq,
                                      uint32_t baudrate,
                                      sl_hal_usart_ovs_t ovs)
{
  uint8_t oversample = 0;
  // Map oversampling.
  switch (ovs) {
    case SL_HAL_USART_OVS_16:
      EFM_ASSERT(baudrate <= (ref_freq / 16));
      oversample = 16;
      break;

    case SL_HAL_USART_OVS_8:
      EFM_ASSERT(baudrate <= (ref_freq / 8));
      oversample = 8;
      break;

    case SL_HAL_USART_OVS_6:
      EFM_ASSERT(baudrate <= (ref_freq / 6));
      oversample = 6;
      break;

    case SL_HAL_USART_OVS_4:
      EFM_ASSERT(baudrate <= (ref_freq / 4));
      oversample = 4;
      break;

    default:
      // Invalid input
      EFM_ASSERT(0);
      break;
  }

  return oversample;
}

#endif // defined(USART_PRESENT)
