/***************************************************************************/ /**
 * @file
 * @brief CPC System Endpoint RCP API implementation.
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

#include <stdbool.h>
#include <string.h>

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#include "sli_cpc_system_common.h"

/***************************************************************************//**
 * Get system reset reason.
 ******************************************************************************/
sl_cpc_system_status_t cpc_get_reset_reason(void)
{
  sl_cpc_system_status_t last_status = STATUS_RESET_UNKNOWN;
  uint32_t reset_cause;

  (void) reset_cause;
  return last_status;
}

/***************************************************************************//**
 * Return bootloader informations
 ******************************************************************************/
void cpc_get_bootloader_info(sli_cpc_system_bootloader_info_t *infos)
{
  infos->type = SL_CPC_BOOTLOADER_NONE;
  infos->version = 0xFFFFFFFF;
  infos->capabilities = 0xFFFFFFFF;
}

/***************************************************************************//**
 * Reset system; Should not return
 ******************************************************************************/
void cpc_system_reset(sli_cpc_system_reboot_mode_t reboot_mode)
{
  (void)reboot_mode;
  EFM_ASSERT(0);

  exit(1);
}
