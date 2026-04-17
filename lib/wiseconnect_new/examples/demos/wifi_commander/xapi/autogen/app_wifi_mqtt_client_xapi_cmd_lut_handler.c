
/********************************************************************************
 * @file app_wifi_mqtt_client_xapi_cmd_lut_handler.c 
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

#include <stdio.h>

#include "app_internal_wifi_api.h"
#include "app_xapi.h"
#include "xapi.h"

void app_wifi_cmd_mqtt_client_init(const void *);
void app_wifi_cmd_mqtt_client_set_broker(const void *);
void app_wifi_cmd_mqtt_client_set_last_will_topic(const void *);
void app_wifi_cmd_mqtt_client_set_last_will_message(const void *);
void app_wifi_cmd_mqtt_client_set_client_config(const void *);
void app_wifi_cmd_mqtt_client_deinit(const void *);
void app_wifi_cmd_mqtt_client_get_broker(const void *);
void app_wifi_cmd_mqtt_client_get_last_will_topic(const void *);
void app_wifi_cmd_mqtt_client_get_last_will_message(const void *);
void app_wifi_cmd_mqtt_client_get_client_config(const void *);
void app_wifi_cmd_mqtt_client_connect(const void *);
void app_wifi_cmd_mqtt_client_disconnect(const void *);
void app_wifi_cmd_mqtt_client_subscribe(const void *);
void app_wifi_cmd_mqtt_client_unsubscribe(const void *);
void app_wifi_cmd_mqtt_client_publish_topic(const void *);
void app_wifi_cmd_mqtt_client_publish_message(const void *);
void app_wifi_cmd_mqtt_client_publish(const void *);
void app_wifi_cmd_mqtt_client_set_client_id(const void *);
void app_wifi_cmd_mqtt_client_get_client_id(const void *);

const struct xapi_cmd app_internal_xapi_cmd_table_wifi_mqtt_client[] = {
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_init,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_INIT_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_set_broker,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_SET_BROKER_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_set_last_will_topic,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_SET_LAST_WILL_TOPIC_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_set_last_will_message,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_SET_LAST_WILL_MESSAGE_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_set_client_config,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_SET_CLIENT_CONFIG_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_deinit,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_DEINIT_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_get_broker,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_GET_BROKER_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_get_last_will_topic,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_GET_LAST_WILL_TOPIC_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_get_last_will_message,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_GET_LAST_WILL_MESSAGE_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_get_client_config,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_GET_CLIENT_CONFIG_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_connect,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_CONNECT_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_disconnect,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_DISCONNECT_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_subscribe,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_SUBSCRIBE_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_unsubscribe,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_UNSUBSCRIBE_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_publish_topic,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_PUBLISH_TOPIC_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_publish_message,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_PUBLISH_MESSAGE_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_publish,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_PUBLISH_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_set_client_id,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_SET_CLIENT_ID_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_mqtt_client_get_client_id,
    .class_id = APP_INTERNAL_WIFI_MQTT_CLIENT_CLASS_ID,
    .command_id = APP_WIFI_MQTT_CLIENT_GET_CLIENT_ID_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  { 0 }
};