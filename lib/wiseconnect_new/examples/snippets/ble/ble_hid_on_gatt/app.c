/*******************************************************************************
* @file  ble_hid.c
* @brief 
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

/**
 * Include files
 * */

//! SL Wi-Fi SDK includes
#include "sl_board_configuration.h"
#include "sl_constants.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "cmsis_os2.h"
#include "sl_utility.h"
#include "FreeRTOSConfig.h"
//! BLE include file to refer BLE APIs
#include "ble_config.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"

//! Common include file
#include "rsi_common_apis.h"
#include <string.h>
#if SL_SI91X_TICKLESS_MODE == 0 && defined(SLI_SI91X_MCU_INTERFACE)
#include "sl_si91x_m4_ps.h"
#include "sl_si91x_power_manager.h"
#endif
//! [ble_gls] is a tag for every print
#ifdef SL_SI91X_PRINT_DBG_LOG
#include <stdio.h>
#define LOG_SCAN(fmt, args...) scanf(fmt, ##args)
#else
#define LOG_SCAN(fmt, args...)
#endif

//! Specify GATT role client/server
#define SERVER    0
#define CLIENT    1
#define GATT_ROLE SERVER

//! remote device address
#define RANDOM_ADDRESS                 1
#define PUBLIC_ADDRESS                 0
#define RSI_BLE_REMOTE_BD_ADDRESS      "04:D4:C4:9A:F3:CC"
#define RSI_BLE_REMOTE_BD_ADDRESS_TYPE RANDOM_ADDRESS

//! local device name
#define RSI_BLE_APP_HIDS "HID_OVER_GATT"

//! Remote Device Name to connect
#define RSI_REMOTE_DEVICE_NAME "SILABS_DEV"
#define MAX_DEVICE_NAME_LENGTH 31

#define RSI_BLE_SMP_PASSKEY 0x000000

//! attribute properties
#define RSI_BLE_ATT_PROP_RD         0x02
#define RSI_BLE_ATT_PROP_WR_NO_RESP 0x04
#define RSI_BLE_ATT_PROP_WR         0x08
#define RSI_BLE_ATT_PROP_NOTIFY     0x10
#define RSI_BLE_ATT_PROP_INDICATE   0x20

//! Configuration bitmap for attributes
#define RSI_BLE_ATT_MAINTAIN_IN_HOST BIT(0)
#define RSI_BLE_ATT_SECURITY_ENABLE  BIT(1)

#define RSI_BLE_ATT_CONFIG_BITMAP (RSI_BLE_ATT_MAINTAIN_IN_HOST)

//! application event list
#define RSI_APP_EVENT_ADV_REPORT              0x00
#define RSI_BLE_EVENT_CONN                    0x01
#define RSI_BLE_EVENT_DISCONN                 0x02
#define RSI_BLE_EVENT_GATT_WR                 0x03
#define RSI_BLE_EVENT_GATT_RD                 0x04
#define RSI_BLE_EVENT_GATT_PROFILE_RESP       0x05
#define RSI_BLE_EVENT_GATT_CHAR_SERVICES_RESP 0x06
#define RSI_BLE_EVENT_GATT_CHAR_DESC_RESP     0x07
#define RSI_BLE_EVENT_MTU                     0x08
#define RSI_BLE_EVENT_SMP_REQ                 0x09
#define RSI_BLE_EVENT_SMP_RESP                0x0a
#define RSI_BLE_EVENT_SMP_PASSKEY             0x0b
#define RSI_BLE_EVENT_SMP_FAILED              0x0c
#define RSI_BLE_EVENT_SMP_ENCRYPT_STARTED     0x0d
#define RSI_BLE_EVENT_SMP_PASSKEY_DISPLAY     0x0e
#define RSI_BLE_EVENT_LTK_REQ                 0x0f
#define RSI_BLE_SC_PASSKEY_EVENT              0x10
#define RSI_BLE_GATT_SEND_DATA                0x11
#define RSI_BLE_GATT_ERROR                    0x12

#define GATT_READ_RESP      0x00
#define GATT_READ_BLOB_RESP 0x01

#define GATT_READ_ZERO_OFFSET 0x00

/*=======================================================================*/
//!    Powersave configurations
/*=======================================================================*/
#define ENABLE_NWP_POWER_SAVE 0 //! Set to 1 for powersave mode

#if ENABLE_NWP_POWER_SAVE
//! Power Save Profile Mode
#define PSP_MODE RSI_SLEEP_MODE_2
//! Power Save Profile type
#define PSP_TYPE RSI_MAX_PSP

sl_wifi_performance_profile_v2_t wifi_profile = { .profile = ASSOCIATED_POWER_SAVE_LOW_LATENCY };
#endif

#define MITM_REQ 0x01

//! BLE attribute service types uuid values
#define RSI_BLE_CHAR_SERV_UUID                 0x2803
#define RSI_BLE_CLIENT_CHAR_UUID               0x2902
#define RSI_BLE_REPORT_REFERENCE_UUID          0x2908
#define RSI_BLE_EXT_REPORT_REFERENCE_UUID      0x2907
#define RSI_BLE_CHAR_PRESENTATION_FORMATE_UUID 0x2904
//! BLE characteristic service uuid
#define RSI_BLE_GAP_SERVICE_UUID  0x1800
#define RSI_BLE_GATT_SERVICE_UUID 0x1801

#define RSI_BLE_DEVICE_NAME_UUID 0x2A00
#define RSI_BLE_APPEARANCE_UUID  0x2A01
#define RSI_BLE_PPC_UUID         0x2A04

#define RSI_BLE_SERVICE_CHANGED_UUID 0x2A05

//! BLE characteristic service uuid
#define RSI_BLE_HID_SERVICE_UUID                0x1812
#define RSI_BLE_HID_PROTOCOL_MODE_UUID          0x2A4E
#define RSI_BLE_HID_REPORT_UUID                 0x2A4D
#define RSI_BLE_HID_REPORT_MAP_UUID             0x2A4B
#define RSI_BLE_HID_BOOT_MOUSE_IN_REPORT        0x2A33
#define RSI_BLE_HID_INFO_UUID                   0x2A4A
#define RSI_BLE_HID_CONTROL_POINT_UUID          0x2A4C
#define RSI_BLE_HID_PNP_TYPE                    0x2a50
#define RSI_BLE_HID_BATTERY_SERVICE             0x180F
#define RSI_BLE_HID_BATTERY_LEVEL               0x2A19
#define RSI_BLE_HID_BOOT_KEYBOARD_INPUT_REPORT  0x2A22
#define RSI_BLE_HID_BOOT_KEYBOARD_OUTPUT_REPORT 0x2A32

#define RSI_BLE_DEVICE_INFORMATION 0x180A
#define RSI_MANIFACTURE_NAME       0x2A29

//! hid protocol mode
#define PROTOCOL_MODE_BOOT_UP   0x00
#define PROTOCOL_MODE_REPORT    0x01
#define HID_KDB_IN_RPT_DATA_LEN 0x0B
#define BD_ADDR_ARRAY_LEN       0x18

#define REPORT_IN_NOTIFY_ENABLE 0
#define CONNECTED               1
#define ENCRYPT_EN              2
#define ADVERTISE               3
#define SCAN                    4

#define MIN_CONN_INTERVAL 36
#define MAX_CONN_INTERVAL 36
#define CONN_LATENCY      0
//! global parameters list
static volatile uint32_t ble_app_event_map;
static volatile uint32_t ble_app_event_map1;

static rsi_ble_event_conn_status_t conn_event_to_app;
static rsi_ble_event_disconnect_t disconn_event_to_app;

static rsi_ble_event_write_t app_ble_write_event;
static rsi_ble_read_req_t app_ble_read_event;

static uint32_t numeric_value;
static rsi_bt_event_encryption_enabled_t glbl_enc_enabled;
static rsi_bt_event_le_ltk_request_t temp_le_ltk_req;
static rsi_ble_event_mtu_t app_ble_mtu_event;
static rsi_bt_event_smp_passkey_display_t smp_passkey_display_event;

static uint16_t rsi_ble_hid_in_report_val_hndl;
static uint8_t remote_dev_addr[BD_ADDR_ARRAY_LEN];
static uint8_t remote_dev_bd_addr[6];
static uint8_t remote_addr_type                    = 0;
static uint8_t remote_name[MAX_DEVICE_NAME_LENGTH] = { 0 };
static uint8_t device_found                        = 0;
static uint32_t smp_passkey                        = 0;
uint16_t att_resp_status                           = 0;
uint8_t passkey[BLE_PASSKEY_SIZE]                  = { 0 };

uint8_t app_state                         = 0;
uint8_t str_remote_address[18]            = { '\0' };
uint16_t desc_range[10]                   = { 0 };
uint8_t desc_handle_index                 = 0;
uint8_t desc_handle_index_1               = 0;
uint8_t hid_data[HID_KDB_IN_RPT_DATA_LEN] = { 0 };

typedef struct rsi_ble_att_list_s {
  uint32_t uuid;
  uint16_t handle;
  uint16_t len;
  void *value;
} rsi_ble_att_list_t;

typedef struct rsi_ble_hid_info_s {
#define BLE_ATT_REC_SIZE      1024
#define BLE_ATT_REC_LIST_SIZE 5
  uint8_t data[BLE_ATT_REC_SIZE];
  uint16_t data_ix;
  uint16_t att_rec_list_count;
  rsi_ble_att_list_t att_rec_list[BLE_ATT_REC_LIST_SIZE];
} rsi_ble_hid_info_t;

static rsi_ble_hid_info_t hid_info_g;
void rsi_ble_hid_srv_gatt_wr_cb(void);
#if (GATT_ROLE == SERVER)
static const uint8_t hid_report_map[] = {
  0x05,
  0x01, // USAGE_PAGE (Generic Desktop)
  0x09,
  0x06, // USAGE (Keyboard)
  0xa1,
  0x01, // COLLECTION (Application)

  //0x85, 0x01,                    //   REPORT_ID
  0x05,
  0x07, //   USAGE_PAGE (Keyboard)
  0x19,
  0xe0, //   USAGE_MINIMUM (Keyboard LeftControl)
  0x29,
  0xe7, //   USAGE_MAXIMUM (Keyboard Right GUI)
  0x15,
  0x00, //   LOGICAL_MINIMUM (0)
  0x25,
  0x01, //   LOGICAL_MAXIMUM (1)
  0x75,
  0x01, //   REPORT_SIZE (1)
  0x95,
  0x08, //   REPORT_COUNT (8)
  0x81,
  0x02, //   INPUT (Data,Var,Abs)

  0x95,
  0x01, //   REPORT_COUNT (1)
  0x75,
  0x08, //   REPORT_SIZE (8)
  0x81,
  0x03, //   INPUT (Cnst,Var,Abs)

  0x95,
  0x05, //   REPORT_COUNT (5)
  0x75,
  0x01, //   REPORT_SIZE (1)
  0x05,
  0x08, //   USAGE_PAGE (LEDs)
  0x19,
  0x01, //   USAGE_MINIMUM (Num Lock)
  0x29,
  0x05, //   USAGE_MAXIMUM (Kana)
  0x91,
  0x02, //   OUTPUT (Data,Var,Abs)

  0x95,
  0x01, //   REPORT_COUNT (1)
  0x75,
  0x03, //   REPORT_SIZE (3)
  0x91,
  0x03, //   OUTPUT (Cnst,Var,Abs)

  0x95,
  0x06, //   REPORT_COUNT (6)
  0x75,
  0x08, //   REPORT_SIZE (8)
  0x15,
  0x00, //   LOGICAL_MINIMUM (0)
  0x25,
  0x65, //   LOGICAL_MAXIMUM (101)
  0x05,
  0x07, //   USAGE_PAGE (Keyboard)
  0x19,
  0x00, //   USAGE_MINIMUM (Reserved (no event indicated))
  0x29,
  0x65, //   USAGE_MAXIMUM (Keyboard Application)
  0x81,
  0x00, //   INPUT (Data,Ary,Abs)
  //0x05, 0x01,	//Usage Minimum (Reserved (no event indicated))
  //0x05, 0x01,	//Usage Maximum (Keyboard Application)
  //0x05, 0x01,	//Input (Data,Array) Key arrays (6 bytes)
  0xc0 // END_COLLECTION

  /*==============================================*/
};
#endif
rsi_ble_event_profile_by_uuid_t ble_servs;
rsi_ble_event_read_by_type1_t char_servs;
rsi_ble_event_gatt_desc_t att_desc;

osSemaphoreId_t ble_main_task_sem;

static const sl_wifi_device_configuration_t config = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode       = SL_SI91X_CLIENT_MODE,
                   .coex_mode       = SL_SI91X_WLAN_BLE_MODE,
                   .feature_bit_map = (SL_WIFI_FEAT_WPS_DISABLE | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
                                       | SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1),
                   .tcp_ip_feature_bit_map =
                     (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map =
                     (SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
#ifdef SLI_SI917
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      | (SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE)),
                   .bt_feature_bit_map         = (SL_SI91X_BT_RF_TYPE | SL_SI91X_ENABLE_BLE_PROTOCOL),
                   .ext_tcp_ip_feature_bit_map = (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
                   //!ENABLE_BLE_PROTOCOL in bt_feature_bit_map
                   .ble_feature_bit_map =
                     ((SL_SI91X_BLE_MAX_NBR_PERIPHERALS(RSI_BLE_MAX_NBR_PERIPHERALS)
                       | SL_SI91X_BLE_MAX_NBR_CENTRALS(RSI_BLE_MAX_NBR_CENTRALS)
                       | SL_SI91X_BLE_MAX_NBR_ATT_SERV(RSI_BLE_MAX_NBR_ATT_SERV)
                       | SL_SI91X_BLE_MAX_NBR_ATT_REC(RSI_BLE_MAX_NBR_ATT_REC))
                      | SL_SI91X_FEAT_BLE_CUSTOM_FEAT_EXTENTION_VALID | SL_SI91X_BLE_PWR_INX(RSI_BLE_PWR_INX)
                      | SL_SI91X_BLE_PWR_SAVE_OPTIONS(RSI_BLE_PWR_SAVE_OPTIONS)
                      | SL_SI91X_916_BLE_COMPATIBLE_FEAT_ENABLE
#if RSI_BLE_GATT_ASYNC_ENABLE
                      | SL_SI91X_BLE_GATT_ASYNC_ENABLE
#endif
                      ),
                   .ble_ext_feature_bit_map =
                     ((SL_SI91X_BLE_NUM_CONN_EVENTS(RSI_BLE_NUM_CONN_EVENTS)
                       | SL_SI91X_BLE_NUM_REC_BYTES(RSI_BLE_NUM_REC_BYTES))
#if RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST
                      | SL_SI91X_BLE_INDICATE_CONFIRMATION_FROM_HOST //indication response from app
#endif
#if RSI_BLE_MTU_EXCHANGE_FROM_HOST
                      | SL_SI91X_BLE_MTU_EXCHANGE_FROM_HOST //MTU Exchange request initiation from app
#endif
#if RSI_BLE_SET_SCAN_RESP_DATA_FROM_HOST
                      | (SL_SI91X_BLE_SET_SCAN_RESP_DATA_FROM_HOST) //Set SCAN Resp Data from app
#endif
#if RSI_BLE_DISABLE_CODED_PHY_FROM_HOST
                      | (SL_SI91X_BLE_DISABLE_CODED_PHY_FROM_HOST) //Disable Coded PHY from app
#endif
#if BLE_SIMPLE_GATT
                      | SL_SI91X_BLE_GATT_INIT
#endif
                      ),
                   .config_feature_bit_map = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP | SL_WIFI_ENABLE_ENHANCED_MAX_PSP) }
};

const osThreadAttr_t thread_attributes = {
  .name       = "application_thread",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityNormal,
  .tz_module  = 0,
  .reserved   = 0,
};
/*==============================================*/
/**
 * @fn         rsi_ble_app_init_events
 * @brief      initializes the event parameter.
 * @param[in]  none.
 * @return     none.
 * @section description
 * This function is used during BLE initialization.
 */
static void rsi_ble_app_init_events()
{
  ble_app_event_map  = 0;
  ble_app_event_map1 = 0;
  return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_set_event
 * @brief      set the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to set/raise the specific event.
 */
void rsi_ble_app_set_event(uint32_t event_num)
{
  if (event_num < 32) {
    ble_app_event_map |= BIT(event_num);
  } else {
    ble_app_event_map1 |= BIT((event_num - 32));
  }

  osSemaphoreRelease(ble_main_task_sem);

  return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_clear_event
 * @brief      clears the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to clear the specific event.
 */
static void rsi_ble_app_clear_event(uint32_t event_num)
{
  if (event_num < 32) {
    ble_app_event_map &= ~BIT(event_num);
  } else {
    ble_app_event_map1 &= ~BIT((event_num - 32));
  }

  return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_get_event
 * @brief      returns the first set event based on priority
 * @param[in]  none.
 * @return     int32_t
 *             > 0  = event number
 *             -1   = not received any event
 * @section description
 * This function returns the highest priority event among all the set events
 */
static int32_t rsi_ble_app_get_event(void)
{
  uint32_t ix;

  for (ix = 0; ix < 64; ix++) {
    if (ix < 32) {
      if (ble_app_event_map & (1 << ix)) {
        return ix;
      }
    } else {
      if (ble_app_event_map1 & (1 << (ix - 32))) {
        return ix;
      }
    }
  }

  return (-1);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_adv_report_event
 * @brief      invoked when advertise report event is received
 * @param[in]  adv_report, pointer to the received advertising report
 * @return     none.
 * @section description
 * This callback function updates the scanned remote devices list
 */
void rsi_ble_on_adv_report_event(rsi_ble_event_adv_report_t *adv_report)
{
  static uint8_t remote_dev_addr[BD_ADDR_ARRAY_LEN] = { 0 };

  if (device_found == 1) {
    return;
  }
  memset(remote_name, 0, MAX_DEVICE_NAME_LENGTH);
  BT_LE_ADPacketExtract(remote_name, adv_report->adv_data, adv_report->adv_data_len);

  remote_addr_type = adv_report->dev_addr_type;
  rsi_6byte_dev_address_to_ascii(remote_dev_addr, (uint8_t *)adv_report->dev_addr);
  memcpy(remote_dev_bd_addr, (uint8_t *)adv_report->dev_addr, 6);

  if (((device_found == 0) && ((strcmp((const char *)remote_name, RSI_REMOTE_DEVICE_NAME)) == 0))
      || ((remote_addr_type == RSI_BLE_REMOTE_BD_ADDRESS_TYPE)
          && ((strcmp((const char *)remote_dev_addr, RSI_BLE_REMOTE_BD_ADDRESS) == 0)))) {
    device_found = 1;
    rsi_ble_app_set_event(RSI_APP_EVENT_ADV_REPORT);
  }
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_enhance_conn_status_event
 * @brief      invoked when enhanced connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
void rsi_ble_on_enhance_conn_status_event(rsi_ble_event_enhance_conn_status_t *resp_enh_conn)
{
  conn_event_to_app.dev_addr_type = resp_enh_conn->dev_addr_type;
  memcpy(conn_event_to_app.dev_addr, resp_enh_conn->dev_addr, RSI_DEV_ADDR_LEN);
  conn_event_to_app.status = resp_enh_conn->status;
  rsi_ble_app_set_event(RSI_BLE_EVENT_CONN);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_connect_event
 * @brief      invoked when connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
static void rsi_ble_on_connect_event(rsi_ble_event_conn_status_t *resp_conn)
{
  LOG_PRINT("conn \n");
  memcpy(&conn_event_to_app, resp_conn, sizeof(rsi_ble_event_conn_status_t));
  rsi_ble_app_set_event(RSI_BLE_EVENT_CONN);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_disconnect_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This callback function indicates disconnected device information and status
 */
static void rsi_ble_on_disconnect_event(rsi_ble_event_disconnect_t *resp_disconnect, uint16_t reason)
{
  UNUSED_PARAMETER(reason); //This statement is added only to resolve compilation warning, value is unchanged
  memcpy(&disconn_event_to_app, resp_disconnect, sizeof(rsi_ble_event_disconnect_t));
  rsi_ble_app_set_event(RSI_BLE_EVENT_DISCONN);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_gatt_write_event
 * @brief      its invoked when write/notify/indication events are received.
 * @param[in]  event_id contains the gatt_write event id (RSI_BLE_EVENT_GATT_WRITE).
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write/notify/indication events are received
 */
static void rsi_ble_on_gatt_write_event(uint16_t event_id, rsi_ble_event_write_t *rsi_ble_write)
{
  UNUSED_PARAMETER(event_id); //This statement is added only to resolve compilation warning, value is unchanged
  LOG_PRINT("gatt wr \n");
  memcpy(&app_ble_write_event, rsi_ble_write, sizeof(rsi_ble_event_write_t));
#if (GATT_ROLE == SERVER)
  rsi_ble_hid_srv_gatt_wr_cb();
#endif
  rsi_ble_app_set_event(RSI_BLE_EVENT_GATT_WR);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_read_req_event
 * @brief      its invoked when gatt read events are received.
 * @param[in]  event_id contains the gatt_read_req_event id
 * @param[in]  rsi_ble_read, read event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when a gatt read request event is received
 */
static void rsi_ble_on_read_req_event(uint16_t event_id, rsi_ble_read_req_t *rsi_ble_read_req)
{
  UNUSED_PARAMETER(event_id); //This statement is added only to resolve compilation warning, value is unchanged
  LOG_PRINT("gatt rd \n");
  memcpy(&app_ble_read_event, rsi_ble_read_req, sizeof(rsi_ble_read_req_t));
  rsi_ble_app_set_event(RSI_BLE_EVENT_GATT_RD);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_profiles_event
 * @brief      invoked when the specific service details are received for
 *             supported specific services.
 * @param[out] p_ble_resp_profile, specific profile details
 * @return     none
 * @section description
 * This is a callback function
 */
static void rsi_ble_on_profiles_event(uint16_t resp_status, rsi_ble_event_profile_by_uuid_t *rsi_ble_resp_profile)
{
  att_resp_status = resp_status;
  memcpy(&ble_servs, rsi_ble_resp_profile, sizeof(rsi_ble_event_profile_by_uuid_t));
  rsi_ble_app_set_event(RSI_BLE_EVENT_GATT_PROFILE_RESP);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_char_services_event
 * @brief      invoked when response is received for characteristic services details
 * @param[out] p_ble_resp_char_services, list of characteristic services.
 * @return     none
 * @section description
 */
static void rsi_ble_on_char_services_event(uint16_t resp_status,
                                           rsi_ble_event_read_by_type1_t *rsi_ble_resp_char_services)
{
  att_resp_status = resp_status;
  memcpy(&char_servs, rsi_ble_resp_char_services, sizeof(rsi_ble_event_read_by_type1_t));
  rsi_ble_app_set_event(RSI_BLE_EVENT_GATT_CHAR_SERVICES_RESP);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_att_desc_event
 * @brief      invoked when response is received for attribute descriptor
 * @param[in]  event_id, it indicates wr/noti/indi event id.
 * @param[in]  rsi_ble_resp_att_desc, wr/noti/indi event parameters.
 * @return     none.
 * @section description
 * this callback function is invoked when response is received for attribute descriptor
 */
static void ble_on_att_desc_event(uint16_t resp_status, rsi_ble_event_gatt_desc_t *rsi_ble_resp_att_desc)
{
  att_resp_status = resp_status;
  memcpy(&att_desc, rsi_ble_resp_att_desc, sizeof(rsi_ble_event_gatt_desc_t));
  rsi_ble_app_set_event(RSI_BLE_EVENT_GATT_CHAR_DESC_RESP);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_request 
 * @brief      its invoked when an smp request event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when an SMP request event is received(we are in central mode)
 * Note: Peripheral requested to start SMP request, we have to send SMP request command
 */
static void rsi_ble_on_smp_request(rsi_bt_event_smp_req_t *remote_dev_address)
{
  memcpy(remote_dev_bd_addr, remote_dev_address->dev_addr, 6);
  LOG_PRINT("smp req\n");
  rsi_ble_app_set_event(RSI_BLE_EVENT_SMP_REQ);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_response 
 * @brief      its invoked when an smp response event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when an SMP response event is received(we are in Peripheral mode)
 * Note: Central initiated SMP protocol, we have to send SMP response command
 */
static void rsi_ble_on_smp_response(rsi_bt_event_smp_resp_t *remote_dev_address)
{
  memcpy(remote_dev_bd_addr, remote_dev_address->dev_addr, 6);
  LOG_PRINT("smp resp\n");
  rsi_ble_app_set_event(RSI_BLE_EVENT_SMP_RESP);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_passkey 
 * @brief      its invoked when an smp passkey event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when an SMP passkey event is received
 * Note: We have to send SMP passkey command
 */
static void rsi_ble_on_smp_passkey(rsi_bt_event_smp_passkey_t *remote_dev_address)
{
  memcpy(remote_dev_bd_addr, remote_dev_address->dev_addr, 6);
  LOG_PRINT("smp passkey\n");
  rsi_ble_app_set_event(RSI_BLE_EVENT_SMP_PASSKEY);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_passkey_display 
 * @brief      its invoked when an smp passkey event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when an SMP passkey event is received
 * Note: We have to send SMP passkey command
 */
static void rsi_ble_on_smp_passkey_display(rsi_bt_event_smp_passkey_display_t *smp_passkey_display)
{
  memcpy(&smp_passkey_display_event, smp_passkey_display, sizeof(rsi_bt_event_smp_passkey_display_t));
  memcpy(passkey, smp_passkey_display_event.passkey, BLE_PASSKEY_SIZE);
  rsi_ble_app_set_event(RSI_BLE_EVENT_SMP_PASSKEY_DISPLAY);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_failed 
 * @brief      its invoked when an smp failed event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when an SMP failed event is received
 */
static void rsi_ble_on_smp_failed(uint16_t status, rsi_bt_event_smp_failed_t *remote_dev_address)
{
  //This statement is added only to resolve compilation warning, value is unchanged
  UNUSED_PARAMETER(remote_dev_address);
  UNUSED_PARAMETER(status);
  rsi_ble_app_set_event(RSI_BLE_EVENT_SMP_FAILED);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_encrypt_started 
 * @brief      its invoked when an encryption started event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when an encryption started event is received
 */
static void rsi_ble_on_encrypt_started(uint16_t resp_status, rsi_bt_event_encryption_enabled_t *enc_enabled)
{
  UNUSED_PARAMETER(resp_status);
  memcpy(&glbl_enc_enabled, enc_enabled, sizeof(rsi_bt_event_encryption_enabled_t));
  rsi_ble_app_set_event(RSI_BLE_EVENT_SMP_ENCRYPT_STARTED);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_le_ltk_req_event
 * @brief      invoked when ltk request event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This callback function indicates linkkey request information
 */
static void rsi_ble_on_le_ltk_req_event(rsi_bt_event_le_ltk_request_t *le_ltk_req)
{
  LOG_PRINT("smp ltk req\n");
  memcpy(&temp_le_ltk_req, le_ltk_req, sizeof(rsi_bt_event_le_ltk_request_t));
  rsi_ble_app_set_event(RSI_BLE_EVENT_LTK_REQ);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_mtu_event
 * @brief      invoked  when an MTU size event is received
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when an MTU size event is received
 */
static void rsi_ble_on_mtu_event(rsi_ble_event_mtu_t *rsi_ble_mtu)
{
  memcpy(&app_ble_mtu_event, rsi_ble_mtu, sizeof(rsi_ble_event_mtu_t));
  rsi_ble_app_set_event(RSI_BLE_EVENT_MTU);
}

#if (GATT_ROLE == SERVER)
/*==============================================*/
/**
 * @fn         rsi_gatt_add_att_to_list
 * @brief      This function is used to store characteristic service attribute.
 * @param[in]  p_hid_info, pointer to hid info structure
 * @param[in]  handle, characteristic service attribute handle.
 * @param[in]  data_len, characteristic value length
 * @param[in]  data, characteristic value pointer
 * @param[in]  uuid, characteristic value uuid
 * @return     none.
 * @section description
 * This function is used to store all attribute records  
 */
static void rsi_gatt_add_att_to_list(rsi_ble_hid_info_t *p_hid_info,
                                     uint16_t handle,
                                     uint16_t data_len,
                                     uint8_t *data,
                                     uint32_t uuid)
{
  LOG_PRINT("gatt add att to list\n");
  if ((p_hid_info->data_ix + data_len) >= BLE_ATT_REC_SIZE) {
    LOG_PRINT("no data memory for att rec values");
    return;
  }

  p_hid_info->att_rec_list[p_hid_info->att_rec_list_count].handle = handle;
  p_hid_info->att_rec_list[p_hid_info->att_rec_list_count].len    = data_len;
  p_hid_info->att_rec_list[p_hid_info->att_rec_list_count].value  = p_hid_info->data + p_hid_info->data_ix;
  memcpy(p_hid_info->data + p_hid_info->data_ix, data, data_len);
  p_hid_info->att_rec_list[p_hid_info->att_rec_list_count].uuid = uuid;
  p_hid_info->att_rec_list_count++;
  p_hid_info->data_ix += data_len;

  return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_add_char_serv_att
 * @brief      this function is used to add characteristic service attribute..
 * @param[in]  serv_handler, service handler.
 * @param[in]  handle, characteristic service attribute handle.
 * @param[in]  val_prop, characteristic value property.
 * @param[in]  att_val_handle, characteristic value handle
 * @param[in]  att_val_uuid, characteristic value uuid
 * @return     none.
 * @section description
 * This function is used at application to add characteristic attribute
 */
static void rsi_ble_add_char_serv_att(void *serv_handler,
                                      uint16_t handle,
                                      uint8_t val_prop,
                                      uint16_t att_val_handle,
                                      uuid_t att_val_uuid)
{
  rsi_ble_req_add_att_t new_att = { 0 };

  //! preparing the attribute service structure
  new_att.serv_handler       = serv_handler;
  new_att.handle             = handle;
  new_att.att_uuid.size      = 2;
  new_att.att_uuid.val.val16 = RSI_BLE_CHAR_SERV_UUID;
  new_att.property           = RSI_BLE_ATT_PROP_RD;

  //! preparing the characteristic attribute value
  new_att.data_len = 6;
  new_att.data[0]  = val_prop;
  rsi_uint16_to_2bytes(&new_att.data[2], att_val_handle);
  rsi_uint16_to_2bytes(&new_att.data[4], att_val_uuid.val.val16);

  //! Add attribute to the service
  rsi_ble_add_attribute(&new_att);

  return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_add_char_val_att
 * @brief      this function is used to add characteristic value attribute.
 * @param[in]  serv_handler, new service handler.
 * @param[in]  handle, characteristic value attribute handle.
 * @param[in]  att_type_uuid, attribute uuid value.
 * @param[in]  val_prop, characteristic value property.
 * @param[in]  data, characteristic value data pointer.
 * @param[in]  data_len, characteristic value length.
 * @return     none.
 * @section description
 * This function is used at application to create new service.
 */
static int32_t rsi_ble_add_char_val_att(rsi_ble_hid_info_t *p_hid_info,
                                        void *serv_handler,
                                        uint16_t handle,
                                        uuid_t att_type_uuid,
                                        uint8_t val_prop,
                                        uint8_t *data,
                                        uint16_t data_len,
                                        uint8_t config_bitmap)
{
  int32_t status;
  rsi_ble_req_add_att_t new_att = { 0 };

  //! preparing the attributes
  new_att.serv_handler  = serv_handler;
  new_att.handle        = handle;
  new_att.config_bitmap = config_bitmap;

  memcpy(&new_att.att_uuid, &att_type_uuid, sizeof(uuid_t));
  new_att.property = val_prop;

  //! preparing the attribute value
  new_att.data_len = data_len;
  memcpy(new_att.data, data, RSI_MIN(sizeof(new_att.data), data_len));

  //! add attribute to the service
  status = rsi_ble_add_attribute(&new_att);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n add attribute failed = %lx \n", status);
    return status;
  }

  if (((config_bitmap & BIT(0)) == 1) || (data_len > 20)) {
    if (!p_hid_info) {
      LOG_PRINT("\n HID INFO is not available \n");
      return RSI_FAILURE;
    }
    rsi_gatt_add_att_to_list(p_hid_info, handle, data_len, data, att_type_uuid.val.val32);
  }

  //! check the attribute property with notification
  if ((val_prop & RSI_BLE_ATT_PROP_NOTIFY) || (val_prop & RSI_BLE_ATT_PROP_INDICATE)) {
    //! if notification property supports then we need to add client characteristic service.

    //! preparing the client characteristic attribute & values
    memset(&new_att, 0, sizeof(rsi_ble_req_add_att_t));
    new_att.serv_handler       = serv_handler;
    new_att.handle             = handle + 1;
    new_att.att_uuid.size      = 2;
    new_att.att_uuid.val.val16 = RSI_BLE_CLIENT_CHAR_UUID;
    new_att.property           = RSI_BLE_ATT_PROP_RD | RSI_BLE_ATT_PROP_WR | RSI_BLE_ATT_PROP_WR_NO_RESP;
    new_att.data_len           = 2;

    //! add attribute to the service
    rsi_ble_add_attribute(&new_att);
  }

  return RSI_SUCCESS;
}

/*==============================================*/
/**
 * @fn         rsi_ble_add_hid_serv
 * @brief      this function is used to add new servcie i.e.., hids.
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to create new service.
 */
static uint32_t rsi_ble_add_hid_serv(rsi_ble_hid_info_t *p_hid_info)
{
  uint8_t len                           = 0;
  uint8_t data[31]                      = { 0 };
  uuid_t new_uuid                       = { 0 };
  uint8_t hdl_offset                    = 0;
  rsi_ble_resp_add_serv_t new_serv_resp = { 0 };

  //! adding new service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_DEVICE_INFORMATION;
  rsi_ble_add_service(new_uuid, &new_serv_resp);
  hdl_offset += 1;

  //!Device info
  //! adding characteristic service attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_MANIFACTURE_NAME;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + hdl_offset,
                            RSI_BLE_ATT_PROP_RD,
                            new_serv_resp.start_handle + hdl_offset + 1,
                            new_uuid);

  //! adding characteristic value attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_MANIFACTURE_NAME;
  strcpy((char *)data, "Silicon Labs");
  len = 12;
  rsi_ble_add_char_val_att(p_hid_info,
                           new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + hdl_offset + 1,
                           new_uuid,
                           RSI_BLE_ATT_PROP_RD,
                           (uint8_t *)data,
                           len,
                           0);
  hdl_offset += 2;

  //! PnP ID
  //! adding characteristic service attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_PNP_TYPE;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + hdl_offset,
                            RSI_BLE_ATT_PROP_RD,
                            new_serv_resp.start_handle + hdl_offset + 1,
                            new_uuid);

  //! adding characteristic value attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_PNP_TYPE;
  uint8_t pnp[31]    = { 0x02, 0xC4, 0x10, 0x01, 0x00, 0x01, 0x00 };
  len                = 7;
  rsi_ble_add_char_val_att(p_hid_info,
                           new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + hdl_offset + 1,
                           new_uuid,
                           RSI_BLE_ATT_PROP_RD,
                           (uint8_t *)pnp,
                           len,
                           0);
  hdl_offset += 2;

  //! adding new service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_BATTERY_SERVICE;
  rsi_ble_add_service(new_uuid, &new_serv_resp);
  //!Device info
  //! adding characteristic service attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_BATTERY_LEVEL;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + hdl_offset,
                            RSI_BLE_ATT_PROP_RD,
                            new_serv_resp.start_handle + hdl_offset + 1,
                            new_uuid);

  //! adding characteristic value attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_BATTERY_LEVEL;
  data[0]            = 0x64;
  len                = 1;
  rsi_ble_add_char_val_att(p_hid_info,
                           new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + hdl_offset + 1,
                           new_uuid,
                           RSI_BLE_ATT_PROP_RD,
                           (uint8_t *)data,
                           len,
                           0);
  hdl_offset += 2;

  //! adding new service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_SERVICE_UUID;
  rsi_ble_add_service(new_uuid, &new_serv_resp);
  //! Protocol mode
  //! adding characteristic service attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_PROTOCOL_MODE_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + hdl_offset,
                            RSI_BLE_ATT_PROP_RD | RSI_BLE_ATT_PROP_WR_NO_RESP,
                            new_serv_resp.start_handle + hdl_offset + 1,
                            new_uuid);

  //! adding characteristic value attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_PROTOCOL_MODE_UUID;
  data[0]            = PROTOCOL_MODE_REPORT;
  len                = 1;
  rsi_ble_add_char_val_att(p_hid_info,
                           new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + hdl_offset + 1,
                           new_uuid,
                           RSI_BLE_ATT_PROP_RD | RSI_BLE_ATT_PROP_WR_NO_RESP,
                           (uint8_t *)data,
                           len,
                           SEC_MODE_1_LEVEL_3);
  hdl_offset += 2;

  //! Keyboard input report 1
  //! adding characteristic service attribute to the service
  rsi_ble_hid_in_report_val_hndl = new_serv_resp.start_handle + hdl_offset + 1;
  new_uuid.size                  = 2;
  new_uuid.val.val16             = RSI_BLE_HID_REPORT_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + hdl_offset,
                            RSI_BLE_ATT_PROP_RD | RSI_BLE_ATT_PROP_NOTIFY,
                            new_serv_resp.start_handle + hdl_offset + 1,
                            new_uuid);

  //! adding characteristic value attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_REPORT_UUID;
  memset(data, 0, HID_KDB_IN_RPT_DATA_LEN);
  len = HID_KDB_IN_RPT_DATA_LEN;
  rsi_ble_add_char_val_att(p_hid_info,
                           new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + hdl_offset + 1,
                           new_uuid,
                           RSI_BLE_ATT_PROP_RD | RSI_BLE_ATT_PROP_NOTIFY,
                           (uint8_t *)data,
                           len,
                           SEC_MODE_1_LEVEL_3);

  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_REPORT_REFERENCE_UUID;
  data[0]            = 0;
  data[1]            = 1;
  len                = 2;
  rsi_ble_add_char_val_att(p_hid_info,
                           new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + hdl_offset + 3,
                           new_uuid,
                           RSI_BLE_ATT_PROP_RD,
                           (uint8_t *)data,
                           len,
                           SEC_MODE_1_LEVEL_3);
  hdl_offset += 4;

  //! BOOT KEYBOARD INPUT
  //! adding characteristic service attribute to the service

  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_BOOT_KEYBOARD_INPUT_REPORT;
  len                = HID_KDB_IN_RPT_DATA_LEN;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + hdl_offset,
                            RSI_BLE_ATT_PROP_RD | RSI_BLE_ATT_PROP_NOTIFY,
                            new_serv_resp.start_handle + hdl_offset + 1,
                            new_uuid);

  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_BOOT_KEYBOARD_INPUT_REPORT;
  memset(data, 0, 0x08);
  len = 0x08;
  rsi_ble_add_char_val_att(p_hid_info,
                           new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + hdl_offset + 1,
                           new_uuid,
                           RSI_BLE_ATT_PROP_RD | RSI_BLE_ATT_PROP_NOTIFY,
                           (uint8_t *)data,
                           len,
                           SEC_MODE_1_LEVEL_3);
  hdl_offset += 3;

  //! BOOT KEYBORAD OUTPUT
  //! adding characteristic service attribute to the service

  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_BOOT_KEYBOARD_OUTPUT_REPORT;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + hdl_offset,
                            RSI_BLE_ATT_PROP_RD | RSI_BLE_ATT_PROP_WR | RSI_BLE_ATT_PROP_WR_NO_RESP,
                            new_serv_resp.start_handle + hdl_offset + 1,
                            new_uuid);

  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_BOOT_KEYBOARD_OUTPUT_REPORT;
  memset(data, 0, 0x08);
  len = 0x08;
  rsi_ble_add_char_val_att(p_hid_info,
                           new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + hdl_offset + 1,
                           new_uuid,
                           RSI_BLE_ATT_PROP_RD | RSI_BLE_ATT_PROP_WR | RSI_BLE_ATT_PROP_WR_NO_RESP,
                           (uint8_t *)data,
                           len,
                           SEC_MODE_1_LEVEL_3);
  hdl_offset += 2;

  //! Keyboard output report 1
  //! adding characteristic service attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_REPORT_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + hdl_offset,
                            RSI_BLE_ATT_PROP_RD | RSI_BLE_ATT_PROP_WR | RSI_BLE_ATT_PROP_WR_NO_RESP,
                            new_serv_resp.start_handle + hdl_offset + 1,
                            new_uuid);

  //! adding characteristic value attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_REPORT_UUID;
  memset(data, 0, HID_KDB_IN_RPT_DATA_LEN);
  len = HID_KDB_IN_RPT_DATA_LEN;
  rsi_ble_add_char_val_att(p_hid_info,
                           new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + hdl_offset + 1,
                           new_uuid,
                           RSI_BLE_ATT_PROP_RD | RSI_BLE_ATT_PROP_WR | RSI_BLE_ATT_PROP_WR_NO_RESP,
                           (uint8_t *)data,
                           len,
                           SEC_MODE_1_LEVEL_3);

  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_REPORT_REFERENCE_UUID;
  data[0]            = 0;
  data[1]            = 2;
  len                = 2;
  rsi_ble_add_char_val_att(p_hid_info,
                           new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + hdl_offset + 2,
                           new_uuid,
                           RSI_BLE_ATT_PROP_RD,
                           (uint8_t *)data,
                           len,
                           SEC_MODE_1_LEVEL_3);

  hdl_offset += 3;
  //Keyboard Output Report 1
  //! Report map
  //! adding characteristic service attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_REPORT_MAP_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + hdl_offset,
                            RSI_BLE_ATT_PROP_RD,
                            new_serv_resp.start_handle + hdl_offset + 1,
                            new_uuid);

  //! adding characteristic value attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_REPORT_MAP_UUID;
  len                = sizeof(hid_report_map);
  rsi_ble_add_char_val_att(p_hid_info,
                           new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + hdl_offset + 1,
                           new_uuid,
                           RSI_BLE_ATT_PROP_RD,
                           (uint8_t *)hid_report_map,
                           len,
                           SEC_MODE_1_LEVEL_3 | RSI_BLE_ATT_MAINTAIN_IN_HOST);
  hdl_offset += 2;

  //! HID info
  //! adding characteristic service attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_INFO_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + hdl_offset,
                            RSI_BLE_ATT_PROP_RD,
                            new_serv_resp.start_handle + hdl_offset + 1,
                            new_uuid);

  //! adding characteristic value attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_INFO_UUID;
  data[0]            = 1;
  data[1]            = 0x11;
  data[2]            = 0;
  data[3]            = 2;
  len                = 4;
  rsi_ble_add_char_val_att(p_hid_info,
                           new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + hdl_offset + 1,
                           new_uuid,
                           RSI_BLE_ATT_PROP_RD,
                           (uint8_t *)data,
                           len,
                           SEC_MODE_1_LEVEL_3);
  hdl_offset += 2;

  //! control point
  //! adding characteristic service attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_CONTROL_POINT_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + hdl_offset,
                            RSI_BLE_ATT_PROP_WR_NO_RESP,
                            new_serv_resp.start_handle + hdl_offset + 1,
                            new_uuid);

  //! adding characteristic value attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_HID_CONTROL_POINT_UUID;
  data[0]            = 0;
  len                = 1;
  rsi_ble_add_char_val_att(p_hid_info,
                           new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + hdl_offset + 1,
                           new_uuid,
                           RSI_BLE_ATT_PROP_WR_NO_RESP,
                           (uint8_t *)data,
                           len,
                           SEC_MODE_1_LEVEL_3);
  hdl_offset += 2;
  return RSI_SUCCESS;
}

/*=============================================*/
/**
 * @fn         rsi_ble_hid_srv_gatt_wr_cb 
 * @brief      handles server side write events.
 * @param[in]  none
 * @return     none
 * @section description
 * handles server side write events and set respective flags.
 */
void rsi_ble_hid_srv_gatt_wr_cb(void)
{
  LOG_PRINT("wr handle\n");
  LOG_PRINT("report handle is:%d\n", rsi_ble_hid_in_report_val_hndl);
  LOG_PRINT("app event handle is:%d\n", *app_ble_write_event.handle);
  if ((rsi_ble_hid_in_report_val_hndl + 1) == *((uint16_t *)app_ble_write_event.handle)) {
    //! 0x01 for notification
    if (app_ble_write_event.att_value[0] == 0x01) {
      //! set a bit
      app_state |= BIT(REPORT_IN_NOTIFY_ENABLE);
      rsi_ble_app_set_event(RSI_BLE_GATT_SEND_DATA);
    } else if (app_ble_write_event.att_value[0] == 0) {
      //! clear a bit
      app_state &= ~BIT(REPORT_IN_NOTIFY_ENABLE);
      rsi_ble_app_clear_event(RSI_BLE_GATT_SEND_DATA);
    }
    LOG_PRINT("Input report notify val: %lu\n", app_state & BIT(REPORT_IN_NOTIFY_ENABLE));
  }
}
#endif

void rsi_ble_on_sc_passkey(rsi_bt_event_sc_passkey_t *sc_passkey)
{
  memcpy(remote_dev_bd_addr, sc_passkey->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(remote_dev_addr, sc_passkey->dev_addr);
  numeric_value = sc_passkey->passkey;
  rsi_ble_app_set_event(RSI_BLE_SC_PASSKEY_EVENT);
}

int8_t hid_kbd_in_rpt_send(uint8_t len, uint8_t *p_data)
{
  rsi_ble_set_local_att_value(rsi_ble_hid_in_report_val_hndl, len, p_data);

  return 0;
}

/*==============================================*/
/**
 * @fn         rsi_ble_gatt_error_event
 * @brief      Callback invoked when a GATT error response is received.
 * @param[in]  resp_status           Status code of the error response.
 * @param[in]  rsi_ble_gatt_error    Pointer to the error response event structure.
 * @return     none
 * @section description
 * This function is called by the BLE stack when a GATT error response is received from the remote device.
 * It can be used to handle GATT protocol errors, such as invalid handle, read/write not permitted, etc.
 */
static void rsi_ble_gatt_error_event(uint16_t resp_status, rsi_ble_event_error_resp_t *rsi_ble_gatt_error)
{
  UNUSED_PARAMETER(resp_status); //This statement is added only to resolve compilation warning, value is unchanged
  memcpy(remote_dev_bd_addr, rsi_ble_gatt_error->dev_addr, 6);
  rsi_ble_app_set_event(RSI_BLE_GATT_ERROR);
}

/*==============================================*/
/**
 * @fn         ble_hid_gatt_application
 * @brief      this is the application of ble hid application.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This function is used to test the ble hid application.
 */
void ble_hids_gatt_application(rsi_ble_hid_info_t *p_hid_info)
{
  int32_t status = 0;
  int32_t event_id, i;
  uint8_t local_dev_addr[BD_ADDR_ARRAY_LEN]           = { 0 };
  uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN] = { 0 };
  sl_wifi_firmware_version_t fw_version               = { 0 };

#if (GATT_ROLE == SERVER)
  uint8_t scan_data_len = 0;
  uint8_t scan_data[32] = { 0 };
  uint8_t adv_data_len  = 0;
  uint8_t adv_data[32]  = {
     0x02, 0x01, 0x05, 0x03, 0x19, 0xC0, 0x03, /* Appearance */
     0x03, 0x03, 0x12, 0x18                    /*0x0F, 0x18, 0x0A, 0x18,  Service UUIDs */
  };

  /* Name */
  adv_data[11] = strlen(RSI_BLE_APP_HIDS) + 1;
  adv_data[12] = 0x09;
  strcpy((char *)&adv_data[13], (char *)RSI_BLE_APP_HIDS);

  adv_data_len = strlen(RSI_BLE_APP_HIDS) + 13;
#elif (GATT_ROLE == CLIENT)
  uuid_t service_uuid;
  // rsi_ble_event_profile_by_uuid_t ble_servs         = { 0 };
  // rsi_ble_resp_char_services_t char_servs = { 0 };
  // rsi_ble_resp_att_descs_t att_desc       = { 0 };
#endif

  status = sl_wifi_init(&config, NULL, sl_wifi_default_event_handler);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\n Wi-Fi Initialization Failed, Error Code : 0x%lX\r\n", status);
    return;
  } else {
    LOG_PRINT(" Wi-Fi Initialization Success\n");
  }

  status = sl_wifi_get_firmware_version(&fw_version);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nFirmware version Failed, Error Code : 0x%lX\r\n", status);
  } else {
    print_firmware_version(&fw_version);
  }

#if (GATT_ROLE == SERVER)
  //! adding ble hid service
  rsi_ble_add_hid_serv(p_hid_info);
#endif

  //! registering the GAP callback functions
  rsi_ble_gap_register_callbacks(rsi_ble_on_adv_report_event,
                                 rsi_ble_on_connect_event,
                                 rsi_ble_on_disconnect_event,
                                 NULL,
                                 NULL,
                                 NULL,
                                 rsi_ble_on_enhance_conn_status_event,
                                 NULL,
                                 NULL,
                                 NULL);

  //! registering the GATT callback functions
  rsi_ble_gatt_register_callbacks(NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  rsi_ble_on_gatt_write_event,
                                  NULL,
                                  NULL,
                                  rsi_ble_on_read_req_event,
                                  rsi_ble_on_mtu_event,
                                  rsi_ble_gatt_error_event,
                                  ble_on_att_desc_event,
                                  NULL,
                                  rsi_ble_on_profiles_event,
                                  rsi_ble_on_char_services_event,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL);

  //! registering the SMP callback functions
  rsi_ble_smp_register_callbacks(rsi_ble_on_smp_request,
                                 rsi_ble_on_smp_response,
                                 rsi_ble_on_smp_passkey,
                                 rsi_ble_on_smp_failed,
                                 rsi_ble_on_encrypt_started,
                                 rsi_ble_on_smp_passkey_display,
                                 rsi_ble_on_sc_passkey,
                                 rsi_ble_on_le_ltk_req_event,
                                 NULL,
                                 NULL,
                                 NULL);
  //! create ble main task if ble protocol is selected
  ble_main_task_sem = osSemaphoreNew(1, 0, NULL);
  if (ble_main_task_sem == NULL) {
    LOG_PRINT("Failed to create ble_main_task_sem semaphore\n");
    return;
  }

  //!  initializing the application events map
  rsi_ble_app_init_events();

  //! Set local name
  rsi_bt_set_local_name((uint8_t *)RSI_BLE_APP_HIDS);

  //! get the local device MAC address.
  status = rsi_bt_get_local_device_address(rsi_app_resp_get_dev_addr);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Get local device address failed = %lx\r\n", status);
    return;
  } else {
    rsi_6byte_dev_address_to_ascii(local_dev_addr, rsi_app_resp_get_dev_addr);
    LOG_PRINT("\r\n Local device address %s \r\n", local_dev_addr);
  }

#if (GATT_ROLE == SERVER)
  //! set advertise data
  rsi_ble_set_advertise_data(adv_data, adv_data_len);
  rsi_ble_set_scan_response_data(scan_data, scan_data_len);
  rsi_ble_set_smp_pairing_capabilty_data_t no_signing_keys_supported_capabilities;

  no_signing_keys_supported_capabilities.io_capability        = RSI_BLE_SMP_IO_CAPABILITY;
  no_signing_keys_supported_capabilities.oob_data_flag        = LOCAL_OOB_DATA_FLAG_NOT_PRESENT;
  no_signing_keys_supported_capabilities.auth_req             = AUTH_REQ_BITS;
  no_signing_keys_supported_capabilities.enc_key_size         = MAXIMUM_ENC_KEY_SIZE_16;
  no_signing_keys_supported_capabilities.ini_key_distribution = INITIATOR_KEYS_TO_DIST;
  no_signing_keys_supported_capabilities.rsp_key_distribution = RESPONDER_KEYS_TO_DIST;
  status = rsi_ble_set_smp_pairing_cap_data(&no_signing_keys_supported_capabilities);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n smp_pairing_cap_data cmd failed = %lX", status);
    return;
  }

  LOG_PRINT("\n Start advertising ...\n");
  //! set device in advertising mode.
  rsi_ble_start_advertising();

  app_state |= BIT(ADVERTISE);
#endif

#if (GATT_ROLE == CLIENT)
  //! start scanning
  status = rsi_ble_start_scanning();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n Start scanning failed, error status: %lx\n", status);
    return;
  }
  LOG_PRINT("\n Start scanning ...\n");
#endif

#if ENABLE_NWP_POWER_SAVE
  LOG_PRINT("\r\n keep module in to power save \r\n");
  //! initiating power save in BLE mode
  status = rsi_bt_power_save_profile(PSP_MODE, PSP_TYPE);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Failed to initiate power save in BLE mode \r\n");
    return;
  }

  //! initiating power save in wlan mode
  status = sl_wifi_set_performance_profile_v2(&wifi_profile);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\n Failed to initiate power save in Wi-Fi mode :%lx\r\n", status);
    return;
  }

  LOG_PRINT("\r\n Module is in power save \r\n");
#endif

  //! waiting for events from controller.
  while (1) {
    //! Application main loop
    //! checking for events list
    event_id = rsi_ble_app_get_event();
    if (event_id == -1) {
#if ((SL_SI91X_TICKLESS_MODE == 0) && SLI_SI91X_MCU_INTERFACE && ENABLE_NWP_POWER_SAVE)
      //! if events are not received loop will be continued.
      if ((!(P2P_STATUS_REG & TA_wakeup_M4)) && (ble_app_event_map == 0) && (ble_app_event_map1 == 0)) {
        P2P_STATUS_REG &= ~M4_wakeup_TA;
        sl_si91x_power_manager_sleep();
      }
#else
      osSemaphoreAcquire(ble_main_task_sem, osWaitForever);
#endif
      continue;
    }
    switch (event_id) {
#if (GATT_ROLE == CLIENT)
      case RSI_APP_EVENT_ADV_REPORT: {
        //! advertise report event.

        //! clear the advertise report event.
        LOG_PRINT("\r\n In Advertising Event\r\n");
        rsi_ble_app_clear_event(RSI_APP_EVENT_ADV_REPORT);
        status = rsi_ble_connect(remote_addr_type, (int8_t *)remote_dev_bd_addr);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n connect status: 0x%lX\r\n", status);
        }

      } break;
#endif
      case RSI_BLE_EVENT_CONN: {
        //! event invokes when connection was completed

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_EVENT_CONN);
        rsi_6byte_dev_address_to_ascii(str_remote_address, conn_event_to_app.dev_addr);
        LOG_PRINT("\r\n Module connected to address : %s \r\n", str_remote_address);
        app_state |= BIT(CONNECTED);
#if (GATT_ROLE == CLIENT)
        rsi_ble_smp_pair_request(conn_event_to_app.dev_addr, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);
#endif
      } break;

      case RSI_BLE_EVENT_DISCONN: {
        //! event invokes when disconnection was completed

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_EVENT_DISCONN);
        if (app_state & BIT(REPORT_IN_NOTIFY_ENABLE)) {
          rsi_ble_app_clear_event(RSI_BLE_GATT_SEND_DATA);
        }
        LOG_PRINT("\r\n Module got disconnected\r\n");
#if ENABLE_NWP_POWER_SAVE
        LOG_PRINT("\r\n keep module in to power save \r\n");
        //! initiating power save in BLE mode
        status = rsi_bt_power_save_profile(RSI_ACTIVE, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\n Failed to initiate power save in BLE mode \r\n");
          return;
        }

        //! initiating Active mode in WLAN mode
        wifi_profile.profile = HIGH_PERFORMANCE;
        status               = sl_wifi_set_performance_profile_v2(&wifi_profile);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n Failed to initiate power save in Wi-Fi mode :%lx\r\n", status);
          return;
        }

        LOG_PRINT("\r\n Module is in power save \r\n");
#endif
        app_state = 0;
        app_state |= BIT(ADVERTISE);
        //! set device in advertising mode.
#if (GATT_ROLE == SERVER)
        LOG_PRINT("\n Start advertising ...\n");
adv:
        status = rsi_ble_start_advertising();
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n Start advertising failed, error status: %lx\n", status);
          goto adv;
        }
#elif (GATT_ROLE == CLIENT)
scan:
        //! start scanning
        device_found = 0;
        status = rsi_ble_start_scanning();
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n Start scanning failed, error status: %lx\n", status);
          goto scan;
        }
        LOG_PRINT("\n Start scanning\n");
#endif
#if ENABLE_NWP_POWER_SAVE
        LOG_PRINT("\r\n keep module in to power save \r\n");
        //! initiating power save in BLE mode
        status = rsi_bt_power_save_profile(PSP_MODE, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\n Failed to initiate power save in BLE mode \r\n");
          return;
        }

        //! initiating power save in wlan mode
        wifi_profile.profile = ASSOCIATED_POWER_SAVE;
        status               = sl_wifi_set_performance_profile_v2(&wifi_profile);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n Failed to initiate power save in Wi-Fi mode :%lx\r\n", status);
          return;
        }

        LOG_PRINT("\r\n Module is in power save \r\n");
#endif
      } break;

      case RSI_BLE_EVENT_GATT_WR: {
        //! event invokes when write/notification events received
#if (GATT_ROLE == CLIENT)
        int ix;
        //Displaying the notification received from HID Device.
        for (ix = 0; ix < app_ble_write_event.length; ix++)
          LOG_PRINT("0x%02x ", app_ble_write_event.att_value[ix]);
        LOG_PRINT("\n");
#endif
        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_EVENT_GATT_WR);
      } break;

      case RSI_BLE_EVENT_GATT_RD: {
        //! event invokes when an read event is received

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_EVENT_GATT_RD);
        for (i = 0; i < p_hid_info->att_rec_list_count; i++) {
          if (p_hid_info->att_rec_list[i].handle == app_ble_read_event.handle) {
            if (app_ble_read_event.type == 0) {
              LOG_PRINT("read resp \n");
              rsi_ble_gatt_read_response(app_ble_read_event.dev_addr,
                                         GATT_READ_RESP,
                                         app_ble_read_event.handle,
                                         GATT_READ_ZERO_OFFSET,
                                         p_hid_info->att_rec_list[i].len,
                                         p_hid_info->att_rec_list[i].value);
            } else {
              LOG_PRINT("read blob resp\n");
              rsi_ble_gatt_read_response(app_ble_read_event.dev_addr,
                                         GATT_READ_BLOB_RESP,
                                         app_ble_read_event.handle,
                                         app_ble_read_event.offset,
                                         p_hid_info->att_rec_list[i].len - app_ble_read_event.offset,
                                         ((uint8_t *)p_hid_info->att_rec_list[i].value) + app_ble_read_event.offset);
            }
          }
        }
      } break;

      case RSI_BLE_EVENT_MTU: {
        //! event invokes when MTU event is received
        LOG_PRINT("\n Received MTU event\n");

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_EVENT_MTU);
      } break;

      case RSI_BLE_EVENT_SMP_REQ: {
        //! initiate SMP protocol as a central

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_EVENT_SMP_REQ);

        //! initiating the SMP pairing process
        status = rsi_ble_smp_pair_request(remote_dev_bd_addr, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);
      } break;

      case RSI_BLE_EVENT_SMP_RESP: {
        //! initiate SMP protocol as a Central

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_EVENT_SMP_RESP);

        //! initiating the SMP pairing process
        status = rsi_ble_smp_pair_response(remote_dev_bd_addr, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);
      } break;

      case RSI_BLE_EVENT_SMP_PASSKEY: {
        //! initiate SMP protocol as a Central

        if ((RSI_BLE_SMP_IO_CAPABILITY == 2) || (RSI_BLE_SMP_IO_CAPABILITY == 4)) {
          LOG_PRINT("\nEnter 6 digit passkey");
          LOG_SCAN("%lu", &smp_passkey);
          rsi_ble_smp_passkey(remote_dev_bd_addr, smp_passkey);
        }
        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_EVENT_SMP_PASSKEY);

        //! initiating the SMP pairing process
        //rsi_ble_smp_passkey (remote_dev_bd_addr, passkey);
      } break;

      case RSI_BLE_EVENT_SMP_PASSKEY_DISPLAY: {
        //! clear the served event
        LOG_PRINT("\r\n smp passkey disp : %s \r\n", passkey);
        rsi_ble_app_clear_event(RSI_BLE_EVENT_SMP_PASSKEY_DISPLAY);
      } break;

      case RSI_BLE_EVENT_SMP_FAILED: {
        //! initiate SMP protocol as a Central
        LOG_PRINT("smp failed\n");

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_EVENT_SMP_FAILED);
      } break;

      case RSI_BLE_EVENT_LTK_REQ: {
        //! event invokes when disconnection was completed

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_EVENT_LTK_REQ);
        //rsi_6byte_dev_address_to_ascii (remote_dev_addr, temp_le_ltk_req.dev_addr);

#if (ROLE == PERIPHERAL_ROLE)
        if ((temp_le_ltk_req.localediv == glbl_enc_enabled.localediv)
            && !((memcmp(temp_le_ltk_req.localrand, glbl_enc_enabled.localrand, 8)))) {
          LOG_PRINT("Positive reply\n");
          LOG_PRINT("\n EDIV match check status : %d \n", (temp_le_ltk_req.localediv == glbl_enc_enabled.localediv));
          LOG_PRINT("\n Random match check status : %d \n",
                    (memcmp(temp_le_ltk_req.localrand, glbl_enc_enabled.localrand, 8)));
          //! give le ltk req reply cmd with positive reply
          status = rsi_ble_ltk_req_reply(temp_le_ltk_req.dev_addr, glbl_enc_enabled.enabled, glbl_enc_enabled.localltk);
          LOG_PRINT("\nLTK :");
          for (i = 0; i < 16; i++) {
            LOG_PRINT("0x%x ", glbl_enc_enabled.localltk[i]);
          }
          LOG_PRINT("\n");
        } else {
          LOG_PRINT("Negative reply\n");
          LOG_PRINT("\n EDIV match check status : %d \n", (temp_le_ltk_req.localediv == glbl_enc_enabled.localediv));
          LOG_PRINT("\n Random match check status : %d \n",
                    (memcmp(temp_le_ltk_req.localrand, glbl_enc_enabled.localrand, 8)));
          //! give le ltk req reply cmd with negative reply
          for (i = 0; i < 8; i++)
            LOG_PRINT(" %d %d \r\n", temp_le_ltk_req.localrand[i], glbl_enc_enabled.localrand[i]);
          status = rsi_ble_ltk_req_reply(temp_le_ltk_req.dev_addr, 0, NULL);
        }
        LOG_PRINT("\n LTK request done \n");
#endif
      } break;

      case RSI_BLE_EVENT_SMP_ENCRYPT_STARTED: {
        LOG_PRINT("smp encrypt resp\n");

        //! start the encrypt event
        uint8_t i = 0;
        LOG_PRINT("\nLTK :");
        for (i = 0; i < 16; i++) {
          LOG_PRINT("0x%x ", glbl_enc_enabled.localltk[i]);
        }
        LOG_PRINT("\n");

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_EVENT_SMP_ENCRYPT_STARTED);

        app_state |= BIT(ENCRYPT_EN);
#if (GATT_ROLE == CLIENT)
        //Get the HID service handles, if it exixts in remote device.
        memset(&service_uuid, 0, sizeof(uuid_t));
        service_uuid.size      = 2;
        service_uuid.val.val16 = RSI_BLE_HID_SERVICE_UUID;
        status                 = rsi_ble_get_profile_async(glbl_enc_enabled.dev_addr, service_uuid, NULL);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\n rsi_ble_get_profile_async : error status 0x%lx \n", status);
        } else {
          LOG_PRINT("\r\n rsi_ble_get_profile_async : successful \n");
        }
#endif
      } break;

#if (GATT_ROLE == CLIENT)
      case RSI_BLE_EVENT_GATT_PROFILE_RESP: {
        //! event invokes when get profile response received

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_EVENT_GATT_PROFILE_RESP);

        if (att_resp_status == 0) {
          LOG_PRINT("Handle range 0x%04x - 0x%04x\n",
                    *(uint16_t *)ble_servs.start_handle,
                    *(uint16_t *)ble_servs.end_handle);
          //! query characteristic services, with in the particular range, from the connected remote device.
          status = rsi_ble_get_char_services_async(remote_dev_bd_addr,
                                                   *(uint16_t *)ble_servs.start_handle,
                                                   *(uint16_t *)ble_servs.end_handle,
                                                   NULL);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n rsi_ble_get_char_services_async : error status 0x%lx \n", status);
          } else {
            LOG_PRINT("\r\n rsi_ble_get_char_services_async : successful \n");
          }
        }
      } break;

      case RSI_BLE_EVENT_GATT_CHAR_SERVICES_RESP: {
        //! event invokes when get characteristics of the service response received

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_EVENT_GATT_CHAR_SERVICES_RESP);

        if (att_resp_status == 0) {
          int ix;
          //Traversing the characteristics list of HID device. Max 3 characteristics per get_char_services call.
          for (ix = 0; ix < char_servs.num_of_services; ix++) {
            LOG_PRINT("Character services of hid profile : ");
            LOG_PRINT(" uuid: 0x%04x handle: 0x%04x\n",
                      char_servs.char_services[ix].char_data.char_uuid.val.val16,
                      char_servs.char_services[ix].handle);
            // if characteristic is HID REPORT TYPE and has notify property we save the respective CCD handle.
            if (char_servs.char_services[ix].char_data.char_uuid.val.val16 == RSI_BLE_HID_REPORT_UUID
                && (char_servs.char_services[ix].char_data.char_property & RSI_BLE_ATT_PROP_NOTIFY)) {
              //Saving the CCD handle, i.e handle + 2, here handle points to characteric declaration.
              desc_range[desc_handle_index++] = char_servs.char_services[ix].handle + 2;
            }
          }
          //If number of characteristic discovered is less than 3, means we have no more chars in the specified service.
          //else start discovering the next list of characteristics starting from the end of the last discovered characteristic.
          if (char_servs.num_of_services >= 3) {
            status = rsi_ble_get_char_services_async(remote_dev_bd_addr,
                                                     char_servs.char_services[ix - 1].handle + 2,
                                                     *(uint16_t *)ble_servs.end_handle,
                                                     NULL);
            if (status != RSI_SUCCESS) {
              LOG_PRINT("\r\n rsi_ble_get_char_services_async : error status 0x%lx \n", status);
            } else {
              LOG_PRINT("\r\n rsi_ble_get_char_services_async : successful \n");
            }
          } else {
            //if all characteristic has been discovered, discover the descriptors one by one from the desc hanlde list.
            if (desc_handle_index > desc_handle_index_1) {
              status = rsi_ble_get_att_descriptors_async(conn_event_to_app.dev_addr,
                                                         desc_range[desc_handle_index_1],
                                                         desc_range[desc_handle_index_1],
                                                         NULL);
              if (status != RSI_SUCCESS) {
                LOG_PRINT("\r\n rsi_ble_get_att_descriptors_async : error status 0x%lx \n", status);
              } else {
                LOG_PRINT("\r\n rsi_ble_get_att_descriptors_async : successful \n");
              }
              desc_handle_index_1 += 1;
            } else {
              desc_handle_index_1 = desc_handle_index = 0;
            }
          }
          //In case we have over shot the handle range of the service, invalid handle(0x4E60) status marks all charactreistics have been read.
        } else if (att_resp_status == 0x4E60) {
          if (desc_handle_index > desc_handle_index_1) {
            //if all characteristic has been discovered, discover the descriptors one by one from the desc handle list.
            status = rsi_ble_get_att_descriptors_async(conn_event_to_app.dev_addr,
                                                       desc_range[desc_handle_index_1],
                                                       desc_range[desc_handle_index_1],
                                                       NULL);
            if (status != RSI_SUCCESS) {
              LOG_PRINT("\r\n rsi_ble_get_att_descriptors_async : error status 0x%lx \n", status);
            } else {
              LOG_PRINT("\r\n rsi_ble_get_att_descriptors_async : successful \n");
            }
            desc_handle_index_1 += 1;
          } else {
            desc_handle_index_1 = desc_handle_index = 0;
          }
        }
      } break;

      case RSI_BLE_EVENT_GATT_CHAR_DESC_RESP: {

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_EVENT_GATT_CHAR_DESC_RESP);

        if (att_resp_status == 0) {
          int ix;
          //Traverse the list to check if the discovered descriptors are CCDS and enable notification.
          for (ix = 0; ix < att_desc.num_of_att; ix++) {
            LOG_PRINT("hancle: 0x%04x - 0x%04x\r\n",
                      *((uint16_t *)att_desc.att_desc[ix].handle),
                      att_desc.att_desc[ix].att_type_uuid.val.val16);
            if (att_desc.att_desc[ix].att_type_uuid.val.val16 == 0x2902) {
              uint8_t data[2];
              data[0] = 0x01;
              data[1] = 0x00;
              status  = rsi_ble_set_att_cmd_async(conn_event_to_app.dev_addr,
                                                 *((uint16_t *)att_desc.att_desc[ix].handle),
                                                 2,
                                                 (uint8_t *)data);
              if (status != RSI_SUCCESS) {
                LOG_PRINT("\r\n rsi_ble_set_att_cmd_async : error status 0x%lx \n", status);
              } else {
                LOG_PRINT("\r\n rsi_ble_set_att_cmd_async : successful \n");
              }
              LOG_PRINT("Notification enabled \n");
            }
          }
          memset(&att_desc, 0, sizeof(rsi_ble_event_gatt_desc_t));
          //Check for the next descriptor in the list.
          if (desc_handle_index > desc_handle_index_1) {
            status = rsi_ble_get_att_descriptors_async(conn_event_to_app.dev_addr,
                                                       desc_range[desc_handle_index_1],
                                                       desc_range[desc_handle_index_1],
                                                       NULL);
            if (status != RSI_SUCCESS) {
              LOG_PRINT("\r\n rsi_ble_get_att_descriptors_async : error status 0x%lx \n", status);
            } else {
              LOG_PRINT("\r\n rsi_ble_get_att_descriptors_async : successful \n");
            }
            desc_handle_index_1 += 1;
          } else {
            desc_handle_index_1 = desc_handle_index = 0;
          }
        }
        memset(&att_desc, 0, sizeof(rsi_ble_event_gatt_desc_t));
      } break;
#endif

      case RSI_BLE_SC_PASSKEY_EVENT: {
        rsi_ble_app_clear_event(RSI_BLE_SC_PASSKEY_EVENT);
        LOG_PRINT("remote addr: %s, sc passkey: %06lu \r\n", remote_dev_addr, numeric_value);

        if ((RSI_BLE_SMP_IO_CAPABILITY == 2) || (RSI_BLE_SMP_IO_CAPABILITY == 4)) {
          LOG_PRINT("\nEnter 6 digit passkey");
          LOG_SCAN("%lu", &smp_passkey);
          rsi_ble_smp_passkey(remote_dev_bd_addr, smp_passkey);
        } else {
          rsi_ble_smp_passkey(remote_dev_bd_addr, numeric_value);
          LOG_PRINT("\nnumeric value %lu\n", numeric_value);
        }
      } break;

      case RSI_BLE_GATT_SEND_DATA:
        //! clear the served event
        status = rsi_ble_conn_params_update(conn_event_to_app.dev_addr,
                                            MIN_CONN_INTERVAL,
                                            MAX_CONN_INTERVAL,
                                            CONN_LATENCY,
                                            SUPERVISION_TIMEOUT);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nrsi_ble_conn_params_update Failed, Error Code : 0x%lX\r\n", status);
          return;
        }
        if (app_state & BIT(CONNECTED)) {
          if (app_state & BIT(REPORT_IN_NOTIFY_ENABLE)) {
            hid_data[2] = 0xb; // key 'h' pressed
            rsi_ble_set_local_att_value(rsi_ble_hid_in_report_val_hndl, HID_KDB_IN_RPT_DATA_LEN, hid_data);
            hid_data[2] = 0x0; // key released
            rsi_ble_set_local_att_value(rsi_ble_hid_in_report_val_hndl, HID_KDB_IN_RPT_DATA_LEN, hid_data);

            hid_data[2] = 0x12; // key 'o' pressed
            rsi_ble_set_local_att_value(rsi_ble_hid_in_report_val_hndl, HID_KDB_IN_RPT_DATA_LEN, hid_data);
            hid_data[2] = 0x0; // key released
            rsi_ble_set_local_att_value(rsi_ble_hid_in_report_val_hndl, HID_KDB_IN_RPT_DATA_LEN, hid_data);

            hid_data[2] = 0xa; // key 'g' pressed
            rsi_ble_set_local_att_value(rsi_ble_hid_in_report_val_hndl, HID_KDB_IN_RPT_DATA_LEN, hid_data);
            hid_data[2] = 0x0; // key released
            rsi_ble_set_local_att_value(rsi_ble_hid_in_report_val_hndl, HID_KDB_IN_RPT_DATA_LEN, hid_data);

            hid_data[2] = 0x2d; // key '_' pressed
            rsi_ble_set_local_att_value(rsi_ble_hid_in_report_val_hndl, HID_KDB_IN_RPT_DATA_LEN, hid_data);
            hid_data[2] = 0x0; // key released
            rsi_ble_set_local_att_value(rsi_ble_hid_in_report_val_hndl, HID_KDB_IN_RPT_DATA_LEN, hid_data);
          }
        }
        break;
      default: {
      }
    }
  }
}

void app_init(void)
{
  osThreadNew((osThreadFunc_t)ble_hids_gatt_application, &hid_info_g, &thread_attributes);
}
