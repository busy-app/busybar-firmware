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
  MQTTFixedBuffer_t xBuffer = { pucNetworkBuffer, xNetworkBufferLength };
  MQTTStatus_t xResult;

  /* Check memory equivalence, but ordering is not guaranteed */
  assert(sizeof(AzureIoTMQTTConnectInfo_t) == sizeof(MQTTConnectInfo_t));
  assert(sizeof(AzureIoTMQTTSubscribeInfo_t) == sizeof(MQTTSubscribeInfo_t));
  assert(sizeof(AzureIoTMQTTPacketInfo_t) == sizeof(MQTTPacketInfo_t));
  assert(sizeof(AzureIoTMQTTPublishInfo_t) == sizeof(MQTTPublishInfo_t));
  assert(sizeof(AzureIoTMQTTResult_t) == sizeof(MQTTStatus_t));
  assert(sizeof(AzureIoTTransportInterface_t) == sizeof(TransportInterface_t));

  xResult = MQTT_Init(xContext,
                      (const TransportInterface_t *)pxTransportInterface,
                      (MQTTGetCurrentTimeFunc_t)xGetTimeFunction,
                      (MQTTEventCallback_t)xUserCallback,
                      &xBuffer);

  return prvTranslateToAzureIoTMQTTResult(xResult);
}

AzureIoTMQTTResult_t AzureIoTMQTT_Connect(AzureIoTMQTTHandle_t xContext,
                                          const AzureIoTMQTTConnectInfo_t *pxConnectInfo,
                                          const AzureIoTMQTTPublishInfo_t *pxWillInfo,
                                          uint32_t ulMilliseconds,
                                          bool *pxSessionPresent)
{
  MQTTStatus_t xResult;

  xResult = MQTT_Connect(xContext,
                         (const MQTTConnectInfo_t *)pxConnectInfo,
                         (const MQTTPublishInfo_t *)pxWillInfo,
                         ulMilliseconds,
                         pxSessionPresent);

  return prvTranslateToAzureIoTMQTTResult(xResult);
}

AzureIoTMQTTResult_t AzureIoTMQTT_Subscribe(AzureIoTMQTTHandle_t xContext,
                                            const AzureIoTMQTTSubscribeInfo_t *pxSubscriptionList,
                                            size_t xSubscriptionCount,
                                            uint16_t usPacketId)
{
  MQTTStatus_t xResult;

  xResult = MQTT_Subscribe(xContext, (const MQTTSubscribeInfo_t *)pxSubscriptionList, xSubscriptionCount, usPacketId);

  return prvTranslateToAzureIoTMQTTResult(xResult);
}

AzureIoTMQTTResult_t AzureIoTMQTT_Publish(AzureIoTMQTTHandle_t xContext,
                                          const AzureIoTMQTTPublishInfo_t *pxPublishInfo,
                                          uint16_t usPacketId)
{
  MQTTStatus_t xResult;

  xResult = MQTT_Publish(xContext, (const MQTTPublishInfo_t *)pxPublishInfo, usPacketId);

  return prvTranslateToAzureIoTMQTTResult(xResult);
}

AzureIoTMQTTResult_t AzureIoTMQTT_Ping(AzureIoTMQTTHandle_t xContext)
{
  MQTTStatus_t xResult;

  xResult = MQTT_Ping(xContext);

  return prvTranslateToAzureIoTMQTTResult(xResult);
}

AzureIoTMQTTResult_t AzureIoTMQTT_Unsubscribe(AzureIoTMQTTHandle_t xContext,
                                              const AzureIoTMQTTSubscribeInfo_t *pxSubscriptionList,
                                              size_t xSubscriptionCount,
                                              uint16_t usPacketId)
{
  MQTTStatus_t xResult;

  xResult = MQTT_Unsubscribe(xContext, (const MQTTSubscribeInfo_t *)pxSubscriptionList, xSubscriptionCount, usPacketId);

  return prvTranslateToAzureIoTMQTTResult(xResult);
}

AzureIoTMQTTResult_t AzureIoTMQTT_Disconnect(AzureIoTMQTTHandle_t xContext)
{
  MQTTStatus_t xResult;

  xResult = MQTT_Disconnect(xContext);

  return prvTranslateToAzureIoTMQTTResult(xResult);
}

AzureIoTMQTTResult_t AzureIoTMQTT_ProcessLoop(AzureIoTMQTTHandle_t xContext, uint32_t ulMilliseconds)
{
  MQTTStatus_t xResult;

  xResult = MQTT_ProcessLoop(xContext, ulMilliseconds);

  return prvTranslateToAzureIoTMQTTResult(xResult);
}

uint16_t AzureIoTMQTT_GetPacketId(AzureIoTMQTTHandle_t xContext)
{
  return MQTT_GetPacketId(xContext);
}

AzureIoTMQTTResult_t AzureIoTMQTT_GetSubAckStatusCodes(const AzureIoTMQTTPacketInfo_t *pxSubackPacket,
                                                       uint8_t **ppucPayloadStart,
                                                       size_t *pxPayloadSize)
{
  MQTTStatus_t xResult;

  xResult = MQTT_GetSubAckStatusCodes((const MQTTPacketInfo_t *)pxSubackPacket, ppucPayloadStart, pxPayloadSize);

  return prvTranslateToAzureIoTMQTTResult(xResult);
}
