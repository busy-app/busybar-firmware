/*******************************************************************************
 * @file  sl_mqtt_client_types.h
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
#include <stdint.h>
#include <stdbool.h>
#include "sl_slist.h"
#include "sl_net_constants.h"
#include "sl_ip_types.h"
#include "sl_wifi_device.h"
#include "sl_si91x_socket_utility.h"

typedef enum MQTTStatus {
  MQTTSuccess = 0,     /**< Function completed successfully. */
  MQTTBadParameter,    /**< At least one parameter was invalid. */
  MQTTNoMemory,        /**< A provided buffer was too small. */
  MQTTSendFailed,      /**< The transport send function failed. */
  MQTTRecvFailed,      /**< The transport receive function failed. */
  MQTTBadResponse,     /**< An invalid packet was received from the server. */
  MQTTServerRefused,   /**< The server refused a CONNECT or SUBSCRIBE. */
  MQTTNoDataAvailable, /**< No data available from the transport interface. */
  MQTTIllegalState,    /**< An illegal state in the state record. */
  MQTTStateCollision,  /**< A collision with an existing state record entry. */
  MQTTKeepAliveTimeout /**< Timeout while waiting for PINGRESP. */
} MQTTStatus_t;

/**
 * @addtogroup SERVICE_MQTT_CONSTANTS 
 * @{
 */

/**
 * @enum sl_mqtt_qos_t
 * @brief MQTT Quality of Service (QoS) levels.
 * 
 * @details
 *   This enumeration defines the Quality of Service (QoS) levels for MQTT messages. QoS levels determine the guarantee of delivery for a message.
 * 
 * @note 
 *   Quality of Service (QoS) level 2 is not currently supported.
 */
typedef enum {
  SL_MQTT_QOS_LEVEL_0, ///< At most once delivery. The message is delivered according to the best efforts of the operating environment. Message loss can occur.
  SL_MQTT_QOS_LEVEL_1, ///< At least once delivery. The message is assured to arrive but duplicates can occur.
  SL_MQTT_QOS_LEVEL_2 ///< Exactly once delivery. The message is assured to arrive exactly once. (Not currently supported).
} sl_mqtt_qos_t;

/**
 * @enum sl_mqtt_client_connection_state_t
 * @brief MQTT Client connection states.
 * 
 * @details
 *   This enumeration defines the various states of the MQTT client during its lifecycle. These states indicate the current status of the client's connection to the MQTT broker.
 */
typedef enum {
  SL_MQTT_CLIENT_DISCONNECTED, ///< Initial state. The client is not connected to the MQTT broker.
  SL_MQTT_CLIENT_TA_INIT, ///< The client has successfully initialized the NWP(Network Wireless Processor) for MQTT.
  SL_MQTT_CLIENT_CONNECTION_FAILED, ///< The client attempted to connect to the MQTT broker but failed.
  SL_MQTT_CLIENT_CONNECTED,         ///< The client is successfully connected to the MQTT broker.
  SL_MQTT_CLIENT_TA_DISCONNECTED ///< The NWP(Network Wireless Processor) is disconnected from the broker, but NWP deinitialization has not yet been called.
} sl_mqtt_client_connection_state_t;

/**
 * @enum sl_mqtt_version_t
 * @brief MQTT Protocol versions.
 * 
 * @details
 *   This enumeration defines the supported versions of the MQTT protocol. The version determines the protocol features and behaviors used by the MQTT client.
 */
typedef enum {
  SL_MQTT_VERSION_3,  ///< MQTT Version 3.0
  SL_MQTT_VERSION_3_1 ///< MQTT Version 3.1
} sl_mqtt_version_t;

/**
 * @enum sl_mqtt_client_event_t
 * @brief MQTT Client Events.
 * 
 * @details
 *   This enumeration defines the various events that can occur in the lifecycle of an MQTT client. These events notifies the application about changes in the client's state, or the receipt of messages.
 */
typedef enum {
  SL_MQTT_CLIENT_CONNECTED_EVENT, ///< Event indicating that the MQTT client has successfully connected to the broker.
  SL_MQTT_CLIENT_DISCONNECTED_EVENT, ///< Event indicating that the MQTT client has disconnected from the broker.
  SL_MQTT_CLIENT_MESSAGE_PUBLISHED_EVENT, ///< Event indicating that a message has been successfully published by the MQTT client.
  SL_MQTT_CLIENT_MESSAGED_RECEIVED_EVENT, ///< Event indicating that a message has been received by the MQTT client.
  SL_MQTT_CLIENT_SUBSCRIBED_EVENT, ///< Event indicating that the MQTT client has successfully subscribed to a topic.
  SL_MQTT_CLIENT_UNSUBSCRIBED_EVENT, ///< Event indicating that the MQTT client has successfully unsubscribed from a topic.
  SL_MQTT_CLIENT_ERROR_EVENT         ///< Event indicating that an error has occurred in the MQTT client.
} sl_mqtt_client_event_t;

/**
 * @enum sl_mqtt_client_error_status_t
 * @brief MQTT Client error statuses.
 * 
 * @details
 *   This enumeration defines the various error statuses that can occur in the MQTT client. These statuses indicate the type of error that has occurred during the client's operations.
 */
typedef enum {
  SL_MQTT_CLIENT_CONNECT_FAILED,      ///< Error status indicating that the MQTT client failed to connect to the broker.
  SL_MQTT_CLIENT_PUBLISH_FAILED,      ///< Error status indicating that the MQTT client failed to publish a message.
  SL_MQTT_CLIENT_SUBSCRIBE_FAILED,    ///< Error status indicating that the MQTT client failed to subscribe to a topic.
  SL_MQTT_CLIENT_UNSUBSCRIBED_FAILED, ///< Error status indicating that the MQTT client failed to unsubscribe from a topic.
  SL_MQTT_CLIENT_DISCONNECT_FAILED, ///< Error status indicating that the MQTT client failed to disconnect from the broker.
  SL_MQTT_CLIENT_UNKNOWN_ERROR      ///< Error status indicating that an unknown error occurred in the MQTT client.
} sl_mqtt_client_error_status_t;

/**
 * @enum sl_mqtt_client_disconnection_reason_t
 * @brief MQTT Client disconnection reasons.
 * 
 * @details
 *   This enumeration defines the various reasons for the disconnection of an MQTT client from the broker. These reasons help in identifying the cause of the disconnection.
 */
typedef enum {
  SL_MQTT_CLIENT_REMOTE_TERMINATE_DISCONNECTION,           ///< Disconnection due to remote termination by the broker.
  SL_MQTT_CLIENT_WLAN_DISCONNECTION,                       ///< Disconnection due to WLAN (Wi-Fi) network issues.
  SL_MQTT_CLIENT_USER_INITIATED_DISCONNECTION,             ///< Disconnection initiated by the user.
  SL_MQTT_CLIENT_KEEP_ALIVE_RESPONSE_TIMEOUT_DISCONNECTION ///< Disconnection due to keep-alive response timeout.
} sl_mqtt_client_disconnection_reason_t;

/**
 * @enum sl_mqtt_tls_flag_t
 * @brief MQTT TLS and certificate options.
 * 
 * @details
 *   This enumeration defines the various TLS (Transport Layer Security) and certificate options available for the MQTT client. These options include enabling TLS, selecting the TLS version, and choosing the certificate index.
 * 
 * @note 
 *   Only one TLS version (SL_MQTT_TLS_TLSV_X_X) can be set at a time.
 * @note 
 *   Only one certificate index (SL_MQTT_TLS_CERT_INDEX_X) can be set at a time.
 * @note
 *   To select certificate index 0, no additional flags need to be configured explicitly.
 */
typedef enum {
  SL_MQTT_TLS_ENABLE       = BIT(0), ///< Enable TLS for MQTT.
  SL_MQTT_TLS_TLSV_1_0     = BIT(1), ///< Enable TLS version 1.0 for MQTT.
  SL_MQTT_TLS_TLSV_1_1     = BIT(2), ///< Enable TLS version 1.1 for MQTT.
  SL_MQTT_TLS_TLSV_1_2     = BIT(3), ///< Enable TLS version 1.2 for MQTT.
  SL_MQTT_TLS_TLSV_1_3     = BIT(4), ///< Enable TLS version 1.3 for MQTT.
  SL_MQTT_TLS_CERT_INDEX_1 = BIT(5), ///< Use certificate index 1 for MQTT.
  SL_MQTT_TLS_CERT_INDEX_2 = BIT(6), ///< Use certificate index 2 for MQTT.
  SL_MQTT_TLS_SNI_ENABLE   = BIT(7)  ///< Enable Server Name Indication (SNI) extension for MQTT.
} sl_mqtt_tls_flag_t;

/** @} */

/**
 * @addtogroup SERVICE_MQTT_TYPES 
 * @{
 */

/**
 * @brief 
 *   Structure representing the MQTT Client Last Will message.
 * 
 * @details
 *   This structure holds the information for the Last Will message that will be sent by the MQTT broker if the client unexpectedly disconnects. The Last Will message is a retained message that can be used to notify other clients about the disconnection.
 */
typedef struct {
  bool is_retained;             ///< Flag indicating whether to retain the Last Will message.
  sl_mqtt_qos_t will_qos_level; ///< Quality of Service level for the Last Will message.
  uint8_t *will_topic;          ///< Pointer to the topic name for the Last Will message.
  uint16_t
    will_topic_length; ///< Length of the topic name. It should not exceed 202 bytes that includes NULL termination character.
  uint8_t *will_message; ///< Pointer to the Last Will message content.
  uint32_t
    will_message_length; ///< Length of the Last Will message content. Should not exceed 60 bytes including NULL termination character.
} sl_mqtt_client_last_will_message_t;

/**
 * @brief 
 *   Structure representing an MQTT Client Message.
 * 
 * @details
 *   This structure holds the information for a message received from or sent to the MQTT broker. It includes details such as the Quality of Service level, packet identifier, and message content.
 */
typedef struct {
  sl_mqtt_qos_t qos_level;    ///< Quality of Service level of the message.
  uint16_t packet_identifier; ///< Packet identifier of the received message.
  bool is_retained;           ///< Flag indicating whether the message is retained by the broker.
  bool is_duplicate_message;  ///< Flag indicating whether this is a duplicate message.
  uint8_t *topic;             ///< Pointer to the topic name. Must not be NULL.
  uint16_t
    topic_length; ///< Length of the topic name. It should not exceed 202 bytes that includes NULL termination character.
  uint8_t *content;        ///< Pointer to the message content. Must not be NULL.
  uint32_t content_length; ///< Length of the message content. It should not exceed 1024 bytes.
} sl_mqtt_client_message_t;

/**
 * @brief 
 *   MQTT Client broker information structure.
 * @details
 *   This structure holds the information required to connect to an MQTT broker, that includes the broker's IP address, port number, connection encryption status, connection timeout, keep-alive interval, and keep-alive retries.
 * @note 
 *   This structure will be deprecated in a future release. Instead, use the [sl-mqtt-broker-v2-t](../wiseconnect-api-reference-guide-mqtt/sl-mqtt-broker-v2-t) structure. This structure is retained for backward compatibility.
 */
typedef struct {
  sl_ip_address_t ip; ///< IP address of the broker.
  uint16_t port;      ///< Port number of the broker.
  bool
    is_connection_encrypted; ///< Indicates if the connection is encrypted. This field would be deprecated in future releases. You are recommended to use `tls_flags` in @ref sl_mqtt_client_configuration_t.
  uint16_t connect_timeout;  ///< MQTT connection timeout in milliseconds.
  uint16_t keep_alive_interval; ///< Keep-alive interval of the MQTT connection in seconds.
  uint16_t keep_alive_retries;  ///< Number of MQTT ping retries.
} sl_mqtt_broker_t;

/**
 * @brief
 *   MQTT Client broker information structure v2.
 *
 * @details
 *   This structure holds the information required to connect to an MQTT broker, including:
 *   - Broker's IP address and port
 *   - MQTT broker hostname
 *   - Connection encryption status
 *   - Connection timeout, keep-alive interval, and keep-alive retries
 *   - SNI support: enable/disable and SNI hostname
 *
 *   @note
 *   - The `host_name` field is reserved for future support of MQTT broker hostname.
 *   - The `sni_host_name` field is used for Server Name Indication (SNI) when connecting over TLS. This is required for AWS IoT and similar brokers.
 *   - If SNI is enabled (`enable_sni` is true), `sni_host_name` must be provided and will be programmed into the firmware before connecting.
 *   - For non-TLS connections, SNI fields are ignored.
 */
typedef struct {
  sl_ip_address_t ip; ///< IP address of the broker.
  uint8_t *host_name; ///< MQTT broker hostname (not currently supported).
  uint16_t port;      ///< Port number of the broker.
  bool
    is_connection_encrypted; ///< Indicates if the connection is encrypted. This field will be deprecated in future releases. Use `tls_flags` in @ref sl_mqtt_client_configuration_t instead.
  uint16_t connect_timeout;  ///< MQTT connection timeout in milliseconds.
  uint16_t keep_alive_interval; ///< Keep-alive interval of the MQTT connection in seconds.
  uint16_t keep_alive_retries;  ///< Number of MQTT ping retries.
  bool enable_sni;              ///< Enable or disable SNI extension in MQTT.
  uint8_t *sni_host_name;       ///< Hostname to use in SNI (required for TLS/SNI connections; e.g., AWS IoT).
} sl_mqtt_broker_v2_t;

/**
 * @brief 
 *   MQTT Client credentials structure.
 * 
 * @details
 *   This structure holds the credentials required for the MQTT client to authenticate with the broker. It includes the lengths of the username and password, and a flexible array to store both the username and password.
 */
typedef struct {
  uint16_t
    username_length; ///< Length of the username. It should not exceed 120 bytes which includes NULL termination character.
  uint16_t
    password_length; ///< Length of the password. It should not exceed 512 bytes which includes NULL termination character.
  uint8_t data[];    ///< Flexible array to store both the username and password.
} sl_mqtt_client_credentials_t;

/**
 * @brief 
 *   MQTT Client Configuration structure.
 * 
 * @details
 *   This structure holds the configuration parameters for the MQTT client, that includes connection settings, retry policies, session options, and security credentials.
 */
typedef struct {
  bool auto_reconnect;            ///< Whether to automatically reconnect to the broker in case of disconnection.
  uint8_t retry_count;            ///< Maximum number of retry attempts for auto reconnect.
  uint16_t minimum_back_off_time; ///< Minimum back-off time (in seconds) between two successive reconnect attempts.
  uint16_t maximum_back_off_time; ///< Maximum back-off time (in seconds) between two successive reconnect attempts.
  bool is_clean_session;          ///< Clean session flag to send to the broker in the connect request.
  sl_mqtt_version_t mqt_version;  ///< MQTT protocol version used by the client.
  uint16_t client_port;           ///< Port number used by the client for the connection.
  sl_net_credential_id_t
    credential_id;    ///< Credential ID for the username and password used in the MQTT connect request.
  uint8_t *client_id; ///< Pointer to the MQTT client ID string.
  uint8_t
    client_id_length; ///< Length of the client ID string. Should not exceed 60 bytes including NULL termination character.
  sl_mqtt_tls_flag_t tls_flags; ///< TLS flags for various MQTT options. See @ref sl_mqtt_tls_flag_t for details.
} sl_mqtt_client_configuration_t;

/**
 * @typedef sl_mqtt_client_event_handler_t
 * @brief
 *   Handler for MQTT client events.
 * 
 * @details
 *   This function pointer type defines the handler for various events that occur on the MQTT client. The handler is invoked with details about the event, including the client on which the event occurred, the type of event, any associated event data, and a user-provided context.
 * 
 * @param client
 *   Pointer to the MQTT client on which the event occurred. This pointer must not be NULL.
 * 
 * @param event
 *   The type of event that occurred. This is of type @ref sl_mqtt_client_event_t.
 * 
 * @param event_data
 *   Pointer to the event data. This parameter is non-null only for events of type MQTT_CLIENT_MESSAGE_RECEIVED and MQTT_CLIENT_ERROR.
 * 
 * @param context
 *   Pointer to the user-provided context. This context is provided at the time of the API call (e.g., @ref sl_mqtt_client_init). The caller must ensure that the lifecycle of the context is retained until the callback is invoked. The deallocation of the context is also the responsibility of the caller.
 */
typedef void (*sl_mqtt_client_event_handler_t)(void *client,
                                               sl_mqtt_client_event_t event,
                                               void *event_data,
                                               void *context);

/**
 * @typedef sl_mqtt_client_message_received_t
 * @brief
 *   Handler for received MQTT client messages.
 * 
 * @details
 *   This function pointer type defines the handler for messages received by the MQTT client. The handler is invoked with details about the client that received the message, the message itself, and a user-provided context.
 * 
 * @param client
 *   Pointer to the MQTT client that received the message. This pointer must not be NULL.
 * 
 * @param message_to_be_published
 *   Pointer to the received message of type @ref sl_mqtt_client_message_t. This pointer must not be NULL.
 * 
 * @param context
 *   Pointer to the user-provided context. This context is provided at the time of the Subscribe API call. The caller (the function or module that invokes the Subscribe API) must ensure that the lifecycle of the context is retained until the callback is invoked. The deallocation of the context is also the responsibility of the caller.
 * 
 * @note
 *   Due to constraints from the firmware, the fields `is_retained`, `qos_level`, `packet_identifier`, and `duplicate_message` of @ref sl_mqtt_client_message_t are not populated and should be ignored while processing the message.
 */
typedef void (*sl_mqtt_client_message_received_t)(void *client,
                                                  sl_mqtt_client_message_t *message_to_be_published,
                                                  void *context);

/**
 * @brief 
 *   MQTT Client Topic Subscription Info structure.
 * 
 * @details
 *   This structure holds the information related to a topic subscription for the MQTT client. It includes the message handler for the topic, the Quality of Service (QoS) level, and the topic itself.
 */
typedef struct {
  sl_slist_node_t next_subscription; ///< Next node in the linked list of subscriptions.
  sl_mqtt_client_message_received_t
    topic_message_handler;           ///< Function pointer to the message handler for the subscribed topic.
  sl_mqtt_qos_t qos_of_subscription; ///< Quality of Service level for the subscription.
  uint16_t
    topic_length; ///< Length of the subscribed topic. It should not exceed 202 bytes that includes NULL termination character.
  uint8_t topic[]; ///< Flexible array to store the topic name.
} sl_mqtt_client_topic_subscription_info_t;

/**
 * @brief 
 *   MQTT Client Handle structure.
 * 
 * @details
 *   This structure represents the handle for an MQTT client. It holds the current state of the client, broker configuration, last will message configuration, client configuration, subscription list, and event handler.
 */
typedef struct {
  sl_mqtt_client_connection_state_t state; ///< Current state of the MQTT client.
  const sl_mqtt_broker_t
    *broker; ///< Pointer to the broker configuration, provided at the time of the connect() API call.
  const sl_mqtt_client_last_will_message_t *
    last_will_message; ///< Pointer to the last will message configuration, provided at the time of the connect() API call.
  const sl_mqtt_client_configuration_t
    *client_configuration; ///< Pointer to the client configuration, provided at the time of the connect() API call.
  sl_mqtt_client_topic_subscription_info_t
    *subscription_list_head; ///< Pointer to the head of the subscription linked list.
  sl_mqtt_client_event_handler_t
    client_event_handler; ///< Function pointer to the event handler, provided at the time of @ref sl_mqtt_client_init.
} sl_mqtt_client_t;

/** @} */
