/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

/**
 * @file azure_iot_version.h
 *
 * @brief Provides version information for the Azure IoT FreeRTOS middleware.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef AZURE_IOT_VERSION_H
#define AZURE_IOT_VERSION_H

#define _azureiotSTRINGIFY2( x )    # x                      /**< @brief Internal */
#define _azureiotSTRINGIFY( x )     _azureiotSTRINGIFY2( x ) /**< @brief Internal */

/**
 * @brief Major numeric identifier.
 */
#define azureiotVERSION_MAJOR         1

/**
 * @brief Minor numeric identifier.
 */
#define azureiotVERSION_MINOR         1

/**
 * @brief Patch numeric identifier.
 */
#define azureiotVERSION_PATCH         0

/**
 * @brief Optional pre-release identifier. SDK is in a pre-release state when present.
 */
#define azureiotVERSION_PRERELEASE    ""

/**
 * @brief The version in string format used for telemetry following the `semver.org` standard
 * (https://semver.org).
 */
#define azureiotVERSION_STRING                      \
    _azureiotSTRINGIFY( azureiotVERSION_MAJOR ) "." \
    _azureiotSTRINGIFY( azureiotVERSION_MINOR ) "." \
    _azureiotSTRINGIFY( azureiotVERSION_PATCH )     \
    azureiotVERSION_PRERELEASE

#endif /* AZURE_IOT_VERSION_H */
