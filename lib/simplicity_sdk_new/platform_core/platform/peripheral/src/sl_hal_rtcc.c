/***************************************************************************//**
 * @file  sl_hal_rtcc.c
 * @brief Real Time Clock with Capture (RTCC) Peripheral API
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

#include "sl_hal_rtcc.h"

#if defined(RTCC_COUNT) && (RTCC_COUNT == 1)

#include <stddef.h>
/***************************************************************************//**
 * @addtogroup rtcc RTCC - Real Timer Counter/Calendar
 *
 * @brief Real Time Counter and Calendar (RTCC) Peripheral API
 *
 * @details
 *  This module contains functions to control the RTCC peripheral of Silicon
 *  Labs 32-bit MCUs and SoCs. The RTCC ensures timekeeping in low energy modes.
 *  The RTCC also includes a BCD calendar mode for easy time and date keeping.
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
extern __INLINE uint32_t sl_hal_rtcc_get_channel_compare_value(uint32_t channel);
extern __INLINE void sl_hal_rtcc_set_channel_compare_value(uint32_t channel,
                                                           uint32_t value);
extern __INLINE uint32_t sl_hal_rtcc_get_channel_capture_value(uint32_t channel);
extern __INLINE uint32_t sl_hal_rtcc_get_channel_ccv(uint32_t channel);
extern __INLINE void sl_hal_rtcc_set_channel_ccv(uint32_t channel,
                                                 uint32_t value);
extern __INLINE uint32_t sl_hal_rtcc_get_combined_counter(void);
extern __INLINE uint32_t sl_hal_rtcc_get_counter(void);
extern __INLINE void sl_hal_rtcc_set_counter(uint32_t value);
extern __INLINE void sl_hal_rtcc_lock(void);
extern __INLINE void sl_hal_rtcc_unlock(void);
extern __INLINE uint32_t sl_hal_rtcc_get_pre_counter(void);
extern __INLINE void sl_hal_rtcc_set_pre_counter(uint32_t preCntVal);
extern __INLINE uint32_t sl_hal_rtcc_get_status(void);
extern __INLINE void sl_hal_rtcc_wait_sync(void);
extern __INLINE void sl_hal_rtcc_start(void);
extern __INLINE void sl_hal_rtcc_stop(void);
extern __INLINE void sl_hal_rtcc_set_interrupts(uint32_t flags);
extern __INLINE uint32_t sl_hal_rtcc_get_enabled_interrupts(void);
extern __INLINE uint32_t sl_hal_rtcc_get_pending_interrupts(void);
extern __INLINE void sl_hal_rtcc_enable_interrupts(uint32_t flags);
extern __INLINE void sl_hal_rtcc_disable_interrupts(uint32_t flags);
extern __INLINE void sl_hal_rtcc_clear_interrupts(uint32_t flags);
extern __INLINE void sl_hal_rtcc_enable(void);
extern __INLINE void sl_hal_rtcc_disable(void);

/***************************************************************************//**
 * Initialize RTCC.
 ******************************************************************************/
void sl_hal_rtcc_init(const sl_hal_rtcc_init_t *init)
{
  EFM_ASSERT(init != NULL);

  sl_hal_rtcc_wait_sync();

  sl_hal_rtcc_disable();
  RTCC->CFG = (RTCC->CFG & ~(_RTCC_CFG_DEBUGRUN_MASK | _RTCC_CFG_PRECNTCCV0TOP_MASK
                             | _RTCC_CFG_CNTCCV1TOP_MASK | _RTCC_CFG_CNTPRESC_MASK
                             | _RTCC_CFG_CNTTICK_MASK))
              | (((uint32_t)init->debug_run) << _RTCC_CFG_DEBUGRUN_SHIFT)
              | (((uint32_t)init->pre_counter_wrap_on_ccv_0) << _RTCC_CFG_PRECNTCCV0TOP_SHIFT)
              | (((uint32_t)init->counter_wrap_on_ccv_1) << _RTCC_CFG_CNTCCV1TOP_SHIFT)
              | (((uint32_t)init->prescaler) << _RTCC_CFG_CNTPRESC_SHIFT)
              | (((uint32_t)init->prescaler_mode) << _RTCC_CFG_CNTTICK_SHIFT);
}

/***************************************************************************//**
 * Configure the selected capture/compare channel of the RTCC.
 ******************************************************************************/
void sl_hal_rtcc_channel_init(uint32_t channel,
                              const sl_hal_rtcc_cc_init_t *init)
{
  EFM_ASSERT(SL_HAL_RTCC_CH_VALID(channel));
  EFM_ASSERT(init != NULL);

  // Configure the selected capture/compare channel.
  RTCC->CC[channel].CTRL = ( (uint32_t)init->channel_mode << _RTCC_CC_CTRL_MODE_SHIFT)
                           | ( (uint32_t)init->compare_match_out_action << _RTCC_CC_CTRL_CMOA_SHIFT)
                           | ( (uint32_t)init->input_edge_select << _RTCC_CC_CTRL_ICEDGE_SHIFT)
                           | ( (uint32_t)init->compare_base << _RTCC_CC_CTRL_COMPBASE_SHIFT);
  if (init->channel_mode == SL_HAL_RTCC_CAPTURE_COMPARE_CHANNEL_MODE_CAPTURE) {
    volatile uint32_t *reg = &PRS->CONSUMER_RTCC_CC0;
    reg[channel] = init->prs_select;
  }
}

/***************************************************************************//**
 * Restore RTCC to its reset state.
 ******************************************************************************/
void sl_hal_rtcc_reset(void)
{
  // Restore all RTCC registers to their default values.
  sl_hal_rtcc_unlock();
  sl_hal_rtcc_enable();
  sl_hal_rtcc_stop();
  sl_hal_rtcc_wait_sync();
  RTCC->PRECNT  = _RTCC_PRECNT_RESETVALUE;
  RTCC->CNT     = _RTCC_CNT_RESETVALUE;
  RTCC->IEN     = _RTCC_IEN_RESETVALUE;
  sl_hal_rtcc_clear_interrupts(_RTCC_IF_MASK);

  for (uint8_t i = 0; i < RTCC_CC_NUM; i++) {
    RTCC->CC[i].CTRL = _RTCC_CC_CTRL_RESETVALUE;
    RTCC->CC[i].OCVALUE  = _RTCC_CC_OCVALUE_RESETVALUE;
  }
  sl_hal_rtcc_wait_sync();
  sl_hal_rtcc_disable();
  RTCC->CFG    = _RTCC_CFG_RESETVALUE;
}

/** @} (end addtogroup rtcc) */
#endif /* RTCC_COUNT && RTCC_COUNT == 1 */
