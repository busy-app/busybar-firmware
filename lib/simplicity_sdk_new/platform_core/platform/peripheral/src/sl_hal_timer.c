/***************************************************************************//**
 * @file
 * @brief Timer/counter (TIMER) Peripheral API
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_hal_timer.h"
#if defined(TIMER_COUNT) && (TIMER_COUNT > 0)

#include "sl_assert.h"

/***************************************************************************//**
 * @addtogroup timer TIMER - Timer/Counter
 * @brief Timer/Counter (TIMER) Peripheral API
 * @details
 *   The timer module consists of three main parts:
 *   @li General timer configuration and enable control.
 *   @li Compare/capture control.
 *   @li Dead time insertion control (may not be available for all timers).
 * @{
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
extern __INLINE void sl_hal_timer_wait_sync(TIMER_TypeDef *timer);
extern __INLINE void sl_hal_timer_wait_ready(TIMER_TypeDef *timer);
extern __INLINE void sl_hal_timer_enable(TIMER_TypeDef *timer);
extern __INLINE void sl_hal_timer_disable(TIMER_TypeDef *timer);
extern __INLINE void sl_hal_timer_start(TIMER_TypeDef *timer);
extern __INLINE void sl_hal_timer_stop(TIMER_TypeDef *timer);
#if defined(_TIMER_CFG_UPDATEMODE_MASK)
extern __INLINE void sl_hal_timer_channel_update_registers(TIMER_TypeDef *timer);
#endif
extern __INLINE uint32_t sl_hal_timer_channel_get_capture(TIMER_TypeDef *timer,
                                                          uint8_t channel);
extern __INLINE uint32_t sl_hal_timer_channel_get_capture_buffer(TIMER_TypeDef *timer,
                                                                 uint8_t channel);
extern __INLINE uint32_t sl_hal_timer_channel_get_compare(TIMER_TypeDef *timer,
                                                          uint8_t channel);
extern __INLINE uint32_t sl_hal_timer_channel_get_compare_buffer(TIMER_TypeDef *timer,
                                                                 uint8_t channel);
extern __INLINE void sl_hal_timer_channel_set_compare_buffer(TIMER_TypeDef *timer,
                                                             uint8_t channel,
                                                             uint32_t value);
extern __INLINE void sl_hal_timer_channel_set_compare(TIMER_TypeDef *timer,
                                                      uint8_t channel,
                                                      uint32_t value);
#if defined(_TIMER_CC_PHASE_MASK)
extern __INLINE void sl_hal_timer_channel_set_phase(TIMER_TypeDef *timer,
                                                    uint8_t channel,
                                                    uint32_t phase);
extern __INLINE uint32_t sl_hal_timer_channel_get_phase(TIMER_TypeDef *timer,
                                                        uint8_t channel);
extern __INLINE void sl_hal_timer_channel_set_phase_buffer(TIMER_TypeDef *timer,
                                                           uint8_t channel,
                                                           uint32_t phase);
extern __INLINE uint32_t sl_hal_timer_channel_get_phase_buffer(TIMER_TypeDef *timer,
                                                               uint8_t channel);
#endif
#if defined(_TIMER_CC_DITHER_MASK)
extern __INLINE void sl_hal_timer_channel_set_dither(TIMER_TypeDef *timer,
                                                     uint8_t channel,
                                                     uint8_t dither);
extern __INLINE uint8_t sl_hal_timer_channel_get_dither(TIMER_TypeDef *timer,
                                                        uint8_t channel);
extern __INLINE void sl_hal_timer_channel_set_dither_buffer(TIMER_TypeDef *timer,
                                                            uint8_t channel,
                                                            uint8_t dither);
extern __INLINE uint8_t sl_hal_timer_channel_get_dither_buffer(TIMER_TypeDef *timer,
                                                               uint8_t channel);
#endif
extern __INLINE uint32_t sl_hal_timer_get_counter(TIMER_TypeDef *timer);
extern __INLINE void sl_hal_timer_set_counter(TIMER_TypeDef *timer,
                                              uint32_t value);
extern __INLINE void sl_hal_timer_set_top_buffer(TIMER_TypeDef *timer,
                                                 uint32_t value);
extern __INLINE void sl_hal_timer_set_top(TIMER_TypeDef *timer,
                                          uint32_t value);
extern __INLINE uint32_t sl_hal_timer_get_top(TIMER_TypeDef *timer);
extern __INLINE uint32_t sl_hal_timer_get_status(TIMER_TypeDef *timer);
#if defined(_TIMER_STATUS2_MASK)
extern __INLINE uint32_t sl_hal_timer_get_status2(TIMER_TypeDef *timer);
#endif
#if defined(_TIMER_LOCK_MASK)
extern __INLINE void sl_hal_timer_lock(TIMER_TypeDef *timer);
extern __INLINE void sl_hal_timer_unlock(TIMER_TypeDef *timer);
#endif
extern __INLINE void sl_hal_timer_dti_enable(TIMER_TypeDef *timer);
extern __INLINE void sl_hal_timer_dti_disable(TIMER_TypeDef *timer);
extern __INLINE uint32_t sl_hal_timer_dti_get_fault(TIMER_TypeDef *timer);
extern __INLINE void sl_hal_timer_dti_clear_fault(TIMER_TypeDef *timer,
                                                  uint32_t flags);
extern __INLINE void sl_hal_timer_dti_unlock(TIMER_TypeDef *timer);
extern __INLINE void sl_hal_timer_dti_lock(TIMER_TypeDef *timer);
extern __INLINE void sl_hal_timer_clear_interrupts(TIMER_TypeDef *timer,
                                                   uint32_t flags);
extern __INLINE void sl_hal_timer_disable_interrupts(TIMER_TypeDef *timer,
                                                     uint32_t flags);
extern __INLINE void sl_hal_timer_enable_interrupts(TIMER_TypeDef *timer,
                                                    uint32_t flags);
extern __INLINE uint32_t sl_hal_timer_get_pending_interrupts(TIMER_TypeDef *timer);
extern __INLINE uint32_t sl_hal_timer_get_enabled_pending_interrupts(TIMER_TypeDef *timer);
extern __INLINE void sl_hal_timer_set_interrupts(TIMER_TypeDef *timer,
                                                 uint32_t flags);

/***************************************************************************//**
 * @brief
 *   Initialize TIMER.
 ******************************************************************************/
void sl_hal_timer_init(TIMER_TypeDef *timer,
                       const sl_hal_timer_init_t *init)
{
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));

  sl_hal_timer_disable(timer);
  sl_hal_timer_wait_ready(timer);

  // Set config register.
  timer->CFG = (init->input_enable_sync         ?   TIMER_CFG_SYNC : 0)
               | (init->count_one_shot          ?   TIMER_CFG_OSMEN : 0)
               | (init->quad_mode_x4            ?   TIMER_CFG_QDM_X4 : 0)
               | (init->debug_run               ?   TIMER_CFG_DEBUGRUN : 0)
               | (init->dma_clear_on_active     ?   TIMER_CFG_DMACLRACT : 0)
               | (init->enable_retime           ?   TIMER_CFG_RETIMEEN_ENABLE : 0)
               | (init->output_disable_sync     ?   TIMER_CFG_DISSYNCOUT : 0)
               | (init->always_track_input      ?   TIMER_CFG_ATI : 0)
               | (init->reload_start_default    ?   TIMER_CFG_RSSCOIST : 0)
               | ((uint32_t)init->count_mode   << _TIMER_CFG_MODE_SHIFT)
               | ((uint32_t)init->clock_select << _TIMER_CFG_CLKSEL_SHIFT)
#if defined(_TIMER_CFG_UPDATEMODE_MASK)
               | (init->enable_update_mode      ?   TIMER_CFG_UPDATEMODE : 0)
#endif
               | (init->prescaler              << _TIMER_CFG_PRESC_SHIFT);

  // Enable timer to set control register.
  sl_hal_timer_enable(timer);

  // Reset the counter.
  timer->CNT = _TIMER_CNT_RESETVALUE;

  // Set control register.
  timer->CTRL = ((uint32_t)init->input_rise_action   << _TIMER_CTRL_RISEA_SHIFT)
                | ((uint32_t)init->input_fall_action << _TIMER_CTRL_FALLA_SHIFT)
                | (init->count_2x                    ?   TIMER_CTRL_X2CNT : 0);

  sl_hal_timer_wait_sync(timer);
  sl_hal_timer_disable(timer);
  sl_hal_timer_wait_ready(timer);
}

/***************************************************************************//**
 * @brief
 *   Initialize the TIMER compare/capture channel.
 ******************************************************************************/
void sl_hal_timer_channel_init(TIMER_TypeDef *timer,
                               uint8_t channel,
                               const sl_hal_timer_channel_init_t *init)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));
  // Make sure channel number is valid.
  EFM_ASSERT(channel < SL_HAL_TIMER_CHANNEL_NUM(timer));

  const uint8_t channel_num = SL_HAL_TIMER_CHANNEL_NUM(timer);
  bool timer_enabled;
  uint8_t i;
  // OCB values of all channels, value of 7 represents an assumption about the maximum channels per timer.
  uint32_t ocb_value[7];
  // OCBV values of all channels, value of 7 represents an assumption about the maximum channels per timer.
  uint32_t ocbv[7];
  uint32_t topb_value;
  uint32_t topbv;

  timer_enabled = timer->EN & _TIMER_EN_EN_MASK;

  for (i = 0; i < channel_num; i++) {
    ocb_value[i] = timer->CC[i].OCB;
    ocbv[i] = timer->STATUS & (_TIMER_STATUS_OCBV0_MASK << i);
  }
  topb_value = timer->TOPB;
  topbv = timer->STATUS & _TIMER_STATUS_TOPBV_MASK;

  sl_hal_timer_disable(timer);
  sl_hal_timer_wait_ready(timer);

  timer->CC[channel].CFG = (init->output_initial_state     ? TIMER_CC_CFG_COIST : 0)
                           | (init->prs_output             ? TIMER_CC_CFG_PRSCONF : 0)
                           | (init->filter                 ? TIMER_CC_CFG_FILT_ENABLE : 0)
                           | (init->input_watermark_level  ? TIMER_CC_CFG_ICFWL : 0)
                           | ((uint32_t)init->input_type   << _TIMER_CC_CFG_INSEL_SHIFT)
                           | ((uint32_t)init->channel_mode << _TIMER_CC_CFG_MODE_SHIFT);

  // Enable timer to set control register.
  sl_hal_timer_enable(timer);

  timer->CC[channel].CTRL = (init->output_invert ?   TIMER_CC_CTRL_OUTINV : 0)
                            | ((uint32_t)init->compare_match_output_action << _TIMER_CC_CTRL_CMOA_SHIFT)
                            | ((uint32_t)init->compare_underflow_output_action << _TIMER_CC_CTRL_CUFOA_SHIFT)
                            | ((uint32_t)init->compare_overflow_output_action << _TIMER_CC_CTRL_COFOA_SHIFT)
                            | ((uint32_t)init->input_capture_event << _TIMER_CC_CTRL_ICEVCTRL_SHIFT)
                            | ((uint32_t)init->input_capture_edge  << _TIMER_CC_CTRL_ICEDGE_SHIFT);
#if defined(_TIMER_CC_PHASE_MASK)
  timer->CC[channel].PHASE = (init->phase << _TIMER_CC_PHASE_PHASE_SHIFT);
#endif
#if defined(_TIMER_CC_DITHER_MASK)
  timer->CC[channel].DITHER = (init->dither << _TIMER_CC_DITHER_DITHER_SHIFT);
#endif

  if (timer_enabled) {
    for (i = 0; i < channel_num; i++) {
      if (ocbv[i] != 0) {
        // If the OCBV value is valid, write it back to the OCB register.
        timer->CC[i].OCB = ocb_value[i];
      }
    }
    // Write back the TOPB value.
    if (topbv != 0) {
      timer->TOPB = topb_value;
    }
  } else {
    sl_hal_timer_wait_sync(timer);
    sl_hal_timer_disable(timer);
    sl_hal_timer_wait_ready(timer);
  }
}

/***************************************************************************//**
 * @brief
 *   Initialize the TIMER DTI unit. This must be called before enabling the timer.
 ******************************************************************************/
void sl_hal_timer_dti_init(TIMER_TypeDef *timer,
                           const sl_hal_timer_dti_init_t *init)
{
  EFM_ASSERT(SL_HAL_TIMER_SUPPORTS_DTI(timer));

  // Make sure the DTI unit is disabled while initializing.
  sl_hal_timer_disable(timer);
  sl_hal_timer_wait_ready(timer);
  sl_hal_timer_dti_disable(timer);

  timer->DTCFG = (init->auto_restart         ?   TIMER_DTCFG_DTDAS   : 0)
                 | (init->ch0_always_running ?   _TIMER_DTCFG_DTAR_SHIFT : 0)
                 | (init->safe_state         ?   _TIMER_DTCFG_DTFATS_SHIFT : 0)
                 | (init->enable_prs_input   ?   TIMER_DTCFG_DTPRSEN : 0);

  timer->DTTIMECFG = (init->prescaler   << _TIMER_DTTIMECFG_DTPRESC_SHIFT)
                     | ((uint32_t)init->rise_time << _TIMER_DTTIMECFG_DTRISET_SHIFT)
                     | ((uint32_t)init->fall_time << _TIMER_DTTIMECFG_DTFALLT_SHIFT);

  timer->DTFCFG = (init->enable_fault_source_core_loockup ? TIMER_DTFCFG_DTLOCKUPFEN : 0)
#if defined(_TIMER_DTFCFG_DTEM23FEN_MASK)
                  | (init->enable_fault_source_em23           ? TIMER_DTFCFG_DTEM23FEN : 0)
#endif
                  | (init->enable_fault_source_debugger     ? TIMER_DTFCFG_DTDBGFEN : 0)
                  | (init->enable_fault_source_prs_0        ? TIMER_DTFCFG_DTPRS0FEN : 0)
                  | (init->enable_fault_source_prs_1        ? TIMER_DTFCFG_DTPRS1FEN : 0)
                  | ((uint32_t)(init->fault_action)   << _TIMER_DTFCFG_DTFA_SHIFT);

  // Enable timer to set control register.
  sl_hal_timer_enable(timer);

  timer->DTCTRL = (init->output_polarity ? TIMER_DTCTRL_DTIPOL  : 0)
                  | (init->invert_complementary_output ? TIMER_DTCTRL_DTCINV  : 0);

  // Set up the DTOGEN register.
  timer->DTOGEN = init->output_gen_mask;

  // Clear any previous DTI faults.
  sl_hal_timer_dti_clear_fault(timer, sl_hal_timer_dti_get_fault(timer));

  sl_hal_timer_wait_sync(timer);
  sl_hal_timer_disable(timer);
  sl_hal_timer_wait_ready(timer);
}

/***************************************************************************//**
 * @brief
 *   Reset the TIMER to the same state that it was in after a hardware reset.
 ******************************************************************************/
void sl_hal_timer_reset(TIMER_TypeDef *timer)
{
  uint8_t i;
  const uint8_t channel_num = SL_HAL_TIMER_CHANNEL_NUM(timer);

  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_TIMER_REF_VALID(timer));

  // Enable timer.
  sl_hal_timer_enable(timer);
  sl_hal_timer_wait_sync(timer);
  sl_hal_timer_stop(timer);

  timer->CMD  = _TIMER_CMD_RESETVALUE;
  timer->CTRL = _TIMER_CTRL_RESETVALUE;
  timer->IEN  = _TIMER_IEN_RESETVALUE;
  timer->IF_CLR = _TIMER_IF_MASK;
  timer->TOPB = _TIMER_TOPB_RESETVALUE;
  // Write TOP after TOPB to invalidate TOPB (clear TIMER_STATUS_TOPBV).
  timer->TOP  = _TIMER_TOP_RESETVALUE;
  timer->CNT  = _TIMER_CNT_RESETVALUE;
  // Do not reset the route register, setting should be done independently.
  // Note: The ROUTE register may be locked by the DTLOCK register.

  for (i = 0; i < channel_num; i++) {
    timer->CC[i].CTRL     = _TIMER_CC_CTRL_RESETVALUE;
    timer->CC[i].OC       = _TIMER_CC_OC_RESETVALUE;
    timer->CC[i].OCB      = _TIMER_CC_OCB_RESETVALUE;
#if defined(_TIMER_CC_PHASE_MASK)
    timer->CC[i].PHASE    = _TIMER_CC_PHASE_RESETVALUE;
    timer->CC[i].PHASEB   = _TIMER_CC_PHASEB_RESETVALUE;
#endif
#if defined(_TIMER_CC_DITHER_MASK)
    timer->CC[i].DITHER   = _TIMER_CC_DITHER_RESETVALUE;
    timer->CC[i].DITHERB  = _TIMER_CC_DITHERB_RESETVALUE;
#endif
  }

  // Reset dead time insertion module, which has no effect on timers without DTI.
  timer->DTLOCK   = TIMER_DTLOCK_DTILOCKKEY_UNLOCK;
  timer->DTCTRL   = _TIMER_DTCTRL_RESETVALUE;
  timer->DTOGEN   = _TIMER_DTOGEN_RESETVALUE;
  timer->DTFAULTC = _TIMER_DTFAULTC_MASK;

  sl_hal_timer_wait_sync(timer);

  // CFG registers must be reset after the timer is disabled.
  sl_hal_timer_disable(timer);
  sl_hal_timer_wait_ready(timer);

  timer->CFG = _TIMER_CFG_RESETVALUE;
  for (i = 0; i < channel_num; i++) {
    timer->CC[i].CFG = _TIMER_CC_CFG_RESETVALUE;
  }

  timer->DTCFG     = _TIMER_DTCFG_RESETVALUE;
  timer->DTFCFG    = _TIMER_DTFCFG_RESETVALUE;
  timer->DTTIMECFG = _TIMER_DTTIMECFG_RESETVALUE;
}

/** @} (end addtogroup timer) */
#endif /* defined(TIMER_COUNT) && (TIMER_COUNT > 0) */
