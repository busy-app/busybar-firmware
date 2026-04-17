/***************************************************************************//**
 * @file
 * @brief A queue for retrying and/or delaying outgoing packets.
 * This is independent of MAC layer retries.  A typical use is to retry
 * MAC layer broadcasts, or to add initial jitter to any outgoing packet.
 *
 * There are really two queues: a "submit" queue for entries that have been
 * submitted to the mac, and a "delay" queue for entries with nonzero delay.
 *
 * This code was originally in grad/retry.c and has been made generic.
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

#ifndef SILABS_RETRY_H
#define SILABS_RETRY_H

#define RETRY_QUEUE_SIZE SL_ZIGBEE_RETRY_QUEUE_SIZE

extern uint8_t sli_zigbee_retry_queue_size;
extern sli_zigbee_retry_queue_entry_t sli_zigbee_retry_queue[];
#define retryEntries (sli_zigbee_retry_queue)

#define sli_zigbee_retry_successful_attempts(entry) ((entry)->attempts >> 4)
#define sli_zigbee_retry_remaining_attempts(entry) ((entry)->attempts & 0x0F)
#define sli_zigbee_retry_increment_successful_attempts(entry) ((entry)->attempts += 0x10)
#define sli_zigbee_retry_set_remaining_attempts(entry, remaining) \
  ((entry)->attempts = ((entry)->attempts & 0xF0) + ((remaining) & 0x0F))

// Returns false if the queues were full.
bool sli_zigbee_retry_submit(sli_zigbee_packet_header_t header,
                             uint8_t retries,
                             uint16_t delayMs,
                             uint8_t flags);

// Remove the header from the delay or submit queues. sli_zigbee_retry_transmit_complete()
// will not be called.
void sli_zigbee_retry_cancel(sli_zigbee_packet_header_t header);

// Called after a transmission succeeds to see if further sends are
// necessary.
bool sli_zigbee_needs_further_transmissions(sli_zigbee_packet_header_t header);

// Called when the radio is available.  Returns the next packet that
// needs to go out, or SL_ZIGBEE_NULL_MESSAGE_BUFFER if there is none.
bool sli_zigbee_retry_transmit(void);

// This call informs the retry module that the packet has been transmitted.
// Typically called from within the sli_802154mac_transmit_complete_callback, which
// has different implementations depending on the stack.
// Returns true if the header is found on the retry table.
bool sli_zigbee_retry_transmit_complete(sli_zigbee_packet_header_t header, sl_status_t status);

// Callback to higher layer immediately before submission to the mac.
// The higher layer code must set the value of the sli_zigbee_retry_queue_entry_t timer
// for the next delay.  If it returns false, the entry is not
// submitted to the mac, and is placed back on the retry queue if
// there are any remaining attempts.
bool sli_zigbee_prepare_retry_entry_for_submission(sli_zigbee_retry_queue_entry_t *entry);

bool sli_zigbee_retry_is_empty(void);
bool sli_zigbee_have_pending_retry_messages(void);
void sli_zigbee_retry_purge(void);
void sli_zigbee_retry_init(void);

// Called by the retry code when it is time to resend a packet.  This
// needs to be provided by whatever routing layer is present.  Returns
// true if successful and false otherwise.
//
// When using ZigBee stand-alone security on the EM2420 this has to copy
// the message and then encrypt it before sending it to the MAC.  Everyone
// else just calls sli_802154mac_submit().  Because of the freshness check done
// using the frame counter messages must be encrypted in the same order
// as they are transmitted, which means that we have to re-encrypt it for
// each retry.

bool sli_zigbee_retry_retransmit(sli_zigbee_packet_header_t header);

#endif //__RETRY_H__
