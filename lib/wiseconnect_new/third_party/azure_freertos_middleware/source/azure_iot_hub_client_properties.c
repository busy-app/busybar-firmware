/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot_hub_client_properties.c
 * @brief Implementation of the Azure IoT Hub Client properties.
 */

#include "azure_iot_hub_client_properties.h"
#include "azure_iot_private.h"

AzureIoTResult_t AzureIoTHubClientProperties_BuilderBeginComponent( AzureIoTHubClient_t * pxAzureIoTHubClient,
                                                                    AzureIoTJSONWriter_t * pxJSONWriter,
                                                                    const uint8_t * pucComponentName,
                                                                    uint16_t usComponentNameLength )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;
    az_span xComponentSpan;

    if( ( pxAzureIoTHubClient == NULL ) ||
        ( pxJSONWriter == NULL ) ||
        ( pucComponentName == NULL ) ||
        ( usComponentNameLength == 0 ) )
    {
        AZLogError( ( "AzureIoTHubClientProperties_BuilderBeginComponent failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        xComponentSpan = az_span_create( ( uint8_t * ) pucComponentName, usComponentNameLength );

        if( az_result_failed(
                xCoreResult = az_iot_hub_client_properties_writer_begin_component( &pxAzureIoTHubClient->_internal.xAzureIoTHubClientCore,
                                                                                   &pxJSONWriter->_internal.xCoreWriter, xComponentSpan ) ) )
        {
            AZLogError( ( "Could not begin component: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTHubClientProperties_BuilderEndComponent( AzureIoTHubClient_t * pxAzureIoTHubClient,
                                                                  AzureIoTJSONWriter_t * pxJSONWriter )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;

    if( ( pxAzureIoTHubClient == NULL ) || ( pxJSONWriter == NULL ) )
    {
        AZLogError( ( "AzureIoTHubClientProperties_BuilderEndComponent failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        if( az_result_failed(
                xCoreResult = az_iot_hub_client_properties_writer_end_component( &pxAzureIoTHubClient->_internal.xAzureIoTHubClientCore,
                                                                                 &pxJSONWriter->_internal.xCoreWriter ) ) )
        {
            AZLogError( ( "Could not end component: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTHubClientProperties_BuilderBeginResponseStatus( AzureIoTHubClient_t * pxAzureIoTHubClient,
                                                                         AzureIoTJSONWriter_t * pxJSONWriter,
                                                                         const uint8_t * pucPropertyName,
                                                                         uint16_t usPropertyNameLength,
                                                                         int32_t lAckCode,
                                                                         int32_t lAckVersion,
                                                                         const uint8_t * pucAckDescription,
                                                                         uint16_t usAckDescriptionLength )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;
    az_span xPropertyName;
    az_span xAckDescription;

    if( ( pxAzureIoTHubClient == NULL ) ||
        ( pxJSONWriter == NULL ) ||
        ( pucPropertyName == NULL ) ||
        ( usPropertyNameLength == 0 ) )
    {
        AZLogError( ( "AzureIoTHubClientProperties_BuilderBeginResponseStatus failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        xPropertyName = az_span_create( ( uint8_t * ) pucPropertyName, usPropertyNameLength );
        xAckDescription = az_span_create( ( uint8_t * ) pucAckDescription, usAckDescriptionLength );

        if( az_result_failed(
                xCoreResult = az_iot_hub_client_properties_writer_begin_response_status( &pxAzureIoTHubClient->_internal.xAzureIoTHubClientCore,
                                                                                         &pxJSONWriter->_internal.xCoreWriter,
                                                                                         xPropertyName,
                                                                                         lAckCode, lAckVersion,
                                                                                         xAckDescription ) ) )
        {
            AZLogError( ( "Could not begin response: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTHubClientProperties_BuilderEndResponseStatus( AzureIoTHubClient_t * pxAzureIoTHubClient,
                                                                       AzureIoTJSONWriter_t * pxJSONWriter )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;

    if( ( pxAzureIoTHubClient == NULL ) || ( pxJSONWriter == NULL ) )
    {
        AZLogError( ( "AzureIoTHubClientProperties_BuilderEndResponseStatus failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        if( az_result_failed(
                xCoreResult = az_iot_hub_client_properties_writer_end_response_status( &pxAzureIoTHubClient->_internal.xAzureIoTHubClientCore,
                                                                                       &pxJSONWriter->_internal.xCoreWriter ) ) )
        {
            AZLogError( ( "Could not end response: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTHubClientProperties_GetPropertiesVersion( AzureIoTHubClient_t * pxAzureIoTHubClient,
                                                                   AzureIoTJSONReader_t * pxJSONReader,
                                                                   AzureIoTHubMessageType_t xResponseType,
                                                                   uint32_t * pulVersion )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;
    az_iot_hub_client_properties_message_type xCoreMessageType;

    if( ( pxAzureIoTHubClient == NULL ) ||
        ( pxJSONReader == NULL ) ||
        ( ( xResponseType != eAzureIoTHubPropertiesRequestedMessage ) &&
          ( xResponseType != eAzureIoTHubPropertiesWritablePropertyMessage ) ) ||
        ( pulVersion == NULL ) )
    {
        AZLogError( ( "AzureIoTHubClientProperties_GetPropertiesVersion failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        xCoreMessageType = xResponseType == eAzureIoTHubPropertiesRequestedMessage ?
                           AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_GET_RESPONSE : AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_WRITABLE_UPDATED;

        if( az_result_failed(
                xCoreResult = az_iot_hub_client_properties_get_properties_version( &pxAzureIoTHubClient->_internal.xAzureIoTHubClientCore,
                                                                                   &pxJSONReader->_internal.xCoreReader, xCoreMessageType, ( int32_t * ) pulVersion ) ) )
        {
            AZLogError( ( "Could not get property version: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTHubClientProperties_GetNextComponentProperty( AzureIoTHubClient_t * pxAzureIoTHubClient,
                                                                       AzureIoTJSONReader_t * pxJSONReader,
                                                                       AzureIoTHubMessageType_t xResponseType,
                                                                       AzureIoTHubClientPropertyType_t xPropertyType,
                                                                       const uint8_t ** ppucComponentName,
                                                                       uint32_t * pulComponentNameLength )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;
    az_span xComponentSpan;
    az_iot_hub_client_properties_message_type xCoreMessageType;

    if( ( pxAzureIoTHubClient == NULL ) || ( pxJSONReader == NULL ) ||
        ( ( xResponseType != eAzureIoTHubPropertiesRequestedMessage ) &&
          ( xResponseType != eAzureIoTHubPropertiesWritablePropertyMessage ) ) ||
        ( ppucComponentName == NULL ) || ( pulComponentNameLength == NULL ) )
    {
        AZLogError( ( "AzureIoTHubClientProperties_GetNextComponentProperty failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        xComponentSpan = az_span_create( ( uint8_t * ) *ppucComponentName, ( int32_t ) *pulComponentNameLength );
        xCoreMessageType = xResponseType == eAzureIoTHubPropertiesRequestedMessage ?
                           AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_GET_RESPONSE : AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_WRITABLE_UPDATED;

        if( az_result_failed(
                xCoreResult = az_iot_hub_client_properties_get_next_component_property( &pxAzureIoTHubClient->_internal.xAzureIoTHubClientCore,
                                                                                        &pxJSONReader->_internal.xCoreReader,
                                                                                        xCoreMessageType,
                                                                                        ( az_iot_hub_client_property_type ) xPropertyType,
                                                                                        &xComponentSpan ) ) )
        {
            if( xCoreResult == AZ_ERROR_IOT_END_OF_PROPERTIES )
            {
                xResult = eAzureIoTErrorEndOfProperties;
            }
            else
            {
                AZLogError( ( "Could not get next component property: core error=0x%08x", xCoreResult ) );
                xResult = AzureIoT_TranslateCoreError( xCoreResult );
            }
        }
        else
        {
            *ppucComponentName = az_span_ptr( xComponentSpan );
            *pulComponentNameLength = ( uint16_t ) az_span_size( xComponentSpan );

            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}
