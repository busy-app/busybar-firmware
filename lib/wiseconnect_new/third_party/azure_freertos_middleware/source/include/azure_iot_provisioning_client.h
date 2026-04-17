/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot_provisioning_client.h
 *
 * @brief The middleware IoT Provisioning Client used to connect a device to Azure IoT Device Provisioning.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef AZURE_IOT_PROVISIONING_CLIENT_H
#define AZURE_IOT_PROVISIONING_CLIENT_H

#include "FreeRTOS.h"

#include "azure_iot.h"

#include "azure_iot_result.h"
#include "azure_iot_mqtt_port.h"
#include "azure_iot_transport_interface.h"

/* Azure SDK for Embedded C includes */
#include "azure/iot/az_iot_common.h"
#include "azure/iot/az_iot_provisioning_client.h"
#include "azure/core/_az_cfg_prefix.h"

/**
 * @brief The maximum size of the response buffer.
 */
#define azureiotprovisioningRESPONSE_MAX    ( azureiotconfigTOPIC_MAX + azureiotconfigPROVISIONING_REQUEST_PAYLOAD_MAX )

#define azureiotprovisioningNO_WAIT         ( 0 )                       /**< @brief Do not wait on the function call */
#define azureiotprovisioningWAIT_FOREVER    ( ( uint32_t ) 0xFFFFFFFF ) /**< @brief Wait as long as it takes to complete the operation (success or failure) */

/**
 * @brief The options for the Azure IoT Device Provisioning client.
 */
typedef struct AzureIoTProvisioningClientOptions
{
    const uint8_t * pucUserAgent; /**< The user agent to use for this device. */
    uint32_t ulUserAgentLength;   /**< The length of the user agent. */
} AzureIoTProvisioningClientOptions_t;

/**
 * @brief The Azure IoT Device Provisioning client
 */
typedef struct AzureIoTProvisioningClient
{
    struct
    {
        AzureIoTMQTT_t xMQTTContext;

        const uint8_t * pucRegistrationID;
        uint32_t ulRegistrationIDLength;
        const uint8_t * pucEndpoint;
        uint32_t ulEndpointLength;
        const uint8_t * pucIDScope;
        uint32_t ulIDScopeLength;
        const uint8_t * pucSymmetricKey;
        uint32_t ulSymmetricKeyLength;
        const uint8_t * pucRegistrationPayload;
        uint32_t ulRegistrationPayloadLength;

        uint32_t ( * pxTokenRefresh )( struct AzureIoTProvisioningClient * pxAzureIoTProvisioningClient,
                                       uint64_t ullExpiryTimeSecs,
                                       const uint8_t * ucKey,
                                       uint32_t ulKeyLen,
                                       uint8_t * pucSASBuffer,
                                       uint32_t ulSasBufferLen,
                                       uint32_t * pulSaSLength );
        AzureIoTGetHMACFunc_t xHMACFunction;
        AzureIoTGetCurrentTimeFunc_t xGetTimeFunction;

        az_iot_provisioning_client xProvisioningClientCore;

        uint32_t ulWorkflowState;
        uint32_t ulLastOperationResult;
        uint64_t ullRetryAfter;

        uint8_t * pucScratchBuffer;
        uint32_t ulScratchBufferLength;
        uint8_t ucProvisioningLastResponse[ azureiotprovisioningRESPONSE_MAX ];
        size_t xLastResponsePayloadLength;
        uint16_t usLastResponseTopicLength;
        az_iot_provisioning_client_register_response xRegisterResponse;
    } _internal; /**< @brief Internal to the SDK */
} AzureIoTProvisioningClient_t;

/**
 * @brief Initialize the Azure IoT Provisioning Options with default values.
 *
 * @param[out] pxProvisioningClientOptions The #AzureIoTProvisioningClientOptions_t instance to set with default values.
 * @return An #AzureIoTResult_t with the result of the operation.
 */
AzureIoTResult_t AzureIoTProvisioningClient_OptionsInit( AzureIoTProvisioningClientOptions_t * pxProvisioningClientOptions );

/**
 * @brief Initialize the Azure IoT Provisioning Client.
 *
 * @param[out] pxAzureProvClient The #AzureIoTProvisioningClient_t * to use for this call.
 * @param[in] pucEndpoint The IoT Provisioning Hostname.
 * @param[in] ulEndpointLength The length of the IoT Provisioning Hostname.
 * @param[in] pucIDScope The ID scope to use for provisioning.
 * @param[in] ulIDScopeLength The length of the ID scope.
 * @param[in] pucRegistrationID The registration ID to use for provisioning.
 * @param[in] ulRegistrationIDLength The length of the registration ID.
 * @param[in] pxProvisioningClientOptions The #AzureIoTProvisioningClientOptions_t for the IoT Provisioning client instance.
 * @param[in] pucBuffer The buffer to use for MQTT messages.
 * @param[in] ulBufferLength bufferLength The length of the \p pucBuffer.
 * @param[in] xGetTimeFunction A function pointer to a function which gives the current epoch time.
 * @param[in] pxTransportInterface The transport interface to use for the MQTT library.
 * @return AzureIoTResult_t
 */
AzureIoTResult_t AzureIoTProvisioningClient_Init( AzureIoTProvisioningClient_t * pxAzureProvClient,
                                                  const uint8_t * pucEndpoint,
                                                  uint32_t ulEndpointLength,
                                                  const uint8_t * pucIDScope,
                                                  uint32_t ulIDScopeLength,
                                                  const uint8_t * pucRegistrationID,
                                                  uint32_t ulRegistrationIDLength,
                                                  AzureIoTProvisioningClientOptions_t * pxProvisioningClientOptions,
                                                  uint8_t * pucBuffer,
                                                  uint32_t ulBufferLength,
                                                  AzureIoTGetCurrentTimeFunc_t xGetTimeFunction,
                                                  const AzureIoTTransportInterface_t * pxTransportInterface );

/**
 * @brief Deinitialize the Azure IoT Provisioning Client.
 *
 * @param pxAzureProvClient The #AzureIoTProvisioningClient_t * to use for this call.
 */
void AzureIoTProvisioningClient_Deinit( AzureIoTProvisioningClient_t * pxAzureProvClient );

/**
 * @brief Set the symmetric key to use for authentication.
 *
 * @note For X509 cert based authentication, application configures its transport with client side certificate.
 *
 * @param[in] pxAzureProvClient The #AzureIoTProvisioningClient_t * to use for this call.
 * @param[in] pucSymmetricKey The symmetric key to use for the connection.
 * @param[in] ulSymmetricKeyLength The length of the \p pucSymmetricKey.
 * @param[in] xHmacFunction The #AzureIoTGetHMACFunc_t function pointer to a function which computes the HMAC256 over a set of bytes.
 * @return An #AzureIoTResult_t with the result of the operation.
 */
AzureIoTResult_t AzureIoTProvisioningClient_SetSymmetricKey( AzureIoTProvisioningClient_t * pxAzureProvClient,
                                                             const uint8_t * pucSymmetricKey,
                                                             uint32_t ulSymmetricKeyLength,
                                                             AzureIoTGetHMACFunc_t xHmacFunction );

/**
 * @brief Begin the provisioning process.
 *
 * The initial call to this function will issue the request to the service to provision this device.
 * If this function is called with the same pxAzureProvClient after a previous call to it returned
 * "eAzureIoTErrorPending", then the function will simply poll to see if the registration
 * has succeeded or failed. After a successful result, AzureIoTProvisioningClient_GetDeviceAndHub()
 * can be called to get the IoT Hub and device ID.
 *
 * @param[in] pxAzureProvClient The #AzureIoTProvisioningClient_t * to use for this call.
 * @param[in] ulTimeoutMilliseconds Timeout in milliseconds to wait for registration to finish.
 * @return An #AzureIoTResult_t with the result of the operation.
 *      - eAzureIoTErrorPending registration is still in progess.
 *      - eAzureIoTErrorOutOfMemory registration failed because the device is out of memory.
 *      - eAzureIoTErrorServerError registration failed because of a server error.
 *      - eAzureIoTErrorFailed registration failed because of an internal error.
 *      - eAzureIoTSuccess registration is completed.
 */
AzureIoTResult_t AzureIoTProvisioningClient_Register( AzureIoTProvisioningClient_t * pxAzureProvClient,
                                                      uint32_t ulTimeoutMilliseconds );

/**
 * @brief After a registration has been completed, get the IoT Hub hostname and device ID.
 *
 * @param[in] pxAzureProvClient The #AzureIoTProvisioningClient_t * to use for this call.
 * @param[out] pucHubHostname The pointer to a buffer which will be populated with the IoT Hub hostname.
 * @param[out] pulHostnameLength The pointer to the `uint32_t` which will be populated with the length of the hostname.
 * @param[out] pucDeviceID The pointer to a buffer which will be populated with the device ID.
 * @param[out] pulDeviceIDLength The pointer to the uint32_t which will be populated with the length of the device ID.
 * @return An #AzureIoTResult_t with the result of the operation.
 */
AzureIoTResult_t AzureIoTProvisioningClient_GetDeviceAndHub( AzureIoTProvisioningClient_t * pxAzureProvClient,
                                                             uint8_t * pucHubHostname,
                                                             uint32_t * pulHostnameLength,
                                                             uint8_t * pucDeviceID,
                                                             uint32_t * pulDeviceIDLength );

/**
 * @brief Get extended code for Provisioning failure.
 *
 * @note Extended code is 6 digit error code last returned via the Provisioning service, when registration is done.
 *
 * @param[in] pxAzureProvClient The #AzureIoTProvisioningClient_t * to use for this call.
 * @param[out] pulExtendedErrorCode The pointer to the uint32_t which will be populated with the extended code.
 * @return An #AzureIoTResult_t with the result of the operation.
 */
AzureIoTResult_t AzureIoTProvisioningClient_GetExtendedCode( AzureIoTProvisioningClient_t * pxAzureProvClient,
                                                             uint32_t * pulExtendedErrorCode );

/**
 * @brief Set registration payload
 * @details This routine sets registration payload, which is JSON object.
 *
 * @param[in] pxAzureProvClient The #AzureIoTProvisioningClient_t * to use for this call.
 * @param[in] pucPayload A pointer to registration payload.
 * @param[in] ulPayloadLength Length of `payload`. Does not include the `NULL` terminator.
 * @return An #AzureIoTResult_t with the result of the operation.
 */
AzureIoTResult_t AzureIoTProvisioningClient_SetRegistrationPayload( AzureIoTProvisioningClient_t * pxAzureProvClient,
                                                                    const uint8_t * pucPayload,
                                                                    uint32_t ulPayloadLength );

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZURE_IOT_PROVISIONING_CLIENT_H */
