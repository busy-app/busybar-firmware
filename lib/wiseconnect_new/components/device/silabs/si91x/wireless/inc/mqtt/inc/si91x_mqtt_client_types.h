/***************************************************************************/ /**
 * @file  si91x_mqtt_client_types.h
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

#pragma once
#include "stdint.h"
#include "sl_common.h"

//Note: Please go through http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html

#define SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH       202 // This size is including the NULL terminating character.
#define SI91X_MQTT_CLIENT_WILL_TOPIC_MAXIMUM_LENGTH  202 // This size is including the NULL terminating character.
#define SLI_SI91X_MQTT_CLIENT_MESSAGE_MAXIMUM_LENGTH 100

#define SLI_SI91X_MQTT_CLIENT_ID_MAXIMUM_LENGTH   62  // This size is including the NULL terminating character.
#define SI91X_MQTT_CLIENT_USERNAME_MAXIMUM_LENGTH 122 // This size is including the NULL terminating character.
#define SI91X_MQTT_CLIENT_PASSWORD_MAXIMUM_LENGTH 512 // This size is including the NULL terminating character.

#define SLI_SI91X_MQTT_CLIENT_INIT_COMMAND        1
#define SLI_SI91X_MQTT_CLIENT_CONNECT_COMMAND     2
#define SLI_SI91X_MQTT_CLIENT_SUBSCRIBE_COMMAND   3
#define SLI_SI91X_MQTT_CLIENT_PUBLISH_COMMAND     4
#define SLI_SI91X_MQTT_CLIENT_UNSUBSCRIBE_COMMAND 5
#define SLI_SI91X_MQTT_CLIENT_DISCONNECT_COMMAND  8
#define SLI_SI91X_MQTT_CLIENT_DEINIT_COMMAND      9

#define SLI_SI91X_MQTT_CLIENT_TOPIC_DELIMITER        "/"
#define SLI_SI91X_MQTT_CLIENT_SINGLE_LEVEL_WILD_CARD "+"
#define SLI_SI91X_MQTT_CLIENT_MULTI_LEVEL_WILD_CARD  "#"

typedef struct {
  // IP version
  uint32_t ip_version;
  union {
    uint8_t ipv4_address[4];
    uint8_t ipv6_address[16];
  } server_ip_address;
} sli_si91x_mqtt_client_ip_address_t;

typedef struct SL_ATTRIBUTE_PACKED {
  uint32_t command_type;
  // MQTT server IP address
  sli_si91x_mqtt_client_ip_address_t server_ip;
  // MQTT server port
  uint32_t server_port;
  // Client ID Length
  uint8_t client_id_len;
  // client ID, should be unique
  int8_t client_id[SLI_SI91X_MQTT_CLIENT_ID_MAXIMUM_LENGTH];
  // keep alive interval (s)
  uint16_t keep_alive_interval;
  // username Length
  uint8_t username_len;
  // user name
  uint8_t user_name[SI91X_MQTT_CLIENT_USERNAME_MAXIMUM_LENGTH];
  // password Length
  uint16_t password_len;
  // password
  uint8_t password[SI91X_MQTT_CLIENT_PASSWORD_MAXIMUM_LENGTH];
  // clean session(0-1)
  uint8_t clean;
  // 0 : TCP , 1 : SSl
  uint8_t encrypt;
  // MQTT  Client port
  uint32_t client_port;
#ifdef SLI_SI917
  //! Capping tcp retransmission timeout
  uint8_t tcp_max_retransmission_cap_for_emb_mqtt;
#endif
  //! MQTT ping retries.
  uint16_t keep_alive_retries;
} si91x_mqtt_client_init_request_t;

typedef struct SL_ATTRIBUTE_PACKED {
  uint32_t command_type;
  // whether to use username (0-1)
  uint8_t is_username_present;
  //  whether to use pwdFlag (0-1)
  uint8_t is_password_present;
  //  whether to set willmsg (0-1)
  uint8_t will_flag;
  //  retained flag(0-1)
  uint8_t will_retain;
  //  message Qos(0-2)
  uint8_t will_qos;
  // length of topic
  uint8_t will_topic_len;
  //  topic name of will
  uint8_t will_topic[SI91X_MQTT_CLIENT_WILL_TOPIC_MAXIMUM_LENGTH];
  // Length of Will message
  uint8_t will_message_len;
  //  message of will
  uint8_t will_msg[SLI_SI91X_MQTT_CLIENT_MESSAGE_MAXIMUM_LENGTH];

} sli_si91x_mqtt_client_connect_request_t;

typedef struct SL_ATTRIBUTE_PACKED {
  uint32_t command_type;
  // length of TOPIC
  uint8_t topic_len;
  // topic of subscribe message
  int8_t topic[SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH];
  // message Qos, can be 0, 1, or 2
  int8_t qos;

} sli_si91x_mqtt_client_subscribe_t;

typedef struct SL_ATTRIBUTE_PACKED {
  uint32_t command_type;
  // length of TOPIC
  uint8_t topic_len;
  // topic of unsubscribe message
  uint8_t topic[SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH];
  // message Qos, can be 0, 1, or 2
  uint8_t qos;
  // retained flag, can be 0 or 1
  uint8_t retained;
  // duplicate flag, can be 0 or 1
  uint8_t dup;
  // length of publish message(option), if set to 0 or
  //omitted, <message> will be parsed in text format, else
  //hexidecimai format
  uint16_t msg_len;
  // publish message
  int8_t *msg;

} sli_si91x_mqtt_client_publish_request_t;

typedef struct SL_ATTRIBUTE_PACKED {
  uint32_t command_type;
  // length of TOPIC
  uint8_t topic_len;
  // topic of unsubscribe message
  uint8_t topic[SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH];

} sli_si91x_mqtt_client_unsubscribe_request_t;

typedef struct {
  uint32_t command_type;

} sli_si91x_mqtt_client_command_request_t;

typedef struct SL_ATTRIBUTE_PACKED {
  uint16_t mqtt_flags;
  uint16_t current_chunk_length;
  uint16_t topic_length;
  uint8_t data[];
} sli_si91x_mqtt_client_received_message_t;
