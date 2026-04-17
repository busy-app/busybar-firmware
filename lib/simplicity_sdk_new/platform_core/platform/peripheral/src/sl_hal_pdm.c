/***************************************************************************//**
 * @file
 * @brief Pulse Density Modulation (PDM) peripheral API
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

#include "sl_hal_pdm.h"
#if defined(PDM_PRESENT)

#include "sl_assert.h"
#include <stddef.h>

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
extern __INLINE void sl_hal_pdm_wait_sync(PDM_TypeDef *pdm);
extern __INLINE void sl_hal_pdm_wait_ready(PDM_TypeDef *pdm);
extern __INLINE void sl_hal_pdm_enable(PDM_TypeDef *pdm);
extern __INLINE void sl_hal_pdm_disable(PDM_TypeDef *pdm);
extern __INLINE void sl_hal_pdm_clear(PDM_TypeDef *pdm);
extern __INLINE void sl_hal_pdm_fifo_flush(PDM_TypeDef *pdm);
extern __INLINE void sl_hal_pdm_start(PDM_TypeDef *pdm);
extern __INLINE void sl_hal_pdm_stop(PDM_TypeDef *pdm);
extern __INLINE uint32_t sl_hal_pdm_rx(PDM_TypeDef *pdm);
extern __INLINE uint32_t sl_hal_pdm_get_status(PDM_TypeDef *pdm);
extern __INLINE void sl_hal_pdm_clear_interrupts(PDM_TypeDef *pdm,
                                                 uint32_t flags);
extern __INLINE void sl_hal_pdm_set_interrupts(PDM_TypeDef *pdm,
                                               uint32_t flags);
extern __INLINE void sl_hal_pdm_enable_interrupts(PDM_TypeDef *pdm,
                                                  uint32_t flags);
extern __INLINE void sl_hal_pdm_disable_interrupts(PDM_TypeDef *pdm,
                                                   uint32_t flags);
extern __INLINE uint32_t sl_hal_pdm_get_pending_interrupts(PDM_TypeDef *pdm);
extern __INLINE uint32_t sl_hal_pdm_get_enabled_pending_interrupts(PDM_TypeDef *pdm);

/***************************************************************************//**
 * Initialize the PDM peripheral.
 ******************************************************************************/
void sl_hal_pdm_init(PDM_TypeDef *pdm,
                     const sl_hal_pdm_init_t *init)
{
  // A sanity check of configuration parameters.
  EFM_ASSERT(init != NULL);
  EFM_ASSERT(init->down_sampling_rate <= (_PDM_CTRL_DSR_MASK >> _PDM_CTRL_DSR_SHIFT));
  EFM_ASSERT(init->gain               <= (_PDM_CTRL_GAIN_MASK >> _PDM_CTRL_GAIN_SHIFT));
  EFM_ASSERT(init->clk_prescaler      <= (_PDM_CFG1_PRESC_MASK >> _PDM_CFG1_PRESC_SHIFT));

  // Write the CFG0 register with the configurations.
#if defined(PDM_CFG0_NUMCH_THREE)
  pdm->CFG0 = (pdm->CFG0 & ~(_PDM_CFG0_CH3CLKPOL_MASK | _PDM_CFG0_CH2CLKPOL_MASK | _PDM_CFG0_CH1CLKPOL_MASK
                             | _PDM_CFG0_CH0CLKPOL_MASK | _PDM_CFG0_FIFODVL_MASK | _PDM_CFG0_DATAFORMAT_MASK
                             | _PDM_CFG0_NUMCH_MASK | _PDM_CFG0_FORDER_MASK | _PDM_CFG0_STEREOMODECH23_MASK
                             | _PDM_CFG0_STEREOMODECH01_MASK))
              | ((uint32_t)init->ch3_clock_polarity   << _PDM_CFG0_CH3CLKPOL_SHIFT)
              | ((uint32_t)init->ch2_clock_polarity   << _PDM_CFG0_CH2CLKPOL_SHIFT)
              | ((uint32_t)init->ch1_clock_polarity   << _PDM_CFG0_CH1CLKPOL_SHIFT)
              | ((uint32_t)init->ch0_clock_polarity   << _PDM_CFG0_CH0CLKPOL_SHIFT)
              | ((uint32_t)init->fifo_valid_watermark << _PDM_CFG0_FIFODVL_SHIFT)
              | ((uint32_t)init->data_format          << _PDM_CFG0_DATAFORMAT_SHIFT)
              | ((uint32_t)init->number_channels      << _PDM_CFG0_NUMCH_SHIFT)
              | ((uint32_t)init->filter_order         << _PDM_CFG0_FORDER_SHIFT)
              | ((uint32_t)init->ch2ch3_stereo_enable << _PDM_CFG0_STEREOMODECH23_SHIFT)
              | ((uint32_t)init->ch0ch1_stereo_enable << _PDM_CFG0_STEREOMODECH01_SHIFT);
#else
  pdm->CFG0 = (pdm->CFG0 & ~(_PDM_CFG0_CH1CLKPOL_MASK | _PDM_CFG0_CH0CLKPOL_MASK | _PDM_CFG0_FIFODVL_MASK
                             | _PDM_CFG0_DATAFORMAT_MASK | _PDM_CFG0_NUMCH_MASK | _PDM_CFG0_FORDER_MASK
                             | _PDM_CFG0_STEREOMODECH01_MASK))
              | ((uint32_t)init->ch1_clock_polarity   << _PDM_CFG0_CH1CLKPOL_SHIFT)
              | ((uint32_t)init->ch0_clock_polarity   << _PDM_CFG0_CH0CLKPOL_SHIFT)
              | ((uint32_t)init->fifo_valid_watermark << _PDM_CFG0_FIFODVL_SHIFT)
              | ((uint32_t)init->data_format          << _PDM_CFG0_DATAFORMAT_SHIFT)
              | ((uint32_t)init->number_channels      << _PDM_CFG0_NUMCH_SHIFT)
              | ((uint32_t)init->filter_order         << _PDM_CFG0_FORDER_SHIFT)
              | ((uint32_t)init->ch0ch1_stereo_enable << _PDM_CFG0_STEREOMODECH01_SHIFT);
#endif

  // Write the CFG1 register with the configurations.
  pdm->CFG1 = (pdm->CFG1 & ~_PDM_CFG1_PRESC_MASK)
              | ((uint32_t)init->clk_prescaler            << _PDM_CFG1_PRESC_SHIFT);

  sl_hal_pdm_enable(pdm);

  // Write the CTRL register with the configurations.
  pdm->CTRL = (pdm->CTRL & ~(_PDM_CTRL_DSR_MASK | _PDM_CTRL_GAIN_MASK))
              | ((uint32_t)init->down_sampling_rate   << _PDM_CTRL_DSR_SHIFT)
              | ((uint32_t)init->gain                 << _PDM_CTRL_GAIN_SHIFT);
}

/***************************************************************************//**
 * Initialize PDM registers with reset values.
 ******************************************************************************/
void sl_hal_pdm_reset(PDM_TypeDef *pdm)
{
  // Wait for any pending CMD synchronization.
  sl_hal_pdm_wait_sync(pdm);

  if (pdm->EN != 0U) {
    pdm->CMD_SET = PDM_CMD_FIFOFL | PDM_CMD_CLEAR | PDM_CMD_STOP;
    sl_hal_pdm_wait_sync(pdm);
  }

  pdm->EN_SET = PDM_EN_EN;
  pdm->CTRL = _PDM_CTRL_RESETVALUE;
  pdm->IEN = _PDM_IEN_RESETVALUE;
  pdm->IF_CLR = _PDM_IF_MASK;

  // Must wait for SYNCBUSY before disabling an enabled PDM.
  sl_hal_pdm_wait_sync(pdm);

  pdm->EN = _PDM_EN_RESETVALUE;
  pdm->CFG0 = _PDM_CFG0_RESETVALUE;
  pdm->CFG1 = _PDM_CFG1_RESETVALUE;

  sl_hal_pdm_wait_sync(pdm);
}

#endif /* defined(PDM_PRESENT) */
