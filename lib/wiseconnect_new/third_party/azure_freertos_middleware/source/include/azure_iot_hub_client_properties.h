/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot_hub_client_properties.h
 *
 * @brief The middleware IoT Hub Client property functions used for Azure IoT Plug and Play.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 *
 */

#ifndef AZURE_IOT_HUB_CLIENT_PROPERTIES_H
#define AZURE_IOT_HUB_CLIENT_PROPERTIES_H

#include <stdbool.h>
#include <stdint.h>

#include "azure_iot_hub_client.h"
#include "azure_iot_json_reader.h"
#include "azure_iot_json_writer.h"

/* Azure SDK for Embedded C includes */
#include "azure/iot/az_iot_hub_client_properties.h"
#include "azure/core/_az_cfg_prefix.h"

/**
 * @brief Append the necessary characters to a reported properties JSON payload belonging to a
 * component.
 *
 * The payload will be of the form:
 *
 * @code
 * "reported": {
 *     "<pucComponentName>": {
 *         "__t": "c",
 *         "temperature": 23
 *     }
 * }
 * @endcode
 *
 * @note This API only builds the metadata for a component's properties.  The
 * application itself must specify the payload contents between calls
 * to this API and AzureIoTHubClientProperties_BuilderEndComponent() using
 * \p pxJSONWriter to specify the JSON payload.
 *
 * @param[in] pxAzureIoTHubClient The #AzureIoTHubClient_t to use for this call.
 * @param[in,out] pxJSONWriter The #AzureIoTJSONWriter_t to append the necessary characters for an IoT
 * Plug and Play component.
 * @param[in] pucComponentName The component name associated with the reported properties.
 * @param[in] usComponentNameLength The length of \p pucComponentName.
 *
 * @pre \p pxAzureIoTHubClient must not be `NULL`.
 * @pre \p pxJSONWriter must not be `NULL`.
 * @pre \p pucComponentName must not be `NULL`.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The JSON payload was prefixed successfully.
 */
AzureIoTResult_t AzureIoTHubClientProperties_BuilderBeginComponent( AzureIoTHubClient_t * pxAzureIoTHubClient,
                                                                    AzureIoTJSONWriter_t * pxJSONWriter,
                                                                    const uint8_t * pucComponentName,
                                                                    uint16_t usComponentNameLength );

/**
 * @brief Append the necessary characters to end a reported properties JSON payload belonging to a
 * component.
 *
 * @note This API should be used in conjunction with
 * AzureIoTHubClientProperties_BuilderBeginComponent().
 *
 * @param[in] pxAzureIoTHubClient The #AzureIoTHubClient_t to use for this call.
 * @param[in,out] pxJSONWriter The #AzureIoTJSONWriter_t to append the necessary characters for an IoT
 * Plug and Play component.
 *
 * @pre \p pxAzureIoTHubClient must not be `NULL`.
 * @pre \p pxJSONWriter must not be `NULL`.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The JSON payload was suffixed successfully.
 */
AzureIoTResult_t AzureIoTHubClientProperties_BuilderEndComponent( AzureIoTHubClient_t * pxAzureIoTHubClient,
                                                                  AzureIoTJSONWriter_t * pxJSONWriter );

/**
 * @brief Begin a property response to a writable property request from the service.
 *
 * This API should be used in response to an incoming writable properties. More details can be
 * found here:
 *
 * https://docs.microsoft.com/en-us/azure/iot-pnp/concepts-convention#writable-properties
 *
 * The payload will be of the form:
 *
 * **Without component**
 * @code
 * {
 *   "<pucPropertyName>":{
 *     "ac": <lAckCode>,
 *     "av": <lAckVersion>,
 *     "ad": "<pucAckDescription>",
 *     "value": <user_value>
 *   }
 * }
 * @endcode
 *
 * **With component**
 * @code
 * {
 *   "<pucComponentName>": {
 *     "__t": "c",
 *     "<pucPropertyName>": {
 *       "ac": <lAckCode>,
 *       "av": <lAckVersion>,
 *       "ad": "<pucAckDescription>",
 *       "value": <user_value>
 *     }
 *   }
 * }
 * @endcode
 *
 * To send a status for properties belonging to a component, first call the
 * AzureIoTHubClientProperties_BuilderBeginComponent() API to prefix the payload with the
 * necessary identification. The API call flow would look like the following with the listed JSON
 * payload being generated.
 *
 * @code
 * AzureIoTHubClientProperties_BuilderBeginComponent()
 * AzureIoTHubClientProperties_BuilderBeginResponseStatus()
 * // Append user value here (<user_value>) using pxJSONWriter directly.
 * AzureIoTHubClientProperties_BuilderEndResponseStatus()
 * AzureIoTHubClientProperties_BuilderEndComponent()
 * @endcode
 *
 * @note This API only builds the metadata for the properties response.  The
 * application itself must specify the payload contents between calls
 * to this API and AzureIoTHubClientProperties_BuilderEndResponseStatus() using
 * \p pxJSONWriter to specify the JSON payload.
 *
 * @param[in] pxAzureIoTHubClient The #AzureIoTHubClient_t to use for this call.
 * @param[in,out] pxJSONWriter The initialized #AzureIoTJSONWriter_t to append data to.
 * @param[in] pucPropertyName The name of the property to build a response payload for.
 * @param[in] usPropertyNameLength The length of \p pucPropertyName.
 * @param[in] lAckCode The HTTP-like status code to respond with. See #AzureIoTHubMessageStatus_t for
 * possible supported values.
 * @param[in] lAckVersion The version of the property the application is acknowledging.
 * This can be retrieved from the service request by
 * calling az_iot_hub_client_properties_get_properties_version.
 * @param[in] pucAckDescription An optional description detailing the context or any details about
 * the acknowledgement. This can be `NULL`.
 * @param[in] usAckDescriptionLength The length of \p pucAckDescription.
 *
 * @pre \p pxAzureIoTHubClient must not be `NULL`.
 * @pre \p pxJSONWriter must not be `NULL`.
 * @pre \p pucPropertyName must not be `NULL`.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The JSON payload was prefixed successfully.
 */
AzureIoTResult_t AzureIoTHubClientProperties_BuilderBeginResponseStatus( AzureIoTHubClient_t * pxAzureIoTHubClient,
                                                                         AzureIoTJSONWriter_t * pxJSONWriter,
                                                                         const uint8_t * pucPropertyName,
                                                                         uint16_t usPropertyNameLength,
                                                                         int32_t lAckCode,
                                                                         int32_t lAckVersion,
                                                                         const uint8_t * pucAckDescription,
                                                                         uint16_t usAckDescriptionLength );

/**
 * @brief End a properties response payload with confirmation status.
 *
 * @note This API should be used in conjunction with
 * AzureIoTHubClientProperties_BuilderBeginResponseStatus().
 *
 * @param[in] pxAzureIoTHubClient The #AzureIoTHubClient_t to use for this call.
 * @param[in,out] pxJSONWriter The initialized #AzureIoTJSONWriter_t to append data to.
 *
 * @pre \p pxAzureIoTHubClient must not be `NULL`.
 * @pre \p pxJSONWriter must not be `NULL`.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The JSON payload was suffixed successfully.
 */
AzureIoTResult_t AzureIoTHubClientProperties_BuilderEndResponseStatus( AzureIoTHubClient_t * pxAzureIoTHubClient,
                                                                       AzureIoTJSONWriter_t * pxJSONWriter );

/**
 * @brief Read the Azure IoT Plug and Play property version.
 *
 * @warning This modifies the state of the json reader. To then use the same json reader
 * with AzureIoTHubClientProperties_GetNextComponentProperty(), you must call
 * AzureIoTJSONReader_Init() again after this call and before the call to
 * AzureIoTHubClientProperties_GetNextComponentProperty() or make an additional copy before
 * calling this API.
 *
 * @param[in] pxAzureIoTHubClient The #AzureIoTHubClient_t to use for this call.
 * @param[in,out] pxJSONReader The pointer to the #AzureIoTJSONReader_t used to parse through the JSON
 * payload.
 * @param[in] xResponseType The #AzureIoTHubMessageType_t representing the message
 * type associated with the payload.
 * @param[out] pulVersion The numeric version of the properties in the JSON payload.
 *
 * @pre \p pxAzureIoTHubClient must not be `NULL`.
 * @pre \p pxJSONReader must not be `NULL`.
 * @pre \p xResponseType must be #eAzureIoTHubPropertiesRequestedMessage or #eAzureIoTHubPropertiesWritablePropertyMessage.
 * @pre \p pulVersion must not be `NULL`.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess If the function returned a valid version.
 */
AzureIoTResult_t AzureIoTHubClientProperties_GetPropertiesVersion( AzureIoTHubClient_t * pxAzureIoTHubClient,
                                                                   AzureIoTJSONReader_t * pxJSONReader,
                                                                   AzureIoTHubMessageType_t xResponseType,
                                                                   uint32_t * pulVersion );

/**
 * @brief Property type
 *
 */
typedef enum AzureIoTHubClientPropertyType_t
{
    /** @brief Property was originally reported from the device. */
    eAzureIoTHubClientReportedFromDevice = AZ_IOT_HUB_CLIENT_PROPERTY_REPORTED_FROM_DEVICE,
    /** @brief Property was received from the service. */
    eAzureIoTHubClientPropertyWritable = AZ_IOT_HUB_CLIENT_PROPERTY_WRITABLE
} AzureIoTHubClientPropertyType_t;

/**
 * @brief Iteratively read the Azure IoT Plug and Play component properties.
 *
 * @note If you are creating a multi-component application, component names must first be registered
 * via the `pxHubClientOptions->pxComponentList` and `pxHubClientOptions->ulComponentListLength` in
 * AzureIoTHubClient_Init().
 *
 * Note that between calls, the uint8_t* pointed to by \p ppucComponentName shall not be modified,
 * only checked and compared. Internally, the pointer is only changed if the component name changes
 * in the JSON document and is not necessarily set every invocation of the function.
 *
 * On success, the `pxJSONReader` will be set on a valid property name. After checking the
 * property name, the reader can be advanced to the property value by calling
 * AzureIoTJSONReader_NextToken(). Note that on the subsequent call to this API, it is expected that
 * the json reader will be placed AFTER the read property name and value. That means that after
 * reading the property value (including single values or complex objects), the user must call
 * AzureIoTJSONReader_NextToken().
 *
 * Below is a code snippet which you can use as a starting point:
 *
 * @code
 *
 * while (az_result_succeeded(AzureIoTHubClientProperties_GetNextComponentProperty(
 *       &xHubClient, &jr, xResponseType, AZ_IOT_HUB_CLIENT_PROPERTY_WRITABLE, &pucComponentName, &pusComponentNameLength)))
 * {
 *   // Check if property is of interest (substitute user_property for your own property name)
 *   if (AzureIoTJSONReader_TokenIsTextEqual(&jr, user_property, user_property_length))
 *   {
 *     AzureIoTJSONReader_NextToken(&jr);
 *
 *     // Get the property value here
 *     // Example: AzureIoTJSONReader_GetTokenInt32(&jr.token, &user_int);
 *
 *     // Skip to next property value
 *     AzureIoTJSONReader_NextToken(&jr);
 *   }
 *   else
 *   {
 *     // The JSON reader must be advanced regardless of whether the property
 *     // is of interest or not.
 *     AzureIoTJSONReader_NextToken(&jr);
 *
 *     // Skip children in case the property value is an object
 *     AzureIoTJSONReader_SkipChildren(&jr);
 *     AzureIoTJSONReader_NextToken(&jr);
 *   }
 * }
 *
 * @endcode
 *
 * @warning If you need to retrieve more than one \p xPropertyType, you should first complete the
 * scan of all components for the first property type (until the API returns
 * eAzureIoTErrorEndOfProperties). Then you must call AzureIoTJSONReader_Init() again after this call
 * and before the next call to `AzureIoTHubClientProperties_GetNextComponentProperty` with the
 * different \p xPropertyType.
 *
 * @param[in] pxAzureIoTHubClient The #AzureIoTHubClient_t to use for this call.
 * @param[in,out] pxJSONReader The #AzureIoTJSONReader_t to parse through. The ownership of iterating
 * through this json reader is shared between the user and this API.
 * @param[in] xResponseType The #AzureIoTHubMessageType_t representing the message
 * type associated with the payload.
 * @param[in] xPropertyType The #AzureIoTHubClientPropertyType_t to scan for.
 * @param[out] ppucComponentName The output component name for the property returned.
 * @param[out] pulComponentNameLength The length of the output \p ppucComponentName.
 *
 * @pre \p pxAzureIoTHubClient must not be `NULL`.
 * @pre \p pxJSONReader must not be `NULL`.
 * @pre \p xResponseType must be #eAzureIoTHubPropertiesRequestedMessage or #eAzureIoTHubPropertiesWritablePropertyMessage.
 * If eAzureIoTHubClientReportedFromDevice is specified in \p xPropertyType,
 * then \p xResponseType must be #eAzureIoTHubPropertiesRequestedMessage.
 * @pre \p ppucComponentName must not be `NULL`.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess If the function returned a valid #AzureIoTJSONReader_t pointing to the property name and
 * the component name.
 * @retval eAzureIoTErrorFailed If the json reader is passed in at an unexpected location.
 * @retval eAzureIoTErrorEndOfProperties If there are no more properties left for the component.
 */
AzureIoTResult_t AzureIoTHubClientProperties_GetNextComponentProperty( AzureIoTHubClient_t * pxAzureIoTHubClient,
                                                                       AzureIoTJSONReader_t * pxJSONReader,
                                                                       AzureIoTHubMessageType_t xResponseType,
                                                                       AzureIoTHubClientPropertyType_t xPropertyType,
                                                                       const uint8_t ** ppucComponentName,
                                                                       uint32_t * pulComponentNameLength );

#include "azure/core/_az_cfg_suffix.h"

#endif /*AZURE_IOT_HUB_CLIENT_PROPERTIES_H */
