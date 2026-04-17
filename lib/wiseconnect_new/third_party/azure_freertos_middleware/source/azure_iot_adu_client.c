/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

#include "azure_iot_adu_client.h"

#include "azure_iot.h"
/* Kernel includes. */
#include "FreeRTOS.h"

#include "azure_iot_private.h"
#include "azure_iot_json_writer.h"
#include "azure_iot_private.h"
#include <azure/iot/az_iot_adu_client.h>

const uint8_t * AzureIoTADUModelID = ( uint8_t * ) AZ_IOT_ADU_CLIENT_AGENT_MODEL_ID;
const uint32_t AzureIoTADUModelIDLength = sizeof( AZ_IOT_ADU_CLIENT_AGENT_MODEL_ID ) - 1;

AzureIoTResult_t AzureIoTADUClient_OptionsInit( AzureIoTADUClientOptions_t * pxADUClientOptions )
{
    AzureIoTResult_t xResult;

    if( pxADUClientOptions == NULL )
    {
        AZLogError( ( "AzureIoTADUClient_OptionsInit failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        memset( pxADUClientOptions, 0, sizeof( AzureIoTADUClientOptions_t ) );

        xResult = eAzureIoTSuccess;
    }

    return xResult;
}

AzureIoTResult_t AzureIoTADUClient_Init( AzureIoTADUClient_t * pxAzureIoTADUClient,
                                         AzureIoTADUClientOptions_t * pxADUClientOptions )
{
    az_result xCoreResult;
    az_iot_adu_client_options xADUOptions;

    if( pxAzureIoTADUClient == NULL )
    {
        AZLogError( ( "AzureIoTADUClient_Init failed: invalid argument" ) );
        return eAzureIoTErrorInvalidArgument;
    }

    memset( ( void * ) pxAzureIoTADUClient, 0, sizeof( AzureIoTADUClient_t ) );

    xADUOptions = az_iot_adu_client_options_default();

    if( pxADUClientOptions )
    {
        if( ( pxADUClientOptions->pucCompatibilityProperties != NULL ) &&
            ( pxADUClientOptions->ulCompatibilityPropertiesLength > 0 ) )
        {
            xADUOptions.device_compatibility_properties = az_span_create(
                ( uint8_t * ) pxADUClientOptions->pucCompatibilityProperties,
                ( int32_t ) pxADUClientOptions->ulCompatibilityPropertiesLength );
        }
    }

    if( az_result_failed( xCoreResult = az_iot_adu_client_init( &pxAzureIoTADUClient->_internal.xADUClient, &xADUOptions ) ) )
    {
        AZLogError( ( "Failed to initialize az_iot_adu_client_init: core error=0x%08x", xCoreResult ) );
        return AzureIoT_TranslateCoreError( xCoreResult );
    }

    return eAzureIoTSuccess;
}


AzureIoTResult_t AzureIoTADUClient_DevicePropertiesInit( AzureIoTADUClientDeviceProperties_t * pxADUDeviceProperties )
{
    AzureIoTResult_t xResult;

    if( pxADUDeviceProperties == NULL )
    {
        AZLogError( ( "AzureIoTADUClient_DevicePropertiesInit failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        memset( pxADUDeviceProperties, 0, sizeof( AzureIoTADUClientDeviceProperties_t ) );

        xResult = eAzureIoTSuccess;
    }

    return xResult;
}

bool AzureIoTADUClient_IsADUComponent( AzureIoTADUClient_t * pxAzureIoTADUClient,
                                       const uint8_t * pucComponentName,
                                       uint32_t ulComponentNameLength )
{
    if( pxAzureIoTADUClient == NULL )
    {
        AZLogError( ( "AzureIoTADUClient_IsADUComponent failed: invalid argument" ) );
        return false;
    }

    return az_iot_adu_client_is_component_device_update( &pxAzureIoTADUClient->_internal.xADUClient,
                                                         az_span_create( ( uint8_t * ) pucComponentName, ( int32_t ) ulComponentNameLength ) );
}

static void prvCastUpdateRequest( az_iot_adu_client_update_request * pxBaseUpdateRequest,
                                  az_iot_adu_client_update_manifest * pxBaseUpdateManifest,
                                  AzureIoTADUUpdateRequest_t * pxUpdateRequest,
                                  uint8_t * pucUnescapedManifest,
                                  uint32_t ulUnescapedManifestLength )
{
    pxUpdateRequest->xWorkflow.pucID = az_span_ptr( pxBaseUpdateRequest->workflow.id );
    pxUpdateRequest->xWorkflow.ulIDLength = ( uint32_t ) az_span_size( pxBaseUpdateRequest->workflow.id );
    pxUpdateRequest->xWorkflow.xAction = ( AzureIoTADUAction_t ) pxBaseUpdateRequest->workflow.action;
    pxUpdateRequest->xWorkflow.pucRetryTimestamp = az_span_ptr( pxBaseUpdateRequest->workflow.retry_timestamp );
    pxUpdateRequest->xWorkflow.ulRetryTimestampLength = ( uint32_t ) az_span_size( pxBaseUpdateRequest->workflow.retry_timestamp );
    pxUpdateRequest->pucUpdateManifest = ulUnescapedManifestLength > 0 ? pucUnescapedManifest : NULL;
    pxUpdateRequest->ulUpdateManifestLength = ulUnescapedManifestLength;
    pxUpdateRequest->pucUpdateManifestSignature = az_span_ptr( pxBaseUpdateRequest->update_manifest_signature );
    pxUpdateRequest->ulUpdateManifestSignatureLength = ( uint32_t ) az_span_size( pxBaseUpdateRequest->update_manifest_signature );
    pxUpdateRequest->ulFileUrlCount = pxBaseUpdateRequest->file_urls_count;

    for( uint32_t ulFileUrlIndex = 0; ulFileUrlIndex < pxBaseUpdateRequest->file_urls_count; ulFileUrlIndex++ )
    {
        pxUpdateRequest->pxFileUrls[ ulFileUrlIndex ].pucId = az_span_ptr( pxBaseUpdateRequest->file_urls[ ulFileUrlIndex ].id );
        pxUpdateRequest->pxFileUrls[ ulFileUrlIndex ].ulIdLength = ( uint32_t ) az_span_size( pxBaseUpdateRequest->file_urls[ ulFileUrlIndex ].id );
        pxUpdateRequest->pxFileUrls[ ulFileUrlIndex ].pucUrl = az_span_ptr( pxBaseUpdateRequest->file_urls[ ulFileUrlIndex ].url );
        pxUpdateRequest->pxFileUrls[ ulFileUrlIndex ].ulUrlLength = ( uint32_t ) az_span_size( pxBaseUpdateRequest->file_urls[ ulFileUrlIndex ].url );
    }

    if( pxBaseUpdateManifest != NULL )
    {
        pxUpdateRequest->xUpdateManifest.xUpdateId.pucProvider = az_span_ptr( pxBaseUpdateManifest->update_id.provider );
        pxUpdateRequest->xUpdateManifest.xUpdateId.ulProviderLength = ( uint32_t ) az_span_size( pxBaseUpdateManifest->update_id.provider );
        pxUpdateRequest->xUpdateManifest.xUpdateId.pucName = az_span_ptr( pxBaseUpdateManifest->update_id.name );
        pxUpdateRequest->xUpdateManifest.xUpdateId.ulNameLength = ( uint32_t ) az_span_size( pxBaseUpdateManifest->update_id.name );
        pxUpdateRequest->xUpdateManifest.xUpdateId.pucVersion = az_span_ptr( pxBaseUpdateManifest->update_id.version );
        pxUpdateRequest->xUpdateManifest.xUpdateId.ulVersionLength = ( uint32_t ) az_span_size( pxBaseUpdateManifest->update_id.version );

        pxUpdateRequest->xUpdateManifest.xInstructions.ulStepsCount = pxBaseUpdateManifest->instructions.steps_count;

        for( uint32_t ulStepIndex = 0; ulStepIndex < pxBaseUpdateManifest->instructions.steps_count; ulStepIndex++ )
        {
            pxUpdateRequest->xUpdateManifest.xInstructions.pxSteps[ ulStepIndex ].pucHandler =
                az_span_ptr( pxBaseUpdateManifest->instructions.steps[ ulStepIndex ].handler );
            pxUpdateRequest->xUpdateManifest.xInstructions.pxSteps[ ulStepIndex ].ulHandlerLength =
                ( uint32_t ) az_span_size( pxBaseUpdateManifest->instructions.steps[ ulStepIndex ].handler );
            pxUpdateRequest->xUpdateManifest.xInstructions.pxSteps[ ulStepIndex ].pucInstalledCriteria =
                az_span_ptr( pxBaseUpdateManifest->instructions.steps[ ulStepIndex ].handler_properties.installed_criteria );
            pxUpdateRequest->xUpdateManifest.xInstructions.pxSteps[ ulStepIndex ].ulInstalledCriteriaLength =
                ( uint32_t ) az_span_size( pxBaseUpdateManifest->instructions.steps[ ulStepIndex ].handler_properties.installed_criteria );
            pxUpdateRequest->xUpdateManifest.xInstructions.pxSteps[ ulStepIndex ].ulFilesCount =
                pxBaseUpdateManifest->instructions.steps[ ulStepIndex ].files_count;

            for( uint32_t ulFileIndex = 0; ulFileIndex < pxBaseUpdateManifest->instructions.steps[ ulStepIndex ].files_count; ulFileIndex++ )
            {
                pxUpdateRequest->xUpdateManifest.xInstructions.pxSteps[ ulStepIndex ].pxFiles[ ulFileIndex ].pucFileName =
                    az_span_ptr( pxBaseUpdateManifest->instructions.steps[ ulStepIndex ].files[ ulFileIndex ] );
                pxUpdateRequest->xUpdateManifest.xInstructions.pxSteps[ ulStepIndex ].pxFiles[ ulFileIndex ].ulFileNameLength =
                    ( uint32_t ) az_span_size( pxBaseUpdateManifest->instructions.steps[ ulStepIndex ].files[ ulFileIndex ] );
            }
        }

        pxUpdateRequest->xUpdateManifest.ulFilesCount = pxBaseUpdateManifest->files_count;

        for( uint32_t ulFileIndex = 0; ulFileIndex < pxBaseUpdateManifest->files_count; ulFileIndex++ )
        {
            pxUpdateRequest->xUpdateManifest.pxFiles[ ulFileIndex ].pucId =
                az_span_ptr( pxBaseUpdateManifest->files[ ulFileIndex ].id );
            pxUpdateRequest->xUpdateManifest.pxFiles[ ulFileIndex ].ulIdLength =
                ( uint32_t ) az_span_size( pxBaseUpdateManifest->files[ ulFileIndex ].id );
            pxUpdateRequest->xUpdateManifest.pxFiles[ ulFileIndex ].pucFileName =
                az_span_ptr( pxBaseUpdateManifest->files[ ulFileIndex ].file_name );
            pxUpdateRequest->xUpdateManifest.pxFiles[ ulFileIndex ].ulFileNameLength =
                ( uint32_t ) az_span_size( pxBaseUpdateManifest->files[ ulFileIndex ].file_name );
            pxUpdateRequest->xUpdateManifest.pxFiles[ ulFileIndex ].llSizeInBytes =
                pxBaseUpdateManifest->files[ ulFileIndex ].size_in_bytes;
            pxUpdateRequest->xUpdateManifest.pxFiles[ ulFileIndex ].ulHashesCount =
                pxBaseUpdateManifest->files[ ulFileIndex ].hashes_count;

            for( uint32_t ulFileHashIndex = 0; ulFileHashIndex < pxBaseUpdateManifest->files_count; ulFileHashIndex++ )
            {
                pxUpdateRequest->xUpdateManifest.pxFiles[ ulFileIndex ].pxHashes[ ulFileHashIndex ].pucId =
                    az_span_ptr( pxBaseUpdateManifest->files[ ulFileIndex ].hashes[ ulFileHashIndex ].hash_type );
                pxUpdateRequest->xUpdateManifest.pxFiles[ ulFileIndex ].pxHashes[ ulFileHashIndex ].ulIdLength =
                    ( uint32_t ) az_span_size( pxBaseUpdateManifest->files[ ulFileIndex ].hashes[ ulFileHashIndex ].hash_type );
                pxUpdateRequest->xUpdateManifest.pxFiles[ ulFileIndex ].pxHashes[ ulFileHashIndex ].pucHash =
                    az_span_ptr( pxBaseUpdateManifest->files[ ulFileIndex ].hashes[ ulFileHashIndex ].hash_value );
                pxUpdateRequest->xUpdateManifest.pxFiles[ ulFileIndex ].pxHashes[ ulFileHashIndex ].ulHashLength =
                    ( uint32_t ) az_span_size( pxBaseUpdateManifest->files[ ulFileIndex ].hashes[ ulFileHashIndex ].hash_value );
            }
        }

        pxUpdateRequest->xUpdateManifest.pucManifestVersion = az_span_ptr( pxBaseUpdateManifest->manifest_version );
        pxUpdateRequest->xUpdateManifest.ulManifestVersionLength = ( uint32_t ) az_span_size( pxBaseUpdateManifest->manifest_version );
        pxUpdateRequest->xUpdateManifest.pucCreateDateTime = az_span_ptr( pxBaseUpdateManifest->create_date_time );
        pxUpdateRequest->xUpdateManifest.ulCreateDateTimeLength = ( uint32_t ) az_span_size( pxBaseUpdateManifest->create_date_time );
    }
}

AzureIoTResult_t AzureIoTADUClient_ParseRequest( AzureIoTADUClient_t * pxAzureIoTADUClient,
                                                 AzureIoTJSONReader_t * pxReader,
                                                 AzureIoTADUUpdateRequest_t * pxAduUpdateRequest )
{
    az_iot_adu_client_update_request xBaseUpdateRequest;
    az_iot_adu_client_update_manifest xBaseUpdateManifest;
    az_result xAzResult;
    az_json_reader jr;

    if( ( pxAzureIoTADUClient == NULL ) || ( pxReader == NULL ) ||
        ( pxAduUpdateRequest == NULL ) )
    {
        AZLogError( ( "AzureIoTADUClient_ParseRequest failed: invalid argument" ) );
        return eAzureIoTErrorInvalidArgument;
    }

    xAzResult = az_iot_adu_client_parse_service_properties(
        &pxAzureIoTADUClient->_internal.xADUClient,
        &pxReader->_internal.xCoreReader,
        &xBaseUpdateRequest );

    if( az_result_failed( xAzResult ) )
    {
        AZLogError( ( "az_iot_adu_client_parse_service_properties failed" ) );
        return AzureIoT_TranslateCoreError( xAzResult );
    }
    else
    {
        if( az_span_size( xBaseUpdateRequest.update_manifest ) > 0 )
        {
            /* This unescape is done in place to save space. It will modify the original payload buffer. */
            xBaseUpdateRequest.update_manifest = az_json_string_unescape( xBaseUpdateRequest.update_manifest, xBaseUpdateRequest.update_manifest );

            xAzResult = az_json_reader_init( &jr, xBaseUpdateRequest.update_manifest, NULL );

            if( az_result_failed( xAzResult ) )
            {
                AZLogError( ( "az_json_reader_init failed: 0x%08x", xAzResult ) );
                return AzureIoT_TranslateCoreError( xAzResult );
            }

            xAzResult = az_iot_adu_client_parse_update_manifest(
                &pxAzureIoTADUClient->_internal.xADUClient,
                &jr,
                &xBaseUpdateManifest );

            if( az_result_failed( xAzResult ) )
            {
                AZLogError( ( "az_iot_adu_client_parse_update_manifest failed: 0x%08x", xAzResult ) );
                return AzureIoT_TranslateCoreError( xAzResult );
            }
        }
        else
        {
            /*No manifest to parse */
        }

        prvCastUpdateRequest( &xBaseUpdateRequest,
                              az_span_size( xBaseUpdateRequest.update_manifest ) > 0 ? &xBaseUpdateManifest : NULL,
                              pxAduUpdateRequest, az_span_ptr( xBaseUpdateRequest.update_manifest ), ( uint32_t ) az_span_size( xBaseUpdateRequest.update_manifest ) );
    }

    return eAzureIoTSuccess;
}

AzureIoTResult_t AzureIoTADUClient_SendResponse( AzureIoTADUClient_t * pxAzureIoTADUClient,
                                                 AzureIoTHubClient_t * pxAzureIoTHubClient,
                                                 AzureIoTADURequestDecision_t xRequestDecision,
                                                 uint32_t ulPropertyVersion,
                                                 uint8_t * pucWritablePropertyResponseBuffer,
                                                 uint32_t ulWritablePropertyResponseBufferSize,
                                                 uint32_t * pulRequestId )
{
    az_result xCoreResult;
    AzureIoTResult_t xResult;
    az_json_writer jw;
    az_span xPayload;

    if( ( pxAzureIoTADUClient == NULL ) || ( pxAzureIoTHubClient == NULL ) ||
        ( pucWritablePropertyResponseBuffer == NULL ) || ( ulWritablePropertyResponseBufferSize == 0 ) )
    {
        AZLogError( ( "AzureIoTADUClient_SendResponse failed: invalid argument" ) );
        return eAzureIoTErrorInvalidArgument;
    }

    xCoreResult = az_json_writer_init( &jw, az_span_create(
                                           pucWritablePropertyResponseBuffer,
                                           ( int32_t ) ulWritablePropertyResponseBufferSize ), NULL );

    if( az_result_failed( xCoreResult ) )
    {
        AZLogError( ( "az_json_writer_init failed: 0x%08x", xCoreResult ) );
        return AzureIoT_TranslateCoreError( xCoreResult );
    }

    xCoreResult = az_iot_adu_client_get_service_properties_response(
        &pxAzureIoTADUClient->_internal.xADUClient,
        ( int32_t ) ulPropertyVersion,
        xRequestDecision == eAzureIoTADURequestDecisionAccept ? AZ_IOT_ADU_CLIENT_REQUEST_DECISION_ACCEPT : AZ_IOT_ADU_CLIENT_REQUEST_DECISION_REJECT,
        &jw );

    if( az_result_failed( xCoreResult ) )
    {
        AZLogError( ( "az_iot_adu_client_get_service_properties_response failed: 0x%08x (%d)", xCoreResult, ulWritablePropertyResponseBufferSize ) );
        return AzureIoT_TranslateCoreError( xCoreResult );
    }

    xPayload = az_json_writer_get_bytes_used_in_destination( &jw );

    xResult = AzureIoTHubClient_SendPropertiesReported(
        pxAzureIoTHubClient,
        az_span_ptr( xPayload ),
        ( uint32_t ) az_span_size( xPayload ),
        pulRequestId );

    if( xResult != eAzureIoTSuccess )
    {
        AZLogError( ( "[ADU] Failed sending ADU writable properties response: 0x%08x", xResult ) );
        return eAzureIoTErrorPublishFailed;
    }

    return eAzureIoTSuccess;
}

static void prvFillBaseAduDeviceProperties( AzureIoTADUClientDeviceProperties_t * pxDeviceProperties,
                                            az_iot_adu_client_device_properties * pxBaseAduDeviceProperties )
{
    pxBaseAduDeviceProperties->manufacturer = az_span_create(
        ( uint8_t * ) pxDeviceProperties->ucManufacturer,
        ( int32_t ) pxDeviceProperties->ulManufacturerLength );
    pxBaseAduDeviceProperties->model = az_span_create(
        ( uint8_t * ) pxDeviceProperties->ucModel,
        ( int32_t ) pxDeviceProperties->ulModelLength );

    if( pxDeviceProperties->pxCustomProperties == NULL )
    {
        pxBaseAduDeviceProperties->custom_properties = NULL;
    }
    else
    {
        pxDeviceProperties->pxCustomProperties->_internal.xCustomProperties.count =
            ( int32_t ) pxDeviceProperties->pxCustomProperties->ulPropertyCount;

        for( int32_t lPropertyIndex = 0;
             lPropertyIndex < ( int32_t ) pxDeviceProperties->pxCustomProperties->ulPropertyCount;
             lPropertyIndex++ )
        {
            pxDeviceProperties->pxCustomProperties->_internal.xCustomProperties.names[ lPropertyIndex ] =
                az_span_create(
                    pxDeviceProperties->pxCustomProperties->pucPropertyNames[ lPropertyIndex ],
                    ( int32_t ) pxDeviceProperties->pxCustomProperties->ulPropertyNamesLengths[ lPropertyIndex ] );
            pxDeviceProperties->pxCustomProperties->_internal.xCustomProperties.values[ lPropertyIndex ] =
                az_span_create(
                    pxDeviceProperties->pxCustomProperties->pucPropertyValues[ lPropertyIndex ],
                    ( int32_t ) pxDeviceProperties->pxCustomProperties->ulPropertyValuesLengths[ lPropertyIndex ] );
        }

        pxBaseAduDeviceProperties->custom_properties =
            &pxDeviceProperties->pxCustomProperties->_internal.xCustomProperties;
    }

    pxBaseAduDeviceProperties->update_id = az_span_create(
        ( uint8_t * ) pxDeviceProperties->ucCurrentUpdateId,
        ( int32_t ) pxDeviceProperties->ulCurrentUpdateIdLength );
    pxBaseAduDeviceProperties->adu_version = AZ_SPAN_FROM_STR( AZ_IOT_ADU_CLIENT_AGENT_VERSION );
    pxBaseAduDeviceProperties->delivery_optimization_agent_version = az_span_create(
        ( uint8_t * ) pxDeviceProperties->ucDeliveryOptimizationAgentVersion,
        ( int32_t ) pxDeviceProperties->ulDeliveryOptimizationAgentVersionLength );
}

static void prvFillBaseAduWorkflow( AzureIoTADUUpdateRequest_t * pxAduUpdateRequest,
                                    az_iot_adu_client_workflow * pxBaseWorkflow )
{
    if( pxAduUpdateRequest != NULL )
    {
        pxBaseWorkflow->action = ( az_iot_adu_client_service_action ) pxAduUpdateRequest->xWorkflow.xAction;
        pxBaseWorkflow->id = az_span_create(
            ( uint8_t * ) pxAduUpdateRequest->xWorkflow.pucID,
            ( int32_t ) pxAduUpdateRequest->xWorkflow.ulIDLength );
        pxBaseWorkflow->retry_timestamp = az_span_create(
            ( uint8_t * ) pxAduUpdateRequest->xWorkflow.pucRetryTimestamp,
            ( int32_t ) pxAduUpdateRequest->xWorkflow.ulRetryTimestampLength );
    }
}

static void prvFillBaseAduInstallResults( AzureIoTADUClientInstallResult_t * pxUpdateResults,
                                          az_iot_adu_client_install_result * pxBaseInstallResults )
{
    memset( pxBaseInstallResults, 0, sizeof( *pxBaseInstallResults ) );

    if( pxUpdateResults != NULL )
    {
        pxBaseInstallResults->result_code = pxUpdateResults->lResultCode;
        pxBaseInstallResults->extended_result_code = pxUpdateResults->lExtendedResultCode;

        if( ( pxUpdateResults->pucResultDetails != NULL ) &&
            ( pxUpdateResults->ulResultDetailsLength > 0 ) )
        {
            pxBaseInstallResults->result_details = az_span_create(
                ( uint8_t * ) pxUpdateResults->pucResultDetails,
                ( int32_t ) pxUpdateResults->ulResultDetailsLength );
        }
        else
        {
            pxBaseInstallResults->result_details = AZ_SPAN_EMPTY;
        }

        pxBaseInstallResults->step_results_count = ( int32_t ) pxUpdateResults->ulStepResultsCount;

        for( int lIndex = 0; lIndex < ( int32_t ) pxUpdateResults->ulStepResultsCount; lIndex++ )
        {
            pxBaseInstallResults->step_results[ lIndex ].result_code =
                ( int32_t ) pxUpdateResults->pxStepResults[ lIndex ].ulResultCode;
            pxBaseInstallResults->step_results[ lIndex ].extended_result_code =
                ( int32_t ) pxUpdateResults->pxStepResults[ lIndex ].ulExtendedResultCode;

            if( ( pxUpdateResults->pxStepResults[ lIndex ].pucResultDetails != NULL ) &&
                ( pxUpdateResults->pxStepResults[ lIndex ].ulResultDetailsLength > 0 ) )
            {
                pxBaseInstallResults->step_results[ lIndex ].result_details = az_span_create(
                    ( uint8_t * ) pxUpdateResults->pxStepResults[ lIndex ].pucResultDetails,
                    ( int32_t ) pxUpdateResults->pxStepResults[ lIndex ].ulResultDetailsLength
                    );
            }
            else
            {
                pxBaseInstallResults->step_results[ lIndex ].result_details = AZ_SPAN_EMPTY;
            }
        }
    }
}

AzureIoTResult_t AzureIoTADUClient_SendAgentState( AzureIoTADUClient_t * pxAzureIoTADUClient,
                                                   AzureIoTHubClient_t * pxAzureIoTHubClient,
                                                   AzureIoTADUClientDeviceProperties_t * pxDeviceProperties,
                                                   AzureIoTADUUpdateRequest_t * pxAduUpdateRequest,
                                                   AzureIoTADUAgentState_t xAgentState,
                                                   AzureIoTADUClientInstallResult_t * pxUpdateResults,
                                                   uint8_t * pucBuffer,
                                                   uint32_t ulBufferSize,
                                                   uint32_t * pulRequestId )
{
    az_result xAzResult;
    az_iot_adu_client_device_properties xBaseADUDeviceProperties;
    az_iot_adu_client_workflow xBaseWorkflow;
    az_iot_adu_client_install_result xInstallResult;
    az_json_writer jw;
    az_span xPropertiesPayload;

    if( ( pxAzureIoTADUClient == NULL ) || ( pxAzureIoTHubClient == NULL ) ||
        ( pxDeviceProperties == NULL ) || ( pucBuffer == NULL ) || ( ulBufferSize == 0 ) )
    {
        AZLogError( ( "AzureIoTADUClient_SendAgentState failed: invalid argument" ) );
        return eAzureIoTErrorInvalidArgument;
    }

    prvFillBaseAduDeviceProperties( pxDeviceProperties, &xBaseADUDeviceProperties );
    prvFillBaseAduWorkflow( pxAduUpdateRequest, &xBaseWorkflow );
    prvFillBaseAduInstallResults( pxUpdateResults, &xInstallResult );

    xAzResult = az_json_writer_init( &jw, az_span_create( pucBuffer, ( int32_t ) ulBufferSize ), NULL );

    if( az_result_failed( xAzResult ) )
    {
        AZLogError( ( "az_json_writer_init failed: 0x%08x", xAzResult ) );
        return AzureIoT_TranslateCoreError( xAzResult );
    }

    xAzResult = az_iot_adu_client_get_agent_state_payload(
        &pxAzureIoTADUClient->_internal.xADUClient,
        &xBaseADUDeviceProperties,
        ( az_iot_adu_client_agent_state ) xAgentState,
        pxAduUpdateRequest != NULL ? &xBaseWorkflow : NULL,
        pxUpdateResults != NULL ? &xInstallResult : NULL,
        &jw );

    if( az_result_failed( xAzResult ) )
    {
        AZLogError( ( "az_iot_adu_client_get_agent_state_payload failed: 0x%08x", xAzResult ) );
        return AzureIoT_TranslateCoreError( xAzResult );
    }

    xPropertiesPayload = az_json_writer_get_bytes_used_in_destination( &jw );

    if( AzureIoTHubClient_SendPropertiesReported(
            pxAzureIoTHubClient,
            az_span_ptr( xPropertiesPayload ),
            ( uint32_t ) az_span_size( xPropertiesPayload ),
            pulRequestId ) != eAzureIoTSuccess )
    {
        AZLogError( ( "[ADU] Failed sending ADU agent state." ) );
        return eAzureIoTErrorPublishFailed;
    }

    return eAzureIoTSuccess;
}
