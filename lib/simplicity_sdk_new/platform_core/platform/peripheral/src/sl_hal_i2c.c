/***************************************************************************//**
 * @file
 * @brief Inter-Integrated Circuit (I2C) peripheral API
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

#include "sl_hal_i2c.h"
#if defined(I2C_PRESENT)

#include <stddef.h>

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
// Maximum I2C transmission rate constant.
#define SL_HAL_I2C_CR_MAX       8

/*******************************************************************************
 *****************************   LOCAL VARIABLES   *****************************
 ******************************************************************************/
// Lookup table for Nlow + Nhigh setting defined by CLHR. Set the undefined,
// index (0x3) to reflect a default setting just in case.
static const uint8_t prescaled_clk_cycles_sum[] = { 4 + 4, 6 + 3, 11 + 6, 4 + 4 };

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/
#if defined(_SILICON_LABS_32B_SERIES_2)
static void i2c_flush_rx(I2C_TypeDef *i2c);
#endif //_SILICON_LABS_32B_SERIES_2
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
extern __INLINE void sl_hal_i2c_wait_ready(I2C_TypeDef *i2c);
extern __INLINE void sl_hal_i2c_wait_sync(I2C_TypeDef *i2c);
extern __INLINE void sl_hal_i2c_enable(I2C_TypeDef *i2c);
extern __INLINE void sl_hal_i2c_disable(I2C_TypeDef *i2c);
extern __INLINE sl_status_t sl_hal_i2c_send_ack(I2C_TypeDef *i2c);
extern __INLINE void sl_hal_i2c_auto_ack(I2C_TypeDef *p_instance,
                                         uint8_t value);
extern __INLINE sl_status_t sl_hal_i2c_send_nack(I2C_TypeDef *i2c);
extern __INLINE uint32_t sl_hal_i2c_get_status(I2C_TypeDef *i2c);
extern __INLINE void sl_hal_i2c_enable_interrupts(I2C_TypeDef *i2c,
                                                  uint32_t flags);
extern __INLINE void sl_hal_i2c_disable_interrupts(I2C_TypeDef *i2c,
                                                   uint32_t flags);
extern __INLINE void sl_hal_i2c_set_interrupts(I2C_TypeDef *i2c,
                                               uint32_t flags);
extern __INLINE void sl_hal_i2c_clear_interrupts(I2C_TypeDef *i2c,
                                                 uint32_t flags);
extern __INLINE uint32_t sl_hal_i2c_get_pending_interrupts(I2C_TypeDef *i2c);
extern __INLINE uint32_t sl_hal_i2c_get_enabled_interrupts(I2C_TypeDef *i2c);
extern __INLINE uint32_t sl_hal_i2c_get_enabled_pending_interrupts(I2C_TypeDef *i2c);
extern __INLINE void sl_hal_i2c_set_follower_address(I2C_TypeDef *i2c,
                                                     uint16_t addr,
                                                     bool is_10bit_addr);
extern __INLINE uint8_t sl_hal_i2c_get_follower_address(I2C_TypeDef *i2c);
extern __INLINE void sl_hal_i2c_set_follower_mask_address(I2C_TypeDef *i2c,
                                                          uint8_t mask);
extern __INLINE uint8_t sl_hal_i2c_get_follower_mask_address(I2C_TypeDef *i2c);

#if defined(_I2C_CTRL_TXFIL_SHIFT)
extern __INLINE void sl_hal_i2c_set_tx_fifo_threshold(I2C_TypeDef *i2c,
                                                      sl_i2c_fifo_threshold_t threshold_value);
extern __INLINE sl_i2c_fifo_threshold_t sl_hal_i2c_get_tx_fifo_threshold(const I2C_TypeDef *i2c);
#endif //_I2C_CTRL_TXFIL_SHIFT

#if defined(_I2C_CTRL_RXFIL_SHIFT)
extern __INLINE void sl_hal_i2c_set_rx_fifo_threshold(I2C_TypeDef *i2c,
                                                      sl_i2c_fifo_threshold_t threshold_value);
extern __INLINE sl_i2c_fifo_threshold_t sl_hal_i2c_get_rx_fifo_threshold(const I2C_TypeDef *i2c);
#endif //_I2C_CTRL_RXFIL_SHIFT

/***************************************************************************//**
 * Initializes the I2C Peripheral.
 ******************************************************************************/
void sl_hal_i2c_init(I2C_TypeDef *i2c,
                     sl_i2c_operating_mode_t  mode)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  // Reset the i2c peripheral.
  sl_hal_i2c_reset(i2c);

  // Wait for synchronization to complete.
  sl_hal_i2c_wait_sync(i2c);

  // Leader or follower mode configuration.
  i2c->CTRL_SET = (mode << _I2C_CTRL_SLAVE_SHIFT);
}

/***************************************************************************//**
 * Transmits 1 byte.
 ******************************************************************************/
void sl_hal_i2c_tx(I2C_TypeDef *i2c,
                   uint8_t data)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

#if defined(_I2C_STATUS_TXFL_MASK)
  // Check TX FIFO has space for at least one more frame.
  while (!(i2c->STATUS & _I2C_STATUS_TXFL_MASK)) {
  }
#endif //_I2C_STATUS_TXFL_MASK

  i2c->TXDATA = (uint32_t)data;
}

/***************************************************************************//**
 * Receives 1 byte.
 ******************************************************************************/
uint8_t sl_hal_i2c_rx(I2C_TypeDef *i2c)
{
  uint8_t rx_data = 0;
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  // Check for data to be available in receive buffer.
  while (!(i2c->STATUS & _I2C_STATUS_RXDATAV_MASK)) {
  }

  // Read data.
  rx_data = (uint8_t)i2c->RXDATA;

#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)  \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_3)
  // Errata I2C_E303 , I2C Fails to indicate new incoming data.
  uint32_t status = sl_hal_i2c_get_status(i2c);

  // Look for invalid RXDATAV = 0 and RXFULL = 1 condition.
  if (((status & I2C_IF_RXDATAV) == 0) & ((status & I2C_IF_RXFULL) != 0)) {
    // Performing a dummy read of the RXFIFO (I2C_RXDATA).
    // This restores the expected RXDATAV = 1 and RXFULL = 0 condition.
    (void)i2c->RXDATA;

    // The dummy read will also set the RXUFIF flag bit, which should be ignored and cleared.
    sl_hal_i2c_clear_interrupts(i2c, I2C_IF_RXUF);
  }
#endif

  return rx_data;
}

/***************************************************************************//**
 *  Set I2C clock frequency.
 ******************************************************************************/
void sl_hal_i2c_set_clock_frequency(I2C_TypeDef *i2c,
                                    uint32_t i2c_clk,
                                    uint32_t freq_scl,
                                    sl_i2c_clock_hlr_t clhr)
{
  uint32_t prescaled_clk_cycles;
  uint32_t min_freq = 0;
  uint32_t denominator;
  int32_t clkdiv;

  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  // Avoid dividing by 0.
  EFM_ASSERT(freq_scl);
  EFM_ASSERT(i2c_clk);

  // Wait for synchronization to complete.
  sl_hal_i2c_wait_sync(i2c);

  // Set clhr mode.
  i2c->CTRL_SET = clhr << _I2C_CTRL_CLHR_SHIFT;

  // Check the minumum HF peripheral clock.
  if (i2c->CTRL & I2C_CTRL_SLAVE) {
    switch (clhr) {
      case SL_I2C_CLK_HLR_STANDARD:
        min_freq = 2000000;
        break;
      case SL_I2C_CLK_HLR_ASYMMETRIC:
        min_freq = 5000000;
        break;
      case SL_I2C_CLK_HLR_FAST:
        min_freq = 14000000;
        break;
      default:
        EFM_ASSERT(false);
        break;
    }
  } else {
    switch (clhr) {
      case SL_I2C_CLK_HLR_STANDARD:
        min_freq = 2000000;
        break;
      case SL_I2C_CLK_HLR_ASYMMETRIC:
        min_freq = 9000000;
        break;
      case SL_I2C_CLK_HLR_FAST:
        min_freq = 20000000;
        break;
      default:
        EFM_ASSERT(false);
        break;
    }
  }

  // Reference frequency must be larger-than min frequency.
  EFM_ASSERT(i2c_clk > min_freq);

  // I2C SCL frequency is given by: freq_scl = i2c_clk/((Nlow + Nhigh) * (DIV + 1) + SL_HAL_I2C_CR_MAX).
  // Therefore, DIV = ((i2c_clk - (SL_HAL_I2C_CR_MAX * freq_scl))/((Nlow + Nhigh) * freq_scl)) - 1.
  // For more details, see the reference manual I2C Clock Generation chapter.

  // Calculation: n = Nlow + Nhigh.
  prescaled_clk_cycles = (uint32_t)prescaled_clk_cycles_sum[clhr];
  denominator = prescaled_clk_cycles * freq_scl;

  // Ensure denominator is never zero.
  EFM_ASSERT(denominator);

  // Perform integer division so that clkdiv is rounded up.
  clkdiv = (int32_t)(((i2c_clk - (SL_HAL_I2C_CR_MAX * freq_scl) + denominator - 1)
                      / denominator) - 1);

  // Verify that the resulting clock divider is within limits.
  EFM_ASSERT(clkdiv >= 0);
  EFM_ASSERT((uint32_t)clkdiv <= _I2C_CLKDIV_DIV_MASK);

  // The clock divisor must be at least 1 in follower mode according to the reference.
  // Manual (in which case there is normally no need to set the bus frequency).
  if ((i2c->CTRL & I2C_CTRL_SLAVE) && (clkdiv == 0)) {
    clkdiv = 1;
  }

  // Wait for synchronization to complete.
  sl_hal_i2c_wait_sync(i2c);

  i2c->CLKDIV = (clkdiv & _I2C_CLKDIV_DIV_MASK);
}
/***************************************************************************//**
 * Get I2C clock frequency.
 ******************************************************************************/
uint32_t sl_hal_i2c_get_clock_frequency(I2C_TypeDef *i2c,
                                        uint32_t i2c_clk)
{
  uint32_t prescaled_clk_cycles;

  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  // Make sure module is enabled.
  EFM_ASSERT(i2c->EN & _I2C_EN_EN_MASK);

  // Wait for synchronization to complete.
  sl_hal_i2c_wait_sync(i2c);

  // Calculating n , n = Nlow + Nhigh.
  prescaled_clk_cycles = (uint32_t)prescaled_clk_cycles_sum[(i2c->CTRL & _I2C_CTRL_CLHR_MASK)
                                                            >> _I2C_CTRL_CLHR_SHIFT];

  return i2c_clk / ((prescaled_clk_cycles * (i2c->CLKDIV + 1)) + SL_HAL_I2C_CR_MAX);
}
/***************************************************************************//**
 * Configure the I2C to its reset state.
 ******************************************************************************/
void sl_hal_i2c_reset(I2C_TypeDef *i2c)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

#if defined(_SILICON_LABS_32B_SERIES_2)
  // Cancel ongoing operations and clear TX buffer
  i2c->CMD       = I2C_CMD_CLEARPC | I2C_CMD_CLEARTX | I2C_CMD_ABORT;
  i2c->CTRL      = _I2C_CTRL_RESETVALUE;
  i2c->CLKDIV    = _I2C_CLKDIV_RESETVALUE;
  i2c->SADDR     = _I2C_SADDR_RESETVALUE;
  i2c->SADDRMASK = _I2C_SADDRMASK_RESETVALUE;

#else
  // Cancel ongoing operations and clear TX and RX FIFO's.
  i2c->CMD  = I2C_CMD_CLEARPC | I2C_CMD_CLEARTX | I2C_CMD_ABORT | I2C_CMD_CLEARRX;

  // Reset the module.
  i2c->SWRST = I2C_SWRST_SWRST;

  // Wait until reset.
  while (i2c->SWRST & _I2C_SWRST_RESETTING_MASK) {
  }
#endif //_SILICON_LABS_32B_SERIES_2

  // Disable the I2C Peripheral.
  sl_hal_i2c_disable(i2c);

  // Disable I2C Interrupts.
  sl_hal_i2c_disable_interrupts(i2c, _I2C_IEN_MASK);

#if defined(_SILICON_LABS_32B_SERIES_2)
  // Flush the receive data buffer.
  i2c_flush_rx(i2c);
#endif //_SILICON_LABS_32B_SERIES_2

  // Clear the pending I2C Interrupts.
  sl_hal_i2c_clear_interrupts(i2c, _I2C_IF_MASK);
}

/***************************************************************************//**
 * Sends the Start Command.
 ******************************************************************************/
sl_status_t sl_hal_i2c_start_cmd(I2C_TypeDef *i2c)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  sl_status_t status = SL_STATUS_OK;

  if ((i2c->EN & _I2C_EN_EN_MASK) != I2C_EN_EN) {
    status = SL_STATUS_NOT_INITIALIZED;
  } else {
    // Intiate Start Command.
    i2c->CMD = I2C_CMD_START;

    // Wait for synchronization to complete.
    sl_hal_i2c_wait_sync(i2c);
  }

  return status;
}

/***************************************************************************//**
 * Sends the Stop Command.
 ******************************************************************************/
sl_status_t sl_hal_i2c_stop_cmd(I2C_TypeDef *i2c)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  sl_status_t status = SL_STATUS_OK;

  if ((i2c->EN & _I2C_EN_EN_MASK) != I2C_EN_EN) {
    status = SL_STATUS_NOT_INITIALIZED;
  } else {
    // Intiate Stop Command.
    i2c->CMD = I2C_CMD_STOP;

    // Wait for synchronization to complete.
    sl_hal_i2c_wait_sync(i2c);
  }

  return status;
}

/***************************************************************************//**
 * Clears the Tx , RX Buffers , Shift registers and pending commands.
 ******************************************************************************/
void sl_hal_i2c_flush_buffers(I2C_TypeDef *i2c)
{
  // Abort the current transmission making the bus go idle.
  if (i2c->STATE & I2C_STATE_BUSY) {
    i2c->CMD = I2C_CMD_ABORT;

    // Wait for synchronization to complete.
    sl_hal_i2c_wait_sync(i2c);
  }

#if defined(_SILICON_LABS_32B_SERIES_2)
  // Clear pending commands,clear TX Buffer.
  i2c->CMD = (I2C_CMD_CLEARPC | I2C_CMD_CLEARTX);

  // Flush the receive data buffer.
  i2c_flush_rx(i2c);
#else
  // Clear pending commands,clear TX Buffer & Shift register.
  i2c->CMD = (I2C_CMD_CLEARPC | I2C_CMD_CLEARTX | I2C_CMD_CLEARRX);
#endif //_SILICON_LABS_32B_SERIES_2

  // Wait for synchronization to complete.
  sl_hal_i2c_wait_sync(i2c);
}

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/***************************************************************************//**
 * Empty the received data buffer.
 ******************************************************************************/
#if defined(_SILICON_LABS_32B_SERIES_2)
static void i2c_flush_rx(I2C_TypeDef *i2c)
{
  // Empty the received data buffer.
  while (i2c->STATUS & I2C_STATUS_RXDATAV) {
    (void)i2c->RXDATA;
  }

  // Clear data valid interrupt.
  sl_hal_i2c_clear_interrupts(i2c, I2C_IF_RXDATAV);
}
#endif //_SILICON_LABS_32B_SERIES_2

#endif /* defined(I2C_PRESENT) */
