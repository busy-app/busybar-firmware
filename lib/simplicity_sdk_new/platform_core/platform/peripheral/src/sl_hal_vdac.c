/***************************************************************************//**
 * @file
 * @brief Digital to Analog Converter (VDAC) Peripheral API.
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

#include "sl_hal_vdac.h"
#if defined(VDAC_PRESENT)
#include <stddef.h>

/***************************************************************************//**
 * @addtogroup vdac VDAC - Digital to Analog Voltage Converter.
 * @brief Digital to Analog Voltage Converter (VDAC) Peripheral API.
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

// Validation of the VDAC channel for assert statements.
#define SLI_HAL_VDAC_CH_VALID(channel)    ((channel) <= 1)

// A maximum VDAC clock.
#define SLI_HAL_VDAC_MAX_CLOCK            1000000

// The maximum clock frequency of the internal clock oscillator, 10 MHz + 20%.
#define SLI_HAL_VDAC_INTERNAL_CLOCK_FREQ  12000000

/** @endcond */

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

/*******************************************************************************
 ***************************   GLOBAL FUNCTIONS   ******************************
 ******************************************************************************/

extern __INLINE void sl_hal_vdac_wait_sync(VDAC_TypeDef *vdac);
extern __INLINE void sl_hal_vdac_wait_ready(VDAC_TypeDef *vdac);
extern __INLINE void sl_hal_vdac_enable(VDAC_TypeDef *vdac);
extern __INLINE void sl_hal_vdac_disable(VDAC_TypeDef *vdac);
extern __INLINE void sl_hal_vdac_enable_interrupts(VDAC_TypeDef *vdac,
                                                   uint32_t flags);
extern __INLINE void sl_hal_vdac_disable_interrupts(VDAC_TypeDef *vdac,
                                                    uint32_t flags);
extern __INLINE void sl_hal_vdac_set_interrupts(VDAC_TypeDef *vdac,
                                                uint32_t flags);
extern __INLINE void sl_hal_vdac_clear_interrupts(VDAC_TypeDef *vdac,
                                                  uint32_t flags);
extern __INLINE uint32_t sl_hal_vdac_get_pending_interrupts(VDAC_TypeDef *vdac);
extern __INLINE uint32_t sl_hal_vdac_get_enabled_pending_interrupts(VDAC_TypeDef *vdac);
extern __INLINE void sl_hal_vdac_sine_mode_start(VDAC_TypeDef *vdac);
extern __INLINE void sl_hal_vdac_sine_mode_stop(VDAC_TypeDef *vdac);
extern __INLINE uint32_t sl_hal_vdac_get_status(VDAC_TypeDef *vdac);

/***************************************************************************//**
 * Initialize VDAC.
 ******************************************************************************/
void sl_hal_vdac_init(VDAC_TypeDef *vdac,
                      const sl_hal_vdac_init_t *init)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  // Verifies that the initialization configuration pointer is not NULL.
  EFM_ASSERT(init != NULL);

  sl_hal_vdac_disable(vdac);
  sl_hal_vdac_wait_ready(vdac);

  vdac->CFG  = (vdac->CFG & ~(_VDAC_CFG_WARMUPTIME_MASK | _VDAC_CFG_DBGHALT_MASK | _VDAC_CFG_ONDEMANDCLK_MASK
  #if defined(VDAC_CFG_SINEMODEPRS)
                              | _VDAC_CFG_SINEMODEPRS_MASK
  #endif
  #if defined(VDAC_CFG_OUTENPRS)
                              | _VDAC_CFG_OUTENPRS_MASK
  #endif
                              | _VDAC_CFG_DMAWU_MASK
                              | _VDAC_CFG_BIASKEEPWARM_MASK
                              | _VDAC_CFG_REFRESHPERIOD_MASK
                              | _VDAC_CFG_TIMEROVRFLOWPERIOD_MASK
                              | _VDAC_CFG_PRESC_MASK
                              | _VDAC_CFG_REFRSEL_MASK
                              | _VDAC_CFG_CH0PRESCRST_MASK
                              | _VDAC_CFG_SINERESET_MASK
                              | _VDAC_CFG_SINEMODE_MASK
                              | _VDAC_CFG_DIFF_MASK))
#if defined(VDAC_CFG_SINEMODEPRS)
               | ((uint32_t)init->sine_mode_prs_enable << _VDAC_CFG_SINEMODEPRS_SHIFT)
#endif
#if defined(VDAC_CFG_OUTENPRS)
               | ((uint32_t)init->prs_output_enable    << _VDAC_CFG_OUTENPRS_SHIFT)
#endif
               | ((uint32_t)init->warmup_time          << _VDAC_CFG_WARMUPTIME_SHIFT)
               | ((uint32_t)init->debug_halt           << _VDAC_CFG_DBGHALT_SHIFT)
               | ((uint32_t)init->on_demand_clk        << _VDAC_CFG_ONDEMANDCLK_SHIFT)
               | ((uint32_t)init->dma_wakeup           << _VDAC_CFG_DMAWU_SHIFT)
               | ((uint32_t)init->bias_keep_warm       << _VDAC_CFG_BIASKEEPWARM_SHIFT)
               | ((uint32_t)init->refresh              << _VDAC_CFG_REFRESHPERIOD_SHIFT)
               | ((uint32_t)init->timer_overflow       << _VDAC_CFG_TIMEROVRFLOWPERIOD_SHIFT)
               | ((uint32_t)init->prescaler            << _VDAC_CFG_PRESC_SHIFT)
               | ((uint32_t)init->reference            << _VDAC_CFG_REFRSEL_SHIFT)
               | ((uint32_t)init->ch0_reset_prescaler  << _VDAC_CFG_CH0PRESCRST_SHIFT)
               | ((uint32_t)init->sine_reset           << _VDAC_CFG_SINERESET_SHIFT)
               | ((uint32_t)init->sine_enable          << _VDAC_CFG_SINEMODE_SHIFT)
               | ((uint32_t)init->diff                 << _VDAC_CFG_DIFF_SHIFT);
}

/***************************************************************************//**
 * Initialize a VDAC channel.
 ******************************************************************************/
void sl_hal_vdac_init_channel(VDAC_TypeDef *vdac,
                              const sl_hal_vdac_init_channel_t *init,
                              uint32_t channel)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  // Verifies that the initialization configuration pointer is not NULL.
  EFM_ASSERT(init != NULL);

  // Verifies that the channel is valid.
  EFM_ASSERT(SLI_HAL_VDAC_CH_VALID(channel));

  uint32_t channel_config;
  uint32_t vdac_status = vdac->STATUS;

  sl_hal_vdac_disable(vdac);
  sl_hal_vdac_wait_ready(vdac);

  channel_config = ((uint32_t)init->warmup_keep_on             << _VDAC_CH0CFG_KEEPWARM_SHIFT)
                   | ((uint32_t)init->high_cap_load_enable     << _VDAC_CH0CFG_HIGHCAPLOADEN_SHIFT)
                   | ((uint32_t)init->fifo_low_data_threshold  << _VDAC_CH0CFG_FIFODVL_SHIFT)
                   | ((uint32_t)init->ch_refresh_source        << _VDAC_CH0CFG_REFRESHSOURCE_SHIFT)
                   | ((uint32_t)init->trigger_mode             << _VDAC_CH0CFG_TRIGMODE_SHIFT)
                   | ((uint32_t)init->power_mode               << _VDAC_CH0CFG_POWERMODE_SHIFT)
                   | ((uint32_t)init->sample_off_mode          << _VDAC_CH0CFG_CONVMODE_SHIFT);

  if (channel == SL_HAL_VDAC_CHANNEL_0) {
    vdac->CH0CFG = channel_config;

    vdac->OUTTIMERCFG = (vdac->OUTTIMERCFG & ~_VDAC_OUTTIMERCFG_CH0OUTHOLDTIME_MASK)
                        | (init->hold_out_time << _VDAC_OUTTIMERCFG_CH0OUTHOLDTIME_SHIFT);

    sl_hal_vdac_enable(vdac);

    vdac->OUTCTRL = ((uint32_t)(vdac->OUTCTRL & ~(_VDAC_OUTCTRL_ABUSPINSELCH0_MASK | _VDAC_OUTCTRL_ABUSPORTSELCH0_MASK | _VDAC_OUTCTRL_SHORTCH0_MASK
                                                  | _VDAC_OUTCTRL_AUXOUTENCH0_MASK | _VDAC_OUTCTRL_MAINOUTENCH0_MASK)))

                    | ((uint32_t)init->pin              << _VDAC_OUTCTRL_ABUSPINSELCH0_SHIFT)
                    | ((uint32_t)init->port             << _VDAC_OUTCTRL_ABUSPORTSELCH0_SHIFT)
                    | ((uint32_t)init->short_output     << _VDAC_OUTCTRL_SHORTCH0_SHIFT)
                    | ((uint32_t)init->aux_out_enable   << _VDAC_OUTCTRL_AUXOUTENCH0_SHIFT)
                    | ((uint32_t)init->main_out_enable  << _VDAC_OUTCTRL_MAINOUTENCH0_SHIFT);
  } else {
    vdac->CH1CFG = channel_config;

    vdac->OUTTIMERCFG = (vdac->OUTTIMERCFG & ~_VDAC_OUTTIMERCFG_CH1OUTHOLDTIME_MASK)
                        | (init->hold_out_time << _VDAC_OUTTIMERCFG_CH1OUTHOLDTIME_SHIFT);

    sl_hal_vdac_enable(vdac);

    vdac->OUTCTRL = ((uint32_t)(vdac->OUTCTRL & ~(_VDAC_OUTCTRL_ABUSPINSELCH1_MASK | _VDAC_OUTCTRL_ABUSPORTSELCH1_MASK | _VDAC_OUTCTRL_SHORTCH1_MASK
                                                  | _VDAC_OUTCTRL_AUXOUTENCH1_MASK | _VDAC_OUTCTRL_MAINOUTENCH1_MASK)))

                    | ((uint32_t)init->pin             << _VDAC_OUTCTRL_ABUSPINSELCH1_SHIFT)
                    | ((uint32_t)init->port            << _VDAC_OUTCTRL_ABUSPORTSELCH1_SHIFT)
                    | ((uint32_t)init->short_output    << _VDAC_OUTCTRL_SHORTCH1_SHIFT)
                    | ((uint32_t)init->aux_out_enable  << _VDAC_OUTCTRL_AUXOUTENCH1_SHIFT)
                    | ((uint32_t)init->main_out_enable << _VDAC_OUTCTRL_MAINOUTENCH1_SHIFT);
  }

  // Check if the other channel had to be turned off above and needs to be turned on again.
  if (channel == SL_HAL_VDAC_CHANNEL_0) {
    if (vdac_status & VDAC_STATUS_CH0ENS) {
      // Enable the VDAC channel 0.
      vdac->CMD_SET = VDAC_CMD_CH0EN;
    }
  } else {
    if (vdac_status & VDAC_STATUS_CH1ENS) {
      // Enable the VDAC channel 1.
      vdac->CMD_SET = VDAC_CMD_CH1EN;
    }
  }
}

/***************************************************************************//**
 * Enable the VDAC channel.
 ******************************************************************************/
void sl_hal_vdac_enable_channel(VDAC_TypeDef *vdac,
                                uint32_t channel)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  // Verifies that the channel is valid.
  EFM_ASSERT(SLI_HAL_VDAC_CH_VALID(channel));

  sl_hal_vdac_wait_sync(vdac);

  if (channel == SL_HAL_VDAC_CHANNEL_0) {
    vdac->CMD_SET = VDAC_CMD_CH0EN;
    while ((vdac->STATUS & VDAC_STATUS_CH0ENS) == 0) {
      // Wait for channel 0 to be enabled.
    }
  } else {
    vdac->CMD_SET = VDAC_CMD_CH1EN;
    while ((vdac->STATUS & VDAC_STATUS_CH1ENS) == 0) {
      // Wait for channel 1 to be enabled.
    }
  }
}

/***************************************************************************//**
 * Disable the VDAC channel.
 ******************************************************************************/
void sl_hal_vdac_disable_channel(VDAC_TypeDef *vdac,
                                 uint32_t channel)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  // Verifies that the channel is valid.
  EFM_ASSERT(SLI_HAL_VDAC_CH_VALID(channel));

  sl_hal_vdac_wait_sync(vdac);
  if (channel == SL_HAL_VDAC_CHANNEL_0) {
    vdac->CMD_SET = VDAC_CMD_CH0DIS;
    sl_hal_vdac_wait_sync(vdac);
    vdac->CMD_SET = VDAC_CMD_CH0FIFOFLUSH;
  } else {
    vdac->CMD_SET = VDAC_CMD_CH1DIS;
    sl_hal_vdac_wait_sync(vdac);
    vdac->CMD_SET = VDAC_CMD_CH1FIFOFLUSH;
  }
  sl_hal_vdac_wait_sync(vdac);
}

/***************************************************************************//**
 * Set the output signal of a VDAC channel to a given value.
 ******************************************************************************/
void sl_hal_vdac_set_output_channel(VDAC_TypeDef *vdac,
                                    uint32_t channel,
                                    uint32_t value)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  // Verifies that the channel is valid.
  EFM_ASSERT(SLI_HAL_VDAC_CH_VALID(channel));

  switch (channel) {
    case SL_HAL_VDAC_CHANNEL_0:
      // Ensures the value does not exceed the maximum allowed for channel 0.
      EFM_ASSERT(value <= _VDAC_CH0F_MASK);
      vdac->CH0F = value;
      break;
    case SL_HAL_VDAC_CHANNEL_1:
      // Ensures the value does not exceed the maximum allowed for channel 1.
      EFM_ASSERT(value <= _VDAC_CH1F_MASK);
      vdac->CH1F = value;
      break;
    default:
      EFM_ASSERT(0);
      break;
  }
}

/***************************************************************************//**
 * Calculate the prescaler value used to determine VDAC clock.
 ******************************************************************************/
uint32_t sl_hal_vdac_calculate_prescaler(VDAC_TypeDef *vdac,
                                         uint32_t vdac_frequency,
                                         uint32_t reference_frequency)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  uint32_t prescaler = 0;

  // Make sure that the selected VDAC clock is below the maximum value.
  if (vdac_frequency > SLI_HAL_VDAC_MAX_CLOCK) {
    vdac_frequency = SLI_HAL_VDAC_MAX_CLOCK;
  }
  // Iterate to determine the best prescaler value. Start with the lowest
  // prescaler value to get the first equal or less VDAC
  // frequency value.
  for (prescaler = 0; prescaler <= (_VDAC_CFG_PRESC_MASK >> _VDAC_CFG_PRESC_SHIFT); prescaler++) {
    if ((reference_frequency / (prescaler + 1)) <= vdac_frequency) {
      break;
    }
  }
  // If prescaler is higher than the maximum prescaler value, make sure to return the maximum value.
  if (prescaler > (_VDAC_CFG_PRESC_MASK >> _VDAC_CFG_PRESC_SHIFT)) {
    prescaler = _VDAC_CFG_PRESC_MASK >> _VDAC_CFG_PRESC_SHIFT;
  }

  return prescaler;
}

/***************************************************************************//**
 * Reset VDAC to its default state.
 ******************************************************************************/
void sl_hal_vdac_reset(VDAC_TypeDef *vdac)
{
  // Verifies that the VDAC reference is valid.
  EFM_ASSERT(SL_HAL_VDAC_REF_VALID(vdac));

  sl_hal_vdac_wait_sync(vdac);

  // Disable both VDAC channels if VDAC is enabled.
  if (vdac->EN & VDAC_EN_EN) {
    sl_hal_vdac_disable_channel(vdac, SL_HAL_VDAC_CHANNEL_0);
    sl_hal_vdac_disable_channel(vdac, SL_HAL_VDAC_CHANNEL_1);
  }
  // Disable the VDAC module.
  sl_hal_vdac_disable(vdac);
  sl_hal_vdac_wait_ready(vdac);

  // Reset the VDAC.
  vdac->SWRST_SET = VDAC_SWRST_SWRST;
  while (vdac->SWRST & _VDAC_SWRST_RESETTING_MASK) {
    // Wait for the reset to finish.
  }
}

/** @} (end addtogroup vdac) */
#endif /* defined(VDAC_PRESENT) */
