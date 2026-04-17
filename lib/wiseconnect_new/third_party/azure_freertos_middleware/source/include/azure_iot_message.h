/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot_message.h
 *
 * @brief Azure IoT FreeRTOS middleware message APIs and structs
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 *
 */

#ifndef AZURE_IOT_MESSAGE_H
#define AZURE_IOT_MESSAGE_H

#include "azure_iot.h"
#include "azure_iot_result.h"

/* Azure SDK for Embedded C includes */
#include "azure/iot/az_iot_hub_client_properties.h"
#include "azure/core/_az_cfg_prefix.h"

/**
 * @brief The bag of properties associated with a message.
 *
 */
typedef struct AzureIoTMessageProperties
{
    struct
    {
        az_iot_message_properties xProperties;
    } _internal; /**< @brief Internal to the SDK */
} AzureIoTMessageProperties_t;

/**
 * @brief Initialize the message properties.
 *
 * @note The properties init API will not encode properties. In order to support
 *       the following characters, they must be percent-encoded (RFC3986) as follows:
 *         - `/` : `%2F`
 *         - `%` : `%25`
 *         - `#` : `%23`
 *         - `&` : `%26`
 *       Only these characters would have to be encoded. If you would like to avoid the need to
 *       encode the names/values, avoid using these characters in names and values.
 *
 * @param[out] pxMessageProperties The #AzureIoTMessageProperties_t* to use for the operation.
 * @param[out] pucBuffer The pointer to the buffer.
 * @param[in] ulAlreadyWrittenLength The length of the properties already written (if applicable).
 * @param[in] ulBufferLength The length of \p pucBuffer.
 * @return An #AzureIoTResult_t with the result of the operation.
 */
AzureIoTResult_t AzureIoTMessage_PropertiesInit( AzureIoTMessageProperties_t * pxMessageProperties,
                                                 uint8_t * pucBuffer,
                                                 uint32_t ulAlreadyWrittenLength,
                                                 uint32_t ulBufferLength );

/**
 * @brief Append a property name and value to a message.
 *
 * @note The properties init API will not encode properties. In order to support
 *       the following characters, they must be percent-encoded (RFC3986) as follows:
 *         - `/` : `%2F`
 *         - `%` : `%25`
 *         - `#` : `%23`
 *         - `&` : `%26`
 *       Only these characters would have to be encoded. If you would like to avoid the need to
 *       encode the names/values, avoid using these characters in names and values.
 *
 * @param[in] pxMessageProperties The #AzureIoTMessageProperties_t* to use for the operation.
 * @param[in] pucName The name of the property to append.
 * @param[in] ulNameLength The length of \p pucName.
 * @param[in] pucValue The value of the property to append.
 * @param[in] ulValueLength The length of \p pucValue.
 * @return An #AzureIoTResult_t with the result of the operation.
 */
AzureIoTResult_t AzureIoTMessage_PropertiesAppend( AzureIoTMessageProperties_t * pxMessageProperties,
                                                   const uint8_t * pucName,
                                                   uint32_t ulNameLength,
                                                   const uint8_t * pucValue,
                                                   uint32_t ulValueLength );

/**
 * @brief Find a property in the message property bag.
 *
 * @param[in] pxMessageProperties The #AzureIoTMessageProperties_t* to use for the operation.
 * @param[in] pucName The name of the property to find.
 * @param[in] ulNameLength Length of the property name.
 * @param[out] ppucOutValue The output pointer to the property value.
 * @param[out] pulOutValueLength The length of \p ppucOutValue.
 * @return An #AzureIoTResult_t with the result of the operation.
 */
AzureIoTResult_t AzureIoTMessage_PropertiesFind( AzureIoTMessageProperties_t * pxMessageProperties,
                                                 const uint8_t * pucName,
                                                 uint32_t ulNameLength,
                                                 const uint8_t ** ppucOutValue,
                                                 uint32_t * pulOutValueLength );

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZURE_IOT_MESSAGE_H */
