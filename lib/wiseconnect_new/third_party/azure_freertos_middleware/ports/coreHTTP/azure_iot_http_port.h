/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot_http_port.h
 * @brief Defines Azure IoT HTTP port based on coreHTTP
 *
 */

#ifndef AZURE_IOT_HTTP_PORT_H
#define AZURE_IOT_HTTP_PORT_H

#include <stdio.h>

#include "azure_iot_transport_interface.h"

#include "core_http_client.h"

typedef struct AzureIoTCoreHTTPContext
{
    HTTPRequestInfo_t xRequestInfo;
    HTTPRequestHeaders_t xRequestHeaders;
    HTTPResponse_t xResponse;
    AzureIoTTransportInterface_t * pxHTTPTransport;
} AzureIoTCoreHTTPContext_t;

/* Maps HTTPContext directly to AzureIoTHTTP */
typedef AzureIoTCoreHTTPContext_t AzureIoTHTTP_t;

#endif /* AZURE_IOT_HTTP_PORT_H */
