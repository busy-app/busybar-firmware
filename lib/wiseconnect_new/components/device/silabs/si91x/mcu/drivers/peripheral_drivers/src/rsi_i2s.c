/******************************************************************************
* @file  rsi_i2s.c
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

// Include files

#include "rsi_ccp_user_config.h"
#if !defined(A11_ROM) || !defined(I2S_ROMDRIVER_PRESENT)
#include "rsi_rom_clks.h"
#include "rsi_rom_ulpss_clk.h"
#include "rsi_rom_egpio.h"
#include "rsi_rom_udma_wrapper.h"
#include "rsi_i2s.h"
#include "SAI.h"

#define I2S_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MIN \
  46 // Minimum pin number for specific range of HP pins to act as ULP pins
#define I2S_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MAX \
  49 // Maximum pin number for specific range of HP pins to act as ULP pins

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
static const uint32_t i2s_pll_freq[] = { 256000,  512000,   768000,   1024000,  1411200, 2048000, 2822400,
                                         3072000, 4096000,  4233600,  4608000,  5644800, 6144000, 8467200,
                                         9216000, 11289600, 12288000, 18432000, 24576000 };

#define I2S_PLL_FREQ_COUNT (sizeof(i2s_pll_freq) / sizeof(i2s_pll_freq[0]))
// Fixed post-divider of 2
#define I2S_FIXED_POST_DIV 2U

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static uint8_t I2S_get_freq_div_factor(uint32_t req_freq_hz, uint32_t *clk_source_freq_hz);

/**
 * @brief Calculate the I2S divider for a requested output frequency.
 *
 * Chooses a PLL source that is an integer multiple of @p req_freq_hz,
 * then returns divider = (source / req_freq_hz) / I2S_FIXED_POST_DIV.
 *
 * Search order is from highest PLL source to lowest, which is typically
 * preferred for clock quality. The function only returns a divider that
 * fits into 8 bits. On failure, returns 0 and sets *clk_source_hz = 0.
 *
 * @param[in]  req_freq_hz    Requested I2S clock frequency in Hz
 *                            (e.g., 352800 for 0.3528 MHz).
 * @param[out] clk_source_hz  Selected PLL source frequency in Hz.
 *
 * @return Divider (8-bit) or 0 if no exact, representable solution exists.
 */
static uint8_t I2S_get_freq_div_factor(uint32_t req_freq_hz, uint32_t *clk_source_hz)
{
  if (clk_source_hz == NULL || req_freq_hz == 0U) {
    if (clk_source_hz) {
      *clk_source_hz = 0U;
    }
    return 0U;
  }

  /* Search from highest source down to lowest to prefer cleaner clocking */
  for (int i = (int)I2S_PLL_FREQ_COUNT - 1; i >= 0; --i) {
    const uint32_t src = i2s_pll_freq[i];

    /* Exact multiple only (change to nearest-fit if you later allow tolerance) */
    if ((src % req_freq_hz) != 0U) {
      continue;
    }

    const uint32_t pre_div = src / req_freq_hz;
    uint32_t div;

    if (pre_div == 1U) {
      /* PLL source matches requested frequency exactly; set divider = 0 to bypass and avoid post-divider */
      div = 0U;
    } else {
      /* If pre_div is odd, the fixed post-divider cannot produce the requested frequency exactly */
      if (pre_div & 1U) {
        continue;
      }
      div = pre_div / I2S_FIXED_POST_DIV;
    }

    if (div > I2S_MAX_CLK_DIVISION_FACTOR) {
      /* Not representable in hardware (6 bits)—try next source */
      continue;
    }

    *clk_source_hz = src;
    return (uint8_t)div;
  }

  /* No valid exact solution found */
  *clk_source_hz = 0U;
  return 0U;
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/** @addtogroup SOC17
* @{
*/
/*==============================================*/
/**
 *  @fn          void RSI_I2S_InterruptEnableDisable(I2S_RESOURCES *i2s, boolean_t chnlNum, uint32_t maskFlag, boolean_t mask)
 *  @brief       This API is used to enable or disable i2s interrupt.
 *  @param[in]   i2s      : Pointer to I2S resources
 *  @param[in]   chnlNum  : Channel Number
 *  @param[in]   maskFlag : Mask Flag value.
 *  @param[in]   mask     : This parameter used to disable or enable interrupt mask. 
 *  @return      None
 */
void RSI_I2S_InterruptEnableDisable(I2S_RESOURCES *i2s, boolean_t chnlNum, uint32_t maskFlag, boolean_t mask)
{
  if (mask) {
    i2s->reg->CHANNEL_CONFIG[chnlNum].I2S_IMR |= maskFlag;
  } else {
    i2s->reg->CHANNEL_CONFIG[chnlNum].I2S_IMR &= ~maskFlag;
  }
}
/*==============================================*/
/**
 *  @fn          int32_t I2S_Control(uint32_t control,
 *               uint32_t arg1,
 *               uint32_t arg2,
 *               I2S_RESOURCES *i2s,
 *               UDMA_RESOURCES *udma,
 *               RSI_UDMA_HANDLE_T udmaHandle)
 *  @brief       This APIs used to controls the I2S interface and executes various operations 	 
 *  @param[in]   control   : The parameter control specifies the operation
 *  @param[in]   arg1      : The parameter arg1 provides, depending on the operation,
 *                           additional information or sets values
 *  @param[in]   arg2      : The parameter arg2 provides, depending on the operation and/or arg1, 
 *                           additional information or sets values.
 *  @param[in]   i2s       : Pointer to I2S resources
 *  @param[in]   udma      : Pointer to UDMA resources
 *  @param[in]   udmaHandle: Pointer to UDMA handler
 *  @return      \ref execution_status
 */
int32_t I2S_Control(uint32_t control,
                    uint32_t arg1,
                    uint32_t arg2,
                    I2S_RESOURCES *i2s,
                    UDMA_RESOURCES *udma,
                    RSI_UDMA_HANDLE_T udmaHandle)
{
  uint32_t master = 0, data_bits = 0;
  uint32_t bit_freq = 0, val = 0;
  bool mic_transfer = false;

  if ((i2s->flags & I2S_FLAG_POWERED) == 0U) {
    // I2S not powered
    return ARM_DRIVER_ERROR;
  }

  master    = 0U;
  data_bits = 0U;

  switch (control & ARM_SAI_CONTROL_Msk) {
    case ARM_SAI_CONFIGURE_TX:
      break;
    case ARM_SAI_CONFIGURE_RX:
      break;
    case ARM_SAI_CONFIGURE_MIC:
      mic_transfer = true;
      break;
    case ARM_SAI_CONTROL_TX:
      // I2S Enable
      i2s->reg->I2S_IER_b.IEN = 1;
      if ((i2s->info->status.tx_busy == 0U) || (i2s->dma_tx)) {
        // Set TX level to 0
        val = i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_TXFCR_b.TXCHET;
        val |= 0x8; // max FIFO level for TX overflow
        i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_TXFCR_b.TXCHET = (volatile unsigned int)(val & 0x0F);
        if (i2s->info->status.tx_busy == 0U) {
          // Ready to detect TX underflow
          i2s->info->tx.num = 0U;
        }
      }

      // Mute
      if ((arg1 & 2U) != 0U) {
        if (i2s->xfer_chnl) {
          i2s->reg->CHANNEL_CONFIG[1].I2S_TER_b.TXCHEN = 0x1;
        } else {
          i2s->reg->CHANNEL_CONFIG[0].I2S_TER_b.TXCHEN = 0x1;
        }
      } else {
      }
      return ARM_DRIVER_OK;

    case ARM_SAI_CONTROL_RX:
      i2s->reg->I2S_IER_b.IEN   = 1;
      i2s->reg->I2S_IRER_b.RXEN = 0x0;

      // Set Stop
      val = i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RFCR_b.RXCHDT;
      if (i2s->info->status.rx_busy == 0U) {
        // Set FIFO level to full, to detect RX overflow
        val |= 0x8;
        i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RFCR_b.RXCHDT = (volatile unsigned int)(val & 0x0F);

        // Ready to detect RX overflow
        i2s->info->rx.num = 0U;

        // Enable I2S receive interrupt
        //RSI_I2S_InterruptEnableDisable( i2s,i2s->xfer_chnl, F_RXDAM ,INTR_UNMASK );
      } else {
        if (i2s->dma_rx != NULL) {
          // Set user RX FIFO level
          val |= (uint32_t)(i2s->rx_fifo_level);

          // Enable I2S receive interrupt
          //RSI_I2S_InterruptEnableDisable( i2s,i2s->xfer_chnl, F_RXDAM ,INTR_UNMASK );
        }
      }
      return ARM_DRIVER_OK;

    case ARM_SAI_MASK_SLOTS_TX:
      return ARM_DRIVER_ERROR;

    case ARM_SAI_MASK_SLOTS_RX:
      return ARM_DRIVER_ERROR;

    case ARM_SAI_ABORT_SEND:
      // Disable TX interrupt
      RSI_I2S_InterruptEnableDisable(i2s, i2s->xfer_chnl, F_TXFEM, INTR_MASK);

      if (i2s->dma_tx) {
        if (i2s->info->status.tx_busy != 0U) {
          if ((i2s->reg == I2S0) || (i2s->reg == I2S1)) {
            // Disable DMA channel
            UDMAx_ChannelDisable(i2s->dma_tx->channel, udma, udmaHandle);
          }
        }
      }

      // Reset TX channel FIFO
      i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_TFF_b.TXCHFR = 0x1;

      // Reset counters
      i2s->info->tx.cnt         = 0U;
      i2s->info->tx.num         = 0U;
      i2s->info->tx.residue_cnt = 0U;
      i2s->info->tx.residue_num = 0U;

      // Clear busy flag
      i2s->info->status.tx_busy = 0U;

      if (i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_TER_b.TXCHEN) {
        // Transmitter is enabled

        // Set FIFO level to full and enable TX interrupt, to detect RX overflow
        val = i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_TXFCR_b.TXCHET;
        val |= (8U);
        i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_TXFCR_b.TXCHET = (volatile unsigned int)(val & 0x0F);
        //RSI_I2S_InterruptEnableDisable( i2s,i2s->xfer_chnl, F_TXFOM ,INTR_UNMASK );
      }
      i2s->reg->I2S_CER_b.CLKEN = DISABLE;
      return ARM_DRIVER_OK;

    case ARM_SAI_ABORT_RECEIVE:
      // Disable RX interrupt
      RSI_I2S_InterruptEnableDisable(i2s, i2s->xfer_chnl, F_RXDAM, INTR_MASK);

      if (i2s->dma_rx) {
        if (i2s->info->status.rx_busy != 0U) {
          if ((i2s->reg == I2S0) || (i2s->reg == I2S1)) {
            // Disable DMA channel
            UDMAx_ChannelDisable(i2s->dma_rx->channel, udma, udmaHandle);
          }
        }
      }

      // Reset RX FIFO
      i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RFF_b.RXCHFR = 0x1;

      // Reset counters
      i2s->info->rx.cnt         = 0U;
      i2s->info->rx.num         = 0U;
      i2s->info->rx.residue_cnt = 0U;
      i2s->info->rx.residue_num = 0U;

      // Clear busy flag
      i2s->info->status.rx_busy = 0U;

      if (i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RER_b.RXCHEN) {
        // Receiver is enabled
        // Set FIFO level to full and enable RX interrupt, to detect RX overflow
        val = i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RFCR_b.RXCHDT;
        val |= (8U);
        i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RFCR_b.RXCHDT = (volatile unsigned int)(val & 0x0F);
        RSI_I2S_InterruptEnableDisable(i2s, i2s->xfer_chnl, F_RXFOM, INTR_UNMASK);
      }
      return ARM_DRIVER_OK;

    default:
      return ARM_DRIVER_ERROR;
  }

  // Protocol
  val = (control & ARM_SAI_PROTOCOL_Msk);
  if (val == ARM_SAI_PROTOCOL_PCM_SHORT || val == ARM_SAI_PROTOCOL_PCM_LONG) {
    i2s->protocol = PCM_PROTOCOL;
    if (i2s->reg == I2S0) {
      MISC_SOFT_SET_REG_2 |= PCM_ENA; //|PCM_FSYNCSTART ;
    }
    if (i2s->reg == I2S1) {
      RSI_ULPSS_UlpPeriClkEnable(ULPCLK, PCM_ENA /*|PCM_FSYNCSTART*/);
    }
  }
  // Mode
  switch (control & ARM_SAI_MODE_Msk) {
    case ARM_SAI_MODE_MASTER:
      master = 1U;
      if (i2s->reg == I2S0) {
        // sets master mode for I2S0(M4SS)
        MISC_CFG_MISC_CTRL1 |= (1 << 23);
      }
      if (i2s->reg == I2S1) {
        // sets master mode for I2S1(ULPSS)
        ULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_MASTER_SLAVE_MODE_b = 1;
      }
      break;
    case ARM_SAI_MODE_SLAVE:
      // Slave mode
      if (i2s->reg == I2S0) {
        // sets master mode for I2S0(M4SS)
        MISC_CFG_MISC_CTRL1 &= ~((uint32_t)(1 << 23));
      }
      if (i2s->reg == I2S1) {
/* sets master mode for I2S1(ULPSS)*/
#ifndef I2S1_LOOP_BACK
        ULPCLK->ULP_I2S_CLK_GEN_REG_b.ULP_I2S_MASTER_SLAVE_MODE_b = 0;
#endif
      }
      break;
    default:
      return ARM_DRIVER_ERROR;
  }

  // Synchronization
  switch (control & ARM_SAI_SYNCHRONIZATION_Msk) {
    case ARM_SAI_ASYNCHRONOUS:
      // Asynchronous mode requires SCK and WS pins
      break;
    case ARM_SAI_SYNCHRONOUS:
      if (master == 1U) {
        // Only Slave can be synchronous
        return ARM_SAI_ERROR_SYNCHRONIZATION;
      }
      // 4-pin mode: SCK and WS signals are shared between
      // I2S transmit and receive blocks
      break;
    default:
      return ARM_SAI_ERROR_SYNCHRONIZATION;
  }

  // Data size
  switch ((control & ARM_SAI_DATA_SIZE_Msk) >> ARM_SAI_DATA_SIZE_Pos) {
    case 16 - 1:
      data_bits = 16;
      if (i2s->protocol == PCM_PROTOCOL) {
        MISC_SOFT_SET_REG_2 |= (PCM_RES_16_BIT << 2);
        i2s->reg->I2S_CCR_b.SCLKG                               = PCM_RES_16_BIT;
        i2s->reg->I2S_CCR_b.WSS                                 = 0x0;
        i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_TCR_b.WLEN = PCM_RES_16_BIT;
        i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RCR_b.WLEN = PCM_RES_16_BIT;
      } else {
        i2s->reg->I2S_CCR_b.SCLKG = RES_16_BIT;
        if (mic_transfer) { // Mic ICS43434 in dev-kit uses this configuration
          i2s->reg->I2S_CCR_b.WSS = 0x02;
        } else {
          i2s->reg->I2S_CCR_b.WSS = 0x00;
        }
        i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_TCR_b.WLEN = RES_16_BIT;
        i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RCR_b.WLEN = RES_16_BIT;
      }
      break;
    case 24 - 1:
      data_bits = 24;
      if (i2s->protocol == PCM_PROTOCOL) {
        MISC_SOFT_SET_REG_2 |= (PCM_RES_24_BIT << 2);
        i2s->reg->I2S_CCR_b.SCLKG                               = PCM_RES_24_BIT;
        i2s->reg->I2S_CCR_b.WSS                                 = 0x1;
        i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_TCR_b.WLEN = PCM_RES_24_BIT;
        i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RCR_b.WLEN = PCM_RES_24_BIT;
      } else {
        i2s->reg->I2S_CCR_b.SCLKG                               = RES_24_BIT;
        i2s->reg->I2S_CCR_b.WSS                                 = 0x1;
        i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_TCR_b.WLEN = RES_24_BIT;
        i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RCR_b.WLEN = RES_24_BIT;
      }
      break;
    case 32 - 1:
      data_bits = 32;
      if (i2s->protocol == PCM_PROTOCOL) {
        MISC_SOFT_SET_REG_2 |= ((PCM_RES_32_BIT) << 2);
        i2s->reg->I2S_CCR_b.SCLKG                               = PCM_RES_32_BIT;
        i2s->reg->I2S_CCR_b.WSS                                 = 0x2;
        i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_TCR_b.WLEN = PCM_RES_32_BIT;
        i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RCR_b.WLEN = PCM_RES_32_BIT;
      } else {
        i2s->reg->I2S_CCR_b.SCLKG                               = RES_32_BIT;
        i2s->reg->I2S_CCR_b.WSS                                 = 0x2;
        i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_TCR_b.WLEN = RES_32_BIT;
        i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RCR_b.WLEN = RES_32_BIT;
      }

      break;
    default:
      return ARM_SAI_ERROR_DATA_SIZE;
  }

  // Companding
  val = control & ARM_SAI_COMPANDING_Msk;
  if (val != ARM_SAI_COMPANDING_NONE) {
    return ARM_SAI_ERROR_COMPANDING;
  }

  // Clock polarity
  val = control & ARM_SAI_CLOCK_POLARITY_Msk;
  if (val != ARM_SAI_CLOCK_POLARITY_0) {
    return ARM_SAI_ERROR_CLOCK_POLARITY;
  }

  // Master clock pin

  // Frame length
  val = ((arg1 & ARM_SAI_FRAME_LENGTH_Msk) >> ARM_SAI_FRAME_LENGTH_Pos);
  if ((val != 0U) && (val != (data_bits * 2))) {
    //return ARM_SAI_ERROR_FRAME_LENGHT;    // Commented this for supporting the different data resolutions
  }

  // Audio Frequency
  if (master == 1U) {
    // As per standard I2S bit clock frequency calculation
    bit_freq = (data_bits * 2 * arg2);
    if (i2s->reg == I2S0) {
      uint8_t div_fac = I2S_get_freq_div_factor(bit_freq, &bit_freq);
      if (bit_freq == 0) {
        // Error: Could not find valid PLL source/divider for requested audio frequency
        return ARM_SAI_ERROR_AUDIO_FREQ;
      }
      // div_fac == 0 is valid: means PLL bypass (source == requested frequency)
      RSI_CLK_SetI2sPllFreq(M4CLK, bit_freq, 40000000);
      RSI_CLK_I2sClkConfig(M4CLK, I2S_PLLCLK, div_fac);
      RSI_CLK_PeripheralClkEnable(M4CLK, I2SM_CLK, ENABLE_STATIC_CLK);
    }
    if (i2s->reg == I2S1) {
      if (i2s->clk->clk_src == ULP_I2S_REF_CLK) {
        val = system_clocks.ulpss_ref_clk / bit_freq;
        RSI_ULPSS_UlpI2sClkConfig(ULPCLK, ULP_I2S_REF_CLK, (uint16_t)val / 2);
      }
      if (i2s->clk->clk_src == ULP_I2S_ULP_MHZ_RC_CLK) {
        val = system_clocks.rc_mhz_clock / bit_freq;
        RSI_ULPSS_UlpI2sClkConfig(ULPCLK, ULP_I2S_ULP_MHZ_RC_CLK, (uint16_t)val / 2);
      }
      if (i2s->clk->clk_src == ULP_I2S_PLL_CLK) {
        val = system_clocks.i2s_pll_clock / bit_freq;
        RSI_ULPSS_UlpI2sClkConfig(ULPCLK, ULP_I2S_PLL_CLK, (uint16_t)val / 2);
      }
    }
  }

  // Save values to registers and globals
  if ((control & ARM_SAI_CONTROL_Msk) == ARM_SAI_CONFIGURE_TX) {
    i2s->info->tx.data_bits = (uint8_t)data_bits;
    i2s->info->tx.master    = (uint8_t)master;
  } else {
    i2s->info->rx.data_bits = (uint8_t)data_bits;
    i2s->info->rx.master    = (uint8_t)master;
  }

  i2s->flags |= I2S_FLAG_CONFIGURED;

  return ARM_DRIVER_OK;
}
/*==============================================*/
/**
 *  @fn          void I2S0_Chnl0_PinMux(I2S_RESOURCES *i2s)
 *  @brief       Configure pins to I2S0 channel0. 
 *  @param[in]   i2s      : Pointer to I2S resources
 *  @return      None
 */
void I2S0_Chnl0_PinMux(I2S_RESOURCES *i2s)
{
  // SCK
  if (i2s->io.sclk->pin > 63) {
    RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(i2s->io.sclk->pin - 64));
    RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(i2s->io.sclk->pin - 64), 6);
  }
  if (i2s->io.sclk->pad_sel != 0) {
    RSI_EGPIO_PadSelectionEnable(i2s->io.sclk->pad_sel);
  }
  if (i2s->io.sclk->pin >= 25 && i2s->io.sclk->pin <= 30) {
    RSI_EGPIO_HostPadsGpioModeEnable(i2s->io.sclk->pin);
  }
  RSI_EGPIO_PadReceiverEnable(i2s->io.sclk->pin);
  RSI_EGPIO_SetPinMux(EGPIO, i2s->io.sclk->port, i2s->io.sclk->pin, i2s->io.sclk->mode);

  // WSCLK
  if (i2s->io.wsclk->pin > 63) {
    RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(i2s->io.wsclk->pin - 64));
    RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(i2s->io.wsclk->pin - 64), 6);
  }
  if (i2s->io.wsclk->pad_sel != 0) {
    RSI_EGPIO_PadSelectionEnable(i2s->io.wsclk->pad_sel);
  }
  if (i2s->io.wsclk->pin >= 25 && i2s->io.wsclk->pin <= 30) {
    RSI_EGPIO_HostPadsGpioModeEnable(i2s->io.wsclk->pin);
  }
  RSI_EGPIO_PadReceiverEnable(i2s->io.wsclk->pin);
  RSI_EGPIO_SetPinMux(EGPIO, i2s->io.wsclk->port, i2s->io.wsclk->pin, i2s->io.wsclk->mode);

  //dout0
  if (i2s->io.dout0->pin > 63) {
    RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(i2s->io.dout0->pin - 64));
    RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(i2s->io.dout0->pin - 64), 6);
  }
  if (i2s->io.dout0->pad_sel != 0) {
    RSI_EGPIO_PadSelectionEnable(i2s->io.dout0->pad_sel);
  }
  if (i2s->io.dout0->pin >= 25 && i2s->io.dout0->pin <= 30) {
    RSI_EGPIO_HostPadsGpioModeEnable(i2s->io.dout0->pin);
  }
  RSI_EGPIO_PadReceiverEnable(i2s->io.dout0->pin);
  RSI_EGPIO_SetPinMux(EGPIO, i2s->io.dout0->port, i2s->io.dout0->pin, i2s->io.dout0->mode);

  //din0
  if (i2s->io.din0->pin > 63) {
    RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(i2s->io.din0->pin - 64));
    RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(i2s->io.din0->pin - 64), 6);
  }
  if (i2s->io.din0->pad_sel != 0) {
    RSI_EGPIO_PadSelectionEnable(i2s->io.din0->pad_sel);
  }
  if (i2s->io.din0->pin >= 25 && i2s->io.din0->pin <= 30) {
    RSI_EGPIO_HostPadsGpioModeEnable(i2s->io.din0->pin);
  }
  RSI_EGPIO_PadReceiverEnable(i2s->io.din0->pin);
  RSI_EGPIO_SetPinMux(EGPIO, i2s->io.din0->port, i2s->io.din0->pin, i2s->io.din0->mode);
}

/*==============================================*/
/**
 *  @fn          void I2S0_Chnl1_PinMux(I2S_RESOURCES *i2s)
 *  @brief       Configure pins to I2S0 channel1. 
 *  @param[in]   i2s      : Pointer to I2S resources
 *  @return      None
 */
void I2S0_Chnl1_PinMux(I2S_RESOURCES *i2s)
{
  // SCK
  if (i2s->io.sclk->pin > 63) {
    RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(i2s->io.sclk->pin - 64));
    RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(i2s->io.sclk->pin - 64), 6);
  }
  if (i2s->io.sclk->pad_sel != 0) {
    RSI_EGPIO_PadSelectionEnable(i2s->io.sclk->pad_sel);
  }
  if (i2s->io.sclk->pin >= 25 && i2s->io.sclk->pin <= 30) {
    RSI_EGPIO_HostPadsGpioModeEnable(i2s->io.sclk->pin);
  }
  RSI_EGPIO_PadReceiverEnable(i2s->io.sclk->pin);
  RSI_EGPIO_SetPinMux(EGPIO, i2s->io.sclk->port, i2s->io.sclk->pin, i2s->io.sclk->mode);

  // WSCLK
  if (i2s->io.wsclk->pin > 63) {
    RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(i2s->io.wsclk->pin - 64));
    RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(i2s->io.wsclk->pin - 64), 6);
  }
  if (i2s->io.wsclk->pad_sel != 0) {
    RSI_EGPIO_PadSelectionEnable(i2s->io.wsclk->pad_sel);
  }
  if (i2s->io.wsclk->pin >= 25 && i2s->io.wsclk->pin <= 30) {
    RSI_EGPIO_HostPadsGpioModeEnable(i2s->io.wsclk->pin);
  }
  RSI_EGPIO_PadReceiverEnable(i2s->io.wsclk->pin);
  RSI_EGPIO_SetPinMux(EGPIO, i2s->io.wsclk->port, i2s->io.wsclk->pin, i2s->io.wsclk->mode);

  //dout1
  if (i2s->io.dout1->pin > 63) {
    RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(i2s->io.dout1->pin - 64));
    RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(i2s->io.dout1->pin - 64), 6);
  }
  if (i2s->io.dout1->pad_sel != 0) {
    RSI_EGPIO_PadSelectionEnable(i2s->io.dout1->pad_sel);
  }
  if (i2s->io.dout1->pin >= 25 && i2s->io.dout1->pin <= 30) {
    RSI_EGPIO_HostPadsGpioModeEnable(i2s->io.dout1->pin);
  }
  RSI_EGPIO_PadReceiverEnable(i2s->io.dout1->pin);
  RSI_EGPIO_SetPinMux(EGPIO, i2s->io.dout1->port, i2s->io.dout1->pin, i2s->io.dout1->mode);

  //din1
  if (i2s->io.din1->pin > 63) {
    RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(i2s->io.din1->pin - 64));
    RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(i2s->io.din1->pin - 64), 6);
  }
  if (i2s->io.din1->pad_sel != 0) {
    RSI_EGPIO_PadSelectionEnable(i2s->io.din1->pad_sel);
  }
  if (i2s->io.din1->pin >= 25 && i2s->io.din1->pin <= 30) {
    RSI_EGPIO_HostPadsGpioModeEnable(i2s->io.din1->pin);
  }
  RSI_EGPIO_PadReceiverEnable(i2s->io.din1->pin);
  RSI_EGPIO_SetPinMux(EGPIO, i2s->io.din1->port, i2s->io.din1->pin, i2s->io.din1->mode);
}

/*==============================================*/
/**
 *  @fn          void I2S1_PinMux(I2S_RESOURCES *i2s)
 *  @brief       Configure pins to I2S1 Interface. 
 *  @param[in]   i2s      : Pointer to I2S resources
 *  @return      None
 */
void I2S1_PinMux(I2S_RESOURCES *i2s)
{
  // SCK
  //if the pin is ULP_GPIO then set the pin mode for direct ULP_GPIO.
  if (i2s->io.sclk->pin >= GPIO_MAX_PIN) {
    RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(i2s->io.sclk->pin - GPIO_MAX_PIN));
    RSI_EGPIO_SetPinMux(EGPIO1, i2s->io.sclk->port, (uint8_t)(i2s->io.sclk->pin - GPIO_MAX_PIN), i2s->io.sclk->mode);
  } else { // if the pin is SoC GPIO then set the HP GPIO mode to ULP_PERI_ON_SOC_PIN_MODE.
    RSI_EGPIO_PadReceiverEnable((uint8_t)(i2s->io.sclk->pin));
    RSI_EGPIO_SetPinMux(EGPIO, i2s->io.sclk->port, i2s->io.sclk->pin, EGPIO_PIN_MUX_MODE9);
    if (i2s->io.sclk->pad_sel != 0) {
      RSI_EGPIO_PadSelectionEnable(i2s->io.sclk->pad_sel);
    }

    if (i2s->io.sclk->pin >= I2S_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MIN
        && i2s->io.sclk->pin <= I2S_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MAX) {
      RSI_EGPIO_UlpSocGpioMode(ULPCLK, (i2s->io.sclk->pin - 38), i2s->io.sclk->mode);
    } else {
      RSI_EGPIO_UlpSocGpioMode(ULPCLK, (i2s->io.sclk->pin - 8), i2s->io.sclk->mode);
    }
  }
  // for WSCLK
  //if the pin is ULP_GPIO then set the pin mode for direct ULP_GPIO.
  if (i2s->io.wsclk->pin >= GPIO_MAX_PIN) {
    RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(i2s->io.wsclk->pin - GPIO_MAX_PIN));
    RSI_EGPIO_SetPinMux(EGPIO1, i2s->io.wsclk->port, (uint8_t)(i2s->io.wsclk->pin - GPIO_MAX_PIN), i2s->io.wsclk->mode);
  } else { // if the pin is SoC GPIO then set the HP GPIO mode to ULP_PERI_ON_SOC_PIN_MODE.
    RSI_EGPIO_PadReceiverEnable((uint8_t)(i2s->io.wsclk->pin));
    RSI_EGPIO_SetPinMux(EGPIO, i2s->io.wsclk->port, i2s->io.wsclk->pin, EGPIO_PIN_MUX_MODE9);
    if (i2s->io.wsclk->pad_sel != 0) {
      RSI_EGPIO_PadSelectionEnable(i2s->io.wsclk->pad_sel);
    }

    if (i2s->io.wsclk->pin >= I2S_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MIN
        && i2s->io.wsclk->pin <= I2S_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MAX) {
      RSI_EGPIO_UlpSocGpioMode(ULPCLK, (i2s->io.wsclk->pin - 38), i2s->io.wsclk->mode);
    } else {
      RSI_EGPIO_UlpSocGpioMode(ULPCLK, (i2s->io.wsclk->pin - 6), i2s->io.wsclk->mode);
    }
  }

  // TX pin
  //if the pin is ULP_GPIO then set the pin mode for direct ULP_GPIO.
  if (i2s->io.dout0->pin >= GPIO_MAX_PIN) {
    RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(i2s->io.dout0->pin - GPIO_MAX_PIN));
    RSI_EGPIO_SetPinMux(EGPIO1, i2s->io.dout0->port, (uint8_t)(i2s->io.dout0->pin - GPIO_MAX_PIN), i2s->io.dout0->mode);
  } else { // if the pin is SoC GPIO then set the HP GPIO mode to ULP_PERI_ON_SOC_PIN_MODE.
    RSI_EGPIO_PadReceiverEnable((uint8_t)(i2s->io.dout0->pin));
    RSI_EGPIO_SetPinMux(EGPIO, i2s->io.dout0->port, i2s->io.dout0->pin, EGPIO_PIN_MUX_MODE9);
    if (i2s->io.dout0->pad_sel != 0) {
      RSI_EGPIO_PadSelectionEnable(i2s->io.dout0->pad_sel);
    }

    if (i2s->io.dout0->pin >= I2S_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MIN
        && i2s->io.dout0->pin <= I2S_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MAX) {
      RSI_EGPIO_UlpSocGpioMode(ULPCLK, (i2s->io.dout0->pin - 38), i2s->io.dout0->mode);
    } else {
      RSI_EGPIO_UlpSocGpioMode(ULPCLK, (i2s->io.dout0->pin - 6), i2s->io.dout0->mode);
    }
  }

  // RX pin
  //if the pin is ULP_GPIO then set the pin mode for direct ULP_GPIO.
  if (i2s->io.din0->pin >= GPIO_MAX_PIN) {
    RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(i2s->io.din0->pin - GPIO_MAX_PIN));
    RSI_EGPIO_SetPinMux(EGPIO1, i2s->io.din0->port, (uint8_t)(i2s->io.din0->pin - GPIO_MAX_PIN), i2s->io.din0->mode);
  } else { // if the pin is SoC GPIO then set the HP GPIO mode to ULP_PERI_ON_SOC_PIN_MODE.
    RSI_EGPIO_PadReceiverEnable((uint8_t)(i2s->io.din0->pin));
    RSI_EGPIO_SetPinMux(EGPIO, i2s->io.din0->port, i2s->io.din0->pin, EGPIO_PIN_MUX_MODE9);
    if (i2s->io.din0->pad_sel != 0) {
      RSI_EGPIO_PadSelectionEnable(i2s->io.din0->pad_sel);
    }

    if (i2s->io.din0->pin >= I2S_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MIN
        && i2s->io.din0->pin <= I2S_ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MAX) {
      RSI_EGPIO_UlpSocGpioMode(ULPCLK, (i2s->io.din0->pin - 38), i2s->io.din0->mode);
    } else {
      RSI_EGPIO_UlpSocGpioMode(ULPCLK, (i2s->io.din0->pin - 6), i2s->io.din0->mode);
    }
  }
}

/*==============================================*/
/**
 *  @fn          void i2s_chnl_Init(ARM_SAI_SignalEvent_t cb_event, I2S_RESOURCES *i2s)
 *  @brief       Initialize I2S Channel. 
 *  @param[in]   cb_event : Pointer to \ref ARM_SAI_SignalEvent_t
 *  @param[in]   i2s      : Pointer to I2S resources
 *  @return      None
 */
void i2s_chnl_Init(ARM_SAI_SignalEvent_t cb_event, I2S_RESOURCES *i2s)
{
  // Initialize I2S Run-Time resources
  i2s->info->cb_event            = cb_event;
  i2s->info->status.frame_error  = 0U;
  i2s->info->status.rx_busy      = 0U;
  i2s->info->status.rx_overflow  = 0U;
  i2s->info->status.tx_busy      = 0U;
  i2s->info->status.tx_underflow = 0U;
  if (i2s->reg == I2S0) {
#if defined(SLI_SI917)
    RSI_PS_M4ssPeriPowerUp(M4SS_PWRGATE_ULP_EFUSE_PERI);
#else
    RSI_PS_M4ssPeriPowerUp(M4SS_PWRGATE_ULP_PERI3);
#endif
    if (i2s->xfer_chnl) {
      I2S0_Chnl1_PinMux(i2s);
    } else {
      I2S0_Chnl0_PinMux(i2s);
    }
  }
  if (i2s->reg == I2S1) {
    RSI_PS_UlpssPeriPowerUp(ULPSS_PWRGATE_ULP_I2S);
    if (i2s->reg == I2S1) {
      I2S1_PinMux(i2s);
    }
  }
}

/*==============================================*/
/**
 *  @fn          int32_t I2S_Initialize(ARM_SAI_SignalEvent_t cb_event,
 *                       I2S_RESOURCES *i2s,
 *                       UDMA_RESOURCES *udma,
 *                       RSI_UDMA_DESC_T *UDMA_Table,
 *                       RSI_UDMA_HANDLE_T *udmaHandle,
 *                       uint32_t *mem)
 *  @brief       Initialize I2S Interface. 
 *  @param[in]   cb_event   : Pointer to \ref ARM_SAI_SignalEvent_t
 *  @param[in]   i2s        : Pointer to I2S resources
 *  @param[in]   udma       : Pointer to UDMA resources
 *  @param[in]   UDMA_Table : Pointer to UDMA descriptor table
 *  @param[in]   udmaHandle : Pointer to UDMA handler
 *  @param[in]   mem        : Pointer to memory area used to driver context
 *  @return      \ref execution_status
 */
int32_t I2S_Initialize(ARM_SAI_SignalEvent_t cb_event,
                       I2S_RESOURCES *i2s,
                       UDMA_RESOURCES *udma,
                       RSI_UDMA_DESC_T *UDMA_Table,
                       RSI_UDMA_HANDLE_T *udmaHandle,
                       uint32_t *mem)
{
  if (i2s->flags & I2S_FLAG_INITIALIZED) {
    // Driver is already initialized
    return ARM_DRIVER_OK;
  }

  i2s_chnl_Init(cb_event, i2s);

  // UDMA Initialize
  if ((i2s->dma_tx != NULL) || (i2s->dma_rx != NULL)) {
    if ((i2s->reg == I2S0) || (i2s->reg == I2S1)) {
      *udmaHandle = UDMAx_Initialize(udma, UDMA_Table, udmaHandle, mem);
    }
  }
  i2s->flags = I2S_FLAG_INITIALIZED;

  return ARM_DRIVER_OK;
}

/*==============================================*/
/**
 *  @fn          int32_t I2S_Uninitialize(I2S_RESOURCES *i2s, UDMA_RESOURCES *udma)
 *  @brief       Uninitialize I2S Interface. 
 *  @param[in]   i2s        : Pointer to I2S resources
 *  @param[in]   udma       : Pointer to UDMA resources
 *  @return      \ref execution_status
 */
int32_t I2S_Uninitialize(I2S_RESOURCES *i2s, UDMA_RESOURCES *udma)
{
  UNUSED_PARAMETER(udma);
  // Disable I2S peripheral clock
  if (i2s->reg == I2S0) {
    RSI_CLK_PeripheralClkDisable2(M4CLK, I2SM_INTF_SCLK_ENABLE | I2SM_PCLK_ENABLE);
  }
  if (i2s->reg == I2S1) {
    RSI_ULPSS_UlpPeriClkDisable(ULPCLK, CLK_ENABLE_I2S);
  }
  i2s->flags = 0U;
  return ARM_DRIVER_OK;
}

/*==============================================*/
/**
 *  @fn          int32_t I2S_PowerControl(ARM_POWER_STATE state, I2S_RESOURCES *i2s, UDMA_RESOURCES *udma, RSI_UDMA_HANDLE_T udmaHandle)
 *  @brief       Configure Power to I2S interface. 
 *  @param[in]   state      : I2S Power state \ref ARM_POWER_STATE
 *  @param[in]   i2s        : Pointer to I2S resources
 *  @param[in]   udma       : Pointer to UDMA resources
 *  @param[in]   udmaHandle : Pointer to UDMA handler
 *  @return      \ref execution_status
 */
int32_t I2S_PowerControl(ARM_POWER_STATE state, I2S_RESOURCES *i2s, UDMA_RESOURCES *udma, RSI_UDMA_HANDLE_T udmaHandle)
{
  switch (state) {
    case ARM_POWER_OFF:
      // Disable I2S IRQ
      NVIC_DisableIRQ(i2s->irq_num);

      if (i2s->xfer_chnl) {
        if ((i2s->dma_tx) && (i2s->info->status.tx_busy != 0U)) {
          if (i2s->reg == I2S0) {
            UDMAx_ChannelDisable(i2s->dma_tx->channel, udma, udmaHandle);
          }
          if (i2s->reg == I2S1) {
            return ERROR_I2S_INVALID_ARG;
          }
        }
      } else {
        if ((i2s->dma_tx) && (i2s->info->status.tx_busy != 0U)) {
          if ((i2s->reg == I2S0) || (i2s->reg == I2S0)) {
            UDMAx_ChannelDisable(i2s->dma_tx->channel, udma, udmaHandle);
          }
        }
      }

      // Disable I2S peripheral
      i2s->reg->I2S_IER_b.IEN = 0;

      // Disable I2S peripheral clock
      if (i2s->reg == I2S0) {
        RSI_CLK_PeripheralClkDisable2(M4CLK, I2SM_INTF_SCLK_ENABLE | I2SM_PCLK_ENABLE);
      }
      if (i2s->reg == I2S1) {
        RSI_ULPSS_UlpPeriClkDisable(ULPCLK, CLK_ENABLE_I2S);
      }

      // Clear pending I2S interrupts in NVIC
      NVIC_ClearPendingIRQ(i2s->irq_num);

      // Clear driver variables
      i2s->info->status.frame_error  = 0U;
      i2s->info->status.rx_busy      = 0U;
      i2s->info->status.rx_overflow  = 0U;
      i2s->info->status.tx_busy      = 0U;
      i2s->info->status.tx_underflow = 0U;

      i2s->flags &= ~I2S_FLAG_POWERED;
      break;

    case ARM_POWER_LOW:
      return ARM_DRIVER_ERROR_UNSUPPORTED;

    case ARM_POWER_FULL:
      if ((i2s->flags & I2S_FLAG_INITIALIZED) == 0U) {
        return ARM_DRIVER_ERROR;
      }
      if ((i2s->flags & I2S_FLAG_POWERED) != 0U) {
        return ARM_DRIVER_OK;
      }
      if (i2s->reg == I2S0) {
        RSI_CLK_PeripheralClkEnable(M4CLK, I2SM_CLK, ENABLE_STATIC_CLK);
        //RSI_CLK_I2sClkConfig(M4CLK,i2s->clk->clk_src,i2s->clk->div_fact);
      }
      if (i2s->reg == I2S1) {
        RSI_ULPSS_PeripheralEnable(ULPCLK, ULP_I2S_CLK, ENABLE_STATIC_CLK);
        //        RSI_ULPSS_UlpI2sClkConfig(ULPCLK ,RTE_I2S1_CLK_SRC,8);
      }
      // Disable I2S interrupts
      RSI_I2S_InterruptEnableDisable(i2s, i2s->xfer_chnl, F_RXDAM | F_TXFEM | F_RXFOM | F_TXFOM, INTR_MASK);

      // Disable transmitter and receiver blocks
      // Disable all TX channels
      if (i2s->xfer_chnl) {
        i2s->reg->CHANNEL_CONFIG[0].I2S_TER_b.TXCHEN = 0x0;
        i2s->reg->CHANNEL_CONFIG[0].I2S_RER_b.RXCHEN = 0x0;
      } else {
        i2s->reg->CHANNEL_CONFIG[1].I2S_TER_b.TXCHEN = 0x0;
        i2s->reg->CHANNEL_CONFIG[1].I2S_RER_b.RXCHEN = 0x0;
      }
      i2s->reg->I2S_IRER_b.RXEN = 0;
      i2s->reg->I2S_ITER_b.TXEN = 0;

      // Clear chnl0 driver variables
      i2s->info->status.frame_error  = 0U;
      i2s->info->status.rx_busy      = 0U;
      i2s->info->status.rx_overflow  = 0U;
      i2s->info->status.tx_busy      = 0U;
      i2s->info->status.tx_underflow = 0U;

      i2s->info->rx.residue_cnt = 0U;
      i2s->info->rx.residue_num = 0U;
      i2s->info->tx.residue_cnt = 0U;
      i2s->info->tx.residue_num = 0U;
      i2s->flags                = I2S_FLAG_POWERED | I2S_FLAG_INITIALIZED;

      // Clear and Enable SAI IRQ
      NVIC_ClearPendingIRQ(i2s->irq_num);
      //NVIC_EnableIRQ(i2s->irq_num);
      break;

    default:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  return ARM_DRIVER_OK;
}

/*==============================================*/
/**
 *  @fn          int32_t I2S_Send(const void *data,
 *               uint32_t num,
 *               I2S_RESOURCES *i2s,
 *               UDMA_RESOURCES *udma,
 *               UDMA_Channel_Info *chnl_info,
 *               RSI_UDMA_HANDLE_T udmaHandle)
 *  @brief       This APIs used to send data to I2S transmitter  
 *  @param[in]   data      : Pointer to buffer with data to send to I2S transmitter
 *  @param[in]   num       : Number of data items to send
 *  @param[in]   i2s       : Pointer to I2S resources
 *  @param[in]   udma      : Pointer to UDMA resources
 *  @param[in]   chnl_info : Pointer to store uDMA channel informations
 *  @param[in]   udmaHandle: Pointer to UDMA handler
 *  @return      \ref execution_status
 */
int32_t I2S_Send(const void *data,
                 uint32_t num,
                 I2S_RESOURCES *i2s,
                 UDMA_RESOURCES *udma,
                 UDMA_Channel_Info *chnl_info,
                 RSI_UDMA_HANDLE_T udmaHandle)
{
  volatile int32_t stat       = 0;
  RSI_UDMA_CHA_CFG_T chnl_cfg = { 0 };
  uint32_t resolution         = 0;

  if ((data == NULL) || (num == 0U)) {
    // Invalid parameters
    return ARM_DRIVER_ERROR_PARAMETER;
  }
  if ((i2s->flags & I2S_FLAG_CONFIGURED) == 0U) {
    // I2S is not configured (mode not selected)
    return ARM_DRIVER_ERROR;
  }
  if (i2s->info->status.tx_busy) {
    // Send is not completed yet
    return ARM_DRIVER_ERROR_BUSY;
  }
  i2s->info->status.tx_busy      = 1U;
  i2s->info->status.tx_underflow = 0U;
  i2s->info->tx.buf              = (uint8_t *)data;
  i2s->info->tx.cnt              = 0U;
  num                            = num * (i2s->info->tx.data_bits / 8U);

  i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_TXFCR_b.TXCHET = (unsigned int)((i2s->tx_fifo_level) & 0x0F);

  // Only 32-bit value can be written to FIFO. If there is data left from last send,
  // fill residue buffer to 32bits and write to TX FIFO
  if (i2s->info->tx.residue_cnt != 0U) {
    while ((i2s->info->tx.residue_cnt < 4U) && (i2s->info->tx.cnt < num)) {
      i2s->info->tx.residue_buf[i2s->info->tx.residue_cnt++] = i2s->info->tx.buf[i2s->info->tx.cnt++];
    }
    if (i2s->info->tx.residue_cnt == 4U) {
      // Write 32bits to TX FIFO
      i2s->reg->CHANNEL_CONFIG[1].I2S_LTHR = i2s->info->tx.residue_buf[3];
      i2s->reg->CHANNEL_CONFIG[1].I2S_LTHR = (i2s->reg->CHANNEL_CONFIG[1].I2S_LTHR << 8) | i2s->info->tx.residue_buf[2];
      i2s->reg->CHANNEL_CONFIG[1].I2S_LTHR = (i2s->reg->CHANNEL_CONFIG[1].I2S_LTHR << 8) | i2s->info->tx.residue_buf[1];
      i2s->reg->CHANNEL_CONFIG[1].I2S_LTHR = (i2s->reg->CHANNEL_CONFIG[1].I2S_LTHR << 8) | i2s->info->tx.residue_buf[0];

      // There is no valid data in residue buffer
      i2s->info->tx.residue_cnt = 0U;
    }
  }
  if (i2s->info->tx.residue_cnt != 0U) {
    while ((i2s->info->tx.residue_cnt < 4U) && (i2s->info->tx.cnt < num)) {
      i2s->info->tx.residue_buf[i2s->info->tx.residue_cnt++] = i2s->info->tx.buf[i2s->info->tx.cnt++];
    }
    if (i2s->info->tx.residue_cnt == 4U) {
      // Write 32bits to TX FIFO
      i2s->reg->CHANNEL_CONFIG[1].I2S_LTHR = i2s->info->tx.residue_buf[3];
      i2s->reg->CHANNEL_CONFIG[1].I2S_LTHR = (i2s->reg->CHANNEL_CONFIG[1].I2S_LTHR << 8) | i2s->info->tx.residue_buf[2];
      i2s->reg->CHANNEL_CONFIG[1].I2S_LTHR = (i2s->reg->CHANNEL_CONFIG[1].I2S_LTHR << 8) | i2s->info->tx.residue_buf[1];
      i2s->reg->CHANNEL_CONFIG[1].I2S_LTHR = (i2s->reg->CHANNEL_CONFIG[1].I2S_LTHR << 8) | i2s->info->tx.residue_buf[0];

      // There is no valid data in residue buffer
      i2s->info->tx.residue_cnt = 0U;
    }
  }
  //RSI_I2S_InterruptEnableDisable(i2s,i2s->xfer_chnl,F_TXFEM,INTR_MASK);
  RSI_I2S_InterruptEnableDisable(i2s, i2s->xfer_chnl, F_TXFEM, INTR_UNMASK);

  i2s->info->tx.num                                         = num;
  i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_TER_b.TXCHEN = 0x1;

  // DMA mode for Channel0
  if (i2s->dma_tx != NULL) {
    num = num - i2s->info->tx.cnt;
    if (num < 4U) {
      // Enable I2S transmitter interrupt
      RSI_I2S_InterruptEnableDisable(i2s, i2s->xfer_chnl, F_TXFEM, INTR_UNMASK);
    } else {
      chnl_cfg.altStruct = 0;
      chnl_cfg.burstReq  = 1;
      if ((num / 2) < 1024) {
        i2s->dma_tx->control.totalNumOfDMATrans = (unsigned int)(((num / (i2s->info->tx.data_bits / 8U)) - 1) & 0x03FF);
      } else {
        i2s->dma_tx->control.totalNumOfDMATrans = 0x3FF;
      }
      if (i2s->reg == I2S0) {
        chnl_cfg.channelPrioHigh = UDMA0_CHNL_PRIO_LVL;
      } else {
        chnl_cfg.channelPrioHigh = UDMA1_CHNL_PRIO_LVL;
      }
      chnl_cfg.dmaCh     = i2s->dma_tx->channel;
      chnl_cfg.periAck   = 0;
      chnl_cfg.periphReq = 0;
      chnl_cfg.reqMask   = 0;
      resolution         = i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_TCR_b.WLEN;
      if ((resolution == RES_12_BIT) || (resolution == RES_16_BIT)) {
        i2s->dma_tx->control.srcSize = SRC_SIZE_16;
        i2s->dma_tx->control.srcInc  = SRC_INC_16;
        i2s->dma_tx->control.dstSize = DST_SIZE_16;
        i2s->dma_tx->control.dstInc  = DST_INC_NONE;
      } else {
        i2s->dma_tx->control.srcSize = SRC_SIZE_32;
        i2s->dma_tx->control.srcInc  = SRC_INC_32;
        i2s->dma_tx->control.dstSize = DST_SIZE_32;
        i2s->dma_tx->control.dstInc  = DST_INC_NONE;
      }
      // Configure DMA channel
      if ((i2s->reg == I2S0) || (i2s->reg == I2S1)) {
        stat = UDMAx_ChannelConfigure(udma,
                                      i2s->dma_tx->channel,
                                      (uint32_t)(i2s->info->tx.buf),
                                      (uint32_t)(&(i2s->reg->I2S_TXDMA)),
                                      num / (i2s->info->tx.data_bits / 8U), //num / 4U,
                                      i2s->dma_tx->control,
                                      &chnl_cfg,
                                      i2s->dma_tx->cb_event,
                                      chnl_info,
                                      udmaHandle);
        if (stat == -1) {
          return ARM_DRIVER_ERROR;
        }
        UDMAx_ChannelEnable(i2s->dma_tx->channel, udma, udmaHandle);
        UDMAx_DMAEnable(udma, udmaHandle);
#ifdef I2S_LOOP_BACK
        if (i2s->reg == I2S0) {
          i2s->reg->I2S_IRER_b.RXEN = 0x1;
        }
#endif
#ifdef I2S1_LOOP_BACK
        if (i2s->reg == I2S1) {
          i2s->reg->I2S_IRER_b.RXEN = 0x1;
        }
#endif
        i2s->reg->I2S_ITER_b.TXEN = 0x1;
        if (i2s->info->tx.master) {
          i2s->reg->I2S_CER_b.CLKEN = ENABLE;
        }
      }
    }
  } else {
    // Enable I2S transmitter interrupt
    RSI_I2S_InterruptEnableDisable(i2s, i2s->xfer_chnl, F_TXFEM, INTR_UNMASK);
  }
  return ARM_DRIVER_OK;
}

/*==============================================*/
/**
 *  @fn          int32_t I2S_Receive(void *data,
 *               uint32_t num,
 *               I2S_RESOURCES *i2s,
 *               UDMA_RESOURCES *udma,
 *               UDMA_Channel_Info *chnl_info,
 *               RSI_UDMA_HANDLE_T udmaHandle)
 *  @brief       This APIs used to receive data from I2S receiver  
 *  @param[in]   data      : Pointer to buffer recive data from I2S receiver.
 *  @param[in]   num       : Number of data items to send
 *  @param[in]   i2s       : Pointer to I2S resources
 *  @param[in]   udma      : Pointer to UDMA resources
 *  @param[in]   chnl_info : Pointer to store uDMA channel informations
 *  @param[in]   udmaHandle: Pointer to UDMA handler
 *  @return      \ref execution_status
 */
int32_t I2S_Receive(void *data,
                    uint32_t num,
                    I2S_RESOURCES *i2s,
                    UDMA_RESOURCES *udma,
                    UDMA_Channel_Info *chnl_info,
                    RSI_UDMA_HANDLE_T udmaHandle)
{
  volatile int32_t stat       = 0;
  RSI_UDMA_CHA_CFG_T chnl_cfg = { 0 };
  uint32_t resolution         = 0;

  if ((data == NULL) || (num == 0U)) {
    // Invalid parameters
    return ARM_DRIVER_ERROR_PARAMETER;
  }
  if ((i2s->flags & I2S_FLAG_CONFIGURED) == 0U) {
    // I2S is not configured (mode not selected)
    return ARM_DRIVER_ERROR;
  }
  if (i2s->info->status.rx_busy) {
    // Receive is not completed yet
    return ARM_DRIVER_ERROR_BUSY;
  }

  // Set Receive active flag
  i2s->info->status.rx_busy = 1U;

  // Clear RX overflow flag
  i2s->info->status.rx_overflow = 0U;

  // Save receive buffer info
  i2s->info->rx.buf = (uint8_t *)data;
  i2s->info->rx.cnt = 0U;

  // Convert from number of samples to number of bytes
  num = num * (i2s->info->rx.data_bits / 8U);

  // Set FIFO level and enable RX DMA
  i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RFCR_b.RXCHDT = (unsigned int)((i2s->rx_fifo_level) & 0x0F);

  while ((i2s->info->rx.cnt < num) && (i2s->info->rx.residue_cnt < i2s->info->rx.residue_num)) {
    // RX Data available in residue buffer
    i2s->info->rx.buf[i2s->info->rx.cnt++] = i2s->info->rx.residue_buf[i2s->info->rx.residue_cnt++];

    if (i2s->info->rx.residue_cnt == i2s->info->rx.residue_num) {
      // Residue buffer empty
      i2s->info->rx.residue_cnt = 0U;
      i2s->info->rx.residue_num = 0U;
    }
  }

  // Disable I2S receive interrupt
  //RSI_I2S_InterruptEnableDisable(i2s,i2s->xfer_chnl,F_RXDAM,INTR_MASK );
  RSI_I2S_InterruptEnableDisable(i2s, i2s->xfer_chnl, F_RXDAM, INTR_UNMASK);
  i2s->info->rx.num = num;

  num = num - i2s->info->rx.cnt;
  // DMA mode
  if ((i2s->dma_rx != NULL) && (num >= 4U)) {
    // Set offset in RX Buffer for DMA transfer
    //offset = i2s->info->rx.cnt;

    // Update RX count
    num               = num / 4;
    i2s->info->rx.cnt = i2s->info->rx.cnt + (num * 4);

    chnl_cfg.altStruct = 0;
    chnl_cfg.burstReq  = 1;
    if ((num / 2) < 1024) {
      i2s->dma_rx->control.totalNumOfDMATrans = (unsigned int)(((num / (i2s->info->rx.data_bits / 8U)) - 1) & 0x03FF);
    } else {
      i2s->dma_rx->control.totalNumOfDMATrans = 0x3FF;
    }
    if (i2s->reg == I2S0) {
      chnl_cfg.channelPrioHigh = UDMA0_CHNL_PRIO_LVL;
    } else {
      chnl_cfg.channelPrioHigh = UDMA1_CHNL_PRIO_LVL;
    }
    chnl_cfg.dmaCh     = i2s->dma_rx->channel;
    chnl_cfg.periAck   = 0;
    chnl_cfg.periphReq = 0;
    chnl_cfg.reqMask   = 0;
    resolution         = i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RCR_b.WLEN;
    if ((resolution == RES_12_BIT) || (resolution == RES_16_BIT)) {
      i2s->dma_rx->control.srcSize = SRC_SIZE_16;
      i2s->dma_rx->control.srcInc  = SRC_INC_NONE;
      i2s->dma_rx->control.dstSize = DST_SIZE_16;
      i2s->dma_rx->control.dstInc  = DST_INC_16;
    } else {
      i2s->dma_rx->control.srcSize = SRC_SIZE_32;
      i2s->dma_rx->control.srcInc  = SRC_INC_NONE;
      i2s->dma_rx->control.dstSize = DST_SIZE_32;
      i2s->dma_rx->control.dstInc  = DST_INC_32;
    }
    // Configure DMA channel
    if ((i2s->reg == I2S0) || (i2s->reg == I2S1)) {
      stat = UDMAx_ChannelConfigure(udma,
                                    i2s->dma_rx->channel,
                                    (uint32_t)(&(i2s->reg->I2S_RXDMA)),
                                    (uint32_t)(i2s->info->rx.buf),
                                    (i2s->info->rx.cnt) / (i2s->info->rx.data_bits / 8U),
                                    i2s->dma_rx->control,
                                    &chnl_cfg,
                                    i2s->dma_rx->cb_event,
                                    chnl_info,
                                    udmaHandle);

      if (stat == -1) {
        return ARM_DRIVER_ERROR;
      }
      i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RER_b.RXCHEN = 0x1;
// Only enable RXEN for the correct instance and avoid duplicate writes
#ifndef I2S_LOOP_BACK
      if (i2s->reg == I2S0) {
        i2s->reg->I2S_IRER_b.RXEN = 0x1;
      }
#endif
#ifndef I2S1_LOOP_BACK
      if (i2s->reg == I2S1) {
        i2s->reg->I2S_IRER_b.RXEN = 0x1;
      }
#endif
      UDMAx_ChannelEnable(i2s->dma_rx->channel, udma, udmaHandle);
      UDMAx_DMAEnable(udma, udmaHandle);
#ifndef I2S_LOOP_BACK
      if (i2s->reg == I2S0 && i2s->info->rx.master) {
        i2s->reg->I2S_CER_b.CLKEN = ENABLE; //RX_Clock is not required in Loopback connections.
      }
#endif
#ifndef I2S1_LOOP_BACK
      if (i2s->reg == I2S1 && i2s->info->rx.master) {
        i2s->reg->I2S_CER_b.CLKEN = ENABLE; //RX_Clock is not required in Loopback connections.
      }
#endif
    }
  } else {
    RSI_I2S_InterruptEnableDisable(i2s, i2s->xfer_chnl, F_RXDAM, INTR_UNMASK);
  }
  return ARM_DRIVER_OK;
}

/*==============================================*/
/**
 *  @fn          uint32_t I2S_GetTxCount(I2S_RESOURCES *i2s)
 *  @brief       This APIs used to Get transmitted data count.
 *  @param[in]   i2s  : Pointer to I2S resources
 *  @return      number of data items transmitted
 */
uint32_t I2S_GetTxCount(I2S_RESOURCES *i2s)
{
  uint32_t cnt = 0;

  // Convert count in bytes to count of samples
  cnt = i2s->info->tx.cnt / (i2s->info->tx.data_bits / 8U);
  return (cnt);
}

/*==============================================*/
/**
 *  @fn          uint32_t I2S_GetRxCount(I2S_RESOURCES *i2s)
 *  @brief       This APIs used to Get received data count.
 *  @param[in]   i2s  : Pointer to I2S resources
 *  @return      number of data items received
 */
uint32_t I2S_GetRxCount(I2S_RESOURCES *i2s)
{
  uint32_t cnt = 0;

  // Convert count in bytes to count of samples
  cnt = i2s->info->rx.cnt / (i2s->info->rx.data_bits / 8U);
  return (cnt);
}

/*==============================================*/
/**
 *  @fn          ARM_SAI_STATUS I2S_GetStatus(I2S_RESOURCES *i2s)
 *  @brief       This APIs used to retrieves the current SAI interface status 	 
 *  @param[in]   i2s   : Pointer to I2S resources
 *  @return      Get SAI status.
 */
ARM_SAI_STATUS I2S_GetStatus(I2S_RESOURCES *i2s)
{
  ARM_SAI_STATUS status;

  status.frame_error  = (unsigned int)(i2s->info->status.frame_error & 0x01);
  status.rx_busy      = (unsigned int)(i2s->info->status.rx_busy & 0x01);
  status.rx_overflow  = (unsigned int)(i2s->info->status.rx_overflow & 0x01);
  status.tx_busy      = (unsigned int)(i2s->info->status.tx_busy & 0x01);
  status.tx_underflow = (unsigned int)(i2s->info->status.tx_underflow & 0x01);

  return status;
}

/*==============================================*/
/**
 *  @fn          void I2S_IRQHandler(I2S_RESOURCES *i2s)
 *  @brief       I2S Interrupt handler.	 
 *  @param[in]   i2s   : Pointer to I2S resources
 *  @return      None
 */
void I2S_IRQHandler(I2S_RESOURCES *i2s)
{
  uint32_t state, val = 0, event, level = 0;
  uint32_t i, j;
  void *ptr_buf;

  state = i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_ISR;
  event = 0U;

  if (state) {
    // Fill TX FIFO if needed
    // Check for TX underflow
    if (i2s->info->tx.num == 0U) {
      // Set TX underflow event and flag
      i2s->info->status.tx_underflow = 1U;
      event |= ARM_SAI_EVENT_TX_UNDERFLOW;

      // Disable TX interrupt
      //RSI_I2S_InterruptEnableDisable( i2s->reg,i2s->xfer_chnl, F_TXFEM ,INTR_MASK );
    }
    if (state & I2S_STAT_TRANSMIT_EMPTY) {
      while (((i2s->info->tx.cnt + 4U) <= i2s->info->tx.num)) {
        // Copy all available 32bit data to FIFO, until FIFO is full
        ptr_buf = i2s->info->tx.buf + i2s->info->tx.cnt;
        if ((i2s->info->tx.cnt) / 2 == 0) {
          i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_LTHR = *(uint32_t *)(ptr_buf);
        }
        if ((i2s->info->tx.cnt) / 2 == 1) {
          i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RTHR = *(uint32_t *)(ptr_buf);
        }
        // Update TX buffer info
        i2s->info->tx.cnt += 4U;
        level--;
      }

      if ((i2s->info->tx.cnt < i2s->info->tx.num) && (level != 0U)) {
        while (i2s->info->tx.cnt < i2s->info->tx.num) {
          // Copy remaining data to residue buffer (data < 32bits)
          i2s->info->tx.residue_buf[i2s->info->tx.residue_cnt++] = i2s->info->tx.buf[i2s->info->tx.cnt++];
        }
      }

      if (i2s->info->tx.cnt == i2s->info->tx.num) {
        i2s->info->status.tx_busy = 0U;
        i2s->info->tx.num         = 0U;
        event |= ARM_SAI_EVENT_SEND_COMPLETE;
      }
    }

    // RX interrupt
    if (state & I2S_STAT_RXDATA_OVERRUN) {
      // Check for RX overflow
      if (i2s->info->rx.num == 0U) {
        // Flush residue buffer
        i2s->info->rx.residue_cnt = 0U;
        i2s->info->rx.residue_num = 0U;

        i2s->info->status.rx_overflow = 1U;
        event |= ARM_SAI_EVENT_RX_OVERFLOW;
        RSI_I2S_InterruptEnableDisable(i2s, i2s->xfer_chnl, F_RXFOM, INTR_MASK);
      }
    }
    if (state & I2S_STAT_RXDA) {
      // Get FIFO level
      while (((i2s->info->rx.cnt + 4U) <= i2s->info->rx.num) && (level != 0U)) {
        // Read FIFO
        ptr_buf = i2s->info->rx.buf + i2s->info->rx.cnt;

        if ((i2s->info->rx.cnt) / 2 == 0) {
          *(uint32_t *)(ptr_buf) = i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_LRBR;
        }
        if ((i2s->info->rx.cnt) / 2 == 1) {
          *(uint32_t *)(ptr_buf) = i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RRBR;
        }

        i2s->info->rx.cnt += 4U;
        level--;
      }

      if ((i2s->info->rx.cnt < i2s->info->rx.num) && (level != 0U)) {
        if ((i2s->info->rx.cnt) / 2 == 0) {
          // Read FIFO
          val = i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_LRBR;
        }
        if ((i2s->info->rx.cnt) / 2 == 1) {
          // Read FIFO
          val = i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RRBR;
        }
        j = 0U;
        for (i = 0U; i < 4U; i++) {
          if ((i2s->info->rx.cnt < i2s->info->rx.num)) {
            i2s->info->rx.buf[i2s->info->rx.cnt++] = (uint8_t)(val >> j);
          } else {
            i2s->info->rx.residue_buf[i2s->info->rx.residue_num++] = (uint8_t)(val >> j);
          }
          j += 8U;
        }
      }

      if (i2s->info->rx.cnt == i2s->info->rx.num) {
        i2s->info->status.rx_busy = 0U;
        i2s->info->rx.num         = 0U;
        event |= ARM_SAI_EVENT_RECEIVE_COMPLETE;
      }
    }
  }
  if ((event != 0U) && (i2s->info->cb_event != NULL)) {
    i2s->info->cb_event(event);
  }
}

/*==============================================*/
/**
 *  @fn          void I2S_UDMA_Tx_Event(uint32_t event, uint8_t dmaCh, I2S_RESOURCES *i2s) 
 *  @brief       This API used to process UDMA tx event	 
 *  @param[in]   event   : I2S DMA transmit done event
 *  @param[in]   dmaCh   : I2S dma tx channel number
 *  @param[in]   i2s     : Pointer to I2S resources
 *  @return      None
 */
void I2S_UDMA_Tx_Event(uint32_t event, uint8_t dmaCh, I2S_RESOURCES *i2s)
{
  uint32_t evt = 0;
  (void)dmaCh;

  switch (event) {
    case UDMA_EVENT_XFER_DONE:
      // Update TX buffer info
      i2s->info->tx.cnt += (i2s->info->tx.num / 4U) * 4U;

      while (i2s->info->tx.cnt < i2s->info->tx.num) {
        // Copy remaining data to residue buffer (data < 32bits)
        i2s->info->tx.residue_buf[i2s->info->tx.residue_cnt++] = i2s->info->tx.buf[i2s->info->tx.cnt++];
      }

      // Clear TX busy flag
      i2s->info->status.tx_busy = 0U;

      // Clear TX num and enable TX interrupt to detect TX underflow
      i2s->info->tx.num = 0U;

      //i2s->reg->IRQ |= I2S_IRQ_TX_IRQ_ENABLE;
      RSI_I2S_InterruptEnableDisable(i2s, i2s->xfer_chnl, F_TXFOM, INTR_UNMASK);

      // Set Send complete event
      evt = ARM_SAI_EVENT_SEND_COMPLETE;
      break;
    case UDMA_EVENT_ERROR:
      break;
  }
  if ((evt != 0U) && (i2s->info->cb_event != NULL)) {
    i2s->info->cb_event(evt);
  }
}

/*==============================================*/
/**
 *  @fn          void I2S_UDMA_Rx_Event(uint32_t event, uint8_t dmaCh, I2S_RESOURCES *i2s) 
 *  @brief       This API used to process UDMA rx event  	 
 *  @param[in]   event   : I2S DMA receive done event
 *  @param[in]   dmaCh   : I2S dma rx channel number
 *  @param[in]   i2s     : Pointer to I2S resources
 *  @return      None
 */
void I2S_UDMA_Rx_Event(uint32_t event, uint8_t dmaCh, I2S_RESOURCES *i2s)
{
  uint32_t evt = 0U;
  uint32_t val;
  (void)dmaCh;

  switch (event) {
    case UDMA_EVENT_XFER_DONE:
      if (i2s->info->rx.cnt == i2s->info->rx.num) {
        // Clear RX busy flag
        i2s->info->status.rx_busy = 0U;

        // Set receive complete event
        evt = ARM_SAI_EVENT_RECEIVE_COMPLETE;

        // Set FIFO level to full, to detect RX overflow
        val = i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RFCR_b.RXCHDT;
        val |= (8U);
        i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RFCR_b.RXCHDT = (unsigned int)(val & 0x0F);
      } else {
        // Set user defined level, to retrieve remaining requested data
        val = i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RFCR_b.RXCHDT;
        val |= (uint32_t)(i2s->rx_fifo_level);
        i2s->reg->CHANNEL_CONFIG[i2s->xfer_chnl].I2S_RFCR_b.RXCHDT = (unsigned int)(val & 0x0F);
      }

      RSI_I2S_InterruptEnableDisable(i2s, i2s->xfer_chnl, F_RXFOM, INTR_UNMASK);
      ;
      break;
    case UDMA_EVENT_ERROR:
      break;
  }
  if ((evt != 0U) && (i2s->info->cb_event != NULL)) {
    i2s->info->cb_event(evt);
  }
}
/** @} */

#endif //A11_ROM || I2S_ROMDRIVER_PRESENT
