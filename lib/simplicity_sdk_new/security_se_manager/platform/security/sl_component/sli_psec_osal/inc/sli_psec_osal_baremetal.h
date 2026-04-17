/**************************************************************************/ /**
 * @file
 * @brief OS abstraction primitives for the platform/security components
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

#ifndef SLI_PSEC_OSAL_BAREMETAL_H
#define SLI_PSEC_OSAL_BAREMETAL_H

// -----------------------------------------------------------------------------
// Includes
#include "sl_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Defines

/// In order to wait forever in blocking functions the user can pass the
/// following value.
#define SLI_PSEC_OSAL_WAIT_FOREVER  (-1)
/// In order to return immediately in blocking functions the user can pass the
/// following value.
#define SLI_PSEC_OSAL_NON_BLOCKING  (0)

// -----------------------------------------------------------------------------
// Typedefs

/// Completion type used to wait for and signal end of operation.
typedef volatile unsigned int sli_psec_osal_completion_t;

/// SLI PSEC lock definition for Baremetal.
typedef volatile unsigned int sli_psec_osal_lock_t;

// -----------------------------------------------------------------------------
// Globals

#if defined(SLI_PSEC_OSAL_TEST)
/// Global variable to keep track of ticks in bare metal test apps.
extern unsigned int sli_sli_psec_test_ticks;
#endif

// -----------------------------------------------------------------------------
// Functions

/***************************************************************************//**
 * @brief          Initialize a completion object.
 *
 * @param p_comp   Pointer to an sli_psec_osal_completion_t object allocated
 *                 by the user.
 *
 * @return         Status code, @ref sl_status.h.
 *****************************************************************************/
__STATIC_INLINE
sl_status_t sli_psec_osal_init_completion(sli_psec_osal_completion_t *p_comp)
{
  *p_comp = 0;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * @brief          Free a completion object.
 *
 * @param p_comp   Pointer to an sli_psec_osal_completion_t object.
 *
 * @return         Status code, @ref sl_status.h.
 *****************************************************************************/
__STATIC_INLINE
sl_status_t sli_psec_osal_free_completion(sli_psec_osal_completion_t *p_comp)
{
  *p_comp = 0;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * @brief          Wait for completion event.
 *
 * @param p_comp   Pointer to completion object which must be initialized by
 *                 calling sli_psec_osal_completion_init before calling this
 *                 function.
 *
 * @param ticks    Ticks to wait for the completion.
 *                 Pass a value of SLI_PSEC_OSAL_WAIT_FOREVER in order to
 *                 wait forever.
 *                 Pass a value of SLI_PSEC_OSAL_NON_BLOCKING in order to
 *                 return immediately.
 *
 * @return         Status code, @ref sl_status.h. Typcally SL_STATUS_OK if success,
 *                 or SL_STATUS_TIMEOUT if no completion within the given ticks.
 *****************************************************************************/
__STATIC_INLINE sl_status_t
sli_psec_osal_wait_completion(sli_psec_osal_completion_t *p_comp, int ticks)
{
  int ret = SL_STATUS_TIMEOUT;

  if (ticks == SLI_PSEC_OSAL_WAIT_FOREVER) {
    while ( *p_comp == 0 ) {
#if defined(SLI_PSEC_OSAL_TEST)
      sli_sli_psec_test_ticks++;
#endif
    }
    *p_comp = 0;
    ret = SL_STATUS_OK;
  } else {
    while ((*p_comp == 0) && (ticks > 0)) {
      ticks--;
#if defined(SLI_PSEC_OSAL_TEST)
      sli_sli_psec_test_ticks++;
#endif
    }
    if (*p_comp == 1) {
      *p_comp = 0;
      ret = SL_STATUS_OK;
    }
  }

  return ret;
}

/***************************************************************************//**
 * @brief          Signal completion.
 *
 * @param p_comp   Pointer to completion object which must be initialized by
 *                 calling sli_psec_osal_completion_init before calling this
 *                 function.
 *
 * @return         Status code, @ref sl_status.h.
 *****************************************************************************/
__STATIC_INLINE
sl_status_t sli_psec_osal_complete(sli_psec_osal_completion_t* p_comp)
{
  *p_comp = 1;
  return SL_STATUS_OK;
}

#ifdef __cplusplus
}
#endif

#endif // SLI_PSEC_OSAL_BAREMETAL_H
