/***************************************************************************//**
 * @file binding-table-ipc-callback-events.c
 * @brief callback event handlers for binding-table
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
// automatically generated from binding-table.h.  Do not manually edit
#include "stack/internal/src/ipc/binding-table-ipc-callback-events.h"
#include "stack/internal/src/ipc/zigbee_ipc_callback_events.h"
#include "stack/include/multi-network.h"
extern void sl_zigbee_wakeup_common_task(void);
extern sl_status_t sl_zigbee_af_push_network_index(uint8_t networkIndex);
extern sl_status_t sl_zigbee_af_pop_network_index(void);

void sli_zigbee_stack_remote_delete_binding_handler(uint8_t index,
                                                    sl_zigbee_zdo_status_t status)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.remote_delete_binding_handler.index = index;
  cb_event->data.remote_delete_binding_handler.status = status;
  cb_event->tag = SLI_ZIGBEE_STACK_REMOTE_DELETE_BINDING_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_remote_set_binding_handler(sl_zigbee_binding_table_entry_t *entry,
                                                 sl_zigbee_zdo_status_t status)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));

  if (entry != NULL) {
    cb_event->data.remote_set_binding_handler.entry = *entry;
  }

  cb_event->data.remote_set_binding_handler.status = status;
  cb_event->tag = SLI_ZIGBEE_STACK_REMOTE_SET_BINDING_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_binding_table_process_ipc_event(sl_zigbee_stack_cb_event_t *cb_event)
{
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  (void)sl_zigbee_af_push_network_index(cb_event->network_idx);
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  switch (cb_event->tag) {
    case SLI_ZIGBEE_STACK_REMOTE_DELETE_BINDING_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_remote_delete_binding_handler(cb_event->data.remote_delete_binding_handler.index,
                                              cb_event->data.remote_delete_binding_handler.status);
      break;

    case SLI_ZIGBEE_STACK_REMOTE_SET_BINDING_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_remote_set_binding_handler(&cb_event->data.remote_set_binding_handler.entry,
                                           cb_event->data.remote_set_binding_handler.status);
      break;

    default:
      /* do nothing */
      break;
  }
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_zigbee_af_pop_network_index();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
}
