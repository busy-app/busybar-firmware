/***************************************************************************/ /**
 * @file  sl_mqtt_client.c
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
#include "sl_net.h"
#include "sl_slist.h"
#include "sl_si91x_driver.h"
#include "stdint.h"
#include <stdbool.h>
#include <string.h>
#include <sl_string.h>
#include "sl_mqtt_client.h"
#include "sl_mqtt_client_types.h"
#include "si91x_mqtt_client_types.h"
#include "si91x_mqtt_client_utility.h"
#include "sl_status.h"
#include "sli_wifi_constants.h"
#include "sli_wifi_utility.h"
#include "sl_rsi_utility.h"

/**
 * MQTT CLIENT STATE MACHINE
========================================================================|
	STATE			                        EVENT					          NEXT_STATE  |
========================================================================|
	CONNECTED	                        |	CONNECT				        |	UNKNOWN
	CONNECTED	                        |	RECONNECT			        |	UNKNOWN
	CONNECTED	                        |	DISCONNECT			      |	TA_DISCONNECTED -> DISCONNECTED
	CONNECTED	                        | PUBLISH			          | CONNECTED
	CONNECTED	                        |	SUBSCRIBE			        |	CONNECTED
	CONNECTED	                        |	UNSUBSCRIBE			      |	CONNECTED
	CONNECTED	                        |	ON_DISCONNECT_RECV^   |	DISCONNECTED
              
	DISCONNECTED                      |	CONNECT				        |	TRANSPORTATION_LAYER_CONNECTED -> CONNECTED
	DISCONNECTED                      |	RECONNECT			        |	was_connect_called_previously ? TRANSPORTATION_LAYER_CONNECTED -> CONNECTED: UNKNOWN
	DISCONNECTED                      |	ANY_OTHER_EVENT	      |	UNKNOWN
                                      
	TRANSPORTATION_LAYER_CONNECTED 	  |	ON_CONNECT_SUCCESS^   |	CONNECTED
	TRANSPORTATION_LAYER_CONNECTED	  |	ON_CONNECT_FAILED^	  |	TRANSPORTATION_LAYER_CONNECTED
	TRANSPORTATION_LAYER_CONNECTED	  |	ANY_OTHER_EVENT		    |	UNKNOWN

	TA_DISCONNECTED                   | ON_DISCONNECT_SUCCESS^| DISCONNECTED
	TA_DISCONNECTED                   | ON_DISCONNECT_FAILED^ | TA_DISCONNECTED
	TA_DISCONNECTED                   | ANY_OTHER_EVENT		    | UNKNOWN
============================================================

	*UNKOWN -> Throw Error
	^ -> firmware events
**/

// Declaring strtok_r as extern to suppress implicit declaration warning.
extern char *strtok_r(char *, const char *, char **);

extern sli_wifi_command_queue_t cmd_queues[SI91X_CMD_MAX];

#define SI91X_MQTT_CLIENT_INIT_TIMEOUT        5000
#define SI91X_MQTT_CLIENT_DISCONNECT_TIMEOUT  5000
#define SI91X_MQTT_CHECK_RETAIN_MESSAGE       BIT(0)
#define SI91X_MQTT_CHECK_QOS_LEVEL            (BIT(1) | BIT(2))
#define SI91X_MQTT_CHECK_IS_DUPLICATE_MESSAGE BIT(3)

#define VERIFY_AND_RETURN_ERROR_IF_FALSE(condition, status) \
  do {                                                      \
    if (!(condition)) {                                     \
      if (PRINT_ERROR_LOGS) {                               \
        PRINT_STATUS(INFO_TAG, status)                      \
      }                                                     \
      return status;                                        \
    }                                                       \
  } while (0)

static sl_mqtt_client_t *mqtt_client;
static sl_mqtt_client_error_status_t sli_si91x_get_event_error_status(sl_mqtt_client_event_t event);
static void sli_si91x_handle_connected_event(sl_status_t status,
                                             sl_si91x_mqtt_client_context_t *sdk_context,
                                             const sl_wifi_system_packet_t *rx_packet,
                                             bool *is_error_event,
                                             uint8_t **event_data,
                                             sl_mqtt_client_disconnection_reason_t *reason);
static void sli_si91x_handle_subscribed_event(sl_status_t status,
                                              sl_si91x_mqtt_client_context_t *sdk_context,
                                              bool *is_error_event);
static void sli_si91x_handle_unsubscribed_event(sl_status_t status,
                                                sl_si91x_mqtt_client_context_t *sdk_context,
                                                bool *is_error_event);
static void sli_si91x_handle_message_received_event(sl_si91x_mqtt_client_context_t *sdk_context,
                                                    sl_wifi_system_packet_t *rx_packet);
static void sli_si91x_handle_disconnected_event(sl_status_t status,
                                                sl_si91x_mqtt_client_context_t *sdk_context,
                                                const sl_wifi_system_packet_t *rx_packet,
                                                bool *is_error_event,
                                                uint8_t **event_data,
                                                sl_mqtt_client_disconnection_reason_t *reason);
/**
 * A internal helper function to get node of list which matches the given topic.
 * @param client 		Pointer to client object whose subscription list needs to be searched.
 * @param topic			Topic which needs to be searched in list.
 * @param topic_length	Length of the topic that needs to be searched.
 * @param required_node	A double pointer to node pointer.
 */
static void sli_si91x_get_subscription(const sl_mqtt_client_t *client,
                                       const uint8_t *topic,
                                       const uint16_t topic_length,
                                       sl_mqtt_client_topic_subscription_info_t **required_subscription)
{
  sl_mqtt_client_topic_subscription_info_t *subscription = client->subscription_list_head;

  uint8_t subscribed_topic[SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH] = { 0 };
  uint8_t received_topic[SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH]   = { 0 };
  *required_subscription                                           = NULL;

  // If the topic length is greater than or equal to SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH, return.
  if (topic_length >= SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH) {
    return;
  }

  while (subscription != NULL) {
    memcpy(subscribed_topic, subscription->topic, subscription->topic_length);
    memcpy(received_topic, topic, topic_length);

    // adding null character at the end of the string as topic might not end with null terminating character.
    // the size of topic is guaranteed to be less than SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH.
    subscribed_topic[subscription->topic_length] = '\0';

    char *subscribed_topic_save_ptr = NULL;
    char *received_topic_save_ptr   = NULL;

    char *subscribed_topic_token = NULL;
    char *received_topic_token   = NULL;

    subscribed_topic_token =
      strtok_r((char *)subscribed_topic, SLI_SI91X_MQTT_CLIENT_TOPIC_DELIMITER, &subscribed_topic_save_ptr);
    received_topic_token =
      strtok_r((char *)received_topic, SLI_SI91X_MQTT_CLIENT_TOPIC_DELIMITER, &received_topic_save_ptr);

    while (subscribed_topic_token != NULL && received_topic_token != NULL) {
      uint8_t subscribe_topic_length =
        (uint8_t)(sl_strnlen(subscribed_topic_token, SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH));

      // This boolean stores whether the subscribed_topic_token is wildcard or not by checking the length of the token and character stored in it.
      uint8_t is_wild_card =
        (subscribe_topic_length == 1)
        && ((strncmp(subscribed_topic_token, SLI_SI91X_MQTT_CLIENT_MULTI_LEVEL_WILD_CARD, 1) == 0)
            || (strncmp(subscribed_topic_token, SLI_SI91X_MQTT_CLIENT_SINGLE_LEVEL_WILD_CARD, 1) == 0));

      uint8_t is_multi_level_wild_card =
        is_wild_card && (strncmp(subscribed_topic_token, SLI_SI91X_MQTT_CLIENT_MULTI_LEVEL_WILD_CARD, 1) == 0);

      // if subscribed_topic_token isn't wildcard and tokens does not match, break the loop and continue searching with other subscriptions.
      if (!is_wild_card
          && ((subscribe_topic_length != sl_strnlen(received_topic_token, SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH))
              || memcmp(subscribed_topic_token, received_topic_token, subscribe_topic_length) != 0)) {
        break;
      } else if (is_multi_level_wild_card) {
        // if the subscribed_topic_token is "#", the assign the current subscription as required_subscription and return.
        *required_subscription = subscription;
        return;
      } else {
        // The execution enters the else block either in case of single level wildcard or successful comparison of current token
        subscribed_topic_token = strtok_r(NULL, SLI_SI91X_MQTT_CLIENT_TOPIC_DELIMITER, &subscribed_topic_save_ptr);
        received_topic_token   = strtok_r(NULL, SLI_SI91X_MQTT_CLIENT_TOPIC_DELIMITER, &received_topic_save_ptr);
      }

      // If both strings reach end of token, assign this subscription as required_subscription
      // Example: subscription Topic: Home/+/TemperatureSensor
      //          Received Topic:     Home/Garden/TemperatureSensor
      if (subscribed_topic_token == NULL && received_topic_token == NULL) {
        *required_subscription = subscription;
        return;
      }
    }

    subscription = (sl_mqtt_client_topic_subscription_info_t *)subscription->next_subscription.node;
  }
}

static void sli_si91x_remove_and_free_all_subscriptions(sl_mqtt_client_t *client)
{
  if (client == NULL) {
    SL_DEBUG_LOG("MQTT client instance not initialized yet\n");
    return;
  }
  // Free subscription list.
  sl_mqtt_client_topic_subscription_info_t *node_to_be_freed;
  while ((node_to_be_freed = (sl_mqtt_client_topic_subscription_info_t *)(sl_slist_pop(
            (sl_slist_node_t **)&client->subscription_list_head)))
         != NULL) {
    free(node_to_be_freed);
  }
}
static inline bool is_connect_previously_called(const sl_mqtt_client_t *client)
{
  return (NULL != client->client_configuration);
}

sl_status_t sli_si91x_build_mqtt_sdk_context_if_async(sl_mqtt_client_event_t event,
                                                      sl_mqtt_client_t *client,
                                                      void *user_context,
                                                      void *sdk_data,
                                                      uint32_t timeout,
                                                      sl_si91x_mqtt_client_context_t **context)
{
  *context = NULL;
  if (timeout > 0) {
    return SL_STATUS_OK;
  }

  sl_si91x_mqtt_client_context_t *mqtt_client_sdk_context = calloc(sizeof(sl_si91x_mqtt_client_context_t), 1);

  if (mqtt_client_sdk_context == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  mqtt_client_sdk_context->client       = client;
  mqtt_client_sdk_context->event        = event;
  mqtt_client_sdk_context->sdk_data     = sdk_data;
  mqtt_client_sdk_context->user_context = user_context;

  *context = mqtt_client_sdk_context;

  return SL_STATUS_OK;
}

void sli_si91x_get_mqtt_client(sl_mqtt_client_t **client)
{
  *client = mqtt_client;
}

/**
 * A internal function to map the event to its appropriate mqtt error state.
 * @param event	Event whose sl_mqtt_client_event_t value is required.
 * @return sl_mqtt_client_error_status_t for the Event.
 */
static sl_mqtt_client_error_status_t sli_si91x_get_event_error_status(sl_mqtt_client_event_t event)
{
  switch (event) {
    case SL_MQTT_CLIENT_CONNECTED_EVENT: {
      return SL_MQTT_CLIENT_CONNECT_FAILED;
    }

    case SL_MQTT_CLIENT_DISCONNECTED_EVENT: {
      return SL_MQTT_CLIENT_DISCONNECT_FAILED;
    }

    case SL_MQTT_CLIENT_MESSAGE_PUBLISHED_EVENT: {
      return SL_MQTT_CLIENT_PUBLISH_FAILED;
    }

    case SL_MQTT_CLIENT_SUBSCRIBED_EVENT: {
      return SL_MQTT_CLIENT_SUBSCRIBE_FAILED;
    }

    case SL_MQTT_CLIENT_UNSUBSCRIBED_EVENT: {
      return SL_MQTT_CLIENT_UNSUBSCRIBED_FAILED;
    }

    default: {
      return SL_MQTT_CLIENT_UNKNOWN_ERROR;
    }
  }
}

/**
 * @brief Get MQTT client credentials from the credential manager.
 *
 * This function retrieves the MQTT client credentials associated with the given credential ID.
 *
 * @param[in] credential_id The ID of the MQTT client credentials.
 * @param[out] credentials Double pointer to the MQTT client credentials.
 *
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 *
 * @note The function allocates memory for the credentials in case of successfully fetching the data,
 *       and the caller is responsible for freeing the memory.
 */
static sl_status_t sli_si91x_fetch_mqtt_client_credentials(sl_net_credential_id_t credential_id,
                                                           sl_mqtt_client_credentials_t **credentials)
{
  SL_VERIFY_POINTER_OR_RETURN(credentials, SL_STATUS_WIFI_NULL_PTR_ARG);

  *credentials = NULL;

  if (credential_id == (sl_net_credential_id_t)SL_NET_INVALID_CREDENTIAL_TYPE) {
    return SL_STATUS_OK;
  }

  uint32_t maximum_credential_size = sizeof(sl_mqtt_client_credentials_t) + SI91X_MQTT_CLIENT_USERNAME_MAXIMUM_LENGTH
                                     + SI91X_MQTT_CLIENT_PASSWORD_MAXIMUM_LENGTH;

  sl_mqtt_client_credentials_t *mqtt_credentials = malloc(maximum_credential_size);

  if (mqtt_credentials == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  memset(mqtt_credentials, 0, maximum_credential_size);
  sl_net_credential_type_t type = SL_NET_INVALID_CREDENTIAL_TYPE;

  sl_status_t status = sl_net_get_credential(credential_id, &type, mqtt_credentials, &maximum_credential_size);

  if (status != SL_STATUS_OK || type != SL_NET_MQTT_CLIENT_CREDENTIAL) {
    free(mqtt_credentials);
    return status != SL_STATUS_OK ? status : SL_STATUS_INVALID_CREDENTIALS;
  }

  if (mqtt_credentials->username_length >= SI91X_MQTT_CLIENT_USERNAME_MAXIMUM_LENGTH
      || mqtt_credentials->password_length >= SI91X_MQTT_CLIENT_PASSWORD_MAXIMUM_LENGTH) {
    free(mqtt_credentials);
    return SL_STATUS_INVALID_PARAMETER;
  }

  *credentials = mqtt_credentials;
  return SL_STATUS_OK;
}

/**
 * @brief Sends the MQTT init command to the firmware.
 * 
 * @param client[in]        Pointer to the MQTT client object.
 * @param credentials[out]   Pointer to the MQTT client credentials.
 *
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 */
static sl_status_t sli_si91x_send_firmware_mqtt_init(const sl_mqtt_client_t *client,
                                                     const sl_mqtt_client_credentials_t *credentials)
{

  si91x_mqtt_client_init_request_t si91x_init_request = { 0 };

  memcpy(&si91x_init_request.server_ip.server_ip_address,
         &client->broker->ip.ip,
         client->broker->ip.type == SL_IPV4 ? SL_IPV4_ADDRESS_LENGTH : SL_IPV6_ADDRESS_LENGTH);
  si91x_init_request.server_ip.ip_version = client->broker->ip.type == SL_IPV6 ? SL_IPV6_VERSION : SL_IPV4_VERSION;

  si91x_init_request.command_type = SLI_SI91X_MQTT_CLIENT_INIT_COMMAND;

  si91x_init_request.server_port = client->broker->port;
  si91x_init_request.clean       = client->client_configuration->is_clean_session;
  si91x_init_request.client_port = client->client_configuration->client_port;

  // Maintain backward compatibility by checking if tls_flags are non-zero
  if (client->client_configuration->tls_flags != 0) {
    si91x_init_request.encrypt = (uint8_t)client->client_configuration->tls_flags;
  } else {
    si91x_init_request.encrypt = client->broker->is_connection_encrypted;
  }

  si91x_init_request.client_id_len = client->client_configuration->client_id_length;
  memcpy(si91x_init_request.client_id,
         client->client_configuration->client_id,
         client->client_configuration->client_id_length);

  si91x_init_request.keep_alive_interval = client->broker->keep_alive_interval;
  si91x_init_request.keep_alive_retries  = client->broker->keep_alive_retries;

  if (credentials != NULL) {

    memcpy(si91x_init_request.user_name, &credentials->data[0], credentials->username_length);

    // credentials.username_length is being used as offset as we store both user_name, password in same array.
    memcpy(si91x_init_request.password, &credentials->data[credentials->username_length], credentials->password_length);

    si91x_init_request.username_len = (uint8_t)(credentials->username_length);
    si91x_init_request.password_len = (credentials->password_length);
  }

  return sli_si91x_driver_send_command(SLI_WLAN_REQ_EMB_MQTT_CLIENT,
                                       SLI_SI91X_NETWORK_CMD,
                                       &si91x_init_request,
                                       sizeof(si91x_init_request),
                                       SLI_WLAN_RSP_EMB_MQTT_CLIENT_WAIT_TIME,
                                       NULL,
                                       NULL);
}

sl_status_t sl_mqtt_client_init(sl_mqtt_client_t *client, sl_mqtt_client_event_handler_t event_handler)
{
  SL_VERIFY_POINTER_OR_RETURN(event_handler, SL_STATUS_WIFI_NULL_PTR_ARG);

  client->client_event_handler = event_handler;
  sl_slist_init((sl_slist_node_t **)&client->subscription_list_head);

  mqtt_client = client;
  return SL_STATUS_OK;
}

sl_status_t sl_mqtt_client_deinit(sl_mqtt_client_t *client)
{

  VERIFY_AND_RETURN_ERROR_IF_FALSE(client->state == SL_MQTT_CLIENT_DISCONNECTED, SL_STATUS_INVALID_STATE);
  memset(client, 0, sizeof(sl_mqtt_client_t));

  mqtt_client = NULL;
  return SL_STATUS_OK;
}

sl_status_t sl_mqtt_client_connect(sl_mqtt_client_t *client,
                                   const sl_mqtt_broker_t *broker,
                                   const sl_mqtt_client_last_will_message_t *last_will,
                                   const sl_mqtt_client_configuration_t *configuration,
                                   uint32_t connect_timeout)
{
  SL_VERIFY_POINTER_OR_RETURN(client, SL_STATUS_WIFI_NULL_PTR_ARG);

  VERIFY_AND_RETURN_ERROR_IF_FALSE(
    (client->state == SL_MQTT_CLIENT_DISCONNECTED || client->state == SL_MQTT_CLIENT_TA_INIT),
    SL_STATUS_INVALID_STATE);

  // check if this the first time connect() call being made,
  // In subsequent calls it not mandatory to pass parameters as we store them in client structure
  if (!is_connect_previously_called(client)) {
    SL_VERIFY_POINTER_OR_RETURN(broker, SL_STATUS_WIFI_NULL_PTR_ARG);
    SL_VERIFY_POINTER_OR_RETURN(configuration, SL_STATUS_WIFI_NULL_PTR_ARG);
  }

  if ((configuration != NULL && configuration->client_id_length >= SLI_SI91X_MQTT_CLIENT_ID_MAXIMUM_LENGTH)
      || (last_will != NULL && last_will->will_topic_length >= SI91X_MQTT_CLIENT_WILL_TOPIC_MAXIMUM_LENGTH)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status;
  sli_si91x_mqtt_client_connect_request_t si91x_connect_request = { 0 };
  sl_si91x_mqtt_client_context_t *sdk_context                   = NULL;
  sl_mqtt_client_credentials_t *credentials                     = NULL;

  // If user provides valid values in subsequent calls, we store the new values else we keep referring to structures provided in first connect call.
  client->broker               = broker == NULL ? client->broker : broker;
  client->client_configuration = configuration == NULL ? client->client_configuration : configuration;

  // Special case for last_will, as NULL can be a legitimate value if client wouldn't want to provide a last will.
  client->last_will_message = last_will;

  status = sli_si91x_fetch_mqtt_client_credentials(client->client_configuration->credential_id, &credentials);

  if (status != SL_STATUS_OK) {
    SL_CLEANUP_MALLOC(credentials);
    return status;
  }

  // Host connect() call maps to two commands in firmware, init() and connect()
  // since we can't send(At least with current design) two command in aysnc mode,
  // We send init command in sync mode, whereas, connect will be sent as async
  if (client->state == SL_MQTT_CLIENT_DISCONNECTED) {
    status = sli_si91x_send_firmware_mqtt_init(client, credentials);

    if (status != SL_STATUS_OK) {
      SL_CLEANUP_MALLOC(credentials);

      client->state = SL_MQTT_CLIENT_DISCONNECTED;
      return status;
    }

    client->state = SL_MQTT_CLIENT_TA_INIT;
  }

  si91x_connect_request.command_type = SLI_SI91X_MQTT_CLIENT_CONNECT_COMMAND;
  // NWP takes the username and password from init_request and validation bit from the connect request.
  if (credentials != NULL) {
    si91x_connect_request.is_password_present = 1;
    si91x_connect_request.is_username_present = 1;

    SL_CLEANUP_MALLOC(credentials);
  }

  if (client->last_will_message != NULL) {
    si91x_connect_request.will_retain = client->last_will_message->is_retained;
    si91x_connect_request.will_qos    = (uint8_t)(client->last_will_message->will_qos_level);
    si91x_connect_request.will_topic_len =
      (uint8_t)(client->last_will_message->will_topic_length); // Narrowing of variable
    si91x_connect_request.will_message_len =
      (uint8_t)(client->last_will_message->will_message_length); // Narrowing of variable

    memcpy(si91x_connect_request.will_topic,
           client->last_will_message->will_topic,
           client->last_will_message->will_topic_length);
    memcpy(si91x_connect_request.will_msg,
           client->last_will_message->will_message,
           client->last_will_message->will_message_length);

    si91x_connect_request.will_flag = 1;
  }

  status = sli_si91x_build_mqtt_sdk_context_if_async(SL_MQTT_CLIENT_CONNECTED_EVENT,
                                                     client,
                                                     NULL,
                                                     NULL,
                                                     connect_timeout,
                                                     &sdk_context);
  VERIFY_STATUS_AND_RETURN(status);

  status = sli_si91x_driver_send_command(
    SLI_WLAN_REQ_EMB_MQTT_CLIENT,
    SLI_SI91X_NETWORK_CMD,
    &si91x_connect_request,
    sizeof(si91x_connect_request),
    connect_timeout == 0 ? SLI_WIFI_RETURN_IMMEDIATELY : SLI_WIFI_WAIT_FOR(connect_timeout),
    sdk_context,
    NULL);

  if (status == SL_STATUS_IN_PROGRESS) {
    return status;
  } else if (status != SL_STATUS_OK) {
    if (client->state == SL_MQTT_CLIENT_DISCONNECTED || status == SL_STATUS_SI91X_COMMAND_ISSUED_IN_REJOIN_STATE) {
      SL_DEBUG_LOG("\r\nWLAN disconnected. No need to call the disconnect again.\r\n");
      return status;
    }
    client->state = SL_MQTT_CLIENT_CONNECTION_FAILED;
    status        = sl_mqtt_client_disconnect(client, SI91X_MQTT_CLIENT_DISCONNECT_TIMEOUT);
    if (status != SL_STATUS_OK) {
      SL_DEBUG_LOG(
        "Failed to disconnect the client after failed connection attempt. User needs to call disconnect explicitly.");
    }
    SL_CLEANUP_MALLOC(sdk_context);
    return status;
  }

  client->state = SL_MQTT_CLIENT_CONNECTED;
  return SL_STATUS_OK;
}

sl_status_t sl_mqtt_client_disconnect(sl_mqtt_client_t *client, uint32_t timeout)
{

  VERIFY_AND_RETURN_ERROR_IF_FALSE(
    (client->state == SL_MQTT_CLIENT_CONNECTED || client->state == SL_MQTT_CLIENT_TA_DISCONNECTED
     || client->state == SL_MQTT_CLIENT_CONNECTION_FAILED),
    SL_STATUS_INVALID_STATE);

  SL_VERIFY_POINTER_OR_RETURN(client, SL_STATUS_WIFI_NULL_PTR_ARG);

  sl_status_t status                          = SL_STATUS_OK;
  sl_si91x_mqtt_client_context_t *sdk_context = NULL;

  // As in connect, disconnect() call maps to disconnect and deinit in firmware
  // We need to call disconnect even if the previous connect call was failed.
  if (client->state == SL_MQTT_CLIENT_CONNECTED) {
    sli_si91x_mqtt_client_command_request_t si91x_disconnect_request = { .command_type =
                                                                           SLI_SI91X_MQTT_CLIENT_DISCONNECT_COMMAND };

    status = sli_si91x_driver_send_command(SLI_WLAN_REQ_EMB_MQTT_CLIENT,
                                           SLI_SI91X_NETWORK_CMD,
                                           &si91x_disconnect_request,
                                           sizeof(si91x_disconnect_request),
                                           SLI_WLAN_RSP_EMB_MQTT_CLIENT_WAIT_TIME,
                                           sdk_context,
                                           NULL);

    VERIFY_STATUS_AND_RETURN(status);

    client->state = SL_MQTT_CLIENT_TA_DISCONNECTED;
  }

  sli_si91x_mqtt_client_command_request_t si91x_deinit_request = { .command_type =
                                                                     SLI_SI91X_MQTT_CLIENT_DEINIT_COMMAND };

  status = sli_si91x_build_mqtt_sdk_context_if_async(SL_MQTT_CLIENT_DISCONNECTED_EVENT,
                                                     client,
                                                     NULL,
                                                     NULL,
                                                     timeout,
                                                     &sdk_context);
  VERIFY_STATUS_AND_RETURN(status);

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_EMB_MQTT_CLIENT,
                                         SLI_SI91X_NETWORK_CMD,
                                         &si91x_deinit_request,
                                         sizeof(si91x_deinit_request),
                                         timeout <= 0 ? SLI_WIFI_RETURN_IMMEDIATELY : SLI_WIFI_WAIT_FOR(timeout),
                                         sdk_context,
                                         NULL);

  if (status == SL_STATUS_IN_PROGRESS) {
    return status;
  } else if (status != SL_STATUS_OK) {
    SL_CLEANUP_MALLOC(sdk_context);
    return status;
  }

  client->state = SL_MQTT_CLIENT_DISCONNECTED;
  sli_si91x_remove_and_free_all_subscriptions(client);

  return SL_STATUS_OK;
}

sl_status_t sl_mqtt_client_publish(sl_mqtt_client_t *client,
                                   const sl_mqtt_client_message_t *message,
                                   uint32_t timeout,
                                   void *context)
{
  VERIFY_AND_RETURN_ERROR_IF_FALSE(client->state == SL_MQTT_CLIENT_CONNECTED, SL_STATUS_INVALID_STATE);

  SL_VERIFY_POINTER_OR_RETURN(client, SL_STATUS_WIFI_NULL_PTR_ARG);

  if (message->topic_length >= SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status;
  sl_si91x_mqtt_client_context_t *sdk_context = NULL;
  uint32_t publish_request_size = sizeof(sli_si91x_mqtt_client_publish_request_t) + message->content_length;

  sli_si91x_mqtt_client_publish_request_t *si91x_publish_request = calloc(publish_request_size, 1);
  if (si91x_publish_request == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  status = sli_si91x_build_mqtt_sdk_context_if_async(SL_MQTT_CLIENT_MESSAGE_PUBLISHED_EVENT,
                                                     client,
                                                     context,
                                                     NULL,
                                                     timeout,
                                                     &sdk_context);

  if (status != SL_STATUS_OK) {
    SL_CLEANUP_MALLOC(si91x_publish_request);
    return SL_STATUS_ALLOCATION_FAILED;
  }

  si91x_publish_request->command_type = SLI_SI91X_MQTT_CLIENT_PUBLISH_COMMAND;

  si91x_publish_request->dup      = message->is_duplicate_message;
  si91x_publish_request->qos      = (uint8_t)(message->qos_level);
  si91x_publish_request->retained = message->is_retained;

  si91x_publish_request->topic_len = (uint8_t)(message->topic_length);    // Narrowing of variable
  si91x_publish_request->msg_len   = (uint16_t)(message->content_length); // Narrowing of variable

  si91x_publish_request->msg = (int8_t *)si91x_publish_request + sizeof(sli_si91x_mqtt_client_publish_request_t);
  memcpy(si91x_publish_request->topic, message->topic, message->topic_length);
  memcpy(si91x_publish_request->msg, message->content, message->content_length);

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_EMB_MQTT_CLIENT,
                                         SLI_SI91X_NETWORK_CMD,
                                         si91x_publish_request,
                                         publish_request_size,
                                         timeout <= 0 ? SLI_WIFI_RETURN_IMMEDIATELY : SLI_WIFI_WAIT_FOR(timeout),
                                         sdk_context,
                                         NULL);
  free(si91x_publish_request);

  if (status == SL_STATUS_IN_PROGRESS) {
    return status;
  }

  SL_CLEANUP_MALLOC(sdk_context);
  VERIFY_STATUS_AND_RETURN(status);

  return status;
}

sl_status_t sl_mqtt_client_subscribe(sl_mqtt_client_t *client,
                                     const uint8_t *topic,
                                     uint16_t topic_length,
                                     sl_mqtt_qos_t qos_level,
                                     uint32_t timeout,
                                     sl_mqtt_client_message_received_t message_handler,
                                     void *context)
{
  VERIFY_AND_RETURN_ERROR_IF_FALSE(client->state == SL_MQTT_CLIENT_CONNECTED, SL_STATUS_INVALID_STATE);

  SL_VERIFY_POINTER_OR_RETURN(client, SL_STATUS_WIFI_NULL_PTR_ARG);
  SL_VERIFY_POINTER_OR_RETURN(topic, SL_STATUS_WIFI_NULL_PTR_ARG);
  SL_VERIFY_POINTER_OR_RETURN(message_handler, SL_STATUS_WIFI_NULL_PTR_ARG);

  if (topic_length >= SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status;
  sli_si91x_mqtt_client_subscribe_t si91x_subscribe_request = { 0 };
  sl_si91x_mqtt_client_context_t *sdk_context               = NULL;

  sl_mqtt_client_topic_subscription_info_t *subscription =
    calloc(sizeof(sl_mqtt_client_topic_subscription_info_t) + topic_length, 1);

  status = sli_si91x_build_mqtt_sdk_context_if_async(SL_MQTT_CLIENT_SUBSCRIBED_EVENT,
                                                     client,
                                                     context,
                                                     subscription,
                                                     timeout,
                                                     &sdk_context);

  if (subscription == NULL || status != SL_STATUS_OK) {
    SL_CLEANUP_MALLOC(subscription);
    SL_CLEANUP_MALLOC(sdk_context);

    return SL_STATUS_ALLOCATION_FAILED;
  }

  si91x_subscribe_request.command_type = SLI_SI91X_MQTT_CLIENT_SUBSCRIBE_COMMAND;

  si91x_subscribe_request.topic_len = (uint8_t)topic_length;
  si91x_subscribe_request.qos       = (int8_t)qos_level;

  subscription->qos_of_subscription   = qos_level;
  subscription->topic_length          = topic_length;
  subscription->topic_message_handler = message_handler;

  memcpy(si91x_subscribe_request.topic, topic, topic_length);
  memcpy(subscription->topic, topic, topic_length);

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_EMB_MQTT_CLIENT,
                                         SLI_SI91X_NETWORK_CMD,
                                         &si91x_subscribe_request,
                                         sizeof(si91x_subscribe_request),
                                         timeout <= 0 ? SLI_WIFI_RETURN_IMMEDIATELY : SLI_WIFI_WAIT_FOR(timeout),
                                         sdk_context,
                                         NULL);

  if (status == SL_STATUS_IN_PROGRESS) {
    return status;
  } else if (status != SL_STATUS_OK) {

    free(subscription);
    SL_CLEANUP_MALLOC(sdk_context);
    return status;
  }

  sl_slist_push((sl_slist_node_t **)&client->subscription_list_head, (sl_slist_node_t *)subscription);
  return status;
}

sl_status_t sl_mqtt_client_unsubscribe(sl_mqtt_client_t *client,
                                       const uint8_t *topic,
                                       uint16_t topic_length,
                                       uint32_t timeout,
                                       void *context)
{
  VERIFY_AND_RETURN_ERROR_IF_FALSE(client->state == SL_MQTT_CLIENT_CONNECTED, SL_STATUS_INVALID_STATE);

  SL_VERIFY_POINTER_OR_RETURN(client, SL_STATUS_WIFI_NULL_PTR_ARG);
  SL_VERIFY_POINTER_OR_RETURN(topic, SL_STATUS_WIFI_NULL_PTR_ARG);

  if (topic_length >= SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status;
  sl_si91x_mqtt_client_context_t *sdk_context                           = NULL;
  sli_si91x_mqtt_client_unsubscribe_request_t si91x_unsubscribe_request = { 0 };
  sl_mqtt_client_topic_subscription_info_t *subscription;

  sli_si91x_get_subscription(client, topic, topic_length, &subscription);

  status = sli_si91x_build_mqtt_sdk_context_if_async(SL_MQTT_CLIENT_UNSUBSCRIBED_EVENT,
                                                     client,
                                                     context,
                                                     subscription,
                                                     timeout,
                                                     &sdk_context);

  VERIFY_STATUS_AND_RETURN(status);
  si91x_unsubscribe_request.command_type = SLI_SI91X_MQTT_CLIENT_UNSUBSCRIBE_COMMAND;
  si91x_unsubscribe_request.topic_len    = (uint8_t)topic_length;
  memcpy(si91x_unsubscribe_request.topic, topic, topic_length);

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_EMB_MQTT_CLIENT,
                                         SLI_SI91X_NETWORK_CMD,
                                         &si91x_unsubscribe_request,
                                         sizeof(si91x_unsubscribe_request),
                                         timeout <= 0 ? SLI_WIFI_RETURN_IMMEDIATELY : SLI_WIFI_WAIT_FOR(timeout),
                                         sdk_context,
                                         NULL);

  if (status == SL_STATUS_IN_PROGRESS) {
    return status;
  } else if (status != SL_STATUS_OK) {

    SL_CLEANUP_MALLOC(sdk_context);
    return status;
  }

  if (subscription != NULL) {
    sl_slist_remove((sl_slist_node_t **)&client->subscription_list_head, (sl_slist_node_t *)subscription);
    free(subscription);
  }

  return status;
}

static uint8_t sli_si91x_mqtt_identification_function(sl_wifi_buffer_t *buffer, const void *user_data)
{
  UNUSED_PARAMETER(user_data);
  if (buffer == NULL)
    return false;
  const sl_wifi_system_packet_t *packet = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  return (SLI_WLAN_REQ_EMB_MQTT_CLIENT == packet->command);
}

sl_status_t sli_si91x_mqtt_event_handler(sl_status_t status,
                                         sl_si91x_mqtt_client_context_t *sdk_context,
                                         sl_wifi_system_packet_t *rx_packet)
{
  bool is_error_event                          = false;
  uint8_t *event_data                          = NULL;
  sl_mqtt_client_disconnection_reason_t reason = { 0 };

  switch (sdk_context->event) {
    case SL_MQTT_CLIENT_CONNECTED_EVENT:
      sli_si91x_handle_connected_event(status, sdk_context, rx_packet, &is_error_event, &event_data, &reason);
      break;

    case SL_MQTT_CLIENT_SUBSCRIBED_EVENT:
      sli_si91x_handle_subscribed_event(status, sdk_context, &is_error_event);
      break;

    case SL_MQTT_CLIENT_UNSUBSCRIBED_EVENT:
      sli_si91x_handle_unsubscribed_event(status, sdk_context, &is_error_event);
      break;

    case SL_MQTT_CLIENT_MESSAGED_RECEIVED_EVENT:
      sli_si91x_handle_message_received_event(sdk_context, rx_packet);
      return SL_STATUS_OK;

    case SL_MQTT_CLIENT_DISCONNECTED_EVENT:
      sli_si91x_handle_disconnected_event(status, sdk_context, rx_packet, &is_error_event, &event_data, &reason);
      break;

    case SL_MQTT_CLIENT_MESSAGE_PUBLISHED_EVENT:
      if (status != SL_STATUS_OK) {
        is_error_event = true;
      }
      break;

    default:
      break;
  }

  // Populate the error status only if the event is an error event.
  sl_mqtt_client_error_status_t *error_status = NULL;
  if (is_error_event) {
    error_status = malloc(sizeof(sl_mqtt_client_error_status_t));
    if (error_status == NULL) {
      free(sdk_context);
      return SL_STATUS_ALLOCATION_FAILED;
    }
    *error_status = sli_si91x_get_event_error_status(sdk_context->event);
  }

  sdk_context->client->client_event_handler(sdk_context->client,
                                            is_error_event ? SL_MQTT_CLIENT_ERROR_EVENT : sdk_context->event,
                                            is_error_event ? (void *)error_status : (void *)event_data,
                                            sdk_context->user_context);

  // Free the sdk_context after event handler is triggered.
  free(sdk_context);
  // Free error_status if it was allocated.
  if (error_status != NULL) {
    free(error_status);
  }
  return SL_STATUS_OK;
}

static void sli_si91x_handle_connected_event(sl_status_t status,
                                             sl_si91x_mqtt_client_context_t *sdk_context,
                                             const sl_wifi_system_packet_t *rx_packet,
                                             bool *is_error_event,
                                             uint8_t **event_data,
                                             sl_mqtt_client_disconnection_reason_t *reason)
{
  if (status == SL_STATUS_OK) {
    sdk_context->client->state = SL_MQTT_CLIENT_CONNECTED;
    return;
  }

  if (rx_packet->command == SLI_WIFI_RSP_JOIN) {
    *reason                    = SL_MQTT_CLIENT_WLAN_DISCONNECTION;
    *event_data                = (uint8_t *)reason;
    sdk_context->client->state = SL_MQTT_CLIENT_DISCONNECTED;
    // Free all subscriptions as we have disconnected from MQTT broker
    sli_si91x_remove_and_free_all_subscriptions(sdk_context->client);
    return;
  }

  *is_error_event = true;
  // This state updates is necessary as we need to send NWP disconnect even in case of connection failure.
  sdk_context->client->state = SL_MQTT_CLIENT_CONNECTION_FAILED;
  // NWP requires deinit call if connection fails for any reason.
  sl_status_t disconnection_status = SL_STATUS_FAIL;
  disconnection_status = sl_mqtt_client_disconnect(sdk_context->client, SI91X_MQTT_CLIENT_DISCONNECT_TIMEOUT);

  if (disconnection_status != SL_STATUS_OK) {
    SL_DEBUG_LOG(
      "Failed to disconnect the client after failed connection attempt. User needs to call disconnect explicitly.");
  } else {
    sdk_context->client->state = SL_MQTT_CLIENT_DISCONNECTED;
  }
  return;
}

static void sli_si91x_handle_subscribed_event(sl_status_t status,
                                              sl_si91x_mqtt_client_context_t *sdk_context,
                                              bool *is_error_event)
{
  if (status != SL_STATUS_OK) {
    // Free subscription passed in subscribe() call if subscription call failed.
    free(sdk_context->sdk_data);
    *is_error_event = true;
    return;
  }

  // As subscription is success, add the subscription to list.
  sl_slist_push((sl_slist_node_t **)&sdk_context->client->subscription_list_head,
                (sl_slist_node_t *)sdk_context->sdk_data);
  return;
}

static void sli_si91x_handle_unsubscribed_event(sl_status_t status,
                                                sl_si91x_mqtt_client_context_t *sdk_context,
                                                bool *is_error_event)
{
  if (status != SL_STATUS_OK) {
    *is_error_event = true;
    return;
  }

  // Free subscription if the unsubscription API call is successful.
  sl_slist_remove((sl_slist_node_t **)&sdk_context->client->subscription_list_head,
                  (sl_slist_node_t *)sdk_context->sdk_data);
  free(sdk_context->sdk_data);
  return;
}

static void sli_si91x_handle_message_received_event(sl_si91x_mqtt_client_context_t *sdk_context,
                                                    sl_wifi_system_packet_t *rx_packet)
{
  // Extract the MQTT message from payload and create sl_mqtt_message
  sl_mqtt_client_message_t received_message;
  sl_mqtt_client_topic_subscription_info_t *subscription;

  sli_si91x_mqtt_client_received_message_t *si91x_message = (sli_si91x_mqtt_client_received_message_t *)rx_packet->data;

  received_message.topic          = si91x_message->data;
  received_message.topic_length   = si91x_message->topic_length;
  received_message.content_length = si91x_message->current_chunk_length;
  received_message.content        = &si91x_message->data[si91x_message->topic_length];

  // Extract the MQTT flags from the received message
  // The flags are stored in the first four bits of the mqtt_flags field

  // Use the SI91X_MQTT_CHECK_RETAIN_MESSAGE macro to extract the zeroth bit and determine if the message is retained
  received_message.is_retained = si91x_message->mqtt_flags & SI91X_MQTT_CHECK_RETAIN_MESSAGE;

  // Use the SI91X_MQTT_CHECK_QOS_LEVEL macro to extract the first and second bits and determine the QoS level
  received_message.qos_level = (si91x_message->mqtt_flags & SI91X_MQTT_CHECK_QOS_LEVEL) >> 1;

  // Use the SI91X_MQTT_CHECK_IS_DUPLICATE_MESSAGE macro to extract the third bit and determine if the message is a duplicate
  received_message.is_duplicate_message = si91x_message->mqtt_flags & SI91X_MQTT_CHECK_IS_DUPLICATE_MESSAGE;

  sli_si91x_get_subscription(sdk_context->client, received_message.topic, received_message.topic_length, &subscription);

  if (subscription == NULL) {
    SL_DEBUG_LOG("Unable to find subscription: Dropping MQTT message handling");
  } else {
    subscription->topic_message_handler(sdk_context->client, &received_message, sdk_context->user_context);
  }

  free(sdk_context);
  return;
}

static void sli_si91x_handle_disconnected_event(sl_status_t status,
                                                sl_si91x_mqtt_client_context_t *sdk_context,
                                                const sl_wifi_system_packet_t *rx_packet,
                                                bool *is_error_event,
                                                uint8_t **event_data,
                                                sl_mqtt_client_disconnection_reason_t *reason)
{
  /* 
     If the received packet is a disconnect packet and the status is not successful, 
     it implies that the disconnect was initiated by the user and failed. 
     However, if the status is SL_STATUS_SI91X_MQTT_KEEP_ALIVE_TERMINATE_ERROR, 
     it indicates a keep-alive terminate error, not a user-initiated disconnect failure.
  */
  if (rx_packet->command == SLI_WLAN_REQ_EMB_MQTT_CLIENT && status != SL_STATUS_OK
      && status != SL_STATUS_SI91X_MQTT_KEEP_ALIVE_TERMINATE_ERROR) {
    *is_error_event = true;
    return;
  }

  /* Flush the pending tx request packets from the network command queue */
  sl_status_t flush_status =
    sli_si91x_flush_queue_based_on_type(&cmd_queues[SLI_SI91X_NETWORK_CMD],
                                        SL_SI91X_RESPONSE_FLAG(SLI_SI91X_NETWORK_CMD),
                                        (uint16_t)status,
                                        (sli_si91x_compare_function_t)sli_si91x_mqtt_identification_function,
                                        NULL);
  if (flush_status != SL_STATUS_OK) {
    SL_DEBUG_LOG("\r\nFlush nodes failed Status:%ld\r\n", status);
  }

  bool is_keep_alive_response_timeout_termination =
    (rx_packet->command == SLI_WLAN_REQ_EMB_MQTT_CLIENT && status == SL_STATUS_SI91X_MQTT_KEEP_ALIVE_TERMINATE_ERROR);
  sl_status_t disconnection_status = SL_STATUS_FAIL;

  if (rx_packet->command == SLI_WLAN_RSP_MQTT_REMOTE_TERMINATE || is_keep_alive_response_timeout_termination) {
    sdk_context->client->state = SL_MQTT_CLIENT_TA_DISCONNECTED;
    disconnection_status       = sl_mqtt_client_disconnect(sdk_context->client, SI91X_MQTT_CLIENT_DISCONNECT_TIMEOUT);

    // NWP requires deinit call if remote termination is received.
    // If the disconnect call fails, we can't set the state to disconnected.
    if (disconnection_status != SL_STATUS_OK) {
      SL_DEBUG_LOG(
        "Failed to disconnect the client after remote termination. User needs to call disconnect explicitly");
      return;
    }

    *reason     = is_keep_alive_response_timeout_termination ? SL_MQTT_CLIENT_KEEP_ALIVE_RESPONSE_TIMEOUT_DISCONNECTION
                                                             : SL_MQTT_CLIENT_REMOTE_TERMINATE_DISCONNECTION;
    *event_data = (uint8_t *)reason;
  } else {
    // As keep alive response timeout is already handled,
    // we can safely assume that the disconnection is user initiated if the frame type is SLI_WLAN_REQ_EMB_MQTT_CLIENT.
    *reason     = (rx_packet->command == SLI_WIFI_RSP_JOIN) ? SL_MQTT_CLIENT_WLAN_DISCONNECTION
                                                            : SL_MQTT_CLIENT_USER_INITIATED_DISCONNECTION;
    *event_data = (uint8_t *)reason;
  }

  if (rx_packet->command == SLI_WIFI_RSP_JOIN || rx_packet->command == SLI_WLAN_REQ_EMB_MQTT_CLIENT
      || ((rx_packet->command == SLI_WLAN_RSP_MQTT_REMOTE_TERMINATE || is_keep_alive_response_timeout_termination)
          && disconnection_status == SL_STATUS_OK)) {
    sdk_context->client->state = SL_MQTT_CLIENT_DISCONNECTED;
    // Free all subscriptions as we have disconnected from mqtt broker
    sli_si91x_remove_and_free_all_subscriptions(sdk_context->client);
  }
  return;
}

void sli_mqtt_client_cleanup()
{
  if (mqtt_client == NULL) {
    SL_DEBUG_LOG("MQTT client instance not initialized yet\n");
    return;
  }
  sli_si91x_remove_and_free_all_subscriptions(mqtt_client);
  memset(mqtt_client, 0, sizeof(sl_mqtt_client_t));
  mqtt_client = NULL;
}

sl_status_t sl_mqtt_client_connect_v2(sl_mqtt_client_t *client,
                                      const sl_mqtt_broker_v2_t *broker,
                                      const sl_mqtt_client_last_will_message_t *last_will_message,
                                      const sl_mqtt_client_configuration_t *configuration,
                                      uint32_t connect_timeout)
{
  sl_status_t status;

  // Only handle SNI and broker conversion if broker is provided
  sl_mqtt_broker_t *legacy_broker_ptr = NULL;
  if (broker != NULL) {
    /* If TLS + SNI requested, program SNI into firmware before CONNECT.
       Use sni_host_name from v2 config. */
    if (broker->enable_sni) {
      // Validate SNI host name presence and length
      if (broker->sni_host_name == NULL || sl_strlen((const char *)broker->sni_host_name) == 0) {
        return SL_STATUS_INVALID_PARAMETER; // SNI enabled but no valid host name provided
      }
      size_t sni_host_name_len = sl_strlen((const char *)broker->sni_host_name);
      sli_si91x_tls_extension_info_t *internal_sni =
        (sli_si91x_tls_extension_info_t *)malloc(sizeof(sli_si91x_tls_extension_info_t) + sni_host_name_len);
      if (internal_sni == NULL) {
        return SL_STATUS_ALLOCATION_FAILED;
      }
      internal_sni->type   = SL_SI91X_TLS_EXTENSION_SNI_TYPE;
      internal_sni->length = (uint16_t)sni_host_name_len;
      memcpy(internal_sni->value, broker->sni_host_name, sni_host_name_len);
      status = sli_configure_sni(internal_sni, broker->sni_host_name, SI91X_SNI_FOR_MQTT);
      free(internal_sni);
      if (status != SL_STATUS_OK) {
        return status;
      }
    }
    // Convert sl_mqtt_broker_v2_t to sl_mqtt_broker_t for legacy API compatibility
    legacy_broker_ptr = (sl_mqtt_broker_t *)malloc(sizeof(sl_mqtt_broker_t));
    if (legacy_broker_ptr == NULL) {
      return SL_STATUS_ALLOCATION_FAILED;
    }
    legacy_broker_ptr->ip                      = broker->ip;
    legacy_broker_ptr->port                    = broker->port;
    legacy_broker_ptr->is_connection_encrypted = broker->is_connection_encrypted;
    legacy_broker_ptr->connect_timeout         = broker->connect_timeout;
    legacy_broker_ptr->keep_alive_interval     = broker->keep_alive_interval;
    legacy_broker_ptr->keep_alive_retries      = broker->keep_alive_retries;
  }

  status = sl_mqtt_client_connect(client, legacy_broker_ptr, last_will_message, configuration, connect_timeout);
  if (legacy_broker_ptr != NULL) {
    free(legacy_broker_ptr);
  }
  return status;
}