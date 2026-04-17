/***************************************************************************//**
 * @file
 * @brief System Real Time Counter (SYSRTC) Peripheral API
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_hal_sysrtc.h"
#if defined(SYSRTC_COUNT) && (SYSRTC_COUNT > 0)
#include "stddef.h"

/***************************************************************************//**
 * @addtogroup sysrtc SYSRTC - System Real Time Counter
 * @brief System Real Time Counter (SYSRTC) Peripheral API
 * @details
 *  This module contains functions to control the SYSRTC peripheral of Silicon
 *  Labs 32-bit MCUs and SoCs. The SYSRTC ensures timekeeping in low energy modes.
 * @{
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

extern __INLINE void sl_hal_sysrtc_wait_ready(void);
extern __INLINE void sl_hal_sysrtc_start(void);
extern __INLINE void sl_hal_sysrtc_stop(void);
extern __INLINE void sl_hal_sysrtc_lock(void);
extern __INLINE void sl_hal_sysrtc_unlock(void);
#ifdef _SYSRTC_IF_MASK
extern __INLINE void sl_hal_sysrtc_enable_interrupts(uint32_t flags);
extern __INLINE void sl_hal_sysrtc_disable_interrupts(uint32_t flags);
extern __INLINE void sl_hal_sysrtc_clear_interrupts(uint32_t flags);
extern __INLINE uint32_t sl_hal_sysrtc_get_interrupts(void);
extern __INLINE uint32_t sl_hal_sysrtc_get_enabled_interrupts(void);
extern __INLINE void sl_hal_sysrtc_set_interrupts(uint32_t flags);
#endif
extern __INLINE void sl_hal_sysrtc_set_counter(uint32_t value);
#ifdef _SYSRTC_MSCNT_MASK
extern __INLINE void sl_hal_sysrtc_start_ms(void);
extern __INLINE void sl_hal_sysrtc_stop_ms(void);
extern __INLINE uint32_t sl_hal_sysrtc_get_ms_counter(void);
extern __INLINE void sl_hal_sysrtc_set_ms_compare(uint32_t value);
extern __INLINE uint32_t sl_hal_sysrtc_get_ms_compare(void);
extern __INLINE void sl_hal_sysrtc_set_ms_compare_buffer(uint32_t value);
extern __INLINE uint32_t sl_hal_sysrtc_get_ms_compare_buffer(void);
#endif

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/
#if !defined(SYSRTC_SWRST_SWRST)
static void sli_hal_sysrtc_manual_reset(void);
#endif // !SYSRTC_SWRST_SWRST

/***************************************************************************//**
 * Initializes SYSRTC module.
 ******************************************************************************/
void sl_hal_sysrtc_init(const sl_hal_sysrtc_init_t *init)
{
  // Wait to be ready.
  sl_hal_sysrtc_wait_ready();

  if (SYSRTC_INST->EN == SYSRTC_EN_EN) {
    // Disable the module.
    sl_hal_sysrtc_disable();
    // Wait to be ready.
    sl_hal_sysrtc_wait_ready();
  }

  // Set configuration.
  SYSRTC_INST->CFG = (init->enable_debug_run ? 1UL : 0UL) << _SYSRTC_CFG_DEBUGRUN_SHIFT;
}

/***************************************************************************//**
 * Enables the SYSRTC module and starts the counter.
 ******************************************************************************/
void sl_hal_sysrtc_enable(void)
{
  // Wait if disabling.
  sl_hal_sysrtc_wait_ready();

  // Enable SYSRTC module.
  SYSRTC_INST->EN_SET = SYSRTC_EN_EN;

  // Start counter.
  SYSRTC_INST->CMD = SYSRTC_CMD_START;
}

/***************************************************************************//**
 * Stops the counter and disables the SYSRTC.
 ******************************************************************************/
void sl_hal_sysrtc_disable(void)
{
  if (SYSRTC_INST->EN != SYSRTC_EN_EN) {
    return;
  }

  // Stop counter.
  sl_hal_sysrtc_stop();

  // Disable module.
  SYSRTC_INST->EN_CLR = SYSRTC_EN_EN;
}

/***************************************************************************//**
 * Restores SYSRTC to its reset state.
 ******************************************************************************/
void sl_hal_sysrtc_reset(void)
{
  // Reset timer.
#if defined (SYSRTC_SWRST_SWRST)
  SYSRTC0->SWRST = SYSRTC_SWRST_SWRST;
#else
  sli_hal_sysrtc_manual_reset();
#endif // SYSRTC_SWRST_SWRST
}

/***************************************************************************//**
 * Manually restores SYSRTC to its reset state.
 ******************************************************************************/
#if !defined(SYSRTC_SWRST_SWRST)
static void sli_hal_sysrtc_manual_reset(void)
{
  sl_hal_sysrtc_disable();
  sl_hal_sysrtc_wait_ready();

  SYSRTC0->GRP0_CTRL = _SYSRTC_GRP0_CTRL_RESETVALUE;
  SYSRTC0->GRP0_IEN = _SYSRTC_GRP0_IEN_RESETVALUE;
  SYSRTC0->GRP0_CMP0VALUE = _SYSRTC_GRP0_CMP0VALUE_RESETVALUE;
#if defined(SYSRTC_GRP0_CTRL_CMP1EN)
  SYSRTC0->GRP0_CMP1VALUE = _SYSRTC_GRP0_CMP1VALUE_RESETVALUE;
#endif
#if defined(SYSRTC_GRP0_CTRL_CMP2EN)
  SYSRTC0->GRP0_CMP2VALUE = _SYSRTC_GRP0_CMP2VALUE_RESETVALUE;
#endif
#if defined(_SYSRTC_GRP0_PRETRIG_MASK)
  SYSRTC0->GRP0_PRETRIG = _SYSRTC_GRP0_PRETRIG_RESETVALUE;
#endif
#if defined(_SYSRTC_GRP0_PRETRIGSTATUS_MASK)
  SYSRTC0->GRP0_PRETRIGSTATUS = _SYSRTC_GRP0_PRETRIGSTATUS_RESETVALUE;
#endif
  SYSRTC0->GRP0_IF_CLR = _SYSRTC_GRP0_IF_MASK;

#if defined(_SYSRTC_GRP1_IF_MASK)
  SYSRTC0->GRP1_CTRL = _SYSRTC_GRP1_CTRL_RESETVALUE;
  SYSRTC0->GRP1_IEN = _SYSRTC_GRP1_IEN_RESETVALUE;
  SYSRTC0->GRP1_IF_CLR = _SYSRTC_GRP1_IF_MASK;
  SYSRTC0->GRP1_CMP0VALUE = _SYSRTC_GRP1_CMP0VALUE_RESETVALUE;
#if defined(SYSRTC_GRP1_CTRL_CMP1EN)
  SYSRTC0->GRP1_CMP1VALUE = _SYSRTC_GRP1_CMP1VALUE_RESETVALUE;
#endif
#if defined(SYSRTC_GRP1_CTRL_CMP2EN)
  SYSRTC0->GRP1_CMP2VALUE = _SYSRTC_GRP1_CMP2VALUE_RESETVALUE;
#endif
#if defined(_SYSRTC_GRP1_PRETRIG_MASK)
  SYSRTC0->GRP1_PRETRIG = _SYSRTC_GRP1_PRETRIG_RESETVALUE;
#endif
#if defined(_SYSRTC_GRP1_PRETRIGSTATUS_MASK)
  SYSRTC0->GRP1_PRETRIGSTATUS = _SYSRTC_GRP1_PRETRIGSTATUS_RESETVALUE;
#endif
#endif // _SYSRTC_GRP1_IF_MASK

#if defined(_SYSRTC_GRP2_IF_MASK)
  SYSRTC0->GRP2_CTRL = _SYSRTC_GRP2_CTRL_RESETVALUE;
  SYSRTC0->GRP2_IEN = _SYSRTC_GRP2_IEN_RESETVALUE;
  SYSRTC0->GRP2_IF_CLR = _SYSRTC_GRP2_IF_MASK;
  SYSRTC0->GRP2_CMP0VALUE = _SYSRTC_GRP2_CMP0VALUE_RESETVALUE;
#if defined(SYSRTC_GRP2_CTRL_CMP1EN)
  SYSRTC0->GRP2_CMP1VALUE = _SYSRTC_GRP2_CMP1VALUE_RESETVALUE;
#endif
#if defined(SYSRTC_GRP2_CTRL_CMP2EN)
  SYSRTC0->GRP2_CMP2VALUE = _SYSRTC_GRP2_CMP2VALUE_RESETVALUE;
#endif
#if defined(_SYSRTC_GRP2_PRETRIG_MASK)
  SYSRTC0->GRP2_PRETRIG = _SYSRTC_GRP2_PRETRIG_RESETVALUE;
#endif
#if defined(_SYSRTC_GRP2_PRETRIGSTATUS_MASK)
  SYSRTC0->GRP2_PRETRIGSTATUS = _SYSRTC_GRP2_PRETRIGSTATUS_RESETVALUE;
#endif
#endif // _SYSRTC_GRP2_IF_MASK

#if defined(_SYSRTC_IF_MASK)
  SYSRTC0->IEN = _SYSRTC_IEN_RESETVALUE;
  SYSRTC0->IF_CLR = _SYSRTC_IF_MASK;
#endif

  SYSRTC0->CFG = _SYSRTC_CFG_RESETVALUE;
  SYSRTC0->CNT = _SYSRTC_CNT_RESETVALUE;

#if defined(_SYSRTC_MSCNT_MASK)
  SYSRTC0->MSCMPVAL = _SYSRTC_MSCMPVAL_RESETVALUE;
  SYSRTC0->MSCMPBUF = _SYSRTC_MSCMPBUF_RESETVALUE;
#endif

}
#endif // !SYSRTC_SWRST_SWRST


/** @} (end addtogroup sysrtc) */
#endif /* defined(SYSRTC_COUNT) && (SYSRTC_COUNT > 0) */
