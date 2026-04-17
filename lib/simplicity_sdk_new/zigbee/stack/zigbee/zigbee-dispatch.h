/***************************************************************************//**
 * @file
 * @brief
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

#ifndef SILABS_ZIGBEE_DISPATCH_H
#define SILABS_ZIGBEE_DISPATCH_H

// Values for sli_zigbee_current_message_status;
enum {
  NO_CURRENT_MESSAGE      = 0,
  HAVE_CURRENT_MESSAGE    = 0x01,
  HAVE_CURRENT_DATAGRAM   = 0x02,
  CURRENT_REPLY_SENT      = 0x04
};
extern uint8_t sli_zigbee_current_message_status;

extern sl_802154_short_addr_t sli_zigbee_current_sender;
extern sl_802154_long_addr_t *sli_zigbee_current_sender_eui64;
extern sl_zigbee_aps_frame_t sli_zigbee_current_aps_struct;
extern uint8_t sli_zigbee_current_binding_index;

void sli_zigbee_call_incoming_message_handler(sli_buffer_manager_buffer_t header,
                                              sl_zigbee_aps_frame_t *incomingApsFrame,
                                              uint8_t type,
                                              bool networkBroadcast);

#endif /* SILABS_ZIGBEE_DISPATCH_H */
