/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot_json_writer.c
 * @brief Implementation of the Azure IoT JSON writer.
 */

#include "azure_iot_json_writer.h"

#include <stdbool.h>
#include <stdint.h>

#include "azure_iot.h"
#include "azure_iot_private.h"

AzureIoTResult_t AzureIoTJSONWriter_Init( AzureIoTJSONWriter_t * pxWriter,
                                          uint8_t * pucBuffer,
                                          uint32_t ulBufferSize )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;
    az_span xJSONSpan;

    if( ( pxWriter == NULL ) || ( pucBuffer == NULL ) || ( ulBufferSize == 0 ) )
    {
        AZLogError( ( "AzureIoTJSONWriter_Init failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        xJSONSpan = az_span_create( ( uint8_t * ) pucBuffer, ( int32_t ) ulBufferSize );

        if( az_result_failed( xCoreResult = az_json_writer_init( &pxWriter->_internal.xCoreWriter, xJSONSpan, NULL ) ) )
        {
            AZLogError( ( "Could not initialize the JSON reader: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTJSONWriter_AppendPropertyWithInt32Value( AzureIoTJSONWriter_t * pxWriter,
                                                                  const uint8_t * pucPropertyName,
                                                                  uint32_t ulPropertyNameLength,
                                                                  int32_t lValue )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;
    az_span xPropertyNameSpan;

    if( ( pxWriter == NULL ) || ( pucPropertyName == NULL ) || ( ulPropertyNameLength == 0 ) )
    {
        AZLogError( ( "AzureIoTJSONWriter_AppendPropertyWithInt32Value failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        xPropertyNameSpan = az_span_create( ( uint8_t * ) pucPropertyName, ( int32_t ) ulPropertyNameLength );

        if( az_result_failed( xCoreResult = az_json_writer_append_property_name( &pxWriter->_internal.xCoreWriter, xPropertyNameSpan ) ) ||
            az_result_failed( xCoreResult = az_json_writer_append_int32( &pxWriter->_internal.xCoreWriter, lValue ) ) )
        {
            AZLogError( ( "Could not append property and int32: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTJSONWriter_AppendPropertyWithDoubleValue( AzureIoTJSONWriter_t * pxWriter,
                                                                   const uint8_t * pucPropertyName,
                                                                   uint32_t ulPropertyNameLength,
                                                                   double xValue,
                                                                   uint16_t usFractionalDigits )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;
    az_span xPropertyNameSpan;

    if( ( pxWriter == NULL ) || ( pucPropertyName == NULL ) || ( ulPropertyNameLength == 0 ) )
    {
        AZLogError( ( "AzureIoTJSONWriter_AppendPropertyWithDoubleValue failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        xPropertyNameSpan = az_span_create( ( uint8_t * ) pucPropertyName, ( int32_t ) ulPropertyNameLength );

        if( az_result_failed( xCoreResult = az_json_writer_append_property_name( &pxWriter->_internal.xCoreWriter, xPropertyNameSpan ) ) ||
            az_result_failed( xCoreResult = az_json_writer_append_double( &pxWriter->_internal.xCoreWriter, xValue, ( int32_t ) usFractionalDigits ) ) )
        {
            AZLogError( ( "Could not append property and double: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTJSONWriter_AppendPropertyWithBoolValue( AzureIoTJSONWriter_t * pxWriter,
                                                                 const uint8_t * pucPropertyName,
                                                                 uint32_t ulPropertyNameLength,
                                                                 bool xValue )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;
    az_span xPropertyNameSpan;

    if( ( pxWriter == NULL ) || ( pucPropertyName == NULL ) || ( ulPropertyNameLength == 0 ) )
    {
        AZLogError( ( "AzureIoTJSONWriter_AppendPropertyWithBoolValue failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        xPropertyNameSpan = az_span_create( ( uint8_t * ) pucPropertyName, ( int32_t ) ulPropertyNameLength );

        if( az_result_failed( xCoreResult = az_json_writer_append_property_name( &pxWriter->_internal.xCoreWriter, xPropertyNameSpan ) ) ||
            az_result_failed( xCoreResult = az_json_writer_append_bool( &pxWriter->_internal.xCoreWriter, xValue ) ) )
        {
            AZLogError( ( "Could not append property and bool: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTJSONWriter_AppendPropertyWithStringValue( AzureIoTJSONWriter_t * pxWriter,
                                                                   const uint8_t * pucPropertyName,
                                                                   uint32_t ulPropertyNameLength,
                                                                   const uint8_t * pucValue,
                                                                   uint32_t ulValueLen )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;
    az_span xPropertyNameSpan;
    az_span xValueSpan;

    if( ( pxWriter == NULL ) || ( pucPropertyName == NULL ) || ( ulPropertyNameLength == 0 ) ||
        ( pucValue == NULL ) || ( ulValueLen == 0 ) )
    {
        AZLogError( ( "AzureIoTJSONWriter_AppendPropertyWithStringValue failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        xPropertyNameSpan = az_span_create( ( uint8_t * ) pucPropertyName, ( int32_t ) ulPropertyNameLength );
        xValueSpan = az_span_create( ( uint8_t * ) pucValue, ( int32_t ) ulValueLen );

        if( az_result_failed( xCoreResult = az_json_writer_append_property_name( &pxWriter->_internal.xCoreWriter, xPropertyNameSpan ) ) ||
            az_result_failed( xCoreResult = az_json_writer_append_string( &pxWriter->_internal.xCoreWriter, xValueSpan ) ) )
        {
            AZLogError( ( "Could not append property and string: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

int32_t AzureIoTJSONWriter_GetBytesUsed( AzureIoTJSONWriter_t * pxWriter )
{
    if( pxWriter == NULL )
    {
        AZLogError( ( "AzureIoTJSONWriter_GetBytesUsed failed: invalid argument" ) );
        return -1;
    }

    return az_span_size( az_json_writer_get_bytes_used_in_destination( &pxWriter->_internal.xCoreWriter ) );
}

AzureIoTResult_t AzureIoTJSONWriter_AppendString( AzureIoTJSONWriter_t * pxWriter,
                                                  const uint8_t * pucValue,
                                                  uint32_t ulValueLen )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;
    az_span xValueSpan;

    if( ( pxWriter == NULL ) || ( pucValue == NULL ) || ( ulValueLen == 0 ) )
    {
        AZLogError( ( "AzureIoTJSONWriter_AppendString failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        xValueSpan = az_span_create( ( uint8_t * ) pucValue, ( int32_t ) ulValueLen );

        if( az_result_failed( xCoreResult = az_json_writer_append_string( &pxWriter->_internal.xCoreWriter, xValueSpan ) ) )
        {
            AZLogError( ( "Could not append string: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTJSONWriter_AppendJSONText( AzureIoTJSONWriter_t * pxWriter,
                                                    const uint8_t * pucJSON,
                                                    uint32_t ulJSONLen )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;
    az_span xJSONSpan;

    if( ( pxWriter == NULL ) || ( pucJSON == NULL ) || ( ulJSONLen == 0 ) )
    {
        AZLogError( ( "AzureIoTJSONWriter_AppendJSONText failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        xJSONSpan = az_span_create( ( uint8_t * ) pucJSON, ( int32_t ) ulJSONLen );

        if( az_result_failed( xCoreResult = az_json_writer_append_json_text( &pxWriter->_internal.xCoreWriter, xJSONSpan ) ) )
        {
            AZLogError( ( "Could not append JSON text: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTJSONWriter_AppendPropertyName( AzureIoTJSONWriter_t * pxWriter,
                                                        const uint8_t * pusValue,
                                                        uint32_t ulValueLen )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;
    az_span xPropertyNameSpan;

    if( ( pxWriter == NULL ) || ( pusValue == NULL ) || ( ulValueLen == 0 ) )
    {
        AZLogError( ( "AzureIoTJSONWriter_AppendPropertyName failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        xPropertyNameSpan = az_span_create( ( uint8_t * ) pusValue, ( int32_t ) ulValueLen );

        if( az_result_failed( xCoreResult = az_json_writer_append_property_name( &pxWriter->_internal.xCoreWriter, xPropertyNameSpan ) ) )
        {
            AZLogError( ( "Could not append property name: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTJSONWriter_AppendBool( AzureIoTJSONWriter_t * pxWriter,
                                                bool xValue )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;

    if( pxWriter == NULL )
    {
        AZLogError( ( "AzureIoTJSONWriter_AppendBool failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        if( az_result_failed( xCoreResult = az_json_writer_append_bool( &pxWriter->_internal.xCoreWriter, xValue ) ) )
        {
            AZLogError( ( "Could not append bool: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTJSONWriter_AppendInt32( AzureIoTJSONWriter_t * pxWriter,
                                                 int32_t lValue )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;

    if( pxWriter == NULL )
    {
        AZLogError( ( "AzureIoTJSONWriter_AppendInt32 failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        if( az_result_failed( xCoreResult = az_json_writer_append_int32( &pxWriter->_internal.xCoreWriter, lValue ) ) )
        {
            AZLogError( ( "Could not append int32: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTJSONWriter_AppendDouble( AzureIoTJSONWriter_t * pxWriter,
                                                  double xValue,
                                                  uint16_t usFractionalDigits )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;

    if( pxWriter == NULL )
    {
        AZLogError( ( "AzureIoTJSONWriter_AppendDouble failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        if( az_result_failed( xCoreResult = az_json_writer_append_double( &pxWriter->_internal.xCoreWriter, xValue, usFractionalDigits ) ) )
        {
            AZLogError( ( "Could not append double: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTJSONWriter_AppendNull( AzureIoTJSONWriter_t * pxWriter )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;

    if( pxWriter == NULL )
    {
        AZLogError( ( "AzureIoTJSONWriter_AppendNull failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        if( az_result_failed( xCoreResult = az_json_writer_append_null( &pxWriter->_internal.xCoreWriter ) ) )
        {
            AZLogError( ( "Could not append NULL: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTJSONWriter_AppendBeginObject( AzureIoTJSONWriter_t * pxWriter )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;

    if( pxWriter == NULL )
    {
        AZLogError( ( "AzureIoTJSONWriter_AppendBeginObject failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        if( az_result_failed( xCoreResult = az_json_writer_append_begin_object( &pxWriter->_internal.xCoreWriter ) ) )
        {
            AZLogError( ( "Could not append begin object: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTJSONWriter_AppendBeginArray( AzureIoTJSONWriter_t * pxWriter )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;

    if( pxWriter == NULL )
    {
        AZLogError( ( "AzureIoTJSONWriter_AppendBeginArray failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        if( az_result_failed( xCoreResult = az_json_writer_append_begin_array( &pxWriter->_internal.xCoreWriter ) ) )
        {
            AZLogError( ( "Could not append begin array: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTJSONWriter_AppendEndObject( AzureIoTJSONWriter_t * pxWriter )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;

    if( pxWriter == NULL )
    {
        AZLogError( ( "AzureIoTJSONWriter_AppendEndObject failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        if( az_result_failed( xCoreResult = az_json_writer_append_end_object( &pxWriter->_internal.xCoreWriter ) ) )
        {
            AZLogError( ( "Could not append end object: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}

AzureIoTResult_t AzureIoTJSONWriter_AppendEndArray( AzureIoTJSONWriter_t * pxWriter )
{
    AzureIoTResult_t xResult;
    az_result xCoreResult;

    if( pxWriter == NULL )
    {
        AZLogError( ( "AzureIoTJSONWriter_AppendEndArray failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        if( az_result_failed( xCoreResult = az_json_writer_append_end_array( &pxWriter->_internal.xCoreWriter ) ) )
        {
            AZLogError( ( "Could not append end array: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}
