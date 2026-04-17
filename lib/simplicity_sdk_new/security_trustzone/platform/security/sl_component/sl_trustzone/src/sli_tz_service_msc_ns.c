/***************************************************************************//**
 * @file
 * @brief Silicon Labs TrustZone secure MSC service (nonsecure veneers side).
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
#include <stdint.h>
#include <stdbool.h>

#include "sli_tz_ns_interface.h"
#include "sli_tz_s_interface.h"
#include "sli_tz_funcs_sids_autogen.h"
#include "em_device.h"

bool MSC_LockGetLocked(void)
{
  return (bool)sli_tz_ns_interface_dispatch_simple_noarg(
    (sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
    SLI_TZ_MSC_GET_LOCKED_SID);
}

void MSC_LockSetLocked(void)
{
  sli_tz_ns_interface_dispatch_simple_noarg(
    (sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
    SLI_TZ_MSC_SET_LOCKED_SID);
}

void MSC_LockSetUnlocked(void)
{
  sli_tz_ns_interface_dispatch_simple_noarg(
    (sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
    SLI_TZ_MSC_SET_UNLOCKED_SID);
}

uint32_t MSC_ReadCTRLGet(void)
{
  return sli_tz_ns_interface_dispatch_simple_noarg(
    (sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
    SLI_TZ_MSC_GET_READCTRL_SID);
}

void MSC_ReadCTRLSet(uint32_t value)
{
  sli_tz_ns_interface_dispatch_simple(
    (sli_tz_veneer_simple_fn)sli_tz_s_interface_dispatch_simple,
    SLI_TZ_MSC_SET_READCTRL_SID,
    value);
}

#if defined(_MSC_PAGELOCK0_MASK) || defined(_MSC_INST_PAGELOCKWORD0_MASK)

void MSC_PageLockSetLocked(uint32_t page_number)
{
  sli_tz_ns_interface_dispatch_simple(
    (sli_tz_veneer_simple_fn)sli_tz_s_interface_dispatch_simple,
    SLI_TZ_MSC_SET_PAGELOCK_SID,
    page_number);
}

bool MSC_PageLockGetLocked(uint32_t page_number)
{
  return (bool)sli_tz_ns_interface_dispatch_simple(
    (sli_tz_veneer_simple_fn)sli_tz_s_interface_dispatch_simple,
    SLI_TZ_MSC_GET_PAGELOCK_SID,
    page_number);
}

#endif

#if defined(_MSC_USERDATASIZE_MASK)
uint32_t MSC_UserDataGetSize(void)
{
  return sli_tz_ns_interface_dispatch_simple_noarg(
    (sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
    SLI_TZ_MSC_GET_USERDATA_SIZE_SID);
}
#endif

#if defined(_MSC_MISCLOCKWORD_MASK)
uint32_t MSC_MiscLockWordGet(void)
{
  return sli_tz_ns_interface_dispatch_simple_noarg(
    (sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
    SLI_TZ_MSC_GET_MISCLOCKWORD_SID);
}

void MSC_MiscLockWordSet(uint32_t value)
{
  sli_tz_ns_interface_dispatch_simple(
    (sli_tz_veneer_simple_fn)sli_tz_s_interface_dispatch_simple,
    SLI_TZ_MSC_SET_MISCLOCKWORD_SID,
    value);
}
#endif
