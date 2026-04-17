/***************************************************************************//**
 * @file app.c
 * @brief Application-level Functions
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_hal_sysrtc.h"
#include "sl_main_init.h"

void app_init(void)
{
  sl_hal_sysrtc_config_t sysrtc_config = SYSRTC_CONFIG_DEFAULT;
  uint32_t status;

  status = sl_hal_sysrtc_get_status();
  if (!(status & SYSRTC_STATUS_RUNNING)) {
    /**
     * SYSRTC is needed for RAIL timer synchronization. If it's not started
     * by sleep timer, it must be started manually. Peripheral bus clock is
     * enabled by device init/clock manager.
     */
    sl_hal_sysrtc_init(&sysrtc_config);
    sl_hal_sysrtc_enable();
  }
}
