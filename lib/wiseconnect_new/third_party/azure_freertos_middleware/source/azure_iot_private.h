/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

#ifndef AZURE_IOT_PRIVATE_H
#define AZURE_IOT_PRIVATE_H

#include "azure_iot_result.h"

/* Azure SDK for Embedded C includes */
#include "azure/az_core.h"
#include "azure/core/_az_cfg_prefix.h"

/**
 * @brief Translate embedded errors to middleware errors
 *
 * @param[in] xCoreError The `az_result` to translate to a middleware error
 *
 * @result The #AzureIoTResult_t translated from \p xCoreError.
 */
AzureIoTResult_t AzureIoT_TranslateCoreError( az_result xCoreError );

/**
 * @brief As part of symmetric key authentication, HMAC256 a buffer of bytes and base64 encode the result.
 *
 * @note This is used within Azure IoT Hub and Device Provisioning APIs if a symmetric key is set.
 *
 * @param[in] xAzureIoTHMACFunction The #AzureIoTGetHMACFunc_t function to use for HMAC256 hashing.
 * @param[in] pucKey A pointer to the base64 encoded key.
 * @param[in] ulKeySize The length of the \p pucKey.
 * @param[in] pucMessage A pointer to the blob to be hashed.
 * @param[in] ulMessageSize The length of \p pucMessage.
 * @param[in] pucBuffer An intermediary buffer to put the base64 decoded key.
 * @param[in] ulBufferLength The length of \p pucBuffer.
 * @param[out] pucOutput The buffer into which the resulting HMAC256 hashed, base64 encoded message will
 * be placed.
 * @param[in] ulOutputSize Size of \p pucOutput.
 * @param[out] pulOutputLength The output length of \p pucOutput.
 * @return An #AzureIoTResult_t with the result of the operation.
 */
AzureIoTResult_t AzureIoT_Base64HMACCalculate( AzureIoTGetHMACFunc_t xAzureIoTHMACFunction,
                                               const uint8_t * pucKey,
                                               uint32_t ulKeySize,
                                               const uint8_t * pucMessage,
                                               uint32_t ulMessageSize,
                                               uint8_t * pucBuffer,
                                               uint32_t ulBufferLength,
                                               uint8_t * pucOutput,
                                               uint32_t ulOutputSize,
                                               uint32_t * pulOutputLength );

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZURE_IOT_PRIVATE_H */
