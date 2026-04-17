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
#include "zigbee_sleep_config.h"
#include "network-creator.h"
#include "network-creator-security.h"
#include "network-steering.h"
#include "find-and-bind-target.h"
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
#define LIGHT_ENDPOINT           1
#define ZIGBEE_STARTUP_DELAY_MS 40

static sl_zigbee_af_event_t commissioning_led_event;
static sl_zigbee_af_event_t finding_and_binding_event;

#if defined(OPENTHREAD_FTD)
  #include <assert.h>
  #include <openthread-core-config.h>
  #include <openthread/config.h>

  #include <openthread/cli.h>
  #include <openthread/diag.h>
  #include <openthread/tasklet.h>

  #include "openthread-system.h"

static otInstance *     sInstance       = NULL;

/*
 * Provide, if required an "otPlatLog()" function
 */
#if OPENTHREAD_CONFIG_LOG_OUTPUT == OPENTHREAD_CONFIG_LOG_OUTPUT_APP
void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aFormat, ...)
{
  OT_UNUSED_VARIABLE(aLogLevel);
  OT_UNUSED_VARIABLE(aLogRegion);
  OT_UNUSED_VARIABLE(aFormat);

  va_list ap;
  va_start(ap, aFormat);
  otCliPlatLogv(aLogLevel, aLogRegion, aFormat, ap);
  va_end(ap);
}
#endif

void sl_ot_create_instance(void)
{
  #if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
  size_t   otInstanceBufferLength = 0;
  uint8_t *otInstanceBuffer       = NULL;

  // Call to query the buffer size
  (void)otInstanceInit(NULL, &otInstanceBufferLength);

  // Call to allocate the buffer
  otInstanceBuffer = (uint8_t *)malloc(otInstanceBufferLength);
  assert(otInstanceBuffer);

  // Initialize OpenThread with the buffer
  sInstance = otInstanceInit(otInstanceBuffer, &otInstanceBufferLength);
  #else
  sInstance = otInstanceInitSingle();
  #endif
  assert(sInstance);
}

otInstance *otGetInstance(void)
{
  return sInstance;
}

#define OT_NWK_UP (otThreadGetDeviceRole(sInstance) != OT_DEVICE_ROLE_DISABLED)
#else
#define OT_NWK_UP false
#endif //#if defined(OPENTHREAD_FTD)

//---------------
// Event handlers

static void commissioning_led_event_handler(sl_zigbee_af_event_t *event)
{
  (void)event;
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

static void finding_and_binding_event_handler(sl_zigbee_af_event_t *event)
{
  (void)event;
  if (sl_zigbee_af_network_state() == SL_ZIGBEE_JOINED_NETWORK) {
    sl_zigbee_af_event_set_inactive(&finding_and_binding_event);

    sl_zigbee_app_debug_println("Find and bind target start: 0x%02X",
                                sl_zigbee_af_find_and_bind_target_start(LIGHT_ENDPOINT));
  }
}

//----------------------
// Implemented Callbacks

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action. The framework
 * will always process the stack status after the callback returns.
 */
bool zb_nwk_up = false;
void sl_zigbee_af_stack_status_cb(sl_status_t status)
{
  // Note, the ZLL state is automatically updated by the stack and the plugin.
  if (status == SL_STATUS_NETWORK_DOWN) {
    led_turn_off(COMMISSIONING_STATUS_LED);
    zb_nwk_up = false;
  } else if (status == SL_STATUS_NETWORK_UP) {
    led_turn_on(COMMISSIONING_STATUS_LED);
    sl_zigbee_af_event_set_active(&finding_and_binding_event);
    zb_nwk_up = true;
  }
}

/** @brief Init
 * Application init function
 */
void sl_zigbee_af_main_init_cb(void)
{
  sl_zigbee_af_event_init(&commissioning_led_event, commissioning_led_event_handler);
  sl_zigbee_af_isr_event_init(&finding_and_binding_event, finding_and_binding_event_handler);
}

/** @brief Complete network steering.
 *
 * This callback is fired when the Network Steering plugin is complete.
 *
 * @param status On success this will be set to SL_STATUS_OK to indicate a
 * network was joined successfully. On failure this will be the status code of
 * the last join or scan attempt.
 *
 * @param totalBeacons The total number of 802.15.4 beacons that were heard,
 * including beacons from different devices with the same PAN ID.
 *
 * @param joinAttempts The number of join attempts that were made to get onto
 * an open Zigbee network.
 *
 * @param finalState The finishing state of the network steering process. From
 * this, one is able to tell on which channel mask and with which key the
 * process was complete.
 */
void sl_zigbee_af_network_steering_complete_cb(sl_status_t status,
                                               uint8_t totalBeacons,
                                               uint8_t joinAttempts,
                                               uint8_t finalState)
{
  (void)totalBeacons;
  (void)joinAttempts;
  (void)finalState;
  sl_zigbee_app_debug_println("Join network complete: 0x%02X", status);
}

/** @brief Complete the network creation process.
 *
 * This callback notifies the user that the network creation process has
 * completed successfully.
 *
 * @param network The network that the network creator plugin successfully
 * formed.
 *
 * @param usedSecondaryChannels Whether or not the network creator wants to
 * form a network on the secondary channels.
 */
void sl_zigbee_af_network_creator_complete_cb(const sl_zigbee_network_parameters_t *network,
                                              bool usedSecondaryChannels)
{
  (void)network;
  (void)usedSecondaryChannels;
  sl_zigbee_app_debug_println("Form Network Complete: 0x%02X",
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
  (void)manufacturerCode;
  (void)type;
  (void)size;
  (void)value;
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

/** @brief On/off Cluster Server Post Init
 *
 * Following resolution of the On/Off state at startup for this endpoint, perform any
 * additional initialization needed; e.g., synchronize hardware state.
 *
 * @param endpoint Endpoint that is being initialized
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

/** @brief
 *
 * Application framework equivalent of ::sl_zigbee_radio_needs_calibrating_handler
 */
void sl_zigbee_af_radio_needs_calibrating_cb(void)
{
  sl_mac_calibrate_current_channel();
}

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT) && (SL_ZIGBEE_APP_FRAMEWORK_USE_BUTTON_TO_STAY_AWAKE == 0)
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"

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
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED) {
    sl_zigbee_af_event_set_active(&finding_and_binding_event);
  }
}
#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT && SL_ZIGBEE_APP_FRAMEWORK_USE_BUTTON_TO_STAY_AWAKE == 0

//Internal testing stuff
#if defined(SL_ZIGBEE_TEST)
void sl_zigbee_af_hal_button_isr_cb(uint8_t button, uint8_t state)
{
  UNUSED_VAR(button);
  if (state == BUTTON_RELEASED) {
    sl_zigbee_af_event_set_active(&finding_and_binding_event);
  }
}
#endif // EBER_TEST

void sl_rail_mux_invalid_rx_channel_detected_cb(int new_rx_channel, int old_rx_channel)
{
  if (zb_nwk_up || OT_NWK_UP) {
    sl_zigbee_af_core_println("WARNING: MUX detected invalid RX attempts on both %d and %d channels",
                              new_rx_channel, old_rx_channel);
    EFM_ASSERT(false);
  }
}

#ifdef SL_CATALOG_BLUETOOTH_PRESENT

//------------------------------------------------------------------------------
// Bluetooth Event handler

#include "zigbee_app_framework_event.h"
#include "sl_zigbee_system_common.h"
#include "sl_bluetooth.h"
#include "sl_bluetooth_advertiser_config.h"
#include "sl_bluetooth_connection_config.h"
#include "sl_component_catalog.h"
static uint8_t cli_adv_handle;
static uint8_t activeBleConnections = 0;
void zb_ble_dmp_print_ble_address(uint8_t *address)
{
  sl_zigbee_app_debug_print("\nBLE address: [%02X %02X %02X %02X %02X %02X]\n",
                            address[5], address[4], address[3],
                            address[2], address[1], address[0]);
}
struct {
  bool inUse;
  bool isMaster;
  uint8_t connectionHandle;
  uint8_t bondingHandle;
  uint8_t remoteAddress[6];
} bleConnectionTable[SL_BT_CONFIG_MAX_CONNECTIONS];

void bleConnectionInfoTableInit(void)
{
  uint8_t i;
  for (i = 0; i < SL_BT_CONFIG_MAX_CONNECTIONS; i++) {
    bleConnectionTable[i].inUse = false;
  }
}
uint8_t bleConnectionInfoTableFindUnused(void)
{
  uint8_t i;
  for (i = 0; i < SL_BT_CONFIG_MAX_CONNECTIONS; i++) {
    if (!bleConnectionTable[i].inUse) {
      return i;
    }
  }
  return 0xFF;
}

bool bleConnectionInfoTableIsEmpty(void)
{
  uint8_t i;
  for (i = 0; i < SL_BT_CONFIG_MAX_CONNECTIONS; i++) {
    if (bleConnectionTable[i].inUse) {
      return false;
    }
  }
  return true;
}

uint8_t bleConnectionInfoTableLookup(uint8_t connHandle)
{
  uint8_t i;
  for (i = 0; i < SL_BT_CONFIG_MAX_CONNECTIONS; i++) {
    if (bleConnectionTable[i].inUse
        && bleConnectionTable[i].connectionHandle == connHandle) {
      return i;
    }
  }
  return 0xFF;
}

void bleConnectionInfoTablePrintEntry(uint8_t index)
{
  assert(index < SL_BT_CONFIG_MAX_CONNECTIONS
         && bleConnectionTable[index].inUse);
  sl_zigbee_app_debug_println("**** Connection Info index[%d]****", index);
  sl_zigbee_app_debug_println("connection handle 0x%02X",
                              bleConnectionTable[index].connectionHandle);
  sl_zigbee_app_debug_println("bonding handle = 0x%02X",
                              bleConnectionTable[index].bondingHandle);
  sl_zigbee_app_debug_println("local node is %s",
                              (bleConnectionTable[index].isMaster) ? "master" : "slave");
  sl_zigbee_app_debug_print("remote address: ");
  zb_ble_dmp_print_ble_address(bleConnectionTable[index].remoteAddress);
  sl_zigbee_app_debug_println("");
  sl_zigbee_app_debug_println("*************************");
}

void sl_bt_on_event(sl_bt_msg_t* evt)
{
  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_system_boot_id: {
      bd_addr ble_address;
      uint8_t type;
      sl_status_t status = sl_bt_system_hello();
      sl_zigbee_app_debug_println("BLE hello: %s",
                                  (status == SL_STATUS_OK) ? "success" : "error");

      #define SCAN_WINDOW 5
      #define SCAN_INTERVAL 10

      status = sl_bt_scanner_set_parameters(sl_bt_scanner_scan_mode_active,
                                            (uint16_t)SCAN_INTERVAL,
                                            (uint16_t)SCAN_WINDOW);

      status = sl_bt_system_get_identity_address(&ble_address, &type);
      zb_ble_dmp_print_ble_address(ble_address.addr);

      status = sl_bt_advertiser_create_set(&cli_adv_handle);
      if (status) {
        sl_zigbee_app_debug_println("sl_bt_advertiser_create_set status 0x%02x", status);
      }
    }
    break;

    case sl_bt_evt_connection_opened_id: {
      sl_zigbee_app_debug_println("sl_bt_evt_connection_opened_id \n");
      sl_bt_evt_connection_opened_t *conn_evt =
        (sl_bt_evt_connection_opened_t*) &(evt->data);
      uint8_t index = bleConnectionInfoTableFindUnused();
      if (index == 0xFF) {
        sl_zigbee_app_debug_println("MAX active BLE connections");
        assert(index < 0xFF);
      } else {
        bleConnectionTable[index].inUse = true;
        bleConnectionTable[index].isMaster = (conn_evt->role > 0);
        bleConnectionTable[index].connectionHandle = conn_evt->connection;
        bleConnectionTable[index].bondingHandle = conn_evt->bonding;
        (void) memcpy(bleConnectionTable[index].remoteAddress,
                      conn_evt->address.addr, 6);

        activeBleConnections++;
        sl_bt_connection_set_preferred_phy(conn_evt->connection, sl_bt_test_phy_1m, 0xff);
        sl_zigbee_app_debug_println("BLE connection opened");
        bleConnectionInfoTablePrintEntry(index);
        sl_zigbee_app_debug_println("%d active BLE connection",
                                    activeBleConnections);
      }
    }
    break;
    case sl_bt_evt_connection_phy_status_id: {
      sl_bt_evt_connection_phy_status_t *conn_evt =
        (sl_bt_evt_connection_phy_status_t *)&(evt->data);
      // indicate the PHY that has been selected
      sl_zigbee_app_debug_println("now using the %dMPHY\r\n",
                                  conn_evt->phy);
    }
    break;
    case sl_bt_evt_connection_closed_id: {
      sl_bt_evt_connection_closed_t *conn_evt =
        (sl_bt_evt_connection_closed_t*) &(evt->data);

      uint8_t index = bleConnectionInfoTableLookup(conn_evt->connection);
      assert(index < 0xFF);

      bleConnectionTable[index].inUse = false;
      if ( activeBleConnections ) {
        --activeBleConnections;
      }

      sl_zigbee_app_debug_println(
        "BLE connection closed, handle=0x%02x, reason=0x%02x : [%d] active BLE connection",
        conn_evt->connection, conn_evt->reason, activeBleConnections);
    }
    break;

    case sl_bt_evt_scanner_legacy_advertisement_report_id: {
      sl_zigbee_app_debug_print("Scan response, address type=0x%02x",
                                evt->data.evt_scanner_legacy_advertisement_report.address_type);
      zb_ble_dmp_print_ble_address(evt->data.evt_scanner_legacy_advertisement_report.address.addr);
      sl_zigbee_app_debug_println("");
    }
    break;

    case sl_bt_evt_connection_parameters_id: {
      sl_bt_evt_connection_parameters_t* param_evt =
        (sl_bt_evt_connection_parameters_t*) &(evt->data);
      sl_zigbee_app_debug_println(
        "BLE connection parameters are updated, handle=0x%02x, interval=0x%02x, latency=0x%02x, timeout=0x%02x, security=0x%02x",
        param_evt->connection,
        param_evt->interval,
        param_evt->latency,
        param_evt->timeout,
        param_evt->security_mode);
    }
    break;

    case sl_bt_evt_gatt_service_id: {
      sl_bt_evt_gatt_service_t* service_evt =
        (sl_bt_evt_gatt_service_t*) &(evt->data);
      uint8_t i;
      sl_zigbee_app_debug_println(
        "GATT service, conn_handle=0x%02x, service_handle=0x%04x",
        service_evt->connection, service_evt->service);
      sl_zigbee_app_debug_print("UUID=[");
      for (i = 0; i < service_evt->uuid.len; i++) {
        sl_zigbee_app_debug_print("0x%04x ", service_evt->uuid.data[i]);
      }
      sl_zigbee_app_debug_println("]");
    }
    break;

    default:
      break;
  }
}
void zb_ble_dmp_print_ble_connections(void)
{
  uint8_t i;
  if (bleConnectionInfoTableIsEmpty()) {
    sl_zigbee_app_debug_println("No BLE connections.");
  } else {
    for (i = 0; i < SL_BT_CONFIG_MAX_CONNECTIONS; i++) {
      if (bleConnectionTable[i].inUse) {
        bleConnectionInfoTablePrintEntry(i);
      }
    }
  }
}
#endif //SL_CATALOG_BLUETOOTH_PRESENT
