/***************************************************************************//**
 * @file
 * @brief A library for retrieving Ember stack counters over the air.
 *
 * The library allows the application to request counters from a remote
 * node.  The request specifies whether the remote node should clear
 * its counters after sending back the response.
 * The library also handles and responds to incoming counter requests apropriately.
 * A convenience method recognizes incoming responses.
 *
 * Summary of requirements:
 * - Include counters.c and counters-ota.c in the build on an EmberZNet processor.
 *   Include only counters-ota-host.c in the build on an EZSP host processor.
 * - Call ::sl_zigbee_is_incoming_counters_request() in the incoming message handler.
 * - Call ::sl_zigbee_is_incoming_counters_response() in the incoming message handler.
 * - Call ::sl_zigbee_is_outgoing_counters_response() in the message sent handler.
 *
 * The request and response messages use profile id SL_ZIGBEE_PRIVATE_PROFILE_ID,
 * and the following cluster ids:
 *
 * - SL_ZIGBEE_REPORT_COUNTERS_REQUEST
 * - SL_ZIGBEE_REPORT_AND_CLEAR_COUNTERS_REQUEST
 * - SL_ZIGBEE_REPORT_COUNTERS_RESPONSE
 * - SL_ZIGBEE_REPORT_AND_CLEAR_COUNTERS_RESPONSE
 *
 * The request has no payload.  The payload of the response is a list of the
 * non-zero counters.  Each entry in the list consists of the one-byte counter
 * id followed by a two-byte counter, low byte first.  If all entries do not
 * fit into a single payload, multiple response messages are sent.
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

#ifndef SILABS_APP_UTIL_COUNTERS_OTA_H
#define SILABS_APP_UTIL_COUNTERS_OTA_H

/** To ensure the counters response does not exceed the maximum
 * payload length, it is divided into separate messages if necessary.
 * The maximum length of 48 leaves sufficient room for headers, all
 * security modes, and source routing subframes if present.
 * It should not be necessary to change this value, but if you do,
 * make sure you know what you're doing.
 */
#define MAX_PAYLOAD_LENGTH 48

/** Sends an request to the specified destination to send back a report
 * of the non-zero counters.
 *
 * @param destination the address of the node to send the request to.
 * @param clearCounters whether or not the destination should reset its
 * counters to zero after successfully reporting them back to the requestor.
 * Note that due to technical constraints, counters are always cleared
 * on a network coprocessor such as the EM260.
 * @return SL_STATUS_OK if the request was successfully submitted for
 * sending.  See ::sl_zigbee_send_unicast() or ::sl_zigbee_ezsp_send_unicast for possible
 * failure statuses.
 */
sl_status_t sl_zigbee_send_counters_request(sl_802154_short_addr_t destination,
                                            bool clearCounters);

/** The application must call this function at the beginning of
 * the incoming message handler.  It returns TURE if the incoming
 * message was a counters request and should be ignored by
 * the rest of the incoming message handler.
 *
 * @param apsFrame the APS frame passed to the incoming message handler.
 * @param sender the node id of the sender of the request.
 * @return true if the message was a counters request and should be
 * ignored by the rest of the incoming message handler.
 */
bool sl_zigbee_is_incoming_counters_request(sl_zigbee_aps_frame_t *apsFrame,
                                            sl_802154_short_addr_t sender);

/** The application may call this function within the incoming
 * message handler to determine if the message is a counters
 * response.  If so, it is up to the application to decode
 * the payload, whose format is described above.
 *
 * @param apsFrame the APS frame passed to the incoming message handler.
 * @return true if the message is a counters response.
 */
bool sl_zigbee_is_incoming_counters_response(sl_zigbee_aps_frame_t *apsFrame);

/** The application must call this function at the begining of the
 * message sent handler.  It returns true if the message was a
 * counters response and should be ignored by the rest of the handler.
 *
 * @param apsFrame the APS frame passed to the message sent handler.
 * @param status the status passed to the message sent handler.
 * @param return true if the message was a counters response and should
 * be ignored by the rest of the message sent handler.
 */
bool sl_zigbee_is_outgoing_counters_response(sl_zigbee_aps_frame_t *apsFrame,
                                             sl_status_t status);

#endif // SILABS_APP_UTIL_COUNTERS_OTA_H
