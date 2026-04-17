/***************************************************************************//**
 * @file app.c
 * @brief Application code
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdio.h>

#include "sl_assert.h"
#include "app.h"
#include "sl_wisun_app_core.h"
#include "sl_wisun_app_core_util.h"
#include "sl_component_catalog.h"
#include "sl_wisun_event_mgr.h"
#include "sl_wisun_config.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////
//        CoAP Meter Application task function
////////////////////////////////////////////////////////
#if defined(SL_CATALOG_WISUN_COAP_PRESENT)
void app_task(void *args)
{
  (void) args;

  uint8_t lfn_evt_ch = 0U;
  sl_wisun_device_type_t dev_type = SL_WISUN_ROUTER;

  // connect to the wisun network
  sl_wisun_app_core_util_connect_and_wait();

  dev_type = sl_wisun_app_core_get_device_type();

  if (dev_type == SL_WISUN_LFN) {
    EFM_ASSERT(app_wisun_em_subscribe_evt_notification(SL_WISUN_MSG_LFN_WAKE_UP_IND_ID,
                                                       &lfn_evt_ch) != SL_STATUS_FAIL);
  }

  while (1) {
    if (dev_type == SL_WISUN_LFN) {
      app_wisun_em_wait_evt_notification(SL_WISUN_MSG_LFN_WAKE_UP_IND_ID, lfn_evt_ch);
    } else {
      sl_wisun_app_core_util_dispatch_thread();
    }
  }
}

////////////////////////////////////////////////////////
//        Simple UDP Meter Application task function
////////////////////////////////////////////////////////
#else

#include "sl_wisun_meter.h"

void app_task(void *args)
{
  (void) args;

  // connect to the wisun network
  sl_wisun_app_core_util_connect_and_wait();

  // Simple Meter loop
  sl_wisun_meter_loop();
}
#endif

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
