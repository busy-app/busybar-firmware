/***************************************************************************/ /**
 * @file  sl_mqtt_client.h
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
#include "sl_mqtt_client_types.h"
#include "sl_status.h"
#include <stdint.h>

/**
 * @addtogroup SERVICE_MQTT_FUNCTIONS 
 * @{
 */

/***************************************************************************/ /**
 * @brief
 *   Initialize the MQTT client and register the event handler.
 * 
 * @details
 *   This function initializes the MQTT client by setting up the event handler, and initializing the subscription list. It also assigns the provided client structure to the global MQTT client pointer.
 * 
 * @param[in] client
 *   Pointer to the MQTT client structure of type @ref sl_mqtt_client_t. This pointer must not be NULL.
 * 
 * @param[in] event_handler
 *   Event handler of type @ref sl_mqtt_client_event_handler_t that would be called for various events on the client. This pointer must not be NULL.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_WIFI_NULL_PTR_ARG: The event_handler pointer was NULL.
 ******************************************************************************/
sl_status_t sl_mqtt_client_init(sl_mqtt_client_t *client, sl_mqtt_client_event_handler_t event_handler);

/***************************************************************************/ /**
 * @brief
 *   Deinitialize the MQTT client.
 * 
 * @details
 *   This function deinitializes the MQTT client by clearing its state and resetting the global MQTT client pointer. It ensures that the client is in a disconnected state before deinitializing.
 * 
 * @pre
 *   The MQTT client must be in the disconnected state before calling this function.
 * 
 * @param[in] client
 *   Pointer to the MQTT client structure of type @ref sl_mqtt_client_t. This pointer must not be NULL.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_INVALID_STATE: The client is not in the disconnected state.
 ******************************************************************************/
sl_status_t sl_mqtt_client_deinit(sl_mqtt_client_t *client);

/***************************************************************************/
/**
 * @brief
 *   Connect the client to an MQTT broker.
 * 
 * @details
 *   This function connects the MQTT client to the specified broker using the provided configuration and last will message. It supports both synchronous, and asynchronous operations based on the timeout value.
 * 
 * @pre
 *   @ref sl_mqtt_client_init should be called before this API.
 * 
 * @param[in] client
 *   Pointer to the MQTT client structure of type @ref sl_mqtt_client_t that would be connected to the broker.
 * 
 * @param[in] broker
 *   Pointer to the broker configuration of type @ref sl_mqtt_broker_t. This parameter could be NULL for subsequent calls to retain the previous broker configuration.
 * 
 * @param[in] last_will_message
 *   Pointer to the last will message of the client of type @ref sl_mqtt_client_last_will_message_t. This parameter could be NULL for subsequent calls if no will message is to be sent.
 * 
 * @param[in] configuration
 *   Pointer to the client configuration of type @ref sl_mqtt_client_configuration_t. This parameter could be NULL for subsequent calls to retain the previous configuration.
 * 
 * @param[in] timeout
 *   Timeout for the API is in milliseconds. If the value is zero, the API operates in asynchronous mode.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_IN_PROGRESS: Operation is in progress (for asynchronous calls).
 *   - SL_STATUS_INVALID_PARAMETER: One or more parameters are invalid.
 *   - SL_STATUS_FAIL: Operation failed.
 * 
 * @note
 *   In subsequent calls to connect, it is optional to provide broker, last_will_message, and configuration parameters.
 *   If broker and configuration parameters are assigned as NULL, the values are retains from the first connect() call.
 *   For the last_will_message parameter, values assigned in each connect() call are considered. If the last_will_message parameter value is NULL, no will message are sent to the broker.
 *   is_clean_session, credential_id, client_id, and client_id_length of @ref sl_mqtt_client_configuration_t are consider.
 *   The topic length of the last_will_message must be less than SI91X_MQTT_CLIENT_WILL_TOPIC_MAXIMUM_LENGTH.
 *   The client ID length should be less than SI91X_MQTT_CLIENT_CLIENT_ID_MAXIMUM_LENGTH.
 *   The username and password length must be less than SI91X_MQTT_CLIENT_USERNAME_MAXIMUM_LENGTH and SI91X_MQTT_CLIENT_PASSWORD_MAXIMUM_LENGTH, respectively.
 *
 * @note
 *   This function uses a user-configurable timeout parameter that is not affected
 *   by the global timeout scaling factors (SL_WIFI_INTERNAL_COMMANDS_TIMEOUT_SF,
 *   SL_WIFI_MANAGEMENT_COMMANDS_TIMEOUT_SF, SL_WIFI_NETWORK_COMMANDS_TIMEOUT_SF)
 *   or the additional wait time configuration (SL_TX_ADDITIONAL_WAIT_TIME).
 * 
 * @note
 *   Moving forward, this API will be deprecated. Instead, use the [sl-mqtt-client-connect-v2](../wiseconnect-api-reference-guide-mqtt/service-mqtt-functions#sl-mqtt-client-connect-v2) API. This is retained for backward compatibility.
 */
sl_status_t sl_mqtt_client_connect(sl_mqtt_client_t *client,
                                   const sl_mqtt_broker_t *broker,
                                   const sl_mqtt_client_last_will_message_t *last_will_message,
                                   const sl_mqtt_client_configuration_t *configuration,
                                   uint32_t timeout) SL_DEPRECATED_API_WISECONNECT_4_0;

/***************************************************************************/
/**
 * @brief
 *   Connect the client to an MQTT broker using the extended v2 configuration.
 *
 * @details
 *   This function connects the MQTT client to the specified broker using the provided v2 configuration and last will message. It supports both synchronous and asynchronous operations based on the connect_timeout value. The v2 configuration allows enabling Server Name Indication (SNI) via the 'enable_sni' field and passing additional parameters for advanced TLS features.
 *
 * @pre
 *   @ref sl_mqtt_client_init should be called before this API.
 *
 * @param[in] client
 *   Pointer to the MQTT client structure of type @ref sl_mqtt_client_t that would be connected to the broker.
 *
 * @param[in] broker
 *   Pointer to the broker configuration of type @ref sl_mqtt_broker_v2_t. You can set this parameter to NULL in later calls to reuse the previous broker configuration.
 *
 * @param[in] last_will_message
 *   Pointer to the last will message of the client of type @ref sl_mqtt_client_last_will_message_t. You can set this parameter to NULL in later calls if no will message is to be sent.
 *
 * @param[in] configuration
 *   Pointer to the client configuration of type @ref sl_mqtt_client_configuration_t. You can set this parameter to NULL in later calls to reuse the previous broker configuration.
 *
 * @param[in] connect_timeout
 *   Timeout for the API is in milliseconds. If the value is zero, the API operates in asynchronous mode.
 *
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_IN_PROGRESS: Operation is in progress (for asynchronous calls).
 *   - SL_STATUS_INVALID_PARAMETER: One or more parameters are invalid.
 *   - SL_STATUS_FAIL: Operation failed.
 *
 * @note
 *   In subsequent calls to connect, it is optional to provide broker, last_will, and configuration parameters.
 *   If broker and configuration parameters are assigned as NULL, the values are retained from the first connect_v2() call.
 *   For the last_will parameter, values assigned in each connect_v2() call are considered. If the last_will parameter value is NULL, no will message is sent to the broker.
 *   The v2 configuration allows enabling SNI via the 'enable_sni' field and passing a hostname or SNI extension for advanced TLS features.
 *   The topic length of the last_will message must be less than SI91X_MQTT_CLIENT_WILL_TOPIC_MAXIMUM_LENGTH.
 *   The client ID length should be less than SI91X_MQTT_CLIENT_CLIENT_ID_MAXIMUM_LENGTH.
 *   The username and password length must be less than SI91X_MQTT_CLIENT_USERNAME_MAXIMUM_LENGTH and SI91X_MQTT_CLIENT_PASSWORD_MAXIMUM_LENGTH, respectively.
 *
 * @note
 *   This function uses a user-configurable timeout parameter that is not affected
 *   by the global timeout scaling factors (SL_WIFI_INTERNAL_COMMANDS_TIMEOUT_SF,
 *   SL_WIFI_MANAGEMENT_COMMANDS_TIMEOUT_SF, SL_WIFI_NETWORK_COMMANDS_TIMEOUT_SF)
 *   or the additional wait time configuration (SL_TX_ADDITIONAL_WAIT_TIME).
 * 
 * @note
 *   SNI Limitation:
 *   - Only one SNI command can be active at a time, irrespective of protocol (MQTT or HTTP).
 *   - You must wait for the connection to be fully established before initiating another connection with SNI.
 *   - If multiple SNI commands are sent simultaneously, SL_STATUS_SI91X_FEATURE_NOT_AVAILABLE will be returned.
 *   - For reconnection scenarios, set SNI before calling reconnect.
 */
sl_status_t sl_mqtt_client_connect_v2(sl_mqtt_client_t *client,
                                      const sl_mqtt_broker_v2_t *broker,
                                      const sl_mqtt_client_last_will_message_t *last_will_message,
                                      const sl_mqtt_client_configuration_t *configuration,
                                      uint32_t connect_timeout);

/***************************************************************************/ /**
 * @brief
 *   Disconnect the client from the MQTT broker.
 * 
 * @details
 *   This function disconnects the MQTT client from the broker and deinitializes the client context. It supports synchronous, and asynchronous operations based on the timeout value. The function ensures that the client is in a valid state before attempting to disconnect. 
 *   - **Synchronous Mode:** If the timeout value is non-zero then, the function waits for the disconnect operation to complete within the specified timeout period.
 *   - **Asynchronous Mode:** If the timeout value is zero then, the function returns immediately, and the results are provided via the @ref sl_mqtt_client_event_handler_t callback.
 
 * @pre
 *   @ref sl_mqtt_client_connect should be called before this API.
 * 
 * @param[in] client
 *   Pointer to the MQTT client structure of type @ref sl_mqtt_client_t disconnects from the broker. The pointer value must not be NULL.
 * 
 * @param[in] timeout
 *   Timeout for the API in milliseconds. If the value is zero, the API is asynchronous.
 * 
 * @return
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_IN_PROGRESS: Operation is in progress (for asynchronous calls).
 *   - SL_STATUS_INVALID_PARAMETER: One or more parameters are invalid.
 *   - SL_STATUS_INVALID_STATE: The client is not in a valid state to disconnect.
 *   - SL_STATUS_FAIL: Operation failed.
 * 
 * @note
 *   The function deinitializes the MQTT client context after the disconnection.
 *   The client must be in one of the following states to call this function: SL_MQTT_CLIENT_CONNECTED, SL_MQTT_CLIENT_TA_DISCONNECTED, or SL_MQTT_CLIENT_CONNECTION_FAILED.
 *
 * @note
 *   This function uses a user-configurable timeout parameter that is not affected
 *   by the global timeout scaling factors (SL_WIFI_INTERNAL_COMMANDS_TIMEOUT_SF,
 *   SL_WIFI_MANAGEMENT_COMMANDS_TIMEOUT_SF, SL_WIFI_NETWORK_COMMANDS_TIMEOUT_SF)
 *   or the additional wait time configuration (SL_TX_ADDITIONAL_WAIT_TIME).
 ******************************************************************************/
sl_status_t sl_mqtt_client_disconnect(sl_mqtt_client_t *client, uint32_t timeout);

/***************************************************************************/ /**
 * @brief 
 *   Publishes the message to the MQTT broker.
 * 
 * @details
 *   This function publishes a message to the specified topic of the MQTT broker. It supports both synchronous, and asynchronous operations based on the timeout value. The function ensures that the client is in the connected state before attempting to publish the message.
 * 
 * @pre
 *   @ref sl_mqtt_client_connect should be called before this API.
 * 
 * @param[in] client	
 *   Pointer to the MQTT client structure of type @ref sl_mqtt_client_t publishes the message. This pointer value must not be NULL, and the client must be in a connected state.
 * 
 * @param[in] message	
 *   Pointer publishes the message structure of @ref sl_mqtt_client_message_t type.The pointer must not be NULL, and the topic lenth value must be less than SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH.
 * 
 * @param[in] timeout	
 *   Timeout for the API is in milliseconds. If the value is zero, the API is asynchronous.
 * 
 * @param[in] context   
 *   Pointer to the context returns the event handler if the API is called asynchronously. The caller ensures that the lifecycle of the context is retained until the callback is invoked. The deallocation of the context is the responsibility of the caller.
 * 
 * @return			
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_IN_PROGRESS: Operation is in progress (for asynchronous calls).
 *   - SL_STATUS_INVALID_PARAMETER: One or more parameters are invalid.
 *   - SL_STATUS_INVALID_STATE: The client is not in a valid state to publish.
 *   - SL_STATUS_ALLOCATION_FAILED: Memory allocation failed.
 *   - SL_STATUS_FAIL: Operation failed.
 * 
 * @note
 *   The maximum length of the topic must be less than SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH.
 *   The function allocates memory for the publish request, and free the space after the operation.
 *
 * @note
 *   This function uses a user-configurable timeout parameter that is not affected
 *   by the global timeout scaling factors (SL_WIFI_INTERNAL_COMMANDS_TIMEOUT_SF,
 *   SL_WIFI_MANAGEMENT_COMMANDS_TIMEOUT_SF, SL_WIFI_NETWORK_COMMANDS_TIMEOUT_SF)
 *   or the additional wait time configuration (SL_TX_ADDITIONAL_WAIT_TIME).
 ******************************************************************************/
sl_status_t sl_mqtt_client_publish(sl_mqtt_client_t *client,
                                   const sl_mqtt_client_message_t *message,
                                   uint32_t timeout,
                                   void *context);

/***************************************************************************/ /**
 * @brief 
 *   Subscribe a client to a specific topic on the MQTT broker.
 * 
 * @details
 *   This function subscribes the specified client to a given topic on the MQTT broker. It supports both synchronous, and asynchronous operations based on the timeout value. The function ensures that the client is in a connected state before attempting to subscribe to the topic.
 * 
 * @pre
 *   @ref sl_mqtt_client_connect should be called before this API.
 * 
 * @param[in] client			
 *   Pointer to the MQTT client structure of type @ref sl_mqtt_client_t that needs to subscribe to the topic. This pointer must not be NULL and the client must be in a connected state.
 * 
 * @param[in] topic
 *   Pointer to the topic string that the client wants to subscribe to. This pointer must not be NULL.
 * 
 * @param[in] topic_length		
 *   Length of the topic string. The maximum length must be less than SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH.
 * 
 * @param[in] qos_level			
 *   Quality of Service level of type @ref sl_mqtt_qos_t is subscribed by the client to use the topic.
 * 
 * @param[in] timeout			
 *   Timeout for the API is in milliseconds. If the value is zero, the API operates in asynchronous mode.
 * 
 * @param[in] message_handler 	
 *   Pointer to the message handler function of type @ref sl_mqtt_client_message_received_t that will be invoked for messages published on the subscribed topic. This pointer must not be NULL.
 * 
 * @param[in] context
 *   Pointer to the context that will be sent back to the message handler. The caller must ensure that the lifecycle of the context is retained until the callback is invoked. The deallocation of the context is also the responsibility of the caller.
 * 
 * @return					
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_IN_PROGRESS: Operation is in progress (for asynchronous calls).
 *   - SL_STATUS_INVALID_PARAMETER: One or more parameters are invalid.
 *   - SL_STATUS_INVALID_STATE: The client is not in a valid state to subscribe.
 *   - SL_STATUS_ALLOCATION_FAILED: Memory allocation failed.
 *   - SL_STATUS_FAIL: Operation failed.
 * 
 * @note
 *   The maximum length of the topic should be less than SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH.
 *
 * @note
 *   This function uses a user-configurable timeout parameter that is not affected
 *   by the global timeout scaling factors (SL_WIFI_INTERNAL_COMMANDS_TIMEOUT_SF,
 *   SL_WIFI_MANAGEMENT_COMMANDS_TIMEOUT_SF, SL_WIFI_NETWORK_COMMANDS_TIMEOUT_SF)
 *   or the additional wait time configuration (SL_TX_ADDITIONAL_WAIT_TIME).
 ******************************************************************************/
sl_status_t sl_mqtt_client_subscribe(sl_mqtt_client_t *client,
                                     const uint8_t *topic,
                                     uint16_t topic_length,
                                     sl_mqtt_qos_t qos_level,
                                     uint32_t timeout,
                                     sl_mqtt_client_message_received_t message_handler,
                                     void *context);

/***************************************************************************/ /**
 * @brief 
 *   Unsubscribe a client from a specific topic on the MQTT broker.
 * 
 * @details
 *   This function unsubscribes the specified client from a given topic on the MQTT broker. It supports both synchronous, and asynchronous operations based on the timeout value. The function ensures that the client is in a connected state before attempting to unsubscribe from the topic.
 * 
 * @pre
 *   @ref sl_mqtt_client_subscribe should be called before this API.
 * 
 * @param[in] client			
 *   Pointer to the MQTT client structure of type @ref sl_mqtt_client_t that needs to unsubscribe from the topic. This pointer must not be NULL and the client must be in a connected state.
 * 
 * @param[in] topic
 *   Pointer to the topic string that the client wants to unsubscribe from. This pointer must not be NULL.
 * 
 * @param[in] topic_length		
 *   Length of the topic string. The maximum length should be less than SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH.
 * 
 * @param[in] timeout			
 *   Timeout for the API in milliseconds. If the value is zero, the API is asynchronous.
 * 
 * @param[in] context
 *   Pointer to the context that will be sent back to the event handler if the API is called asynchronously. The caller must ensure that the lifecycle of the context is retained until the callback is invoked. The deallocation of the context is also the responsibility of the caller.
 * 
 * @return					
 *   sl_status_t - Status of the operation. For more details, see https://docs.silabs.com/gecko-platform/latest/platform-common/status.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_IN_PROGRESS: Operation is in progress (for asynchronous calls).
 *   - SL_STATUS_INVALID_PARAMETER: One or more parameters are invalid.
 *   - SL_STATUS_INVALID_STATE: The client is not in a valid state to unsubscribe.
 *   - SL_STATUS_ALLOCATION_FAILED: Memory allocation failed.
 *   - SL_STATUS_FAIL: Operation failed.
 * 
 * @note
 *   The maximum length of the topic must be less than SI91X_MQTT_CLIENT_TOPIC_MAXIMUM_LENGTH.
 *
 * @note
 *   This function uses a user-configurable timeout parameter that is not affected
 *   by the global timeout scaling factors (SL_WIFI_INTERNAL_COMMANDS_TIMEOUT_SF,
 *   SL_WIFI_MANAGEMENT_COMMANDS_TIMEOUT_SF, SL_WIFI_NETWORK_COMMANDS_TIMEOUT_SF)
 *   or the additional wait time configuration (SL_TX_ADDITIONAL_WAIT_TIME).
 ******************************************************************************/
sl_status_t sl_mqtt_client_unsubscribe(sl_mqtt_client_t *client,
                                       const uint8_t *topic,
                                       uint16_t topic_length,
                                       uint32_t timeout,
                                       void *context);

/** @} */
