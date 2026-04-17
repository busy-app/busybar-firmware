/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */

#ifndef DEMO_CONFIG_H
#define DEMO_CONFIG_H

/* FreeRTOS config include. */
#include "FreeRTOSConfig.h"

/*
 * This plug-and-play model can be found at:
 * https://github.com/Azure/iot-plugandplay-models/blob/main/dtmi/com/example/thermostat-1.json
 * This Model ID is tightly tied to the code implementation in `sample_azure_iot_pnp_simulated_device.c`
 * If you intend to test a different Model ID, please provide the implementation of the model on your application.
 */
#define sampleazureiotMODEL_ID "dtmi:com:example:Thermostat;1"

/**************************************************/
/******* DO NOT CHANGE the following order ********/
/**************************************************/

/* Include logging header files and define logging macros in the following order:
 * 1. Include the header file "logging_levels.h".
 * 2. Define the LIBRARY_LOG_NAME and LIBRARY_LOG_LEVEL macros depending on
 * the logging configuration for DEMO.
 * 3. Include the header file "logging_stack.h", if logging is enabled for DEMO.
 */

//#include "logging_levels.h"

/* Logging configuration for the Demo. */
#ifndef LIBRARY_LOG_NAME
#define LIBRARY_LOG_NAME "AzureIoTDemo"
#endif

#ifndef LIBRARY_LOG_LEVEL
#define LIBRARY_LOG_LEVEL LOG_INFO
#endif

/*
 * The function prints to the console before the network is connected;
 * then a UDP port after the network has connected. */
extern void vLoggingPrintf(const char *pcFormatString, ...);

/* Map the SdkLog macro to the logging function to enable logging */
#ifndef SdkLog
#define SdkLog(message) vLoggingPrintf message
#endif

//#include "logging_stack.h"

/************ End of logging configuration ****************/

/**
 * @brief IoTHub module Id.
 *
 * @note This is optional argument for IoTHub
 */
#define democonfigMODULE_ID ""

/**
 * @brief IoTHub hostname.
 *
 */
#define democonfigHOSTNAME "<YOUR IOT HUB HOSTNAME HERE>"

/**
 * @brief IoTHub device Id.
 *
 */
#define democonfigDEVICE_ID "<YOUR DEVICE ID HERE>"

//To use Symmetric Key set macro USE_SYMMETRIC_KEY value to 1 or else set as 0 to use x509 Key
#define USE_SYMMETRIC_KEY 1

#if USE_SYMMETRIC_KEY
/**
* @brief Device symmetric key
*
*/
#define democonfigDEVICE_SYMMETRIC_KEY "<Symmetric key>"

#else
/**
* @brief Client's X509 Certificate.
*
*/
#define democonfigCLIENT_CERTIFICATE_PEM "<YOUR DEVICE CERT HERE>"

/**
* @brief Client's private key.
*
*/
#define democonfigCLIENT_PRIVATE_KEY_PEM "<YOUR DEVICE PRIVATE KEY HERE>"
#endif

/**
 * @brief Size of the network buffer for MQTT packets.
 */
#define democonfigNETWORK_BUFFER_SIZE (2048U)

#endif /* DEMO_CONFIG_H */
