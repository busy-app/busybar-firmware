/*
 * EVALUATION AND USE OF THIS SOFTWARE IS SUBJECT TO THE TERMS AND
 * CONDITIONS OF THE CONTROLLING LICENSE AGREEMENT FOUND AT LICENSE.md
 * IN THIS SDK. IF YOU DO NOT AGREE TO THE LICENSE TERMS AND CONDITIONS,
 * PLEASE RETURN ALL SOURCE FILES TO SILICON LABORATORIES.
 * (c) Copyright 2018, Silicon Laboratories Inc.  All rights reserved.
 */

#include "cmsis_os2.h"
#include "sl_status.h"

/* Converts a time in milliseconds to a time in ticks.
 */
#ifndef SLI_SYSTEM_MS_TO_TICKS
#define SLI_SYSTEM_MS_TO_TICKS(ms)                                                       \
  ({                                                                                     \
    uint64_t _t = ((uint64_t)(ms) * (uint64_t)osKernelGetTickFreq() + 999ULL) / 1000ULL; \
    if (_t > UINT32_MAX)                                                                 \
      _t = UINT32_MAX;                                                                   \
    (uint32_t) _t;                                                                       \
  })
#endif

#ifndef SLI_SYSTEM_TICKS_TO_MS
#define SLI_SYSTEM_TICKS_TO_MS(ticks)                              \
  ({                                                               \
    uint64_t _f  = (uint64_t)osKernelGetTickFreq();                \
    uint64_t _ms = ((uint64_t)(ticks)*1000ULL + (_f - 1ULL)) / _f; \
    if (_ms > UINT32_MAX)                                          \
      _ms = UINT32_MAX;                                            \
    (uint32_t) _ms;                                                \
  })
#endif

/******************************************************************************
 * @brief
 *   A utility function that converts osStatus_t to sl_status_t
 * @param[in] status
 *   status of type osStatus_t that needs to be converted to equivalent sl_status_t
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 *****************************************************************************/
static inline sl_status_t sli_convert_cmsis_os_status(osStatus_t status)
{
  switch (status) {
    case osOK:
      return SL_STATUS_OK;
    case osErrorTimeout:
      return SL_STATUS_TIMEOUT;
    case osErrorParameter:
      return SL_STATUS_INVALID_PARAMETER;
    case osErrorISR:
      return SL_STATUS_ISR;
    case osErrorResource:
    case osErrorNoMemory:
      return SL_STATUS_NO_MORE_RESOURCE;
    default:
      return SL_STATUS_FAIL;
  }
}
