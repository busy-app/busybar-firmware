/***************************************************************************//**
 * @file
 * @brief Watchdog (WDOG) peripheral API
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

#include "sl_hal_wdog.h"
#if defined(WDOG_COUNT) && (WDOG_COUNT > 0)

#include "sl_core.h"

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

extern __INLINE void sl_hal_wdog_lock(WDOG_TypeDef *wdog);
extern __INLINE void sl_hal_wdog_wait_sync(WDOG_TypeDef *wdog);
extern __INLINE void sl_hal_wdog_wait_ready(WDOG_TypeDef *wdog);
extern __INLINE void sl_hal_wdog_unlock(WDOG_TypeDef *wdog);
extern __INLINE void sl_hal_wdog_clear_interrupts(WDOG_TypeDef *wdog,
                                                  uint32_t flags);
extern __INLINE void sl_hal_wdog_disable_interrupts(WDOG_TypeDef *wdog,
                                                    uint32_t flags);
extern __INLINE void sl_hal_wdog_enable_interrupts(WDOG_TypeDef *wdog,
                                                   uint32_t flags);
extern __INLINE uint32_t sl_hal_wdog_get_pending_interrupts(WDOG_TypeDef *wdog);
extern __INLINE uint32_t sl_hal_wdog_get_enabled_pending_interrupts(WDOG_TypeDef *wdog);
extern __INLINE void sl_hal_wdog_set_interrupts(WDOG_TypeDef *wdog,
                                                uint32_t flags);
extern __INLINE bool sl_hal_wdog_is_enabled(WDOG_TypeDef *wdog);
extern __INLINE bool sl_hal_wdog_is_locked(WDOG_TypeDef *wdog);

/***************************************************************************//**
 * Enable the watchdog peripheral.
 ******************************************************************************/
void sl_hal_wdog_enable(WDOG_TypeDef *wdog)
{
  EFM_ASSERT(SL_HAL_WDOG_REF_VALID(wdog));

  // SYNCBUSY may stall when locked.
  if ((wdog->STATUS & _WDOG_STATUS_LOCK_MASK) == WDOG_STATUS_LOCK_LOCKED) {
    return;
  }

  wdog->EN_SET = WDOG_EN_EN;
}

/***************************************************************************//**
 * Disable the watchdog peripheral.
 ******************************************************************************/
void sl_hal_wdog_disable(WDOG_TypeDef *wdog)
{
  EFM_ASSERT(SL_HAL_WDOG_REF_VALID(wdog));

  // SYNCBUSY may stall when locked.
  if ((wdog->STATUS & _WDOG_STATUS_LOCK_MASK) == WDOG_STATUS_LOCK_LOCKED) {
    return;
  }

  sl_hal_wdog_wait_sync(wdog);

  wdog->EN_CLR = WDOG_EN_EN;
}

/***************************************************************************//**
 * Feed the watchdog peripheral.
 ******************************************************************************/
void sl_hal_wdog_feed(WDOG_TypeDef *wdog)
{
  EFM_ASSERT(SL_HAL_WDOG_REF_VALID(wdog));

  // WDOG should not be fed while it is disabled.
  if ((wdog->EN & WDOG_EN_EN) == 0U) {
    return;
  }

  // We need a restricted section around the check for sync and the clear command
  // because sending a clear command while a previous command is being synchronized
  // will cause a BusFault.
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  if ((wdog->SYNCBUSY & WDOG_SYNCBUSY_CMD) == 0U) {
    wdog->CMD = WDOG_CMD_CLEAR;
  }
  CORE_EXIT_ATOMIC();
}

/***************************************************************************//**
 * Initialize watchdog (assuming the watchdog configuration has not been
 * locked).
 ******************************************************************************/
void sl_hal_wdog_init(WDOG_TypeDef *wdog,
                      const sl_hal_wdog_init_t *init)
{
  EFM_ASSERT(SL_HAL_WDOG_REF_VALID(wdog));

  sl_hal_wdog_disable(wdog);
  sl_hal_wdog_wait_ready(wdog);

  wdog->CFG = (wdog->CFG & ~(_WDOG_CFG_DEBUGRUN_MASK | _WDOG_CFG_CLRSRC_MASK | _WDOG_CFG_EM4BLOCK_MASK
#if defined(_WDOG_CFG_EM1RUN_MASK)
                             | _WDOG_CFG_EM1RUN_MASK
#endif
#if defined(_WDOG_CFG_EM2RUN_MASK)
                             | _WDOG_CFG_EM2RUN_MASK
#endif
#if defined(_WDOG_CFG_EM3RUN_MASK)
                             | _WDOG_CFG_EM3RUN_MASK
#endif
                             | _WDOG_CFG_PRS0MISSRSTEN_MASK | _WDOG_CFG_PRS1MISSRSTEN_MASK | _WDOG_CFG_WDOGRSTDIS_MASK
                             | _WDOG_CFG_WARNSEL_MASK | _WDOG_CFG_WINSEL_MASK | _WDOG_CFG_PERSEL_MASK))

              | (((uint32_t)init->debug_run) << _WDOG_CFG_DEBUGRUN_SHIFT)
              | (((uint32_t)init->clear_source) << _WDOG_CFG_CLRSRC_SHIFT)
#if defined(_WDOG_CFG_EM1RUN_MASK)
              | (((uint32_t)init->em1_run) << _WDOG_CFG_EM1RUN_SHIFT)
#endif
#if defined(_WDOG_CFG_EM2RUN_MASK)
              | (((uint32_t)init->em2_run) << _WDOG_CFG_EM2RUN_SHIFT)
#endif
#if defined(_WDOG_CFG_EM3RUN_MASK)
              | (((uint32_t)init->em3_run) << _WDOG_CFG_EM3RUN_SHIFT)
#endif
              | (((uint32_t)init->em4_block) << _WDOG_CFG_EM4BLOCK_SHIFT)
              | (((uint32_t)init->prs0_missing_reset_enable) << _WDOG_CFG_PRS0MISSRSTEN_SHIFT)
              | (((uint32_t)init->prs1_missing_reset_enable) << _WDOG_CFG_PRS1MISSRSTEN_SHIFT)
              | (((uint32_t)init->reset_disable)  << _WDOG_CFG_WDOGRSTDIS_SHIFT)
              | (((uint32_t)init->warning_time_select) << _WDOG_CFG_WARNSEL_SHIFT)
              | (((uint32_t)init->window_time_select) << _WDOG_CFG_WINSEL_SHIFT)
              | (((uint32_t)init->period_select) << _WDOG_CFG_PERSEL_SHIFT);

  if (init->lock) {
    sl_hal_wdog_lock(wdog);
    sl_hal_wdog_wait_sync(wdog);
  }
}

#endif /* defined(WDOG_COUNT) && (WDOG_COUNT > 0) */
