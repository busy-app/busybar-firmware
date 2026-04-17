/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot_mqtt.h
 *
 * @brief Azure IoT MQTT is the MQTT interface that AzureIoT middleware depends upon.
 *
 * @note This interface is private and subjected to change. Currently, there is only
 *       one implementation for this interface, which uses coreMQTT as underlying MQTT stack.
 */

#ifndef AZURE_IOT_MQTT_H
#define AZURE_IOT_MQTT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "azure_iot_mqtt_port.h"
#include "azure_iot_transport_interface.h"


#define azureiotmqttPACKET_TYPE_CONNECT        ( ( uint8_t ) 0x10U )    /**< @brief CONNECT (client-to-server). */
#define azureiotmqttPACKET_TYPE_CONNACK        ( ( uint8_t ) 0x20U )    /**< @brief CONNACK (server-to-client). */
#define azureiotmqttPACKET_TYPE_PUBLISH        ( ( uint8_t ) 0x30U )    /**< @brief PUBLISH (bidirectional). */
#define azureiotmqttPACKET_TYPE_PUBACK         ( ( uint8_t ) 0x40U )    /**< @brief PUBACK (bidirectional). */
#define azureiotmqttPACKET_TYPE_PUBREC         ( ( uint8_t ) 0x50U )    /**< @brief PUBREC (bidirectional). */
#define azureiotmqttPACKET_TYPE_PUBREL         ( ( uint8_t ) 0x62U )    /**< @brief PUBREL (bidirectional). */
#define azureiotmqttPACKET_TYPE_PUBCOMP        ( ( uint8_t ) 0x70U )    /**< @brief PUBCOMP (bidirectional). */
#define azureiotmqttPACKET_TYPE_SUBSCRIBE      ( ( uint8_t ) 0x82U )    /**< @brief SUBSCRIBE (client-to-server). */
#define azureiotmqttPACKET_TYPE_SUBACK         ( ( uint8_t ) 0x90U )    /**< @brief SUBACK (server-to-client). */
#define azureiotmqttPACKET_TYPE_UNSUBSCRIBE    ( ( uint8_t ) 0xA2U )    /**< @brief UNSUBSCRIBE (client-to-server). */
#define azureiotmqttPACKET_TYPE_UNSUBACK       ( ( uint8_t ) 0xB0U )    /**< @brief UNSUBACK (server-to-client). */
#define azureiotmqttPACKET_TYPE_PINGREQ        ( ( uint8_t ) 0xC0U )    /**< @brief PINGREQ (client-to-server). */
#define azureiotmqttPACKET_TYPE_PINGRESP       ( ( uint8_t ) 0xD0U )    /**< @brief PINGRESP (server-to-client). */
#define azureiotmqttPACKET_TYPE_DISCONNECT     ( ( uint8_t ) 0xE0U )    /**< @brief DISCONNECT (client-to-server). */

#define azureiotmqttGET_PACKET_TYPE( ucType )    ( ( ucType ) & 0xF0U ) /**< @brief Get the packet type according to the MQTT spec */

/**
 * @brief Quality of service values.
 */
typedef enum AzureIoTMQTTQoS
{
    eAzureIoTMQTTQoS0 = 0, /** Delivery at most once. */
    eAzureIoTMQTTQoS1 = 1, /** Delivery at least once. */
    eAzureIoTMQTTQoS2 = 2  /** Delivery exactly once. */
} AzureIoTMQTTQoS_t;

/**
 * @brief MQTT suback ack status states.
 */
typedef enum AzureIoTMQTTSubAckStatus
{
    eMQTTSubAckSuccessQos0 = 0x00, /** Success with a maximum delivery at QoS 0. */
    eMQTTSubAckSuccessQos1 = 0x01, /** Success with a maximum delivery at QoS 1. */
    eMQTTSubAckSuccessQos2 = 0x02, /** Success with a maximum delivery at QoS 2. */
    eMQTTSubAckFailure = 0x80      /** Failure. */
} AzureIoTMQTTSubAckStatus_t;

/**
 * @brief Result values used for Azure IoT MQTT functions.
 */
typedef enum AzureIoTMQTTResult
{
    eAzureIoTMQTTSuccess = 0,      /** Function completed successfully. */
    eAzureIoTMQTTBadParameter,     /** At least one parameter was invalid. */
    eAzureIoTMQTTNoMemory,         /** A provided buffer was too small. */
    eAzureIoTMQTTSendFailed,       /** The transport send function failed. */
    eAzureIoTMQTTRecvFailed,       /** The transport receive function failed. */
    eAzureIoTMQTTBadResponse,      /** An invalid packet was received from the server. */
    eAzureIoTMQTTServerRefused,    /** The server refused a CONNECT or SUBSCRIBE. */
    eAzureIoTMQTTNoDataAvailable,  /** No data available from the transport interface. */
    eAzureIoTMQTTIllegalState,     /** An illegal state in the state record. */
    eAzureIoTMQTTStateCollision,   /** A collision with an existing state record entry. */
    eAzureIoTMQTTKeepAliveTimeout, /** Timeout while waiting for PINGRESP. */
    eAzureIoTMQTTFailed            /** Function failed with Unknown Error. */
} AzureIoTMQTTResult_t;

/**
 * @brief Connection info for the MQTT client.
 */
typedef struct AzureIoTMQTTConnectInfo
{
    /**
     * @brief Whether to establish a new, clean session or resume a previous session.
     */
    bool xCleanSession;

    /**
     * @brief MQTT keep alive period, in seconds.
     */
    uint16_t usKeepAliveSeconds;

    /**
     * @brief MQTT client identifier. Must be unique per client.
     */
    const uint8_t * pcClientIdentifier;

    /**
     * @brief Length of the client identifier.
     */
    uint16_t usClientIdentifierLength;

    /**
     * @brief MQTT user name. Set to NULL if not used.
     */
    const uint8_t * pcUserName;

    /**
     * @brief Length of MQTT user name. Set to 0 if not used.
     *        or set pcUserName to NULL.
     */
    uint16_t usUserNameLength;

    /**
     * @brief MQTT password. Set to NULL if not used.
     */
    const uint8_t * pcPassword;

    /**
     * @brief Length of MQTT password. Set to 0 if not used
     *        or set pcPassword to NULL.
     */
    uint16_t usPasswordLength;
} AzureIoTMQTTConnectInfo_t;

/**
 * @brief Subscription info for the MQTT client.
 */
typedef struct AzureIoTMQTTSubscribeInfo
{
    /**
     * @brief Quality of Service for subscription.
     */
    AzureIoTMQTTQoS_t xQoS;

    /**
     * @brief Topic filter to subscribe to.
     */
    const uint8_t * pcTopicFilter;

    /**
     * @brief Length of subscription topic filter.
     */
    uint16_t usTopicFilterLength;
} AzureIoTMQTTSubscribeInfo_t;

/**
 * @brief Publish info for the MQTT client.
 */
typedef struct AzureIoTMQTTPublishInfo
{
    /**
     * @brief Quality of Service for message.
     */
    AzureIoTMQTTQoS_t xQOS;

    /**
     * @brief Whether this is a retained message.
     */
    bool xRetain;

    /**
     * @brief Whether this is a duplicate publish message.
     */
    bool xDup;

    /**
     * @brief Topic name on which the message is published.
     */
    const uint8_t * pcTopicName;

    /**
     * @brief Length of topic name.
     */
    uint16_t usTopicNameLength;

    /**
     * @brief Message payload.
     */
    const void * pvPayload;

    /**
     * @brief Message payload length.
     */
    size_t xPayloadLength;
} AzureIoTMQTTPublishInfo_t;

/**
 * @brief MQTT packet deserialized info for the MQTT client.
 */
typedef struct AzureIoTMQTTDeserializedInfo
{
    uint16_t usPacketIdentifier;                 /**< @brief Packet ID of deserialized packet. */
    AzureIoTMQTTPublishInfo_t * pxPublishInfo;   /**< @brief Pointer to deserialized publish info. */
    AzureIoTMQTTResult_t xDeserializationResult; /**< @brief Return code of deserialization. */
} AzureIoTMQTTDeserializedInfo_t;

/**
 * @brief MQTT packet info for the MQTT client.
 */
typedef struct AzureIoTMQTTPacketInfo
{
    /**
     * @brief Type of incoming MQTT packet.
     */
    uint8_t ucType;

    /**
     * @brief Remaining serialized data in the MQTT packet.
     */
    uint8_t * pucRemainingData;

    /**
     * @brief Length of remaining serialized data.
     */
    size_t xRemainingLength;
} AzureIoTMQTTPacketInfo_t;

/**
 * @brief Typedef of the MQTT client which is defined by the MQTT port.
 */
typedef AzureIoTMQTT_t * AzureIoTMQTTHandle_t;

/**
 * @brief The time function to be used for MQTT functionality.
 *
 * @note Must return the time since Unix epoch.
 */
typedef uint32_t ( * AzureIoTMQTTGetCurrentTimeFunc_t )( void );

/**
 * @brief The callback function which will be invoked on receipt of an MQTT message.
 */
typedef void ( * AzureIoTMQTTEventCallback_t )( AzureIoTMQTTHandle_t pContext,
                                                struct AzureIoTMQTTPacketInfo * pxPacketInfo,
                                                struct AzureIoTMQTTDeserializedInfo * pxDeserializedInfo );


/**
 * @brief Initialize an AzureIoTMQTT context.
 *
 * @note This function is for integration of the Azure IoT middleware and
 *       the underlying MQTT stack and should in general not be invoked by
 *       applications directly.
 *
 * @param[in] xContext The AzureIoTMQTT context to initialize.
 * @param[in] pxTransportInterface The transport interface to use with the context.
 * @param[in] xGetTimeFunction The time utility function to use with the context.
 * @param[in] xUserCallback The user callback to use with the context to
 * notify about incoming packet events.
 * @param[in] pucNetworkBuffer Network buffer provided for the context.
 * @param[in] xNetworkBufferLength Length of network buffer.
 *
 * @return An #AzureIoTMQTTResult_t with the result of the operation.
 */
AzureIoTMQTTResult_t AzureIoTMQTT_Init( AzureIoTMQTTHandle_t xContext,
                                        const AzureIoTTransportInterface_t * pxTransportInterface,
                                        AzureIoTMQTTGetCurrentTimeFunc_t xGetTimeFunction,
                                        AzureIoTMQTTEventCallback_t xUserCallback,
                                        uint8_t * pucNetworkBuffer,
                                        size_t xNetworkBufferLength );


/**
 * @brief Establish an MQTT session.
 *
 * @note Last Will and Testament is not used with Azure IoT Hub.
 *
 * This function will send MQTT CONNECT packet and receive a CONNACK packet. The
 * send and receive from the network is done through the transport interface.
 *
 * @param[in] xContext Initialized AzureIoTMQTT context.
 * @param[in] pxConnectInfo MQTT CONNECT packet information.
 * @param[in] pxWillInfo Last Will and Testament. Pass NULL if Last Will and Testament is not used.
 * @param[in] ulMilliseconds Maximum time in milliseconds to wait for a CONNACK packet.
 * @param[out] pxSessionPresent Whether a previous session was present.
 * Only relevant if not establishing a clean session.
 *
 * @return An #AzureIoTMQTTResult_t with the result of the operation.
 */
AzureIoTMQTTResult_t AzureIoTMQTT_Connect( AzureIoTMQTTHandle_t xContext,
                                           const AzureIoTMQTTConnectInfo_t * pxConnectInfo,
                                           const AzureIoTMQTTPublishInfo_t * pxWillInfo,
                                           uint32_t ulMilliseconds,
                                           bool * pxSessionPresent );

/**
 * @brief Sends MQTT SUBSCRIBE for the given list of topic filters to
 * the broker.
 *
 * @param[in] xContext Initialized AzureIoTMQTT context.
 * @param[in] pxSubscriptionList List of MQTT subscription infos.
 * @param[in] xSubscriptionCount The number of elements in pxSubscriptionList.
 * @param[in] usPacketId Packet ID ( generated by #AzureIoTMQTT_GetPacketId ).
 *
 * @return An #AzureIoTMQTTResult_t with the result of the operation.
 */
AzureIoTMQTTResult_t AzureIoTMQTT_Subscribe( AzureIoTMQTTHandle_t xContext,
                                             const AzureIoTMQTTSubscribeInfo_t * pxSubscriptionList,
                                             size_t xSubscriptionCount,
                                             uint16_t usPacketId );

/**
 * @brief Publishes a message to the given topic name.
 *
 * @param[in] xContext Initialized AzureIoTMQTT context.
 * @param[in] pxPublishInfo MQTT PUBLISH packet parameters.
 * @param[in] usPacketId packet ID ( generated by #AzureIoTMQTT_GetPacketId ).
 *
 * @return An #AzureIoTMQTTResult_t with the result of the operation.
 */
AzureIoTMQTTResult_t AzureIoTMQTT_Publish( AzureIoTMQTTHandle_t xContext,
                                           const AzureIoTMQTTPublishInfo_t * pxPublishInfo,
                                           uint16_t usPacketId );

/**
 * @brief Sends a MQTT PINGREQ to broker.
 *
 * @param[in] xContext Initialized AzureIoTMQTT context.
 *
 * @return An #AzureIoTMQTTResult_t with the result of the operation.
 */
AzureIoTMQTTResult_t AzureIoTMQTT_Ping( AzureIoTMQTTHandle_t xContext );

/**
 * @brief Sends MQTT UNSUBSCRIBE for the given list of topic filters to
 * the broker.
 *
 * @param[in] xContext Initialized AzureIoTMQTT context.
 * @param[in] pxSubscriptionList List of MQTT subscription infos.
 * @param[in] xSubscriptionCount The number of elements in pxSubscriptionList.
 * @param[in] usPacketId packet ID ( generated by #AzureIoTMQTT_GetPacketId ).
 *
 * @return An #AzureIoTMQTTResult_t with the result of the operation.
 */
AzureIoTMQTTResult_t AzureIoTMQTT_Unsubscribe( AzureIoTMQTTHandle_t xContext,
                                               const AzureIoTMQTTSubscribeInfo_t * pxSubscriptionList,
                                               size_t xSubscriptionCount,
                                               uint16_t usPacketId );

/**
 * @brief Disconnect a MQTT session.
 *
 * @param[in] xContext Initialized AzureIoTMQTT context.
 *
 * @return An #AzureIoTMQTTResult_t with the result of the operation.
 */
AzureIoTMQTTResult_t AzureIoTMQTT_Disconnect( AzureIoTMQTTHandle_t xContext );


/**
 * @brief Loop to receive packets from the transport interface. Handles keep
 * alive.
 *
 * @param[in] xContext Initialized AzureIoTMQTT context.
 * @param[in] ulMilliseconds Minimum time in milliseconds that the process loop will
 * run, unless an error occurs.
 *
 * @return An #AzureIoTMQTTResult_t with the result of the operation.
 */
AzureIoTMQTTResult_t AzureIoTMQTT_ProcessLoop( AzureIoTMQTTHandle_t xContext,
                                               uint32_t ulMilliseconds );

/**
 * @brief Get a packet ID.
 *
 * @param[in] xContext Initialized AzureIoTMQTT context.
 *
 * @return A non-zero number.
 */
uint16_t AzureIoTMQTT_GetPacketId( AzureIoTMQTTHandle_t xContext );


/**
 * @brief Parses the payload of a MQTT SUBACK packet that contains status codes
 * corresponding to topic filter subscription requests from the original
 * subscribe packet.
 *
 * Each return code in the SUBACK packet corresponds to a topic filter in the
 * SUBSCRIBE Packet being acknowledged.
 * The status codes can be one of the following:
 *  - 0x00 - Success - Maximum QoS 0
 *  - 0x01 - Success - Maximum QoS 1
 *  - 0x02 - Success - Maximum QoS 2
 *  - 0x80 - Failure
 * Refer to #AzureIoTMQTTSubAckStatus_t for the status codes.
 *
 * @param[in] pxSubackPacket The SUBACK packet whose payload is to be parsed.
 * @param[out] ppucPayloadStart This is populated with the starting address
 * of the payload (or return codes for topic filters) in the SUBACK packet.
 * @param[out] pxPayloadSize This is populated with the size of the payload
 * in the SUBACK packet. It represents the number of topic filters whose
 * SUBACK status is present in the packet.
 *
 * @return An #AzureIoTMQTTResult_t with the result of the operation.
 */
AzureIoTMQTTResult_t AzureIoTMQTT_GetSubAckStatusCodes( const AzureIoTMQTTPacketInfo_t * pxSubackPacket,
                                                        uint8_t ** ppucPayloadStart,
                                                        size_t * pxPayloadSize );

#endif /* AZURE_IOT_MQTT_H */
