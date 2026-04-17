/***************************************************************************//**
 * @file message_ipc_callback_events.c
 * @brief callback event handlers for message
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
// automatically generated from message.h.  Do not manually edit
#include "stack/internal/src/ipc/message_ipc_callback_events.h"
#include "stack/internal/src/ipc/zigbee_ipc_callback_events.h"
#include "stack/include/multi-network.h"
extern void sl_zigbee_wakeup_common_task(void);
extern sl_status_t sl_zigbee_af_push_network_index(uint8_t networkIndex);
extern sl_status_t sl_zigbee_af_pop_network_index(void);

void sli_zigbee_stack_gpep_incoming_message_handler(sl_zigbee_gp_params_t *params)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));

  if (params != NULL) {
    cb_event->data.gpep_incoming_message_handler.params = *params;
  }

  cb_event->tag = SLI_ZIGBEE_STACK_GPEP_INCOMING_MESSAGE_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_id_conflict_handler(sl_802154_short_addr_t conflictingId)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.id_conflict_handler.conflictingId = conflictingId;
  cb_event->tag = SLI_ZIGBEE_STACK_ID_CONFLICT_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_incoming_many_to_one_route_request_handler(sl_802154_short_addr_t source,
                                                                 sl_802154_long_addr_t longId,
                                                                 uint8_t cost)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.incoming_many_to_one_route_request_handler.source = source;

  if (longId != NULL) {
    memmove(cb_event->data.incoming_many_to_one_route_request_handler.longId, longId, sizeof(sl_802154_long_addr_t));
  }

  cb_event->data.incoming_many_to_one_route_request_handler.cost = cost;
  cb_event->tag = SLI_ZIGBEE_STACK_INCOMING_MANY_TO_ONE_ROUTE_REQUEST_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_incoming_message_handler(sl_zigbee_incoming_message_type_t type,
                                               sl_zigbee_aps_frame_t *apsFrame,
                                               sl_zigbee_rx_packet_info_t *packetInfo,
                                               uint8_t messageLength,
                                               uint8_t *message)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.incoming_message_handler.type = type;

  if (apsFrame != NULL) {
    cb_event->data.incoming_message_handler.apsFrame = *apsFrame;
  }

  if (packetInfo != NULL) {
    cb_event->data.incoming_message_handler.packetInfo = *packetInfo;
  }

  cb_event->data.incoming_message_handler.messageLength = messageLength;

  if (message != NULL) {
    memmove(cb_event->data.incoming_message_handler.message, message, sizeof(uint8_t) * messageLength);
  }

  cb_event->tag = SLI_ZIGBEE_STACK_INCOMING_MESSAGE_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_incoming_network_status_handler(uint8_t errorCode,
                                                      sl_802154_short_addr_t target)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.incoming_network_status_handler.errorCode = errorCode;
  cb_event->data.incoming_network_status_handler.target = target;
  cb_event->tag = SLI_ZIGBEE_STACK_INCOMING_NETWORK_STATUS_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_incoming_route_error_handler(sl_status_t status,
                                                   sl_802154_short_addr_t target)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.incoming_route_error_handler.status = status;
  cb_event->data.incoming_route_error_handler.target = target;
  cb_event->tag = SLI_ZIGBEE_STACK_INCOMING_ROUTE_ERROR_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_incoming_route_record_handler(sl_zigbee_rx_packet_info_t *packetInfo,
                                                    uint8_t relayCount,
                                                    uint8_t *relayList)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));

  if (packetInfo != NULL) {
    cb_event->data.incoming_route_record_handler.packetInfo = *packetInfo;
  }

  cb_event->data.incoming_route_record_handler.relayCount = relayCount;

  if (relayList != NULL) {
    memmove(cb_event->data.incoming_route_record_handler.relayList, relayList, sizeof(uint8_t) * relayCount);
  }

  cb_event->tag = SLI_ZIGBEE_STACK_INCOMING_ROUTE_RECORD_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_message_sent_handler(sl_status_t status,
                                           sl_zigbee_outgoing_message_type_t type,
                                           uint16_t indexOrDestination,
                                           sl_zigbee_aps_frame_t *apsFrame,
                                           uint16_t messageTag,
                                           uint8_t messageLength,
                                           uint8_t *message)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.message_sent_handler.status = status;
  cb_event->data.message_sent_handler.type = type;
  cb_event->data.message_sent_handler.indexOrDestination = indexOrDestination;

  if (apsFrame != NULL) {
    cb_event->data.message_sent_handler.apsFrame = *apsFrame;
  }

  cb_event->data.message_sent_handler.messageTag = messageTag;
  cb_event->data.message_sent_handler.messageLength = messageLength;

  if (message != NULL) {
    memmove(cb_event->data.message_sent_handler.message, message, sizeof(uint8_t) * messageLength);
  }

  cb_event->tag = SLI_ZIGBEE_STACK_MESSAGE_SENT_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_override_incoming_route_record_handler(sl_zigbee_rx_packet_info_t *packetInfo,
                                                             uint8_t relayCount,
                                                             uint8_t *relayList,
                                                             bool *consumed)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));

  if (packetInfo != NULL) {
    cb_event->data.override_incoming_route_record_handler.packetInfo = *packetInfo;
  }

  cb_event->data.override_incoming_route_record_handler.relayCount = relayCount;

  if (relayList != NULL) {
    memmove(cb_event->data.override_incoming_route_record_handler.relayList, relayList, sizeof(uint8_t) * relayCount);
  }

  if (consumed != NULL) {
    cb_event->data.override_incoming_route_record_handler.consumed = *consumed;
  }

  cb_event->tag = SLI_ZIGBEE_STACK_OVERRIDE_INCOMING_ROUTE_RECORD_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_post_incoming_packet_filter_cb(sl_zigbee_zigbee_packet_type_t packetType,
                                                     uint8_t *packetData,
                                                     uint8_t size_p,
                                                     uint8_t *data,
                                                     uint8_t size_d,
                                                     sl_zigbee_packet_action_t action)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.post_incoming_packet_filter_cb.packetType = packetType;

  if (packetData != NULL) {
    memmove(cb_event->data.post_incoming_packet_filter_cb.packetData, packetData, sizeof(uint8_t) * size_p);
  }

  cb_event->data.post_incoming_packet_filter_cb.size_p = size_p;

  if (data != NULL) {
    memmove(cb_event->data.post_incoming_packet_filter_cb.data, data, sizeof(uint8_t) * size_d);
  }

  cb_event->data.post_incoming_packet_filter_cb.size_d = size_d;
  cb_event->data.post_incoming_packet_filter_cb.action = action;
  cb_event->tag = SLI_ZIGBEE_STACK_POST_INCOMING_PACKET_FILTER_CB_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_post_outgoing_packet_filter_cb(sl_zigbee_zigbee_packet_type_t packetType,
                                                     uint8_t *packetData,
                                                     uint8_t size_p,
                                                     uint8_t *data,
                                                     uint8_t size_d,
                                                     sl_zigbee_packet_action_t action)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.post_outgoing_packet_filter_cb.packetType = packetType;

  if (packetData != NULL) {
    memmove(cb_event->data.post_outgoing_packet_filter_cb.packetData, packetData, sizeof(uint8_t) * size_p);
  }

  cb_event->data.post_outgoing_packet_filter_cb.size_p = size_p;

  if (data != NULL) {
    memmove(cb_event->data.post_outgoing_packet_filter_cb.data, data, sizeof(uint8_t) * size_d);
  }

  cb_event->data.post_outgoing_packet_filter_cb.size_d = size_d;
  cb_event->data.post_outgoing_packet_filter_cb.action = action;
  cb_event->tag = SLI_ZIGBEE_STACK_POST_OUTGOING_PACKET_FILTER_CB_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_redirect_outgoing_message_handler(uint8_t mac_index,
                                                        uint8_t packet_length,
                                                        uint8_t *packet_contents,
                                                        uint8_t priority)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.redirect_outgoing_message_handler.mac_index = mac_index;
  cb_event->data.redirect_outgoing_message_handler.packet_length = packet_length;

  if (packet_contents != NULL) {
    memmove(cb_event->data.redirect_outgoing_message_handler.packet_contents, packet_contents, sizeof(uint8_t) * packet_length);
  }

  cb_event->data.redirect_outgoing_message_handler.priority = priority;
  cb_event->tag = SLI_ZIGBEE_STACK_REDIRECT_OUTGOING_MESSAGE_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_message_process_ipc_event(sl_zigbee_stack_cb_event_t *cb_event)
{
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  (void)sl_zigbee_af_push_network_index(cb_event->network_idx);
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  switch (cb_event->tag) {
    case SLI_ZIGBEE_STACK_GPEP_INCOMING_MESSAGE_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_gpep_incoming_message_handler(&cb_event->data.gpep_incoming_message_handler.params);
      break;

    case SLI_ZIGBEE_STACK_ID_CONFLICT_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_id_conflict_handler(cb_event->data.id_conflict_handler.conflictingId);
      break;

    case SLI_ZIGBEE_STACK_INCOMING_MANY_TO_ONE_ROUTE_REQUEST_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_incoming_many_to_one_route_request_handler(cb_event->data.incoming_many_to_one_route_request_handler.source,
                                                           cb_event->data.incoming_many_to_one_route_request_handler.longId,
                                                           cb_event->data.incoming_many_to_one_route_request_handler.cost);
      break;

    case SLI_ZIGBEE_STACK_INCOMING_MESSAGE_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_incoming_message_handler(cb_event->data.incoming_message_handler.type,
                                         &cb_event->data.incoming_message_handler.apsFrame,
                                         &cb_event->data.incoming_message_handler.packetInfo,
                                         cb_event->data.incoming_message_handler.messageLength,
                                         cb_event->data.incoming_message_handler.message);
      break;

    case SLI_ZIGBEE_STACK_INCOMING_NETWORK_STATUS_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_incoming_network_status_handler(cb_event->data.incoming_network_status_handler.errorCode,
                                                cb_event->data.incoming_network_status_handler.target);
      break;

    case SLI_ZIGBEE_STACK_INCOMING_ROUTE_ERROR_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_incoming_route_error_handler(cb_event->data.incoming_route_error_handler.status,
                                             cb_event->data.incoming_route_error_handler.target);
      break;

    case SLI_ZIGBEE_STACK_INCOMING_ROUTE_RECORD_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_incoming_route_record_handler(&cb_event->data.incoming_route_record_handler.packetInfo,
                                              cb_event->data.incoming_route_record_handler.relayCount,
                                              cb_event->data.incoming_route_record_handler.relayList);
      break;

    case SLI_ZIGBEE_STACK_MESSAGE_SENT_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_message_sent_handler(cb_event->data.message_sent_handler.status,
                                     cb_event->data.message_sent_handler.type,
                                     cb_event->data.message_sent_handler.indexOrDestination,
                                     &cb_event->data.message_sent_handler.apsFrame,
                                     cb_event->data.message_sent_handler.messageTag,
                                     cb_event->data.message_sent_handler.messageLength,
                                     cb_event->data.message_sent_handler.message);
      break;

    case SLI_ZIGBEE_STACK_OVERRIDE_INCOMING_ROUTE_RECORD_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_override_incoming_route_record_handler(&cb_event->data.override_incoming_route_record_handler.packetInfo,
                                                       cb_event->data.override_incoming_route_record_handler.relayCount,
                                                       cb_event->data.override_incoming_route_record_handler.relayList,
                                                       &cb_event->data.override_incoming_route_record_handler.consumed);
      break;

    case SLI_ZIGBEE_STACK_POST_INCOMING_PACKET_FILTER_CB_IPC_EVENT_TYPE:
      sl_zigbee_post_incoming_packet_filter_cb(cb_event->data.post_incoming_packet_filter_cb.packetType,
                                               cb_event->data.post_incoming_packet_filter_cb.packetData,
                                               cb_event->data.post_incoming_packet_filter_cb.size_p,
                                               cb_event->data.post_incoming_packet_filter_cb.data,
                                               cb_event->data.post_incoming_packet_filter_cb.size_d,
                                               cb_event->data.post_incoming_packet_filter_cb.action);
      break;

    case SLI_ZIGBEE_STACK_POST_OUTGOING_PACKET_FILTER_CB_IPC_EVENT_TYPE:
      sl_zigbee_post_outgoing_packet_filter_cb(cb_event->data.post_outgoing_packet_filter_cb.packetType,
                                               cb_event->data.post_outgoing_packet_filter_cb.packetData,
                                               cb_event->data.post_outgoing_packet_filter_cb.size_p,
                                               cb_event->data.post_outgoing_packet_filter_cb.data,
                                               cb_event->data.post_outgoing_packet_filter_cb.size_d,
                                               cb_event->data.post_outgoing_packet_filter_cb.action);
      break;

    case SLI_ZIGBEE_STACK_REDIRECT_OUTGOING_MESSAGE_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_redirect_outgoing_message_handler(cb_event->data.redirect_outgoing_message_handler.mac_index,
                                                  cb_event->data.redirect_outgoing_message_handler.packet_length,
                                                  cb_event->data.redirect_outgoing_message_handler.packet_contents,
                                                  cb_event->data.redirect_outgoing_message_handler.priority);
      break;

    default:
      /* do nothing */
      break;
  }
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_zigbee_af_pop_network_index();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
}
