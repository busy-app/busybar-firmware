/***************************************************************************//**
 * @file
 * @brief CS Antenna for NCP host
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

#include <stdint.h>
#include "cs_antenna.h"
#include "sl_bt_api.h"
#include "cs_acp.h"

#define ANTENNA_CONFIGURE_MSG_LEN  (sizeof(cs_acp_cmd_id_t) + 1)

sl_status_t cs_antenna_configure(bool wired)
{
  cs_acp_cmd_t acp_cmd;
  acp_cmd.cmd_id = CS_ACP_CMD_ANTENNA_CONFIGURE;
  acp_cmd.data.antenna_config_wired = (uint8_t)wired;

  return sl_bt_user_cs_service_message_to_target(ANTENNA_CONFIGURE_MSG_LEN,
                                                 (uint8_t *)&acp_cmd,
                                                 0,
                                                 NULL,
                                                 NULL);
}
