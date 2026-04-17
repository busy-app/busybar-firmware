/***************************************************************************//**
 * @file
 * @brief PIXELRZ Peripheral API
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

#include "sl_hal_pixelrz.h"
#if defined(PIXELRZ_COUNT) && (PIXELRZ_COUNT > 0)

/***************************************************************************//**
 * @addtogroup pixelrz PIXELRZ
 * @brief PIXELRZ Peripheral API
 * @details
 *   The PIXELRZ module consists of these main parts:
 *   @li General PIXELRZ configuration and enable control.
 *   @li Control functions that can be used when module is enabled.
 *   @li Interrupts handling.
 * @{
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
extern __INLINE void sl_hal_pixelrz_wait_ready(PIXELRZ_TypeDef *pixelrz);
extern __INLINE void sl_hal_pixelrz_wait_sync(PIXELRZ_TypeDef *pixelrz);
extern __INLINE void sl_hal_pixelrz_enable(PIXELRZ_TypeDef *pixelrz);
extern __INLINE void sl_hal_pixelrz_disable(PIXELRZ_TypeDef *pixelrz);
extern __INLINE void sl_hal_pixelrz_tx(PIXELRZ_TypeDef *pixelrz, uint32_t data);
extern __INLINE void sl_hal_pixelrz_enable_tx(PIXELRZ_TypeDef *pixelrz);
extern __INLINE void sl_hal_pixelrz_disable_tx(PIXELRZ_TypeDef *pixelrz);
extern __INLINE void sl_hal_pixelrz_clear_tx(PIXELRZ_TypeDef *pixelrz);
extern __INLINE void sl_hal_pixelrz_reset(PIXELRZ_TypeDef *pixelrz);
extern __INLINE uint32_t sl_hal_pixelrz_get_status(PIXELRZ_TypeDef *pixelrz);
extern __INLINE void sl_hal_pixelrz_clear_interrupts(PIXELRZ_TypeDef *pixelrz,
                                                     uint32_t flags);
extern __INLINE void sl_hal_pixelrz_set_interrupts(PIXELRZ_TypeDef *pixelrz,
                                                   uint32_t flags);
extern __INLINE void sl_hal_pixelrz_disable_interrupts(PIXELRZ_TypeDef *pixelrz,
                                                       uint32_t flags);
extern __INLINE void sl_hal_pixelrz_enable_interrupts(PIXELRZ_TypeDef *pixelrz,
                                                      uint32_t flags);
extern __INLINE uint32_t sl_hal_pixelrz_get_pending_interrupts(PIXELRZ_TypeDef *pixelrz);
extern __INLINE uint32_t sl_hal_pixelrz_get_enabled_interrupts(PIXELRZ_TypeDef *pixelrz);
extern __INLINE uint32_t sl_hal_pixelrz_get_enabled_pending_interrupts(PIXELRZ_TypeDef *pixelrz);

/***************************************************************************//**
 * @brief
 *   Initialize PIXELRZ.
 ******************************************************************************/
void sl_hal_pixelrz_init(PIXELRZ_TypeDef *pixelrz,
                         const sl_hal_pixelrz_init_t *init)
{
  EFM_ASSERT(SL_HAL_PIXELRZ_REF_VALID(pixelrz));
  // Init structure must be provided.
  EFM_ASSERT(init);

  sl_hal_pixelrz_reset(pixelrz);
  sl_hal_pixelrz_wait_ready(pixelrz);

  // Set config register.
  pixelrz->CFG = ((uint32_t)init->reset_symbol_enable << _PIXELRZ_CFG_RESETEN_SHIFT)
                 | ((uint32_t)init->stop_symbol_enable << _PIXELRZ_CFG_STOPEN_SHIFT)
                 | ((uint32_t)init->trigger_mode << _PIXELRZ_CFG_TRIGSEL_SHIFT)
                 | ((uint32_t)init->mode << _PIXELRZ_CFG_MODE_SHIFT)
                 | ((uint32_t)init->msb_first_enable << _PIXELRZ_CFG_MSBF_SHIFT)
                 | ((uint32_t)init->memalign_32b_enable << _PIXELRZ_CFG_MEMALIGN_SHIFT)
                 | ((uint32_t)init->prescaler << _PIXELRZ_CFG_DIV_SHIFT)
                 | ((uint32_t)init->fifo_watermark_level << _PIXELRZ_CFG_TXFWL_SHIFT)
                 | ((uint32_t)init->jitter_enable << _PIXELRZ_CFG_JITTEREN_SHIFT)
                 | ((uint32_t)init->output_invert_enable << _PIXELRZ_CFG_OUTINV_SHIFT);

  // Set frame config register.
  pixelrz->FRAMECFG = ((uint32_t)init->pixel_number << _PIXELRZ_FRAMECFG_PIXELNUM_SHIFT)
                      | ((uint32_t)init->pixel_width << _PIXELRZ_FRAMECFG_PIXELWIDTH_SHIFT)
                      | ((uint32_t)init->trail_data_width  << _PIXELRZ_FRAMECFG_TRAILWIDTH_SHIFT);

  pixelrz->T0CFG = ((uint32_t)init->zero_symbol.high_time << _PIXELRZ_T0CFG_HIGH_SHIFT)
                   | ((uint32_t)init->zero_symbol.low_time << _PIXELRZ_T0CFG_LOW_SHIFT);
  pixelrz->T1CFG = ((uint32_t)init->one_symbol.high_time << _PIXELRZ_T1CFG_HIGH_SHIFT)
                   | ((uint32_t)init->one_symbol.low_time << _PIXELRZ_T1CFG_LOW_SHIFT);
  pixelrz->TSTOPCFG = ((uint32_t)init->stop_symbol.high_time << _PIXELRZ_TSTOPCFG_HIGH_SHIFT)
                      | ((uint32_t)init->stop_symbol.low_time << _PIXELRZ_TSTOPCFG_LOW_SHIFT);
  pixelrz->TRSTCFG = ((uint32_t)init->reset_symbol.high_time << _PIXELRZ_TRSTCFG_HIGH_SHIFT)
                     | ((uint32_t)init->reset_symbol.low_time << _PIXELRZ_TRSTCFG_LOW_SHIFT);
}

/***************************************************************************//**
 * @brief
 *   Get symbol from time settings. Check the datasheet of addressable for timing
 *   configuration.
 ******************************************************************************/
sl_hal_pixelrz_symbol_t sl_hal_pixelrz_get_symbol_configuration(float time_high_us,
                                                                float time_low_us,
                                                                uint32_t freq)
{
  sl_hal_pixelrz_symbol_t symbol;
  symbol.high_time = (uint16_t)(time_high_us * (float)freq / 1000000.0f);
  symbol.low_time = (uint16_t)(time_low_us * (float)freq / 1000000.0f);

  return symbol;
}

/** @} (end addtogroup pixelrz) */
#endif /* defined(PIXELRZ_COUNT) && (PIXELRZ_COUNT > 0) */
