/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

// *******************************************************************
// Stack Profile Parameters
//
// We use the HCL profile.

#if defined(SL_ZIGBEE_TEST)
#define SL_ZIGBEE_COMMAND_INTEPRETER_HAS_DESCRIPTION_FIELD
#endif

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

// Ideally this should be set to ZIGBEE_PRO_STACK_PROFILE in zigbee.h
#define SL_ZIGBEE_STACK_PROFILE 2

// *******************************************************************
// Ember static memory requirements
//
// There are constants that define the amount of static memory
// required by the stack. If the application does not set them then
// the default values from sl_zigbee_configuration_defaults.h are used.
//
// for example, this changes the default number of buffers to 8

#define SL_ZIGBEE_RETRY_QUEUE_SIZE 15
#define SL_ZIGBEE_STORE_AND_FORWARD_QUEUE_SIZE 5

// Need larger buffer to run max data size mac tests.
#define SL_ZIGBEE_COMMAND_BUFFER_LENGTH 150

// We take the default address table size (8) and add 5 entries
// for the Security Address Cache.
#define BASE_ADDRESS_TABLE_SIZE     8
#define SECURITY_ADDRESS_CACHE_SIZE 5

// To enable interopability for those non-polling RxOnWhenIdle end devices,
// set the timeout very large (4 hours).
#define SL_ZIGBEE_END_DEVICE_POLL_TIMEOUT MINUTES_256
// Certain MAC tests do associations without sending any network encrypted data
// (including Device Announces), so to not age out the child quickly, we match
// the short timeout to SL_ZIGBEE_END_DEVICE_POLL_TIMEOUT
#define SL_ZIGBEE_SHORT_CHILD_TIMEOUT SL_ZIGBEE_END_DEVICE_POLL_TIMEOUT
#define SL_ZIGBEE_PAN_ID_CONFLICT_REPORT_THRESHOLD 1
#define SL_ZIGBEE_END_DEVICE_KEEP_ALIVE_SUPPORT_MODE  SL_ZIGBEE_KEEP_ALIVE_SUPPORT_ALL

#define SL_ZIGBEE_ZIGBEE_NUM_NETWORK_RETRIES_DEFAULT 15
// Sets the size of the static buffer that will be used in the packet-Handoff
#define PACKET_HANDOFF_BUFFER_SIZE 256

// *******************************************************************
// for defining password protected CLI events
#define ZA_CLI_FULL

// *******************************************************************
// Application Handlers
//
// By default, a number of stub handlers are automatically provided
// that have no effect.  If the application would like to implement any
// of these handlers itself, it needs to define the appropriate macro

#define SL_ZIGBEE_APPLICATION_HAS_REMOTE_BINDING_HANDLER
#define SL_ZIGBEE_APPLICATION_HAS_TRUST_CENTER_JOIN_HANDLER
#define SL_ZIGBEE_APPLICATION_HAS_CHILD_JOIN_HANDLER
#define SL_ZIGBEE_APPLICATION_HAS_SWITCH_KEY_HANDLER
#define SL_ZIGBEE_APPLICATION_HAS_LINK_KEY_STORAGE_HANDLER
#define SL_ZIGBEE_APPLICATION_HAS_INCOMING_ROUTE_ERROR_HANDLER
#define SL_ZIGBEE_APPLICATION_HAS_ZIGBEE_KEY_ESTABLISHMENT_HANDLER
#define SL_ZIGBEE_APPLICATION_HAS_COUNTER_HANDLER
#define SL_ZIGBEE_APPLICATION_HAS_RAW_HANDLER
#define SL_ZIGBEE_APPLICATION_HAS_INCOMING_MANY_TO_ONE_ROUTE_REQUEST_HANDLER
#define SL_ZIGBEE_APPLICATION_HAS_DUTY_CYCLE_HANDLER
#define SL_ZIGBEE_APPLICATION_HAS_OVERRIDE_SOURCE_ROUTING
// needed for mac-address-filtering
#define SL_ZIGBEE_AF_PLUGIN_PACKET_HANDOFF
#define SL_ZIGBEE_CALLBACK_INCOMING_PACKET_FILTER
#define SL_ZIGBEE_APPLICATION_HAS_PAN_ID_CONFLICT_HANDLER

#ifdef MAC_TEST_COMMANDS_SUPPORT
 #define SL_ZIGBEE_APPLICATION_HAS_ENERGY_SCAN_RESULT_HANDLER
 #define SL_ZIGBEE_APPLICATION_HAS_MAC_PASSTHROUGH_FILTER_HANDLER
 #define SL_ZIGBEE_APPLICATION_HAS_ORPHAN_NOTIFICATION_HANDLER
#endif // MAC_TEST_COMMANDS_SUPPORT

// *******************************************************************
// Miscellaneous Application defines

#define SL_ZIGBEE_REQUIRE_EXACT_COMMAND_NAME

// To enable passing ZDO Request messages up to the application from stack
//    Enabled so Pro-Compliance tests would receive Parent Announces
#define SL_ZIGBEE_APPLICATION_RECEIVES_SUPPORTED_ZDO_REQUESTS
