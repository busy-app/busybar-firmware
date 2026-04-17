/***************************************************************************//**
 * @file
 * @brief implementation of the ZigBee network departure.
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

#ifndef LEAVE_H
#define LEAVE_H

// External entry point for removing our children.  This is
// NLME_LEAVE.request(DeviceAddr = childId, ...)

sl_status_t sli_zigbee_stack_zigbee_remove_child(sl_802154_short_addr_t childId, uint8_t options);

// Messages arriving from the outside world.

void sli_zigbee_handle_leave_command(sl_802154_short_addr_t source,
                                     sli_zigbee_packet_header_t header,
                                     uint8_t commandFrameIndex);

// A callback for notification that a leave message has been sent.

void sli_zigbee_leave_command_sent(sli_zigbee_packet_header_t header);

// Leave the network without sending any messages.

void sli_zigbee_leave_network_quietly(void);

sl_status_t sli_zigbee_leave_internal(uint8_t options,
                                      sl_802154_short_addr_t nodeThatSentLeaveMessage,
                                      sl_zigbee_leave_reason_t reason);

#endif // LEAVE_H
