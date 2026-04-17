/********************************************************************************
 * @file  app_xapi_cmd_event.c
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
#include "sl_status.h"
#include "app_wifi_xapi_cmd_rx_handler.h"
#include "si91x_device.h"
#include "sl_net.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_driver.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "app_xapi_cmd_event.h"
#include "app_xapi_global.h"

osThreadId_t cmd_event_id = NULL;
QueueHandle_t queue_fd;

void wifi_cmd_event_init(void)
{
  queue_fd = xQueueCreate(10, sizeof(event_data_t *));

  if (queue_fd == NULL) {
    app_wifi_evt_system_error(SL_STATUS_ALLOCATION_FAILED, 0, NULL);
    return;
  }

  if (NULL == cmd_event_id) {
    const osThreadAttr_t attr = {

      .name       = "cmd_event",
      .priority   = osPriorityRealtime,
      .stack_mem  = 0,
      .stack_size = 1636,
      .cb_mem     = 0,
      .cb_size    = 0,
      .attr_bits  = 0u,
      .tz_module  = 0u,
    };
    cmd_event_id = osThreadNew(cmd_event_thread, NULL, &attr);

    if (cmd_event_id == NULL) {
      app_wifi_evt_system_error(SL_STATUS_ALLOCATION_FAILED, 0, NULL);
    }
  }
  return;
}

void cmd_event_thread(void *args)
{
  UNUSED_PARAMETER(args);
  event_data_t *data; // Pointer to event_data_t structure
  uint8_t event_id;
  sl_status_t status;
  volatile bool reap = FALSE;

  while (1) {
    // Receive the event_data_t structure from the queue_fd
    reap = xQueueReceive(queue_fd, &data, 1000); //portMAX_DELAY

    if (reap) {
      event_id = data->event_id;

      // Process the event based on the event_id
      switch (event_id) {
        case APP_NET_UP_EVENT: {
          app_wifi_cmd_net_intf_up_t *cmd = (app_wifi_cmd_net_intf_up_t *)(data->data);

          // Call the sl_net_up function
          status = sl_net_up(cmd->net_interface, cmd->profile_id);

          if (status != SL_STATUS_OK) {
            app_wifi_evt_net_intf_network_up_completed(cmd->net_interface, (status == SL_STATUS_OK));
          } else {
            app_wifi_evt_net_intf_network_up_completed(cmd->net_interface, (status == SL_STATUS_OK));
            set_default_net_interface(cmd->net_interface);
          }
          break;
        }
        default: {
          break;
        }
      }
      free(data->data);
      free(data);
    }
  }
}

QueueHandle_t wifi_cmd_get_queue_handle(void)
{
  return queue_fd;
}
