/***************************************************************************//**
 * @file child_ipc_callback_events.c
 * @brief callback event handlers for child
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
// automatically generated from child.h.  Do not manually edit
#include "stack/internal/src/ipc/child_ipc_callback_events.h"
#include "stack/internal/src/ipc/zigbee_ipc_callback_events.h"
#include "stack/include/multi-network.h"
extern void sl_zigbee_wakeup_common_task(void);
extern sl_status_t sl_zigbee_af_push_network_index(uint8_t networkIndex);
extern sl_status_t sl_zigbee_af_pop_network_index(void);

void sli_zigbee_stack_child_join_handler(uint8_t index,
                                         bool joining,
                                         sl_802154_short_addr_t childId,
                                         sl_802154_long_addr_t childEui64,
                                         sl_zigbee_node_type_t childType)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.child_join_handler.index = index;
  cb_event->data.child_join_handler.joining = joining;
  cb_event->data.child_join_handler.childId = childId;

  if (childEui64 != NULL) {
    memmove(cb_event->data.child_join_handler.childEui64, childEui64, sizeof(sl_802154_long_addr_t));
  }

  cb_event->data.child_join_handler.childType = childType;
  cb_event->tag = SLI_ZIGBEE_STACK_CHILD_JOIN_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_poll_complete_handler(sl_status_t status)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.poll_complete_handler.status = status;
  cb_event->tag = SLI_ZIGBEE_STACK_POLL_COMPLETE_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_poll_handler(sl_802154_short_addr_t childId,
                                   bool transmitExpected)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.poll_handler.childId = childId;
  cb_event->data.poll_handler.transmitExpected = transmitExpected;
  cb_event->tag = SLI_ZIGBEE_STACK_POLL_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_child_process_ipc_event(sl_zigbee_stack_cb_event_t *cb_event)
{
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  (void)sl_zigbee_af_push_network_index(cb_event->network_idx);
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  switch (cb_event->tag) {
    case SLI_ZIGBEE_STACK_CHILD_JOIN_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_child_join_handler(cb_event->data.child_join_handler.index,
                                   cb_event->data.child_join_handler.joining,
                                   cb_event->data.child_join_handler.childId,
                                   cb_event->data.child_join_handler.childEui64,
                                   cb_event->data.child_join_handler.childType);
      break;

    case SLI_ZIGBEE_STACK_POLL_COMPLETE_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_poll_complete_handler(cb_event->data.poll_complete_handler.status);
      break;

    case SLI_ZIGBEE_STACK_POLL_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_poll_handler(cb_event->data.poll_handler.childId,
                             cb_event->data.poll_handler.transmitExpected);
      break;

    default:
      /* do nothing */
      break;
  }
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_zigbee_af_pop_network_index();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
}
