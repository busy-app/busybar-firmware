/***************************************************************************/ /**
 * @file sli_wifi_command_engine.c
 * @brief
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
#include "sli_wlan_command_engine.h"
#include "sl_constants.h"

/******************************************************
 *               Macro Definitions
 ******************************************************/

/******************************************************
 *               Type Definitions
 ******************************************************/

/******************************************************
 *               Global Variable Definitions
 ******************************************************/

/******************************************************
  *               Function Declarations
 ******************************************************/

/******************************************************
  *               Function Definitions
 ******************************************************/

sl_status_t sli_wifi_command_engine_init(void)
{
  sl_status_t status = SL_STATUS_OK;

  status = sli_wifi_event_engine_init();
  VERIFY_STATUS_AND_RETURN(status);

  // Initialize the command engine
  status = sli_command_engine_init(&sli_wifi_command_engine,
                                   (const sli_command_engine_configuration_t *)&sli_wifi_command_engine_config);
  VERIFY_STATUS_AND_RETURN(status);

  return SL_STATUS_OK;
}

sl_status_t sli_wifi_command_engine_deinit(void)
{
  sl_status_t status = SL_STATUS_OK;

  // Deinitialize the command engine
  status = sli_command_engine_deinit(&sli_wifi_command_engine);
  VERIFY_STATUS_AND_RETURN(status);

  status = sli_event_engine_deinit();
  VERIFY_STATUS_AND_RETURN(status);

  return SL_STATUS_OK;
}
