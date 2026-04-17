/***************************************************************************//**
 * @file app.c
 * @brief Callbacks implementation and application specific code.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "app/framework/plugin/network-creator-security/network-creator-security.h"
#include "app/framework/plugin/network-creator/network-creator.h"
#include "app/framework/plugin/reporting/reporting.h"
#include "stack/include/zigbee-security-manager.h"
#ifdef SL_CATALOG_ZIGBEE_DIRECT_TUNNELING_PRESENT
#include "zigbee_direct_tunneling.h"
#endif //SL_CATALOG_ZIGBEE_DIRECT_TUNNELING_PRESENT
#include "zigbee_direct_common.h"
#include "zigbee_direct_tlv.h"
#include "sl_component_catalog.h"
#include "stack/config/sl_zigbee_token_defines.h"

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
static sl_zigbee_af_event_t button_event;
#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT

#if defined(SL_CATALOG_LED0_PRESENT)
#include "sl_led.h"
#include "sl_simple_led_instances.h"
#define led_turn_on(led) sl_led_turn_on(led)
#define led_turn_off(led) sl_led_turn_off(led)
#define led_toggle(led) sl_led_toggle(led)
#define LED0     (&sl_led_led0)
#define LED1     (&sl_led_led1)
#else // !SL_CATALOG_LED0_PRESENT
#define led_turn_on(led)
#define led_turn_off(led)
#define led_toggle(led)
#endif // SL_CATALOG_LED0_PRESENT

#define SOURCE_ADDRESS_LEN 8
static sl_802154_long_addr_t SwitchEUI;

//---------------------
// Forward declarations
static bool startPjoinAndIdentifying(uint16_t identifyTime);
#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
static void toggleOnoffAttribute(void);
#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT

extern uint8_t adv_handle[];
extern sl_zigbee_network_parameters_t sli_zigbee_direct_network_params;

uint32_t sl_zigbee_af_zigbee_direct_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                                 sl_service_function_context_t *context);

//--------------
// Event handler

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
static void button_event_handler(sl_zigbee_af_event_t *event)
{
  (void)event;
  sl_status_t status = 0;

  if (lastButton == BUTTON0) {
    toggleOnoffAttribute();
  } else if (lastButton == BUTTON1) {
    sl_zigbee_network_status_t state = sl_zigbee_af_network_state();
    if (state != SL_ZIGBEE_JOINED_NETWORK) {
      sl_dmp_ui_display_zigbee_state(DMP_UI_FORMING);
      status = sl_zigbee_af_network_creator_start(true); // centralized
      sl_zigbee_core_debug_print("%s network %s: 0x%02X\n", "Form centralized", "start", status);
    } else {
      // joined on NWK
      if (longPress == false) {
        if (sl_zigbee_get_permit_joining()) {
          sl_zigbee_permit_joining(0);
        } else {
          if (startPjoinAndIdentifying(180)) {
            sl_dmp_ui_zigbee_permit_join(true);
            sl_zigbee_core_debug_print("pJoin for 180 sec: 0x%02X\n", status);
          }
        }
      } else {
        status = sl_zigbee_leave_network(SL_ZIGBEE_LEAVE_NWK_WITH_NO_OPTION);
        sl_zigbee_clear_binding_table();
        sl_zigbee_core_debug_print("leave NWK: 0x%02X\n", status);
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
extern void bleConnectionInfoTableInit(void);
extern uint8_t sl_my_passcode[SL_ZIGBEE_ENCRYPTION_KEY_SIZE];
void sl_zigbee_af_main_init_cb(void)
{
  sl_status_t init_status;
  #if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
  sl_zigbee_af_isr_event_init(&button_event, button_event_handler);
  #endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT

  bleConnectionInfoTableInit();
  sli_zigbee_direct_security_init();

  sli_zigbee_direct_network_params.panId = sl_zigbee_get_pseudo_random_number();

  sl_zigbee_af_event_init(&sli_zigbee_direct_anonymous_join_event, sli_zigbee_direct_anonymous_join_event_handler);

  (void)sl_token_manager_get_data(COMMON_TOKEN_PLUGIN_ZDD_JOIN_TIMEOUT,
                                  (void *)&sl_zigbee_direct_anonymous_join_timeout_sec,
                                  sizeof(uint32_t));

  sl_zigbee_af_event_set_delay_ms(&sli_zigbee_direct_anonymous_join_event, sl_zigbee_direct_anonymous_join_timeout_sec * 1000);

  init_status = sl_zigbee_subscribe_to_zcl_commands(0x003d, 0xFFFF, ZCL_DIRECTION_CLIENT_TO_SERVER, sl_zigbee_af_zigbee_direct_cluster_server_command_parse);
  sl_zigbee_core_debug_println("Cluster Command parse init %02X", init_status);
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
        led_turn_off(LED1);
        sl_dmp_ui_light_off();
      } else {
        led_turn_on(LED0);
        led_turn_on(LED1);
        sl_dmp_ui_light_on();
      }
      if ((sl_dmp_ui_get_light_direction() == DMP_UI_DIRECTION_BLUETOOTH)
          || (sl_dmp_ui_get_light_direction() == DMP_UI_DIRECTION_SWITCH)) {
        sl_dmp_ui_update_direction(sl_dmp_ui_get_light_direction());
      } else {
        sl_dmp_ui_update_direction(DMP_UI_DIRECTION_ZIGBEE);
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
extern bool sl_zigbee_af_pre_command_received_callback_z_d_d(sl_zigbee_af_cluster_command_t* cmd);
bool sl_zigbee_af_pre_command_received_cb(sl_zigbee_af_cluster_command_t* cmd)
{
  if ((cmd->commandId == ZCL_ON_COMMAND_ID)
      || (cmd->commandId == ZCL_OFF_COMMAND_ID)
      || (cmd->commandId == ZCL_TOGGLE_COMMAND_ID)) {
    (void) memset(SwitchEUI, 0, SOURCE_ADDRESS_LEN);
    sl_zigbee_lookup_eui64_by_node_id(cmd->source, SwitchEUI);
    // Only print when rx prints are enabled
    if (sl_zigbee_af_print_received_messages) {
      sl_zigbee_app_debug_print(
        "SWITCH ZCL toggle/on/off EUI [%02X %02X %02X %02X %02X %02X %02X %02X]\n",
        SwitchEUI[7],
        SwitchEUI[6],
        SwitchEUI[5],
        SwitchEUI[4],
        SwitchEUI[3],
        SwitchEUI[2],
        SwitchEUI[1],
        SwitchEUI[0]);
    }
  }
  return sl_zigbee_af_pre_command_received_callback_z_d_d(cmd);
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
        sl_zigbee_core_debug_print("deleted binding entry: %d\n", i);
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
static void toggleOnoffAttribute(void)
{
  static sl_802154_long_addr_t lightEUI;
  sl_status_t status;
  int8u data;
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
  } else {
    sl_zigbee_core_debug_print("read onoff attr: 0x%02X\n", status);
  }

  status = sl_zigbee_af_write_attribute(sl_zigbee_af_primary_endpoint(),
                                        ZCL_ON_OFF_CLUSTER_ID,
                                        ZCL_ON_OFF_ATTRIBUTE_ID,
                                        CLUSTER_MASK_SERVER,
                                        (int8u *) &data,
                                        ZCL_BOOLEAN_ATTRIBUTE_TYPE);
  sl_zigbee_core_debug_print("write to onoff attr: 0x%02X\n", status);
}
#endif // SL_CATALOG_SIMPLE_BUTTON_PRESENT

static bool startPjoinAndIdentifying(uint16_t identifyTime)
{
  sl_status_t status;
  sl_802154_long_addr_t wildcardEui64 = SL_ZIGBEE_NULL_EUI64;
  sl_zigbee_sec_man_key_t centralizedKey = ZIGBEE_3_CENTRALIZED_SECURITY_LINK_KEY;

  sl_zigbee_sec_man_import_transient_key(wildcardEui64, &centralizedKey);
  status = sl_zigbee_permit_joining(identifyTime);

  sl_zigbee_af_write_server_attribute(sl_zigbee_af_primary_endpoint(),
                                      ZCL_IDENTIFY_CLUSTER_ID,
                                      ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                                      (uint8_t *) &identifyTime,
                                      sizeof(identifyTime));
  return (status == SL_STATUS_OK);
}
