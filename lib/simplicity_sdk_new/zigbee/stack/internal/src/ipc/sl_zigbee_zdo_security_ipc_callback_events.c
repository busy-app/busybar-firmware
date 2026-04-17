/***************************************************************************//**
 * @file sl_zigbee_zdo_security_ipc_callback_events.c
 * @brief callback event handlers for sl_zigbee_zdo_security
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
// automatically generated from sl_zigbee_zdo_security.h.  Do not manually edit
#include "stack/internal/src/ipc/sl_zigbee_zdo_security_ipc_callback_events.h"
#include "stack/internal/src/ipc/zigbee_ipc_callback_events.h"
#include "stack/include/multi-network.h"
extern void sl_zigbee_wakeup_common_task(void);
extern sl_status_t sl_zigbee_af_push_network_index(uint8_t networkIndex);
extern sl_status_t sl_zigbee_af_pop_network_index(void);

void sli_zigbee_stack_get_authentication_level_callback(sl_zigbee_zdo_status_t rsp_status,
                                                        sl_802154_long_addr_t target,
                                                        uint8_t join_method,
                                                        uint8_t link_key_update)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.get_authentication_level_callback.rsp_status = rsp_status;

  if (target != NULL) {
    memmove(cb_event->data.get_authentication_level_callback.target, target, sizeof(sl_802154_long_addr_t));
  }

  cb_event->data.get_authentication_level_callback.join_method = join_method;
  cb_event->data.get_authentication_level_callback.link_key_update = link_key_update;
  cb_event->tag = SLI_ZIGBEE_STACK_GET_AUTHENTICATION_LEVEL_CALLBACK_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_set_authenticaion_level_callback(sl_802154_long_addr_t target,
                                                       sl_zigbee_initial_join_method *initial_join_method,
                                                       sl_zigbee_active_link_key_type *active_link_key_type)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));

  if (target != NULL) {
    memmove(cb_event->data.set_authenticaion_level_callback.target, target, sizeof(sl_802154_long_addr_t));
  }

  if (initial_join_method != NULL) {
    cb_event->data.set_authenticaion_level_callback.initial_join_method = *initial_join_method;
  }

  if (active_link_key_type != NULL) {
    cb_event->data.set_authenticaion_level_callback.active_link_key_type = *active_link_key_type;
  }

  cb_event->tag = SLI_ZIGBEE_STACK_SET_AUTHENTICAION_LEVEL_CALLBACK_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_zdo_retrieve_authentication_token_complete_callback(sl_status_t status)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.zdo_retrieve_authentication_token_complete_callback.status = status;
  cb_event->tag = SLI_ZIGBEE_STACK_ZDO_RETRIEVE_AUTHENTICATION_TOKEN_COMPLETE_CALLBACK_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_sl_zigbee_zdo_security_process_ipc_event(sl_zigbee_stack_cb_event_t *cb_event)
{
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  (void)sl_zigbee_af_push_network_index(cb_event->network_idx);
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  switch (cb_event->tag) {
    case SLI_ZIGBEE_STACK_GET_AUTHENTICATION_LEVEL_CALLBACK_IPC_EVENT_TYPE:
      sl_zigbee_get_authentication_level_callback(cb_event->data.get_authentication_level_callback.rsp_status,
                                                  cb_event->data.get_authentication_level_callback.target,
                                                  cb_event->data.get_authentication_level_callback.join_method,
                                                  cb_event->data.get_authentication_level_callback.link_key_update);
      break;

    case SLI_ZIGBEE_STACK_SET_AUTHENTICAION_LEVEL_CALLBACK_IPC_EVENT_TYPE:
      sl_zigbee_set_authenticaion_level_callback(cb_event->data.set_authenticaion_level_callback.target,
                                                 &cb_event->data.set_authenticaion_level_callback.initial_join_method,
                                                 &cb_event->data.set_authenticaion_level_callback.active_link_key_type);
      break;

    case SLI_ZIGBEE_STACK_ZDO_RETRIEVE_AUTHENTICATION_TOKEN_COMPLETE_CALLBACK_IPC_EVENT_TYPE:
      sl_zigbee_zdo_retrieve_authentication_token_complete_callback(cb_event->data.zdo_retrieve_authentication_token_complete_callback.status);
      break;

    default:
      /* do nothing */
      break;
  }
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_zigbee_af_pop_network_index();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
}
