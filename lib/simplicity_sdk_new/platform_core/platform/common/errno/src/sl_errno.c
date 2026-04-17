/***************************************************************************//**
 * @file sl_errno.c
 * @brief source to provide errno support.
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

#include "errno.h"
#include "sl_core.h"
#include "sl_compiler.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

// Retrieve global variable used for errno depending if in kernel context and errno is enabled.
#if defined(SL_CATALOG_FREERTOS_KERNEL_PRESENT)
#include "FreeRTOSConfig.h"
#if defined(configUSE_POSIX_ERRNO) && (configUSE_POSIX_ERRNO == 1)
extern int FreeRTOS_errno;
#define SLI_ERRNO_GLOBAL_VAR FreeRTOS_errno
#endif
#elif defined(SL_CATALOG_MICRIUMOS_KERNEL_PRESENT)
#include "os_cfg.h"
#if defined(OS_CFG_ERRNO_EN) && (OS_CFG_ERRNO_EN == 1)
extern int micriumos_errno;
#define SLI_ERRNO_GLOBAL_VAR micriumos_errno
#endif
#endif

// Set backup errno variable for baremetal or RTOS context without errno enabled
#ifndef SLI_ERRNO_GLOBAL_VAR
static int single_thread_errno = 0;
#define SLI_ERRNO_GLOBAL_VAR single_thread_errno
#endif

// Set a global errno variable for interrupt context
static int interrupt_errno = 0;

/***************************************************************************//**
 * Errno function used with macro errno.
 *
 * @return  Address of global variable used to store errno value
 ******************************************************************************/
int *sl_errno(void)
{
  if (CORE_InIrqContext()) {
    return &interrupt_errno;
  }
  return &SLI_ERRNO_GLOBAL_VAR;
}

// Re-definition of standard library weak errno function
#if defined(__GNUC__)
__USED int *__errno(void)
{
  if (CORE_InIrqContext()) {
    return &interrupt_errno;
  }
  return &SLI_ERRNO_GLOBAL_VAR;
}
#elif defined(__ICCARM__)
__USED int volatile *__aeabi_errno_addr(void)
{
  if (CORE_InIrqContext()) {
    return &interrupt_errno;
  }
  return &SLI_ERRNO_GLOBAL_VAR;
}
#endif
