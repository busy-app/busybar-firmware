/***************************************************************************//**
 * @file sl_zigbee_duty_cycle_ipc_callback_events.c
 * @brief callback event handlers for sl_zigbee_duty_cycle
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
// automatically generated from sl_zigbee_duty_cycle.h.  Do not manually edit
#include "stack/internal/src/ipc/sl_zigbee_duty_cycle_ipc_callback_events.h"
#include "stack/internal/src/ipc/zigbee_ipc_callback_events.h"
#include "stack/include/multi-network.h"
extern void sl_zigbee_wakeup_common_task(void);
extern sl_status_t sl_zigbee_af_push_network_index(uint8_t networkIndex);
extern sl_status_t sl_zigbee_af_pop_network_index(void);

void sli_zigbee_stack_duty_cycle_handler(uint8_t channelPage,
                                         uint8_t channel,
                                         sl_zigbee_duty_cycle_state_t state,
                                         uint8_t totalDevices,
                                         sl_zigbee_per_device_duty_cycle_t *arrayOfDeviceDutyCycles)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.duty_cycle_handler.channelPage = channelPage;
  cb_event->data.duty_cycle_handler.channel = channel;
  cb_event->data.duty_cycle_handler.state = state;
  cb_event->data.duty_cycle_handler.totalDevices = totalDevices;

  if (arrayOfDeviceDutyCycles != NULL) {
    memmove(cb_event->data.duty_cycle_handler.arrayOfDeviceDutyCycles, arrayOfDeviceDutyCycles, sizeof(sl_zigbee_per_device_duty_cycle_t) * totalDevices);
  }

  cb_event->tag = SLI_ZIGBEE_STACK_DUTY_CYCLE_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_sl_zigbee_duty_cycle_process_ipc_event(sl_zigbee_stack_cb_event_t *cb_event)
{
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  (void)sl_zigbee_af_push_network_index(cb_event->network_idx);
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  switch (cb_event->tag) {
    case SLI_ZIGBEE_STACK_DUTY_CYCLE_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_duty_cycle_handler(cb_event->data.duty_cycle_handler.channelPage,
                                   cb_event->data.duty_cycle_handler.channel,
                                   cb_event->data.duty_cycle_handler.state,
                                   cb_event->data.duty_cycle_handler.totalDevices,
                                   cb_event->data.duty_cycle_handler.arrayOfDeviceDutyCycles);
      break;

    default:
      /* do nothing */
      break;
  }
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_zigbee_af_pop_network_index();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
}
