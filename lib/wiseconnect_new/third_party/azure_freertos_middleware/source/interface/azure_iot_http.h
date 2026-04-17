/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot_http.h
 *
 * @brief The port file for http APIs.
 *
 * Used in ADU.
 *
 */
#ifndef AZURE_IOT_HTTP_H
#define AZURE_IOT_HTTP_H

#include <stdint.h>

#include "azure_iot_http_port.h"
#include "azure_iot_transport_interface.h"

#define azureiothttpHttpRangeRequestEndOfFile    -1

typedef AzureIoTHTTP_t * AzureIoTHTTPHandle_t;

typedef enum AzureIoTHTTPResult
{
    eAzureIoTHTTPSuccess = 0,
    eAzureIoTHTTPInvalidParameter,
    eAzureIoTHTTPNetworkError,
    eAzureIoTHTTPPartialResponse,
    eAzureIoTHTTPNoResponse,
    eAzureIoTHTTPInsufficientMemory,
    eAzureIoTHTTPSecurityAlertResponseHeadersSizeLimitExceeded,
    eAzureIoTHTTPSecurityAlertExtraneousResponseData,
    eAzureIoTHTTPSecurityAlertInvalidChunkHeader,
    eAzureIoTHTTPSecurityAlertInvalidProtocolVersion,
    eAzureIoTHTTPSecurityAlertInvalidStatusCode,
    eAzureIoTHTTPSecurityAlertInvalidCharacter,
    eAzureIoTHTTPSecurityAlertInvalidContentLength,
    eAzureIoTHTTPParserInternalError,
    eAzureIoTHTTPHeaderNotFound,
    eAzureIoTHTTPInvalidResponse,
    eAzureIoTHTTPError
} AzureIoTHTTPResult_t;

AzureIoTHTTPResult_t AzureIoTHTTP_Init( AzureIoTHTTPHandle_t xHTTPHandle,
                                        AzureIoTTransportInterface_t * pxHTTPTransport,
                                        const char * pucURL,
                                        uint32_t ulURLLength,
                                        const char * pucPath,
                                        uint32_t ulPathLength,
                                        char * pucHeaderBuffer,
                                        uint32_t ulHeaderBufferLength );

AzureIoTHTTPResult_t AzureIoTHTTP_Request( AzureIoTHTTPHandle_t xHTTPHandle,
                                           int32_t lRangeStart,
                                           int32_t lRangeEnd,
                                           char * pucDataBuffer,
                                           uint32_t ulDataBufferLength,
                                           char ** ppucOutData,
                                           uint32_t * pulOutDataLength );

AzureIoTHTTPResult_t AzureIoTHTTP_RequestSizeInit( AzureIoTHTTPHandle_t xHTTPHandle,
                                                   AzureIoTTransportInterface_t * pxHTTPTransport,
                                                   const char * pucURL,
                                                   uint32_t ulURLLength,
                                                   const char * pucPath,
                                                   uint32_t ulPathLength,
                                                   char * pucHeaderBuffer,
                                                   uint32_t ulHeaderBufferLength );

int32_t AzureIoTHTTP_RequestSize( AzureIoTHTTPHandle_t xHTTPHandle,
                                  char * pucDataBuffer,
                                  uint32_t ulDataBufferLength );

AzureIoTHTTPResult_t AzureIoTHTTP_Deinit( AzureIoTHTTPHandle_t xHTTPHandle );

#endif /* AZURE_IOT_HTTP_H */
