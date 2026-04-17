/***************************************************************************//**
 * @file sl_token_manager_lock.c
 * @brief Token Manager data access lock API implementation.
 * @version 1.0.0
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

#include "sl_token_manager_lock.h"
#include "sl_core.h"
#include "sl_common.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "cmsis_os2.h"
#endif

//****************************************************************************

/// @cond DO_NOT_INCLUDE_WITH_DOXYGEN
#if defined(SL_CATALOG_KERNEL_PRESENT) && !defined(_SILICON_LABS_32B_SERIES_2)
static osMutexId_t ctm_mutex;   // Common token manager lock mutex
#define CTM_ERROR_ASSERT()   do { EFM_ASSERT(false); } while (0)
#else
static CORE_DECLARE_IRQ_STATE;
#endif
/// @endcond

/***************************************************************************//**
 * @addtogroup token_manager
 * @{
 ******************************************************************************/
/***************************************************************************//**
 * @addtogroup lock
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @details
 * Create a mutex for enabling and disabling locks.
 *
 * This implementation provides options for using "mutexes" for RTOS users
 * and "core_critical" APIs for bare-metal users.
 *
 * @note RTOS users should avoid invoking the token manager lock APIs from within
 *       critical sections, as this may result in unexpected behavior.
 *       Ensure that the kernel is initialized before calling this API.
 ******************************************************************************/
SL_WEAK void sl_token_manager_lock_create_mutex(void)
{
#if defined(SL_CATALOG_KERNEL_PRESENT) && !defined(_SILICON_LABS_32B_SERIES_2)
  if (ctm_mutex == NULL) {
    const osMutexAttr_t mutex_attr = {
      "CTM Mutex",
      osMutexRecursive | osMutexPrioInherit,
      NULL,
      0
    };
    // Create and initialize a mutex
    ctm_mutex = osMutexNew(&mutex_attr);
    if (ctm_mutex == NULL) {
      CTM_ERROR_ASSERT();
    }
  }
#endif
}

/***************************************************************************//**
 * @details
 * The default lock-begin implementation.
 *
 * @note RTOS users should avoid invoking the token manager lock APIs from within
 *       critical sections, as this may result in unexpected behavior.
 *       Ensure that the kernel is initialized before calling this API.
 ******************************************************************************/
SL_WEAK void sl_token_manager_lock_begin(void)
{
#if defined(SL_CATALOG_KERNEL_PRESENT) && !defined(_SILICON_LABS_32B_SERIES_2)
  osStatus_t os_status = osError;
  // Bypass the lock if the kernel is not running
  if (osKernelGetState() == osKernelRunning) {
    if (ctm_mutex == NULL) {
      // Create the mutex if it hasn't been created yet
      sl_token_manager_lock_create_mutex();
    }
    // Acquire the mutex
    os_status = osMutexAcquire(ctm_mutex, osWaitForever);
    if (os_status != osErrorISR && os_status != osOK) {
      CTM_ERROR_ASSERT();
    }
  }
#else
  // Enter critical section for bare-metal systems
  CORE_ENTER_CRITICAL();
#endif
}

/***************************************************************************//**
 * @details
 * The default lock-end implementation.
 *
 * @note RTOS users should avoid invoking the token manager lock APIs from within
 *       critical sections, as this may result in unexpected behavior.
 *       Ensure that the kernel is initialized before calling this API.
 ******************************************************************************/
SL_WEAK void sl_token_manager_lock_end(void)
{
#if defined(SL_CATALOG_KERNEL_PRESENT) && !defined(_SILICON_LABS_32B_SERIES_2)
  osStatus_t os_status = osError;
  // Bypass the lock if the kernel is not running
  if (osKernelGetState() == osKernelRunning) {
    // Release the mutex
    os_status = osMutexRelease(ctm_mutex);
    if (os_status != osErrorISR && os_status != osOK) {
      CTM_ERROR_ASSERT();
    }
  }
#else
  // Exit critical section for bare-metal systems
  CORE_EXIT_CRITICAL();
#endif
}

/** @} (end addtogroup lock) */
/** @} (end addtogroup token_manager) */
