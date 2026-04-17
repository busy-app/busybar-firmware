/***************************************************************************//**
 * # License
 * <b> Copyright 2025 Silicon Laboratories Inc. www.silabs.com </b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of the Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * https://www.silabs.com/about-us/legal/master-software-license-agreement
 * By installing, copying or otherwise using this software, you agree to the
 * terms of the MSLA.
 *
 ******************************************************************************/

#ifndef _ZW_LOG_CONFIG_H_
#define _ZW_LOG_CONFIG_H_

// <<< Use Configuration Wizard in Context Menu >>>

// <h> General

// <o ZW_LOG_BUFFER_SIZE> Size of the log buffer <1..255:1> <f.d>
// <i> Size (in bytes) of the buffer reserved for log messages
// <i> Default: 96
#define ZW_LOG_BUFFER_SIZE  96

// <q ZW_LOG_DISPLAY_LEVEL> Display log level
// <i> If enabled, log messages will be prefixed with their level
// <i> Default: 1
#define ZW_LOG_DISPLAY_LEVEL  1

// <q ZW_LOG_DISPLAY_COMPONENT> Display log source component
// <i> If enabled, log messages will be prefixed with the ID of their originating component
// <i> Default: 0
#define ZW_LOG_DISPLAY_COMPONENT  0

// <q ZW_LOG_DISPLAY_TIMESTAMP> Display log timestamp
// <i> If enabled, log messages will be prefixed with the timestamp based on the FreeRTOS tick count
// <i> Default: 0
#define ZW_LOG_DISPLAY_TIMESTAMP  0

// </h>
// <h> Output channels

// <s ZW_LOG_CHANNEL_DEBUG> IO Stream instance for Debug messages
// <i> rtt, vcom or any custom user-created instance
// <d> ""
#define ZW_LOG_CHANNEL_DEBUG  ""

// <s ZW_LOG_CHANNEL_INFO> IO Stream instance for Info messages
// <i> rtt, vcom or any custom user-created instance
// <d> "vcom"
#define ZW_LOG_CHANNEL_INFO  "vcom"

// <s ZW_LOG_CHANNEL_WARNING> IO Stream instance for Warning messages
// <i> rtt, vcom or any custom user-created instance
// <d> ""
#define ZW_LOG_CHANNEL_WARNING  ""

// <s ZW_LOG_CHANNEL_ERROR> IO Stream instance for Error messages
// <i> rtt, vcom or any custom user-created instance
// <d> ""
#define ZW_LOG_CHANNEL_ERROR  ""

// </h>
// <h> Component filtering

// <q ZW_LOG_ENABLE_APP> Application
// <i> If enabled, log messages from the application will be displayed
// <i> Default: 1
#define ZW_LOG_ENABLE_APP  1

// <q ZW_LOG_ENABLE_HW> Hardware
// <i> If enabled, log messages from the platform hardware layer will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_HW  0

// <q ZW_LOG_ENABLE_CC_ASSOCIATION> CC Association
// <i> If enabled, log messages from the Association command class will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_CC_ASSOCIATION  0

// <q ZW_LOG_ENABLE_CC_BATTERY> CC Battery
// <i> If enabled, log messages from the Battery command class will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_CC_BATTERY  0

// <q ZW_LOG_ENABLE_CC_BINARY_SWITCH> CC Binary Switch
// <i> If enabled, log messages from the Binary Switch command class will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_CC_BINARY_SWITCH  0

// <q ZW_LOG_ENABLE_CC_CENTRAL_SCENE> CC Central Scene
// <i> If enabled, log messages from the Central Scene command class will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_CC_CENTRAL_SCENE  0

// <q ZW_LOG_ENABLE_CC_COLOR_SWITCH> CC Color Switch
// <i> If enabled, log messages from the Color Switch command class will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_CC_COLOR_SWITCH  0

// <q ZW_LOG_ENABLE_CC_DOOR_LOCK> CC Door Lock
// <i> If enabled, log messages from the Door Lock command class will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_CC_DOOR_LOCK  0

// <q ZW_LOG_ENABLE_CC_FIRMWARE_UPDATE> CC Firmware Update
// <i> If enabled, log messages from the Firmware Update command class will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_CC_FIRMWARE_UPDATE  0

// <q ZW_LOG_ENABLE_CC_INDICATOR> CC Indicator
// <i> If enabled, log messages from the Indicator command class will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_CC_INDICATOR  0

// <q ZW_LOG_ENABLE_CC_MULTI_CHANNEL> CC Multi Channel
// <i> If enabled, log messages from the Multi Channel command class will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_CC_MULTI_CHANNEL  0

// <q ZW_LOG_ENABLE_CC_MULTILEVEL_SENSOR> CC Multilevel Sensor
// <i> If enabled, log messages from the Multilevel Sensor command class will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_CC_MULTILEVEL_SENSOR  0

// <q ZW_LOG_ENABLE_CC_MULTILEVEL_SWITCH> CC Multilevel Switch
// <i> If enabled, log messages from the Multilevel Switch command class will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_CC_MULTILEVEL_SWITCH  0

// <q ZW_LOG_ENABLE_CC_NOTIFICATION> CC Notification
// <i> If enabled, log messages from the Notification command class will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_CC_NOTIFICATION  0

// <q ZW_LOG_ENABLE_CC_USER_CODE> CC User Code
// <i> If enabled, log messages from the User Code command class will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_CC_USER_CODE  0

// <q ZW_LOG_ENABLE_CC_USER_CREDENTIAL> CC User Credential
// <i> If enabled, log messages from the User Credential command class will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_CC_USER_CREDENTIAL  0

// <q ZW_LOG_ENABLE_CC_VERSION> CC Version
// <i> If enabled, log messages from the Version command class will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_CC_VERSION  0

// <q ZW_LOG_ENABLE_CC_WAKE_UP> CC Wake Up
// <i> If enabled, log messages from the Wake Up command class will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_CC_WAKE_UP  0

// <q ZW_LOG_ENABLE_CC_ZWAVE_PLUS_INFO> CC Z-Wave Plus Info
// <i> If enabled, log messages from the Z-Wave Plus Info command class will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_CC_ZWAVE_PLUS_INFO  0

// <q ZW_LOG_ENABLE_ZAF_ACTUATOR> ZAF Actuator
// <i> If enabled, log messages from the ZAF Actuator will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_ZAF_ACTUATOR  0

// <q ZW_LOG_ENABLE_ZAF_APP_TIMER> ZAF App Timer
// <i> If enabled, log messages from the ZAF App Timer will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_ZAF_APP_TIMER  0

// <q ZW_LOG_ENABLE_ZAF_CMD_PUBLISHER> ZAF Command Publisher
// <i> If enabled, log messages from the ZAF Command Publisher will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_ZAF_CMD_PUBLISHER  0

// <q ZW_LOG_ENABLE_ZAF_COMMON> ZAF Common
// <i> If enabled, log messages from the ZAF Common will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_ZAF_COMMON  0

// <q ZW_LOG_ENABLE_ZAF_EVENT_DISTRIBUTOR> ZAF Event Distributor
// <i> If enabled, log messages from the ZAF Event Distributor will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_ZAF_EVENT_DISTRIBUTOR  0

// <q ZW_LOG_ENABLE_ZAF_NVM> ZAF NVM
// <i> If enabled, log messages from the ZAF NVM will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_ZAF_NVM  0

// <q ZW_LOG_ENABLE_ZAF_TRANSPORT> ZAF Transport
// <i> If enabled, log messages from the ZAF Transport will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_ZAF_TRANSPORT  0

// <q ZW_LOG_ENABLE_ZAF_TSE> ZAF TSE
// <i> If enabled, log messages from the ZAF TSE will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_ZAF_TSE  0

// <q ZW_LOG_ENABLE_SHUTDOWN_MANAGER> shutdown manager
// <i> If enabled, log messages from the shutdown manager will be displayed
// <i> Default: 0
#define ZW_LOG_ENABLE_SHUTDOWN_MANAGER  0

// </h>

// <<< end of configuration section >>>

#endif /* _ZW_LOG_CONFIG_H_ */
