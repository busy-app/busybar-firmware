/***************************************************************************//**
 * @file
 * @brief Silicon Labs TrustZone non-secure interface dispatch functionality.
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sli_tz_ns_interface.h"
#include "sl_status.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
  #include "sl_component_catalog.h"
#endif

#if defined(SL_CATALOG_MICRIUMOS_KERNEL_PRESENT) \
  || defined(SL_CATALOG_FREERTOS_KERNEL_PRESENT)
  #include "sli_psec_osal.h"
  #include "sl_assert.h"
  #define RTOS_KERNEL_PRESENT
#endif

//------------------------------------------------------------------------------
// RTOS Support

#if defined(RTOS_KERNEL_PRESENT)

static sli_psec_osal_lock_t skl_lock = { 0 };

#endif // RTOS_KERNEL_PRESENT

//------------------------------------------------------------------------------
// Global Functions

// Initialise the TrustZone NS interface.
// If RTOS is enabled, initialise the SKL lock.
// This function is typically called by the service_init event_handler.
sl_status_t sli_tz_ns_interface_init(void)
{
  sl_status_t sl_status = SL_STATUS_OK;
  #if defined(RTOS_KERNEL_PRESENT)
  SLI_PSEC_OSAL_KERNEL_CRITICAL_SECTION_START

    sl_status = sli_psec_osal_init_lock(&skl_lock);

  SLI_PSEC_OSAL_KERNEL_CRITICAL_SECTION_END
  #endif // RTOS_KERNEL_PRESENT

  return sl_status;
}

__attribute__((__used__))
int32_t sli_tz_ns_interface_dispatch(sli_tz_veneer_fn fn,
                                     uint32_t arg0,
                                     uint32_t arg1,
                                     uint32_t arg2,
                                     uint32_t arg3)
{
  #if defined(RTOS_KERNEL_PRESENT)
  if (SLI_PSEC_OSAL_KERNEL_RUNNING) {
    sl_status_t sl_status = sli_psec_osal_take_lock(&skl_lock);
    EFM_ASSERT(sl_status == SL_STATUS_OK);
  }
  #endif // RTOS_KERNEL_PRESENT

  int32_t result = fn((sli_tz_invec *)arg0, arg1, (sli_tz_outvec *)arg2, arg3);

  #if defined(RTOS_KERNEL_PRESENT)
  if (SLI_PSEC_OSAL_KERNEL_RUNNING) {
    sl_status_t sl_status = sli_psec_osal_give_lock(&skl_lock);
    EFM_ASSERT(sl_status == SL_STATUS_OK);
  }
  #endif // RTOS_KERNEL_PRESENT

  return result;
}

__attribute__((__used__))
uint32_t sli_tz_ns_interface_dispatch_simple(sli_tz_veneer_simple_fn fn,
                                             uint32_t sid,
                                             uint32_t arg)
{
  #if defined(RTOS_KERNEL_PRESENT)
  if (SLI_PSEC_OSAL_KERNEL_RUNNING) {
    sl_status_t sl_status = sli_psec_osal_take_lock(&skl_lock);
    EFM_ASSERT(sl_status == SL_STATUS_OK);
  }
  #endif // RTOS_KERNEL_PRESENT

  uint32_t result = fn(sid, arg);

  #if defined(RTOS_KERNEL_PRESENT)
  if (SLI_PSEC_OSAL_KERNEL_RUNNING) {
    sl_status_t sl_status = sli_psec_osal_give_lock(&skl_lock);
    EFM_ASSERT(sl_status == SL_STATUS_OK);
  }
  #endif // RTOS_KERNEL_PRESENT

  return result;
}

__attribute__((__used__))
int32_t sli_tz_ns_interface_dispatch_noarg(sli_tz_veneer_noarg_fn fn)
{
  #if defined(RTOS_KERNEL_PRESENT)
  if (SLI_PSEC_OSAL_KERNEL_RUNNING) {
    sl_status_t sl_status = sli_psec_osal_take_lock(&skl_lock);
    EFM_ASSERT(sl_status == SL_STATUS_OK);
  }
  #endif // RTOS_KERNEL_PRESENT

  int32_t result = fn();

  #if defined(RTOS_KERNEL_PRESENT)
  if (SLI_PSEC_OSAL_KERNEL_RUNNING) {
    sl_status_t sl_status = sli_psec_osal_give_lock(&skl_lock);
    EFM_ASSERT(sl_status == SL_STATUS_OK);
  }
  #endif // RTOS_KERNEL_PRESENT

  return result;
}

__attribute__((__used__))
int32_t sli_tz_ns_interface_dispatch_simple_noarg(sli_tz_veneer_simple_noarg_fn fn,
                                                  uint32_t sid)
{
  #if defined(RTOS_KERNEL_PRESENT)
  if (SLI_PSEC_OSAL_KERNEL_RUNNING) {
    sl_status_t sl_status = sli_psec_osal_take_lock(&skl_lock);
    EFM_ASSERT(sl_status == SL_STATUS_OK);
  }
  #endif // RTOS_KERNEL_PRESENT

  int32_t result = fn(sid);

  #if defined(RTOS_KERNEL_PRESENT)
  if (SLI_PSEC_OSAL_KERNEL_RUNNING) {
    sl_status_t sl_status = sli_psec_osal_give_lock(&skl_lock);
    EFM_ASSERT(sl_status == SL_STATUS_OK);
  }
  #endif // RTOS_KERNEL_PRESENT

  return result;
}
