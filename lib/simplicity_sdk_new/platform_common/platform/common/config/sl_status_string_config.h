/***************************************************************************//**
 * @file sl_status_string_config.h
 * @brief Status String Functions Configuration - Configuration Template File
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

/*********************************************************************************************************
 *********************************************************************************************************
 *                                               MODULE
 ********************************************************************************************************
 *******************************************************************************************************/

#ifndef  SL_STATUS_STRING_CONFIG_H
#define  SL_STATUS_STRING_CONFIG_H

// <h> SL_STATUS string components Configuration

/********************************************************************************************************
********************************************************************************************************
*                                             STRING COMPONENTS
*********************************************************************************************************
********************************************************************************************************/
// <q SL_STATUS_STRING_ENABLE_GENERIC> Enable generic string
// <i> Enable (1) or disable (0) generic string components.
// <i> Default: 1
#define  SL_STATUS_STRING_ENABLE_GENERIC                             1

// <q SL_STATUS_STRING_ENABLE_BLUETOOTH> Enable Bluetooth string components
// <i> Enable (1) or disable (0) Bluetooth string components.
// <i> Default: 1
#define  SL_STATUS_STRING_ENABLE_BLUETOOTH                           1

// <q SL_STATUS_STRING_ENABLE_WIFI> Enable WiFi string components
// <i> Enable (1) or disable (0) Wifi string components.
// <i> Default: 1
#define  SL_STATUS_STRING_ENABLE_WIFI                                1

// <q SL_STATUS_STRING_ENABLE_COMPUTE> Enable Compute (MVP Driver & MVP Math) string components
// <i> Enable (1) or disable (0) MVP Driver/MVP Math string components.
// <i> Default: 1
#define  SL_STATUS_STRING_ENABLE_COMPUTE                             1

// <q SL_STATUS_STRING_ENABLE_ZIGBEE> Enable Zigbee status string components
// <i> Enable (1) or disable (0) Zigbee string components.
// <i> Default: 1
#define  SL_STATUS_STRING_ENABLE_ZIGBEE                              1

// <q SL_STATUS_STRING_ENABLE_PLATFORM> Enable Platform status string components
// <i> Enable (1) or disable (0) Platform string components.
// <i> Default: 0
#define  SL_STATUS_STRING_ENABLE_PLATFORM                            0

// <q SL_STATUS_STRING_ENABLE_HARDWARE> Enable Hardware status string components
// <i> Enable (1) or disable (0) Hardware string components.
// <i> Default: 0
#define  SL_STATUS_STRING_ENABLE_HARDWARE                            0

// <q SL_STATUS_STRING_ENABLE_CAN_CANOPEN> Enable CAN & CANopen status string components
// <i> Enable (1) or disable (0) CAN & CANopen string components.
// <i> Default: 0
#define  SL_STATUS_STRING_ENABLE_CAN_CANOPEN                         0

// <q SL_STATUS_STRING_ENABLE_CONNECT> Enable Connect status string components
// <i> Enable (1) or disable (0) Connect string components.
// <i> Default: 0
#define  SL_STATUS_STRING_ENABLE_CONNECT                             0

// <q SL_STATUS_STRING_ENABLE_NET_SUITE> Enable Net Suite status string components
// <i> Enable (1) or disable (0) Net Suite string components.
// <i> Default: 0
#define  SL_STATUS_STRING_ENABLE_NET_SUITE                           0

// <q SL_STATUS_STRING_ENABLE_THREAD> Enable Thread status string components
// <i> Enable (1) or disable (0) Thread string components.
// <i> Default: 0
#define  SL_STATUS_STRING_ENABLE_THREAD                              0

// <q SL_STATUS_STRING_ENABLE_USB> Enable USB status string components
// <i> Enable (1) or disable (0) USB string components.
// <i> Default: 0
#define  SL_STATUS_STRING_ENABLE_USB                                 0

// <q SL_STATUS_STRING_ENABLE_Z_WAVE> Enable Z-Wave status string components
// <i> Enable (1) or disable (0) Z-Wave string components.
// <i> Default: 0
#define  SL_STATUS_STRING_ENABLE_Z_WAVE                              0

// <q SL_STATUS_STRING_ENABLE_GECKO_OS> Enable Gecko OS status string components
// <i> Enable (1) or disable (0) Gecko OS string components.
// <i> Default: 0
#define  SL_STATUS_STRING_ENABLE_GECKO_OS                            0

// </h>

#endif // End of status_string_config module include.

// <<< end of configuration section >>>
