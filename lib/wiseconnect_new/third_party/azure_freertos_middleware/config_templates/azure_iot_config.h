/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

#ifndef AZURE_IOT_CONFIG_H
#define AZURE_IOT_CONFIG_H

/**
 * 
 * Configuring middleware to use custom logging
 * 
 * Note: below mapping assumes user providing LogE,
 *  LogW, LogI and LogD implementation.
 * 
 * */
// 
// #define AZLogError( message )    LogE( message )
// #define AZLogWarn( message )     LogW( message )
// #define AZLogInfo( message )     LogI( message )
// #define AZLogDebug( message )    LogD( message )
//

/**
 * 
 * Configuring middleware to use FreeRTOS logging
 * 
 * */
// #include "logging_levels.h"
// 
// #ifndef LIBRARY_LOG_NAME
//     #define LIBRARY_LOG_NAME    "AZ IOT"
// #endif
// 
// #ifndef LIBRARY_LOG_LEVEL
//     #define LIBRARY_LOG_LEVEL    LOG_INFO
// #endif
// 
// extern void vLoggingPrintf( const char * pcFormatString,
//                             ... );
// 
// #ifndef SdkLog
//     #define SdkLog( message )    vLoggingPrintf message
// #endif
// 
// #include "logging_stack.h"

/**
 * @brief Default timeout used for generating SAS token.
 *
 */
// #define azureiotconfigDEFAULT_TOKEN_TIMEOUT_IN_SEC    ( 60 * 60U )

/**
 * @brief MQTT keep alive.
 *
 */
// #define azureiotconfigKEEP_ALIVE_TIMEOUT_SECONDS    ( 60U )

/**
 * @brief Receive timeout for MQTT CONNACK.
 *
 */
// #define azureiotconfigCONNACK_RECV_TIMEOUT_MS    ( 1000U )

/**
 * @brief Wait timeout of MQTT SUBACK.
 */
// #define azureiotconfigSUBACK_WAIT_INTERVAL_MS    ( 10U )

/**
 * @brief Max MQTT username.
 */
// #define azureiotconfigUSERNAME_MAX    ( 256U )

/**
 * @brief Max MQTT password.
 */
// #define azureiotconfigPASSWORD_MAX    ( 256U )

/**
 * @brief Max MQTT topic length.
 */
// #define azureiotconfigTOPIC_MAX    ( 128U )

/**
 * @brief Max provisioning response payload supported.
 *
 */
// #define azureiotconfigPROVISIONING_REQUEST_PAYLOAD_MAX    ( 512U )

#endif /* AZURE_IOT_CONFIG_H */
