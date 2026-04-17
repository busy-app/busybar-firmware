/***************************************************************************//**
 * @file
 * @brief Routines for the Level Control plugin, which implements the
 *        Level Control cluster.
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

#include "app/framework/include/af.h"
#include "app/framework/plugin/level-control/level-control.h"

/** @brief Level Control Cluster Server Post Init
 *
 * Following resolution of the Current Level at startup for this endpoint,
 * perform any additional initialization needed; e.g., synchronize hardware
 * state.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
WEAK(void sl_zigbee_af_level_control_cluster_server_post_init_cb(uint8_t endpoint))
{
  (void)endpoint;
}

/** @brief Level Control Coupled Color Temp Change
 *
 * Adjust Color Control cluster Color Temperature in response to a change in
 * Level Control cluster CurrentLevel.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
WEAK(void sl_zigbee_af_level_control_coupled_color_temp_change_cb(uint8_t endpoint))
{
  (void)endpoint;
}
