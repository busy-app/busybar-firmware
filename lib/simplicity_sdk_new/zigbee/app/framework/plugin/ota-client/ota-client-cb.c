/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

/** @brief Pre-Bootload Callback
 *
 * Called shortly before installing the downloaded image.
 *
 * @param srcEndpoint     Ver.: always
 * @param serverEndpoint  Ver.: always
 * @param serverNodeId    Ver.: always
 */
WEAK(void sl_zigbee_af_ota_client_pre_bootload_cb(uint8_t srcEndpoint,
                                                  uint8_t serverEndpoint,
                                                  sl_802154_short_addr_t serverNodeId))
{
  UNUSED_VAR(srcEndpoint);
  UNUSED_VAR(serverEndpoint);
  UNUSED_VAR(serverNodeId);
}
