/***************************************************************************/ /**
 * @file
 * @brief Implementation of CPC Secondary reset functionality.
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
#include "sli_cpc_system_common.h"
#include "sl_cpc_drv_secondary_sdio.h"
/***************************************************************************/ /**
 * Get system reset reason.
 ******************************************************************************/
sl_cpc_system_status_t cpc_get_reset_reason(void)
{
  // For handling si91x reset cause
  return sl_si91x_read_reset_reason();
}

/***************************************************************************/ /**
 * Received reset request; Prepare for system reset
 ******************************************************************************/
sl_cpc_system_status_t cpc_secondary_on_reset_request(sli_cpc_system_reboot_mode_t reboot_mode)
{
  (void)reboot_mode;
  return STATUS_OK;
}

/***************************************************************************/ /**
 * Reset system; Should not return
 ******************************************************************************/
void cpc_system_reset(sli_cpc_system_reboot_mode_t reboot_mode)
{
  // Chip Reset
  (void)reboot_mode;
  /* 91x note - Both M4SS and TASS are reset */
  sl_si91x_device_reset();
}
