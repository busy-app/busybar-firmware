/***************************************************************************//**
 * @file
 * @brief implementation of the ZigBee application support sublayer.
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

#ifndef SILABS_APPLICATION_SUPPORT_H
#define SILABS_APPLICATION_SUPPORT_H

// Internal additions to the outgoing message types.  These are for:
//  - payload-carrying APS ACKs
//  - APS data unicasts and broadcasts.  Use of these types
//    avoids disables the call to sli_zigbee_stack_message_sent_handler() when the
//    transmission is complete.  These are used for ZDO replies
//    and stack-initiated ZDO requests.
//
#define SL_ZIGBEE_OUTGOING_UNICAST_REPLY   (SL_ZIGBEE_OUTGOING_BROADCAST + 1)
#define SL_ZIGBEE_OUTGOING_STACK_DIRECT    (SL_ZIGBEE_OUTGOING_BROADCAST + 2)
#define SL_ZIGBEE_OUTGOING_STACK_BROADCAST (SL_ZIGBEE_OUTGOING_BROADCAST + 3)
#define SL_ZIGBEE_OUTGOING_COMMAND_ACK     (SL_ZIGBEE_OUTGOING_BROADCAST + 4)
//----------------------------------------------------------------
// Called by the network layer on incoming messages.

void sli_zigbee_application_check_incoming_queue(void);
void sli_zigbee_application_process_incoming(sli_zigbee_packet_header_t header, bool decrypt);

//----------------------------------------------------------------
// Used to release APS messages pending retry during reinitialization.

void sli_zigbee_aps_purge(void);
void sli_zigbee_destination_specific_queue_purge(sl_802154_short_addr_t id);

//----------------------------------------------------------------
// Called by the dispatch code to send a reply.

sl_status_t sli_zigbee_send_reply(uint16_t clusterId,
                                  sli_buffer_manager_buffer_t reply);

//----------------------------------------------------------------
// Used to intercept ZDO requests for sleepy end device  children.
// Returns true if this request should be intercepted and actually acts on the
// message if takeAction is true.

bool sli_zigbee_handle_sleepy_request(sl_802154_short_addr_t source,
                                      sl_802154_short_addr_t destination,
                                      sli_zigbee_packet_header_t header,
                                      bool takeAction);

//----------------------------------------------------------------
// Creates and sends an APS message, or, if headerReturnLoc is non-NULL,
// just creates the message.

sl_status_t sli_zigbee_send_aps_message(uint8_t mode,
                                        uint16_t indexOrDestination,
                                        sl_zigbee_aps_frame_t *apsStruct,
                                        uint8_t radius,
                                        sli_buffer_manager_buffer_t message,
                                        sli_zigbee_packet_header_t *headerReturnLoc);

//----------------------------------------------------------------
// Create a read-to-transmit unicast message with network and APS frames.
// Returns SL_ZIGBEE_NULL_MESSAGE_BUFFER if there are insufficient buffers
// available.

sli_zigbee_packet_header_t sli_zigbee_make_aps_unicast(sl_802154_short_addr_t destination,
                                                       uint16_t options,
                                                       uint16_t clusterId,
                                                       uint8_t *payload,
                                                       uint8_t payloadLength);

//----------------------------------------------------------------
// Adjusts sli_zigbee_stack_maximum_aps_payload_length() to account for
// SL_ZIGBEE_APS_OPTION_SOURCE_EUI64 and SL_ZIGBEE_APS_OPTION_DESTINATION_EUI64.

uint8_t sli_zigbee_maximum_aps_payload_with_options(uint16_t options);
void sli_zigbee_stack_zdo_aps_ack_received(uint16_t cluster_id, uint16_t short_address);

#endif // SILABS_APPLICATION_SUPPORT_H
