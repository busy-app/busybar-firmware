/***************************************************************************//**
 * @file
 * @brief Sending and handling route errors.
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

#ifndef ROUTE_ERROR_H
#define ROUTE_ERROR_H

// Sends a route error message to 'destination'.  The 'target' argument
// goes in the command payload.

#define sli_zigbee_send_route_status(destination, target, errorCode) \
  (sli_zigbee_send_route_error((destination), (target), (errorCode)))

// Send a route error message with additional payload bytes appended
void sli_zigbee_send_route_error_payload(sl_802154_short_addr_t destination,
                                         sl_802154_short_addr_t target,
                                         uint8_t errorCode,
                                         uint8_t *payload,
                                         uint8_t payload_len);

// Send a route error message to the destination argument.
// The target argument goes in the command payload.
#define sli_zigbee_send_route_error(destination, target, errorCode) \
  sli_zigbee_send_route_error_payload((destination), (target), (errorCode), NULL, 0)

// Make the command that the above call would send.
sli_zigbee_packet_header_t sli_zigbee_make_network_status_message(uint8_t errorCode,
                                                                  sl_802154_short_addr_t destination,
                                                                  sl_802154_short_addr_t target);

void sli_zigbee_handle_route_error(sli_zigbee_packet_header_t header, uint8_t commandFrameIndex);

#endif // ROUTE_ERROR_H
