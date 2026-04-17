/***************************************************************************//**
 * @file raw-message-ipc-callback-events.c
 * @brief callback event handlers for raw-message
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
// automatically generated from raw-message.h.  Do not manually edit
#include "stack/internal/src/ipc/raw-message-ipc-callback-events.h"
#include "stack/internal/src/ipc/zigbee_ipc_callback_events.h"
#include "stack/include/multi-network.h"
extern void sl_zigbee_wakeup_common_task(void);
extern sl_status_t sl_zigbee_af_push_network_index(uint8_t networkIndex);
extern sl_status_t sl_zigbee_af_pop_network_index(void);

void sli_802154_stack_filter_match_message_handler(sl_zigbee_mac_filter_match_data_t filterValueMatch,
                                                   uint8_t legacyPassthroughType,
                                                   sl_zigbee_rx_packet_info_t *packetInfo,
                                                   uint8_t messageLength,
                                                   uint8_t *messageContents)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.filter_match_message_handler.filterValueMatch = filterValueMatch;
  cb_event->data.filter_match_message_handler.legacyPassthroughType = legacyPassthroughType;

  if (packetInfo != NULL) {
    cb_event->data.filter_match_message_handler.packetInfo = *packetInfo;
  }

  cb_event->data.filter_match_message_handler.messageLength = messageLength;

  if (messageContents != NULL) {
    memmove(cb_event->data.filter_match_message_handler.messageContents, messageContents, sizeof(uint8_t) * messageLength);
  }

  cb_event->tag = SLI_802154_STACK_FILTER_MATCH_MESSAGE_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_802154_stack_passthrough_message_handler(sl_zigbee_mac_passthrough_type_t messageType,
                                                  sl_zigbee_rx_packet_info_t *packetInfo,
                                                  uint8_t messageLength,
                                                  uint8_t *messageContents)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.passthrough_message_handler.messageType = messageType;

  if (packetInfo != NULL) {
    cb_event->data.passthrough_message_handler.packetInfo = *packetInfo;
  }

  cb_event->data.passthrough_message_handler.messageLength = messageLength;

  if (messageContents != NULL) {
    memmove(cb_event->data.passthrough_message_handler.messageContents, messageContents, sizeof(uint8_t) * messageLength);
  }

  cb_event->tag = SLI_802154_STACK_PASSTHROUGH_MESSAGE_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_raw_transmit_complete_handler(uint8_t messageLength,
                                                    uint8_t *messageContents,
                                                    sl_status_t status,
                                                    uint8_t messageTag)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.raw_transmit_complete_handler.messageLength = messageLength;

  if (messageContents != NULL) {
    memmove(cb_event->data.raw_transmit_complete_handler.messageContents, messageContents, sizeof(uint8_t) * messageLength);
  }

  cb_event->data.raw_transmit_complete_handler.status = status;
  cb_event->data.raw_transmit_complete_handler.messageTag = messageTag;
  cb_event->tag = SLI_ZIGBEE_STACK_RAW_TRANSMIT_COMPLETE_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_raw_message_process_ipc_event(sl_zigbee_stack_cb_event_t *cb_event)
{
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  (void)sl_zigbee_af_push_network_index(cb_event->network_idx);
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  switch (cb_event->tag) {
    case SLI_802154_STACK_FILTER_MATCH_MESSAGE_HANDLER_IPC_EVENT_TYPE:
      sl_802154_filter_match_message_handler(cb_event->data.filter_match_message_handler.filterValueMatch,
                                             cb_event->data.filter_match_message_handler.legacyPassthroughType,
                                             &cb_event->data.filter_match_message_handler.packetInfo,
                                             cb_event->data.filter_match_message_handler.messageLength,
                                             cb_event->data.filter_match_message_handler.messageContents);
      break;

    case SLI_802154_STACK_PASSTHROUGH_MESSAGE_HANDLER_IPC_EVENT_TYPE:
      sl_802154_passthrough_message_handler(cb_event->data.passthrough_message_handler.messageType,
                                            &cb_event->data.passthrough_message_handler.packetInfo,
                                            cb_event->data.passthrough_message_handler.messageLength,
                                            cb_event->data.passthrough_message_handler.messageContents);
      break;

    case SLI_ZIGBEE_STACK_RAW_TRANSMIT_COMPLETE_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_raw_transmit_complete_handler(cb_event->data.raw_transmit_complete_handler.messageLength,
                                              cb_event->data.raw_transmit_complete_handler.messageContents,
                                              cb_event->data.raw_transmit_complete_handler.status,
                                              cb_event->data.raw_transmit_complete_handler.messageTag);
      break;

    default:
      /* do nothing */
      break;
  }
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_zigbee_af_pop_network_index();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
}
