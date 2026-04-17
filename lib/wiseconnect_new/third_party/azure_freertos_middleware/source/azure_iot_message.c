/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot_message.c
 *
 * @brief Azure IoT Middleware message API implementation.
 *
 */

#include "azure_iot_message.h"

/*-----------------------------------------------------------*/

AzureIoTResult_t AzureIoTMessage_PropertiesInit( AzureIoTMessageProperties_t * pxMessageProperties,
                                                 uint8_t * pucBuffer,
                                                 uint32_t ulAlreadyWrittenLength,
                                                 uint32_t ulBufferLength )
{
    az_span xPropertyBufferSpan = az_span_create( pucBuffer, ( int32_t ) ulBufferLength );
    az_result xResult;

    if( ( pxMessageProperties == NULL ) ||
        ( pucBuffer == NULL ) )
    {
        AZLogError( ( "AzureIoTMessage_PropertiesInit failed: Invalid argument" ) );
        return eAzureIoTErrorInvalidArgument;
    }

    xResult = az_iot_message_properties_init( &pxMessageProperties->_internal.xProperties,
                                              xPropertyBufferSpan, ( int32_t ) ulAlreadyWrittenLength );

    if( az_result_failed( xResult ) )
    {
        return eAzureIoTErrorFailed;
    }

    return eAzureIoTSuccess;
}
/*-----------------------------------------------------------*/

AzureIoTResult_t AzureIoTMessage_PropertiesAppend( AzureIoTMessageProperties_t * pxMessageProperties,
                                                   const uint8_t * pucName,
                                                   uint32_t ulNameLength,
                                                   const uint8_t * pucValue,
                                                   uint32_t ulValueLength )
{
    az_span xNameSpan = az_span_create( ( uint8_t * ) pucName, ( int32_t ) ulNameLength );
    az_span xValueSpan = az_span_create( ( uint8_t * ) pucValue, ( int32_t ) ulValueLength );
    az_result xResult;

    if( ( pxMessageProperties == NULL ) ||
        ( pucName == NULL ) || ( ulNameLength == 0 ) ||
        ( pucValue == NULL ) || ( ulValueLength == 0 ) )
    {
        AZLogError( ( "AzureIoTMessage_PropertiesAppend failed: Invalid argument" ) );
        return eAzureIoTErrorInvalidArgument;
    }

    xResult = az_iot_message_properties_append( &pxMessageProperties->_internal.xProperties,
                                                xNameSpan, xValueSpan );

    if( az_result_failed( xResult ) )
    {
        return eAzureIoTErrorFailed;
    }

    return eAzureIoTSuccess;
}
/*-----------------------------------------------------------*/

AzureIoTResult_t AzureIoTMessage_PropertiesFind( AzureIoTMessageProperties_t * pxMessageProperties,
                                                 const uint8_t * pucName,
                                                 uint32_t ulNameLength,
                                                 const uint8_t ** ppucOutValue,
                                                 uint32_t * pulOutValueLength )
{
    az_span xNameSpan = az_span_create( ( uint8_t * ) pucName, ( int32_t ) ulNameLength );
    az_span xOutValueSpan;
    az_result xResult;

    if( ( pxMessageProperties == NULL ) ||
        ( pucName == NULL ) || ( ulNameLength == 0 ) ||
        ( ppucOutValue == NULL ) || ( pulOutValueLength == NULL ) )
    {
        AZLogError( ( "AzureIoTMessage_PropertiesFind failed: Invalid argument" ) );
        return eAzureIoTErrorInvalidArgument;
    }

    xResult = az_iot_message_properties_find( &pxMessageProperties->_internal.xProperties,
                                              xNameSpan, &xOutValueSpan );

    if( az_result_failed( xResult ) )
    {
        return eAzureIoTErrorItemNotFound;
    }

    *ppucOutValue = az_span_ptr( xOutValueSpan );
    *pulOutValueLength = ( uint32_t ) az_span_size( xOutValueSpan );

    return eAzureIoTSuccess;
}
/*-----------------------------------------------------------*/
