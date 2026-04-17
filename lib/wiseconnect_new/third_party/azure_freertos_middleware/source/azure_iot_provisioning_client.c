/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot_provisioning_client.c
 * @brief Implementation of Azure IoT Provisioning client.
 *
 */

#include "azure_iot_provisioning_client.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* azure iot includes. */
#include "azure_iot_mqtt.h"
#include "azure_iot_private.h"
#include "azure_iot_result.h"

/* Azure SDK for Embedded C includes */
#include "azure/az_iot.h"
#include "azure/core/az_version.h"

#ifndef azureiotprovisioningDEFAULT_TOKEN_TIMEOUT_IN_SEC
    #define azureiotprovisioningDEFAULT_TOKEN_TIMEOUT_IN_SEC    azureiotconfigDEFAULT_TOKEN_TIMEOUT_IN_SEC
#endif /* azureiotprovisioningDEFAULT_TOKEN_TIMEOUT_IN_SEC */

#ifndef azureiotprovisioningKEEP_ALIVE_TIMEOUT_SECONDS
    #define azureiotprovisioningKEEP_ALIVE_TIMEOUT_SECONDS    azureiotconfigKEEP_ALIVE_TIMEOUT_SECONDS
#endif /* azureiotprovisioningKEEP_ALIVE_TIMEOUT_SECONDS */

#ifndef azureiotprovisioningCONNACK_RECV_TIMEOUT_MS
    #define azureiotprovisioningCONNACK_RECV_TIMEOUT_MS    azureiotconfigCONNACK_RECV_TIMEOUT_MS
#endif /* azureiotprovisioningCONNACK_RECV_TIMEOUT_MS */

#ifndef azureiotprovisioningUSER_AGENT
    #define azureiotprovisioningUSER_AGENT    ""
#endif /* azureiotprovisioningUSER_AGENT */

#ifndef azureiotprovisioningPROCESS_LOOP_TIMEOUT_MS
    #define azureiotprovisioningPROCESS_LOOP_TIMEOUT_MS    ( 500U )
#endif /* azureiotprovisioningPROCESS_LOOP_TIMEOUT_MS */

/* Define various workflow (WF) states Provisioning can get into */
#define azureiotprovisioningWF_STATE_INIT           ( 0x0 )
#define azureiotprovisioningWF_STATE_CONNECT        ( 0x1 )
#define azureiotprovisioningWF_STATE_SUBSCRIBE      ( 0x2 )
#define azureiotprovisioningWF_STATE_REQUEST        ( 0x3 )
#define azureiotprovisioningWF_STATE_RESPONSE       ( 0x4 )
#define azureiotprovisioningWF_STATE_SUBSCRIBING    ( 0x5 )
#define azureiotprovisioningWF_STATE_REQUESTING     ( 0x6 )
#define azureiotprovisioningWF_STATE_WAITING        ( 0x7 )
#define azureiotprovisioningWF_STATE_COMPLETE       ( 0x8 )

#define azureiotPrvGetMaxInt( a, b )    ( ( a ) > ( b ) ? ( a ) : ( b ) )

#define azureiotprovisioningREQUEST_PAYLOAD_LABEL            "payload"
#define azureiotprovisioningREQUEST_REGISTRATION_ID_LABEL    "registrationId"

#define azureiotprovisioningHMACBufferLength                 ( 48 )
/*-----------------------------------------------------------*/

/**
 *
 * State transitions :
 *          CONNECT -> { SUBSCRIBE | COMPLETE } -> { SUBSCRIBING | COMPLETE } -> { REQUEST | COMPLETE } ->
 *          REQUESTING -> { RESPONSE | COMPLETE } -> { WAITING | COMPLETE } -> { REQUEST | COMPLETE }
 *
 * Note : At COMPLETE state, we will check the status of lastOperation to see if WorkFlow( WF )
 *        finshed without error.
 *
 **/
static void prvProvClientUpdateState( AzureIoTProvisioningClient_t * pxAzureProvClient,
                                      uint32_t ulActionResult )
{
    uint32_t ulState = pxAzureProvClient->_internal.ulWorkflowState;

    pxAzureProvClient->_internal.ulLastOperationResult = ulActionResult;

    switch( ulState )
    {
        case azureiotprovisioningWF_STATE_CONNECT:
            pxAzureProvClient->_internal.ulWorkflowState =
                ulActionResult == eAzureIoTSuccess ? azureiotprovisioningWF_STATE_SUBSCRIBE :
                azureiotprovisioningWF_STATE_COMPLETE;
            break;

        case azureiotprovisioningWF_STATE_SUBSCRIBE:
            pxAzureProvClient->_internal.ulWorkflowState =
                ulActionResult == eAzureIoTSuccess ? azureiotprovisioningWF_STATE_SUBSCRIBING :
                azureiotprovisioningWF_STATE_COMPLETE;
            break;

        case azureiotprovisioningWF_STATE_SUBSCRIBING:
            pxAzureProvClient->_internal.ulWorkflowState =
                ulActionResult == eAzureIoTSuccess ? azureiotprovisioningWF_STATE_REQUEST :
                azureiotprovisioningWF_STATE_COMPLETE;
            break;

        case azureiotprovisioningWF_STATE_REQUEST:
            pxAzureProvClient->_internal.ulWorkflowState =
                ulActionResult == eAzureIoTSuccess ? azureiotprovisioningWF_STATE_REQUESTING :
                azureiotprovisioningWF_STATE_COMPLETE;
            break;

        case azureiotprovisioningWF_STATE_REQUESTING:
            pxAzureProvClient->_internal.ulWorkflowState =
                ulActionResult == eAzureIoTSuccess ? azureiotprovisioningWF_STATE_RESPONSE :
                azureiotprovisioningWF_STATE_COMPLETE;
            break;

        case azureiotprovisioningWF_STATE_RESPONSE:
            pxAzureProvClient->_internal.ulWorkflowState =
                ulActionResult == eAzureIoTErrorPending ? azureiotprovisioningWF_STATE_WAITING :
                azureiotprovisioningWF_STATE_COMPLETE;
            break;

        case azureiotprovisioningWF_STATE_WAITING:
            pxAzureProvClient->_internal.ulWorkflowState =
                ulActionResult == eAzureIoTSuccess ? azureiotprovisioningWF_STATE_REQUEST :
                azureiotprovisioningWF_STATE_COMPLETE;
            break;

        case azureiotprovisioningWF_STATE_COMPLETE:
            AZLogInfo( ( "AzureIoTProvisioning state 'Complete'" ) );
            break;

        default:
            AZLogError( ( "AzureIoTProvisioning unknown state: [%u]", ulState ) );
            configASSERT( false );
            break;
    }

    AZLogDebug( ( "AzureIoTProvisioning updated state from [%u] -> [%u]", ulState,
                  pxAzureProvClient->_internal.ulWorkflowState ) );
}
/*-----------------------------------------------------------*/

/**
 *
 * Implementation of the connect action. This action is only allowed in azureiotprovisioningWF_STATE_CONNECT
 *
 * */
static void prvProvClientConnect( AzureIoTProvisioningClient_t * pxAzureProvClient )
{
    AzureIoTMQTTConnectInfo_t xConnectInfo = { 0 };
    AzureIoTResult_t xResult;
    AzureIoTMQTTResult_t xMQTTResult;
    bool xSessionPresent;
    uint32_t ulPasswordLength = 0;
    size_t xMQTTUsernameLength;
    az_result xCoreResult;

    if( pxAzureProvClient->_internal.ulWorkflowState != azureiotprovisioningWF_STATE_CONNECT )
    {
        AZLogError( ( "AzureIoTProvisioning connect action called in wrong state: [%u]",
                      pxAzureProvClient->_internal.ulWorkflowState ) );
        return;
    }

    xConnectInfo.pcUserName = pxAzureProvClient->_internal.pucScratchBuffer;
    xConnectInfo.pcPassword = xConnectInfo.pcUserName + azureiotconfigUSERNAME_MAX;

    if( az_result_failed(
            xCoreResult = az_iot_provisioning_client_get_user_name( &pxAzureProvClient->_internal.xProvisioningClientCore,
                                                                    ( char * ) xConnectInfo.pcUserName,
                                                                    azureiotconfigUSERNAME_MAX, &xMQTTUsernameLength ) ) )
    {
        AZLogError( ( "AzureIoTProvisioning failed to get username: core error=0x%08x", xCoreResult ) );
        xResult = AzureIoT_TranslateCoreError( xCoreResult );
    }
    /* Check if token refresh is set, then generate password */
    else if( ( pxAzureProvClient->_internal.pxTokenRefresh ) &&
             ( pxAzureProvClient->_internal.pxTokenRefresh( pxAzureProvClient,
                                                            pxAzureProvClient->_internal.xGetTimeFunction() +
                                                            azureiotprovisioningDEFAULT_TOKEN_TIMEOUT_IN_SEC,
                                                            pxAzureProvClient->_internal.pucSymmetricKey,
                                                            pxAzureProvClient->_internal.ulSymmetricKeyLength,
                                                            ( uint8_t * ) xConnectInfo.pcPassword,
                                                            azureiotconfigPASSWORD_MAX,
                                                            &ulPasswordLength ) ) )
    {
        AZLogError( ( "AzureIoTProvisioning failed to generate auth token" ) );
        xResult = eAzureIoTErrorTokenGenerationFailed;
    }
    else
    {
        xConnectInfo.xCleanSession = true;
        xConnectInfo.pcClientIdentifier = pxAzureProvClient->_internal.pucRegistrationID;
        xConnectInfo.usClientIdentifierLength = ( uint16_t ) pxAzureProvClient->_internal.ulRegistrationIDLength;
        xConnectInfo.usUserNameLength = ( uint16_t ) xMQTTUsernameLength;
        xConnectInfo.usKeepAliveSeconds = azureiotprovisioningKEEP_ALIVE_TIMEOUT_SECONDS;
        xConnectInfo.usPasswordLength = ( uint16_t ) ulPasswordLength;

        if( ( xMQTTResult = AzureIoTMQTT_Connect( &( pxAzureProvClient->_internal.xMQTTContext ),
                                                  &xConnectInfo, NULL, azureiotprovisioningCONNACK_RECV_TIMEOUT_MS,
                                                  &xSessionPresent ) ) != eAzureIoTMQTTSuccess )
        {
            AZLogError( ( "AzureIoTProvisioning failed to establish MQTT connection: Server=%.*s, MQTT error=0x%08x",
                          pxAzureProvClient->_internal.ulEndpointLength,
                          pxAzureProvClient->_internal.pucEndpoint,
                          xMQTTResult ) );
            xResult = eAzureIoTErrorServerError;
        }
        else
        {
            /* Successfully established and MQTT connection with the broker. */
            AZLogInfo( ( "AzureIoTProvisioning established an MQTT connection with %.*s",
                         pxAzureProvClient->_internal.ulEndpointLength,
                         pxAzureProvClient->_internal.pucEndpoint ) );
            xResult = eAzureIoTSuccess;
        }
    }

    prvProvClientUpdateState( pxAzureProvClient, xResult );
}
/*-----------------------------------------------------------*/

/**
 *
 * Implementation of subscribe action, this action is only allowed in azureiotprovisioningWF_STATE_SUBSCRIBE
 *
 * */
static void prvProvClientSubscribe( AzureIoTProvisioningClient_t * pxAzureProvClient )
{
    AzureIoTMQTTSubscribeInfo_t xMQTTSubscription = { 0 };
    AzureIoTMQTTResult_t xMQTTResult;
    AzureIoTResult_t xResult;
    uint16_t usSubscribePacketIdentifier;

    if( pxAzureProvClient->_internal.ulWorkflowState != azureiotprovisioningWF_STATE_SUBSCRIBE )
    {
        AZLogWarn( ( "AzureIoTProvisioning subscribe action called in wrong state: [%u]",
                     pxAzureProvClient->_internal.ulWorkflowState ) );
    }
    else
    {
        xMQTTSubscription.xQoS = eAzureIoTMQTTQoS0;
        xMQTTSubscription.pcTopicFilter = ( const uint8_t * ) AZ_IOT_PROVISIONING_CLIENT_REGISTER_SUBSCRIBE_TOPIC;
        xMQTTSubscription.usTopicFilterLength = ( uint16_t ) sizeof( AZ_IOT_PROVISIONING_CLIENT_REGISTER_SUBSCRIBE_TOPIC ) - 1;

        usSubscribePacketIdentifier = AzureIoTMQTT_GetPacketId( &( pxAzureProvClient->_internal.xMQTTContext ) );

        AZLogInfo( ( "AzureIoTProvisioning attempting to subscribe to the MQTT topic: %s", AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC ) );

        if( ( xMQTTResult = AzureIoTMQTT_Subscribe( &( pxAzureProvClient->_internal.xMQTTContext ),
                                                    &xMQTTSubscription, 1, usSubscribePacketIdentifier ) ) != eAzureIoTMQTTSuccess )
        {
            AZLogError( ( "AzureIoTProvisioning failed to subscribe to MQTT topic: MQTT error=0x%08x", xMQTTResult ) );
            xResult = eAzureIoTErrorSubscribeFailed;
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }

        prvProvClientUpdateState( pxAzureProvClient, xResult );
    }
}
/*-----------------------------------------------------------*/

/**
 *
 * Implementation of request action. This action is only allowed in azureiotprovisioningWF_STATE_REQUEST
 *
 * */
static void prvProvClientRequest( AzureIoTProvisioningClient_t * pxAzureProvClient )
{
    AzureIoTMQTTResult_t xMQTTResult;
    AzureIoTResult_t xResult;
    AzureIoTMQTTPublishInfo_t xMQTTPublishInfo = { 0 };
    size_t xMQTTTopicLength;
    uint32_t xMQTTPayloadLength = 0;
    uint16_t usPublishPacketIdentifier;
    az_result xCoreResult;
    az_span xCustomPayloadProperty = az_span_create( ( uint8_t * ) pxAzureProvClient->_internal.pucRegistrationPayload,
                                                     ( int32_t ) pxAzureProvClient->_internal.ulRegistrationPayloadLength );

    /* Check the state.  */
    if( pxAzureProvClient->_internal.ulWorkflowState != azureiotprovisioningWF_STATE_REQUEST )
    {
        AZLogWarn( ( "AzureIoTProvisioning request action called in wrong state: [%u]",
                     pxAzureProvClient->_internal.ulWorkflowState ) );
    }
    else
    {
        /* Check if previous this is the 1st request or subsequent query request */
        if( pxAzureProvClient->_internal.xLastResponsePayloadLength == 0 )
        {
            xCoreResult =
                az_iot_provisioning_client_register_get_publish_topic( &pxAzureProvClient->_internal.xProvisioningClientCore,
                                                                       ( char * ) pxAzureProvClient->_internal.pucScratchBuffer,
                                                                       azureiotconfigTOPIC_MAX, &xMQTTTopicLength );
        }
        else
        {
            xCoreResult =
                az_iot_provisioning_client_query_status_get_publish_topic( &pxAzureProvClient->_internal.xProvisioningClientCore,
                                                                           pxAzureProvClient->_internal.xRegisterResponse.operation_id,
                                                                           ( char * ) pxAzureProvClient->_internal.pucScratchBuffer,
                                                                           azureiotconfigTOPIC_MAX, &xMQTTTopicLength );
        }

        if( az_result_failed( xCoreResult ) )
        {
            prvProvClientUpdateState( pxAzureProvClient, eAzureIoTErrorFailed );
            return;
        }

        xMQTTPayloadLength = pxAzureProvClient->_internal.ulScratchBufferLength - ( uint32_t ) xMQTTTopicLength;
        xCoreResult =
            az_iot_provisioning_client_get_request_payload( &pxAzureProvClient->_internal.xProvisioningClientCore,
                                                            xCustomPayloadProperty, NULL,
                                                            ( uint8_t * ) ( pxAzureProvClient->_internal.pucScratchBuffer +
                                                                            xMQTTTopicLength ),
                                                            ( size_t ) xMQTTPayloadLength, ( size_t * ) &xMQTTPayloadLength );

        if( az_result_failed( xCoreResult ) )
        {
            prvProvClientUpdateState( pxAzureProvClient, eAzureIoTErrorFailed );
            return;
        }

        xMQTTPublishInfo.xQOS = eAzureIoTMQTTQoS0;
        xMQTTPublishInfo.pcTopicName = pxAzureProvClient->_internal.pucScratchBuffer;
        xMQTTPublishInfo.usTopicNameLength = ( uint16_t ) xMQTTTopicLength;
        xMQTTPublishInfo.pvPayload = pxAzureProvClient->_internal.pucScratchBuffer + xMQTTTopicLength;
        xMQTTPublishInfo.xPayloadLength = xMQTTPayloadLength;
        usPublishPacketIdentifier = AzureIoTMQTT_GetPacketId( &( pxAzureProvClient->_internal.xMQTTContext ) );

        if( ( xMQTTResult = AzureIoTMQTT_Publish( &( pxAzureProvClient->_internal.xMQTTContext ),
                                                  &xMQTTPublishInfo, usPublishPacketIdentifier ) ) != eAzureIoTMQTTSuccess )
        {
            AZLogError( ( "AzureIoTProvisioning failed to publish prov request: MQTT error=0x%08x", xMQTTResult ) );
            xResult = eAzureIoTErrorPublishFailed;
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }

        prvProvClientUpdateState( pxAzureProvClient, xResult );
    }
}
/*-----------------------------------------------------------*/

/**
 *
 * Implementation of parsing response action, this action is only allowed in azureiotprovisioningWF_STATE_RESPONSE
 *
 * */
static void prvProvClientParseResponse( AzureIoTProvisioningClient_t * pxAzureProvClient )
{
    az_result xCoreResult;
    az_span xTopic = az_span_create( pxAzureProvClient->_internal.ucProvisioningLastResponse,
                                     ( int32_t ) pxAzureProvClient->_internal.usLastResponseTopicLength );
    az_span xPayload = az_span_create( pxAzureProvClient->_internal.ucProvisioningLastResponse +
                                       pxAzureProvClient->_internal.usLastResponseTopicLength,
                                       ( int32_t ) pxAzureProvClient->_internal.xLastResponsePayloadLength );

    /* Check the state.  */
    if( pxAzureProvClient->_internal.ulWorkflowState != azureiotprovisioningWF_STATE_RESPONSE )
    {
        AZLogWarn( ( "AzureIoTProvisioning parse response action called in wrong state: [%u]",
                     pxAzureProvClient->_internal.ulWorkflowState ) );
        return;
    }

    if( ( pxAzureProvClient->_internal.usLastResponseTopicLength == 0 ) ||
        ( pxAzureProvClient->_internal.xLastResponsePayloadLength == 0 ) )
    {
        AZLogError( ( "AzureIoTProvisioning client failed with invalid server response" ) );
        prvProvClientUpdateState( pxAzureProvClient, eAzureIoTErrorInvalidResponse );
        return;
    }

    xCoreResult =
        az_iot_provisioning_client_parse_received_topic_and_payload( &pxAzureProvClient->_internal.xProvisioningClientCore,
                                                                     xTopic, xPayload, &pxAzureProvClient->_internal.xRegisterResponse );

    if( xCoreResult == AZ_ERROR_IOT_TOPIC_NO_MATCH )
    {
        AZLogInfo( ( "AzureIoTProvisioning ignoring unknown topic." ) );
        /* Maintaining the same state. */
        return;
    }
    else if( az_result_failed( xCoreResult ) )
    {
        AZLogError( ( "AzureIoTProvisioning client failed to parse packet: core error=0x%08x", xCoreResult ) );
        prvProvClientUpdateState( pxAzureProvClient, eAzureIoTErrorFailed );
        return;
    }

    if( az_iot_provisioning_client_operation_complete( pxAzureProvClient->_internal.xRegisterResponse.operation_status ) )
    {
        switch( pxAzureProvClient->_internal.xRegisterResponse.operation_status )
        {
            case AZ_IOT_PROVISIONING_STATUS_ASSIGNED:
                prvProvClientUpdateState( pxAzureProvClient, eAzureIoTSuccess );
                break;

            case AZ_IOT_PROVISIONING_STATUS_FAILED:
                AZLogError( ( "AzureIoTProvisioning client registration failed with error %u: TrackingID: [%.*s] \"%.*s\"",
                              pxAzureProvClient->_internal.xRegisterResponse.registration_state.extended_error_code,
                              az_span_size( pxAzureProvClient->_internal.xRegisterResponse.registration_state.error_tracking_id ),
                              az_span_ptr( pxAzureProvClient->_internal.xRegisterResponse.registration_state.error_tracking_id ),
                              az_span_size( pxAzureProvClient->_internal.xRegisterResponse.registration_state.error_message ),
                              az_span_ptr( pxAzureProvClient->_internal.xRegisterResponse.registration_state.error_message ) ) );
                prvProvClientUpdateState( pxAzureProvClient, eAzureIoTErrorServerError );
                break;

            case AZ_IOT_PROVISIONING_STATUS_DISABLED:
                AZLogError( ( "AzureIoTProvisioning client: device is disabled." ) );
                prvProvClientUpdateState( pxAzureProvClient, eAzureIoTErrorServerError );
                break;

            default:
                AZLogError( ( "AzureIoTProvisioning unexpected operation status %d", pxAzureProvClient->_internal.xRegisterResponse.operation_status ) );
        }
    }
    else /* Operation is not complete. */
    {
        if( pxAzureProvClient->_internal.xRegisterResponse.retry_after_seconds == 0 )
        {
            pxAzureProvClient->_internal.xRegisterResponse.retry_after_seconds = azureiotconfigPROVISIONING_POLLING_INTERVAL_S;
        }

        pxAzureProvClient->_internal.ullRetryAfter =
            pxAzureProvClient->_internal.xGetTimeFunction() +
            pxAzureProvClient->_internal.xRegisterResponse.retry_after_seconds;
        prvProvClientUpdateState( pxAzureProvClient, eAzureIoTErrorPending );
    }
}
/*-----------------------------------------------------------*/

/**
 *
 * Implementation of wait check action. this action is only allowed in azureiotprovisioningWF_STATE_WAITING
 *
 * */
static void prvProvClientCheckTimeout( AzureIoTProvisioningClient_t * pxAzureProvClient )
{
    if( pxAzureProvClient->_internal.ulWorkflowState == azureiotprovisioningWF_STATE_WAITING )
    {
        if( pxAzureProvClient->_internal.xGetTimeFunction() >
            pxAzureProvClient->_internal.ullRetryAfter )
        {
            pxAzureProvClient->_internal.ullRetryAfter = 0;
            prvProvClientUpdateState( pxAzureProvClient, eAzureIoTSuccess );
        }
    }
}
/*-----------------------------------------------------------*/

/**
 * Trigger state machine action base on the state.
 *
 **/
static void prvProvClientTriggerAction( AzureIoTProvisioningClient_t * pxAzureProvClient )
{
    switch( pxAzureProvClient->_internal.ulWorkflowState )
    {
        case azureiotprovisioningWF_STATE_CONNECT:
            prvProvClientConnect( pxAzureProvClient );
            break;

        case azureiotprovisioningWF_STATE_SUBSCRIBE:
            prvProvClientSubscribe( pxAzureProvClient );
            break;

        case azureiotprovisioningWF_STATE_REQUEST:
            prvProvClientRequest( pxAzureProvClient );
            break;

        case azureiotprovisioningWF_STATE_RESPONSE:
            prvProvClientParseResponse( pxAzureProvClient );
            break;

        case azureiotprovisioningWF_STATE_SUBSCRIBING:
        case azureiotprovisioningWF_STATE_REQUESTING:
        case azureiotprovisioningWF_STATE_COMPLETE:
            /* None action taken here, as these states are waiting for receive path. */
            break;

        case azureiotprovisioningWF_STATE_WAITING:
            prvProvClientCheckTimeout( pxAzureProvClient );
            break;

        default:
            AZLogError( ( "AzureIoTProvisioning state not handled: [%u]",
                          pxAzureProvClient->_internal.ulWorkflowState ) );
            configASSERT( false );
    }
}
/*-----------------------------------------------------------*/

/**
 *  Run the workflow : trigger action on state and process receive path in MQTT loop
 *
 */
static AzureIoTResult_t prvProvClientRunWorkflow( AzureIoTProvisioningClient_t * pxAzureProvClient,
                                                  uint32_t ulTimeoutMilliseconds )
{
    AzureIoTMQTTResult_t xMQTTResult;
    AzureIoTResult_t xResult;
    uint32_t ulWaitTime;

    do
    {
        if( pxAzureProvClient->_internal.ulWorkflowState == azureiotprovisioningWF_STATE_COMPLETE )
        {
            AZLogDebug( ( "AzureIoTProvisioning state is already in complete state" ) );
            break;
        }

        if( ulTimeoutMilliseconds > azureiotprovisioningPROCESS_LOOP_TIMEOUT_MS )
        {
            ulTimeoutMilliseconds -= azureiotprovisioningPROCESS_LOOP_TIMEOUT_MS;
            ulWaitTime = azureiotprovisioningPROCESS_LOOP_TIMEOUT_MS;
        }
        else
        {
            ulWaitTime = ulTimeoutMilliseconds;
            ulTimeoutMilliseconds = 0;
        }

        prvProvClientTriggerAction( pxAzureProvClient );

        if( pxAzureProvClient->_internal.ulWorkflowState == azureiotprovisioningWF_STATE_COMPLETE )
        {
            AZLogDebug( ( "AzureIoTProvisioning is in complete state: status=0x%08x",
                          pxAzureProvClient->_internal.ulLastOperationResult ) );
            break;
        }
        else if( ( xMQTTResult =
                       AzureIoTMQTT_ProcessLoop( &( pxAzureProvClient->_internal.xMQTTContext ),
                                                 ulWaitTime ) ) != eAzureIoTMQTTSuccess )
        {
            AZLogError( ( "AzureIoTProvisioning failed to process loop: ProcessLoopDuration=%u, MQTT error=0x%08x",
                          ulTimeoutMilliseconds, xMQTTResult ) );
            prvProvClientUpdateState( pxAzureProvClient, eAzureIoTErrorFailed );
            break;
        }
    } while( ulTimeoutMilliseconds );

    if( ( pxAzureProvClient->_internal.ulWorkflowState != azureiotprovisioningWF_STATE_COMPLETE ) )
    {
        xResult = eAzureIoTErrorPending;
    }
    else
    {
        xResult = pxAzureProvClient->_internal.ulLastOperationResult;
    }

    return xResult;
}
/*-----------------------------------------------------------*/


/**
 *  Process MQTT Subscribe Ack from Provisioning Service
 *
 */
static void prvProvClientMQTTProcessSubAck( AzureIoTProvisioningClient_t * pxAzureProvClient,
                                            AzureIoTMQTTPublishInfo_t * pxPacketInfo )
{
    ( void ) pxPacketInfo;

    /* We assume success since IoT Provisioning would disconnect if there was a problem subscribing. */
    prvProvClientUpdateState( pxAzureProvClient, eAzureIoTSuccess );
}
/*-----------------------------------------------------------*/

/**
 * Process MQTT Response from Provisioning Service
 *
 */
static void prvProvClientMQTTProcessResponse( AzureIoTProvisioningClient_t * pxAzureProvClient,
                                              AzureIoTMQTTPublishInfo_t * pxPublishInfo )
{
    if( pxAzureProvClient->_internal.ulWorkflowState == azureiotprovisioningWF_STATE_REQUESTING )
    {
        if( ( pxPublishInfo->xPayloadLength + pxPublishInfo->usTopicNameLength ) <=
            sizeof( pxAzureProvClient->_internal.ucProvisioningLastResponse ) )
        {
            /* Copy topic + payload into the response buffer.
             * ucProvisioningLastResponse = [ topic payload ] */
            pxAzureProvClient->_internal.usLastResponseTopicLength =
                pxPublishInfo->usTopicNameLength;
            memcpy( pxAzureProvClient->_internal.ucProvisioningLastResponse,
                    pxPublishInfo->pcTopicName, pxPublishInfo->usTopicNameLength );

            pxAzureProvClient->_internal.xLastResponsePayloadLength =
                pxPublishInfo->xPayloadLength;
            memcpy( pxAzureProvClient->_internal.ucProvisioningLastResponse +
                    pxAzureProvClient->_internal.usLastResponseTopicLength,
                    pxPublishInfo->pvPayload, pxPublishInfo->xPayloadLength );

            prvProvClientUpdateState( pxAzureProvClient, eAzureIoTSuccess );
        }
        else
        {
            AZLogError( ( "AzureIoTProvisioning process response failed with buffer too small required size is %d bytes",
                          pxPublishInfo->xPayloadLength + pxPublishInfo->usTopicNameLength ) );
            prvProvClientUpdateState( pxAzureProvClient, eAzureIoTErrorOutOfMemory );
        }
    }
}
/*-----------------------------------------------------------*/

/**
 * Callback that receives all the Event from MQTT
 *
 */
static void prvProvClientEventCallback( AzureIoTMQTTHandle_t pxMQTTContext,
                                        AzureIoTMQTTPacketInfo_t * pxPacketInfo,
                                        AzureIoTMQTTDeserializedInfo_t * pxDeserializedInfo )
{
    AzureIoTProvisioningClient_t * pxAzureProvClient = ( AzureIoTProvisioningClient_t * ) pxMQTTContext;

    if( ( pxPacketInfo->ucType & 0xF0U ) == azureiotmqttPACKET_TYPE_SUBACK )
    {
        prvProvClientMQTTProcessSubAck( pxAzureProvClient,
                                        pxDeserializedInfo->pxPublishInfo );
    }
    else if( ( pxPacketInfo->ucType & 0xF0U ) == azureiotmqttPACKET_TYPE_PUBLISH )
    {
        prvProvClientMQTTProcessResponse( pxAzureProvClient,
                                          pxDeserializedInfo->pxPublishInfo );
    }
    else
    {
        AZLogDebug( ( "AzureIoTProvisioning received packet of type: %d", pxPacketInfo->ucType ) );
    }
}
/*-----------------------------------------------------------*/

/**
 * Generate the SAS token based on :
 *   https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-devguide-security#use-a-shared-access-policy
 *
 */
static uint32_t prvProvClientGetToken( AzureIoTProvisioningClient_t * pxAzureProvClient,
                                       uint64_t ullExpiryTimeSecs,
                                       const uint8_t * ucKey,
                                       uint32_t ulKeyLen,
                                       uint8_t * pucSASBuffer,
                                       uint32_t ulSasBufferLen,
                                       uint32_t * pulSaSLength )
{
    uint8_t * pucHMACBuffer;
    az_span xSpan = az_span_create( pucSASBuffer, ( int32_t ) ulSasBufferLen );
    az_result xCoreResult;
    uint32_t ulBytesUsed;
    uint32_t ulSignatureLength;
    uint32_t ulBufferLeft;
    size_t xLength;

    xCoreResult = az_iot_provisioning_client_sas_get_signature( &( pxAzureProvClient->_internal.xProvisioningClientCore ),
                                                                ullExpiryTimeSecs, xSpan, &xSpan );

    if( az_result_failed( xCoreResult ) )
    {
        AZLogError( ( "AzureIoTProvisioning failed to get signature: core error=0x%08x", xCoreResult ) );
        return AzureIoT_TranslateCoreError( xCoreResult );
    }

    ulBytesUsed = ( uint32_t ) az_span_size( xSpan );
    ulBufferLeft = ulSasBufferLen - ulBytesUsed;

    if( ulBufferLeft < azureiotprovisioningHMACBufferLength )
    {
        AZLogError( ( "AzureIoTProvisioning token generation failed with insufficient buffer size" ) );
        return eAzureIoTErrorOutOfMemory;
    }

    /* Calculate HMAC at the end of buffer, so we do less data movement when returning back to caller. */
    ulBufferLeft -= azureiotprovisioningHMACBufferLength;
    pucHMACBuffer = pucSASBuffer + ulSasBufferLen - azureiotprovisioningHMACBufferLength;

    if( AzureIoT_Base64HMACCalculate( pxAzureProvClient->_internal.xHMACFunction,
                                      ucKey, ulKeyLen, pucSASBuffer, ulBytesUsed, pucSASBuffer + ulBytesUsed, ulBufferLeft,
                                      pucHMACBuffer, azureiotprovisioningHMACBufferLength,
                                      &ulSignatureLength ) )
    {
        AZLogError( ( "AzureIoTProvisioning failed to encoded HMAC hash" ) );
        return eAzureIoTErrorFailed;
    }

    if( ( ulSasBufferLen <= azureiotprovisioningHMACBufferLength ) )
    {
        AZLogError( ( "AzureIoTProvisioning failed with insufficient buffer size" ) );
        return eAzureIoTErrorOutOfMemory;
    }

    xSpan = az_span_create( pucHMACBuffer, ( int32_t ) ulSignatureLength );
    xCoreResult = az_iot_provisioning_client_sas_get_password( &( pxAzureProvClient->_internal.xProvisioningClientCore ),
                                                               xSpan, ullExpiryTimeSecs, AZ_SPAN_EMPTY, ( char * ) pucSASBuffer,
                                                               ulSasBufferLen - azureiotprovisioningHMACBufferLength,
                                                               &xLength );

    if( az_result_failed( xCoreResult ) )
    {
        AZLogError( ( "AzureIoTProvisioning failed to generate token: core error=0x%08x", xCoreResult ) );
        return AzureIoT_TranslateCoreError( xCoreResult );
    }

    *pulSaSLength = ( uint32_t ) xLength;

    return eAzureIoTSuccess;
}
/*-----------------------------------------------------------*/

/**
 * Get number of millseconds passed to MQTT stack
 *
 */
static uint32_t prvProvClientGetTimeMillseconds( void )
{
    TickType_t xTickCount = 0;
    uint32_t ulTimeMs = 0UL;

    /* Get the current tick count. */
    xTickCount = xTaskGetTickCount();

    /* Convert the ticks to milliseconds. */
    ulTimeMs = ( uint32_t ) xTickCount * azureiotMILLISECONDS_PER_TICK;

    return ulTimeMs;
}
/*-----------------------------------------------------------*/

AzureIoTResult_t AzureIoTProvisioningClient_OptionsInit( AzureIoTProvisioningClientOptions_t * pxProvisioningClientOptions )
{
    AzureIoTResult_t xResult;

    if( pxProvisioningClientOptions == NULL )
    {
        AZLogError( ( "AzureIoTProvisioningClient_OptionsInit failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        pxProvisioningClientOptions->pucUserAgent = ( const uint8_t * ) azureiotprovisioningUSER_AGENT;
        pxProvisioningClientOptions->ulUserAgentLength = sizeof( azureiotprovisioningUSER_AGENT ) - 1;
        xResult = eAzureIoTSuccess;
    }

    return xResult;
}
/*-----------------------------------------------------------*/

AzureIoTResult_t AzureIoTProvisioningClient_Init( AzureIoTProvisioningClient_t * pxAzureProvClient,
                                                  const uint8_t * pucEndpoint,
                                                  uint32_t ulEndpointLength,
                                                  const uint8_t * pucIDScope,
                                                  uint32_t ulIDScopeLength,
                                                  const uint8_t * pucRegistrationID,
                                                  uint32_t ulRegistrationIDLength,
                                                  AzureIoTProvisioningClientOptions_t * pxProvisioningClientOptions,
                                                  uint8_t * pucBuffer,
                                                  uint32_t ulBufferLength,
                                                  AzureIoTGetCurrentTimeFunc_t xGetTimeFunction,
                                                  const AzureIoTTransportInterface_t * pxTransportInterface )
{
    AzureIoTResult_t xResult;
    az_span xEndpoint = az_span_create( ( uint8_t * ) pucEndpoint, ( int32_t ) ulEndpointLength );
    az_span xIDScope = az_span_create( ( uint8_t * ) pucIDScope, ( int32_t ) ulIDScopeLength );
    az_span xRegistrationID = az_span_create( ( uint8_t * ) pucRegistrationID, ( int32_t ) ulRegistrationIDLength );
    az_result xCoreResult;
    AzureIoTMQTTResult_t xMQTTResult;
    az_iot_provisioning_client_options xOptions = az_iot_provisioning_client_options_default();
    uint8_t * pucNetworkBuffer;
    uint32_t ulNetworkBufferLength;

    if( ( pxAzureProvClient == NULL ) ||
        ( pucEndpoint == NULL ) || ( ulEndpointLength == 0 ) ||
        ( pucIDScope == NULL ) || ( ulIDScopeLength == 0 ) ||
        ( pucRegistrationID == NULL ) || ( ulRegistrationIDLength == 0 ) ||
        ( pucBuffer == NULL ) || ( ulBufferLength == 0 ) ||
        ( xGetTimeFunction == NULL ) || ( pxTransportInterface == NULL ) )
    {
        AZLogError( ( "AzureIoTProvisioningClient_Init failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else if( ( ulBufferLength < ( azureiotconfigTOPIC_MAX + azureiotconfigPROVISIONING_REQUEST_PAYLOAD_MAX ) ) ||
             ( ulBufferLength < ( azureiotconfigUSERNAME_MAX + azureiotconfigPASSWORD_MAX ) ) )
    {
        AZLogError( ( "AzureIoTProvisioningClient_Init failed: insufficient buffer size" ) );
        xResult = eAzureIoTErrorOutOfMemory;
    }
    else
    {
        memset( pxAzureProvClient, 0, sizeof( AzureIoTProvisioningClient_t ) );
        /* Setup scratch buffer to be used by middleware */
        pxAzureProvClient->_internal.ulScratchBufferLength =
            azureiotPrvGetMaxInt( ( azureiotconfigUSERNAME_MAX + azureiotconfigPASSWORD_MAX ),
                                  ( azureiotconfigTOPIC_MAX + azureiotconfigPROVISIONING_REQUEST_PAYLOAD_MAX ) );
        pxAzureProvClient->_internal.pucScratchBuffer = pucBuffer;
        pucNetworkBuffer = pucBuffer + pxAzureProvClient->_internal.ulScratchBufferLength;
        ulNetworkBufferLength = ulBufferLength - pxAzureProvClient->_internal.ulScratchBufferLength;

        pxAzureProvClient->_internal.pucEndpoint = pucEndpoint;
        pxAzureProvClient->_internal.ulEndpointLength = ulEndpointLength;
        pxAzureProvClient->_internal.pucIDScope = pucIDScope;
        pxAzureProvClient->_internal.ulIDScopeLength = ulIDScopeLength;
        pxAzureProvClient->_internal.pucRegistrationID = pucRegistrationID;
        pxAzureProvClient->_internal.ulRegistrationIDLength = ulRegistrationIDLength;
        pxAzureProvClient->_internal.xGetTimeFunction = xGetTimeFunction;

        if( pxProvisioningClientOptions )
        {
            xOptions.user_agent = az_span_create( ( uint8_t * ) pxProvisioningClientOptions->pucUserAgent,
                                                  ( int32_t ) pxProvisioningClientOptions->ulUserAgentLength );
        }
        else
        {
            xOptions.user_agent = az_span_create( ( uint8_t * ) azureiotprovisioningUSER_AGENT,
                                                  sizeof( azureiotprovisioningUSER_AGENT ) - 1 );
        }

        xCoreResult = az_iot_provisioning_client_init( &( pxAzureProvClient->_internal.xProvisioningClientCore ),
                                                       xEndpoint, xIDScope, xRegistrationID, &xOptions );

        if( az_result_failed( xCoreResult ) )
        {
            AZLogError( ( "AzureIoTProvisioning initialization failed: core error=0x%08x", xCoreResult ) );
            xResult = AzureIoT_TranslateCoreError( xCoreResult );
        }
        else if( ( xMQTTResult = AzureIoTMQTT_Init( &( pxAzureProvClient->_internal.xMQTTContext ),
                                                    pxTransportInterface, prvProvClientGetTimeMillseconds,
                                                    prvProvClientEventCallback, pucNetworkBuffer,
                                                    ulNetworkBufferLength ) ) != eAzureIoTMQTTSuccess )
        {
            AZLogError( ( "AzureIoTProvisioning initialization failed: MQTT error=0x%08x", xMQTTResult ) );
            xResult = eAzureIoTErrorInitFailed;
        }
        else
        {
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}
/*-----------------------------------------------------------*/

void AzureIoTProvisioningClient_Deinit( AzureIoTProvisioningClient_t * pxAzureProvClient )
{
    if( pxAzureProvClient == NULL )
    {
        AZLogError( ( "AzureIoTProvisioningClient_Deinit failed: invalid argument" ) );
    }
}
/*-----------------------------------------------------------*/

AzureIoTResult_t AzureIoTProvisioningClient_SetSymmetricKey( AzureIoTProvisioningClient_t * pxAzureProvClient,
                                                             const uint8_t * pucSymmetricKey,
                                                             uint32_t ulSymmetricKeyLength,
                                                             AzureIoTGetHMACFunc_t xHmacFunction )
{
    AzureIoTResult_t xResult;

    if( ( pxAzureProvClient == NULL ) ||
        ( pucSymmetricKey == NULL ) || ( ulSymmetricKeyLength == 0 ) ||
        ( xHmacFunction == NULL ) )
    {
        AZLogError( ( "AzureIoTProvisioningClient_SetSymmetricKey failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        pxAzureProvClient->_internal.pucSymmetricKey = pucSymmetricKey;
        pxAzureProvClient->_internal.ulSymmetricKeyLength = ulSymmetricKeyLength;
        pxAzureProvClient->_internal.pxTokenRefresh = prvProvClientGetToken;
        pxAzureProvClient->_internal.xHMACFunction = xHmacFunction;
        xResult = eAzureIoTSuccess;
    }

    return xResult;
}
/*-----------------------------------------------------------*/

AzureIoTResult_t AzureIoTProvisioningClient_Register( AzureIoTProvisioningClient_t * pxAzureProvClient,
                                                      uint32_t ulTimeoutMilliseconds )
{
    AzureIoTResult_t xResult;

    if( pxAzureProvClient == NULL )
    {
        AZLogError( ( "AzureIoTProvisioningClient_Register failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else
    {
        if( pxAzureProvClient->_internal.ulWorkflowState == azureiotprovisioningWF_STATE_INIT )
        {
            pxAzureProvClient->_internal.ulWorkflowState = azureiotprovisioningWF_STATE_CONNECT;
        }

        xResult = prvProvClientRunWorkflow( pxAzureProvClient, ulTimeoutMilliseconds );
    }

    return xResult;
}
/*-----------------------------------------------------------*/

AzureIoTResult_t AzureIoTProvisioningClient_GetDeviceAndHub( AzureIoTProvisioningClient_t * pxAzureProvClient,
                                                             uint8_t * pucHubHostname,
                                                             uint32_t * pulHostnameLength,
                                                             uint8_t * pucDeviceID,
                                                             uint32_t * pulDeviceIDLength )
{
    uint32_t ulHostnameLength;
    uint32_t ulDeviceIDLength;
    AzureIoTResult_t xResult;
    az_span * pxHostname;
    az_span * pxDeviceID;

    if( ( pxAzureProvClient == NULL ) || ( pucHubHostname == NULL ) ||
        ( pulHostnameLength == NULL ) || ( pucDeviceID == NULL ) || ( pulDeviceIDLength == NULL ) )
    {
        AZLogError( ( "AzureIoTProvisioningClient_GetDeviceAndHub failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else if( pxAzureProvClient->_internal.ulWorkflowState != azureiotprovisioningWF_STATE_COMPLETE )
    {
        AZLogError( ( "AzureIoTProvisioning client state is not in complete state" ) );
        xResult = eAzureIoTErrorFailed;
    }
    else if( pxAzureProvClient->_internal.ulLastOperationResult )
    {
        xResult = pxAzureProvClient->_internal.ulLastOperationResult;
    }
    else
    {
        pxHostname = &pxAzureProvClient->_internal.xRegisterResponse.registration_state.assigned_hub_hostname;
        pxDeviceID = &pxAzureProvClient->_internal.xRegisterResponse.registration_state.device_id;
        ulHostnameLength = ( uint32_t ) az_span_size( *pxHostname );
        ulDeviceIDLength = ( uint32_t ) az_span_size( *pxDeviceID );

        if( ( *pulHostnameLength < ulHostnameLength ) || ( *pulDeviceIDLength < ulDeviceIDLength ) )
        {
            AZLogWarn( ( "AzureIoTProvisioning memory buffer passed is not enough to store hub info" ) );
            xResult = eAzureIoTErrorFailed;
        }
        else
        {
            memcpy( pucHubHostname, az_span_ptr( *pxHostname ), ulHostnameLength );
            memcpy( pucDeviceID, az_span_ptr( *pxDeviceID ), ulDeviceIDLength );
            *pulHostnameLength = ulHostnameLength;
            *pulDeviceIDLength = ulDeviceIDLength;
            xResult = eAzureIoTSuccess;
        }
    }

    return xResult;
}
/*-----------------------------------------------------------*/

AzureIoTResult_t AzureIoTProvisioningClient_GetExtendedCode( AzureIoTProvisioningClient_t * pxAzureProvClient,
                                                             uint32_t * pulExtendedErrorCode )
{
    AzureIoTResult_t xResult;

    if( ( pxAzureProvClient == NULL ) ||
        ( pulExtendedErrorCode == NULL ) )
    {
        AZLogError( ( "AzureIoTProvisioningClient_GetExtendedCode failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else if( pxAzureProvClient->_internal.ulWorkflowState != azureiotprovisioningWF_STATE_COMPLETE )
    {
        AZLogError( ( "AzureIoTProvisioning client state is not in complete state" ) );
        xResult = eAzureIoTErrorFailed;
    }
    else
    {
        *pulExtendedErrorCode =
            pxAzureProvClient->_internal.xRegisterResponse.registration_state.extended_error_code;
        xResult = eAzureIoTSuccess;
    }

    return xResult;
}
/*-----------------------------------------------------------*/

AzureIoTResult_t AzureIoTProvisioningClient_SetRegistrationPayload( AzureIoTProvisioningClient_t * pxAzureProvClient,
                                                                    const uint8_t * pucPayload,
                                                                    uint32_t ulPayloadLength )
{
    AzureIoTResult_t xResult;

    if( ( pxAzureProvClient == NULL ) ||
        ( pucPayload == NULL ) || ( ulPayloadLength == 0 ) )
    {
        AZLogError( ( "AzureIoTProvisioningClient_SetRegistrationPayload failed: invalid argument" ) );
        xResult = eAzureIoTErrorInvalidArgument;
    }
    else if( pxAzureProvClient->_internal.ulWorkflowState != azureiotprovisioningWF_STATE_INIT )
    {
        AZLogError( ( "AzureIoTProvisioning client state is not in init" ) );
        xResult = eAzureIoTErrorFailed;
    }
    else
    {
        pxAzureProvClient->_internal.pucRegistrationPayload = pucPayload;
        pxAzureProvClient->_internal.ulRegistrationPayloadLength = ulPayloadLength;
        xResult = eAzureIoTSuccess;
    }

    return xResult;
}
/*-----------------------------------------------------------*/
