/***************************************************************************//**
 * @file
 * @brief Back-Up Real Time Counter (BURTC) module peripheral API
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

#include "sl_hal_burtc.h"

#if defined(BURTC_PRESENT)

#include <stddef.h>

#include "sl_assert.h"
#include "sl_common.h"

/***************************************************************************//**
 * @addtogroup burtc BURTC - Back-Up Real Time Counter
 * @{
 ******************************************************************************/
extern __INLINE void sl_hal_burtc_clear_interrupts(uint32_t flags);
extern __INLINE void sl_hal_burtc_disable_interrupts(uint32_t flags);
extern __INLINE void sl_hal_burtc_enable_interrupts(uint32_t flags);
extern __INLINE uint32_t sl_hal_burtc_get_pending_interrupts(void);
extern __INLINE uint32_t sl_hal_burtc_get_enabled_pending_interrupts(void);
extern __INLINE void sl_hal_burtc_set_interrupts(uint32_t flags);
extern __INLINE uint32_t sl_hal_burtc_get_status(void);
extern __INLINE void sl_hal_burtc_wait_sync(void);
extern __INLINE void sl_hal_burtc_start(void);
extern __INLINE void sl_hal_burtc_stop(void);
extern __INLINE uint32_t sl_hal_burtc_get_counter(void);
extern __INLINE void sl_hal_burtc_lock(void);
extern __INLINE void sl_hal_burtc_unlock(void);

/***************************************************************************//**
 * @brief Convert dividend to a prescaler logarithmic value. Only works for even
 *        numbers equal to 2^n.
 *
 * @param[in] div Unscaled dividend.
 *
 * @return Base 2 logarithm of input, as used by fixed prescalers.
 ******************************************************************************/
__STATIC_INLINE uint32_t div_to_log2(uint32_t div)
{
  uint32_t log2;

  /* Prescaler accepts an argument of 32768 or less, valid values being 2^n. */
  EFM_ASSERT((div > 0UL) && (div <= 32768UL));

  /* Count leading zeroes and "reverse" result, Cortex-M3 intrinsic. */
  log2 = (31UL - __CLZ(div));

  return log2;
}

/***************************************************************************//**
 * @brief
 *   Initialize the Back-Up RTC.
 ******************************************************************************/
void sl_hal_burtc_init(const sl_hal_burtc_init_t *init)
{
  EFM_ASSERT(init != NULL);

  uint32_t presc;

  presc = div_to_log2(init->clock_divider);

  sl_hal_burtc_disable();

  BURTC->CFG = (presc << _BURTC_CFG_CNTPRESC_SHIFT)
               | ((init->compare0_top ? 1UL : 0UL) << _BURTC_CFG_COMPTOP_SHIFT)
               | ((init->debug_run ? 1UL : 0UL) << _BURTC_CFG_DEBUGRUN_SHIFT);

  BURTC->EM4WUEN = ((init->em4_comparator ? 1UL : 0UL) << _BURTC_EM4WUEN_COMPEM4WUEN_SHIFT)
                   | ((init->em4_overflow ? 1UL : 0UL) << _BURTC_EM4WUEN_OFEM4WUEN_SHIFT);
}

/***************************************************************************//**
 * @brief
 *   Reset the Back-Up RTC.
 ******************************************************************************/
void sl_hal_burtc_reset(void)
{
  if (BURTC->EN != 0U) {
    sl_hal_burtc_wait_sync();
  }

  sl_hal_burtc_enable();

  sl_hal_burtc_stop();

  BURTC->CNT     = 0x0;
  BURTC->PRECNT  = 0x0;
  BURTC->COMP    = 0x0;
  BURTC->EM4WUEN = _BURTC_EM4WUEN_RESETVALUE;
  BURTC->IEN     = _BURTC_IEN_RESETVALUE;
  BURTC->IF_CLR  = _BURTC_IF_MASK;

  sl_hal_burtc_disable();

  BURTC->CFG = _BURTC_CFG_RESETVALUE;
}

/***************************************************************************//**
 * @brief
 *   Enable the Back up RTC.
 ******************************************************************************/
void sl_hal_burtc_enable(void)
{
  if (BURTC->EN == 1U) {
    /* Trying to enable BURTC when it's already enabled */
    return;
  }

  BURTC->EN_SET = BURTC_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Disable the Back up RTC.
 ******************************************************************************/
void sl_hal_burtc_disable(void)
{
  if (BURTC->EN == 0U) {
    /* Trying to disable BURTC when it's already disabled */
    return;
  }

  sl_hal_burtc_wait_sync();

  sl_hal_burtc_stop();
  sl_hal_burtc_wait_sync(); /* Wait for the stop to synchronize */

  BURTC->EN_CLR = BURTC_EN_EN;

#if defined(_BURTC_EN_DISABLING_MASK)
  while (BURTC->EN & _BURTC_EN_DISABLING_MASK) {
    /* Wait for disabling to finish */
  }
#else
  while (BURTC->SYNCBUSY != 0U) {
    /* Wait for disabling to finish */
  }
#endif
}

/***************************************************************************//**
 * @brief
 *   Reset the Back-Up RTC counter.
 ******************************************************************************/
void sl_hal_burtc_reset_counter(void)
{
  sl_hal_burtc_stop();
  BURTC->CNT = 0U;
  sl_hal_burtc_start();
}

/***************************************************************************//**
 * @brief
 *   Set the Back-Up RTC compare register.
 ******************************************************************************/
void sl_hal_burtc_set_compare(uint32_t value)
{
  /* Wait for last potential write to complete. */
  while (BURTC->SYNCBUSY & BURTC_SYNCBUSY_COMP) {
    /* Wait last writing of COMP is synchronizing to LF clock. */
  }

  /* Configure compare channel 0 */
  BURTC->COMP = value;

  while (BURTC->SYNCBUSY & BURTC_SYNCBUSY_COMP) {
    /* Wait last writing of COMP is synchronizing to LF clock. */
  }
}

/***************************************************************************//**
 * @brief
 *   Get the Back-Up RTC compare register value.
 ******************************************************************************/
uint32_t sl_hal_burtc_get_compare(void)
{
  return BURTC->COMP;
}

/** @} (end addtogroup BURTC) */
#endif /* defined( BURTC_PRESENT ) */
