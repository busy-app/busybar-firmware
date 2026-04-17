/***************************************************************************//**
 * @file
 * @brief NVM3 examples functions
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef NVM3_APP_H
#define NVM3_APP_H

#include "em_device.h"
#include "sl_status.h"

static inline void nvm3_store_timing(uint32_t *p_cycles,
                                     uint32_t *p_time_us,
                                     uint32_t cycles,
                                     uint32_t us)
{
  if (p_cycles) {
    *p_cycles  = cycles;
  }
  if (p_time_us) {
    *p_time_us = us;
  }
}

#if defined(DWT_CTRL_CYCCNTENA_Msk)

#define nvm3_timed_call_ret(CODE, p_cycles, p_time_us)                              \
  ({                                                                                \
    const uint32_t _hclk = SystemHCLKGet();                                         \
    DWT->CYCCNT = 0;                                                                \
    sl_status_t _sta = (CODE);                                                      \
    uint32_t _cycles = DWT->CYCCNT;                                                 \
    uint32_t _us = _hclk ? (uint32_t)((uint64_t)_cycles * 1000000ULL / _hclk) : 0U; \
    nvm3_store_timing((p_cycles), (p_time_us), _cycles, _us);                       \
    _sta;                                                                           \
  })

#endif

/***************************************************************************//**
 * Initialize NVM3 example.
 ******************************************************************************/
void nvm3_app_init(void);

/***************************************************************************//**
 * Enable cycle counter in DWT.
 ******************************************************************************/
__STATIC_INLINE void enable_cycle_count(void)
{
#if defined(DWT_CTRL_CYCCNTENA_Msk)
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
#else
  (void)0;
#endif
}

#endif  // NVM3_APP_H
