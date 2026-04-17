/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot_core_mqtt.c
 * @brief Implements the port for Azure IoT MQTT based on coreMQTT.
 *
 * @note This interface is private and subjected to change. Currently, there is only
 *       one implementation for this interface, which uses coreMQTT as underlying MQTT stack.
 *
 */

#include <assert.h>

#include "azure_iot_mqtt.h"
#include "sl_mqtt_client.h"
#include <string.h>

/******************************************************
 *                    Constants
 ******************************************************/
//Resolve hostname <hostname>.azure-devices.net and store in azure_mqtt_broker_ip variable
#define MQTT_BROKER_IP   "40.78.196.103"
#define MQTT_BROKER_PORT 8886

#define CLIENT_PORT 4002

#define CLIENT_ID "WISECONNECT-SDK-MQTT-CLIENT-ID"

#define TOPIC_TO_BE_SUBSCRIBED "THERMOSTAT-DATA\0"
#define QOS_OF_SUBSCRIPTION    SL_MQTT_QOS_LEVEL_1

#define PUBLISH_TOPIC          "WISECONNECT_SDK_TOPIC"
#define PUBLISH_MESSAGE        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do"
#define QOS_OF_PUBLISH_MESSAGE 0

#define IS_DUPLICATE_MESSAGE 0
#define IS_MESSAGE_RETAINED  1
#define IS_CLEAN_SESSION     1

#define LAST_WILL_TOPIC       "WISECONNECT-SDK-MQTT-CLIENT-LAST-WILL"
#define LAST_WILL_MESSAGE     "WISECONNECT-SDK-MQTT-CLIENT has been disconnect from network"
#define QOS_OF_LAST_WILL      1
#define IS_LAST_WILL_RETAINED 1

#define ENCRYPT_CONNECTION     0
#define KEEP_ALIVE_INTERVAL    2000
#define MQTT_CONNECT_TIMEOUT   5000
#define MQTT_KEEPALIVE_RETRIES 0

#define SEND_CREDENTIALS 0

#define USERNAME "username"
#define PASSWORD "password"

/******************************************************
 *               Variable Definitions
 ******************************************************/

sl_mqtt_client_credentials_t *client_credentails = NULL;

sl_mqtt_client_configuration_t mqtt_client_configuration = { .is_clean_session = IS_CLEAN_SESSION,
                                                             .client_id        = (uint8_t *)CLIENT_ID,
                                                             .client_id_length = strlen(CLIENT_ID),
                                                             .client_port      = CLIENT_PORT };

sl_mqtt_broker_t mqtt_broker_configuration = {
  .port                    = MQTT_BROKER_PORT,
  .is_connection_encrypted = ENCRYPT_CONNECTION,
  .connect_timeout         = MQTT_CONNECT_TIMEOUT,
  .keep_alive_interval     = KEEP_ALIVE_INTERVAL,
  .keep_alive_retries      = MQTT_KEEPALIVE_RETRIES,
};

sl_mqtt_client_message_t message_to_be_published = {
  .qos_level            = QOS_OF_PUBLISH_MESSAGE,
  .is_retained          = IS_MESSAGE_RETAINED,
  .is_duplicate_message = IS_DUPLICATE_MESSAGE,
  .topic                = (uint8_t *)PUBLISH_TOPIC,
  .topic_length         = strlen(PUBLISH_TOPIC),
  .content              = (uint8_t *)PUBLISH_MESSAGE,
  .content_length       = strlen(PUBLISH_MESSAGE),
};

sl_mqtt_client_last_will_message_t last_will_message = {
  .is_retained         = IS_LAST_WILL_RETAINED,
  .will_qos_level      = QOS_OF_LAST_WILL,
  .will_topic          = (uint8_t *)LAST_WILL_TOPIC,
  .will_topic_length   = strlen(LAST_WILL_TOPIC),
  .will_message        = (uint8_t *)LAST_WILL_MESSAGE,
  .will_message_length = strlen(LAST_WILL_MESSAGE),
};
/**
 * Maps CoreMQTT errors to AzureIoTMQTT errors.
 **/
static AzureIoTMQTTResult_t prvTranslateToAzureIoTMQTTResult(MQTTStatus_t xResult)
{
  AzureIoTMQTTResult_t xReturn;

  switch (xResult) {
    case MQTTSuccess:
      xReturn = eAzureIoTMQTTSuccess;
      break;

    case MQTTBadParameter:
      xReturn = eAzureIoTMQTTBadParameter;
      break;

    case MQTTNoMemory:
      xReturn = eAzureIoTMQTTNoMemory;
      break;

    case MQTTSendFailed:
      xReturn = eAzureIoTMQTTSendFailed;
      break;

    case MQTTRecvFailed:
      xReturn = eAzureIoTMQTTRecvFailed;
      break;

    case MQTTBadResponse:
      xReturn = eAzureIoTMQTTBadResponse;
      break;

    case MQTTServerRefused:
      xReturn = eAzureIoTMQTTServerRefused;
      break;

    case MQTTNoDataAvailable:
      xReturn = eAzureIoTMQTTNoDataAvailable;
      break;

    case MQTTIllegalState:
      xReturn = eAzureIoTMQTTIllegalState;
      break;

    case MQTTStateCollision:
      xReturn = eAzureIoTMQTTStateCollision;
      break;

    case MQTTKeepAliveTimeout:
      xReturn = eAzureIoTMQTTKeepAliveTimeout;
      break;

    default:
      xReturn = eAzureIoTMQTTFailed;
      break;
  }

  return xReturn;
}

AzureIoTMQTTResult_t AzureIoTMQTT_Init(AzureIoTMQTTHandle_t xContext,
                                       const AzureIoTTransportInterface_t *pxTransportInterface,
                                       AzureIoTMQTTGetCurrentTimeFunc_t xGetTimeFunction,
                                       AzureIoTMQTTEventCallback_t xUserCallback,
                                       uint8_t *pucNetworkBuffer,
                                       size_t xNetworkBufferLength)
{
  // MQTTFixedBuffer_t xBuffer = { pucNetworkBuffer, xNetworkBufferLength };
  // MQTTStatus_t xResult;

  // /* Check memory equivalence, but ordering is not guaranteed */
  // assert(sizeof(AzureIoTMQTTConnectInfo_t) == sizeof(MQTTConnectInfo_t));
  // assert(sizeof(AzureIoTMQTTSubscribeInfo_t) == sizeof(MQTTSubscribeInfo_t));
  // assert(sizeof(AzureIoTMQTTPacketInfo_t) == sizeof(MQTTPacketInfo_t));
  // assert(sizeof(AzureIoTMQTTPublishInfo_t) == sizeof(MQTTPublishInfo_t));
  // assert(sizeof(AzureIoTMQTTResult_t) == sizeof(MQTTStatus_t));
  // assert(sizeof(AzureIoTTransportInterface_t) == sizeof(TransportInterface_t));

  // xResult = MQTT_Init(xContext,
  //                     (const TransportInterface_t *)pxTransportInterface,
  //                     (MQTTGetCurrentTimeFunc_t)xGetTimeFunction,
  //                     (MQTTEventCallback_t)xUserCallback,
  //                     &xBuffer);
    ( void ) xContext;
    ( void ) pxTransportInterface;
    ( void ) xGetTimeFunction;
    ( void ) pucNetworkBuffer;
    ( void ) xNetworkBufferLength;

  sl_status_t status = sl_mqtt_client_init((sl_mqtt_client_t *)xContext, (sl_mqtt_client_event_handler_t) xUserCallback);

  return prvTranslateToAzureIoTMQTTResult(status);
}

AzureIoTMQTTResult_t AzureIoTMQTT_Connect(AzureIoTMQTTHandle_t xContext,
                                          const AzureIoTMQTTConnectInfo_t *pxConnectInfo,
                                          const AzureIoTMQTTPublishInfo_t *pxWillInfo,
                                          uint32_t ulMilliseconds,
                                          bool *pxSessionPresent)
{
  // MQTTStatus_t xResult;

  // xResult = MQTT_Connect(xContext,
  //                        (const MQTTConnectInfo_t *)pxConnectInfo,
  //                        (const MQTTPublishInfo_t *)pxWillInfo,
  //                        ulMilliseconds,
  //                        pxSessionPresent);

    ( void ) xContext;
    ( void ) pxConnectInfo;
    ( void ) pxWillInfo;
    ( void ) ulMilliseconds;
    ( void ) pxSessionPresent;

// last_will_message.is_retained = pxWillInfo->xRetain;
// last_will_message.will_message = pxWillInfo->pvPayload;
// last_will_message.will_message_length = pxWillInfo->xPayloadLength  ;
// last_will_message.will_topic = pxWillInfo->pcTopicName ;
// last_will_message.will_topic_length = pxWillInfo->usTopicNameLength;
// last_will_message.will_qos_level = pxWillInfo->xQOS;

sl_status_t status;
  status = sl_net_inet_addr(MQTT_BROKER_IP, &mqtt_broker_configuration.ip.ip.v4.value);
  if (status != SL_STATUS_OK) {
    printf("Failed to convert IP address \r\n");

    //mqtt_client_cleanup();
    return status;
  }
 status = sl_mqtt_client_connect((sl_mqtt_client_t *)xContext,
                                   (const sl_mqtt_broker_t *)&mqtt_broker_configuration,
                                   (const sl_mqtt_client_last_will_message_t *)&last_will_message,
                                   (const sl_mqtt_client_configuration_t *)&mqtt_client_configuration,
                                   (uint32_t)ulMilliseconds);
  return prvTranslateToAzureIoTMQTTResult(status);
}

void print_char_buffer(char *buffer, uint32_t buffer_length)
{
  for (uint32_t index = 0; index < buffer_length; index++) {
    printf("%c", buffer[index]);
  }

  printf("\r\n");
}

void mqtt_client_message_handler(void *client, sl_mqtt_client_message_t *message, void *context)
{
  (void)context;
  sl_status_t status;
  printf("Message Received on Topic: ");

  print_char_buffer((char *)message->topic, message->topic_length);
  print_char_buffer((char *)message->content, message->content_length);

  // Unsubscribing to already subscribed topic.
  status = sl_mqtt_client_unsubscribe(client,
                                      (uint8_t *)TOPIC_TO_BE_SUBSCRIBED,
                                      strlen(TOPIC_TO_BE_SUBSCRIBED),
                                      0,
                                      TOPIC_TO_BE_SUBSCRIBED);
  if (status != SL_STATUS_IN_PROGRESS) {
    printf("Failed to unsubscribe : 0x%lx\r\n", status);

    //mqtt_client_cleanup();
    return;
  }
}

AzureIoTMQTTResult_t AzureIoTMQTT_Subscribe(AzureIoTMQTTHandle_t xContext,
                                            const AzureIoTMQTTSubscribeInfo_t *pxSubscriptionList,
                                            size_t xSubscriptionCount,
                                            uint16_t usPacketId)
{
  // MQTTStatus_t xResult;

  // xResult = MQTT_Subscribe(xContext, (const MQTTSubscribeInfo_t *)pxSubscriptionList, xSubscriptionCount, usPacketId);
    ( void ) xContext;
    ( void ) pxSubscriptionList;
    ( void ) xSubscriptionCount;
    ( void ) usPacketId;

    sl_status_t status = sl_mqtt_client_subscribe((sl_mqtt_client_t *)xContext,
                                     (const uint8_t *)pxSubscriptionList->pcTopicFilter,
                                     (uint16_t) pxSubscriptionList->usTopicFilterLength,
                                     (sl_mqtt_qos_t) pxSubscriptionList->xQoS,
                                     (uint32_t) 0, // timeout,
                                     (sl_mqtt_client_message_received_t) mqtt_client_message_handler,
                                     (void *)pxSubscriptionList->pcTopicFilter);

  return prvTranslateToAzureIoTMQTTResult(MQTTSuccess);
}

AzureIoTMQTTResult_t AzureIoTMQTT_Publish(AzureIoTMQTTHandle_t xContext,
                                          const AzureIoTMQTTPublishInfo_t *pxPublishInfo,
                                          uint16_t usPacketId)
{
  // MQTTStatus_t xResult;

  // xResult = MQTT_Publish(xContext, (const MQTTPublishInfo_t *)pxPublishInfo, usPacketId);

    ( void ) xContext;
    ( void ) pxPublishInfo;
    ( void ) usPacketId;
    // message_to_be_published.is_retained = pxPublishInfo->xRetain;
    // message_to_be_published.is_duplicate_message = pxPublishInfo->xDup;
    // message_to_be_published.qos_level = pxPublishInfo->xQOS;
    // message_to_be_published.content_length = pxPublishInfo->xPayloadLength;
    // message_to_be_published.content = pxPublishInfo->pvPayload;
    // message_to_be_published.topic = pxPublishInfo->pcTopicName;
    // message_to_be_published.topic_length = pxPublishInfo->usTopicNameLength;

   sl_status_t status = sl_mqtt_client_publish((sl_mqtt_client_t *)xContext,
                                   (const sl_mqtt_client_message_t *)&message_to_be_published,
                                   0,  //(uint32_t) timeout,
                                   (void *)&message_to_be_published);


  return prvTranslateToAzureIoTMQTTResult(MQTTSuccess);
}

AzureIoTMQTTResult_t AzureIoTMQTT_Ping(AzureIoTMQTTHandle_t xContext)
{
  // MQTTStatus_t xResult;

  // xResult = MQTT_Ping(xContext);
    ( void ) xContext;
  return prvTranslateToAzureIoTMQTTResult(MQTTSuccess);
}

AzureIoTMQTTResult_t AzureIoTMQTT_Unsubscribe(AzureIoTMQTTHandle_t xContext,
                                              const AzureIoTMQTTSubscribeInfo_t *pxSubscriptionList,
                                              size_t xSubscriptionCount,
                                              uint16_t usPacketId)
{
  // MQTTStatus_t xResult;

  // xResult = MQTT_Unsubscribe(xContext, (const MQTTSubscribeInfo_t *)pxSubscriptionList, xSubscriptionCount, usPacketId);
    ( void ) xContext;
    ( void ) pxSubscriptionList;
    ( void ) xSubscriptionCount;
    ( void ) usPacketId;
  
   sl_status_t status = sl_mqtt_client_subscribe((sl_mqtt_client_t *)xContext,
                                     (const uint8_t *)pxSubscriptionList->pcTopicFilter,
                                     (uint16_t) pxSubscriptionList->usTopicFilterLength,
                                     (sl_mqtt_qos_t) pxSubscriptionList->xQoS,
                                     (uint32_t) 0, // timeout,
                                     (sl_mqtt_client_message_received_t) mqtt_client_message_handler,
                                     (void *)pxSubscriptionList->pcTopicFilter);
  return prvTranslateToAzureIoTMQTTResult(MQTTSuccess);
}

AzureIoTMQTTResult_t AzureIoTMQTT_Disconnect(AzureIoTMQTTHandle_t xContext)
{
  // MQTTStatus_t xResult;

  // xResult = MQTT_Disconnect(xContext);
    ( void ) xContext;
  return prvTranslateToAzureIoTMQTTResult(MQTTSuccess);
}

AzureIoTMQTTResult_t AzureIoTMQTT_ProcessLoop(AzureIoTMQTTHandle_t xContext, uint32_t ulMilliseconds)
{
  // MQTTStatus_t xResult;

  // xResult = MQTT_ProcessLoop(xContext, ulMilliseconds);
   ( void ) xContext;
    ( void ) ulMilliseconds;
  return prvTranslateToAzureIoTMQTTResult(MQTTSuccess);
}

uint16_t AzureIoTMQTT_GetPacketId(AzureIoTMQTTHandle_t xContext)
{
    ( void ) xContext;
  return MQTT_GetPacketId(MQTTSuccess);
}

AzureIoTMQTTResult_t AzureIoTMQTT_GetSubAckStatusCodes(const AzureIoTMQTTPacketInfo_t *pxSubackPacket,
                                                       uint8_t **ppucPayloadStart,
                                                       size_t *pxPayloadSize)
{
  // MQTTStatus_t xResult;

  // xResult = MQTT_GetSubAckStatusCodes((const MQTTPacketInfo_t *)pxSubackPacket, ppucPayloadStart, pxPayloadSize);
  ( void ) pxSubackPacket;
  ( void ) ppucPayloadStart;
  ( void ) pxPayloadSize;
  return prvTranslateToAzureIoTMQTTResult(MQTTSuccess);
}
