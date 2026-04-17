/***************************************************************************//**
 * @file app.c
 * @brief Callbacks implementation and application specific code.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "app/framework/include/af.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "network-creator.h"
#include "network-creator-security.h"
#include "network-steering.h"
#include "find-and-bind-target.h"
#ifdef SL_CATALOG_ZIGBEE_GREEN_POWER_SERVER_PRESENT
#include "green-power-server.h"
#include "green-power-common.h"
#endif //SL_CATALOG_ZIGBEE_GREEN_POWER_SERVER_PRESENT
#if defined(SL_CATALOG_ZIGBEE_MULTIRAIL_DEMO_PRESENT)
#include "multirail-demo.h"
#endif // SL_CATALOG_ZIGBEE_MULTIRAIL_DEMO_PRESENT

#if defined(SL_CATALOG_LED0_PRESENT)
#include "sl_led.h"
#include "sl_simple_led_instances.h"
#define led_turn_on(led) sl_led_turn_on(led)
#define led_turn_off(led) sl_led_turn_off(led)
#define led_toggle(led) sl_led_toggle(led)
#define COMMISSIONING_STATUS_LED (&sl_led_led0)
#define ON_OFF_LIGHT_LED         (&sl_led_led0)
#else // !SL_CATALOG_LED0_PRESENT
#define led_turn_on(led)
#define led_turn_off(led)
#define led_toggle(led)
#endif // SL_CATALOG_LED0_PRESENT

#define LED_BLINK_PERIOD_MS      2000
#define ZIGBEE_STARTUP_DELAY_MS 40

#ifdef SL_CATALOG_ZIGBEE_REAL_IPC_PRESENT
#define MAX_BUFFER_SIZE 127
#else
#define MAX_BUFFER_SIZE 128
#endif // SL_CATALOG_ZIGBEE_REAL_IPC_PRESENT

static sl_zigbee_af_event_t commissioning_led_event;
static sl_zigbee_af_event_t finding_and_binding_event;
static sl_zigbee_af_event_t sink_commissioning_mode_event;
#if defined(SL_CATALOG_ZIGBEE_MULTIRAIL_DEMO_PRESENT)
static sl_zigbee_af_event_t gp_transmit_complete_event;
#endif // SL_CATALOG_ZIGBEE_MULTIRAIL_DEMO_PRESENT
static bool enterComm;

#define LIGHT_ENDPOINT 1

//---------------
// Event handlers

void commissioning_led_event_handler(sl_zigbee_af_event_t *event)
{
  UNUSED_VAR(event);

  if (sl_zigbee_af_network_state() == SL_ZIGBEE_JOINED_NETWORK) {
    uint16_t identifyTime;
    sl_zigbee_af_read_server_attribute(LIGHT_ENDPOINT,
                                       ZCL_IDENTIFY_CLUSTER_ID,
                                       ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                                       (uint8_t *)&identifyTime,
                                       sizeof(identifyTime));
    if (identifyTime > 0) {
      led_toggle(COMMISSIONING_STATUS_LED);
      sl_zigbee_af_event_set_delay_ms(&commissioning_led_event,
                                      LED_BLINK_PERIOD_MS << 1);
    } else {
      led_turn_on(COMMISSIONING_STATUS_LED);
    }
  } else {
    sl_zigbee_af_network_steering_autostart();
  }
}

void finding_and_binding_event_handler(sl_zigbee_af_event_t *event)
{
  UNUSED_VAR(event);

  if (sl_zigbee_af_network_state() == SL_ZIGBEE_JOINED_NETWORK) {
    sl_zigbee_app_debug_println("Find and bind target start: 0x%02X",
                                sl_zigbee_af_find_and_bind_target_start(LIGHT_ENDPOINT));
  }
}

extern sl_zigbee_af_zcl_request_status_t sli_zigbee_af_green_power_server_gp_sink_commissioning_mode_command_handler(uint8_t options,
                                                                                                                     uint16_t gpmAddrForSecurity,
                                                                                                                     uint16_t gpmAddrForPairing,
                                                                                                                     uint8_t sinkEndpoint);

// Enter or exit sink commissioning mode
static void sink_commissioning_mode_event_handler(sl_zigbee_af_event_t *event)
{
  UNUSED_VAR(event);

  uint8_t options = SL_ZIGBEE_AF_GP_SINK_COMMISSIONING_MODE_OPTIONS_INVOLVE_PROXIES \
                    | ((enterComm) ? SL_ZIGBEE_AF_GP_SINK_COMMISSIONING_MODE_OPTIONS_ACTION : 0);

  sli_zigbee_af_green_power_server_gp_sink_commissioning_mode_command_handler(options,    //options - (Involve Proxy | Enter)
                                                                              0xFFFF,        //addr
                                                                              0xFFFF,        //addr
                                                                              LIGHT_ENDPOINT);        //light Endpoint
  if (enterComm) {
    led_turn_on(COMMISSIONING_STATUS_LED);
    enterComm = false;
  } else {
    led_turn_off(COMMISSIONING_STATUS_LED);
    enterComm = true;
  }
}

#if defined(SL_CATALOG_ZIGBEE_MULTIRAIL_DEMO_PRESENT)

extern void sli_zigbee_stack_d_gp_sent_handler(sl_status_t status, uint8_t gpepHandle);

void gp_transmit_complete_event_handler(sl_zigbee_af_event_t *event)
{
  UNUSED_VAR(event);
  sli_zigbee_stack_d_gp_sent_handler(SL_STATUS_OK, 0);
}

#endif // SL_CATALOG_ZIGBEE_MULTIRAIL_DEMO_PRESENT

//----------------------
// Implemented Callbacks

/** @brief Init
 * Application init function
 */
void sl_zigbee_af_main_init_cb(void)
{
  sl_zigbee_af_event_init(&commissioning_led_event, commissioning_led_event_handler);
  sl_zigbee_af_isr_event_init(&finding_and_binding_event, finding_and_binding_event_handler);
  sl_zigbee_af_isr_event_init(&sink_commissioning_mode_event, sink_commissioning_mode_event_handler);
#if defined(SL_CATALOG_ZIGBEE_MULTIRAIL_DEMO_PRESENT)
  sl_zigbee_af_isr_event_init(&gp_transmit_complete_event, gp_transmit_complete_event_handler);
#endif // SL_CATALOG_ZIGBEE_MULTIRAIL_DEMO_PRESENT
  // Start the commissioning LED event after network is up, and communicated to app framework task in the main loop
  sl_zigbee_af_event_set_delay_ms(&commissioning_led_event, ZIGBEE_STARTUP_DELAY_MS);
}

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action.  The return code
 * from this callback is ignored by the framework.  The framework will always
 * process the stack status after the callback returns.
 */
void sl_zigbee_af_stack_status_cb(sl_status_t status)
{
  // Make sure to change the ZLL factory new state based on whether or not
  // we are on a network.
  if (status == SL_STATUS_NETWORK_DOWN) {
    led_turn_off(COMMISSIONING_STATUS_LED);
  } else if (status == SL_STATUS_NETWORK_UP) {
    led_turn_on(COMMISSIONING_STATUS_LED);
    sl_zigbee_af_event_set_active(&finding_and_binding_event);
  }
}

/** @brief Complete network steering.
 *
 * This callback is fired when the Network Steering plugin is complete.
 *
 * @param status On success this will be set to SL_STATUS_OK to indicate a
 * network was joined successfully. On failure this will be the status code of
 * the last join or scan attempt. Ver.: always
 *
 * @param totalBeacons The total number of 802.15.4 beacons that were heard,
 * including beacons from different devices with the same PAN ID. Ver.: always
 * @param joinAttempts The number of join attempts that were made to get onto
 * an open Zigbee network. Ver.: always
 *
 * @param finalState The finishing state of the network steering process. From
 * this, one is able to tell on which channel mask and with which key the
 * process was complete. Ver.: always
 */
void sl_zigbee_af_network_steering_complete_cb(sl_status_t status,
                                               uint8_t totalBeacons,
                                               uint8_t joinAttempts,
                                               uint8_t finalState)
{
  UNUSED_VAR(totalBeacons);
  UNUSED_VAR(joinAttempts);
  UNUSED_VAR(finalState);

  sl_zigbee_app_debug_println("%s network %s: 0x%02X", "Join", "complete", status);

  if (status != SL_STATUS_OK) {
    status = sl_zigbee_af_network_creator_start(false); // distributed
    sl_zigbee_app_debug_println("%s network %s: 0x%02X", "Form", "start", status);
  }
}

/** @brief Complete the network creation process.
 *
 * This callback notifies the user that the network creation process has
 * completed successfully.
 *
 * @param network The network that the network creator plugin successfully
 * formed. Ver.: always
 *
 * @param usedSecondaryChannels Whether or not the network creator wants to
 * form a network on the secondary channels. Ver.: always
 */
void sl_zigbee_af_network_creator_complete_cb(const sl_zigbee_network_parameters_t *network,
                                              bool usedSecondaryChannels)
{
  UNUSED_VAR(network);
  UNUSED_VAR(usedSecondaryChannels);
  sl_zigbee_app_debug_println("%s network %s: 0x%02X",
                              "Form distributed",
                              "complete",
                              SL_STATUS_OK);
}

/** @brief Post Attribute Change
 *
 * This function is called by the application framework after it changes an
 * attribute value. The value passed into this callback is the value to which
 * the attribute was set by the framework.
 */
void sl_zigbee_af_post_attribute_change_cb(uint8_t endpoint,
                                           sl_zigbee_af_cluster_id_t clusterId,
                                           sl_zigbee_af_attribute_id_t attributeId,
                                           uint8_t mask,
                                           uint16_t manufacturerCode,
                                           uint8_t type,
                                           uint8_t size,
                                           uint8_t* value)
{
  UNUSED_VAR(manufacturerCode);
  UNUSED_VAR(type);
  UNUSED_VAR(size);
  UNUSED_VAR(value);

  if (clusterId == ZCL_ON_OFF_CLUSTER_ID
      && attributeId == ZCL_ON_OFF_ATTRIBUTE_ID
      && mask == CLUSTER_MASK_SERVER) {
    bool onOff;
    if (sl_zigbee_af_read_server_attribute(endpoint,
                                           ZCL_ON_OFF_CLUSTER_ID,
                                           ZCL_ON_OFF_ATTRIBUTE_ID,
                                           (uint8_t *)&onOff,
                                           sizeof(onOff))
        == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      if (onOff) {
        led_turn_on(ON_OFF_LIGHT_LED);
      } else {
        led_turn_off(ON_OFF_LIGHT_LED);
      }
    }
  }
}

/** @brief On/off Cluster Server Post Init.
 *
 * Following resolution of the On/Off state at startup for this endpoint, perform any
 * additional initialization needed; e.g., synchronize hardware state.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void sl_zigbee_af_on_off_cluster_server_post_init_cb(uint8_t endpoint)
{
  // At startup, trigger a read of the attribute and possibly a toggle of the
  // LED to make sure they are always in sync.
  sl_zigbee_af_post_attribute_change_cb(endpoint,
                                        ZCL_ON_OFF_CLUSTER_ID,
                                        ZCL_ON_OFF_ATTRIBUTE_ID,
                                        CLUSTER_MASK_SERVER,
                                        0,
                                        0,
                                        0,
                                        NULL);
}

/** @brief Green power server commissioning timeout callback
 *
 * This function is called by the Green Power Server upon expiry of any of the
 * commissioning timer , those are server commissioning window expiry, generic
 * switch commissioning or multi-sensor commissioning timer expiry.
 *
 * @param commissioningTimeoutType one of the types
 *                     COMMISSIONING_TIMEOUT_TYPE_COMMISSIONING_WINDOW_TIMEOUT,
 *                     COMMISSIONING_TIMEOUT_TYPE_GENERIC_SWITCH
 *                     or COMMISSIONING_TIMEOUT_TYPE_MULTI_SENSOR Ver.: always
 * @param numberOfEndpoints Number of sink endpoints participated in the commissioning Ver.: always
 * @param endpoints list of sink endpoints Ver.: always
 */
void sl_zigbee_af_green_power_server_commissioning_timeout_cb(uint8_t commissioningTimeoutType,
                                                              uint8_t numberOfEndpoints,
                                                              uint8_t * endpoints)
{
  UNUSED_VAR(commissioningTimeoutType);
  UNUSED_VAR(numberOfEndpoints);
  UNUSED_VAR(endpoints);
  // Commissioning exit, clear the state and LED indication.
  enterComm = false;
  led_turn_off(COMMISSIONING_STATUS_LED);
}

/** @brief Green power server pairing complete callback
 *
 * This function is called by the Green Power Server upon the completion of the pairing
 * and to indicate the closure of the pairing session.
 *
 * @param numberOfEndpoints number of sink endpoints participated in the pairing Ver.: always
 * @param endpoints list of sink endpoints Ver.: always
 */
void sl_zigbee_af_green_power_server_pairing_complete_cb(uint8_t numberOfEndpoints,
                                                         uint8_t * endpoints)
{
  sl_zigbee_app_debug_print("%s: No of Eps = %02X EPs[",
                            __FUNCTION__,
                            numberOfEndpoints);
  sl_zigbee_app_debug_print_buffer(endpoints, numberOfEndpoints, true);
  sl_zigbee_app_debug_println("]");
  led_turn_off(COMMISSIONING_STATUS_LED);
}

/** @brief
 *
 * Application framework equivalent of ::sl_zigbee_radio_needs_calibrating_handler
 */
void sl_zigbee_af_radio_needs_calibrating_cb(void)
{
  sl_mac_calibrate_current_channel();
}
#ifdef SL_CATALOG_ZIGBEE_GREEN_POWER_SERVER_PRESENT

// Finds and returns the Gp Controlable application endpoint in the APS group
static uint16_t findAppEndpointGroupId(uint8_t endpoint)
{
  for (uint8_t i = 0; i < SL_ZIGBEE_BINDING_TABLE_SIZE; i++) {
    sl_zigbee_binding_table_entry_t binding;
    if (sl_zigbee_get_binding(i, &binding) == SL_STATUS_OK
        && binding.type == SL_ZIGBEE_MULTICAST_BINDING
        && binding.local == endpoint) {
      uint16_t groupId = (binding.identifier[1] << 8) | binding.identifier[0];
      return groupId;
    }
  }
  return 0;
}

// This implementation is targeting the test spec (4.3.4.1)
// where it uses the same group for Gp endpoint and application endpoints.
// to read the groupId of the operational endpoint and use that groupId
// to add the GP endpoint for groupcast notifications.
void sl_zigbee_af_green_power_server_pre_sink_pairing_cb(sli_zigbee_gp_comm_data_saved_t *commissioningGpd)
{
  if (commissioningGpd->preSinkCbSource == GP_PRE_SINK_PAIRING_CALLBACK_PAIRING_CONFIGURATION) {
    return;
  }
  // Set up the grouplist incase it did not have one, supply default values.
  uint8_t *count = commissioningGpd->groupList;
  uint8_t *grouplist = commissioningGpd->groupList + 1;
  *count = 0;
  uint8_t totalEndpointCount = sl_zigbee_get_endpoint_count();
  for (uint8_t i = 0; i < totalEndpointCount && grouplist < (commissioningGpd->groupList + GP_SIZE_OF_SINK_LIST_ENTRIES_OCTET_STRING); i++) {
    uint8_t endpoint = sl_zigbee_get_endpoint(i);
    if (endpoint != 0xff // A valid application endpoint value.
        && isCommissioningAppEndpoint(endpoint)) {
      uint16_t groupId = findAppEndpointGroupId(endpoint);
      if (0 != groupId) {
        (*count)++;
        memcpy(grouplist, &groupId, sizeof(groupId));
        grouplist += sizeof(groupId);
        uint16_t alias = sli_zigbee_gpd_alias(&(commissioningGpd->addr));
        memcpy(grouplist, &alias, sizeof(alias));
        grouplist += sizeof(alias);
      }
    }
  }
}
#endif //SL_CATALOG_ZIGBEE_GREEN_POWER_SERVER_PRESENT

#if defined(SL_CATALOG_ZIGBEE_MULTIRAIL_DEMO_PRESENT)
static void appGpScheduleOutgoingGpdf(sl_zigbee_zigbee_packet_type_t packetType,
                                      int8u* packetData,
                                      int8u size_p);

sl_zigbee_packet_action_t sl_zigbee_pre_incoming_packet_filter_cb(sl_zigbee_zigbee_packet_type_t packetType,
                                                                  int8u* packetData,
                                                                  int8u* size_p,
                                                                  void* data,
                                                                  uint8_t size_d)
{
  UNUSED_VAR(data);
  UNUSED_VAR(size_d);
  appGpScheduleOutgoingGpdf(packetType, packetData, *size_p);

  return SL_ZIGBEE_ACCEPT_PACKET;
}

/** @brief A callback called whenever a secondary instance RAIL event occurs.
 *
 * @param[in] handle A handle for a RAIL instance.
 * @param[in] events A bit mask of RAIL events (full list in rail_types.h)
 */
void sl_zigbee_af_multirail_demo_rail_event_cb(sl_rail_handle_t handle,
                                               sl_rail_events_t events)
{
  UNUSED_VAR(handle);

  if (events & SL_RAIL_EVENT_TX_PACKET_SENT) {
    sl_zigbee_af_event_set_active(&gp_transmit_complete_event);
#ifdef SL_CATALOG_KERNEL_PRESENT
    sl_zigbee_wakeup_common_task();
#endif // SL_CATALOG_KERNEL_PRESENT
  }
}
#endif // SL_CATALOG_ZIGBEE_MULTIRAIL_DEMO_PRESENT

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT) && (SL_ZIGBEE_AF_PLUGIN_IDLE_SLEEP_USE_BUTTONS == 0)
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"

#define BUTTON0 0
#define BUTTON1 1

/***************************************************************************//**
 * A callback called in interrupt context whenever a button changes its state.
 *
 * @remark Can be implemented by the application if required. This function
 * can contain the functionality to be executed in response to changes of state
 * in each of the buttons, or callbacks to appropriate functionality.
 *
 * @note The button state should not be updated in this function, it is updated
 * by specific button driver prior to arriving here
 *
   @param[out] handle             Pointer to button instance
 ******************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  if (handle && sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    if (SL_SIMPLE_BUTTON_INSTANCE(BUTTON0) == handle) {
      sl_zigbee_af_event_set_active(&sink_commissioning_mode_event);
    } else if (SL_SIMPLE_BUTTON_INSTANCE(BUTTON1) == handle) {
      sl_zigbee_af_event_set_active(&finding_and_binding_event);
    }
  }
}
#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT && SL_ZIGBEE_AF_PLUGIN_IDLE_SLEEP_USE_BUTTONS == 0

//-------------------------------------
// Custom CLI commands and related code

#if defined(SL_CATALOG_ZIGBEE_MULTIRAIL_DEMO_PRESENT)

void gpAppGpTxQueueInit(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  sl_zigbee_gp_clear_tx_queue();
  sl_rail_handle_t h = sl_zigbee_af_multirail_demo_init(NULL,
                                                        NULL,
                                                        true,
                                                        sl_rail_get_tx_power_dbm(sl_zigbee_get_rail_handle()),
                                                        NULL,
                                                        0,
                                                        0xFFFF,
                                                        NULL);
  sl_zigbee_app_debug_println("Additional RAIL handle %sinitialized", h ? "" : "not ");
}

void gpAppGpTxQueueSet(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_gp_address_t addr = { 0 };

  addr.applicationId = sl_cli_get_argument_uint8(arguments, 0);
  if (addr.applicationId != SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
    sl_zigbee_app_debug_println("Invalid application ID");
    return;
  }
  addr.id.sourceId = sl_cli_get_argument_uint32(arguments, 1);

  uint8_t gpdCommandId = sl_cli_get_argument_uint8(arguments, 2);

  uint8_t gpdAsduLength = 0;
  uint8_t gpdAsdu[255];
  gpdAsduLength = sl_zigbee_copy_hex_arg(arguments, 3, gpdAsdu, sizeof(gpdAsdu), false);

  if (sl_zigbee_d_gp_send(true,//bool action,
                          false,//bool useCca,
                          &addr,
                          gpdCommandId,
                          gpdAsduLength,
                          gpdAsdu,
                          0,
                          0) != SL_STATUS_OK) {
    sl_zigbee_app_debug_println("Failed to add entry");
  }
}

void gpAppGpTxQueueGet(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_gp_tx_queue_entry_t txQueue;
  txQueue.addr.applicationId = sl_cli_get_argument_uint8(arguments, 0);
  if (txQueue.addr.applicationId != SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
    sl_zigbee_app_debug_println("Invalid application ID");
    return;
  }
  txQueue.addr.id.sourceId = sl_cli_get_argument_uint32(arguments, 1);

  uint8_t buffer[MAX_BUFFER_SIZE];
  uint16_t length = 0;

  if (sl_zigbee_gp_get_tx_queue_entry_from_queue(&txQueue,
                                                 buffer,
                                                 &length,
                                                 sizeof(buffer)) != SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
    sl_zigbee_app_debug_println("");
    sl_zigbee_app_debug_print_buffer(buffer, length, true);
    sl_zigbee_app_debug_println("");
  } else {
    sl_zigbee_app_debug_println("No entry found");
  }
}

void gpAppGpTxQueueRemove(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_gp_address_t addr = { 0 };

  addr.applicationId = sl_cli_get_argument_uint8(arguments, 0);
  if (addr.applicationId != SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
    sl_zigbee_app_debug_println("Invalid application ID");
    return;
  }
  addr.id.sourceId = sl_cli_get_argument_uint32(arguments, 1);

  if (sl_zigbee_d_gp_send(false,//bool action,
                          false,//bool useCca,
                          &addr,
                          0,
                          0,
                          SL_ZIGBEE_NULL_MESSAGE_BUFFER,
                          0,
                          0) != SL_STATUS_OK) {
    sl_zigbee_app_debug_println("No entry found");
  }
}

void gpAppGpSetTxQueueSize(sl_cli_command_arg_t *arguments)
{
  uint8_t size = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_gp_set_max_tx_queue_entry((uint16_t)size);
}

void gpAppGpGetTxQueueSize(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  sl_zigbee_app_debug_println("Max Tx Queue Size = %d", sl_zigbee_get_gp_max_tx_q_list_count());
}

void gpAppGpGetTxQueueCount(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  sl_zigbee_app_debug_print("Tx Queue Count = %d\n", sl_zigbee_get_gp_tx_q_list_count());
}

void gpAppGpPrintTxQueue(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  uint16_t listCount = sl_zigbee_get_gp_tx_q_list_count();
  sl_zigbee_app_debug_println("");
  sl_zigbee_app_debug_println("Number of Gp Tx Queue entries : %d", listCount);
  if (listCount != 0) {
    for (int i = 0; i < listCount; i++) {
      sl_zigbee_gp_tx_queue_entry_t entry;
      uint8_t data[MAX_BUFFER_SIZE];
      uint16_t dataLength;
      sl_zigbee_gp_get_tx_queue_entry_from_queue_index(i,
                                                       &entry,
                                                       data,
                                                       &dataLength);

      sl_zigbee_app_debug_println("Entry [%d] :", i++);
      sl_zigbee_app_debug_println("  App Id =  %d", entry.addr.applicationId);
      sl_zigbee_app_debug_println("  Src Id = 0x%04X", entry.addr.id.sourceId);
      sl_zigbee_app_debug_println("  Data Length = %d", dataLength);
      sl_zigbee_app_debug_print("  Data = [");
      sl_zigbee_app_debug_print_buffer(data, dataLength, true);
      sl_zigbee_app_debug_println("]");
    }
  }
}

// The time between receiving a GP packet and sending a scheduled response
#ifndef GP_RX_OFFSET_USEC
  #define GP_RX_OFFSET_USEC 20500
#endif
#define macToAppDelay(macTimeStamp) ((sl_rail_get_time(SL_RAIL_EFR32_HANDLE) & 0x00FFFFFF) - (macTimeStamp))

extern uint8_t sli_mac_lower_mac_get_radio_channel(uint8_t mac_index);

extern sli_buffer_manager_buffer_t sli_zigbee_gpdf_make_header(bool useCca,
                                                               sl_zigbee_gp_address_t *src,
                                                               sl_zigbee_gp_address_t *dst);

static void free_gp_tx_queue_entry(sl_zigbee_gp_tx_queue_entry_t* entry)
{
  sl_zigbee_gp_remove_from_tx_queue(entry);
}

// Gets an entry from the Gp Stub queue if available and RAIL 2 is ready
static sl_zigbee_gp_tx_queue_entry_t* get_gp_stub_tx_queue(sl_zigbee_gp_address_t* addr,
                                                           uint8_t *asduLength,
                                                           uint8_t *asdu)
{
  // Steal the GP Queue to send it through the additional RAIL handle
  // Check if RAIL 2 handle available and there is anything in GP Stub queue
  sl_zigbee_gp_tx_queue_entry_t sli_zigbee_gp_tx_queue;
  memcpy(&sli_zigbee_gp_tx_queue.addr, addr, sizeof(sl_zigbee_gp_address_t));
  uint8_t data[MAX_BUFFER_SIZE];
  uint16_t dataLength;
  if (sl_zigbee_af_multirail_demo_get_handle()
      && sl_zigbee_gp_get_tx_queue_entry_from_queue(&sli_zigbee_gp_tx_queue,
                                                    data,
                                                    &dataLength,
                                                    sizeof(data)) != SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
    // Allocate a buffer and prepare a outgoing MAC header using gpd address in the sli_zigbee_gp_tx_queue
    sli_buffer_manager_buffer_t header = sli_zigbee_gpdf_make_header(true, NULL, &(sli_zigbee_gp_tx_queue.addr));
    if (header == SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
      return NULL;
    }

    // Add the command Id from the queue to the buffer
    uint8_t len = sl_legacy_buffer_manager_message_buffer_length(header) + 1;
    if ((sl_legacy_buffer_manager_append_to_linked_buffers(header,
                                                           &(sli_zigbee_gp_tx_queue.gpdCommandId),
                                                           1) == SL_STATUS_OK)
        // Copy Command Payload from the queue to the buffer and update the length
        && (sl_legacy_buffer_manager_set_linked_buffers_length(header,
                                                               sl_legacy_buffer_manager_message_buffer_length(header)
                                                               + dataLength) == SL_STATUS_OK)
        // Add the payload
        && (sl_legacy_buffer_manager_copy_to_linked_buffers(data,
                                                            header,
                                                            len,
                                                            dataLength) == SL_STATUS_OK)) {
      // Clear the Stub queue because everything is serialised in header
      sl_zigbee_gp_remove_from_tx_queue(&sli_zigbee_gp_tx_queue);

      // Prepare a RAIL frame to be transported using the additional handle
      uint8_t outPktLength = sl_legacy_buffer_manager_message_buffer_length(header);
      uint8_t outPkt[MAX_BUFFER_SIZE] = { 0 };
      // RAIL Frame : [Total Length (excludes itself) | <-----MAC FRAME ---->| 2 byte CRC]
      outPkt[0] = outPktLength + 2;
      // Copy the data from the buffer
      sl_legacy_buffer_manager_copy_from_linked_buffers(header,
                                                        0,
                                                        &outPkt[1],
                                                        outPktLength);
      // Free the header as the rail frame will be submitted with a new buffer asdu
      sl_legacy_buffer_manager_release_message_buffer(header);

      static sl_zigbee_gp_tx_queue_entry_t copyOfGpStubTxQueue;
      copyOfGpStubTxQueue.inUse = true;
      *asduLength = outPkt[0] + 1;
      memmove(asdu, &outPkt, *asduLength);
      memcpy(&(copyOfGpStubTxQueue.addr), addr, sizeof(sl_zigbee_gp_address_t));
      return &copyOfGpStubTxQueue;
    }
  }
  return NULL;
}

static void appGpScheduleOutgoingGpdf(sl_zigbee_zigbee_packet_type_t packetType,
                                      int8u* packetData,
                                      int8u size_p)
{
  // Application ID: 0 (source ID)
  // MAC Frame : [<---------------MAC Header------------->||<------------------------------------NWK Frame----------------------------------->]
  //              FC(2) | Seq(1) | DstPan(2) | DstAddr(2) || FC(1) | ExtFC(0/1) | SrcId(0/4) | SecFc(0/4) | MIC(0/4) | <------GPDF(1/n)------>
  //              FC    : ExtFC Present(b7)=1| AC(b6)=0| Protocol Ver(b5-b2)=3 GP frames| Frame Type(b1-b0) = 0
  //              ExtFC :  rxAfteTX (b6) = 1 |  AppId(b2-b0) = 0

  // Application ID: 2 (IEEE address)
  // MAC Frame : [<---------------MAC Header-------------------------------->||<------------------------------------NWK Frame----------------------------------->]
  //              FC(2) | Seq(1) | DstPan(2) | DstAddr(2) | Long SrcAddr (8) || FC(1) | ExtFC(0/1) | Endpoint(1) | SecFc(0/4) | MIC(0/4) | <------GPDF(1/n)------>
  //              FC    : ExtFC Present(b7)=1| AC(b6)=0| Protocol Ver(b5-b2)=3 GP frames| Frame Type(b1-b0) = 0
  //              ExtFC :  rxAfteTX (b6) = 1 |  AppId(b2-b0) = 2

  bool isLongSource = ((packetData[1] & 0xC0) == 0xC0);
  uint8_t nwkFC = (isLongSource ? packetData[15] : packetData[7]);
  uint8_t nwkExtendedFC = (isLongSource ? packetData[16] : packetData[8]);

  if (packetType == SL_ZIGBEE_ZIGBEE_PACKET_TYPE_RAW_MAC
      && size_p > 9 // minimum)
      && ((nwkFC & 0x3C) == 0x0C)            // FC : GP Frames has Protocol Version = 3 ?
      && (((nwkFC & 0xC3) == 0x01)           // FC : ExtFC(b7) = 0 | AC(b6) = 0 | Frame type(b1-b0) = 1 = Maint, Frame
          || (((nwkFC & 0xC3) == 0x80)     // FC : ExtFC(b7) = 1 | AC(b6) = 0 | Frame type(b1-b0) = 0 = Data frame
              && ((nwkExtendedFC & 0xC0) == 0x40)))) { // ExtFC :  Direction(b7) = 0 | rxAfteTX (b6) = 1
    // The last 3 bytes of packetData contain the MAC time stamp
    const uint32_t macTimeStamp = ((uint32_t)packetData[size_p - 3] << 16)
                                  + ((uint32_t)packetData[size_p - 2] << 8)
                                  + ((uint32_t)packetData[size_p - 1]);

    // Do we have enough time to schedule the response?
    if (macToAppDelay(macTimeStamp) < GP_RX_OFFSET_USEC) {
      sl_zigbee_gp_address_t gpdAddr = { 0 };
      gpdAddr.applicationId = isLongSource ? SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS : SL_ZIGBEE_GP_APPLICATION_SOURCE_ID;
      if ((nwkFC & 0xC3) == 0x80) {
        if ((nwkExtendedFC & 0x07) == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
          (void) memcpy(&gpdAddr.id.sourceId, &packetData[9], sizeof(sl_zigbee_gp_source_id_t));
        } else {
          (void) memcpy(&gpdAddr.id.gpdIeeeAddress, &packetData[7], sizeof(sl_802154_long_addr_t));   // copy the long source address in MAC header
          gpdAddr.endpoint = packetData[17];
        }
      }
      // Is there a queued response for this source ID?
      uint8_t outPktLength;
      uint8_t outPkt[MAX_BUFFER_SIZE];
      sl_zigbee_gp_tx_queue_entry_t* entry = get_gp_stub_tx_queue(&gpdAddr, &outPktLength, (uint8_t*)&outPkt);
      if (entry) {
        // Schedule sending the response.
        sl_rail_scheduler_info_t schedulerInfo = {
          .priority = 50,
          .slip_time = 2000,
          .transaction_time = 5000
        };
        sl_rail_scheduled_tx_config_t scheduledTxConfig = {
          .mode = SL_RAIL_TIME_DELAY,
          // We could reuse macToAppDelay here, but recalculating the delay
          // will give us the most up-to-date timings:
          .when = GP_RX_OFFSET_USEC - macToAppDelay(macTimeStamp)
        };

        sl_rail_status_t UNUSED status = sl_zigbee_af_multirail_demo_send(outPkt,
                                                                          outPktLength,
                                                                          sli_mac_lower_mac_get_radio_channel(0),
                                                                          &scheduledTxConfig,
                                                                          &schedulerInfo);
        free_gp_tx_queue_entry(entry);
      }
    }
  }
}

void sl_zigbee_multirail_gp_tx_queue_init(void)
{
#if !defined SL_ZIGBEE_TEST
  sl_zigbee_gp_clear_tx_queue();
  sl_zigbee_af_multirail_demo_init(NULL,
                                   NULL,
                                   true,
                                   sl_rail_get_tx_power_dbm(sl_zigbee_get_rail_handle()),
                                   NULL,
                                   0,
                                   0xFFFF,
                                   NULL);
#endif // SL_ZIGBEE_TEST
}

#endif // SL_CATALOG_ZIGBEE_MULTIRAIL_DEMO_PRESENT
