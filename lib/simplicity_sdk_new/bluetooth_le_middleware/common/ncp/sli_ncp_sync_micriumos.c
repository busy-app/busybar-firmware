/***************************************************************************//**
 * @file
 * @brief Micirum OS utilities for synchronizing tasks handling Bluetooth events
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
#include "os.h"
#include "sli_ncp_sync.h"
#include "sl_ncp.h"

// OS Event Flag ID
#define EVT_FLAG ((OS_FLAGS)0x01)

// OS Event Flag Group
static OS_FLAG_GRP ncp_event_flags;

// Initialization.
void sli_ncp_sync_init(void)
{
  RTOS_ERR err;
  OSFlagCreate(&ncp_event_flags, "NCP flags", (OS_FLAGS)0, &err);
  if (err.Code != RTOS_ERR_NONE) {
    sl_ncp_on_error(SL_NCP_ERROR_RUNTIME, SL_STATUS_NULL_POINTER);
  }
}

// Signal that en event was processed.
void sli_ncp_sync_signal(void)
{
  RTOS_ERR err;
  OSFlagPost(&ncp_event_flags, EVT_FLAG, OS_OPT_POST_FLAG_SET, &err);
  (void)err;
}

// Wait until the next is processed.
void sli_ncp_sync_wait(void)
{
  RTOS_ERR err;
  OSFlagPend(&ncp_event_flags,
             EVT_FLAG,
             (OS_TICK)0,
             OS_OPT_PEND_FLAG_SET_ALL + OS_OPT_PEND_FLAG_CONSUME + OS_OPT_PEND_BLOCKING,
             NULL,
             &err);
  if (err.Code != RTOS_ERR_NONE) {
    sl_ncp_on_error(SL_NCP_ERROR_RUNTIME, SL_STATUS_FAIL);
  }
}
