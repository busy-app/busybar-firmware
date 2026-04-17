/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot_json_reader.h
 *
 * @brief The JSON reader used by the middleware for PnP properties.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 *
 */

#ifndef AZURE_IOT_JSON_READER_H
#define AZURE_IOT_JSON_READER_H

#include <stdbool.h>
#include <stdint.h>

#include "azure_iot_result.h"

/* Azure SDK for Embedded C includes */
#include "azure/core/az_json.h"
#include "azure/core/_az_cfg_prefix.h"

/**
 * @brief Defines symbols for the various kinds of JSON tokens that make up any JSON text.
 */
typedef enum AzureIoTJSONTokenType
{
    eAzureIoTJSONTokenNONE = AZ_JSON_TOKEN_NONE,
    eAzureIoTJSONTokenBEGIN_OBJECT = AZ_JSON_TOKEN_BEGIN_OBJECT,
    eAzureIoTJSONTokenEND_OBJECT = AZ_JSON_TOKEN_END_OBJECT,
    eAzureIoTJSONTokenBEGIN_ARRAY = AZ_JSON_TOKEN_BEGIN_ARRAY,
    eAzureIoTJSONTokenEND_ARRAY = AZ_JSON_TOKEN_END_ARRAY,
    eAzureIoTJSONTokenPROPERTY_NAME = AZ_JSON_TOKEN_PROPERTY_NAME,
    eAzureIoTJSONTokenSTRING = AZ_JSON_TOKEN_STRING,
    eAzureIoTJSONTokenNUMBER = AZ_JSON_TOKEN_NUMBER,
    eAzureIoTJSONTokenTRUE = AZ_JSON_TOKEN_TRUE,
    eAzureIoTJSONTokenFALSE = AZ_JSON_TOKEN_FALSE,
    eAzureIoTJSONTokenNULL = AZ_JSON_TOKEN_NULL
} AzureIoTJSONTokenType_t;

/**
 * @brief The struct to use for Azure IoT JSON reader functionality.
 */
typedef struct AzureIoTJSONReader
{
    struct
    {
        az_json_reader xCoreReader;
    } _internal; /**< @brief Internal to the SDK */
} AzureIoTJSONReader_t;

/**
 * @brief Initializes an #AzureIoTJSONReader_t to read the JSON payload contained within the provided
 * buffer.
 *
 * @param[out] pxReader A pointer to an #AzureIoTJSONReader_t instance to initialize.
 * @param[in] pucBuffer A pointer to a buffer containing the JSON text to read.
 * @param[in] ulBufferSize Length of buffer.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The #AzureIoTJSONReader_t is initialized successfully.
 * @retval other Initialization failed.
 *
 */
AzureIoTResult_t AzureIoTJSONReader_Init( AzureIoTJSONReader_t * pxReader,
                                          const uint8_t * pucBuffer,
                                          uint32_t ulBufferSize );

/**
 * @brief Reads the next token in the JSON text and updates the reader state.
 *
 * @param[in] pxReader A pointer to an #AzureIoTJSONReader_t instance containing the JSON to
 * read.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The token was read successfully.
 */
AzureIoTResult_t AzureIoTJSONReader_NextToken( AzureIoTJSONReader_t * pxReader );

/**
 * @brief Reads and skips over any nested JSON elements.
 *
 * @param[in] pxReader A pointer to an #AzureIoTJSONReader_t instance containing the JSON to
 * read.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The children of the current JSON token are skipped successfully.
 *
 * @remarks If the current token kind is a property name, the reader first moves to the property
 * value. Then, if the token kind is start of an object or array, the reader moves to the matching
 * end object or array. For all other token kinds, the reader doesn't move and returns eAzureIoTSuccess.
 */
AzureIoTResult_t AzureIoTJSONReader_SkipChildren( AzureIoTJSONReader_t * pxReader );

/**
 * @brief Gets the JSON token's boolean value.
 *
 * @param[in] pxReader A pointer to an #AzureIoTJSONReader_t instance.
 * @param[out] pxValue A pointer to a boolean to receive the value.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The boolean value is returned.
 */
AzureIoTResult_t AzureIoTJSONReader_GetTokenBool( AzureIoTJSONReader_t * pxReader,
                                                  bool * pxValue );

/**
 * @brief Gets the JSON token's number as a 32-bit signed integer.
 *
 * @param[in] pxReader A pointer to an #AzureIoTJSONReader_t instance.
 * @param[out] plValue A pointer to a variable to receive the value.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The number is returned.
 */
AzureIoTResult_t AzureIoTJSONReader_GetTokenInt32( AzureIoTJSONReader_t * pxReader,
                                                   int32_t * plValue );

/**
 * @brief Gets the JSON token's number as a `double`.
 *
 * @param[in] pxReader A pointer to an #AzureIoTJSONReader_t instance.
 * @param[out] pxValue A pointer to a variable to receive the value.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The number is returned.
 */
AzureIoTResult_t AzureIoTJSONReader_GetTokenDouble( AzureIoTJSONReader_t * pxReader,
                                                    double * pxValue );

/**
 * @brief Gets the JSON token's string after unescaping it, if required.
 *
 * @param[in] pxReader A pointer to an #AzureIoTJSONReader_t instance.
 * @param[out] pucBuffer A pointer to a buffer where the string should be copied into.
 * @param[in] ulBufferSize The maximum available space within the buffer referred to by pucBuffer.
 * @param[out] pusBytesCopied Contains the number of bytes written to the \p
 * destination which denote the length of the unescaped string.
 *
 * @return An #AzureIoTResult_t value indicating the result of the operation.
 * @retval eAzureIoTSuccess The property name was appended successfully.
 */
AzureIoTResult_t AzureIoTJSONReader_GetTokenString( AzureIoTJSONReader_t * pxReader,
                                                    uint8_t * pucBuffer,
                                                    uint32_t ulBufferSize,
                                                    uint32_t * pusBytesCopied );

/**
 * @brief Determines whether the unescaped JSON token value that the #AzureIoTJSONReader_t points to is
 * equal to the expected text within the provided buffer bytes by doing a case-sensitive comparison.
 *
 * @param[in] pxReader A pointer to an #AzureIoTJSONReader_t instance containing the JSON string token.
 * @param[in] pucExpectedText A pointer to lookup text to compare the token against.
 * @param[in] ulExpectedTextLength Length of \p pucExpectedText.
 *
 * @return true if the current JSON token value in the JSON source semantically matches the
 * expected lookup text, with the exact casing; otherwise, false.
 *
 * @remarks This operation is only valid for the string and property name token kinds. For all other
 * token kinds, it returns false.
 */
bool AzureIoTJSONReader_TokenIsTextEqual( AzureIoTJSONReader_t * pxReader,
                                          const uint8_t * pucExpectedText,
                                          uint32_t ulExpectedTextLength );

/**
 * @brief Determines type of token currently #AzureIoTJSONReader_t points to.
 *
 * @param[in] pxReader A pointer to an #AzureIoTJSONReader_t instance.
 * @param[out] pxTokenType The returned type of the token.
 *
 * @return An #AzureIoTResult_t value indicating the type of token.
 */
AzureIoTResult_t AzureIoTJSONReader_TokenType( AzureIoTJSONReader_t * pxReader,
                                               AzureIoTJSONTokenType_t * pxTokenType );

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZURE_IOT_JSON_READER_H */
