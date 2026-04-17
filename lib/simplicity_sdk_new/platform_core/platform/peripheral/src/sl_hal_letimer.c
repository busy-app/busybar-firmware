/***************************************************************************//**
 * @file  sl_hal_letimer.c
 *
 * @brief Low Energy Timer (LETIMER) peripheral API
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

#include "sl_hal_letimer.h"
#if defined(LETIMER_COUNT) && (LETIMER_COUNT > 0)

/***************************************************************************//**
 * @addtogroup letimer LETIMER - Low Energy Timer
 * @brief Low Energy Timer (LETIMER) Peripheral API
 * @details
 *  This module contains functions to control the LETIMER peripheral of Silicon
 *  Labs 32-bit MCUs and SoCs. The LETIMER is a down-counter that can keep track
 *  of time and output configurable waveforms.
 * @{
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
extern __INLINE void sl_hal_letimer_enable(LETIMER_TypeDef *letimer);
extern __INLINE void sl_hal_letimer_disable(LETIMER_TypeDef *letimer);
extern __INLINE void sl_hal_letimer_wait_ready(LETIMER_TypeDef *letimer);
extern __INLINE void sl_hal_letimer_wait_sync(LETIMER_TypeDef *letimer);
extern __INLINE uint32_t sl_hal_letimer_get_status(LETIMER_TypeDef *letimer);
extern __INLINE void sl_hal_letimer_enable_interrupts(LETIMER_TypeDef *letimer,
                                                      uint32_t flags);
extern __INLINE void sl_hal_letimer_disable_interrupts(LETIMER_TypeDef *letimer,
                                                       uint32_t flags);
extern __INLINE void sl_hal_letimer_clear_interrupts(LETIMER_TypeDef *letimer,
                                                     uint32_t flags);
extern __INLINE void sl_hal_letimer_set_interrupts(LETIMER_TypeDef *letimer,
                                                   uint32_t flags);
extern __INLINE uint32_t sl_hal_letimer_get_pending_interrupts(LETIMER_TypeDef *letimer);
extern __INLINE uint32_t sl_hal_letimer_get_enabled_pending_interrupts(LETIMER_TypeDef *letimer);
#if defined(_LETIMER_LOCK_MASK)
extern __INLINE void sl_hal_letimer_lock(LETIMER_TypeDef *letimer);
extern __INLINE void sl_hal_letimer_unlock(LETIMER_TypeDef *letimer);
#endif
extern __INLINE void sl_hal_letimer_start(LETIMER_TypeDef *letimer);
extern __INLINE void sl_hal_letimer_stop(LETIMER_TypeDef *letimer);
extern __INLINE void sl_hal_letimer_set_counter(LETIMER_TypeDef *letimer,
                                                uint32_t value);
extern __INLINE uint32_t sl_hal_letimer_get_counter(LETIMER_TypeDef *letimer);
extern __INLINE void sl_hal_letimer_set_top(LETIMER_TypeDef *letimer,
                                            uint32_t value);
extern __INLINE uint32_t sl_hal_letimer_get_top(LETIMER_TypeDef *letimer);
extern __INLINE void sl_hal_letimer_set_top_buffer(LETIMER_TypeDef *letimer,
                                                   uint32_t value);
extern __INLINE uint32_t sl_hal_letimer_get_top_buffer(LETIMER_TypeDef *letimer);

/***************************************************************************//**
 * @brief
 *   Initialize LETIMER.
 ******************************************************************************/
void sl_hal_letimer_init(LETIMER_TypeDef *letimer,
                         const sl_hal_letimer_init_t *init)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

  sl_hal_letimer_enable(letimer);
  sl_hal_letimer_wait_sync(letimer);

  // Write the CTRL register with the configurations.
  letimer->CTRL = (letimer->CTRL & ~(_LETIMER_CTRL_CNTPRESC_MASK | _LETIMER_CTRL_REPMODE_MASK
                                     | _LETIMER_CTRL_UFOA0_MASK | _LETIMER_CTRL_UFOA1_MASK
                                     | _LETIMER_CTRL_DEBUGRUN_MASK | _LETIMER_CTRL_CNTTOPEN_MASK
                                     | _LETIMER_CTRL_BUFTOP_MASK | _LETIMER_CTRL_OPOL0_MASK
                                     | _LETIMER_CTRL_OPOL1_MASK))
                  | (((uint32_t)init->prescaler) << _LETIMER_CTRL_CNTPRESC_SHIFT)
                  | (((uint32_t)init->repeat_mode) << _LETIMER_CTRL_REPMODE_SHIFT)
                  | (((uint32_t)init->underflow_output0_action) << _LETIMER_CTRL_UFOA0_SHIFT)
                  | (((uint32_t)init->underflow_output1_action) << _LETIMER_CTRL_UFOA1_SHIFT)
                  | (((uint32_t)init->debug_run) << _LETIMER_CTRL_DEBUGRUN_SHIFT)
                  | (((uint32_t)init->enable_top) << _LETIMER_CTRL_CNTTOPEN_SHIFT)
                  | (((uint32_t)init->buffered_top) << _LETIMER_CTRL_BUFTOP_SHIFT)
                  | (((uint32_t)init->output0_polarity) << _LETIMER_CTRL_OPOL0_SHIFT)
                  | (((uint32_t)init->output1_polarity) << _LETIMER_CTRL_OPOL1_SHIFT);

  sl_hal_letimer_disable(letimer);
  sl_hal_letimer_wait_ready(letimer);
}

/***************************************************************************//**
 * @brief
 *   Reset the LETIMER to the same state that it was in after a hardware reset.
 ******************************************************************************/
void sl_hal_letimer_reset(LETIMER_TypeDef *letimer)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

#if defined(_LETIMER_SWRST_MASK)
  letimer->SWRST_SET = LETIMER_SWRST_SWRST;
#else
  sl_hal_letimer_enable(letimer);
  sl_hal_letimer_wait_sync(letimer);

  // Make sure stopped first, before resetting other registers.
  letimer->CMD     = LETIMER_CMD_STOP | LETIMER_CMD_CLEAR
                     | LETIMER_CMD_CTO0 | LETIMER_CMD_CTO1;
  sl_hal_letimer_wait_sync(letimer);
  letimer->CTRL    = _LETIMER_CTRL_RESETVALUE;
  letimer->CNT     = _LETIMER_CNT_RESETVALUE;
  letimer->COMP0   = _LETIMER_COMP0_RESETVALUE;
  letimer->COMP1   = _LETIMER_COMP1_RESETVALUE;
  letimer->REP0    = _LETIMER_REP0_RESETVALUE;
  letimer->REP1    = _LETIMER_REP1_RESETVALUE;
  letimer->TOP     = _LETIMER_TOP_RESETVALUE;
  letimer->TOPBUFF = _LETIMER_TOPBUFF_RESETVALUE;
  letimer->IF      = _LETIMER_IF_RESETVALUE;
  letimer->IEN     = _LETIMER_IEN_RESETVALUE;
  letimer->PRSMODE = _LETIMER_PRSMODE_RESETVALUE;

  sl_hal_letimer_wait_sync(letimer);
  sl_hal_letimer_disable(letimer);
#endif
}

/***************************************************************************//**
 * @brief
 *   Set the LETIMER compare register value.
 ******************************************************************************/
void sl_hal_letimer_set_compare(LETIMER_TypeDef *letimer,
                                uint8_t channel,
                                uint32_t value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));
  // Make sure compare value is valid.
  EFM_ASSERT(value <= SL_HAL_LETIMER_MAX_COUNT(letimer));
  // Make sure module is enabled.
  EFM_ASSERT(letimer->EN & _LETIMER_EN_EN_MASK);

  sl_hal_letimer_wait_sync(letimer);

  switch (channel) {
    case 0:
      letimer->COMP0 = value;
      break;
    case 1:
      letimer->COMP1 = value;
      break;
    default:
      // Asserts if the channel doesn't exist
      EFM_ASSERT(false);
  }
}

/***************************************************************************//**
 * @brief
 *   Get the LETIMER compare register value.
 ******************************************************************************/
uint32_t sl_hal_letimer_get_compare(LETIMER_TypeDef *letimer,
                                    uint8_t channel)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

  uint32_t return_value = 0;

  sl_hal_letimer_wait_sync(letimer);

  switch (channel) {
    case 0:
      return_value = letimer->COMP0;
      break;
    case 1:
      return_value = letimer->COMP1;
      break;
    default:
      // Asserts if the channel doesn't exist
      EFM_ASSERT(false);
  }
  return return_value;
}

/***************************************************************************//**
 * @brief
 *   Set the LETIMER repeat register value.
 ******************************************************************************/
void sl_hal_letimer_set_repeat(LETIMER_TypeDef *letimer,
                               uint8_t channel,
                               uint32_t value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));
  // Make sure repeat value is valid.
  EFM_ASSERT((value & ~(_LETIMER_REP0_REP0_MASK >> _LETIMER_REP0_REP0_SHIFT)) == 0);
  // Make sure module is enabled.
  EFM_ASSERT(letimer->EN & _LETIMER_EN_EN_MASK);

  sl_hal_letimer_wait_sync(letimer);

  switch (channel) {
    case 0:
      letimer->REP0 = value;
      break;
    case 1:
      letimer->REP1 = value;
      break;
    default:
      // Asserts if the channel doesn't exist
      EFM_ASSERT(false);
  }
}

/***************************************************************************//**
 * @brief
 *   Get the LETIMER repeat register value.
 ******************************************************************************/
uint32_t sl_hal_letimer_get_repeat(LETIMER_TypeDef *letimer,
                                   uint8_t channel)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_LETIMER_REF_VALID(letimer));

  uint32_t return_value = 0;

  sl_hal_letimer_wait_sync(letimer);

  switch (channel) {
    case 0:
      return_value = letimer->REP0;
      break;
    case 1:
      return_value = letimer->REP1;
      break;
    default:
      // Asserts if the channel doesn't exist
      EFM_ASSERT(false);
  }
  return return_value;
}

/// @} (end addtogroup letimer)
#endif /// defined(LETIMER_COUNT) && (LETIMER_COUNT > 0)
