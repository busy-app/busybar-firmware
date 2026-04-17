/***************************************************************************//**
 * @file app_init.c
 * @brief app_init.c
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "gpd-components-common.h"
#include "sl_rail_util_init.h"

void app_init(void)
{
  // Initialise NV
  sl_zigbee_gpd_nv_init();

  // Provide the rail handle from the configured RAIL instance
  sl_zigbee_gpd_rail_provide_rail_handle(sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_GPDRAIL));

  // Initialise Radio
  sl_zigbee_gpd_radio_init();

  //Initialise the Gpd
  (void)sl_zigbee_gpd_init();
}
