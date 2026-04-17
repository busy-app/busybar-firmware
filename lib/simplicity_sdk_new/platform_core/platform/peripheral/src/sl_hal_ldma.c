/***************************************************************************//**
 * @file
 * @brief Direct memory access (LDMA) module peripheral API
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

#include "sl_hal_ldma.h"

#if defined(LDMA_PRESENT)
#include <stddef.h>
#include "sl_assert.h"
#include "sl_hal_bus.h"
#include "sl_core.h"

/***************************************************************************//**
 * @addtogroup ldma LDMA - Linked DMA
 * @{
 ******************************************************************************/

#if defined(_LDMA_SWRST_MASK) && defined(_LDMA_EN_DISABLING_MASK)
extern __INLINE void sl_hal_ldma_wait_ready(LDMA_TypeDef *ldma);
#endif
extern __INLINE void sl_hal_ldma_enable(LDMA_TypeDef *ldma);
extern __INLINE void sl_hal_ldma_enable_channel(LDMA_TypeDef *ldma,
                                                uint32_t channel);
extern __INLINE void sl_hal_ldma_disable(LDMA_TypeDef *ldma);
extern __INLINE void sl_hal_ldma_disable_channel(LDMA_TypeDef *ldma,
                                                 uint32_t channel);
extern __INLINE uint32_t sl_hal_ldma_get_status(LDMA_TypeDef *ldma);
#if defined(_LDMA_SWRST_MASK)
extern __INLINE void sl_hal_ldma_reset(LDMA_TypeDef *ldma);
#endif
extern __INLINE bool sl_hal_ldma_channel_is_enabled(LDMA_TypeDef *ldma,
                                                    uint32_t channel);
extern __INLINE bool sl_hal_ldma_channel_is_active(LDMA_TypeDef *ldma,
                                                   uint32_t channel);
extern __INLINE void sl_hal_ldma_enable_channel_request(LDMA_TypeDef *ldma,
                                                        uint32_t channel);
extern __INLINE void sl_hal_ldma_disable_channel_request(LDMA_TypeDef *ldma,
                                                         uint32_t channel);
extern __INLINE void sl_hal_ldma_clear_interrupts(LDMA_TypeDef *ldma,
                                                  uint32_t flags);
extern __INLINE void sl_hal_ldma_disable_interrupts(LDMA_TypeDef *ldma,
                                                    uint32_t flags);
extern __INLINE void sl_hal_ldma_enable_interrupts(LDMA_TypeDef *ldma,
                                                   uint32_t flags);
extern __INLINE uint32_t sl_hal_ldma_get_pending_interrupts(LDMA_TypeDef *ldma);
extern __INLINE void sl_hal_ldma_set_interrupts(LDMA_TypeDef *ldma,
                                                uint32_t flags);
extern __INLINE uint32_t sl_hal_ldma_get_enabled_pending_interrupts(LDMA_TypeDef *ldma);
#if defined(_LDMA_IFH_MASK)
extern __INLINE void sl_hal_ldma_clear_high_interrupts(LDMA_TypeDef *ldma,
                                                       uint32_t flags);
extern __INLINE void sl_hal_ldma_disable_high_interrupts(LDMA_TypeDef *ldma,
                                                         uint32_t flags);
extern __INLINE void sl_hal_ldma_enable_high_interrupts(LDMA_TypeDef *ldma,
                                                        uint32_t flags);
extern __INLINE uint32_t sl_hal_ldma_get_pending_high_interrupts(LDMA_TypeDef *ldma);
extern __INLINE void sl_hal_ldma_set_high_interrupts(LDMA_TypeDef *ldma,
                                                     uint32_t flags);
extern __INLINE uint32_t sl_hal_ldma_get_enabled_pending_high_interrupts(LDMA_TypeDef *ldma);
#endif

/***************************************************************************//**
 * @brief
 *   Get LDMAXBAR by ldma instance
 *
 * @param[in] ldma
 *   This struct contains information about the LDMA register declaration.
 *
 * @return
 *   Struct contains information about the LDMAXBAR register declaration.
 ******************************************************************************/
static __INLINE LDMAXBAR_TypeDef *sli_get_ldmaxbar_by_ldma_instance(LDMA_TypeDef *ldma)
{
#if defined(_SILICON_LABS_32B_SERIES_3)
  uint32_t ldma_instance_number = LDMA_NUM(ldma);

  LDMAXBAR_TypeDef * ldmaXbar_instance = LDMAXBAR(ldma_instance_number);

  // ldma instance is null of invalid.
  EFM_ASSERT((uint32_t)ldmaXbar_instance != 0x0UL);

  return ldmaXbar_instance;
#else
  (void)*ldma;
  return LDMAXBAR;
#endif
}

/***************************************************************************//**
 * @brief
 *   Initialize the LDMA controller.
 ******************************************************************************/
void sl_hal_ldma_init(LDMA_TypeDef *ldma,
                      const sl_hal_ldma_init_t *init)
{
  EFM_ASSERT(init != NULL);
  EFM_ASSERT(!(((uint32_t)init->num_fixed_priority << _LDMA_CTRL_NUMFIXED_SHIFT)
               & ~_LDMA_CTRL_NUMFIXED_MASK));

  EFM_ASSERT(!(((uint32_t)init->sync_prs_clr_en << _LDMA_SYNCHWEN_SYNCCLREN_SHIFT)
               & ~_LDMA_SYNCHWEN_SYNCCLREN_MASK));
  EFM_ASSERT(!(((uint32_t)init->sync_prs_set_en << _LDMA_SYNCHWEN_SYNCSETEN_SHIFT)
               & ~_LDMA_SYNCHWEN_SYNCSETEN_MASK));

  ldma->CTRL = (uint32_t)init->num_fixed_priority << _LDMA_CTRL_NUMFIXED_SHIFT;

  ldma->SYNCHWEN = ((uint32_t)init->sync_prs_clr_en << _LDMA_SYNCHWEN_SYNCCLREN_SHIFT)
                   | ((uint32_t)init->sync_prs_set_en << _LDMA_SYNCHWEN_SYNCSETEN_SHIFT);

  ldma->CHDIS = _LDMA_CHEN_MASK;
  ldma->DBGHALT = 0;
  ldma->REQDIS  = 0;
  ldma->IF_CLR = 0xFFFFFFFFU;
}

/***************************************************************************//**
 * @brief
 *   Initialize a DMA transfer.
 ******************************************************************************/
void sl_hal_ldma_init_transfer(LDMA_TypeDef *ldma,
                               uint32_t channel,
                               const sl_hal_ldma_transfer_init_t *transfer_init,
                               const sl_hal_ldma_descriptor_t *descriptor)
{
  CORE_DECLARE_IRQ_STATE;
  uint32_t ch_mask = 1UL << channel;

  EFM_ASSERT(channel < DMA_CHAN_COUNT);
  EFM_ASSERT(transfer_init != NULL);
  EFM_ASSERT(descriptor != NULL);

  EFM_ASSERT(!(transfer_init->request_sel & ~_LDMAXBAR_CH_REQSEL_MASK));

  EFM_ASSERT(!(((uint32_t)transfer_init->arb_slots << _LDMA_CH_CFG_ARBSLOTS_SHIFT)
               & ~_LDMA_CH_CFG_ARBSLOTS_MASK));
  EFM_ASSERT(!(((uint32_t)transfer_init->src_inc_sign << _LDMA_CH_CFG_SRCINCSIGN_SHIFT)
               & ~_LDMA_CH_CFG_SRCINCSIGN_MASK));
  EFM_ASSERT(!(((uint32_t)transfer_init->dst_inc_sign << _LDMA_CH_CFG_DSTINCSIGN_SHIFT)
               & ~_LDMA_CH_CFG_DSTINCSIGN_MASK));
  EFM_ASSERT(!(((uint32_t)transfer_init->loop_count << _LDMA_CH_LOOP_LOOPCNT_SHIFT)
               & ~_LDMA_CH_LOOP_LOOPCNT_MASK));

  CORE_ENTER_ATOMIC();

  sli_get_ldmaxbar_by_ldma_instance(ldma)->CH[channel].REQSEL = transfer_init->request_sel;

  ldma->CH[channel].LOOP = transfer_init->loop_count << _LDMA_CH_LOOP_LOOPCNT_SHIFT;
  ldma->CH[channel].CFG = (transfer_init->arb_slots << _LDMA_CH_CFG_ARBSLOTS_SHIFT)
#if defined(_LDMA_CH_CFG_STRUCTBUSPORT_MASK) || defined(_LDMA_CH_CFG_SRCBUSPORT_MASK)
                          | (transfer_init->struct_bus_port << _LDMA_CH_CFG_STRUCTBUSPORT_SHIFT)
                          | (transfer_init->src_bus_port << _LDMA_CH_CFG_SRCBUSPORT_SHIFT)
                          | (transfer_init->dst_bus_port << _LDMA_CH_CFG_DSTBUSPORT_SHIFT)
#endif
                          | (transfer_init->src_inc_sign << _LDMA_CH_CFG_SRCINCSIGN_SHIFT)
                          | (transfer_init->dst_inc_sign << _LDMA_CH_CFG_DSTINCSIGN_SHIFT);

  // Set the descriptor address.
  ldma->CH[channel].LINK = (uint32_t)descriptor & _LDMA_CH_LINK_LINKADDR_MASK;

  ldma->SYNCHWEN_CLR =
    (((uint32_t)transfer_init->ldma_control_sync_prs_clear_off  << _LDMA_SYNCHWEN_SYNCCLREN_SHIFT)
     | ((uint32_t)transfer_init->ldma_control_sync_prs_set_off << _LDMA_SYNCHWEN_SYNCSETEN_SHIFT))
    & _LDMA_SYNCHWEN_MASK;

  ldma->SYNCHWEN_SET =
    (((uint32_t)transfer_init->ldma_control_sync_prs_clear_on << _LDMA_SYNCHWEN_SYNCCLREN_SHIFT)
     | ((uint32_t)transfer_init->ldma_control_sync_prs_set_on << _LDMA_SYNCHWEN_SYNCSETEN_SHIFT))
    & _LDMA_SYNCHWEN_MASK;

  if (transfer_init->request_dis) {
    sl_hal_ldma_disable_channel_request(ldma, channel);
  }

  if (transfer_init->debug_halt_en) {
    ldma->DBGHALT_SET = ch_mask;
  }

  // Clear the done flag.
  sl_hal_bus_reg_clear_mask(&ldma->CHDONE, ch_mask);

  CORE_EXIT_ATOMIC();
}

#if defined(_LDMA_CH_CTRL_EXTEND_MASK)
/***************************************************************************//**
 * @brief
 *   Initialize an extended DMA transfer.
 ******************************************************************************/
void sl_hal_ldma_init_transfer_extend(LDMA_TypeDef *ldma,
                                      uint32_t channel,
                                      const sl_hal_ldma_transfer_init_t *transfer_init,
                                      const sl_hal_ldma_descriptor_extend_t *descriptor_extend)
{
  // Ensure destination interleaving supported for given channel if enabled.
#if !defined(LDMA0_ILCHNL)
  EFM_ASSERT(!descriptor_extend->dst_il_en || ((1 << channel) & LDMA_ILCHNL));
#else
  EFM_ASSERT(!descriptor_extend->dst_il_en || ((1 << channel) & LDMA_ILCHNL(LDMA_NUM(ldma))));
#endif
#if defined(_LDMA_CH_XCTRL_DUALDSTEN_MASK)
  // Ensure dual destination supported for given channel if enabled.
  EFM_ASSERT(!descriptor_extend->dual_dst_en || ((1 << channel) & LDMA_DUALDSTCHNL(LDMA_NUM(ldma))));
#endif

  sl_hal_ldma_init_transfer(ldma,
                            channel,
                            transfer_init,
                            (const sl_hal_ldma_descriptor_t *)descriptor_extend);
}
#endif

/***************************************************************************//**
 * @brief
 *   Start a DMA transfer.
 ******************************************************************************/
void sl_hal_ldma_start_transfer(LDMA_TypeDef *ldma,
                                uint32_t channel)
{
  uint32_t ch_mask = 1UL << channel;
  EFM_ASSERT(channel < DMA_CHAN_COUNT);

  // Make sure all explicit memory access are complete before proceding.
#if defined(__CORTEXM)
  __DSB();
#else
  __sync_synchronize();
#endif

  // Start a transfer by loading the descriptor in the LINK register.
  sl_hal_bus_reg_clear_mask(&ldma->CHDONE, ch_mask);  /* Clear the done flag. */
  ldma->LINKLOAD_SET = ch_mask;
}

/***************************************************************************//**
 * @brief
 *   Stop a DMA transfer.
 ******************************************************************************/
void sl_hal_ldma_stop_transfer(LDMA_TypeDef *ldma,
                               uint32_t channel)
{
  uint32_t ch_mask = 1UL << channel;

  EFM_ASSERT(channel < DMA_CHAN_COUNT);

  /* *INDENT-OFF* */
  CORE_ATOMIC_SECTION(
    sl_hal_ldma_disable_interrupts(ldma, ch_mask);
    sl_hal_ldma_disable_channel(ldma, channel);
  )
  /* *INDENT-ON* */
}

/***************************************************************************//**
 * @brief
 *   Check if a DMA transfer has completed.
 ******************************************************************************/
bool sl_hal_ldma_transfer_is_done(LDMA_TypeDef *ldma,
                                  uint32_t channel)
{
  bool     ret_val = false;
  uint32_t ch_mask = 1UL << channel;

  EFM_ASSERT(channel < DMA_CHAN_COUNT);

  /* *INDENT-OFF* */
  CORE_ATOMIC_SECTION(
    if (ldma->CHDONE & ch_mask) {
      ret_val = true;
    }
  )
  /* *INDENT-ON* */

  return ret_val;
}

/***************************************************************************//**
 * @brief
 *   Get the number of items remaining in a transfer.
 ******************************************************************************/
uint32_t sl_hal_ldma_transfer_remaining_count(LDMA_TypeDef *ldma,
                                              uint32_t channel)
{
  uint32_t remaining;
  uint32_t done;
  uint32_t ch_mask = 1UL << channel;

  EFM_ASSERT(channel < DMA_CHAN_COUNT);

  /* *INDENT-OFF* */
  CORE_ATOMIC_SECTION(
    done   = ldma->CHDONE;
    remaining = ldma->CH[channel].CTRL;
  )
  /* *INDENT-ON* */

  done     &= ch_mask;

  if (done) {
    return 0;
  }

  remaining = (remaining & _LDMA_CH_CTRL_XFERCNT_MASK) >> _LDMA_CH_CTRL_XFERCNT_SHIFT;

  // +1 because XFERCNT is 0-based.
  return remaining + 1;
}

/// @} (end addtogroup ldma)
#endif // defined( LDMA_PRESENT )
