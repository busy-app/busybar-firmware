/***************************************************************************//**
 * @file zll-api-ipc-callback-events.c
 * @brief callback event handlers for zll-api
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
// automatically generated from zll-api.h.  Do not manually edit
#include "stack/internal/src/ipc/zll-api-ipc-callback-events.h"
#include "stack/internal/src/ipc/zigbee_ipc_callback_events.h"
#include "stack/include/multi-network.h"
extern void sl_zigbee_wakeup_common_task(void);
extern sl_status_t sl_zigbee_af_push_network_index(uint8_t networkIndex);
extern sl_status_t sl_zigbee_af_pop_network_index(void);

void sli_zigbee_stack_zll_address_assignment_handler(sl_zigbee_zll_address_assignment_t *addressInfo,
                                                     sl_zigbee_rx_packet_info_t *packetInfo)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));

  if (addressInfo != NULL) {
    cb_event->data.zll_address_assignment_handler.addressInfo = *addressInfo;
  }

  if (packetInfo != NULL) {
    cb_event->data.zll_address_assignment_handler.packetInfo = *packetInfo;
  }

  cb_event->tag = SLI_ZIGBEE_STACK_ZLL_ADDRESS_ASSIGNMENT_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_zll_network_found_handler(sl_zigbee_zll_network_t *networkInfo,
                                                bool isDeviceInfoNull,
                                                sl_zigbee_zll_device_info_record_t *deviceInfo,
                                                sl_zigbee_rx_packet_info_t *packetInfo)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));

  if (networkInfo != NULL) {
    cb_event->data.zll_network_found_handler.networkInfo = *networkInfo;
  }

  cb_event->data.zll_network_found_handler.isDeviceInfoNull = isDeviceInfoNull;

  if (deviceInfo != NULL) {
    cb_event->data.zll_network_found_handler.deviceInfo = *deviceInfo;
  }

  if (packetInfo != NULL) {
    cb_event->data.zll_network_found_handler.packetInfo = *packetInfo;
  }

  cb_event->tag = SLI_ZIGBEE_STACK_ZLL_NETWORK_FOUND_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_zll_scan_complete_handler(sl_status_t status)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.zll_scan_complete_handler.status = status;
  cb_event->tag = SLI_ZIGBEE_STACK_ZLL_SCAN_COMPLETE_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_zll_touch_link_target_handler(const sl_zigbee_zll_network_t *networkInfo)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));

  if (networkInfo != NULL) {
    cb_event->data.zll_touch_link_target_handler.networkInfo = *networkInfo;
  }

  cb_event->tag = SLI_ZIGBEE_STACK_ZLL_TOUCH_LINK_TARGET_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_zll_api_process_ipc_event(sl_zigbee_stack_cb_event_t *cb_event)
{
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  (void)sl_zigbee_af_push_network_index(cb_event->network_idx);
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  switch (cb_event->tag) {
    case SLI_ZIGBEE_STACK_ZLL_ADDRESS_ASSIGNMENT_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_zll_address_assignment_handler(&cb_event->data.zll_address_assignment_handler.addressInfo,
                                               &cb_event->data.zll_address_assignment_handler.packetInfo);
      break;

    case SLI_ZIGBEE_STACK_ZLL_NETWORK_FOUND_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_zll_network_found_handler(&cb_event->data.zll_network_found_handler.networkInfo,
                                          cb_event->data.zll_network_found_handler.isDeviceInfoNull,
                                          &cb_event->data.zll_network_found_handler.deviceInfo,
                                          &cb_event->data.zll_network_found_handler.packetInfo);
      break;

    case SLI_ZIGBEE_STACK_ZLL_SCAN_COMPLETE_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_zll_scan_complete_handler(cb_event->data.zll_scan_complete_handler.status);
      break;

    case SLI_ZIGBEE_STACK_ZLL_TOUCH_LINK_TARGET_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_zll_touch_link_target_handler(&cb_event->data.zll_touch_link_target_handler.networkInfo);
      break;

    default:
      /* do nothing */
      break;
  }
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_zigbee_af_pop_network_index();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
}
