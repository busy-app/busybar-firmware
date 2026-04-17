/********************************************************************************
 * @file  app_wifi_commander.c
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

#include "app_wifi_api.h"

#include "stdlib.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "xapi.h"
#include "app_wifi_class.h"
#include "app_wifi_xapi_cmd_rx_handler.h"
#include "app_ncp.h"

#define APP_WIFI_EVENT_HANDLER_TASK_NAME  "app_wifi_event_handler"
#define APP_WIFI_EVENT_HANDLER_TASK_STACK 1024
#define APP_WIFI_EVENT_HANDLER_TASK_PRIO  5
#define APP_WIFI_EVT_QUEUE_SIZE           10

static TaskHandle_t app_wifi_event_handler_task_handle;
static void app_wifi_event_handler_task(void *p_arg);

QueueHandle_t app_wifi_evt_queue           = NULL;
static struct xapi_device xapi_wifi_device = {
  .dev_type = app_xapi_dev_type_wifi,
};

/** @brief Table of used XAPI classes */
// These are template contributed by components under normal circumstances
// to optimize for code size by referencing the used classes only.
// Proofs of concept and early development can use a table of all classes.
static const struct app_internal_xapi_class *const app_wifi_class_table[] = { APP_WIFI_XAPI_CLASS(system),
                                                                              APP_WIFI_XAPI_CLASS(net_intf),
                                                                              APP_WIFI_XAPI_CLASS(net_profile),
                                                                              APP_WIFI_XAPI_CLASS(ap),
                                                                              APP_WIFI_XAPI_CLASS(net_cred),
                                                                              APP_WIFI_XAPI_CLASS(common),
                                                                              APP_WIFI_XAPI_CLASS(scan),
                                                                              APP_WIFI_XAPI_CLASS(ping),
                                                                              APP_WIFI_XAPI_CLASS(client),
                                                                              APP_WIFI_XAPI_CLASS(mqtt_client),
                                                                              NULL };

/********************************************************************
 *  APIs to handle XAPI event to host                              *
 *********************************************************************/
sl_status_t app_wifi_init_classes(const struct app_internal_xapi_class *const *classes)
{
  xapi_wifi_device.lib_table = (const struct xapi_lib *const *)classes;
  return xapi_init_device(&xapi_wifi_device);
}

sl_status_t app_wifi_init_device(void)
{
  app_wifi_evt_queue = xQueueCreate(APP_WIFI_EVT_QUEUE_SIZE, sizeof(void *));
  if (app_wifi_evt_queue == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  xTaskCreate(app_wifi_event_handler_task,
              APP_WIFI_EVENT_HANDLER_TASK_NAME,
              APP_WIFI_EVENT_HANDLER_TASK_STACK,
              NULL,
              APP_WIFI_EVENT_HANDLER_TASK_PRIO,
              &app_wifi_event_handler_task_handle);
  if (app_wifi_event_handler_task_handle == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  xapi_init();
  return app_wifi_init_classes(app_wifi_class_table);
}

sl_status_t app_wifi_pop_event(app_wifi_msg_t *event)
{
  app_wifi_msg_t *wifi_packet;
  BaseType_t result = xQueueReceive(app_wifi_evt_queue, &wifi_packet, portMAX_DELAY);
  if (result != pdPASS) {
    return SL_STATUS_NOT_FOUND;
  }

  uint32_t wifi_packet_size = APP_WIFI_MSG_HEADER_LEN + APP_WIFI_MSG_LEN(wifi_packet->header);
  memcpy(event, wifi_packet, wifi_packet_size);
  free(wifi_packet);
  return SL_STATUS_OK;
}

sl_status_t app_wifi_push_event(app_wifi_msg_t *event)
{
  const uint32_t msg_size = APP_XAPI_MSG_HEADER_LEN + APP_XAPI_MSG_LEN(event->header);
  uint8_t *msg            = malloc(msg_size);
  if (msg == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  memcpy(msg, event, msg_size);
  BaseType_t result = xQueueSendToBack(app_wifi_evt_queue, &msg, portMAX_DELAY);
  if (result == pdPASS) {
    return SL_STATUS_OK;
  } else {
    free(msg);
    return SL_STATUS_FULL;
  }
}

sl_status_t app_wifi_push_event_with_data(app_wifi_msg_t *event, size_t data_len, const void *data)
{
  uint8_t dev_type                   = APP_XAPI_MSG_DEVICE_TYPE(event->header);
  uint8_t class_id                   = APP_INTERNAL_XAPI_MSG_CLASS_ID(event->header);
  uint8_t command_id                 = API_INTERNAL_XAPI_MSG_COMMAND_ID(event->header);
  uint32_t msg_data_fix_size         = APP_XAPI_MSG_LEN(event->header);
  uint32_t msg_fix_size              = APP_XAPI_MSG_HEADER_LEN + msg_data_fix_size;
  uint32_t msg_data_var_array_offset = msg_fix_size - 1;
  uint32_t msg_total_size            = msg_fix_size + data_len;
  uint8_t *msg                       = malloc(msg_total_size);
  if (msg == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  event->header = API_INTERNAL_XAPI_MSG_HEADER(class_id,
                                               command_id,
                                               (uint8_t)app_xapi_msg_type_evt | (uint8_t)dev_type,
                                               msg_data_fix_size + data_len);
  memcpy(msg, event, msg_fix_size);
  uint8array *data_array = (uint8array *)&msg[msg_data_var_array_offset];
  data_array->len        = (uint8_t)data_len;
  memcpy(data_array->data, data, data_len);

  BaseType_t result = xQueueSendToBack(app_wifi_evt_queue, &msg, portMAX_DELAY);
  if (result == pdPASS) {
    return SL_STATUS_OK;
  } else {
    free(msg);
    return SL_STATUS_FULL;
  }
}

static void app_wifi_event_handler_task(void *p_arg)
{
  (void)p_arg;
  sl_status_t sc;
  app_wifi_msg_t event;
  while (1) {
    sc = app_wifi_pop_event(&event);
    if (sc == SL_STATUS_OK) {
      app_wifi_on_event(&event);
    }
  }
}

/**********************************************************
 *  APIs to put error event (Used by NCP component)                               *
 **********************************************************/
void app_wifi_send_system_error(uint16_t reason, uint8_t data_len, const uint8_t *data)
{
  app_wifi_evt_system_error(reason, data_len, data);
}

/********************************************************************
 *  APIs to handle XAPI command (Used by NCP Component)                           *
 *********************************************************************/
void app_wifi_handle_command(uint32_t header, void *data)
{
  app_xapi_handle_command(header, data);
}