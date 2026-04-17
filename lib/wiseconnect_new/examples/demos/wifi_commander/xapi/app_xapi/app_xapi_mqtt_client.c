/********************************************************************************
 * @file  app_xapi_mqtt_client.c
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
#include "task.h"
#include "sl_mqtt_client_types.h"
#include "sl_mqtt_client.h"

DEFINE_XAPI_CLASS(wifi, mqtt_client, WIFI, MQTT_CLIENT, NULL, NULL);

#define MAX_BUFFER_SIZE 48

static sl_mqtt_client_t client = { 0 };

static sl_mqtt_client_configuration_t mqtt_client_configuration = { 0 };

static sl_mqtt_broker_t mqtt_broker_configuration = { 0 };

static sl_mqtt_client_message_t mqtt_message = { 0 };

static sl_mqtt_client_last_will_message_t last_will_message = { 0 };

static uint8_t message_to_publish[MAX_BUFFER_SIZE];

static uint8_t will_message_buffer[MAX_BUFFER_SIZE];

static uint8_t topic_to_publish[MAX_BUFFER_SIZE];

static uint8_t last_will_topic_buffer[MAX_BUFFER_SIZE];

static uint8_t topic_to_subscribe[MAX_BUFFER_SIZE];

static uint8_t topic_to_unsubscribe[MAX_BUFFER_SIZE];

void mqtt_client_message_handler(void *client, sl_mqtt_client_message_t *message, void *context)
{
  // Unused parameters
  (void)client;
  (void)context;

  // Notify the application layer about the received MQTT topic and message content
  app_wifi_evt_mqtt_client_message_received_on_topic(message->topic_length, message->topic);
  app_wifi_evt_mqtt_client_message_received_content(message->content_length, message->content);
  return;
}

void mqtt_client_event_handler(void *client, sl_mqtt_client_event_t event, void *event_data, void *context)
{
  // Unused parameter
  (void)client;

  switch (event) {
    // MQTT client successfully connected to the broker
    case SL_MQTT_CLIENT_CONNECTED_EVENT:
      app_wifi_evt_mqtt_client_connected_event(1); // 1 indicates connected
      break;

    // MQTT client got disconnected from the broker
    case SL_MQTT_CLIENT_DISCONNECTED_EVENT:
      app_wifi_evt_mqtt_client_disconnected_event(1); // 1 indicates disconnected
      break;

    // MQTT client successfully subscribed to a topic
    case SL_MQTT_CLIENT_SUBSCRIBED_EVENT: {
      char *topic_buf = (char *)context;

      if (topic_buf != NULL) {
        size_t topic_len = strlen(topic_buf);
        app_wifi_evt_mqtt_client_subscribe_state_event(1, topic_len, topic_buf);
      }
      break;
    }

    // MQTT client successfully unsubscribed from a topic
    case SL_MQTT_CLIENT_UNSUBSCRIBED_EVENT: {
      char *topic_buff = (char *)context;

      if (topic_buff != NULL) {
        size_t topic_len = strlen(topic_buff);
        app_wifi_evt_mqtt_client_unsubscribe_state_event(1, topic_len, topic_buff);
      }
      break;
    }

    // MQTT message has been published successfully
    case SL_MQTT_CLIENT_MESSAGE_PUBLISHED_EVENT: {
      const sl_mqtt_client_message_t *msg = (const sl_mqtt_client_message_t *)context;

      if (msg != NULL && msg->topic != NULL) {
        size_t topic_len = strlen((const char *)msg->topic);
        app_wifi_evt_mqtt_client_publish_state_event(1, topic_len, msg->topic);
      }
      break;
    }

    // An error occurred in the MQTT client
    case SL_MQTT_CLIENT_ERROR_EVENT:
      app_wifi_evt_mqtt_client_error_event((sl_mqtt_client_error_status_t)event_data);
      break;

    // Default case for any unhandled events
    default:
      break;
  }
}

void app_wifi_cmd_mqtt_client_init(const void *nil)
{
  (void)nil; // Unused parameter

  // Set up the MQTT client with broker, last will, and configuration settings
  client.broker               = &mqtt_broker_configuration;
  client.last_will_message    = &last_will_message;
  client.client_configuration = &mqtt_client_configuration;

  // Initialize the MQTT client
  sl_status_t status = sl_mqtt_client_init(&client, mqtt_client_event_handler);
  app_wifi_rsp_mqtt_client_init(status);
  return;
}

void app_wifi_cmd_mqtt_client_set_broker(app_wifi_cmd_mqtt_client_set_broker_t *cmd_input)
{
  // Check for null input
  if (cmd_input == NULL) {
    app_wifi_rsp_mqtt_client_set_broker(SL_STATUS_FAIL);
    return;
  }

  // Get reference to global broker configuration
  sl_mqtt_broker_t *mqtt_broker_config = &mqtt_broker_configuration;

  // Apply broker parameters from the command input
  mqtt_broker_config->port                    = cmd_input->port;
  mqtt_broker_config->is_connection_encrypted = cmd_input->is_connection_encrypted;
  mqtt_broker_config->connect_timeout         = cmd_input->connect_timeout;
  mqtt_broker_config->keep_alive_interval     = cmd_input->keep_alive_interval;
  mqtt_broker_config->keep_alive_retries      = cmd_input->keep_alive_retries;

  // Reverse the IP address byte order to match expected format (MSB to LSB)
  mqtt_broker_config->ip.ip.v4.bytes[0] = cmd_input->ip_address.addr[0];
  mqtt_broker_config->ip.ip.v4.bytes[1] = cmd_input->ip_address.addr[1];
  mqtt_broker_config->ip.ip.v4.bytes[2] = cmd_input->ip_address.addr[2];
  mqtt_broker_config->ip.ip.v4.bytes[3] = cmd_input->ip_address.addr[3];

  // Respond with success
  app_wifi_rsp_mqtt_client_set_broker(SL_STATUS_OK);
  return;
}

void app_wifi_cmd_mqtt_client_set_last_will_topic(app_wifi_cmd_mqtt_client_set_last_will_topic_t *cmd_input)
{
  // Get reference to global last will message
  sl_mqtt_client_last_will_message_t *will_message = &last_will_message;

  // Set last will flags and QoS level
  will_message->is_retained    = (cmd_input->is_retained != 0);
  will_message->will_qos_level = (sl_mqtt_qos_t)cmd_input->qos_level;

  // Copy topic data safely and null-terminate
  memcpy(last_will_topic_buffer, cmd_input->topic.data, cmd_input->topic.len);
  last_will_topic_buffer[cmd_input->topic.len] = '\0';

  // Update last will message structure
  will_message->will_topic        = last_will_topic_buffer;
  will_message->will_topic_length = cmd_input->topic.len;

  // Respond with success
  app_wifi_rsp_mqtt_client_set_last_will_topic(SL_STATUS_OK);
  return;
}

void app_wifi_cmd_mqtt_client_set_last_will_message(app_wifi_cmd_mqtt_client_set_last_will_message_t *cmd_input)
{
  // Get reference to global last will message
  sl_mqtt_client_last_will_message_t *will_message = &last_will_message;

  // Copy message content into internal buffer
  memcpy(will_message_buffer, cmd_input->message.data, cmd_input->message.len);

  // Assign buffer pointer and length to the last will message
  will_message->will_message        = will_message_buffer;
  will_message->will_message_length = cmd_input->message.len;

  // Respond with success
  app_wifi_rsp_mqtt_client_set_last_will_message(SL_STATUS_OK);
  return;
}

void app_wifi_cmd_mqtt_client_set_client_config(app_wifi_cmd_mqtt_client_set_client_config_t *cmd_input)
{
  // Get reference to global client configuration structure
  sl_mqtt_client_configuration_t *client_config = &mqtt_client_configuration;

  // Assign configuration values from input command
  client_config->auto_reconnect        = (cmd_input->is_auto_connect != 0);     // Enable/disable auto reconnect
  client_config->retry_count           = cmd_input->retry_count;                // Number of connection retries
  client_config->minimum_back_off_time = cmd_input->min_back_off_time;          // Minimum backoff time (ms)
  client_config->maximum_back_off_time = cmd_input->max_back_off_time;          // Maximum backoff time (ms)
  client_config->is_clean_session      = (cmd_input->is_clean_session != 0);    // Clean session flag
  client_config->mqt_version           = (sl_mqtt_version_t)cmd_input->version; // MQTT protocol version
  client_config->client_port           = cmd_input->client_port;                // Client-side port
  client_config->credential_id         = (sl_net_credential_id_t)cmd_input->credential_id; // Credential ID for TLS
  client_config->tls_flags             = (sl_mqtt_tls_flag_t)cmd_input->tls_flag;          // TLS-related flags

  app_wifi_rsp_mqtt_client_set_client_config(SL_STATUS_OK);
  return;
}

void app_wifi_cmd_mqtt_client_set_client_id(app_wifi_cmd_mqtt_client_set_client_id_t *cmd_input)
{
  // Get reference to global configuration
  sl_mqtt_client_configuration_t *client_id_config = &mqtt_client_configuration;

  // Set client ID pointer and length
  client_id_config->client_id_length = cmd_input->client_id.len;
  client_id_config->client_id        = cmd_input->client_id.data;

  // Respond with success
  app_wifi_rsp_mqtt_client_set_client_id(SL_STATUS_OK);
  return;
}

void app_wifi_cmd_mqtt_client_get_broker(const void *nil)
{
  (void)nil; // Unused parameter

  // Get reference to broker configuration
  sl_mqtt_broker_t *broker_config = &mqtt_broker_configuration;

  // Send broker details
  app_wifi_rsp_mqtt_client_get_broker(SL_STATUS_OK,
                                      (const void *)&broker_config->ip,       // IP address (pointer)
                                      broker_config->port,                    // Port number
                                      broker_config->is_connection_encrypted, // Encryption flag
                                      broker_config->connect_timeout,         // Connection timeout
                                      broker_config->keep_alive_interval,     // Keep-alive interval
                                      broker_config->keep_alive_retries       // Keep-alive retries
  );
  return;
}

void app_wifi_cmd_mqtt_client_get_client_config(const void *nil)
{
  (void)nil; // Unused parameter

  // Get reference to client configuration
  sl_mqtt_client_configuration_t *client_config = &mqtt_client_configuration;

  // Send configuration details
  app_wifi_rsp_mqtt_client_get_client_config(SL_STATUS_OK,
                                             client_config->auto_reconnect,          // is_auto_connect
                                             client_config->retry_count,             // retry_count
                                             client_config->minimum_back_off_time,   // min_back_off_time
                                             client_config->maximum_back_off_time,   // max_back_off_time (typo fixed)
                                             client_config->is_clean_session,        // is_clean_session
                                             (uint32_t)client_config->mqt_version,   // MQTT version
                                             client_config->client_port,             // Client port
                                             (uint32_t)client_config->credential_id, // Credential ID
                                             (uint32_t)client_config->tls_flags      // TLS flags
  );
  return;
}

void app_wifi_cmd_mqtt_client_get_client_id(const void *nil)
{
  (void)nil; // Unused parameter

  // Get reference to the global MQTT client configuration
  sl_mqtt_client_configuration_t *client_config = &mqtt_client_configuration;

  app_wifi_rsp_mqtt_client_get_client_id(SL_STATUS_OK,
                                         client_config->client_id_length, // Client ID length
                                         client_config->client_id         // Client ID (byte pointer)
  );
  return;
}
void app_wifi_cmd_mqtt_client_get_last_will_topic(const void *nil)
{
  (void)nil; // Unused parameter

  // Get reference to the last will message structure
  sl_mqtt_client_last_will_message_t *will_message = &last_will_message;

  app_wifi_rsp_mqtt_client_get_last_will_topic(SL_STATUS_OK,
                                               will_message->is_retained,              // Retain flag for last will
                                               (uint32_t)will_message->will_qos_level, // QoS level for last will
                                               will_message->will_topic_length,        // Length of the will topic
                                               will_message->will_topic // Pointer to the will topic string
  );
  return;
}

void app_wifi_cmd_mqtt_client_get_last_will_message(const void *nil)
{
  (void)nil; // Unused parameter

  // Get reference to the last will message structure
  sl_mqtt_client_last_will_message_t *will_message = &last_will_message;

  app_wifi_rsp_mqtt_client_get_last_will_message(SL_STATUS_OK,
                                                 will_message->will_message_length, // Length of the will message
                                                 will_message->will_message);       // Pointer to the will message data
  return;
}

void app_wifi_cmd_mqtt_client_deinit(app_wifi_rsp_mqtt_client_init_t *cmd_input)
{
  (void)cmd_input; // Unused parameter

  // Deinitializes the MQTT client.
  sl_status_t status = sl_mqtt_client_deinit(&client);

  // Send response with deinitialization status
  app_wifi_rsp_mqtt_client_deinit(status);
  return;
}

void app_wifi_cmd_mqtt_client_connect(const void *nil)
{
  (void)nil; // Unused parameter

  // Initiates a connection to the configured MQTT broker.
  sl_status_t status =
    sl_mqtt_client_connect(&client, &mqtt_broker_configuration, &last_will_message, &mqtt_client_configuration, 0);

  if (status == SL_STATUS_IN_PROGRESS) {
    status = SL_STATUS_OK;
  }

  // Send the connection result
  app_wifi_rsp_mqtt_client_connect(status);
  return;
}

void app_wifi_cmd_mqtt_client_disconnect(const void *nil)
{
  (void)nil; // Unused parameter

  // Request MQTT disconnection
  sl_status_t status = sl_mqtt_client_disconnect(&client, 0);
  if (status == SL_STATUS_IN_PROGRESS) {
    status = SL_STATUS_OK;
  }

  // Send disconnection result
  app_wifi_rsp_mqtt_client_disconnect(status);
  return;
}

void app_wifi_cmd_mqtt_client_subscribe(app_wifi_cmd_mqtt_client_subscribe_t *cmd_input)
{
  // Check for null input
  if (cmd_input == NULL) {
    return;
  }

  // Copy topic into internal buffer
  memcpy(topic_to_subscribe, cmd_input->topic.data, cmd_input->topic.len);

  sl_status_t status = sl_mqtt_client_subscribe(&client,
                                                (const uint8_t *)topic_to_subscribe,
                                                cmd_input->topic.len,
                                                cmd_input->qos_level,
                                                0,                           // Timeout
                                                mqtt_client_message_handler, // Message handler
                                                topic_to_subscribe           // Context
  );
  if (status == SL_STATUS_IN_PROGRESS) {
    status = SL_STATUS_OK;
  }

  // Send subscribe response
  app_wifi_rsp_mqtt_client_subscribe(status);
  return;
}

void app_wifi_cmd_mqtt_client_unsubscribe(app_wifi_cmd_mqtt_client_unsubscribe_t *cmd_input)
{
  // Check for null input
  if (cmd_input == NULL) {
    return;
  }

  // Copy topic into internal buffer
  memcpy(topic_to_unsubscribe, cmd_input->topic.data, cmd_input->topic.len);

  // Unsubscribe from the specified topic
  sl_status_t status = sl_mqtt_client_unsubscribe(&client,
                                                  (const uint8_t *)topic_to_unsubscribe,
                                                  cmd_input->topic.len,
                                                  0,                   // Timeout
                                                  topic_to_unsubscribe // Context
  );
  if (status == SL_STATUS_IN_PROGRESS) {
    status = SL_STATUS_OK;
  }

  // Send unsubscribe response
  app_wifi_rsp_mqtt_client_unsubscribe(status);
  return;
}

void app_wifi_cmd_mqtt_client_publish_topic(app_wifi_cmd_mqtt_client_publish_topic_t *cmd_input)
{
  // Check for null input
  if (cmd_input == NULL) {
    return;
  }

  // Copy topic into internal buffer
  memcpy(topic_to_publish, cmd_input->topic.data, cmd_input->topic.len);

  // Populate MQTT message structure
  mqtt_message.qos_level            = (sl_mqtt_qos_t)cmd_input->qos_level;
  mqtt_message.packet_identifier    = cmd_input->packet_id;
  mqtt_message.is_retained          = (bool)cmd_input->is_retained;
  mqtt_message.is_duplicate_message = (bool)cmd_input->is_duplicate_message;
  mqtt_message.topic                = topic_to_publish;
  mqtt_message.topic_length         = cmd_input->topic.len;

  app_wifi_rsp_mqtt_client_publish_topic(SL_STATUS_OK);
  return;
}

void app_wifi_cmd_mqtt_client_publish_message(app_wifi_cmd_mqtt_client_publish_message_t *cmd_input)
{
  if (cmd_input == NULL) {
    return; // Prevent null dereference
  }

  // Copy message content into internal buffer
  memcpy(message_to_publish, cmd_input->content.data, cmd_input->content.len);

  // Assign payload data
  mqtt_message.content        = message_to_publish;
  mqtt_message.content_length = cmd_input->content.len;

  app_wifi_rsp_mqtt_client_publish_message(SL_STATUS_OK);
  return;
}

void app_wifi_cmd_mqtt_client_publish(const void *nil)
{
  (void)nil; // Unused parameter

  // Publish the previously prepared message
  sl_status_t status = sl_mqtt_client_publish(&client, &mqtt_message, 0, &mqtt_message);
  if (status == SL_STATUS_IN_PROGRESS) {
    status = SL_STATUS_OK;
  }

  // Send publish result
  app_wifi_rsp_mqtt_client_publish(status);
  return;
}
