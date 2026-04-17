/***************************************************************************//**
 * @file
 * @brief Definitions for the Sleepy Message Queue plugin.
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

#ifndef SILABS_SLEEPY_MESSAGE_QUEUE_H
#define SILABS_SLEEPY_MESSAGE_QUEUE_H

#include "sleepy-message-queue-config.h"

/**
 * @defgroup sleepy-message-queue Sleepy Message Queue
 * @ingroup component
 * @brief API and Callbacks for the Sleepy Message Queue Component
 *
 * This component is a message buffering system used to store messages
 * until they are retrieved or a settable message timeout expires.
 *
 */

/**
 * @addtogroup sleepy-message-queue
 * @{
 */

typedef uint8_t sl_zigbee_af_sleepy_message_id_t;

typedef struct {
  uint8_t *payload;
  uint16_t length;
  uint16_t payloadId;
  sl_802154_long_addr_t dstEui64;
} sl_zigbee_af_sleepy_message_t;

#define SL_ZIGBEE_AF_PLUGIN_SLEEPY_MESSAGE_INVALID_ID 0xFF

/**
 * @name API
 * @{
 */

/**
 * @brief Return the number of unused entries in the sleepy message queue.
 *
 **/
uint8_t sl_zigbee_af_sleepy_message_queue_get_num_unused_entries(void);

/**
 * @brief Store an sl_zigbee_af_sleepy_message_t to the sleepy message queue if an entry is available.
 * @param pmsg A pointer to an sl_zigbee_af_sleepy_message_t structure containing information about the message that should be stored.
 * @param timeoutSec The time in seconds that the message should be stored in the sleepy message queue.
 * @return The sl_zigbee_af_sleepy_message_id_t assigned to the message if stored,
 * or SL_ZIGBEE_AF_PLUGING_SLEEPY_MESSAGE_INVALID_ID if the message could not be stored to the queue.
 * The message may not be stored if the queue is full, or the time duration exceeds the maximum duration.
 **/
sl_zigbee_af_sleepy_message_id_t sl_zigbee_af_sleepy_message_queue_store_message(sl_zigbee_af_sleepy_message_t *pmsg, uint32_t timeoutSec);

/**
 * @brief Return the number of milliseconds remaining until the sleepy message expires.
 * @param sleepyMsgId The sl_zigbee_af_sleepy_message_id_t of the message whose timeout should be found.
 * @return The number of milliseconds until the specified message expires,
 * or 0xFFFFFFFF if a matching active message cannot be found.
 **/
uint32_t sli_zigbee_af_message_m_sec_remaining(sl_zigbee_af_sleepy_message_id_t sleepyMsgId);

/**
 * @brief Return the next sl_zigbee_af_sleepy_message_id_t value (that will expire next) for a given sl_802154_long_addr_t.
 * @param dstEui64 The sl_802154_long_addr_t value of a device whose sl_zigbee_af_sleepy_message_id_t is being queried.
 * @return The sl_zigbee_af_sleepy_message_id_t value of the next-expiring message for the specified sl_802154_long_addr_t
 * if a match was found, or SL_ZIGBEE_AF_PLUGIN_SLEEPY_MESSAGE_INVALID_ID if a matching entry was not found.
 **/
sl_zigbee_af_sleepy_message_id_t sl_zigbee_af_sleepy_message_queue_get_pending_message_id(sl_802154_long_addr_t dstEui64);

/**
 * @brief Search the sleepy message queue for an entry with the specified sl_zigbee_af_sleepy_message_id_t.
 *
 * If a match was found, it copies the message into the sl_zigbee_af_sleepy_message_t structure pointer.
 *
 * @param sleepyMsgId The sl_zigbee_af_sleepy_message_id_t of the sl_zigbee_af_sleepy_message_t structure
 * that should be looked up in the sleepy message queue.
 * @param pmsg A pointer to an sl_zigbee_af_sleepy_message_t structure.
 *  If a message is found in the sleepy message queue with a matching sl_zigbee_af_sleepy_message_id_t,
 *  it will be copied to this structure.
 * @return true if a matching message was found or false if a match was not found.
 **/
bool sl_zigbee_af_sleepy_message_queue_get_pending_message(sl_zigbee_af_sleepy_message_id_t sleepyMsgId, sl_zigbee_af_sleepy_message_t *pmsg);

/**
 * @brief Return the time in milliseconds until the next message in the sleepy message queue will timeout.
 * @return The remaining time in milliseconds until the next message will timeout.
 **/
uint32_t sl_zigbee_af_sleepy_message_queue_get_next_message_event_timeout_ms(void);

/**
 * @brief Return the number of messages in the sleepy message queue that are buffered for a given sl_802154_long_addr_t.
 * @param dstEui64 The destination EUI64 that should be used to count matching messages in the sleepy message queue.
 * @return The number of messages in the sleepy message queue that are being sent to the specified sl_802154_long_addr_t.
 **/
uint8_t sl_zigbee_af_sleepy_message_queue_get_num_messages(sl_802154_long_addr_t dstEui64);

/**
 * @brief Remove the message from the sleepy message queue with the specified sl_zigbee_af_sleepy_message_id_t.
 * @param sleepyMsgId The sl_zigbee_af_sleepy_message_id_t that should be removed from the sleepy message queue.
 * @return true if a matching sl_zigbee_af_sleepy_message_id_t was found and removed from the sleepy message queue, or false if not.
 **/
bool sl_zigbee_af_sleepy_message_queue_remove_message(sl_zigbee_af_sleepy_message_id_t  sleepyMsgId);

/**
 * @brief Remove all messages from the sleepy message queue whose destination address matches the specified sl_802154_long_addr_t.
 * @param dstEui64 The sl_802154_long_addr_t to search for in the sleepy message queue.  All entries with a matching
 *  destination EUI64 should be removed.
 **/
void sl_zigbee_af_sleepy_message_queue_remove_all_messages(sl_802154_long_addr_t dstEui64);

/** @} */ // end of name API

/**
 * @name Callbacks
 * @{
 */

/**
 * @defgroup sleepy_msgs_queue_cb Sleepy Message Queue
 * @ingroup af_callback
 * @brief Callbacks for Sleepy Message Queue Component
 *
 */

/**
 * @addtogroup sleepy_msgs_queue_cb
 * @{
 */

/**
 * @brief Initialize the sleepy message queue.
 *
 * @param init_level  Initialize event and local data
 *                    - SL_ZIGBEE_INIT_LEVEL_EVENT (0x00)
 *                    - SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA (0x01)
 **/
void sl_zigbee_af_sleepy_message_queue_init_cb(uint8_t init_level);

/** @brief Message time out.
 *
 * This function is called by the sleepy message queue when a message times
 * out. The plugin will invalidate the entry in the queue after giving the
 * application a chance to perform any actions on the timed-out message.
 *
 * @param sleepyMsgId   Ver.: always
 */
void sl_zigbee_af_sleepy_message_queue_message_timed_out_cb(uint8_t sleepyMsgId);

/** @} */ // end of sleepy_msgs_queue_cb
/** @} */ // end of name Callbacks
/** @} */ // end of sleepy-message-queue

#endif  // #ifndef _SLEEPY_MESSAGE_QUEUE_H_
