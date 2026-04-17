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

// File: aps-retry.h
//
// Description: Retried messages for the ZigBee application support sublayer.
//
// Author(s): Richard Kelsey <kelsey@ember.com>
//
// Copyright 2007 by Ember Corporation.  All rights reserved.               *80*

#ifndef SILABS_APS_RETRY_H
#define SILABS_APS_RETRY_H
#include "event_queue/event-queue.h"

// Our event.
extern sli_zigbee_event_t sli_zigbee_application_event;

//----------------------------------------------------------------
// Request that a message be queued up for retrying.

sl_status_t sli_zigbee_add_pending_acked_message(sli_zigbee_packet_header_t header,
                                                 uint8_t mode,
                                                 uint8_t addressIndex,
                                                 uint16_t apsStructOptions);

//----------------------------------------------------------------
// Remove the message that elicited this acknowledgement.

void sli_zigbee_application_process_incoming_ack(uint8_t frameControl,
                                                 sl_zigbee_aps_frame_t *incomingApsFrame,
                                                 sli_zigbee_packet_header_t ackHeader);

//----------------------------------------------------------------
// Used by the binding table and route discovery to inform the APS retry
// code that new remote IDs or routes are now available.

void sli_zigbee_note_unicast_retry_event(void);

//----------------------------------------------------------------
// Called on sleepy nodes when the only thing left to run is the APS retry
// event.

void sli_zigbee_maybe_wakeup_aps_retry(void);

//----------------------------------------------------------------
// Removes all the pending messages that are going out using the address table
// entry corresponding to the passed index.
void sli_zigbee_purge_address_table_entry_pending_messages(uint8_t addressTableIndex);

#endif // SILABS_APS_RETRY_H
