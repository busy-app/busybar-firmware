/**
 * @file azure_iot_adu_client.h
 *
 * @brief Definition for the Azure IoT ADU Client.
 *
 */
#ifndef AZURE_IOT_ADU_CLIENT_H
#define AZURE_IOT_ADU_CLIENT_H

#include <stdint.h>

#include "azure_iot_result.h"
#include "azure_iot_hub_client.h"
#include "azure_iot_json_reader.h"
#include <azure/iot/az_iot_adu_client.h>

/**
 * @brief The DTMI specifying the capabilities for the Azure Device Update client.
 *
 * This may be used in the #AzureIoTHubClientOptions_t as the `pucModelID`.
 */
extern const uint8_t * AzureIoTADUModelID;
extern const uint32_t AzureIoTADUModelIDLength;

/**
 * @brief Holds any user-defined custom properties of the device.
 * @remark Implementer can define other device properties to be used
 *         for the compatibility check while targeting the update deployment.
 */
typedef struct AzureIoTADUDeviceCustomProperties
{
    uint8_t * pucPropertyNames[ _az_IOT_ADU_CLIENT_MAX_DEVICE_CUSTOM_PROPERTIES ];
    uint32_t ulPropertyNamesLengths[ _az_IOT_ADU_CLIENT_MAX_DEVICE_CUSTOM_PROPERTIES ];
    uint8_t * pucPropertyValues[ _az_IOT_ADU_CLIENT_MAX_DEVICE_CUSTOM_PROPERTIES ];
    uint32_t ulPropertyValuesLengths[ _az_IOT_ADU_CLIENT_MAX_DEVICE_CUSTOM_PROPERTIES ];
    uint32_t ulPropertyCount;

    struct
    {
        az_iot_adu_device_custom_properties xCustomProperties;
    } _internal;
} AzureIoTADUDeviceCustomProperties_t;

/**
 * @brief ADU Device Properties.
 * @link https://docs.microsoft.com/en-us/azure/iot-hub-device-update/understand-device-update#device-update-agent
 *
 * @note AzureIoTADUClient_DevicePropertiesInit() should be called first to initialize this struct.
 */
typedef struct AzureIoTADUClientDeviceProperties
{
    const uint8_t * ucManufacturer;
    uint32_t ulManufacturerLength;

    const uint8_t * ucModel;
    uint32_t ulModelLength;

    AzureIoTADUDeviceCustomProperties_t * pxCustomProperties;

    const uint8_t * ucCurrentUpdateId;
    uint32_t ulCurrentUpdateIdLength;

    const uint8_t * ucDeliveryOptimizationAgentVersion;
    uint32_t ulDeliveryOptimizationAgentVersionLength;
} AzureIoTADUClientDeviceProperties_t;

/**
 * @brief Actions requested by the ADU Service
 *
 */
typedef enum AzureIoTADUAction
{
    eAzureIoTADUActionApplyDownload = 3,
    eAzureIoTADUActionCancel = 255
} AzureIoTADUAction_t;

/**
 * @brief ADU workflow struct.
 * @remark Format:
 *    {
 *      "action": 3,
 *      "id": "someguid",
 *      "retryTimestamp": "2020-04-22T12:12:12.0000000+00:00"
 *  }
 * @link https://docs.microsoft.com/en-us/azure/iot-hub-device-update/understand-device-update#device-update-agent
 */
typedef struct AzureIoTADUClientWorkflow
{
    AzureIoTADUAction_t xAction;

    const uint8_t * pucID;
    uint32_t ulIDLength;

    const uint8_t * pucRetryTimestamp;
    uint32_t ulRetryTimestampLength;
} AzureIoTADUClientWorkflow_t;

/**
 * @brief The update step result reported by the agent.
 *
 */
typedef struct AzureIoTADUClientStepResult
{
    uint32_t ulResultCode;
    uint32_t ulExtendedResultCode;

    const uint8_t * pucResultDetails;
    uint32_t ulResultDetailsLength;
} AzureIoTADUClientStepResult_t;

/**
 * @brief The update result reported by the agent.
 *
 */
typedef struct AzureIoTADUClientInstallResult
{
    int32_t lResultCode;
    int32_t lExtendedResultCode;

    const uint8_t * pucResultDetails;
    uint32_t ulResultDetailsLength;

    AzureIoTADUClientStepResult_t pxStepResults[ _az_IOT_ADU_CLIENT_MAX_INSTRUCTIONS_STEPS ];
    uint32_t ulStepResultsCount;
} AzureIoTADUClientInstallResult_t;

/**
 * @brief States of the ADU agent
 * @remark State is reported in response to an update request Action.
 * @link https://docs.microsoft.com/en-us/azure/iot-hub-device-update/device-update-plug-and-play#state
 *
 */
typedef enum AzureIoTADUAgentState
{
    eAzureIoTADUAgentStateIdle = 0,
    eAzureIoTADUAgentStateDeploymentInProgress = 6,
    eAzureIoTADUAgentStateFailed = 255,
    eAzureIoTADUAgentStateError,
} AzureIoTADUAgentState_t;

/**
 * @brief Decision values for accepting an update request or not.
 *
 */
typedef enum AzureIoTADURequestDecision
{
    eAzureIoTADURequestDecisionAccept,
    eAzureIoTADURequestDecisionReject
} AzureIoTADURequestDecision_t;

/**
 * @brief A map of file ID to download url.
 *
 */
typedef struct AzureIoTADUUpdateManifestFileUrl
{
    uint8_t * pucId;
    uint32_t ulIdLength;
    uint8_t * pucUrl;
    uint32_t ulUrlLength;
} AzureIoTADUUpdateManifestFileUrl_t;

/**
 * @brief     Identity of the update request.
 * @remark    This version refers to the update request itself.
 *            For verifying if an update request is applicable to an
 *            ADU agent, use the update manifest instructions steps "installed criteria".
 */
typedef struct AzureIoTADUUpdateId
{
    uint8_t * pucProvider;
    uint32_t ulProviderLength;
    uint8_t * pucName;
    uint32_t ulNameLength;
    uint8_t * pucVersion;
    uint32_t ulVersionLength;
} AzureIoTADUUpdateId_t;

/**
 * @brief The name of a file referenced in the update manifest.
 *
 */
typedef struct AzureIoTADUInstructionStepFile
{
    uint8_t * pucFileName;
    uint32_t ulFileNameLength;
} AzureIoTADUUpdateManifestInstructionStepFile_t;

/**
 * @brief Hash value of a given file.
 *
 */
typedef struct AzureIoTADUUpdateManifestFileHash
{
    uint8_t * pucId;
    uint32_t ulIdLength;
    uint8_t * pucHash;
    uint32_t ulHashLength;
} AzureIoTADUUpdateManifestFileHash_t;

/**
 * @brief Details of a file referenced in the update request.
 *
 */
typedef struct AzureIoTADUUpdateManifestFile
{
    uint8_t * pucId;
    uint32_t ulIdLength;
    uint8_t * pucFileName;
    uint32_t ulFileNameLength;
    int64_t llSizeInBytes;
    uint32_t ulHashesCount;
    AzureIoTADUUpdateManifestFileHash_t pxHashes[ _az_IOT_ADU_CLIENT_MAX_FILE_HASH_COUNT ];
} AzureIoTADUUpdateManifestFile_t;

/**
 * @brief A step in the instructions of an ADU update manifest.
 *
 */
typedef struct AzureIoTADUInstructionStep
{
    uint8_t * pucHandler;
    uint32_t ulHandlerLength;
    uint8_t * pucInstalledCriteria;
    uint32_t ulInstalledCriteriaLength;
    uint32_t ulFilesCount;
    AzureIoTADUUpdateManifestInstructionStepFile_t pxFiles[ _az_IOT_ADU_CLIENT_MAX_FILE_COUNT_PER_STEP ];
} AzureIoTADUInstructionStep_t;

/**
 * @brief Instructions in the update manifest.
 */
typedef struct AzureIoTADUInstructions
{
    uint32_t ulStepsCount;
    AzureIoTADUInstructionStep_t pxSteps[ _az_IOT_ADU_CLIENT_MAX_INSTRUCTIONS_STEPS ];
} AzureIoTADUInstructions_t;

/**
 * @brief Structure that holds the parsed contents of the update manifest
 *        sent by the ADU service.
 */
typedef struct AzureIoTADUUpdateManifest
{
    AzureIoTADUUpdateId_t xUpdateId;
    AzureIoTADUInstructions_t xInstructions;
    uint32_t ulFilesCount;
    AzureIoTADUUpdateManifestFile_t pxFiles[ _az_IOT_ADU_CLIENT_MAX_TOTAL_FILE_COUNT ];
    uint8_t * pucManifestVersion;
    uint32_t ulManifestVersionLength;
    uint8_t * pucCreateDateTime;
    uint32_t ulCreateDateTimeLength;
} AzureIoTADUUpdateManifest_t;

/**
 * @brief Structure that holds the parsed contents of the ADU
 *        request in the Plug and Play writable properties sent
 *        by the ADU service.
 */
typedef struct AzureIoTADUUpdateRequest
{
    AzureIoTADUClientWorkflow_t xWorkflow;
    uint8_t * pucUpdateManifest;
    uint32_t ulUpdateManifestLength;
    uint8_t * pucUpdateManifestSignature;
    uint32_t ulUpdateManifestSignatureLength;
    uint32_t ulFileUrlCount;
    AzureIoTADUUpdateManifestFileUrl_t pxFileUrls[ _az_IOT_ADU_CLIENT_MAX_TOTAL_FILE_COUNT ];
    AzureIoTADUUpdateManifest_t xUpdateManifest;
} AzureIoTADUUpdateRequest_t;

/**
 * @brief User-defined options for the Azure IoT ADU client.
 */
typedef struct AzureIoTADUClientOptions
{
    const uint8_t * pucCompatibilityProperties;
    uint32_t ulCompatibilityPropertiesLength;
}
AzureIoTADUClientOptions_t;

/**
 * @brief Azure IoT ADU Client (ADU agent) to handle stages of the ADU process.
 */
typedef struct AzureIoTADUClient
{
    struct
    {
        az_iot_adu_client xADUClient;
    } _internal;
} AzureIoTADUClient_t;

/**
 * @brief Initialize the Azure IoT ADU Options with default values.
 *
 * @param[out] pxADUClientOptions The #AzureIoTADUClientOptions_t instance to set with default values.
 * @return An #AzureIoTResult_t with the result of the operation.
 */
AzureIoTResult_t AzureIoTADUClient_OptionsInit( AzureIoTADUClientOptions_t * pxADUClientOptions );

/**
 * @brief Initialize the Azure IoT ADU Client.
 *
 * @param pxAzureIoTADUClient The #AzureIoTADUClient_t * to use for this call.
 * @param pxADUClientOptions The #AzureIoTADUClientOptions_t for the IoT ADU client instance.
 * @return An #AzureIoTResult_t with the result of the operation.
 */
AzureIoTResult_t AzureIoTADUClient_Init( AzureIoTADUClient_t * pxAzureIoTADUClient,
                                         AzureIoTADUClientOptions_t * pxADUClientOptions );

/**
 * @brief Initialize the Azure IoT Device Properties with default values.
 *
 * @param[out] pxADUDeviceProperties The #AzureIoTADUClientDeviceProperties_t instance to set with default values.
 * @return An #AzureIoTResult_t with the result of the operation.
 */
AzureIoTResult_t AzureIoTADUClient_DevicePropertiesInit( AzureIoTADUClientDeviceProperties_t * pxADUDeviceProperties );

/**
 * @brief Returns whether the component is the ADU component
 *
 * @note If it is, user should follow by parsing the component with the
 *       AzureIoTHubClient_ADUProcessComponent() call. The properties will be
 *       processed into the AzureIoTADUClient.
 *
 * @param[in] pxAzureIoTADUClient The #AzureIoTADUClient_t * to use for this call.
 * @param[in] pucComponentName Name of writable properties component to be
 *                             checked.
 * @param[in] ulComponentNameLength Length of `pucComponentName`.
 * @return A boolean value indicating if the writable properties component
 *         is from ADU service.
 */
bool AzureIoTADUClient_IsADUComponent( AzureIoTADUClient_t * pxAzureIoTADUClient,
                                       const uint8_t * pucComponentName,
                                       uint32_t ulComponentNameLength );

/**
 * @brief Parse the ADU update request into the requisite structure.
 *
 * The JSON reader returned to the caller from AzureIoTHubClientProperties_GetNextComponentProperty()
 * should be passed to this API.
 *
 * @param pxAzureIoTADUClient The #AzureIoTADUClient_t * to use for this call.
 * @param pxReader The initialized JSON reader positioned at the beginning of the ADU subcomponent
 * property.
 * @param pxAduUpdateRequest The #AzureIoTADUUpdateRequest_t into which the properties will be parsed.
 * @return AzureIoTResult_t An #AzureIoTResult_t with the result of the operation.
 */
AzureIoTResult_t AzureIoTADUClient_ParseRequest( AzureIoTADUClient_t * pxAzureIoTADUClient,
                                                 AzureIoTJSONReader_t * pxReader,
                                                 AzureIoTADUUpdateRequest_t * pxAduUpdateRequest );

/**
 * @brief Updates the ADU Agent Client with ADU service device update properties.
 * @remark It must be called whenever writable properties are received containing
 *         ADU service properties (verified with AzureIoTADUClient_IsADUComponent).
 *         It effectively parses the properties (aka, the device update request)
 *         from ADU and sets the state machine to perform the update process if the
 *         the update request is applicable (e.g., if the version is not already
 *         installed).
 *         This function also provides the payload to acknowledge the ADU service
 *         Azure Plug-and-Play writable properties.
 *
 * @param[in] pxAzureIoTADUClient The #AzureIoTADUClient_t * to use for this call.
 * @param[in] pxAzureIoTHubClient The #AzureIoTHubClient_t * to use for this call.
 * @param[in] xRequestDecision  The #AzureIoTADURequestDecision_t for this response.
 * @param[in] ulPropertyVersion Version of the writable properties.
 * @param[in] pucWritablePropertyResponseBuffer
 *              An pointer to the memory buffer where to
 *              write the resulting Azure Plug-and-Play properties acknowledgement
 *              payload.
 * @param[in] ulWritablePropertyResponseBufferSize
 *              Size of `pucWritablePropertyResponseBuffer.
 * @param[in] pulRequestId Pointer to request id to use for the operation.
 * @return An #AzureIoTResult_t with the result of the operation.
 */
AzureIoTResult_t AzureIoTADUClient_SendResponse( AzureIoTADUClient_t * pxAzureIoTADUClient,
                                                 AzureIoTHubClient_t * pxAzureIoTHubClient,
                                                 AzureIoTADURequestDecision_t xRequestDecision,
                                                 uint32_t ulPropertyVersion,
                                                 uint8_t * pucWritablePropertyResponseBuffer,
                                                 uint32_t ulWritablePropertyResponseBufferSize,
                                                 uint32_t * pulRequestId );

/**
 * @brief Sends the current state of the Azure IoT ADU agent.
 *
 * @param[in] pxAzureIoTADUClient The #AzureIoTADUClient_t * to use for this call.
 * @param[in] pxAzureIoTHubClient The #AzureIoTHubClient_t * to use for this call.
 * @param pxDeviceProperties The device information which will be used to generate the payload.
 * @param pxAduUpdateRequest The current #AzureIoTADUUpdateRequest_t. This can be `NULL` if there isn't currently
 * an update request.
 * @param xAgentState The current #AzureIoTADUAgentState_t.
 * @param pxUpdateResults The current #AzureIoTADUClientInstallResult_t. This can be `NULL` if there aren't any
 * results from an update.
 * @param pucBuffer The buffer into which the generated payload will be placed.
 * @param ulBufferSize The length of \p pucBuffer.
 * @param pulRequestId An optional request id to be used for the publish. This can be `NULL`.
 * @return An #AzureIoTResult_t with the result of the operation.
 */
AzureIoTResult_t AzureIoTADUClient_SendAgentState( AzureIoTADUClient_t * pxAzureIoTADUClient,
                                                   AzureIoTHubClient_t * pxAzureIoTHubClient,
                                                   AzureIoTADUClientDeviceProperties_t * pxDeviceProperties,
                                                   AzureIoTADUUpdateRequest_t * pxAduUpdateRequest,
                                                   AzureIoTADUAgentState_t xAgentState,
                                                   AzureIoTADUClientInstallResult_t * pxUpdateResults,
                                                   uint8_t * pucBuffer,
                                                   uint32_t ulBufferSize,
                                                   uint32_t * pulRequestId );


#endif /* AZURE_IOT_ADU_CLIENT_H */
