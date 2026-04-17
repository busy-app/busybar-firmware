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

/** @brief Send
 *
 * This callback is called by the Comms Hub Function (CHF) plugin to report the
 * status of a message that was previously sent.
 *
 * @param status The status of the message that was sent Ver.: always
 * @param destinationDeviceId The EUI64 of the destination device to which the
 * data was sent Ver.: always
 * @param dataLen The length in octets of the data that was sent Ver.: always
 * @param data Buffer containing the raw octets of the data that was sent
 * Ver.: always
 */
WEAK(void sl_zigbee_af_comms_hub_function_send_cb(uint8_t status,
                                                  sl_802154_long_addr_t destinationDeviceId,
                                                  uint16_t dataLen,
                                                  uint8_t *data))
{
  UNUSED_VAR(status);
  UNUSED_VAR(destinationDeviceId);
  UNUSED_VAR(dataLen);
  UNUSED_VAR(data);
}

/** @brief Received
 *
 * This callback is called by the Comms Hub Function (CHF) plugin whenever a
 * message is received.
 *
 * @param senderDeviceId The EUI64 of the sending device Ver.: always
 * @param dataLen The length in octets of the data Ver.: always
 * @param data Buffer containing the raw octets of the data Ver.: always
 */
WEAK(void sl_zigbee_af_comms_hub_function_received_cb(sl_802154_long_addr_t senderDeviceId,
                                                      uint16_t dataLen,
                                                      uint8_t *data))
{
  UNUSED_VAR(senderDeviceId);
  UNUSED_VAR(dataLen);
  UNUSED_VAR(data);
}

/** @brief Alert WAN
 *
 * Notify the application of an Alert that should be sent to the WAN. The
 * second argument is a pointer to the gbz alert buffer. The application is
 * responsible for freeing this buffer.
 *
 * @param alertCode The 16 bit allert code as defined by GBCS Ver.: always
 * @param gbzAlert Buffer continaing the GBZ formatted Alert Ver.: always
 * @param gbzAlertLength The length in octets of the GBZ formatted Alert
 * Ver.: always
 */
WEAK(void sl_zigbee_af_comms_hub_function_alert_wan_cb(uint16_t alertCode,
                                                       uint8_t *gbzAlert,
                                                       uint16_t gbzAlertLength))
{
  UNUSED_VAR(alertCode);
  UNUSED_VAR(gbzAlert);
  UNUSED_VAR(gbzAlertLength);
}

/** @brief Tunnel Opened
 *
 * This function is called by the Comms Hub Function (CHF) plugin when a tunnel
 * is opened.
 *
 * @param remoteDeviceId The EUI64 of the remote device for which a tunnel is
 * opened Ver.: always
 */
WEAK(void sl_zigbee_af_comms_hub_function_tunnel_opened_cb(sl_802154_long_addr_t remoteDeviceId))
{
  UNUSED_VAR(remoteDeviceId);
}
