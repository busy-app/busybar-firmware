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

/** @brief Network Found
 *
 * This is called by the form-and-join library to notify the application of the
 * network found after a call to ::sl_zigbee_scan_for_joinable_network() or
 * ::sl_zigbee_scan_for_next_joinable_network(). See form-and-join documentation for
 * more information.
 *
 * @param networkFound   Ver.: always
 * @param lqi   Ver.: always
 * @param rssi   Ver.: always
 */
WEAK(void sl_zigbee_af_form_and_join_network_found_cb(sl_zigbee_zigbee_network_t *networkFound,
                                                      uint8_t lqi,
                                                      int8_t rssi))
{
  UNUSED_VAR(networkFound);
  UNUSED_VAR(lqi);
  UNUSED_VAR(rssi);
}

/** @brief Unused Pan Id Found
 *
 * This function is called when the form-and-join library finds an unused PAN
 * ID that can be used to form a new network on.
 *
 * @param panId A randomly generated PAN ID without other devices on it.
 * Ver.: always
 * @param channel The channel where the PAN ID can be used to form a new
 * network. Ver.: always
 */
WEAK(void sl_zigbee_af_form_and_join_unused_pan_id_found_cb(sl_802154_pan_id_t panId,
                                                            uint8_t channel))
{
  UNUSED_VAR(panId);
  UNUSED_VAR(channel);
}
