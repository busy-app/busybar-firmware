/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

#ifndef AZURE_IOT_CONFIG_DEFAULTS_H
#define AZURE_IOT_CONFIG_DEFAULTS_H

/**
 * @brief Default timeout used for generating SAS token.
 *
 */
#ifndef azureiotconfigDEFAULT_TOKEN_TIMEOUT_IN_SEC
    #define azureiotconfigDEFAULT_TOKEN_TIMEOUT_IN_SEC    ( 60 * 60U )
#endif

/**
 * @brief MQTT keep alive.
 *
 */
#ifndef azureiotconfigKEEP_ALIVE_TIMEOUT_SECONDS
    #define azureiotconfigKEEP_ALIVE_TIMEOUT_SECONDS    ( 60U )
#endif

/**
 * @brief Receive timeout for MQTT CONNACK.
 *
 */
#ifndef azureiotconfigCONNACK_RECV_TIMEOUT_MS
    #define azureiotconfigCONNACK_RECV_TIMEOUT_MS    ( 1000U )
#endif

/**
 * @brief Wait timeout of MQTT SUBACK.
 */
#ifndef azureiotconfigSUBACK_WAIT_INTERVAL_MS
    #define azureiotconfigSUBACK_WAIT_INTERVAL_MS    ( 10U )
#endif

/**
 * @brief Max MQTT username.
 */
#ifndef azureiotconfigUSERNAME_MAX
    #define azureiotconfigUSERNAME_MAX    ( 256U )
#endif

/**
 * @brief Max MQTT password.
 */
#ifndef azureiotconfigPASSWORD_MAX
    #define azureiotconfigPASSWORD_MAX    ( 256U )
#endif

/**
 * @brief Max MQTT topic length.
 */
#ifndef azureiotconfigTOPIC_MAX
    #define azureiotconfigTOPIC_MAX    ( 128U )
#endif

/**
 * @brief Max provisioning response payload supported.
 *
 */
#ifndef azureiotconfigPROVISIONING_REQUEST_PAYLOAD_MAX
    #define azureiotconfigPROVISIONING_REQUEST_PAYLOAD_MAX    ( 512U )
#endif

/**
 * @brief Provisioning polling interval.
 *
 * @details This is used for cases where the service does not supply a retry-after hint during the
 *          register and query operations.
 */
#ifndef azureiotconfigPROVISIONING_POLLING_INTERVAL_S
    #define azureiotconfigPROVISIONING_POLLING_INTERVAL_S    ( 3U )
#endif

/**
 * @brief Macro that is called in the Azure IoT middleware library for logging "Error" level
 * messages.
 *
 * To enable error level logging in the AzureIoT middleware library:
 *  - Map the macro to the application-specific logging implementation that supports error logging.
 *
 * @note This logging macro is called in the Azure IoT middleware library with parameters wrapped in
 * double parentheses to be ISO C89/C90 standard compliant.
 *
 * <b>Default value</b>: Error logging is turned off, and no code is generated for calls
 * to the macro in the Azure IoT middleware library on compilation.
 */
#ifndef AZLogError
    #define AZLogError( message )
#endif

/**
 * @brief Macro that is called in the Azure IoT middleware library for logging "Warning" level
 * messages.
 *
 * To enable warning level logging in the AzureIoT middleware library:
 *  - Map the macro to the application-specific logging implementation that supports error logging.
 *
 * @note This logging macro is called in the Azure IoT middleware library with parameters wrapped in
 * double parentheses to be ISO C89/C90 standard compliant.
 *
 * <b>Default value</b>: Warning logs are turned off, and no code is generated for calls
 * to the macro in the Azure IoT middleware library on compilation.
 */
#ifndef AZLogWarn
    #define AZLogWarn( message )
#endif

/**
 * @brief Macro that is called in the Azure IoT middleware library for logging "Info" level
 * messages.
 *
 * To enable info level logging in the AzureIoT middleware library:
 *  - Map the macro to the application-specific logging implementation that supports error logging.
 *
 * @note This logging macro is called in the Azure IoT middleware library with parameters wrapped in
 * double parentheses to be ISO C89/C90 standard compliant.
 *
 * <b>Default value</b>: Info logging is turned off, and no code is generated for calls
 * to the macro in the Azure IoT middleware library on compilation.
 */
#ifndef AZLogInfo
    #define AZLogInfo( message )
#endif

/**
 * @brief Macro that is called in the Azure IoT middleware library for logging "Debug" level
 * messages.
 *
 * To enable info level logging in the AzureIoT middleware library:
 *  - Map the macro to the application-specific logging implementation that supports error logging.
 *
 * @note This logging macro is called in the Azure IoT middleware library with parameters wrapped in
 * double parentheses to be ISO C89/C90 standard compliant.
 *
 * <b>Default value</b>: Debug logging is turned off, and no code is generated for calls
 * to the macro in the Azure IoT middleware library on compilation.
 */
#ifndef AZLogDebug
    #define AZLogDebug( message )
#endif

#endif /* AZURE_IOT_CONFIG_DEFAULTS_H */
