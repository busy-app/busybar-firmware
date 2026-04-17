/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot_json_writer.h
 *
 * @brief The JSON writer used by the middleware for PnP properties.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 *
 */

#ifndef AZURE_IOT_JSON_WRITER_H
#define AZURE_IOT_JSON_WRITER_H

#include <stdbool.h>
#include <stdint.h>

#include "azure_iot_result.h"

/* Azure SDK for Embedded C includes */
#include "azure/core/az_json.h"
#include "azure/core/_az_cfg_prefix.h"

/**
 * @brief The struct to use for Azure IoT JSON writer functionality.
 */
typedef struct AzureIoTJSONWriter
{
    struct
    {
        az_json_writer xCoreWriter;
    } _internal; /**< @brief Internal to the SDK */
} AzureIoTJSONWriter_t;

/**
 * @brief Initializes an #AzureIoTJSONWriter_t which writes JSON text into a buffer passed.
 *
 * @param[out] pxWriter A pointer to an #AzureIoTJSONWriter_t the instance to initialize.
 * @param[in] pucBuffer A buffer pointer to which JSON text will be written.
 * @param[in] ulBufferSize Length of buffer.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess Successfully initialized JSON writer.
 */
AzureIoTResult_t AzureIoTJSONWriter_Init( AzureIoTJSONWriter_t * pxWriter,
                                          uint8_t * pucBuffer,
                                          uint32_t ulBufferSize );

/**
 * @brief Appends the UTF-8 property name and value where value is int32
 *
 * @param[in] pxWriter A pointer to an #AzureIoTJSONWriter_t.
 * @param[in] pucPropertyName The UTF-8 encoded property name of the JSON value to be written. The name is
 * escaped before writing.
 * @param[in] ulPropertyNameLength Length of pucPropertyName.
 * @param[in] lValue The value to be written as a JSON number.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The property name and int32 value was appended successfully.
 */
AzureIoTResult_t AzureIoTJSONWriter_AppendPropertyWithInt32Value( AzureIoTJSONWriter_t * pxWriter,
                                                                  const uint8_t * pucPropertyName,
                                                                  uint32_t ulPropertyNameLength,
                                                                  int32_t lValue );

/**
 * @brief Appends the UTF-8 property name and value where value is double
 *
 * @param[in] pxWriter A pointer to an #AzureIoTJSONWriter_t.
 * @param[in] pucPropertyName The UTF-8 encoded property name of the JSON value to be written. The name is
 * escaped before writing.
 * @param[in] ulPropertyNameLength Length of pucPropertyName.
 * @param[in] xValue The value to be written as a JSON number.
 * @param[in] usFractionalDigits The number of digits of the value to write after the decimal point and truncate the rest.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The property name and double value was appended successfully.
 */
AzureIoTResult_t AzureIoTJSONWriter_AppendPropertyWithDoubleValue( AzureIoTJSONWriter_t * pxWriter,
                                                                   const uint8_t * pucPropertyName,
                                                                   uint32_t ulPropertyNameLength,
                                                                   double xValue,
                                                                   uint16_t usFractionalDigits );

/**
 * @brief Appends the UTF-8 property name and value where value is boolean
 *
 * @param[in] pxWriter A pointer to an #AzureIoTJSONWriter_t.
 * @param[in] pucPropertyName The UTF-8 encoded property name of the JSON value to be written. The name is
 * escaped before writing.
 * @param[in] ulPropertyNameLength Length of pucPropertyName.
 * @param[in] usValue The value to be written as a JSON literal `true` or `false`.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The property name and bool value was appended successfully.
 */
AzureIoTResult_t AzureIoTJSONWriter_AppendPropertyWithBoolValue( AzureIoTJSONWriter_t * pxWriter,
                                                                 const uint8_t * pucPropertyName,
                                                                 uint32_t ulPropertyNameLength,
                                                                 bool usValue );

/**
 * @brief Appends the UTF-8 property name and value where value is string
 *
 * @param[in] pxWriter A pointer to an #AzureIoTJSONWriter_t.
 * @param[in] pucPropertyName The UTF-8 encoded property name of the JSON value to be written. The name is
 * escaped before writing.
 * @param[in] ulPropertyNameLength Length of pucPropertyName.
 * @param[in] pucValue The UTF-8 encoded property name of the JSON value to be written. The name is
 * escaped before writing.
 * @param[in] ulValueLen Length of value.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The property name and string value was appended successfully.
 */
AzureIoTResult_t AzureIoTJSONWriter_AppendPropertyWithStringValue( AzureIoTJSONWriter_t * pxWriter,
                                                                   const uint8_t * pucPropertyName,
                                                                   uint32_t ulPropertyNameLength,
                                                                   const uint8_t * pucValue,
                                                                   uint32_t ulValueLen );

/**
 * @brief Returns the length containing the JSON text written to the underlying buffer.
 *
 * @param[in] pxWriter A pointer to an #AzureIoTJSONWriter_t.
 *
 * @return An int32_t containing the length of JSON text built so far. Will return -1 if there was an error.
 */
int32_t AzureIoTJSONWriter_GetBytesUsed( AzureIoTJSONWriter_t * pxWriter );

/**
 * @brief Appends the UTF-8 text value (as a JSON string) into the buffer.
 *
 * @param[in] pxWriter A pointer to an #AzureIoTJSONWriter_t.
 * @param[in] pucValue Pointer of UCHAR buffer that contains UTF-8 encoded value to be written as a JSON string.
 * The value is escaped before writing.
 * @param[in] ulValueLen Length of value.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The string value was appended successfully.
 */
AzureIoTResult_t AzureIoTJSONWriter_AppendString( AzureIoTJSONWriter_t * pxWriter,
                                                  const uint8_t * pucValue,
                                                  uint32_t ulValueLen );

/**
 * @brief Appends an existing UTF-8 encoded JSON text into the buffer, useful for appending nested
 * JSON.
 *
 * @param[in] pxWriter A pointer to an #AzureIoTJSONWriter_t.
 * @param[in] pucJSON A pointer to single, possibly nested, valid, UTF-8 encoded, JSON value to be written as
 * is, without any formatting or spacing changes. No modifications are made to this text, including
 * escaping.
 * @param[in] ulJSONLen Length of `pucJSON`.
 *
 * @remarks A single, possibly nested, JSON value is one that starts and ends with {} or [] or is a
 * single primitive token. The JSON cannot start with an end object or array, or a property name, or
 * be incomplete.
 *
 * @remarks The function validates that the provided JSON to be appended is valid and properly
 * escaped, and fails otherwise.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The provided json_text was appended successfully.
 */
AzureIoTResult_t AzureIoTJSONWriter_AppendJSONText( AzureIoTJSONWriter_t * pxWriter,
                                                    const uint8_t * pucJSON,
                                                    uint32_t ulJSONLen );

/**
 * @brief Appends the UTF-8 property name (as a JSON string) which is the first part of a name/value
 * pair of a JSON object.
 *
 * @param[in] pxWriter A pointer to an #AzureIoTJSONWriter_t.
 * @param[in] pusValue The UTF-8 encoded property name of the JSON value to be written. The name is
 * escaped before writing.
 * @param[in] ulValueLen Length of name.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The property name was appended successfully.
 */
AzureIoTResult_t AzureIoTJSONWriter_AppendPropertyName( AzureIoTJSONWriter_t * pxWriter,
                                                        const uint8_t * pusValue,
                                                        uint32_t ulValueLen );

/**
 * @brief Appends a boolean value (as a JSON literal `true` or `false`).
 *
 * @param[in] pxWriter A pointer to an #AzureIoTJSONWriter_t.
 * @param[in] xValue The value to be written as a JSON literal `true` or `false`.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The bool was appended successfully.
 */
AzureIoTResult_t AzureIoTJSONWriter_AppendBool( AzureIoTJSONWriter_t * pxWriter,
                                                bool xValue );

/**
 * @brief Appends an `int32_t` number value.
 *
 * @param[in] pxWriter A pointer to an #AzureIoTJSONWriter_t.
 * @param[in] lValue The value to be written as a JSON number.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The number was appended successfully.
 */
AzureIoTResult_t AzureIoTJSONWriter_AppendInt32( AzureIoTJSONWriter_t * pxWriter,
                                                 int32_t lValue );

/**
 * @brief Appends a `double` number value.
 *
 * @param[in] pxWriter A pointer to an #AzureIoTJSONWriter_t.
 * @param[in] xValue The value to be written as a JSON number.
 * @param[in] usFractionalDigits The number of digits of the \p value to write after the decimal
 * point and truncate the rest.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The number was appended successfully.
 *
 * @remark Only finite double values are supported. Values such as `NAN` and `INFINITY` are not
 * allowed and would lead to invalid JSON being written.
 *
 * @remark Non-significant trailing zeros (after the decimal point) are not written, even if \p
 * usFractionalDigits is large enough to allow the zero padding.
 *
 * @remark The \p usFractionalDigits must be between 0 and 15 (inclusive). Any value passed in that
 * is larger will be clamped down to 15.
 */
AzureIoTResult_t AzureIoTJSONWriter_AppendDouble( AzureIoTJSONWriter_t * pxWriter,
                                                  double xValue,
                                                  uint16_t usFractionalDigits );

/**
 * @brief Appends the JSON literal `null`.
 *
 * @param[in] pxWriter A pointer to an #AzureIoTJSONWriter_t.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess `null` was appended successfully.
 */
AzureIoTResult_t AzureIoTJSONWriter_AppendNull( AzureIoTJSONWriter_t * pxWriter );

/**
 * @brief Appends the beginning of a JSON object (i.e. `{`).
 *
 * @param[in] pxWriter A pointer to an #AzureIoTJSONWriter_t.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess Object start was appended successfully.
 */
AzureIoTResult_t AzureIoTJSONWriter_AppendBeginObject( AzureIoTJSONWriter_t * pxWriter );

/**
 * @brief Appends the beginning of a JSON array (i.e. `[`).
 *
 * @param[in] pxWriter A pointer to an #AzureIoTJSONWriter_t.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess Array start was appended successfully.
 */
AzureIoTResult_t AzureIoTJSONWriter_AppendBeginArray( AzureIoTJSONWriter_t * pxWriter );

/**
 * @brief Appends the end of the current JSON object (i.e. `}`).
 *
 * @param[in] pxWriter A pointer to an #AzureIoTJSONWriter_t.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess Object end was appended successfully.
 */
AzureIoTResult_t AzureIoTJSONWriter_AppendEndObject( AzureIoTJSONWriter_t * pxWriter );

/**
 * @brief Appends the end of the current JSON array (i.e. `]`).
 *
 * @param[in] pxWriter A pointer to an #AzureIoTJSONWriter_t.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess Array end was appended successfully.
 */
AzureIoTResult_t AzureIoTJSONWriter_AppendEndArray( AzureIoTJSONWriter_t * pxWriter );

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZURE_IOT_JSON_WRITER_H */
