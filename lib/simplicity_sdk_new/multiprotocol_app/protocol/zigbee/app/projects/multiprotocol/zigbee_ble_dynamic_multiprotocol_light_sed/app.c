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

#include PLATFORM_HEADER
#include "hal.h"
#include "sl_zigbee.h"
#include "app/framework/include/af.h"
#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
#include "sl_zigbee_debug_print.h"
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
#include "app/framework/util/af-main.h"
#include "network-steering.h"

#include "app/framework/plugin/reporting/reporting.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_BLE_EVENT_HANDLER_PRESENT
#include "sl_ble_event_handler.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_DISPLAY_PRESENT
#include "sl_dmp_ui.h"
#else // !SL_CATALOG_ZIGBEE_DISPLAY_PRESENT
#include "sl_dmp_ui_stub.h"
#endif // SL_CATALOG_ZIGBEE_DISPLAY_PRESENT

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"

#define BUTTON0         0
#define BUTTON1         1

static uint8_t lastButton;
static bool longPress = false;
static bool leavingNwk = false;
static sl_zigbee_af_event_t button_event;
#endif //SL_CATALOG_SIMPLE_BUTTON_PRESENT

#if defined(SL_CATALOG_LED0_PRESENT)
#include "sl_led.h"
#include "sl_simple_led_instances.h"
#define led_turn_on(led) sl_led_turn_on(led)
#define led_turn_off(led) sl_led_turn_off(led)
#define led_toggle(led) sl_led_toggle(led)
#define LED0     (&sl_led_led0)
#if defined(SL_CATALOG_LED1_PRESENT)
#define LED1     (&sl_led_led1)
#endif // SL_CATALOG_LED1_PRESENT
#else // !SL_CATALOG_LED0_PRESENT
#define led_turn_on(led)
#define led_turn_off(led)
#define led_toggle(led)
#endif // SL_CATALOG_LED0_PRESENT

#define SOURCE_ADDRESS_LEN 8
static sl_802154_long_addr_t SwitchEUI;
static bool identifying = false;

//---------------------
// Forward declarations

static void startIdentifying(void);
static void stopIdentifying(void);
static void setDefaultReportEntry(void);
#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
static void toggleOnoffAttribute(void);
#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT

//--------------
// Event handler

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
void buttonEventHandler(sl_zigbee_af_event_t *event)
{
  (void)event;
  if (lastButton == BUTTON0) {
    toggleOnoffAttribute();
  } else if (lastButton == BUTTON1) {
    sl_zigbee_network_status_t state = sl_zigbee_af_network_state();
    if (state == SL_ZIGBEE_NO_NETWORK) {
      sl_zigbee_af_network_steering_start();
      sl_dmp_ui_display_zigbee_state(DMP_UI_JOINING);
    } else {
      if (!leavingNwk) { // Ignore button1 events while leaving.
        if (!longPress) {
          if (identifying) {
            sl_zigbee_af_app_println("Button- Identify stop");
            stopIdentifying();
          } else if (state == SL_ZIGBEE_JOINED_NETWORK) {
            sl_zigbee_af_app_println("Button- Identify start");
            startIdentifying();
          }
        } else {
          leavingNwk = true;
          sl_zigbee_af_app_println("Button- Leave Nwk");
          sl_zigbee_leave_network(SL_ZIGBEE_LEAVE_NWK_WITH_NO_OPTION);
          sl_zigbee_clear_binding_table();
        }
      }
    }
  }
}
#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT

//----------------------
// Implemented Callbacks

/** @brief Init
 * Application init function
 */
void sl_zigbee_af_main_init_cb(void)
{
  #if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
  sl_zigbee_af_isr_event_init(&button_event, buttonEventHandler);
  #endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT
}

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action. The framework
 * will always process the stack status after the callback returns.
 */
void sl_zigbee_af_stack_status_cb(sl_status_t status)
{
  sl_zigbee_network_status_t nwkState = sl_zigbee_af_network_state();
  sl_zigbee_af_core_println("Stack status=0x%02X, nwkState=%d", status, sl_zigbee_af_network_state());

  switch (nwkState) {
    case SL_ZIGBEE_JOINED_NETWORK:
      sl_dmp_ui_display_zigbee_state(DMP_UI_NETWORK_UP);
      startIdentifying();
      break;
    case SL_ZIGBEE_NO_NETWORK:
      sl_dmp_ui_display_zigbee_state(DMP_UI_NO_NETWORK);
#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
      leavingNwk = false; // leave has completed.
#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT
      stopIdentifying();
      break;
    case SL_ZIGBEE_JOINED_NETWORK_NO_PARENT:
      stopIdentifying();
      break;
    default:
      break;
  }
}

/** @brief Start feedback.
 *
 * This function is called by the Identify plugin when identification begins.
 * It informs the Identify Feedback plugin that it should begin providing its
 * implemented feedback functionality (e.g., LED blinking, buzzer sounding,
 * etc.) until the Identify plugin tells it to stop. The identify time is
 * purely a matter of informational convenience. This plugin does not need to
 * know how long it will identify (the Identify plugin will perform the
 * necessary timekeeping.)
 *
 * @param endpoint The identifying endpoint Ver.: always
 * @param identifyTime The identify time Ver.: always
 */
void sl_zigbee_af_identify_start_feedback_cb(uint8_t endpoint,
                                             uint16_t identifyTime)
{
  (void)endpoint;
  if (identifyTime > 0) {
    identifying = true;
    sl_zigbee_af_app_println("Start Identifying for %dS", identifyTime);
    sl_dmp_ui_zigbee_permit_join(true);
    sl_zigbee_af_set_default_poll_control_cb(SL_ZIGBEE_AF_SHORT_POLL);  // Use short poll while identifying.
  }
}

/** @brief Stop feedback.
 *
 * This function is called by the Identify plugin when identification is
 * finished. It tells the Identify Feedback plugin to stop providing its
 * implemented feedback functionality.
 *
 * @param endpoint The identifying endpoint Ver.: always
 */
void sl_zigbee_af_identify_stop_feedback_cb(uint8_t endpoint)
{
  (void)endpoint;
  if (identifying) {
    identifying = false;
    sl_zigbee_af_app_println("Stop Identifying");
    sl_dmp_ui_zigbee_permit_join(false);
    sl_zigbee_af_set_default_poll_control_cb(SL_ZIGBEE_AF_LONG_POLL); // Revert to long poll when we stop identifying.
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
  (void)totalBeacons;
  (void)joinAttempts;
  (void)finalState;
  if (status == SL_STATUS_OK) {
    setDefaultReportEntry();
    startIdentifying();
    sl_dmp_ui_display_zigbee_state(DMP_UI_NETWORK_UP);
  } else {
    stopIdentifying();
    sl_dmp_ui_display_zigbee_state(DMP_UI_NO_NETWORK);
  }
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
  (void)manufacturerCode;
  (void)type;
  (void)size;
  (void)value;
  if (clusterId == ZCL_ON_OFF_CLUSTER_ID
      && attributeId == ZCL_ON_OFF_ATTRIBUTE_ID
      && mask == CLUSTER_MASK_SERVER) {
    uint8_t data;
    sl_status_t status = sl_zigbee_af_read_attribute(endpoint,
                                                     ZCL_ON_OFF_CLUSTER_ID,
                                                     ZCL_ON_OFF_ATTRIBUTE_ID,
                                                     CLUSTER_MASK_SERVER,
                                                     (int8u*) &data,
                                                     sizeof(data),
                                                     NULL);

    if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      if (data == 0x00) {
        led_turn_off(LED0);
#if defined(SL_CATALOG_LED1_PRESENT)
        led_turn_off(LED1);
#endif //SL_CATALOG_LED1_PRESENT
        sl_dmp_ui_light_off();
#ifdef SL_CATALOG_ZIGBEE_BLE_EVENT_HANDLER_PRESENT
        zb_ble_dmp_notify_light(DMP_UI_LIGHT_OFF);
#endif
      } else {
        led_turn_on(LED0);
#if defined(SL_CATALOG_LED1_PRESENT)
        led_turn_on(LED1);
#endif //SL_CATALOG_LED1_PRESENT
#ifdef SL_CATALOG_ZIGBEE_BLE_EVENT_HANDLER_PRESENT
        zb_ble_dmp_notify_light(DMP_UI_LIGHT_ON);
#endif
        sl_dmp_ui_light_on();
      }
      if ((sl_dmp_ui_get_light_direction() == DMP_UI_DIRECTION_BLUETOOTH)
          || (sl_dmp_ui_get_light_direction() == DMP_UI_DIRECTION_SWITCH)) {
        sl_dmp_ui_update_direction(sl_dmp_ui_get_light_direction());
      } else {
        sl_dmp_ui_update_direction(DMP_UI_DIRECTION_ZIGBEE);
#ifdef SL_CATALOG_ZIGBEE_BLE_EVENT_HANDLER_PRESENT
        zb_ble_dmp_set_source_address(SwitchEUI);
#endif
      }

      sl_dmp_ui_set_light_direction(DMP_UI_DIRECTION_INVALID);
    }
  }
}

/** @brief Pre Command Received
 *
 * This callback is the second in the Application Framework's message processing
 * chain. At this point in the processing of incoming over-the-air messages, the
 * application has determined that the incoming message is a ZCL command. It
 * parses enough of the message to populate an sl_zigbee_af_cluster_command_t struct. The
 * Application Framework defines this struct value in a local scope to the
 * command processing but also makes it available through a global pointer
 * called sl_zigbee_af_current_command, in app/framework/util/util.c. When command
 * processing is complete, this pointer is cleared.
 */
bool sl_zigbee_af_pre_command_received_cb(sl_zigbee_af_cluster_command_t* cmd)
{
  if ((cmd->commandId == ZCL_ON_COMMAND_ID)
      || (cmd->commandId == ZCL_OFF_COMMAND_ID)
      || (cmd->commandId == ZCL_TOGGLE_COMMAND_ID)) {
    (void) memset(SwitchEUI, 0, SOURCE_ADDRESS_LEN);
    sl_zigbee_lookup_eui64_by_node_id(cmd->source, SwitchEUI);
    sl_zigbee_af_core_println(
      "SWITCH ZCL toggle/on/off EUI [%02X %02X %02X %02X %02X %02X %02X %02X]",
      SwitchEUI[7],
      SwitchEUI[6],
      SwitchEUI[5],
      SwitchEUI[4],
      SwitchEUI[3],
      SwitchEUI[2],
      SwitchEUI[1],
      SwitchEUI[0]);
  }
  return false;
}

/** @brief Trust Center Join
 *
 * This callback is called from within the application framework's
 * implementation of sl_zigbee_internal_trust_center_join_handler or sl_zigbee_ezsp_trust_center_join_handler.
 * This callback provides the same arguments passed to the
 * TrustCenterJoinHandler. For more information about the TrustCenterJoinHandler
 * please see documentation included in stack/include/trust-center.h.
 */
void sl_zigbee_af_trust_center_join_cb(sl_802154_short_addr_t newNodeId,
                                       sl_802154_long_addr_t newNodeEui64,
                                       sl_802154_short_addr_t parentOfNewNode,
                                       sl_zigbee_device_update_t status,
                                       sl_zigbee_join_decision_t decision)
{
  (void)newNodeId;
  (void)parentOfNewNode;
  (void)decision;
  if (status == SL_ZIGBEE_DEVICE_LEFT) {
    for (uint8_t i = 0; i < SL_ZIGBEE_BINDING_TABLE_SIZE; i++) {
      sl_zigbee_binding_table_entry_t entry;
      sl_zigbee_get_binding(i, &entry);
      if ((entry.type == SL_ZIGBEE_UNICAST_BINDING)
          && (entry.clusterId == ZCL_ON_OFF_CLUSTER_ID)
          && ((memcmp(entry.identifier, newNodeEui64, EUI64_SIZE)
               == 0))) {
        sl_zigbee_delete_binding(i);
        sl_zigbee_af_app_println("deleted binding entry: %d", i);
        break;
      }
    }
  }
}

/** @brief
 *
 * Application framework equivalent of ::sl_zigbee_radio_needs_calibrating_handler
 */
void sl_zigbee_af_radio_needs_calibrating_cb(void)
{
  sl_mac_calibrate_current_channel();
}

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
#define BUTTON_LONG_PRESS_TIME_MSEC    3000

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
  static uint16_t buttonPressTime;
  uint16_t currentTime = 0;
  if ( sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    if (SL_SIMPLE_BUTTON_INSTANCE(BUTTON1) == handle) {
      buttonPressTime = halCommonGetInt16uMillisecondTick();
    }
  } else if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED) {
    if (SL_SIMPLE_BUTTON_INSTANCE(BUTTON1) == handle) {
      currentTime = halCommonGetInt16uMillisecondTick();
      lastButton = BUTTON1;
      if ((currentTime - buttonPressTime) > BUTTON_LONG_PRESS_TIME_MSEC) {
        longPress = true;
      }
    } else {
      lastButton = BUTTON0;
    }

    sl_zigbee_wakeup_common_task();
    sl_zigbee_af_event_set_active(&button_event);
  }
}
#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT

//-----------------
// Static functions

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
static sl_802154_long_addr_t lightEUI;

static void toggleOnoffAttribute(void)
{
  sl_status_t status;
  uint8_t data;
  status = sl_zigbee_af_read_attribute(sl_zigbee_af_primary_endpoint(),
                                       ZCL_ON_OFF_CLUSTER_ID,
                                       ZCL_ON_OFF_ATTRIBUTE_ID,
                                       CLUSTER_MASK_SERVER,
                                       (int8u*) &data,
                                       sizeof(data),
                                       NULL);

  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    if (data == 0x00) {
      data = 0x01;
    } else {
      data = 0x00;
    }

    sl_dmp_ui_set_light_direction(DMP_UI_DIRECTION_SWITCH);
    sl_zigbee_af_get_eui64(lightEUI);
#ifdef SL_CATALOG_ZIGBEE_BLE_EVENT_HANDLER_PRESENT
    zb_ble_dmp_set_source_address(lightEUI);
#endif
  } else {
    sl_zigbee_af_app_println("read onoff attr: 0x%02X", status);
  }

  status = sl_zigbee_af_write_attribute(sl_zigbee_af_primary_endpoint(),
                                        ZCL_ON_OFF_CLUSTER_ID,
                                        ZCL_ON_OFF_ATTRIBUTE_ID,
                                        CLUSTER_MASK_SERVER,
                                        (int8u *) &data,
                                        ZCL_BOOLEAN_ATTRIBUTE_TYPE);
  sl_zigbee_af_app_println("write to onoff attr: 0x%02X", status);
}
#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT

static void setDefaultReportEntry(void)
{
  sl_zigbee_af_plugin_reporting_entry_t reportingEntry;
  sl_zigbee_af_clear_report_table_cb();
  reportingEntry.direction = SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED;
  reportingEntry.endpoint = sl_zigbee_af_primary_endpoint();
  reportingEntry.clusterId = ZCL_ON_OFF_CLUSTER_ID;
  reportingEntry.attributeId = ZCL_ON_OFF_ATTRIBUTE_ID;
  reportingEntry.mask = CLUSTER_MASK_SERVER;
  reportingEntry.manufacturerCode = SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE;
  reportingEntry.data.reported.minInterval = 0x0001;
  reportingEntry.data.reported.maxInterval = 0x001E; // 30S report interval for SED.
  reportingEntry.data.reported.reportableChange = 0; // onoff is bool type so it is unused
  sl_zigbee_af_reporting_configure_reported_attribute(&reportingEntry);
}

static bool writeIdentifyTime(uint16_t identifyTime)
{
  sl_zigbee_af_status_t status =
    sl_zigbee_af_write_server_attribute(sl_zigbee_af_primary_endpoint(),
                                        ZCL_IDENTIFY_CLUSTER_ID,
                                        ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                                        (uint8_t *)&identifyTime,
                                        sizeof(identifyTime));

  return (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS);
}

static void startIdentifying(void)
{
  writeIdentifyTime(180);
}

static void stopIdentifying(void)
{
  writeIdentifyTime(0);

  sl_dmp_ui_zigbee_permit_join(false);
}
