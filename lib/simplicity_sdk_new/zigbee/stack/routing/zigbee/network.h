/***************************************************************************//**
 * @file
 * @brief The ZigBee network layer interface.
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

#ifndef SILABS_ZIGBEE_NETWORK_H
#define SILABS_ZIGBEE_NETWORK_H

#ifdef DROP_PACKET_DEBUG_ON
  #if defined(SL_ZIGBEE_TEST) || defined(ZIGBEE_STACK_ON_HOST)
    #define DROP_PACKET(x) fprintf(stderr, "DROP:" x)
  #else
    #define DROP_PACKET(x) sli_zigbee_debug_binary_format(EM_DEBUG_PRINTF, "F", "DROP:" x)
  #endif
#else
  #define DROP_PACKET(x)
#endif

void sli_zigbee_all_networks_init(void);
void sli_zigbee_current_network_init(void);

bool sli_zigbee_is_zigbee_state_unauthenticated(void);

void sli_zigbee_network_check_incoming_queue(void);
void sli_zigbee_network_process_incoming(sli_zigbee_packet_header_t header);
void sli_zigbee_process_incoming_broadcast(sli_zigbee_packet_header_t header);
bool sli_zigbee_process_network_header(sli_zigbee_packet_header_t macHeaderOnly,
                                       const uint8_t* networkHeader);

// Sends either broadcast or unicast depending on dest.
bool sli_zigbee_network_send(sli_zigbee_packet_header_t header);

bool sli_zigbee_network_send_broadcast(sli_zigbee_packet_header_t header);

extern uint8_t sli_zigbee_allow_relay;

// Four possible outcomes of sending a message.  'ROUTE_DISCOVERY_UNDERWAY'
// means that the message was not sent, but a route discovery has been
// initiated.

enum {
  SEND_UNICAST_SUCCESS,
  SEND_UNICAST_ROUTE_DISCOVERY_UNDERWAY,
  SEND_UNICAST_NO_ROUTE,
  SEND_UNICAST_FAILURE
};

// Figures out the next hop to send to and then submits the packet to the
// correct queue

uint8_t sli_zigbee_network_send_unicast(sli_zigbee_packet_header_t header);

// Submits the given unicast to the correct using the specified next hop.
// This sets the MAC address, encrypts if necessary,
// and then puts it on the correct MAC queue (direct or indirect).

bool sli_zigbee_network_submit_unicast(sli_zigbee_packet_header_t header, sl_802154_short_addr_t nextHopId);

// Called when transmission of a network message is complete.
void sli_zigbee_network_transmit_complete(sli_zigbee_packet_header_t header,
                                          sl_status_t status,
                                          bool dataPending);

// Called when the network layer finishes sending a data unicast.
void sli_zigbee_network_transmit_complete_callback(sli_zigbee_packet_header_t header, sl_status_t status);

// Used by the MAC to perform network encryption.
uint8_t sli_zigbee_network_encrypt(uint8_t *networkHeaderPointer,
                                   uint8_t packetLength,
                                   sli_zigbee_packet_header_t header);

// Internal function to respond to unknown network commands
void sendUnknownNetworkCommandResponse(sl_802154_short_addr_t dest, uint8_t commandId);

#define UNIFIED_MAC_SUSPEND_CALL_BACK sli_zigbee_network_process_incoming

bool sli_zigbee_super_retries_for_mac_data_poll_complete(sli_zigbee_packet_header_t header, sl_status_t status);

#endif // SILABS_ZIGBEE_NETWORK_H
