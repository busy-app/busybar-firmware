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

/** @brief Device Removed
 *
 * This callback is called by the plugin when a device is removed from the
 * device log.
 *
 * @param deviceId Identifier of the device removed Ver.: always
 */
WEAK(void sl_zigbee_af_gbcs_device_log_device_removed_cb(sl_802154_long_addr_t deviceId))
{
  sl_zigbee_af_app_print("GBCS sl_zigbee_af_gbcs_device_log_device_removed_cb: EUI64=");
  sl_zigbee_af_app_debug_exec(sl_zigbee_af_print_big_endian_eui64(deviceId));
  sl_zigbee_af_app_println("");
}
