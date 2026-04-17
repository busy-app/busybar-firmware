/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot_result.h
 *
 * @brief Azure IoT FreeRTOS middleware result values.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef AZURE_IOT_RESULT_H
#define AZURE_IOT_RESULT_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief The results used by the middleware.
 */
typedef enum AzureIoTResult
{
    /* === Core: Success results ==== */
    /*/ Success. */
    eAzureIoTSuccess = 0,

    /* === Core: Error results === */
    eAzureIoTErrorFailed,                /**< There was a failure. */
    eAzureIoTErrorInvalidArgument,       /**< Input argument does not comply with the expected range of values. */
    eAzureIoTErrorPending,               /**< The status of the operation is pending. */
    eAzureIoTErrorOutOfMemory,           /**< The system is out of memory. */
    eAzureIoTErrorInitFailed,            /**< The initialization failed. */
    eAzureIoTErrorSubackWaitTimeout,     /**< There was timeout while waiting for SUBACK. */
    eAzureIoTErrorTopicNotSubscribed,    /**< Topic not subscribed. */
    eAzureIoTErrorPublishFailed,         /**< Failed to publish. */
    eAzureIoTErrorSubscribeFailed,       /**< Failed to subscribe. */
    eAzureIoTErrorUnsubscribeFailed,     /**< Failed to unsubscribe. */
    eAzureIoTErrorServerError,           /**< There was a server error in registration. */
    eAzureIoTErrorItemNotFound,          /**< The item was not found. */
    eAzureIoTErrorTopicNoMatch,          /**< The received message was not for the currently processed feature. */
    eAzureIoTErrorTokenGenerationFailed, /**< There was a failure. */
    eAzureIoTErrorEndOfProperties,       /**< End of properties when iterating with AzureIoTHubClientProperties_GetNextComponentProperty(). */
    eAzureIoTErrorInvalidResponse,       /**< Invalid response from server. */
    eAzureIoTErrorUnexpectedChar,        /**< Input can't be successfully parsed. */

    /* === JSON: Error results === */
    eAzureIoTErrorJSONInvalidState,    /**< The kind of the token being read is not compatible with the expected type of the value. */
    eAzureIoTErrorJSONNestingOverflow, /**< The JSON depth is too large. */
    eAzureIoTErrorJSONReaderDone       /**< No more JSON text left to process. */
} AzureIoTResult_t;

#endif /* AZURE_IOT_RESULT_H */
