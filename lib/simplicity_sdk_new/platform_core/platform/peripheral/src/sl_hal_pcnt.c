/***************************************************************************//**
 * @file  sl_hal_pcnt.c
 *
 * @brief Pulse Counter (PCNT) peripheral API
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

#include "sl_hal_pcnt.h"
#if defined(PCNT_COUNT) && (PCNT_COUNT > 0)

/***************************************************************************//**
 * @addtogroup pcnt PCNT - Pulse Counter
 * @brief Pulse Counter (PCNT) Peripheral API
 * @details
 *  This module contains functions to control the PCNT peripheral of Silicon
 *  Labs 32-bit MCUs and SoCs. The PCNT decodes incoming pulses. The module has
 *  a quadrature mode which may be used to decode the speed and direction of a
 *  mechanical shaft.
 * @{
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
extern __INLINE bool sl_hal_pcnt_is_enabled(PCNT_TypeDef *pcnt);
extern __INLINE void sl_hal_pcnt_enable(PCNT_TypeDef *pcnt);
extern __INLINE void sl_hal_pcnt_disable(PCNT_TypeDef *pcnt);
extern __INLINE void sl_hal_pcnt_wait_ready(PCNT_TypeDef *pcnt);
extern __INLINE void sl_hal_pcnt_wait_sync(PCNT_TypeDef *pcnt);
extern __INLINE uint32_t sl_hal_pcnt_get_status(PCNT_TypeDef *pcnt);
extern __INLINE void sl_hal_pcnt_enable_interrupts(PCNT_TypeDef *pcnt,
                                                   uint32_t flags);
extern __INLINE void sl_hal_pcnt_disable_interrupts(PCNT_TypeDef *pcnt,
                                                    uint32_t flags);
extern __INLINE void sl_hal_pcnt_clear_interrupts(PCNT_TypeDef *pcnt,
                                                  uint32_t flags);
extern __INLINE void sl_hal_pcnt_set_interrupts(PCNT_TypeDef *pcnt,
                                                uint32_t flags);
extern __INLINE uint32_t sl_hal_pcnt_get_pending_interrupts(PCNT_TypeDef *pcnt);
extern __INLINE uint32_t sl_hal_pcnt_get_enabled_pending_interrupts(PCNT_TypeDef *pcnt);
extern __INLINE void sl_hal_pcnt_lock(PCNT_TypeDef *pcnt);
extern __INLINE void sl_hal_pcnt_unlock(PCNT_TypeDef *pcnt);
extern __INLINE void sl_hal_pcnt_set_top_buffer(PCNT_TypeDef *pcnt,
                                                uint32_t value);
extern __INLINE uint32_t sl_hal_pcnt_get_top_buffer(PCNT_TypeDef *pcnt);
extern __INLINE void sl_hal_pcnt_set_top(PCNT_TypeDef *pcnt,
                                         uint32_t value);
extern __INLINE uint32_t sl_hal_pcnt_get_top(PCNT_TypeDef *pcnt);
extern __INLINE void sl_hal_pcnt_start_main_counter(PCNT_TypeDef *pcnt);
extern __INLINE void sl_hal_pcnt_stop_main_counter(PCNT_TypeDef *pcnt);
extern __INLINE uint32_t sl_hal_pcnt_get_main_counter(PCNT_TypeDef *pcnt);
extern __INLINE void sl_hal_pcnt_start_aux_counter(PCNT_TypeDef *pcnt);
extern __INLINE void sl_hal_pcnt_stop_aux_counter(PCNT_TypeDef *pcnt);
extern __INLINE uint32_t sl_hal_pcnt_get_aux_counter(PCNT_TypeDef *pcnt);
extern __INLINE void sl_hal_pcnt_reset_counters(PCNT_TypeDef *pcnt);

/***************************************************************************//**
 * @brief
 *   Initialize PCNT.
 ******************************************************************************/
void sl_hal_pcnt_init(PCNT_TypeDef *pcnt,
                      const sl_hal_pcnt_init_t *init)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  // Disable PCNT.
  sl_hal_pcnt_disable(pcnt);
  sl_hal_pcnt_wait_ready(pcnt);

  // Write the CFG register with the configurations.
  pcnt->CFG = (pcnt->CFG & ~(_PCNT_CFG_MODE_MASK | _PCNT_CFG_DEBUGHALT_MASK | _PCNT_CFG_FILTEN_MASK
                             | _PCNT_CFG_HYST_MASK | _PCNT_CFG_S0PRSEN_MASK | _PCNT_CFG_S1PRSEN_MASK))
              | (((uint32_t)init->mode) << _PCNT_CFG_MODE_SHIFT)
              | (((uint32_t)init->debug_halt) << _PCNT_CFG_DEBUGHALT_SHIFT)
              | (((uint32_t)init->filter_enable) << _PCNT_CFG_FILTEN_SHIFT)
              | (((uint32_t)init->hysteresis_enable) << _PCNT_CFG_HYST_SHIFT)
              | (((uint32_t)init->s0_prs_enable) << _PCNT_CFG_S0PRSEN_SHIFT)
              | (((uint32_t)init->s1_prs_enable) << _PCNT_CFG_S1PRSEN_SHIFT);

  sl_hal_pcnt_enable(pcnt);

  // Write the CTRL register with the configurations.
  pcnt->CTRL = (pcnt->CTRL & ~(_PCNT_CTRL_S1CDIR_MASK | _PCNT_CTRL_CNTDIR_MASK | _PCNT_CTRL_EDGE_MASK
                               | _PCNT_CTRL_CNTEV_MASK | _PCNT_CTRL_AUXCNTEV_MASK))
               | (((uint32_t)init->s1_count_direction) << _PCNT_CTRL_S1CDIR_SHIFT)
               | (((uint32_t)init->count_down) << _PCNT_CTRL_CNTDIR_SHIFT)
               | (((uint32_t)init->negative_edge) << _PCNT_CTRL_EDGE_SHIFT)
               | (((uint32_t)init->main_count_event) << _PCNT_CTRL_CNTEV_SHIFT)
               | (((uint32_t)init->aux_count_event) << _PCNT_CTRL_AUXCNTEV_SHIFT);

  // Write the OVSCTRL register with the configurations.
  pcnt->OVSCTRL = (pcnt->OVSCTRL & ~(_PCNT_OVSCTRL_FLUTTERRM_MASK | _PCNT_OVSCTRL_FILTLEN_MASK))
                  | (((uint32_t)init->flutter_remove) << _PCNT_OVSCTRL_FLUTTERRM_SHIFT)
                  | (((uint32_t)init->filter_lenght) << _PCNT_OVSCTRL_FILTLEN_SHIFT);

  if (init->mode == SL_HAL_PCNT_MODE_EXT_CLK_SINGLE || init->mode == SL_HAL_PCNT_MODE_EXT_CLK_QUAD) {
    // Enable PCNT Clock Domain Reset. The PCNT must be in reset before changing
    // the clock source to an external clock.
    pcnt->CMD_SET = PCNT_CMD_CORERST;
    CMU->PCNT0CLKCTRL = CMU_PCNT0CLKCTRL_CLKSEL_PCNTS0;
  } else {
    CMU->PCNT0CLKCTRL = CMU_PCNT0CLKCTRL_CLKSEL_EM23GRPACLK;
  }

  // Disable PCNT.
  sl_hal_pcnt_disable(pcnt);
  sl_hal_pcnt_wait_ready(pcnt);
}

/***************************************************************************//**
 * @brief
 *   Reset the PCNT to the same state that it was in after a hardware reset.
 ******************************************************************************/
void sl_hal_pcnt_reset(PCNT_TypeDef *pcnt)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));

  // Disable PCNT.
  sl_hal_pcnt_wait_sync(pcnt);
  sl_hal_pcnt_disable(pcnt);

  // Recommended to switch to internal clock before reset.
  CMU->PCNT0CLKCTRL = CMU_PCNT0CLKCTRL_CLKSEL_EM23GRPACLK;

  sl_hal_pcnt_wait_ready(pcnt);
  pcnt->SWRST_SET = PCNT_SWRST_SWRST;
}

/***************************************************************************//**
 * @brief
 *   Set a counter value.
 ******************************************************************************/
void sl_hal_pcnt_set_main_counter(PCNT_TypeDef *pcnt,
                                  uint32_t value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_PCNT_REF_VALID(pcnt));
  // Make sure counter value is valid.
  EFM_ASSERT(value <= SL_HAL_PCNT_MAX_COUNT(pcnt));
  // Make sure module is enabled.
  EFM_ASSERT(pcnt->EN & _PCNT_EN_EN_MASK);

  uint32_t top = pcnt->TOP;
  if (top != value) {
    sl_hal_pcnt_set_top(pcnt, value);
  }

  sl_hal_pcnt_wait_sync(pcnt);
  pcnt->CMD_SET = PCNT_CMD_LCNTIM;

  if (top != value) {
    sl_hal_pcnt_set_top(pcnt, top);
  }
}

/// @} (end addtogroup pcnt)
#endif /// defined(PCNT_COUNT) && (PCNT_COUNT > 0)
