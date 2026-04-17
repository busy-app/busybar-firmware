/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot_crypto.h
 *
 * @brief The port file for crypto APIs
 *
 * Used for verifying the ADU image payload.
 *
 */

#include <stdint.h>

#include "azure_iot_result.h"

AzureIoTResult_t AzureIoTCrypto_SHA256Calculate( const char * pucInputPtr,
                                                 uint64_t ulInputSize,
                                                 const char * pucOutputPtr,
                                                 uint64_t ulOutputSize );

AzureIoTResult_t AzureIoTCrypto_RS256Verify( const char * pucInputPtr,
                                             uint64_t ulInputSize,
                                             const char * pucSignaturePtr,
                                             uint64_t ulSignatureSize,
                                             const char * pucN,
                                             uint64_t ullNSize,
                                             const char * pucE,
                                             uint64_t ullESize,
                                             const char * pucBufferPtr,
                                             uint32_t ulBufferSize );
