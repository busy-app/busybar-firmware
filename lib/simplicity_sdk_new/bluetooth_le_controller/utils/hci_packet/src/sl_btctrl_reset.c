/******************************************************************************
*
* @file  sl_btctrl_reset.c
* @brief Internal helper functions for reset and getting the reset reason,
* these are called internally by the HCI layer.
*
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

#include <em_device.h>
#include <stdbool.h>
#include <stdint.h>
#include "sl_btctrl_reset.h"
#include "sl_common.h"

#if defined(_SILICON_LABS_32B_SERIES_2)
#include <em_rmu.h>
#elif defined(_SILICON_LABS_32B_SERIES_3)
#include <sl_hal_emu.h>
#else
#error "Unsupported platform"
#endif

SL_ALIGN(4) static uint32_t reset_custom_reason SL_ATTRIBUTE_ALIGN(4) SL_ATTRIBUTE_SECTION(".noinit");

static uint32_t sl_btctrl_reset_get_mcu_reset_cause(void)
{
  static uint32_t cause_cache = 0xFFFFFFFF;
  if (cause_cache == 0xFFFFFFFF) {
#if defined(_SILICON_LABS_32B_SERIES_2)
    cause_cache = RMU_ResetCauseGet();
    RMU_ResetCauseClear();
#elif defined(_SILICON_LABS_32B_SERIES_3)
    cause_cache = sl_hal_emu_get_reset_cause();
    sl_hal_emu_clear_reset_cause();
#endif
  }
  return cause_cache;
}

void sl_btctrl_hci_reset(void)
{
  CORE_ResetSystem();
}

void sl_btctrl_reset_set_custom_reason(uint32_t reason)
{
  reason &= SL_BTCTRL_RESET_VALID_MASK;
  if (reason) {
    reset_custom_reason = reason;
  }
}

uint32_t sl_btctrl_reset_get_custom_reason(void)
{
  static uint32_t reason_cache = SL_BTCTRL_RESET_CUSTOM_REASON_NA;
  if (reason_cache == SL_BTCTRL_RESET_CUSTOM_REASON_NA
      && sl_btctrl_reset_get_mcu_reset_cause() == EMU_RSTCAUSE_SYSREQ) {
    reason_cache = reset_custom_reason;
  }
  return reason_cache;
}

void sl_btctrl_reset_clear_custom_reason(void)
{
  reset_custom_reason = SL_BTCTRL_RESET_CUSTOM_REASON_UNKNOWN;
}
