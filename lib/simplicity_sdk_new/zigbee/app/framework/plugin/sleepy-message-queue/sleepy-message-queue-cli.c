/***************************************************************************//**
 * @file
 * @brief Routines for managing messages for sleepy end devices.
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

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "sleepy-message-queue.h"

// Functions called in this .cli file that are defined elsewhere.
//extern sl_zigbee_af_sleepy_message_id_t sl_zigbee_af_sleepy_message_queue_store_message( sl_zigbee_af_sleepy_message_t *pmsg, uint32_t timeoutSec );
//extern sl_zigbee_af_sleepy_message_id_t sl_zigbee_af_sleepy_message_queue_get_pending_message_id( sl_802154_long_addr_t dstEui64 );
//extern bool sl_zigbee_af_sleepy_message_queue_get_pending_message( sl_zigbee_af_sleepy_message_id_t sleepyMsgId, sl_zigbee_af_sleepy_message_t *pmsg );
//extern uint8_t sl_zigbee_af_sleepy_message_queue_get_num_messages( sl_802154_long_addr_t dstEui64 );

#include "sl_cli.h"

// plugin sleepy-message-queue init <type:1>
void sl_zigbee_af_sleepy_message_queue_init_cli_cb(sl_cli_command_arg_t *arguments)
{
  uint8_t init_level;

  init_level = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_af_sleepy_message_queue_init_cb(init_level);
}

void sli_zigbee_af_sleepy_message_queue_cli_get_next_message_event_timeout_ms(sl_cli_command_arg_t *arguments)
{
  uint32_t timeoutMs;
  uint32_t msTick;
  uint8_t msgId;

  msgId = sl_cli_get_argument_uint8(arguments, 0);
  timeoutMs = sli_zigbee_af_message_m_sec_remaining(msgId);
  msTick = halCommonGetInt32uMillisecondTick();
  //sl_zigbee_af_app_println("==== REM TIME=%d msec, evtTmout=%d, msTick=%d", timeoutMs, msgTimeoutEvent.timeToExecute, msTick );
  sl_zigbee_af_app_println("Remaining Time=%d msec, msTick=%d", timeoutMs, msTick);
}

// plugin sleepy-message-queue unusedCnt
void sli_zigbee_af_sleepy_message_queue_cli_get_num_unused_entries(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  uint8_t cnt;
  cnt = sl_zigbee_af_sleepy_message_queue_get_num_unused_entries();
  sl_zigbee_af_app_println("Unused Message Queue Entries=%d", cnt);
}

// plugin sleepy-message-queue store <timeoutSec:4> <payload*:4> <payloadLength:2> <payloadId:2> <destEui64:8>
void sli_zigbee_af_sleepy_message_queue_cli_store(sl_cli_command_arg_t *arguments)
{
  //uint8_t *payload;
  //uint16_t length;
  //uint16_t payloadId;
  //sl_802154_long_addr_t dstEui64;
  sl_zigbee_af_sleepy_message_t msg;
  sl_zigbee_af_sleepy_message_id_t msgId;

  uint32_t timeoutSec;

  timeoutSec = sl_cli_get_argument_uint32(arguments, 0);
  msg.payload = (uint8_t *)sl_cli_get_argument_string(arguments, 1);
  msg.length = sl_cli_get_argument_uint16(arguments, 2);
  msg.payloadId = sl_cli_get_argument_uint16(arguments, 3);
  sl_zigbee_copy_hex_arg(arguments, 4, msg.dstEui64, EUI64_SIZE, false);

  msgId = sl_zigbee_af_sleepy_message_queue_store_message(&msg, timeoutSec);
  if ( msgId != SL_ZIGBEE_AF_PLUGIN_SLEEPY_MESSAGE_INVALID_ID ) {
    sl_zigbee_af_app_println("Message Stored, msgID=%d, payloadID=%d", msgId, msg.payloadId);
  } else {
    sl_zigbee_af_app_println("ERROR - Message not stored");
  }
}

// plugin sleepy-message-queue getPendingMsgId <destEui64:8>
void sli_zigbee_af_sleepy_message_queue_cli_get_pending_msg_id(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_sleepy_message_id_t msgId;
  sl_802154_long_addr_t eui64;

  sl_zigbee_copy_hex_arg(arguments, 0, eui64, EUI64_SIZE, false);
  msgId = sl_zigbee_af_sleepy_message_queue_get_pending_message_id(eui64);
  sl_zigbee_af_app_println("Message Pending ID=%d", msgId);
}

// plugin sleepy-message-queue getPendingMsg <msgId:1>
void sli_zigbee_af_sleepy_message_queue_cli_get_pending_msg(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_sleepy_message_t   msg;
  sl_zigbee_af_sleepy_message_id_t msgId;
  bool found;

  msgId = sl_cli_get_argument_uint8(arguments, 0);
  found = sl_zigbee_af_sleepy_message_queue_get_pending_message(msgId, &msg);
  //sl_zigbee_af_app_println("Pending Msg Found=%d, payload=%d, length=%d, ID=%d",
  //    found, (uint32_t)msg.payload, msg.length, msg.payloadId );
  sl_zigbee_af_app_println("Pending Msg Found=%d", found);
}

// plugin sleepy-message-queue getNumMsg <destEui64:8>
void sli_zigbee_af_sleepy_message_queue_cli_get_num_pending_msg(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t eui64;
  uint8_t cnt;

  sl_zigbee_copy_hex_arg(arguments, 0, eui64, EUI64_SIZE, false);
  cnt = sl_zigbee_af_sleepy_message_queue_get_num_messages(eui64);
  sl_zigbee_af_app_println("Pending Msg Count=%d", cnt);
}

// plugin sleepy-message-queue remove <msgId:1>
void sli_zigbee_af_sleepy_message_queue_cli_remove_msg_id(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_sleepy_message_id_t msgId;
  bool status;

  msgId = sl_cli_get_argument_uint8(arguments, 0);
  status = sl_zigbee_af_sleepy_message_queue_remove_message(msgId);
  if ( status == true ) {
    sl_zigbee_af_app_println("Removed Msg ID=%d", msgId);
  } else {
    sl_zigbee_af_app_println("ERROR - Msg Remove Failed");
  }
}

// plugin sleepy-message-queue removeAll <destEui64:8>
void sli_zigbee_af_sleepy_message_queue_cli_remove_all_msg(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t eui64;
  sl_zigbee_copy_hex_arg(arguments, 0, eui64, EUI64_SIZE, false);
  sl_zigbee_af_sleepy_message_queue_remove_all_messages(eui64);
  sl_zigbee_af_app_println("Removed All Msgs from {%02X %02X %02X %02X %02X %02X %02X %02X}",
                           eui64[0], eui64[1], eui64[2], eui64[3], eui64[4], eui64[5], eui64[6], eui64[7]);
}

void sli_zigbee_af_sleepy_message_queue_cli_get_current_int32u_millisecond_tick()
{
  uint32_t tickMs;

  tickMs = halCommonGetInt32uMillisecondTick();
  sl_zigbee_af_app_println("MS Tick=%d", tickMs);
}
