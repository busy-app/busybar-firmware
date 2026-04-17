/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

#ifndef EZSP_HOST
#include PLATFORM_HEADER
#include "hal/hal.h"
#endif

#include "app/framework/include/af.h"
#include "zap-cluster-command-parser.h"
#include "stack/include/zigbee-device-stack.h"
#include "network-steering.h"
#include "find-and-bind-target.h"
#include "zll-commissioning-common.h"

#ifdef SL_CATALOG_GECKO_BOOTLOADER_INTERFACE_PRESENT
#include "api/btl_interface.h"
#endif // SL_CATALOG_GECKO_BOOTLOADER_INTERFACE_PRESENT

#include PLATFORM_HEADER

#ifndef SIZE_OF_JOINING_LOOKUP
#define SIZE_OF_JOINING_LOOKUP 4
#endif

#if (0 != SIZE_OF_JOINING_LOOKUP)
#define LOOKUP_DEVICE_ANNCE_LOGIC
#endif

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

// Task id for fast-polling mechanism
#define TEST_HARNESS_JOINING_TASK 0x10000
// Network steering state
extern sl_zigbee_af_plugin_network_steering_joining_state_t sli_zigbee_af_network_steering_state;
typedef enum {
  NETWORK_INITIAL = 0,
  NETWORK_STARTING = 1,
  NETWORK_JOINED = 2,
  NETWORK_REJOINING = 3
} NetworkState;
static NetworkState networkState = NETWORK_INITIAL;

/**
 * Custom CLI.  This command tree is executed by typing "custom <command>"
 * See app/util/serial/sl_zigbee_command_interpreter.h for more detail on writing commands.
 **/

#if (defined LOOKUP_DEVICE_ANNCE_LOGIC)
static sl_802154_long_addr_t devAnnacedArray[SIZE_OF_JOINING_LOOKUP];
static sl_802154_long_addr_t joiningNode;
static uint8_t devAnnceIndex;
static void storeDevAnnce(sl_802154_short_addr_t sl_zigbee_node_id,
                          sl_zigbee_aps_frame_t* apsFrame,
                          uint8_t* message,
                          uint16_t length)
{
  UNUSED_VAR(sl_zigbee_node_id);
  // Catch device annce and store store eui64
  // to aid to lookup to rule out false entries
  if (NULL != apsFrame
      && END_DEVICE_ANNOUNCE == apsFrame->clusterId
      && SL_ZIGBEE_ZDO_ENDPOINT == apsFrame->destinationEndpoint
      && SL_ZIGBEE_ZDO_PROFILE_ID == apsFrame->profileId
      && NULL != message
      && length >= 11) {
    memcpy(devAnnacedArray[devAnnceIndex++], &message[3], EUI64_SIZE);
    // Roll over the index
    if (devAnnceIndex >= SIZE_OF_JOINING_LOOKUP) {
      devAnnceIndex = 0;
    }
  }
}

static bool checkStoredDeviceAnnce(sl_802154_long_addr_t eui64)
{
  // This node may have got added to neighbor table during joining,
  // looking up if it had made a dev annce makes its confirm that the node is
  // in the network.
  for (int i = 0; i < SIZE_OF_JOINING_LOOKUP; i++) {
    if (0 == memcmp(devAnnacedArray[i], eui64, EUI64_SIZE)) {
      return true;
    }
  }
  return false;
}
#endif

void customLookupNeighbourTable(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t eui64;
  sl_zigbee_copy_eui64_arg(arguments, 0, eui64, true);
  //sl_zigbee_copy_hex_arg(arguments, 0, eui64, sizeof(eui64), false);
  sl_802154_short_addr_t nodeId;
  sl_status_t status = sl_zigbee_lookup_node_id_by_eui64(eui64, &nodeId);
#if (defined LOOKUP_DEVICE_ANNCE_LOGIC)
  // The node is in neighbor table - added during association hence
  // check if the node was joining and has made a device annce
  // - if false, then invalidate nodeId
  if (SL_STATUS_OK == status) {
    if (0 == memcmp(joiningNode, eui64, EUI64_SIZE)) {
      if (false == checkStoredDeviceAnnce(eui64)) {
        nodeId = SL_ZIGBEE_NULL_NODE_ID;
      }
    }
  }
#endif
  if (SL_STATUS_OK != status) {
    sl_zigbee_af_core_println("shortaddress:EUI64 unknown");
  } else {
    sl_zigbee_af_core_println("shortaddress:0x%04X", nodeId);
  }
}

#ifdef EZSP_HOST
#define LAUNCH_BOOTLOADER(enabled) sl_zigbee_ezsp_launch_standalone_bootloader((enabled))
#else
#ifndef SL_ZIGBEE_TEST
#define LAUNCH_BOOTLOADER(enabled) bootloader_rebootAndInstall();
#else
#define LAUNCH_BOOTLOADER(enabled)
#endif // SL_ZIGBEE_TEST
#endif // EZSP_HOST

void customResetBootloader(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  LAUNCH_BOOTLOADER(true); //STANDALONE_BOOTLOADER_RECOVERY_MODE
}

/** @brief Pre ZDO Message Received
 *
 * This function passes the application an incoming ZDO message and gives the
 * appictation the opportunity to handle it. By default, this callback returns
 * false indicating that the incoming ZDO message has not been handled and
 * should be handled by the Application Framework.
 *
 * @param sl_zigbee_node_id   Ver.: always
 * @param apsFrame   Ver.: always
 * @param message   Ver.: always
 * @param length   Ver.: always
 */
bool sl_zigbee_af_pre_zdo_message_received_cb(sl_802154_short_addr_t sl_zigbee_node_id,
                                              sl_zigbee_aps_frame_t* apsFrame,
                                              uint8_t* message,
                                              uint16_t length)
{
#if (defined LOOKUP_DEVICE_ANNCE_LOGIC)
  storeDevAnnce(sl_zigbee_node_id,
                apsFrame,
                message,
                length);
#else
  (void)sl_zigbee_node_id;
  (void)apsFrame;
  (void)message;
  (void)length;
#endif
  return false;
}

/** @brief Trust Center Join
 *
 * This callback is called from within the application framework's
 * implementation of sl_zigbee_internal_trust_center_join_handler or sl_zigbee_ezsp_trust_center_join_handler.
 * This callback provides the same arguments passed to the
 * TrustCenterJoinHandler. For more information about the TrustCenterJoinHandler
 * please see documentation included in stack/include/trust-center.h.
 *
 * @param newNodeId   Ver.: always
 * @param newNodeEui64   Ver.: always
 * @param parentOfNewNode   Ver.: always
 * @param status   Ver.: always
 * @param decision   Ver.: always
 */
void sl_zigbee_af_trust_center_join_cb(sl_802154_short_addr_t newNodeId,
                                       sl_802154_long_addr_t newNodeEui64,
                                       sl_802154_short_addr_t parentOfNewNode,
                                       sl_zigbee_device_update_t status,
                                       sl_zigbee_join_decision_t decision)
{
  UNUSED_VAR(parentOfNewNode);
  if (SL_ZIGBEE_STANDARD_SECURITY_SECURED_REJOIN == status
      || SL_ZIGBEE_STANDARD_SECURITY_UNSECURED_JOIN == status
      || SL_ZIGBEE_STANDARD_SECURITY_UNSECURED_REJOIN == status) {
    sl_zigbee_af_add_address_table_entry(newNodeEui64, newNodeId);
#if (defined LOOKUP_DEVICE_ANNCE_LOGIC)
    memcpy(joiningNode, newNodeEui64, EUI64_SIZE);
#endif
  }

  if (status == SL_ZIGBEE_STANDARD_SECURITY_UNSECURED_JOIN && decision == SL_ZIGBEE_USE_PRECONFIGURED_KEY) {
    // Another device has joined using network steering.
    sl_zigbee_af_core_println("Find and bind target start: 0x%02X",
                              sl_zigbee_af_find_and_bind_target_start(1));
  }
}

/** @brief Alarms Cluster Reset Alarm
 *
 *
 *
 * @param alarmCode   Ver.: always
 * @param clusterId   Ver.: always
 */
bool sl_zigbee_af_alarm_cluster_reset_alarm_cb(int8u alarmCode,
                                               int16u clusterId)
{
  UNUSED_VAR(alarmCode);
  UNUSED_VAR(clusterId);
  return TRUE;
}

/** @brief Alarms Cluster Reset All Alarms
 *
 *
 *
 */
bool sl_zigbee_af_alarm_cluster_reset_all_alarms_cb(void)
{
  return TRUE;
}

/** @brief Alarms Cluster Alarm
 *
 *
 *
 * @param alarmCode   Ver.: always
 * @param clusterId   Ver.: always
 */
bool sl_zigbee_af_alarm_cluster_alarm_cb(int8u alarmCode,
                                         int16u clusterId)
{
  UNUSED_VAR(alarmCode);
  UNUSED_VAR(clusterId);
  return TRUE;
}

/** @brief Groups Cluster Add Group Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 */
bool sl_zigbee_af_groups_cluster_add_group_response_cb(int8u status,
                                                       int16u groupId)
{
  UNUSED_VAR(status);
  UNUSED_VAR(groupId);
  return TRUE;
}

/** @brief Groups Cluster View Group Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 * @param groupName   Ver.: always
 */
bool sl_zigbee_af_groups_cluster_view_group_response_cb(int8u status,
                                                        int16u groupId,
                                                        int8u* groupName)
{
  UNUSED_VAR(status);
  UNUSED_VAR(groupId);
  UNUSED_VAR(groupName);
  return TRUE;
}

/** @brief Groups Cluster Get Group Membership Response
 *
 *
 *
 * @param capacity   Ver.: always
 * @param groupCount   Ver.: always
 * @param groupList   Ver.: always
 */
bool sl_zigbee_af_groups_cluster_get_group_membership_response_cb(int8u capacity,
                                                                  int8u groupCount,
                                                                  int8u* groupList)
{
  UNUSED_VAR(capacity);
  UNUSED_VAR(groupCount);
  UNUSED_VAR(groupList);
  return TRUE;
}

/** @brief Groups Cluster Remove Group Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 */
bool sl_zigbee_af_groups_cluster_remove_group_response_cb(int8u status,
                                                          int16u groupId)
{
  UNUSED_VAR(status);
  UNUSED_VAR(groupId);
  return TRUE;
}

/** @brief Scenes Cluster Add Scene Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 * @param sceneId   Ver.: always
 */
bool sl_zigbee_af_scenes_cluster_add_scene_response_cb(int8u status,
                                                       int16u groupId,
                                                       int8u sceneId)
{
  UNUSED_VAR(status);
  UNUSED_VAR(groupId);
  UNUSED_VAR(sceneId);
  return TRUE;
}

/** @brief Scenes Cluster View Scene Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 * @param sceneId   Ver.: always
 * @param transitionTime   Ver.: always
 * @param sceneName   Ver.: always
 * @param extensionFieldSets   Ver.: always
 */
bool sl_zigbee_af_scenes_cluster_view_scene_response_cb(int8u status,
                                                        int16u groupId,
                                                        int8u sceneId,
                                                        int16u transitionTime,
                                                        int8u* sceneName,
                                                        int8u* extensionFieldSets)
{
  UNUSED_VAR(status);
  UNUSED_VAR(groupId);
  UNUSED_VAR(sceneId);
  UNUSED_VAR(transitionTime);
  UNUSED_VAR(sceneName);
  UNUSED_VAR(extensionFieldSets);
  return TRUE;
}

/** @brief Scenes Cluster Remove Scene Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 * @param sceneId   Ver.: always
 */
boolean sl_zigbee_af_scenes_cluster_enhanced_add_scene_response_cb(uint8_t status,
                                                                   uint16_t groupId,
                                                                   uint8_t sceneId)
{
  UNUSED_VAR(status);
  UNUSED_VAR(groupId);
  UNUSED_VAR(sceneId);
  return TRUE;
}

boolean sl_zigbee_af_scenes_cluster_enhanced_view_scene_response_cb(uint8_t status,
                                                                    uint16_t groupId,
                                                                    uint8_t sceneId,
                                                                    uint16_t transitionTime,
                                                                    uint8_t* sceneName,
                                                                    uint8_t* extensionFieldSets)
{
  UNUSED_VAR(status);
  UNUSED_VAR(groupId);
  UNUSED_VAR(sceneId);
  UNUSED_VAR(transitionTime);
  UNUSED_VAR(sceneName);
  UNUSED_VAR(extensionFieldSets);
  return TRUE;
}

boolean sl_zigbee_af_scenes_cluster_copy_scene_response_cb(int8u status,
                                                           int16u groupIdFrom,
                                                           uint8_t sceneIdFrom)
{
  UNUSED_VAR(status);
  UNUSED_VAR(groupIdFrom);
  UNUSED_VAR(sceneIdFrom);
  return TRUE;
}
bool sl_zigbee_af_scenes_cluster_remove_scene_response_cb(int8u status,
                                                          int16u groupId,
                                                          int8u sceneId)
{
  UNUSED_VAR(status);
  UNUSED_VAR(groupId);
  UNUSED_VAR(sceneId);
  return TRUE;
}

/** @brief Scenes Cluster Remove All Scenes Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 */
bool sl_zigbee_af_scenes_cluster_remove_all_scenes_response_cb(int8u status,
                                                               int16u groupId)
{
  UNUSED_VAR(status);
  UNUSED_VAR(groupId);
  return TRUE;
}

/** @brief Scenes Cluster Store Scene Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 * @param sceneId   Ver.: always
 */
bool sl_zigbee_af_scenes_cluster_store_scene_response_cb(int8u status,
                                                         int16u groupId,
                                                         int8u sceneId)
{
  UNUSED_VAR(status);
  UNUSED_VAR(groupId);
  UNUSED_VAR(sceneId);
  return TRUE;
}

/** @brief Scenes Cluster Get Scene Membership Response
 *
 *
 *
 * @param status   Ver.: always
 * @param capacity   Ver.: always
 * @param groupId   Ver.: always
 * @param sceneCount   Ver.: always
 * @param sceneList   Ver.: always
 */
bool sl_zigbee_af_scenes_cluster_get_scene_membership_response_cb(int8u status,
                                                                  int8u capacity,
                                                                  int16u groupId,
                                                                  int8u sceneCount,
                                                                  int8u* sceneList)
{
  UNUSED_VAR(status);
  UNUSED_VAR(capacity);
  UNUSED_VAR(groupId);
  UNUSED_VAR(sceneCount);
  UNUSED_VAR(sceneList);
  return TRUE;
}

/** @brief Thermostat Cluster Current Weekly Schedule
 *
 *
 *
 * @param numberOfTransitionsForSequence   Ver.: always
 * @param dayOfWeekForSequence   Ver.: always
 * @param modeForSequence   Ver.: always
 * @param payload   Ver.: always
 */
bool sl_zigbee_af_thermostat_cluster_current_weekly_schedule_cb(int8u numberOfTransitionsForSequence,
                                                                int8u dayOfWeekForSequence,
                                                                int8u modeForSequence,
                                                                int8u* payload)
{
  UNUSED_VAR(numberOfTransitionsForSequence);
  UNUSED_VAR(dayOfWeekForSequence);
  UNUSED_VAR(modeForSequence);
  UNUSED_VAR(payload);
  return TRUE;
}

/** @brief Thermostat Cluster Relay Status Log
 *
 *
 *
 * @param timeOfDay   Ver.: always
 * @param relayStatus   Ver.: always
 * @param localTemperature   Ver.: always
 * @param humidityInPercentage   Ver.: always
 * @param setpoint   Ver.: always
 * @param unreadEntries   Ver.: always
 */
bool sl_zigbee_af_thermostat_cluster_relay_status_log_cb(int16u timeOfDay,
                                                         int16u relayStatus,
                                                         int16s localTemperature,
                                                         int8u humidityInPercentage,
                                                         int16s setpoint,
                                                         int16u unreadEntries)
{
  UNUSED_VAR(timeOfDay);
  UNUSED_VAR(relayStatus);
  UNUSED_VAR(localTemperature);
  UNUSED_VAR(humidityInPercentage);
  UNUSED_VAR(setpoint);
  UNUSED_VAR(unreadEntries);
  return TRUE;
}

/** @brief Thermostat Cluster Setpoint Raise Lower
 *
 *
 *
 * @param mode   Ver.: always
 * @param amount   Ver.: always
 */
bool sl_zigbee_af_thermostat_cluster_setpoint_raise_lower_cb(int8u mode,
                                                             int8s amount)
{
  UNUSED_VAR(mode);
  UNUSED_VAR(amount);
  return TRUE;
}

/** @brief Thermostat Cluster Set Weekly Schedule
 *
 *
 *
 * @param numberOfTransitionsForSequence   Ver.: always
 * @param dayOfWeekForSequence   Ver.: always
 * @param modeForSequence   Ver.: always
 * @param payload   Ver.: always
 */
bool sl_zigbee_af_thermostat_cluster_set_weekly_schedule_cb(int8u numberOfTransitionsForSequence,
                                                            int8u dayOfWeekForSequence,
                                                            int8u modeForSequence,
                                                            int8u* payload)
{
  UNUSED_VAR(numberOfTransitionsForSequence);
  UNUSED_VAR(dayOfWeekForSequence);
  UNUSED_VAR(modeForSequence);
  UNUSED_VAR(payload);
  return TRUE;
}

/** @brief Thermostat Cluster Get Weekly Schedule
 *
 *
 *
 * @param daysToReturn   Ver.: always
 * @param modeToReturn   Ver.: always
 */
bool sl_zigbee_af_thermostat_cluster_get_weekly_schedule_cb(int8u daysToReturn,
                                                            int8u modeToReturn)
{
  UNUSED_VAR(daysToReturn);
  UNUSED_VAR(modeToReturn);
  return TRUE;
}

/** @brief Thermostat Cluster Clear Weekly Schedule
 *
 *
 *
 */
bool sl_zigbee_af_thermostat_cluster_clear_weekly_schedule_cb(void)
{
  return TRUE;
}

/** @brief Thermostat Cluster Get Relay Status Log
 *
 *
 *
 */
bool sl_zigbee_af_thermostat_cluster_get_relay_status_log_cb(void)
{
  return TRUE;
}

/** @brief IAS Zone Cluster Zone Status Change Notification
 *
 *
 *
 * @param zoneStatus   Ver.: always
 * @param extendedStatus   Ver.: always
 * @param zoneId   Ver.: since ha-1.2-05-3520-29
 * @param delay   Ver.: since ha-1.2-05-3520-29
 */
bool sl_zigbee_af_ias_zone_cluster_zone_status_change_notification_cb(int16u zoneStatus,
                                                                      int8u extendedStatus,
                                                                      int8u zoneId,
                                                                      int16u delay)
{
  UNUSED_VAR(zoneStatus);
  UNUSED_VAR(extendedStatus);
  UNUSED_VAR(zoneId);
  UNUSED_VAR(delay);
  return TRUE;
}

/** @brief IAS Zone Cluster Zone Enroll Request
 *
 *
 *
 * @param zoneType   Ver.: always
 * @param manufacturerCode   Ver.: always
 */
bool sl_zigbee_af_ias_zone_cluster_zone_enroll_request_cb(int16u zoneType,
                                                          int16u manufacturerCode)
{
  UNUSED_VAR(zoneType);
  UNUSED_VAR(manufacturerCode);
  return TRUE;
}

/** @brief IAS Zone Cluster Zone Enroll Response
 *
 *
 *
 * @param enrollResponseCode   Ver.: always
 * @param zoneId   Ver.: always
 */
bool sl_zigbee_af_ias_zone_cluster_zone_enroll_response_cb(int8u enrollResponseCode,
                                                           int8u zoneId)
{
  UNUSED_VAR(enrollResponseCode);
  UNUSED_VAR(zoneId);
  return TRUE;
}

/** @brief Commissioning Cluster Restart Device Response
 *
 *
 *
 * @param status   Ver.: always
 */
bool sl_zigbee_af_commissioning_cluster_restart_device_response_cb(int8u status)
{
  UNUSED_VAR(status);
  return TRUE;
}

/** @brief Commissioning Cluster Save Startup Parameters Response
 *
 *
 *
 * @param status   Ver.: always
 */
bool sl_zigbee_af_commissioning_cluster_save_startup_parameters_response_cb(int8u status)
{
  UNUSED_VAR(status);
  return TRUE;
}

/** @brief Commissioning Cluster Restore Startup Parameters Response
 *
 *
 *
 * @param status   Ver.: always
 */
bool sl_zigbee_af_commissioning_cluster_restore_startup_parameters_response_cb(int8u status)
{
  UNUSED_VAR(status);
  return TRUE;
}

/** @brief Commissioning Cluster Reset Startup Parameters Response
 *
 *
 *
 * @param status   Ver.: always
 */
bool sl_zigbee_af_commissioning_cluster_reset_startup_parameters_response_cb(int8u status)
{
  UNUSED_VAR(status);
  return TRUE;
}

/** @brief Commissioning Cluster Restart Device
 *
 *
 *
 * @param options   Ver.: always
 * @param delay   Ver.: always
 * @param jitter   Ver.: always
 */
bool sl_zigbee_af_commissioning_cluster_restart_device_cb(int8u options,
                                                          int8u delay,
                                                          int8u jitter)
{
  UNUSED_VAR(options);
  UNUSED_VAR(delay);
  UNUSED_VAR(jitter);
  return TRUE;
}

/** @brief Commissioning Cluster Reset Startup Parameters
 *
 *
 *
 * @param options   Ver.: always
 * @param index   Ver.: always
 */
bool sl_zigbee_af_commissioning_cluster_reset_startup_parameters_cb(int8u options,
                                                                    int8u index)
{
  UNUSED_VAR(options);
  UNUSED_VAR(index);
  return TRUE;
}

// The following block of code is only used to perform tests for a SoC TC with
// policy as bdbJoinUsesInstallCodeKey = TRUE
// The action here is to store a MAX_NUMBER_OF_KNOWN_JOINER number of
// EUI64 and Install code and implement the
// sl_zigbee_af_network_creator_security_get_installcode_cb to let the
// application take decision.
#define MAX_NUMBER_OF_KNOWN_JOINER 4
typedef struct {
  sl_802154_long_addr_t eui64;
  uint8_t installCode[16 + 2]; // 6, 8, 12, or 16 bytes plus two-byte CRC
} sli_zigbee_known_joiner_type_t;

static sli_zigbee_known_joiner_type_t knownJoiner[MAX_NUMBER_OF_KNOWN_JOINER] = { 0 };
static uint8_t joinerIndex = 0;

void customAddKnownJoiner(sl_cli_command_arg_t *arguments)
{
  //sl_802154_long_addr_t eui64;
  //uint8_t installCode[16 + 2]; // 6, 8, 12, or 16 bytes plus two-byte CRC
  //uint8_t length;
  sl_zigbee_copy_eui64_arg(arguments, 0, knownJoiner[joinerIndex].eui64, true);
  sl_zigbee_copy_hex_arg(arguments, 1, knownJoiner[joinerIndex].installCode, 18, false);
  if (joinerIndex >= MAX_NUMBER_OF_KNOWN_JOINER) {
    joinerIndex = 0;
  } else {
    joinerIndex++;
  }
}

bool sl_zigbee_af_network_creator_security_get_install_code_cb(sl_802154_long_addr_t newNodeEui64,
                                                               uint8_t *installCode,
                                                               uint8_t *length)
{
  for (int i = 0; i < MAX_NUMBER_OF_KNOWN_JOINER; i++) {
    if (0 == memcmp(newNodeEui64, knownJoiner[i].eui64, 8)) {
      memcpy(installCode, knownJoiner[i].installCode, 18);
      *length = 18;
      return true;
    }
  }
  return false;
}

bool sl_zigbee_af_door_lock_cluster_lock_door_response_cb(uint8_t status)
{
  UNUSED_VAR(status);
  return TRUE;
}

bool sl_zigbee_af_door_lock_cluster_unlock_door_response_cb(uint8_t status)
{
  UNUSED_VAR(status);
  return TRUE;
}

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action.  The return code
 * from this callback is ignored by the framework.  The framework will always
 * process the stack status after the callback returns.
 *
 * @param status   Ver.: always
 */
bool sl_zigbee_af_stack_status_cb(sl_status_t status)
{
  // Note, the ZLL state is automatically updated by the stack and the plugin.
  sl_zigbee_af_core_println("Stack status changed: status = %02X, networkState = %d", status, networkState);

  switch (status) {
    case SL_STATUS_NETWORK_UP:
      if (sli_zigbee_af_network_steering_state != SL_ZIGBEE_AF_PLUGIN_NETWORK_STEERING_STATE_NONE
          || touchLinkInProgress()) {
        // We have joined a PAN using network steering or touchlinking.
        sl_zigbee_af_core_println("Find and bind target start: 0x%02X",
                                  sl_zigbee_af_find_and_bind_target_start(1));
      }
      networkState = NETWORK_JOINED;
      break;

    case SL_STATUS_NETWORK_DOWN:
      if (sl_zigbee_network_state() == SL_ZIGBEE_JOINED_NETWORK_NO_PARENT) {
        networkState = NETWORK_REJOINING;
      } else {
        networkState = NETWORK_INITIAL;
      }
  }

  // This value is ignored by the framework.
  return false;
}

/** @brief Complete
 *
 * This callback is fired when the Network Steering plugin is complete.
 *
 * @param status On success this will be set to SL_STATUS_OK to indicate a
 * network was joined successfully. On failure this will be the status code of
 * the last join or scan attempt. Ver.: always
 * @param totalBeacons The total number of 802.15.4 beacons that were heard,
 * including beacons from different devices with the same PAN ID. Ver.: always
 * @param joinAttempts The number of join attempts that were made to get onto
 * an open Zigbee network. Ver.: always
 * @param finalState The finishing state of the network steering process. From
 * this, one is able to tell on which channel mask and with which key the
 * process was complete. Ver.: always
 */
void sl_zigbee_af_network_steering_complete_cb(sl_status_t status,
                                               uint8_t totalBeacons,
                                               uint8_t joinAttempts,
                                               uint8_t finalState)
{
  sl_zigbee_af_core_println("Network Steering Completed: %s (0x%02X)",
                            (status == SL_STATUS_OK ? "Join Success" : "FAILED"),
                            status);
  sl_zigbee_af_core_println("Finishing state: 0x%02X", finalState);
  sl_zigbee_af_core_println("Beacons heard: %d\nJoin Attempts: %d", totalBeacons, joinAttempts);

  // TODO: All Hubs specifies that we stop after 120 seconds, but this
  // is probably good enough for now.
  sl_zigbee_af_remove_from_current_app_tasks(TEST_HARNESS_JOINING_TASK); // Stop fast-poll
}

/** @brief Touch Link Complete
 *
 * This function is called by the ZLL Commissioning Common plugin when touch linking
 * completes.
 *
 * @param networkInfo The ZigBee and ZLL-specific information about the network
 * and target. Ver.: always
 * @param deviceInformationRecordCount The number of sub-device information
 * records for the target. Ver.: always
 * @param deviceInformationRecordList The list of sub-device information
 * records for the target. Ver.: always
 */
void sl_zigbee_af_zll_commissioning_common_touch_link_complete_cb(const sl_zigbee_zll_network_t *networkInfo,
                                                                  uint8_t deviceInformationRecordCount,
                                                                  const sl_zigbee_zll_device_info_record_t *deviceInformationRecordList)
{
  UNUSED_VAR(networkInfo);
  UNUSED_VAR(deviceInformationRecordCount);
  UNUSED_VAR(deviceInformationRecordList);

  sl_zigbee_af_core_println("%s network %s: 0x%02X",
                            "Touchlink",
                            "complete",
                            SL_STATUS_OK);
}

/** @brief Touch Link Failed
 *
 * This function is called by the ZLL Commissioning plugin if touch linking
 * fails.
 *
 * @param status The reason the touch link failed. Ver.: always
 */
void sl_zigbee_af_zll_commissioning_touch_link_failed_cb(sl_zigbee_af_zll_commissioning_status_t status)
{
  UNUSED_VAR(status);

  sl_zigbee_af_core_println("%s network %s: 0x%02X",
                            "Touchlink",
                            "complete",
                            SL_STATUS_FAIL);
}
