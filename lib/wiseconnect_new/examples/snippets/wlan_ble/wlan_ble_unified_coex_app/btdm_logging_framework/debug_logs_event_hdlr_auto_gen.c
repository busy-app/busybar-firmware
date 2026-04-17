/*******************************************************************************
 * @file  debug_logs_event_hdlr_auto_gen.c.j2
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "debug_logs_event_hdlr_auto_gen.h"
#include "event_loop.h"
#include <stdlib.h>
#include <string.h>

generic_task_cb_t debug_logs_generic_cb;

generic_event_handler_lut_entry_t debug_logs_events_hdlr_lut[] = { { rsi_bt_on_controller_logs }, { NULL } };

/*==============================================*/
/**
 * @fn          void rsi_bt_on_controller_logs_driver_callback(uint16_t status, rsi_bt_event_controller_logs_t *rsi_bt_event_controller_logs)
 * @brief       Function enqueues received event data in driver context to debug_logs_generic_cb.event_queues to be processed in task context.
 * @param[in]   uint16_t status, event status.
 * @param[in]   rsi_bt_event_controller_logs_t *rsi_bt_event_controller_logs, event data.
 * @param[out]  None.
 * @return      None.
 *
 * @section description
 * This function enqueues event data received in driver context to debug_logs_generic_cb.event_queues to be processed in task context.
 *
 */

APP_WEAK void rsi_bt_on_controller_logs_driver_callback(uint16_t status,
                                                        rsi_bt_event_controller_logs_t *rsi_bt_event_controller_logs)
{
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_bt_event_controller_logs_t));
  //! assert if malloc failed
  if (msg == NULL) {
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  }

  //! init message details
  msg->next     = NULL;
  msg->event_id = controller_logs_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;

  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_bt_event_controller_logs, sizeof(rsi_bt_event_controller_logs_t));

  //! enqueue message to debug_logs_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&debug_logs_generic_cb.event_queues[0],
                                 (rsi_app_pkt_t *)msg,
                                 &debug_logs_generic_cb.event_mutex);

  //! post semaphore to signal the event
  osSemaphoreRelease(debug_logs_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_bt_on_controller_logs(uint16_t status, void *event_data)
 * @brief       Handler for controller_logs to be executed in task context.
 * @param[in]   uint16_t status, event status.
 * @param[in]   void *event_data, event data.
 * @return      None.
 *
 * @section description
 * Handler for controller_logs to be executed in task context.
 *
 */
APP_WEAK void rsi_bt_on_controller_logs(uint16_t __attribute__((unused)) status,
                                        void __attribute__((unused)) * event_data)
{
  //! Add handling here
}
