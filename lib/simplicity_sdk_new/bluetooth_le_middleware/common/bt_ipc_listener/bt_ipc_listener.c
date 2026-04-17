/***************************************************************************//**
 * @file
 * @brief Bluetooth IPC Listener Source
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include <stdbool.h>
#include "sl_status.h"
#include "sl_common.h"
#include "sl_bluetooth.h"
#include "cmsis_os2.h"
#include "sl_event_system.h"
#include "app_assert.h"
#include "bt_ipc_listener.h"
#include "bt_ipc_listener_config.h"

#define EVENT_CLASS             SL_EVENT_CLASS_BLUETOOTH
#define EVENT_CLASS_MASK        SL_BT_EVENT_MASK_PUBLIC
#define TASK_NAME               "bt_ipc_listener"

// Queue to store the incoming events.
static sl_event_queue_t event_queue;

// OS task for checking and handling incoming events.
static void event_handler_task(void *p_arg);

// Bluetooth IPC Listener initialization.
void bt_ipc_listener_init(void)
{
  // Create a queue to store the incoming events.
  sl_status_t sc;
  sc = sl_event_queue_create(BT_IPC_LISTENER_EVENT_QUEUE_SIZE, &event_queue);
  app_assert_status(sc);

  // Create an OS task for checking the incoming events.
  osThreadId_t task_id = NULL;
  osThreadAttr_t task_attributes = {
    .name       = TASK_NAME,
    .stack_size = BT_IPC_LISTENER_EVENT_HANDLER_STACK_SIZE,
    .priority   = (osPriority_t)BT_IPC_LISTENER_EVENT_HANDLER_TASK_PRIORITY
  };

  task_id = osThreadNew(event_handler_task, NULL, &task_attributes);
  app_assert_s(task_id != NULL);

  // Subscribe to event.
  sc = sl_event_subscribe(EVENT_CLASS, EVENT_CLASS_MASK, event_queue);
  app_assert_status(sc);
}

/******************************************************************************
 * OS task for checking and handling incoming events.
 *****************************************************************************/
static void event_handler_task(void *p_arg)
{
  (void)p_arg;
  sl_status_t sc;
  uint8_t event_prio;
  sl_event_t *event;

  while (1) {
    // Process the next event in the event queue
    sc = sl_event_queue_get(event_queue, &event_prio, osWaitForever, &event);

    if (sc == SL_STATUS_OK) {
      // Pass the event to the subscribers.
      sl_bt_process_event((sl_bt_msg_t *)event->event_data);

      // Signal the publisher that the event has been handled.
      (void)sl_event_process(&event);
    }
  }
}
