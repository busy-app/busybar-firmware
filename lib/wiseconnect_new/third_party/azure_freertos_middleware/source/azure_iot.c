/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot.c
 *
 * @brief Common Azure IoT code for FreeRTOS middleware.
 *
 */

#include "azure_iot.h"

#include <string.h>

#include "azure_iot_private.h"
#include "azure_iot_result.h"

/* Azure SDK for Embedded C includes */
#include "azure/az_iot.h"
#include "azure/core/az_base64.h"

/* Using SHA256 hash - needs 32 bytes */
#define azureiotBASE64_HASH_BUFFER_SIZE    ( 33 )

/*-----------------------------------------------------------*/

/**
 *
 * Set the log listener in the embedded SDK.
 *
 * */
static void prvAzureIoTLogListener( az_log_classification xClassification,
                                    az_span xMessage )
{
    ( void ) xClassification;
    /* In case logs are stripped out, suppress unused parameter error. */
    ( void ) xMessage;

    AZLogInfo( ( "%.*s", az_span_size( xMessage ), az_span_ptr( xMessage ) ) );
}
/*-----------------------------------------------------------*/

AzureIoTResult_t AzureIoT_TranslateCoreError( az_result xCoreError )
{
    AzureIoTResult_t xResult;

    switch( xCoreError )
    {
        /* Core errors */
        case AZ_OK:
            xResult = eAzureIoTSuccess;
            break;

        case AZ_ERROR_IOT_TOPIC_NO_MATCH:
            xResult = eAzureIoTErrorTopicNoMatch;
            break;

        case AZ_ERROR_IOT_END_OF_PROPERTIES:
            xResult = eAzureIoTErrorEndOfProperties;
            break;

        case AZ_ERROR_ARG:
            xResult = eAzureIoTErrorInvalidArgument;
            break;

        case AZ_ERROR_ITEM_NOT_FOUND:
            xResult = eAzureIoTErrorItemNotFound;
            break;

        case AZ_ERROR_UNEXPECTED_CHAR:
            xResult = eAzureIoTErrorUnexpectedChar;
            break;

        /* JSON errors */
        case AZ_ERROR_JSON_INVALID_STATE:
            xResult = eAzureIoTErrorJSONInvalidState;
            break;

        case AZ_ERROR_JSON_NESTING_OVERFLOW:
            xResult = eAzureIoTErrorJSONNestingOverflow;
            break;

        case AZ_ERROR_JSON_READER_DONE:
            xResult = eAzureIoTErrorJSONReaderDone;
            break;

        /* Default */
        default:
            xResult = eAzureIoTErrorFailed;
    }

    return xResult;
}
/*-----------------------------------------------------------*/

AzureIoTResult_t AzureIoT_Init()
{
    #ifdef AZLogInfo
        az_log_set_message_callback( prvAzureIoTLogListener );
    #else
        ( void ) prvAzureIoTLogListener;
    #endif

    return eAzureIoTSuccess;
}
/*-----------------------------------------------------------*/

void AzureIoT_Deinit()
{
}
/*-----------------------------------------------------------*/

AzureIoTResult_t AzureIoT_Base64HMACCalculate( AzureIoTGetHMACFunc_t xAzureIoTHMACFunction,
                                               const uint8_t * pucKey,
                                               uint32_t ulKeySize,
                                               const uint8_t * pucMessage,
                                               uint32_t ulMessageSize,
                                               uint8_t * pucBuffer,
                                               uint32_t ulBufferLength,
                                               uint8_t * pucOutput,
                                               uint32_t ulOutputSize,
                                               uint32_t * pulOutputLength )
{
    az_result xCoreResult;
    uint8_t * pucHashBuf;
    uint8_t * pucDecodedKeyBuf = pucBuffer;
    uint32_t ulHashBufSize = azureiotBASE64_HASH_BUFFER_SIZE;
    int32_t lDecodedKeyLength;
    int32_t lEncodedLength;
    az_span xEncodedKeySpan;
    az_span xOutputDecodedKeySpan;
    az_span xHashSpan;
    az_span xOutputEncodedHashSpan;

    if( ( xAzureIoTHMACFunction == NULL ) ||
        ( pucKey == NULL ) || ( ulKeySize == 0 ) ||
        ( pucMessage == NULL ) || ( ulMessageSize == 0 ) ||
        ( pucBuffer == NULL ) || ( ulBufferLength == 0 ) ||
        ( pucOutput == NULL ) || ( pulOutputLength == NULL ) )
    {
        AZLogError( ( "AzureIoT_Base64HMACCalculate failed: Invalid argument" ) );
        return eAzureIoTErrorInvalidArgument;
    }

    xEncodedKeySpan = az_span_create( ( uint8_t * ) pucKey, ( int32_t ) ulKeySize );
    xOutputDecodedKeySpan = az_span_create( ( uint8_t * ) pucDecodedKeyBuf, ( int32_t ) ulBufferLength );

    if( az_result_failed( xCoreResult = az_base64_decode( xOutputDecodedKeySpan, xEncodedKeySpan, &lDecodedKeyLength ) ) )
    {
        AZLogError( ( "az_base64_decode failed: core error=0x%08x", xCoreResult ) );
        return eAzureIoTErrorFailed;
    }

    /* Decoded key is less than total decoded buffer size */
    ulBufferLength -= ( uint32_t ) lDecodedKeyLength;

    if( ulHashBufSize > ulBufferLength )
    {
        return eAzureIoTErrorOutOfMemory;
    }

    pucHashBuf = pucDecodedKeyBuf + lDecodedKeyLength;
    memset( pucHashBuf, 0, ulHashBufSize );

    if( xAzureIoTHMACFunction( pucDecodedKeyBuf, ( uint32_t ) lDecodedKeyLength,
                               pucMessage, ( uint32_t ) ulMessageSize,
                               pucHashBuf, ulHashBufSize, &ulHashBufSize ) )
    {
        return eAzureIoTErrorFailed;
    }

    xHashSpan = az_span_create( pucHashBuf, ( int32_t ) ulHashBufSize );
    xOutputEncodedHashSpan = az_span_create( pucOutput, ( int32_t ) ulOutputSize );

    if( az_result_failed( xCoreResult = az_base64_encode( xOutputEncodedHashSpan, xHashSpan, &lEncodedLength ) ) )
    {
        AZLogError( ( "az_base64_decode failed: core error=0x%08x", xCoreResult ) );
        return eAzureIoTErrorFailed;
    }

    *pulOutputLength = ( uint32_t ) lEncodedLength;

    return eAzureIoTSuccess;
}
/*-----------------------------------------------------------*/
