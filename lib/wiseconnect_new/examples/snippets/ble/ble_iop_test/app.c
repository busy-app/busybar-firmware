/*******************************************************************************
* @file  app.c
* @brief
*******************************************************************************
* # License
* <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
/*================================================================================
 * @brief : This file contains example application for Interoperability test
 * @section Description :
 * This application demonstrates the basic functionalities of a Bluetooth device
 =================================================================================*/
//! SL Wi-Fi SDK includes
#include "cmsis_os2.h"
#include "errno.h"
#include "sl_board_configuration.h"
#include "sl_constants.h"
#include "sl_net.h"
#include "sl_si91x_driver.h"
#include "sl_utility.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include <stdio.h>
#include <string.h>
#include "ble_config.h"
#include "rsi_ble.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common.h"
#include "rsi_bt_common_apis.h"
#include "rsi_common_apis.h"
#include "sl_net_constants.h"
#include "ble_iop.h"

#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_hal_soc_soft_reset.h"
#endif

/* Global Variables */
//! Semaphore variable
osSemaphoreId_t ble_main_task_sem;
//! variable to handle the event mapping
static volatile uint32_t ble_app_event_map;
//! variable to handle the event masking
static volatile uint32_t ble_app_event_mask;
//! To store ascii mac address
static uint8_t str_remote_address[BD_ADDR_STRING_SIZE];
//! To store the hexa device address
static uint8_t remote_dev_address[RSI_DEV_ADDR_LEN];
//! Arrays to store bd address
uint8_t str_local_dev_address[BD_ADDR_STRING_SIZE] = { 0 };
//! variable to store the MTU size
static uint16_t mtu_size = PDU_SIZE - 4;
//! to store the return status of the function
sl_status_t status;

/* Structures used in callbacks */
//! structure to store the updated PHY event
static rsi_ble_event_phy_update_t app_phy_update_complete;
//! To store the remote device features
static rsi_ble_event_remote_features_t remote_dev_feature;
//! To store the MTU event information
static rsi_ble_event_mtu_t app_ble_mtu_event;
//! To store the BLE length update event
static rsi_ble_event_data_length_update_t data_length_update;
//! To store the BLE connection update event
static rsi_ble_event_conn_update_t event_conn_update_complete;
//! To store the data of prepare write operation
static rsi_ble_event_prepare_write_t app_ble_prepare_event;
//! To store the data of execute write operation
static rsi_ble_execute_write_t app_ble_execute_write_event;
//! To store secuirty keys
static rsi_bt_event_le_ltk_request_t temp_le_ltk_req;
//! To store the read event
static rsi_ble_read_req_t app_ble_read_event;
//! To store write event information
static rsi_ble_event_write_t app_ble_write_event;

/* Structures that stores handles of services and characteristics */
//! structure that stores the iop service handles
test_iop_serv1 test_iop_service1;
//! Structure to store connection information
connection_information conn_info_t;
//! Structure to store test properties service handles
test_prop iop_test_prop;
//! Structure to store test characteristic handles
test_char_types char_type_handle;
//! Structure to store test iop phase3 handles
iop_phase3_service_struct iop_phase3;
//! Structure to store device information
device_information device_info;

/* Variables used to send and receive data */
//! Variable to store the data of len1 which is used during read operation
static uint8_t read_only_len1 = 0x55;
//! Variable to store the data of len255 which is used during read operation
uint8_t read_only_len255[DATA_LENGTH] = { 0 };
//! Variable to store the data of len1 which is used during write operation
static uint8_t write_value_len1 = 0x55;
//! Variable to store the data of len1 which is used during write operation
static uint8_t write_only_len1_value = 0x55;
//! To store the notification values
static uint8_t iop_test_notification_250_arr[DATA_SIZE_NOTIFY];
//! To store the data of 255 bytes
static uint8_t write_only_len255_value[DATA_LENGTH];
//! to store the data of variable length
static uint8_t iop_test_chr_user_var_4_arr[4];
//! To store the length of data of variable length
static uint8_t iop_test_chr_user_var_4_len;
//! Arrays to store write value of diff length
uint8_t write_value[DATA_LENGTH]            = { 0 };
uint8_t write_value_send[DATA_LENGTH]       = { 0 };
uint8_t write_value_usr_len255[DATA_LENGTH] = { 0 };
uint16_t write_value_len_usr_len255         = 0;
//! variable to store length of written data value
uint8_t write_value_length;
//! To store the status of conn params update
static uint8_t conn_params_updated = 0;
//!Phase 3: Array to store data written to control handle
uint8_t gattdb_control_value[3] = { 0 };
//! Variable to store status of notification
static uint8_t notifies_enabled;
//! user data length 1
uint8_t user_len1;
//! To store the status of notification
int8_t notify_start = false;
//! To store the status of indication
int8_t indicate_start = false;
//! Timers used during the throughput
uint32_t tx_start_timer = 0;
uint32_t tx_stop_timer  = 0;
//! To store the read and write result
int32_t result;
//! Packet count used during throughput
uint64_t tx_pkt_cnt = 0;
//! To store the firmware version
sl_wifi_firmware_version_t version = { 0 };

#if FWUP_UPGRADE
/* Variables used in OTA Test Cases*/
//! To add OTA service
rsi_ble_resp_add_serv_t ota_serv_response;
//! variables to store header and payload of firmware data
uint8_t firmware_header_data[64]       = { 0 };
uint8_t firmware_chunk_fw_payload[255] = { 0 };
//! OTA service UUID
uint8_t ota_service_uuid = { 0 };
//! Variable used during OTA
uint8_t data_tf_start = 0, buf_config_var = 0;
//! To store the data sent during OTA upgrade
static uint8_t chunk_data[255] = { 0 };
//! To handle the status of OTA chunk data
static uint16_t chunk_number = 1;
//! variable to store current firmware version
uint8_t current_fw_version[20] = { 0 };
//! OTA service UUID structure
uuid_t ota_service = { 0 };
//! OTA info characteristic UUID structure
uuid_t ota_info_char_service = { 0 };
//! OTA data characteristic UUID structure
uuid_t ota_data_char_service = { 0 };
//! OTA firmware version characteristic UUID structure
uuid_t ota_fw_version_char_service = { 0 };
//! OTA bd addr characteristic UUID structure
uuid_t ota_bd_add_char_service = { 0 };
//! To store OTA characteristics handle
uint16_t ota_fw_control_handle, ota_fw_tf_handle;
uint32_t start_timer, stop_timer;
//! To status of bonding availability
uint8_t bonding_available = 0;
char passkey_str[6];
//! OTA Flags
bool done = false;
#endif

/* Security Test Case Related */
//! structure to store SMP pairing capabilities
rsi_ble_set_smp_pairing_capabilty_data_t smp_capabilities;
//! To store status of MITM request
uint8_t mitm_req = 0;
//! To store security mode
rsi_bt_connection_security_t connection_security = rsi_bt_connection_mode1_level1;
//! value to send during pairing test case
uint8_t value = 0x55;
//! To store passkey
static uint32_t passkey;
//! Structure to store the resolve key
static rsi_ble_resolve_key_t resolve_key;
//! To store address type
static uint8_t dev_address_type;
//! To store resolvlist size
static uint8_t app_resp_resolvlist_size = 0;
//! To store resolvlist group
static rsi_ble_resolvlist_group_t resolvlist[RSI_MAX_LIST_SIZE];
//! To store security key
static rsi_bt_event_le_security_keys_t temp_le_sec_keys;
//! To store device LTK list
rsi_ble_dev_ltk_list_t ble_dev_ltk_list[RSI_MAX_LIST_SIZE];
//! To store passkey
static uint8_t smp_passkey[BLE_PASSKEY_SIZE];

//! store connection parameter
uint8_t conn_params[12] = { CONN_INTERVAL_MIN_LSB,   CONN_INTERVAL_MIN_MSB,   CONN_INTERVAL_MAX_LSB,
                            CONN_INTERVAL_MAX_MSB,   CONN_LATENCY_LSB,        CONN_LATENCY_MSB,
                            SUPERVISION_TIMEOUT_LSB, SUPERVISION_TIMEOUT_MSB, MIN_CE_LENGTH_LSB,
                            MIN_CE_LENGTH_MSB,       MAX_CE_LENGTH_LSB };

static const sl_wifi_device_configuration_t
  config = { .boot_option = LOAD_NWP_FW,
             .mac_address = NULL,
             .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
             .region_code = US,
             .boot_config = {
               .oper_mode              = SL_SI91X_CLIENT_MODE,
               .coex_mode              = SL_SI91X_WLAN_BLE_MODE,
               .feature_bit_map        = (SL_WIFI_FEAT_WPS_DISABLE | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
                                   | SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1),
               .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
               .custom_feature_bit_map = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),

               .ext_custom_feature_bit_map =
                 (SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0 | SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE),
               .bt_feature_bit_map         = (SL_SI91X_BT_RF_TYPE | SL_SI91X_ENABLE_BLE_PROTOCOL),
               .ext_tcp_ip_feature_bit_map = (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
               //!ENABLE_BLE_PROTOCOL in bt_feature_bit_map
               .ble_feature_bit_map =
                 ((SL_SI91X_BLE_MAX_NBR_PERIPHERALS(RSI_BLE_MAX_NBR_PERIPHERALS)
                   | SL_SI91X_BLE_MAX_NBR_CENTRALS(RSI_BLE_MAX_NBR_CENTRALS)
                   | SL_SI91X_BLE_MAX_NBR_ATT_SERV(RSI_BLE_MAX_NBR_ATT_SERV)
                   | SL_SI91X_BLE_MAX_NBR_ATT_REC(RSI_BLE_MAX_NBR_ATT_REC))
                  | SL_SI91X_FEAT_BLE_CUSTOM_FEAT_EXTENTION_VALID | SL_SI91X_BLE_PWR_INX(RSI_BLE_PWR_INX)
                  | SL_SI91X_BLE_PWR_SAVE_OPTIONS(RSI_BLE_PWR_SAVE_OPTIONS) | SL_SI91X_916_BLE_COMPATIBLE_FEAT_ENABLE
#if 1
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
#if RSI_BLE_GATT_INIT
                  | SL_SI91X_BLE_GATT_INIT
#endif
                  ),
               .config_feature_bit_map = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP) } };

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

/*============================================================================*/
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
  ble_app_event_mask = 0xFFFFFFFF;
  ble_app_event_mask = ble_app_event_mask; //To suppress warning while compiling

  return;
}

/*============================================================================*/
/**
 * @fn         rsi_ble_app_set_event
 * @brief      set the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to set/raise the specific event.
 */
static void rsi_ble_app_set_event(uint32_t event_num)
{
  ble_app_event_map |= BIT(event_num);
  osSemaphoreRelease(ble_main_task_sem);

  return;
}

/*============================================================================*/
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
  ble_app_event_map &= ~BIT(event_num);
  return;
}

/*============================================================================*/
/**
 * @fn         rsi_ble_app_clear_all_events
 * @brief      clears the all event.
 * @return     none.
 * @section description
 * This function is used to clear the specific event.
 */
static void rsi_ble_app_clear_all_event(void)
{
  ble_app_event_map = 0;
  return;
}

/*============================================================================*/
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

  for (ix = 0; ix < 32; ix++) {
    if (ble_app_event_map & (1 << ix)) {
      return ix;
    }
  }

  return (-1);
}

/*============================================================================*/
/**
 * @fn         throughput_calculation
 * @brief      this function is used for throughput calculation
 * @param[in]  start_timer, start timer
 * @param[in]  stop_timer, stop timer
 * @param[in]  packet_count, received packet count
 * @return     none
 * @section description
 * This function is used to start or stop the data transfer to the remote
 * device.
 */
void throughput_calculation(uint32_t start_timer, uint32_t stop_timer, uint32_t packet_count)
{
  float throughput = 0;
  float timing     = 0;

  timing     = (((float)(stop_timer - start_timer)) / 1000);
  throughput = (((float)(packet_count * RSI_BLE_MAX_DATA_LEN * 8)) / timing);
  LOG_PRINT("\r\nThroughput : %.07f bps\n", (float)throughput);
  LOG_PRINT("\r\nThroughput : %.07f kbps\n", (float)(throughput / 1000));
  LOG_PRINT("\r\n Time duration in sec:%0.2f \n", timing);
}

/*============================================================================*/
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
  memcpy(remote_dev_address, resp_enh_conn->dev_addr, RSI_DEV_ADDR_LEN);
  rsi_6byte_dev_address_to_ascii(str_remote_address, resp_enh_conn->dev_addr);

  LOG_PRINT("Connected Device Address: %s\n", str_remote_address);

  rsi_ble_app_set_event(RSI_BLE_CONN_EVENT);
}

/*============================================================================*/
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
  dev_address_type = resp_conn->dev_addr_type;
  memcpy(remote_dev_address, resp_conn->dev_addr, RSI_DEV_ADDR_LEN);
  rsi_6byte_dev_address_to_ascii(str_remote_address, resp_conn->dev_addr);

  rsi_ble_app_set_event(RSI_BLE_CONN_EVENT);
}

/*============================================================================*/
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
  UNUSED_PARAMETER(reason);
  memcpy(remote_dev_address, resp_disconnect->dev_addr, RSI_DEV_ADDR_LEN);
  rsi_6byte_dev_address_to_ascii(str_remote_address, resp_disconnect->dev_addr);

  rsi_ble_app_set_event(RSI_BLE_DISCONN_EVENT);
}

#if SMP_ENABLE
/*============================================================================*/
/**
 * @fn         add_device_to_resolvlist
 * @brief      this function will add device to resolvlistwith updated irks
 * @param[out] ix, index of resolvlist
 * @return     none.
 * @section description
 * add device to resolvlistwith updated irks
 * */
int32_t add_device_to_resolvlist(rsi_ble_resolvlist_group_t *resolvlist_p, rsi_ble_resolve_key_t *resolve_key_p)
{
  uint32_t status = 0;
  uint8_t ix;

  for (ix = 0; ix < RSI_BLE_RESOLVING_LIST_SIZE; ix++) {
    if (resolvlist_p[ix].used == 0) {
      resolvlist_p[ix].used               = 1;
      resolvlist_p[ix].Identity_addr_type = resolve_key_p->Identity_addr_type;
      memcpy(resolvlist_p[ix].Identity_addr, resolve_key_p->Identity_addr, sizeof(resolve_key_p->Identity_addr));
      memcpy(resolvlist_p[ix].peer_irk, resolve_key_p->peer_irk, sizeof(resolve_key_p->peer_irk));
      memcpy(resolvlist_p[ix].local_irk, resolve_key_p->local_irk, sizeof(resolve_key_p->local_irk));
      break;
    }
  }
  if (ix >= RSI_BLE_RESOLVING_LIST_SIZE) {
    return -1;
  }

  //add device to resolvlist or remove from resolvlist or clear the resolvlist
  status = rsi_ble_resolvlist(RSI_BLE_ADD_TO_RESOLVE_LIST,
                              resolve_key_p->Identity_addr_type,
                              temp_le_sec_keys.Identity_addr,
                              resolve_key_p->peer_irk,
                              resolve_key_p->local_irk);
  if (status != RSI_SUCCESS) {
    return status;
  }
  return status;
}

/*============================================================================*/
/**
 * @fn         update_resolvlist
 * @brief      this function will update the resolvlist if different remote
 *              address or different irks for same address received
 * @param[out] none.
 * @return     none.
 * @section description
 * update the resolvlist if different remote address or
 * different irks for same address received
 * */
void update_resolvlist(rsi_ble_resolvlist_group_t *resolvlist_p, rsi_ble_resolve_key_t *resolve_key_p)
{
  uint8_t ix = 0;
  while (ix < RSI_BLE_RESOLVING_LIST_SIZE) {
    if (!strcmp((const char *)resolvlist_p[ix].Identity_addr, (const char *)resolve_key_p->Identity_addr)) {
      if (memcmp(resolvlist_p[ix].peer_irk, resolve_key_p->peer_irk, sizeof(resolve_key_p->peer_irk))) {
        status = add_device_to_resolvlist(resolvlist_p, resolve_key_p);
        break;
      }
    } else {
      strcpy((char *)resolvlist_p[ix].Identity_addr, (char *)resolve_key_p->Identity_addr);
      add_device_to_resolvlist(resolvlist_p, resolve_key_p);
      break;
    }
    ix++;
  }
}

/*============================================================================*/
/**
 * @fn         clear_ltk_list
 * @brief      to clear the stored LTK
 * @param[in]  none
 * @return     1, if success
 * @section description
 * This function is to clear the stored LTK
 */
int8_t clear_ltk_list()
{
  uint8_t ix;

  for (ix = 0; ix < RSI_MAX_LIST_SIZE; ix++) {
    ble_dev_ltk_list[ix].used = 0;
  }
  return 1;
}

/*============================================================================*/
/**
 * @fn         add_device_to_ltk_key_list
 * @brief      to store the device info to the list
 * @param[in]  none
 * @return     -1, if fail.
 * @section description
 * This function is to store device's information to the list
 */
int8_t add_device_to_ltk_key_list(rsi_ble_dev_ltk_list_t *ble_dev_ltk_list,
                                  rsi_bt_event_encryption_enabled_t *enc_enabled)
{
  int8_t status = 0;
  uint8_t ix;

  for (ix = 0; ix < RSI_MAX_LIST_SIZE; ix++) {

    if (ble_dev_ltk_list[ix].used == 1) {
      if ((enc_enabled->dev_addr_type > 1) && (!memcmp(enc_enabled->dev_addr, ble_dev_ltk_list[ix].Identity_addr, 6))) {
        break;
      }

      if ((enc_enabled->dev_addr_type <= 1)
          && (!memcmp(enc_enabled->dev_addr, ble_dev_ltk_list[ix].remote_dev_addr, 6))) {
        break;
      }
    }

    if (ble_dev_ltk_list[ix].used == 0) {
      ble_dev_ltk_list[ix].used       = 1;
      ble_dev_ltk_list[ix].enc_enable = enc_enabled->enabled;
      ble_dev_ltk_list[ix].sc_enable  = enc_enabled->sc_enable;
      memcpy(ble_dev_ltk_list[ix].remote_dev_addr, enc_enabled->dev_addr, RSI_DEV_ADDR_LEN);
      memcpy(ble_dev_ltk_list[ix].localltk, enc_enabled->localltk, LTK_LENGTH);
      memcpy(ble_dev_ltk_list[ix].localrand, enc_enabled->localrand, RAND_LENGTH);
      ble_dev_ltk_list[ix].local_ediv = enc_enabled->localediv;
      break;
    }
  }
  if (ix >= RSI_MAX_LIST_SIZE) {
    return -1;
  }
  return status;
}

/*============================================================================*/
/**
 * @fn         add_security_keys_to_device_list
 * @brief      to store the security keys to the list
 * @param[in]  none
 * @return     -1, if fail.
 * @section description
 * This function is to store security keys information to the list
 */
int8_t add_security_keys_to_device_list(rsi_ble_dev_ltk_list_t *ble_dev_ltk_list,
                                        rsi_bt_event_le_security_keys_t *le_sec_keys)
{
  int8_t status = 0;
  uint8_t ix;
  for (ix = 0; ix < RSI_MAX_LIST_SIZE; ix++) {

    if ((ble_dev_ltk_list[ix].used == 1)
        && (!memcmp(ble_dev_ltk_list[ix].remote_dev_addr, le_sec_keys->dev_addr, RSI_DEV_ADDR_LEN))
        && (ble_dev_ltk_list[ix].remote_dev_addr_type == le_sec_keys->dev_addr_type)) {
      memcpy(ble_dev_ltk_list[ix].local_irk, le_sec_keys->local_irk, 16);
      memcpy(ble_dev_ltk_list[ix].peer_irk, le_sec_keys->remote_irk, 16);
      memcpy(ble_dev_ltk_list[ix].remote_rand, le_sec_keys->remote_rand, 8);
      memcpy(ble_dev_ltk_list[ix].remote_ltk, le_sec_keys->remote_ltk, 16);
      memcpy(ble_dev_ltk_list[ix].Identity_addr, le_sec_keys->Identity_addr, RSI_DEV_ADDR_LEN);
      ble_dev_ltk_list[ix].remote_ediv        = le_sec_keys->remote_ediv;
      ble_dev_ltk_list[ix].Identity_addr_type = le_sec_keys->Identity_addr_type;
      break;
    }
  }
  if (ix >= RSI_MAX_LIST_SIZE) {
    return -1;
  }

  return status;
}

/*============================================================================*/
/**
 * @fn         rsi_get_ltk_list
 * @brief      to get the ltk from the list
 * @param[in]  none
 * @return     -1, if fail.
 * @section description
 * This function is to to get the ltk from the list
 */
int8_t rsi_get_ltk_list(rsi_ble_dev_ltk_list_t *ble_dev_ltk_list, rsi_bt_event_le_ltk_request_t *le_ltk_req)
{
  uint8_t ix;

  for (ix = 0; ix < RSI_MAX_LIST_SIZE; ix++) {

    if (ble_dev_ltk_list[ix].used == 1) {
      if ((le_ltk_req->dev_addr_type > 1)
          && (!(memcmp(le_ltk_req->dev_addr, ble_dev_ltk_list[ix].Identity_addr, RSI_DEV_ADDR_LEN)))) {
        return ix;
      }

      if ((le_ltk_req->dev_addr_type <= 1)
          && (!(memcmp(le_ltk_req->dev_addr, ble_dev_ltk_list[ix].remote_dev_addr, RSI_DEV_ADDR_LEN)))) {
        return ix;
      }
    }
  }
  return -1;
}

/*============================================================================*/
/**
 * @fn         rsi_ble_on_smp_request
 * @brief      its invoked when smp request event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when an SMP request event is received
 * (we are in Central mode)
 * Note: Peripheral requested to start SMP request, we have to send SMP request
 * command
 */
void rsi_ble_on_smp_request(rsi_bt_event_smp_req_t *event_smp_req)
{
  memcpy(remote_dev_address, event_smp_req->dev_addr, RSI_DEV_ADDR_LEN);
  rsi_6byte_dev_address_to_ascii(str_remote_address, event_smp_req->dev_addr);

  rsi_ble_app_set_event(RSI_BLE_SMP_REQ_EVENT);
}

/*============================================================================*/
/**
 * @fn         rsi_ble_on_smp_response
 * @brief      its invoked when smp response event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when an SMP response event is received
 * (we are in Peripheral mode)
 * Note: Central initiated SMP protocol, we have to send SMP response command
 */
void rsi_ble_on_smp_response(rsi_bt_event_smp_resp_t *event_smp_response)
{
  memcpy(remote_dev_address, event_smp_response->dev_addr, RSI_DEV_ADDR_LEN);
  rsi_6byte_dev_address_to_ascii(str_remote_address, event_smp_response->dev_addr);

  rsi_ble_app_set_event(RSI_BLE_SMP_RESP_EVENT);
}

/*============================================================================*/
/**
 * @fn         rsi_ble_on_smp_passkey
 * @brief      its invoked when an SMP passkey event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP passkey events is received
 * Note: We have to send SMP passkey command
 */
void rsi_ble_on_smp_passkey(rsi_bt_event_smp_passkey_t *event_smp_passkey)
{
  memcpy(remote_dev_address, event_smp_passkey->dev_addr, RSI_DEV_ADDR_LEN);
  rsi_6byte_dev_address_to_ascii(str_remote_address, event_smp_passkey->dev_addr);

  rsi_ble_app_set_event(RSI_BLE_SMP_PASSKEY_EVENT);
}

/*============================================================================*/
/**
 * @fn         rsi_ble_on_smp_passkey_display
 * @brief      its invoked when an SMP passkey event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP passkey events is received
 * Note: We have to send SMP passkey command
 */
void rsi_ble_on_smp_passkey_display(rsi_bt_event_smp_passkey_display_t *smp_passkey_display)
{
  memcpy(remote_dev_address, smp_passkey_display->dev_addr, RSI_DEV_ADDR_LEN);
  rsi_6byte_dev_address_to_ascii(str_remote_address, smp_passkey_display->dev_addr);
  memcpy(smp_passkey, smp_passkey_display->passkey, sizeof(smp_passkey_display->passkey));

  rsi_ble_app_set_event(RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT);
}

/*============================================================================*/
/**
 * @fn         rsi_ble_on_sc_passkey
 * @brief      its invoked for a BLE Secure Connection passkey event from the module
 * @param[in]  sc_passkey, hold the Secure Connections (SC) passkey event.
 * @return     none.
 * @section description
 * This callback function is invoked for a BLE Secure Connection passkey event
 * from the module
 */
void rsi_ble_on_sc_passkey(rsi_bt_event_sc_passkey_t *sc_passkey)
{
  memcpy(remote_dev_address, sc_passkey->dev_addr, RSI_DEV_ADDR_LEN);
  rsi_6byte_dev_address_to_ascii(str_remote_address, sc_passkey->dev_addr);
  passkey = sc_passkey->passkey;

  rsi_ble_app_set_event(RSI_BLE_SC_PASSKEY_EVENT);
}

/*============================================================================*/
/**
 * @fn         rsi_ble_on_le_ltk_req_event
 * @brief      invoked when ltk request event is received
 * @param[in]  le_ltk_req, ltk request parameters.
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This callback function indicates LTK device information
 */
static void rsi_ble_on_le_ltk_req_event(rsi_bt_event_le_ltk_request_t *le_ltk_req)
{
  rsi_6byte_dev_address_to_ascii(str_remote_address, le_ltk_req->dev_addr);
  memcpy(&temp_le_ltk_req, le_ltk_req, sizeof(rsi_bt_event_le_ltk_request_t));

  rsi_ble_app_set_event(RSI_BLE_LTK_REQ_EVENT);
}

/*============================================================================*/
/**
 * @fn         rsi_ble_on_le_security_keys
 * @brief      its invoked when le secure keys events is received.
 * @param[in]  rsi_ble_event_le_security_keys, secure event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when security keys event is received after
 * encryption started.
 */
void rsi_ble_on_le_security_keys(rsi_bt_event_le_security_keys_t *rsi_ble_event_le_security_keys)
{
  add_security_keys_to_device_list(ble_dev_ltk_list, rsi_ble_event_le_security_keys);
  memcpy(&temp_le_sec_keys, rsi_ble_event_le_security_keys, sizeof(rsi_bt_event_le_security_keys_t));

  rsi_ble_app_set_event(RSI_BLE_SECURITY_KEYS_EVENT);
}

/*============================================================================*/
/**
 * @fn         rsi_ble_on_smp_failed
 * @brief      its invoked when an SMP failed event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when an SMP failed event is received
 */
void rsi_ble_on_smp_failed(uint16_t status, rsi_bt_event_smp_failed_t *smp_failed_event)
{
  UNUSED_PARAMETER(status);
  memcpy(remote_dev_address, smp_failed_event->dev_addr, RSI_DEV_ADDR_LEN);
  rsi_6byte_dev_address_to_ascii(str_remote_address, smp_failed_event->dev_addr);

  rsi_ble_app_set_event(RSI_BLE_SMP_FAILED_EVENT);
}

/*============================================================================*/
/**
 * @fn         rsi_ble_on_encrypt_started
 * @brief      its invoked when encryption started event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when encryption started events is received
 */
void rsi_ble_on_encrypt_started(uint16_t status, rsi_bt_event_encryption_enabled_t *enc_enabled)
{
  UNUSED_PARAMETER(status);
  add_device_to_ltk_key_list(ble_dev_ltk_list, enc_enabled);

  rsi_ble_app_set_event(RSI_BLE_ENCRYPT_STARTED_EVENT);
}

#endif // SMP_ENABLE

/*============================================================================*/
/**
 * @fn         rsi_ble_fill_128bit_uuid
 * @brief      this function is used to form 128bit uuid of apple device from
 *              32 bit uuid.
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to create new service.
 */
static void rsi_ble_fill_128bit_uuid(uint32_t uuid_32bit, uuid_t *serv_uuid)
{
  uint8_t ix;
  serv_uuid->size = 16;
  rsi_uint32_to_4bytes((uint8_t *)&serv_uuid->val.val128.data1, uuid_32bit);
  rsi_uint16_to_2bytes((uint8_t *)&serv_uuid->val.val128.data2, 0x00);
  rsi_uint16_to_2bytes((uint8_t *)&serv_uuid->val.val128.data3, 0x1000);
  rsi_uint16_to_2bytes(&serv_uuid->val.val128.data4[0], 0x8000);
  for (ix = 0; ix < 6; ix++) {
    serv_uuid->val.val128.data4[2] = 0x26;
    serv_uuid->val.val128.data4[3] = 0x00;
    serv_uuid->val.val128.data4[4] = 0x91;
    serv_uuid->val.val128.data4[5] = 0x52;
    serv_uuid->val.val128.data4[6] = 0x76;
    serv_uuid->val.val128.data4[7] = 0xBB;
  }

  return;
}

/*============================================================================*/
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
  new_att.property           = RSI_BLE_ATT_PROPERTY_READ;

  //! preparing the characteristic attribute value
  if (att_val_uuid.size == UUID_SIZE) {
    new_att.data_len = 4 + att_val_uuid.size;
    new_att.data[0]  = val_prop;
    rsi_uint16_to_2bytes(&new_att.data[2], att_val_handle);
    memcpy(&new_att.data[4], &att_val_uuid.val.val128, sizeof(att_val_uuid.val.val128));
  } else {
    new_att.data_len = 6;
    new_att.data[0]  = val_prop;
    rsi_uint16_to_2bytes(&new_att.data[2], att_val_handle);
    rsi_uint16_to_2bytes(&new_att.data[4], att_val_uuid.val.val16);
  }
  //! Add attribute to the service
  rsi_ble_add_attribute(&new_att);

  return;
}

/*============================================================================*/
/**
 * @fn         rsi_ble_add_char_val_att
 * @brief      this function is used to add value to characteristic.
 * @param[in]  serv_handler, service handler.
 * @param[in]  handle, characteristic service attribute handle.
 * @param[in]  val_prop, characteristic value property.
 * @param[in]  data, value that should be added to characteristic
 * @param[in]  data_len, length of the data that is adding.
 * @param[in]  auth_read, decided whether to handle by host or controller
 * @return     none.
 * @section description
 * This function is used at application to add value to characteristic
 */
static void rsi_ble_add_char_val_att(void *serv_handler,
                                     uint16_t handle,
                                     uuid_t att_type_uuid,
                                     uint8_t val_prop,
                                     uint8_t *data,
                                     uint8_t data_len,
                                     uint8_t auth_read)
{
  rsi_ble_req_add_att_t new_att = { 0 };

  //! preparing the attributes
  new_att.serv_handler  = serv_handler;
  new_att.handle        = handle;
  new_att.config_bitmap = auth_read;
  memcpy(&new_att.att_uuid, &att_type_uuid, sizeof(uuid_t));
  new_att.property = val_prop;

  //! preparing the attribute value
  new_att.data_len = RSI_MIN(sizeof(new_att.data), data_len);
  memcpy(new_att.data, data, new_att.data_len);

  //! add attribute to the service
  rsi_ble_add_attribute(&new_att);

  //! check the attribute property with notification
  if ((val_prop & RSI_BLE_ATT_PROPERTY_NOTIFY) || (val_prop & RSI_BLE_ATT_PROPERTY_INDICATE)) {
    //! if notification property supports then we need to add client characteristic service.

    //! preparing the client characteristic attribute & values
    memset(&new_att, 0, sizeof(rsi_ble_req_add_att_t));
    new_att.serv_handler       = serv_handler;
    new_att.handle             = handle + 1;
    new_att.att_uuid.size      = 2;
    new_att.att_uuid.val.val16 = RSI_BLE_CLIENT_CHAR_UUID;
    new_att.property           = RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE;
    new_att.data_len           = 2;

    //! add attribute to the service
    rsi_ble_add_attribute(&new_att);
  }

  return;
}

/*============================================================================*/
/**
 * @fn         rsi_ble_add_simple_chat_serv
 * @brief      this function is used to add new service i.e.., simple chat service.
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to create new service.
 */
static uint32_t rsi_ble_add_phase3_service(void)
{
  uuid_t new_uuid                       = { 0 };
  rsi_ble_resp_add_serv_t new_serv_resp = { 0 };
  uint8_t data[RSI_BLE_MAX_DATA_LEN]    = { "silabs_sampletest" };

  //0b282ff4-5347-472b-93da-f579103420fa
  static const uuid_t iop_phase3_service_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0x0b282ff4,
    .val.val128.data2 = 0x5347,
    .val.val128.data3 = 0x472b,
    .val.val128.data4 = { 0xda, 0x93, 0x79, 0xf5, 0xfa, 0x20, 0x34, 0x10 }
  };

  //148fd3c4-a00f-3905-d743-e94268e757e3
  static const uuid_t iop_phase3_control_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0x148fd3c4,
    .val.val128.data2 = 0xa00f,
    .val.val128.data3 = 0x3905,
    .val.val128.data4 = { 0x43, 0xd7, 0x42, 0xe9, 0xe3, 0x57, 0xe7, 0x68 }
  };

  //47b73dd6-dee3-4da1-9be0-f5c539a9a4be
  static const uuid_t iop_throughput_notify_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0x47b73dd6,
    .val.val128.data2 = 0xdee3,
    .val.val128.data3 = 0x4da1,
    .val.val128.data4 = { 0xe0, 0x9b, 0xc5, 0xf5, 0xbe, 0xa4, 0xa9, 0x39 }
  };

  //8824e363-7392-4bfc-81b6-3e58104cb2b0
  static const uuid_t iop_sc_pairing_uuid = { .size             = 16,
                                              .reserved         = { 0x00, 0x00, 0x00 },
                                              .val.val128.data1 = 0x8824e363,
                                              .val.val128.data2 = 0x7392,
                                              .val.val128.data3 = 0x4bfc,
                                              .val.val128.data4 = { 0xb6, 0x81, 0x58, 0x3e, 0xb0, 0xb2, 0x4c, 0x10 } };

  //d14a264f-cdac-c2a4-dce5-9b9ca2073aba
  static const uuid_t iop_sc_auth_uuid = { .size             = 16,
                                           .reserved         = { 0x00, 0x00, 0x00 },
                                           .val.val128.data1 = 0xd14a264f,
                                           .val.val128.data2 = 0xcdac,
                                           .val.val128.data3 = 0xc2a4,
                                           .val.val128.data4 = { 0xe5, 0xdc, 0x9c, 0x9b, 0xba, 0x3a, 0x07, 0xa2 } };

  //6a978442-f37b-a07c-1a5f-0e6f15a5fc83
  static const uuid_t iop_sc_bonding_uuid = { .size             = 16,
                                              .reserved         = { 0x00, 0x00, 0x00 },
                                              .val.val128.data1 = 0x6a978442,
                                              .val.val128.data2 = 0xf37b,
                                              .val.val128.data3 = 0xa07c,
                                              .val.val128.data4 = { 0x5f, 0x1a, 0x6f, 0x0e, 0x83, 0xfc, 0xa5, 0x15 } };

  //! adding new service
  new_uuid.size      = 16;
  new_uuid.val.val32 = RSI_BLE_NEW_SERVICE_UUID;
  rsi_ble_fill_128bit_uuid(RSI_BLE_NEW_SERVICE_UUID, &new_uuid);

  rsi_ble_add_service(iop_phase3_service_uuid, &new_serv_resp);

  //! adding control point characteristic service attribute to the service
  new_uuid.size      = 16;
  new_uuid.val.val32 = RSI_BLE_ATTRIBUTE_1_UUID;
  rsi_ble_fill_128bit_uuid(RSI_BLE_ATTRIBUTE_1_UUID, &new_uuid);
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 1,
                            RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                            new_serv_resp.start_handle + 2,
                            iop_phase3_control_uuid);

  //! adding control point characteristic value attribute to the service
  iop_phase3.phase3_control_handle = new_serv_resp.start_handle + 2;
  new_uuid.size                    = 16;
  new_uuid.val.val32               = RSI_BLE_ATTRIBUTE_1_UUID;
  rsi_ble_fill_128bit_uuid(RSI_BLE_ATTRIBUTE_1_UUID, &new_uuid);
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 2,
                           iop_phase3_control_uuid,
                           RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                           data,
                           sizeof(data),
                           1);

  //! adding throughput characteristic service attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_ATTRIBUTE_2_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 3,
                            RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY,
                            new_serv_resp.start_handle + 4,
                            iop_throughput_notify_uuid);

  //! adding throughput characteristic  value attribute to the service
  iop_phase3.tp_notify_handle = new_serv_resp.start_handle + 4;
  new_uuid.size               = 2;
  new_uuid.val.val16          = RSI_BLE_ATTRIBUTE_2_UUID;
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 4,
                           iop_throughput_notify_uuid,
                           RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY,
                           data,
                           sizeof(data),
                           1);

  //! adding just works characteristic service attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_ATTRIBUTE_2_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 6,
                            RSI_BLE_ATT_PROPERTY_READ,
                            new_serv_resp.start_handle + 7,
                            iop_sc_pairing_uuid);

  //! adding just works characteristic value attribute to the service
  //rsi_ble_att2_val_hndl = new_serv_resp.start_handle + 6;
  uint8_t sec_data   = 0x55;
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_ATTRIBUTE_2_UUID;
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 7,
                           iop_sc_pairing_uuid,
                           RSI_BLE_ATT_PROPERTY_READ,
                           &sec_data,
                           sizeof(sec_data),
                           1 | SEC_MODE_1_LEVEL_2);

  iop_phase3.handle_justworks = new_serv_resp.start_handle + 7;
  //! adding secure connection  characteristic service attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_ATTRIBUTE_2_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 8,
                            RSI_BLE_ATT_PROPERTY_READ,
                            new_serv_resp.start_handle + 9,
                            iop_sc_auth_uuid);
  iop_phase3.handle_auth = new_serv_resp.start_handle + 9;

  //! adding secure connection characteristic value attribute to the service
  //rsi_ble_att2_val_hndl = new_serv_resp.start_handle + 8;
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_ATTRIBUTE_2_UUID;
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 9,
                           iop_sc_auth_uuid,
                           RSI_BLE_ATT_PROPERTY_READ,
                           &sec_data,
                           sizeof(sec_data),
                           1 | SEC_MODE_1_LEVEL_3);

  //! adding bonding characteristic service attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_ATTRIBUTE_2_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 10,
                            RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY,
                            new_serv_resp.start_handle + 11,
                            iop_sc_bonding_uuid);

  iop_phase3.handle_bonding = new_serv_resp.start_handle + 11;

  //! adding bonding characteristic value attribute to the service
  //rsi_ble_att2_val_hndl = new_serv_resp.start_handle + 10;
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_ATTRIBUTE_2_UUID;
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 11,
                           iop_sc_bonding_uuid,
                           RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY,
                           &sec_data,
                           sizeof(sec_data),
                           1 | SEC_MODE_1_LEVEL_4);

  return 0;
}

/*============================================================================*/
/**
 * @fn         rsi_ble_iop_test_service
 * @brief      this function is used to add new service.
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to create new service.
 */
uint32_t rsi_ble_iop_test_service(void)
{
  //6a2857fe-9092-4e97-8aae-c028e5b361a8
  static const uuid_t iop_test_service = { .size             = 16,
                                           .reserved         = { 0x00, 0x00, 0x00 },
                                           .val.val128.data1 = 0x6a2857fe,
                                           .val.val128.data2 = 0x9092,
                                           .val.val128.data3 = 0x4e97,
                                           .val.val128.data4 = { 0xae, 0x8a, 0x28, 0xc0, 0xa8, 0x61, 0xb3, 0xe5 } };

  //9e453fb5-42ee-4ed0-aaa4-74e11fc2c79f
  static const uuid_t iop_stack_version = { .size             = 16,
                                            .reserved         = { 0x00, 0x00, 0x00 },
                                            .val.val128.data1 = 0x9e453fb5,
                                            .val.val128.data2 = 0x42ee,
                                            .val.val128.data3 = 0x4ed0,
                                            .val.val128.data4 = { 0xa4, 0xaa, 0xe1, 0x74, 0x9f, 0xc7, 0xc2, 0x1f } };

  //6cb60323-2d62-473d-8815-b73df2ee3517
  static const uuid_t iop_conn_parameters = { .size             = 16,
                                              .reserved         = { 0x00, 0x00, 0x00 },
                                              .val.val128.data1 = 0x6cb60323,
                                              .val.val128.data2 = 0x2d62,
                                              .val.val128.data3 = 0x473d,
                                              .val.val128.data4 = { 0x15, 0x88, 0x3d, 0xb7, 0x17, 0x35, 0xee, 0xf2 } };

  rsi_ble_resp_add_serv_t new_serv_resp = { 0 };
  //! adding new service- IOP test service
  rsi_ble_add_service(iop_test_service, &new_serv_resp);

  //! adding characteristic IOP stack version.
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 1,
                            RSI_BLE_ATT_PROPERTY_READ,
                            new_serv_resp.start_handle + 2,
                            iop_stack_version);
  //! addding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 2,
                           iop_stack_version,
                           RSI_BLE_ATT_PROPERTY_READ,
                           0,
                           1,
                           1);

  test_iop_service1.stack_version = new_serv_resp.start_handle + 2;

  //! adding characteristic IOP stack version.
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 3,
                            RSI_BLE_ATT_PROPERTY_READ,
                            new_serv_resp.start_handle + 4,
                            iop_conn_parameters);
  //! adding initial value to the characteristic.
  conn_info_t.PHY                 = 0x2;
  conn_info_t.supervision_timeout = 200;
  conn_info_t.responder_latency   = 0;
  conn_info_t.connection_interval = 10;
  conn_info_t.pdu_size            = 251;
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 4,
                           iop_conn_parameters,
                           RSI_BLE_ATT_PROPERTY_READ,
                           (uint8_t *)&conn_info_t,
                           sizeof(conn_info_t),
                           1);

  test_iop_service1.conn_param = new_serv_resp.start_handle + 4;

  return 0;
}

/*============================================================================*/
/**
 * @fn         rsi_ble_iop_test_properties
 * @brief      this function is used to add new service.
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to create new service.
 */
uint32_t rsi_ble_iop_test_properties(void)
{
  //75247986-db67-4e19-b0e3-df8e8170be68
  static const uuid_t iop_test_prop_service_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0x75247986,
    .val.val128.data2 = 0xdb67,
    .val.val128.data3 = 0x4e19,
    .val.val128.data4 = { 0xe3, 0xb0, 0x8e, 0xdf, 0x68, 0xbe, 0x70, 0x81 }
  };

  //b3319040-a720-44ce-84ec-d1a69420bbfb
  static const uuid_t read_only_length1_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0xb3319040,
    .val.val128.data2 = 0xa720,
    .val.val128.data3 = 0x44ce,
    .val.val128.data4 = { 0xec, 0x84, 0xa6, 0xd1, 0xfb, 0xbb, 0x20, 0x94 }
  };

  //b3319040-a720-44ce-84ec-d1a69420bbfc
  static const uuid_t read_only_length255_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0xb3319040,
    .val.val128.data2 = 0xa720,
    .val.val128.data3 = 0x44ce,
    .val.val128.data4 = { 0xec, 0x84, 0xa6, 0xd1, 0xfc, 0xbb, 0x20, 0x94 }
  };

  //38333e40-1d8f-47df-a850-9ced9508be27
  static const uuid_t write_only_length1_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0x38333e40,
    .val.val128.data2 = 0x1d8f,
    .val.val128.data3 = 0x47df,
    .val.val128.data4 = { 0x50, 0xa8, 0xed, 0x9c, 0x27, 0xbe, 0x08, 0x95 }
  };

  //38333e40-1d8f-47df-a850-9ced9508be28
  static const uuid_t write_only_length255_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0x38333e40,
    .val.val128.data2 = 0x1d8f,
    .val.val128.data3 = 0x47df,
    .val.val128.data4 = { 0x50, 0xa8, 0xed, 0x9c, 0x28, 0xbe, 0x08, 0x95 }
  };

  //b8c15871-f456-40bc-9785-c144af510fa6
  static const uuid_t write_without_resp_len1_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0xb8c15871,
    .val.val128.data2 = 0xf456,
    .val.val128.data3 = 0x40bc,
    .val.val128.data4 = { 0x85, 0x97, 0x44, 0xc1, 0xa6, 0x0f, 0x51, 0xaf }
  };

  //b8c15871-f456-40bc-9785-c144af510fa7
  static const uuid_t write_without_resp_len255_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0xb8c15871,
    .val.val128.data2 = 0xf456,
    .val.val128.data3 = 0x40bc,
    .val.val128.data4 = { 0x85, 0x97, 0x44, 0xc1, 0xa7, 0x0f, 0x51, 0xaf }
  };

  //14d56543-42e5-4771-a7f3-526dc92463a2
  static const uuid_t notify_len1_uuid = { .size             = 16,
                                           .reserved         = { 0x00, 0x00, 0x00 },
                                           .val.val128.data1 = 0x14d56543,
                                           .val.val128.data2 = 0x42e5,
                                           .val.val128.data3 = 0x4771,
                                           .val.val128.data4 = { 0xf3, 0xa7, 0x6d, 0x52, 0xa2, 0x63, 0x24, 0xc9 } };

  //14d56543-42e5-4771-a7f3-526dc92463a2
  static const uuid_t notify_len255_uuid = { .size             = 16,
                                             .reserved         = { 0x00, 0x00, 0x00 },
                                             .val.val128.data1 = 0x14d56543,
                                             .val.val128.data2 = 0x42e5,
                                             .val.val128.data3 = 0x4771,
                                             .val.val128.data4 = { 0xf3, 0xa7, 0x6d, 0x52, 0xa3, 0x63, 0x24, 0xc9 } };

  //2f1a964e-22f4-4e03-9edf-751c74f5793c
  static const uuid_t indicate_len1_uuid = { .size             = 16,
                                             .reserved         = { 0x00, 0x00, 0x00 },
                                             .val.val128.data1 = 0x2f1a964e,
                                             .val.val128.data2 = 0x22f4,
                                             .val.val128.data3 = 0x4e03,
                                             .val.val128.data4 = { 0xdf, 0x9e, 0x1c, 0x75, 0x3c, 0x79, 0xf5, 0x74 } };

  //2f1a964e-22f4-4e03-9edf-751c74f5793d
  static const uuid_t indicate_len_MTU_3_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0x2f1a964e,
    .val.val128.data2 = 0x22f4,
    .val.val128.data3 = 0x4e03,
    .val.val128.data4 = { 0xdf, 0x9e, 0x1c, 0x75, 0x3d, 0x79, 0xf5, 0x74 }
  };

  rsi_ble_resp_add_serv_t new_serv_resp = { 0 };
  //! adding new service- IOP test properties service
  rsi_ble_add_service(iop_test_prop_service_uuid, &new_serv_resp);

  //! adding characteristic Read only length 1
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 1,
                            RSI_BLE_ATT_PROPERTY_READ,
                            new_serv_resp.start_handle + 2,
                            read_only_length1_uuid);
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 2,
                           read_only_length1_uuid,
                           RSI_BLE_ATT_PROPERTY_READ,
                           (uint8_t *)&read_only_len1,
                           sizeof(read_only_len1),
                           0);

  iop_test_prop.read_only_len1 = new_serv_resp.start_handle + 2;

  //! adding characteristic Read only length255.
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 3,
                            RSI_BLE_ATT_PROPERTY_READ,
                            new_serv_resp.start_handle + 4,
                            read_only_length255_uuid);
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 4,
                           read_only_length255_uuid,
                           RSI_BLE_ATT_PROPERTY_READ,
                           (uint8_t *)&read_only_len255,
                           sizeof(read_only_len255),
                           1);
  iop_test_prop.read_only_len255 = new_serv_resp.start_handle + 4;

  //! adding characteristic write only length 1
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 5,
                            RSI_BLE_ATT_PROPERTY_WRITE,
                            new_serv_resp.start_handle + 6,
                            write_only_length1_uuid);

  iop_test_prop.write_only_len1 = new_serv_resp.start_handle + 6;
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 6,
                           write_only_length1_uuid,
                           RSI_BLE_ATT_PROPERTY_WRITE,
                           (uint8_t *)&write_value_len1,
                           sizeof(write_value_len1),
                           1);

  //! adding characteristic write only length 255
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 7,
                            RSI_BLE_ATT_PROPERTY_WRITE,
                            new_serv_resp.start_handle + 8,
                            write_only_length255_uuid);
  iop_test_prop.write_only_len255 = new_serv_resp.start_handle + 8;
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 8,
                           write_only_length255_uuid,
                           RSI_BLE_ATT_PROPERTY_WRITE,
                           (uint8_t *)&write_value_len1,
                           sizeof(write_value_len1),
                           1);

  //! adding characteristic write without response length 1
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 9,
                            RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE | RSI_BLE_ATT_PROPERTY_READ,
                            new_serv_resp.start_handle + 10,
                            write_without_resp_len1_uuid);

  iop_test_prop.write_without_resp_len1 = new_serv_resp.start_handle + 10;
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 10,
                           write_without_resp_len1_uuid,
                           RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE | RSI_BLE_ATT_PROPERTY_READ,
                           (uint8_t *)&write_value_len1,
                           sizeof(write_value_len1),
                           1);

  //! adding characteristic write without response length 255
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 11,
                            RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE | RSI_BLE_ATT_PROPERTY_READ,
                            new_serv_resp.start_handle + 12,
                            write_without_resp_len255_uuid);
  iop_test_prop.write_without_resp_len255 = new_serv_resp.start_handle + 12;
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 12,
                           write_without_resp_len255_uuid,
                           RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE | RSI_BLE_ATT_PROPERTY_READ,
                           (uint8_t *)&write_value,
                           sizeof(write_value),
                           1);

  //! adding characteristic notify length 1
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 13,
                            RSI_BLE_ATT_PROPERTY_NOTIFY,
                            new_serv_resp.start_handle + 14,
                            notify_len1_uuid);
  iop_test_prop.notify_len1 = new_serv_resp.start_handle + 14;
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 14,
                           notify_len1_uuid,
                           RSI_BLE_ATT_PROPERTY_NOTIFY,
                           &write_only_len1_value,
                           sizeof(write_only_len1_value),
                           1);
  //+ 15 handle assigned to cccd
  //! adding characteristic notify length 255
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 16,
                            RSI_BLE_ATT_PROPERTY_NOTIFY,
                            new_serv_resp.start_handle + 17,
                            notify_len255_uuid);
  iop_test_prop.notify_len3 = new_serv_resp.start_handle + 17;
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 17,
                           notify_len255_uuid,
                           RSI_BLE_ATT_PROPERTY_NOTIFY,
                           &write_only_len1_value,
                           sizeof(write_only_len1_value),
                           1);
  //18 will be cccd
  //! adding characteristic indicate length 1
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 19,
                            RSI_BLE_ATT_PROPERTY_INDICATE,
                            new_serv_resp.start_handle + 20,
                            indicate_len1_uuid);
  iop_test_prop.indicate_len1 = new_serv_resp.start_handle + 20;
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 20,
                           indicate_len1_uuid,
                           RSI_BLE_ATT_PROPERTY_INDICATE,
                           &write_only_len1_value,
                           sizeof(write_only_len1_value),
                           1);
  //indicate cccd 21
  //! adding characteristic indicate length MTU-3
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 22,
                            RSI_BLE_ATT_PROPERTY_INDICATE,
                            new_serv_resp.start_handle + 23,
                            indicate_len_MTU_3_uuid);
  iop_test_prop.indicate_len3 = new_serv_resp.start_handle + 23;
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 23,
                           indicate_len_MTU_3_uuid,
                           RSI_BLE_ATT_PROPERTY_INDICATE,
                           &write_only_len1_value,
                           sizeof(write_only_len1_value),
                           1);
  //CCCD handle 24
  return 0;
}

/*============================================================================*/
/**
 * @fn         rsi_ble_iop_test_characteristics
 * @brief      this function is used to add new service.
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to create new service.
 */
uint32_t rsi_ble_iop_test_characteristics(void)
{
  //3976265f-098c-4253-a2cd-99c16eb13f5c
  static const uuid_t iop_test_char_service_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0x3976265f,
    .val.val128.data2 = 0x098c,
    .val.val128.data3 = 0x4253,
    .val.val128.data4 = { 0xcd, 0xa2, 0xc1, 0x99, 0x5c, 0x3f, 0xb1, 0x6e }
  };

  //999ef454-a850-427f-a87f-e72bc00471ff
  static const uuid_t iop_test_length_1_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0x999ef454,
    .val.val128.data2 = 0xa850,
    .val.val128.data3 = 0x427f,
    .val.val128.data4 = { 0x7f, 0xa8, 0x2b, 0xe7, 0xff, 0x71, 0x04, 0xc0 }
  };

  //d4138f32-397d-407c-8275-a5dad47e4de6
  static const uuid_t iop_test_length_255_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0xd4138f32,
    .val.val128.data2 = 0x397d,
    .val.val128.data3 = 0x407c,
    .val.val128.data4 = { 0x75, 0x82, 0xda, 0xa5, 0xe6, 0x4d, 0x7e, 0xd4 }
  };

  //df8ff726-2022-4f9c-8b4d-96fd4acd3c71
  static const uuid_t iop_test_length_var4_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0xdf8ff726,
    .val.val128.data2 = 0x2022,
    .val.val128.data3 = 0x4f9c,
    .val.val128.data4 = { 0x4d, 0x8b, 0xfd, 0x96, 0x71, 0x3c, 0xcd, 0x4a }
  };

  //d4138f32-397d-407c-8275-a5dad47e4de7
  static const uuid_t iop_test_const_len1_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0xd4138f32,
    .val.val128.data2 = 0x397d,
    .val.val128.data3 = 0x407c,
    .val.val128.data4 = { 0x75, 0x82, 0xda, 0xa5, 0xe7, 0x4d, 0x7e, 0xd4 }
  };

  //d4138f32-397d-407c-8275-a5dad47e4de8
  static const uuid_t iop_test_const_len255_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0xd4138f32,
    .val.val128.data2 = 0x397d,
    .val.val128.data3 = 0x407c,
    .val.val128.data4 = { 0x75, 0x82, 0xda, 0xa5, 0xe8, 0x4d, 0x7e, 0xd4 }
  };

  //bb250d1b-154a-4aed-bfde-0c5e8d577064
  static const uuid_t iop_test_user_len1_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0xbb250d1b,
    .val.val128.data2 = 0x154a,
    .val.val128.data3 = 0x4aed,
    .val.val128.data4 = { 0xde, 0xbf, 0x5e, 0x0c, 0x64, 0x70, 0x57, 0x8d }
  };

  //bb250d1b-154a-4aed-bfde-0c5e8d57705e
  static const uuid_t iop_test_user_len255_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0xbb250d1b,
    .val.val128.data2 = 0x154a,
    .val.val128.data3 = 0x4aed,
    .val.val128.data4 = { 0xde, 0xbf, 0x5e, 0x0c, 0x5e, 0x70, 0x57, 0x8d }
  };

  //bb250d1b-154a-4aed-bfde-0c5e8d57705f
  static const uuid_t iop_test_user_len_var4_uuid = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0xbb250d1b,
    .val.val128.data2 = 0x154a,
    .val.val128.data3 = 0x4aed,
    .val.val128.data4 = { 0xde, 0xbf, 0x5e, 0x0c, 0x5f, 0x70, 0x57, 0x8d }
  };

  rsi_ble_resp_add_serv_t new_serv_resp = { 0 };
  //! adding new service- IOP test characteristic
  rsi_ble_add_service(iop_test_char_service_uuid, &new_serv_resp);

  //! adding characteristic Read only length 1
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 1,
                            RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                            new_serv_resp.start_handle + 2,
                            iop_test_length_1_uuid);
  char_type_handle.char_len1 = new_serv_resp.start_handle + 2;
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 2,
                           iop_test_length_1_uuid,
                           RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                           (uint8_t *)&read_only_len1,
                           sizeof(read_only_len1),
                           1);

  //! adding characteristic Read only length 255
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 3,
                            RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                            new_serv_resp.start_handle + 4,
                            iop_test_length_255_uuid);
  char_type_handle.char_len255 = new_serv_resp.start_handle + 4;
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 4,
                           iop_test_length_255_uuid,
                           RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                           (uint8_t *)&read_only_len1,
                           sizeof(read_only_len1),
                           1);

  //! adding characteristic Read only length 255
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 5,
                            RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                            new_serv_resp.start_handle + 6,
                            iop_test_length_var4_uuid);
  char_type_handle.char_len_var4 = new_serv_resp.start_handle + 6;
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 6,
                           iop_test_length_var4_uuid,
                           RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                           (uint8_t *)&read_only_len1,
                           sizeof(read_only_len1),
                           1);

  //! adding characteristic Read only length 255
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 7,
                            RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                            new_serv_resp.start_handle + 8,
                            iop_test_const_len1_uuid);
  char_type_handle.char_const_len1 = new_serv_resp.start_handle + 8;
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 8,
                           iop_test_const_len1_uuid,
                           RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                           (uint8_t *)&read_only_len1,
                           sizeof(read_only_len1),
                           1);

  //! adding characteristic Read only length 255
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 9,
                            RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                            new_serv_resp.start_handle + 10,
                            iop_test_const_len255_uuid);
  char_type_handle.char_const_len255 = new_serv_resp.start_handle + 10;
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 10,
                           iop_test_const_len255_uuid,
                           RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                           (uint8_t *)&read_only_len1,
                           sizeof(read_only_len1),
                           1);

  //! adding characteristic test user length 1
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 11,
                            RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                            new_serv_resp.start_handle + 12,
                            iop_test_user_len1_uuid);
  char_type_handle.char_usr_len1 = new_serv_resp.start_handle + 12;

  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 12,
                           iop_test_user_len1_uuid,
                           RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                           (uint8_t *)&read_only_len1,
                           sizeof(read_only_len1),
                           1);

  //! adding characteristic test user length 255
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 13,
                            RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                            new_serv_resp.start_handle + 14,
                            iop_test_user_len255_uuid);
  char_type_handle.char_usr_len255 = new_serv_resp.start_handle + 14;
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 14,
                           iop_test_user_len255_uuid,
                           RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                           (uint8_t *)&read_only_len1,
                           sizeof(read_only_len1),
                           1);

  //! adding characteristic test user length variable 4
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 15,
                            RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                            new_serv_resp.start_handle + 16,
                            iop_test_user_len_var4_uuid);
  char_type_handle.char_usr_len_var4 = new_serv_resp.start_handle + 16;
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 16,
                           iop_test_user_len_var4_uuid,
                           RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                           (uint8_t *)&read_only_len1,
                           sizeof(read_only_len1),
                           1);

  return 0;
}

/*============================================================================*/
/**
 * @fn         rsi_ble_add_device_info
 * @brief      this function is used to add new service.
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to create new service.
 */
void rsi_ble_add_device_info(void)
{
  uuid_t new_uuid                       = { 0 };
  rsi_ble_resp_add_serv_t new_serv_resp = { 0 };
  new_uuid.size                         = 2;
  new_uuid.val.val16                    = DEVICE_INFO;
  rsi_ble_add_service(new_uuid, &new_serv_resp);

  new_uuid.size      = 2;
  new_uuid.val.val16 = DEVICE_NAME;
  //! adding characteristic
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 1,
                            RSI_BLE_ATT_PROPERTY_READ,
                            new_serv_resp.start_handle + 2,
                            new_uuid);
  device_info.device_name_handle = new_serv_resp.start_handle + 2;
  new_uuid.size                  = 2;
  new_uuid.val.val16             = DEVICE_NAME;
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 2,
                           new_uuid,
                           RSI_BLE_ATT_PROPERTY_READ,
                           (uint8_t *)DEVICE_NAME_VALUE,
                           sizeof(DEVICE_NAME_VALUE),
                           1);

  new_uuid.size      = 2;
  new_uuid.val.val16 = MODEL_NUMBER;

  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 3,
                            RSI_BLE_ATT_PROPERTY_READ,
                            new_serv_resp.start_handle + 4,
                            new_uuid);
  device_info.model_number_handle = new_serv_resp.start_handle + 4;

  new_uuid.size      = 2;
  new_uuid.val.val16 = MODEL_NUMBER;
  //! adding initial value to the characteristic.
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 4,
                           new_uuid,
                           RSI_BLE_ATT_PROPERTY_READ,
                           (uint8_t *)MODEL_STRING,
                           sizeof(MODEL_STRING),
                           1);
}

#if FWUP_UPGRADE
/*============================================================================*/
/**
 * @fn         rsi_ble_prepare_128bit_uuid
 * @brief      this function is used to prepare the 128bit UUID
 * @param[in]  temp_service,received 128-bit service.
 * @param[out] temp_uuid,formed 128-bit service structure.
 * @return     none.
 * @section description
 * This function prepares the 128bit UUID
 */
static void rsi_ble_prepare_128bit_uuid(uint8_t temp_service[UUID_SIZE], uuid_t *temp_uuid)
{
  temp_uuid->val.val128.data1 =
    ((temp_service[0] << 24) | (temp_service[1] << 16) | (temp_service[2] << 8) | (temp_service[3]));
  temp_uuid->val.val128.data2    = ((temp_service[5]) | (temp_service[4] << 8));
  temp_uuid->val.val128.data3    = ((temp_service[7]) | (temp_service[6] << 8));
  temp_uuid->val.val128.data4[0] = temp_service[9];
  temp_uuid->val.val128.data4[1] = temp_service[8];
  temp_uuid->val.val128.data4[2] = temp_service[11];
  temp_uuid->val.val128.data4[3] = temp_service[10];
  temp_uuid->val.val128.data4[4] = temp_service[15];
  temp_uuid->val.val128.data4[5] = temp_service[14];
  temp_uuid->val.val128.data4[6] = temp_service[13];
  te
}

/*============================================================================*/
/**
 * @fn         rsi_ble_add_ota_serv
 * @brief      this function is used to create the OTA Service
 * @param[in]  none.
 * @return     none.
 * @section description
 * This function is used to create the OTA service
 * UUID: 1D14D6EE-FD63-4FA1-BFA4-8F47B42119F0
 */
static void rsi_ble_add_ota_serv(void)
{

  rsi_ble_resp_add_serv_t new_serv_resp = { 0 };
  uint8_t ota_service_uuid[UUID_SIZE]   = { 0x1d, 0x14, 0xd6, 0xee, 0xfd, 0x63, 0x4f, 0xa1,
                                            0xbf, 0xa4, 0x8f, 0x47, 0xb4, 0x21, 0x19, 0xf0 };
  rsi_ble_prepare_128bit_uuid(ota_service_uuid, &ota_service);
  ota_service.size = UUID_SIZE;
  rsi_ble_add_service(ota_service, &new_serv_resp);
  ota_serv_response.serv_handler = new_serv_resp.serv_handler;
  ota_serv_response.start_handle = new_serv_resp.start_handle;
}

/*============================================================================*/
/**
 * @fn         rsi_ble_add_ota_data_char_serv
 * @brief      this function is used to create the "OTA Data" charactersitic
 * service
 * @param[in]  none.
 * @return     none.
 * @section description
 * Firmware chunks are written on this characteristic
 * UUID: 984227F3-34FC-4045-A5D0-2C581F81A153
 */
static void rsi_ble_add_ota_data_char_serv(void)
{
  uint8_t ota_data_char_serv[UUID_SIZE] = { 0x98, 0x42, 0x27, 0xf3, 0x34, 0xfc, 0x40, 0x45,
                                            0xa5, 0xd0, 0x2c, 0x58, 0x1f, 0x81, 0xa1, 0x53 };

  rsi_ble_prepare_128bit_uuid(ota_data_char_serv, &ota_data_char_service);
  ota_data_char_service.size = UUID_SIZE;

  rsi_ble_add_char_serv_att(ota_serv_response.serv_handler,
                            ota_serv_response.start_handle + 3,
                            RSI_BLE_ATT_PROPERTY_WRITE | RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE,
                            ota_serv_response.start_handle + 4,
                            ota_data_char_service);

  rsi_ble_add_char_val_att(ota_serv_response.serv_handler,
                           ota_serv_response.start_handle + 4,
                           ota_data_char_service,
                           RSI_BLE_ATT_PROPERTY_WRITE | RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE,
                           chunk_data,
                           240,
                           0);
  ota_fw_tf_handle = ota_serv_response.start_handle + 4;
}

/*============================================================================*/
/**
 * @fn         rsi_ble_add_ota_info_char_serv
 * @brief      this function is used to create the OTA Information
 * characteristic service
 * @param[in]  none.
 * @return     none.
 * @section description
 * Information about the "Total number of chunks" and "Size of Last chunk" is
 * written on this characteristic UUID:F7BF3564-FB6D-4E53-88A4-5E37E0326063
 */
static void rsi_ble_add_ota_info_char_serv(void)
{

  uint8_t ota_info_char_serv[UUID_SIZE] = { 0xf7, 0xbf, 0x35, 0x64, 0xfb, 0x6d, 0x4e, 0x53,
                                            0x88, 0xa4, 0x5e, 0x37, 0xe0, 0x32, 0x60, 0x63 };
  ota_info_char_service.size            = UUID_SIZE;

  rsi_ble_prepare_128bit_uuid(ota_info_char_serv, &ota_info_char_service);
  rsi_ble_add_char_serv_att(ota_serv_response.serv_handler,
                            ota_serv_response.start_handle + 1,
                            RSI_BLE_ATT_PROPERTY_WRITE,
                            ota_serv_response.start_handle + 2,
                            ota_info_char_service);

  rsi_ble_add_char_val_att(ota_serv_response.serv_handler,
                           ota_serv_response.start_handle + 2,
                           ota_info_char_service,
                           RSI_BLE_ATT_PROPERTY_WRITE,
                           NULL,
                           5,
                           0);
  ota_fw_control_handle = ota_serv_response.start_handle + 2;
}

/*============================================================================*/
/**
 * @fn         rsi_ble_add_ota_fw_version_char_serv
 * @brief      This function is used to create Firmware version characteristic
 * @param[in]  none.
 * @return     none.
 * @section Description
 * This function is used to create Firmware version characteristic
 * UUID:4F4A2368-8CCA-451E-BFFF-CF0E2EE23E9F
 */
static void rsi_ble_add_ota_fw_version_char_serv(void)
{

  uint8_t fw_version_char_serv[UUID_SIZE] = { 0x4f, 0x4a, 0x23, 0x68, 0x8c, 0xca, 0x45, 0x1e,
                                              0xbf, 0xff, 0xcf, 0x0e, 0x2e, 0xe2, 0x3e, 0x9f };

  ota_fw_version_char_service.size = UUID_SIZE;
  rsi_ble_prepare_128bit_uuid(fw_version_char_serv, &ota_fw_version_char_service);
  rsi_ble_add_char_serv_att(ota_serv_response.serv_handler,
                            ota_serv_response.start_handle + 5,
                            RSI_BLE_ATT_PROPERTY_READ,
                            ota_serv_response.start_handle + 6,
                            ota_fw_version_char_service);
  rsi_ble_add_char_val_att(ota_serv_response.serv_handler,
                           ota_serv_response.start_handle + 6,
                           ota_fw_version_char_service,
                           RSI_BLE_ATT_PROPERTY_READ,
                           (uint8_t *)&version,
                           20,
                           0);
}

/*============================================================================*/
/**
 * @fn         rsi_ble_add_ota_bd_add_char_serv
 * @brief      This function is used to create BD address characteristic
 * @param[in]  none.
 * @return     none.
 * @section Description
 * This function is used to create BD address characteristic
 * UUID:4M4A2368-8CCA-451E-BFFF-CF0E2EE23E9F
 */
static void rsi_ble_add_ota_bd_add_char_serv(void)
{

  uint8_t bd_add_char_serv[UUID_SIZE] = { 0x4b, 0x4a, 0x23, 0x68, 0x8c, 0xca, 0x45, 0x1e,
                                          0xbf, 0xff, 0xcf, 0x0e, 0x2e, 0xe2, 0x3e, 0x9f };

  ota_bd_add_char_service.size = UUID_SIZE;
  rsi_ble_prepare_128bit_uuid(bd_add_char_serv, &ota_bd_add_char_service);
  rsi_ble_add_char_serv_att(ota_serv_response.serv_handler,
                            ota_serv_response.start_handle + 7,
                            RSI_BLE_ATT_PROPERTY_READ,
                            ota_serv_response.start_handle + 8,
                            ota_bd_add_char_service);

  rsi_ble_add_char_val_att(ota_serv_response.serv_handler,
                           ota_serv_response.start_handle + 8,
                           ota_bd_add_char_service,
                           RSI_BLE_ATT_PROPERTY_READ,
                           str_local_dev_address,
                           20,
                           0);
}

/*============================================================================*/
/**
 * @fn         rsi_ble_add_ota_fwup_serv
 * @brief      This function is used to add the OTA Service and its
 * characteristics in the module
 * @param[in]  none.
 * @return     none.
 * @section description
 * The entire firmware update process is done on this service and
 */
static void rsi_ble_add_ota_fwup_serv(void)
{
  //! Adding the "OTA Service" Primary service
  //! UUID: 1D14D6EE-FD63-4FA1-BFA4-8F47B42119F0
  rsi_ble_add_ota_serv();

  //! Adding the "OTA Information" characteristic service attribute
  //! UUID: F7BF3564-FB6D-4E53-88A4-5E37E0326063
  rsi_ble_add_ota_info_char_serv();

  //! Adding the "OTA Data" characteristic service attribute
  //! UUID: 984227F3-34FC-4045-A5D0-2C581F81A153
  rsi_ble_add_ota_data_char_serv();

  //! Adding the "OTA Firmware version" characteristic service attribute
  //! UUID: 4F4A2368-8CCA-451E-BFFF-CF0E2EE23E9F
  rsi_ble_add_ota_fw_version_char_serv();

  //! Adding the "OTA BD Address" characteristic service attribute
  //! UUID: 4B4A2368-8CCA-451E-BFFF-CF0E2EE23E9F
  rsi_ble_add_ota_bd_add_char_serv();
}
#endif

/*============================================================================*/
/**
 * @fn         rsi_ble_data_length_change_event
 * @brief      invoked when data length is set
 * @section description
 * This Callback function indicates data length is set
 */
void rsi_ble_data_length_change_event(rsi_ble_event_data_length_update_t *rsi_ble_data_length_update)
{
  memcpy(&data_length_update, rsi_ble_data_length_update, sizeof(rsi_ble_event_data_length_update_t));
  rsi_ble_app_set_event(RSI_APP_EVENT_DATA_LENGTH_CHANGE);
}

/*============================================================================*/
/**
 * @fn         rsi_ble_phy_update_complete_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This Callback function indicates disconnected device information and status
 */
void rsi_ble_phy_update_complete_event(rsi_ble_event_phy_update_t *rsi_ble_event_phy_update_complete)
{
  memcpy(&app_phy_update_complete, rsi_ble_event_phy_update_complete, sizeof(rsi_ble_event_phy_update_t));
  rsi_ble_app_set_event(RSI_APP_EVENT_PHY_UPDATE_COMPLETE);
}

/*============================================================================*/
/**
 * @fn         rsi_ble_on_conn_update_complete_event
 * @brief      invoked when connection is completed
 * @param[in]  rsi_ble_event_conn_update_complete, connected remote device information
 * @param[in]  resp_status, response status for connection
 * @return     none.
 * @section description
 * This Callback function indicates disconnected device information and status
 */
void rsi_ble_on_conn_update_complete_event(rsi_ble_event_conn_update_t *rsi_ble_event_conn_update_complete,
                                           uint16_t resp_status)
{
  UNUSED_PARAMETER(resp_status);
  memcpy(&event_conn_update_complete, rsi_ble_event_conn_update_complete, sizeof(rsi_ble_event_conn_update_t));
  memcpy(remote_dev_address, rsi_ble_event_conn_update_complete->dev_addr, RSI_DEV_ADDR_LEN);

  rsi_ble_app_set_event(RSI_BLE_CONN_UPDATE_EVENT);
}

/*============================================================================*/
/**
 * @fn         rsi_ble_simple_peripheral_on_remote_features_event
 * @brief      invoked when LE remote features event is received.
 * @param[in] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
void rsi_ble_simple_peripheral_on_remote_features_event(rsi_ble_event_remote_features_t *rsi_ble_event_remote_features)
{
  memcpy(&remote_dev_feature, rsi_ble_event_remote_features, sizeof(rsi_ble_event_remote_features_t));
}

/*============================================================================*/
/**
 * @fn         rsi_ble_on_mtu_event
 * @brief      its invoked when MTU events are received.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when MTU events are received
 */
static void rsi_ble_on_mtu_event(rsi_ble_event_mtu_t *rsi_ble_mtu)
{
  memcpy(&app_ble_mtu_event, rsi_ble_mtu, sizeof(rsi_ble_event_mtu_t));
  rsi_6byte_dev_address_to_ascii(str_remote_address, app_ble_mtu_event.dev_addr);

  rsi_ble_app_set_event(RSI_BLE_MTU_EVENT);
}

/*============================================================================*/
/**
 * @fn         rsi_ble_on_read_req_event
 * @brief      its invoked when read event is received.
 * @param[in]  event_id, it indicates write event id.
 * @param[in]  rsi_ble_read, read event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when read event are
 * received
 */
static void rsi_ble_on_read_req_event(uint16_t event_id, rsi_ble_read_req_t *rsi_ble_read)
{
  UNUSED_PARAMETER(event_id);
  memcpy(&app_ble_read_event, rsi_ble_read, sizeof(rsi_ble_read_req_t));

  rsi_ble_app_set_event(RSI_BLE_GATT_READ_EVENT);
  return;
}

/*============================================================================*/
/**
 * @fn         rsi_ble_on_gatt_write_event
 * @brief      its invoked when write events are received.
 * @param[in]  event_id, it indicates write event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write events are received
 */

static void rsi_ble_on_gatt_write_event(uint16_t event_id, rsi_ble_event_write_t *rsi_ble_write)
{
  UNUSED_PARAMETER(event_id);
  memcpy(&app_ble_write_event, rsi_ble_write, sizeof(rsi_ble_event_write_t));

  rsi_ble_app_set_event(RSI_BLE_GATT_WRITE_EVENT);
}

/*============================================================================*/
/**
 * @fn         rsi_ble_on_gatt_prepare_write_event
 * @brief      its invoked when prepare write request event is received.
 * @param[in]  event_id, it indicates write event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write/notify/indication events are
 * received
 */
static void rsi_ble_on_gatt_prepare_write_event(uint16_t event_id, rsi_ble_event_prepare_write_t *rsi_ble_write)
{
  UNUSED_PARAMETER(event_id);
  memcpy(&app_ble_prepare_event, rsi_ble_write, sizeof(rsi_ble_event_prepare_write_t));
  rsi_ble_app_set_event(RSI_BLE_PREPARE_WRITE_REQUEST_EVENT);

  return;
}

/*============================================================================*/
/**
 * @fn         rsi_ble_on_gatt_execute_write_event
 * @brief      its invoked when execute write request event is received.
 * @param[in]  event_id, it indicates write event id.
 * @param[in]  rsi_ble_execute_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when execute write request events are
 * received
 */
static void rsi_ble_on_gatt_execute_write_event(uint16_t event_id, rsi_ble_execute_write_t *rsi_ble_execute_write)
{
  UNUSED_PARAMETER(event_id);
  memcpy(&app_ble_execute_write_event, rsi_ble_execute_write, sizeof(rsi_ble_execute_write_t));
  rsi_ble_app_set_event(RSI_BLE_EXECUTE_WRITE_REQUEST_EVENT);
}

/*============================================================================*/
/**
 * @fn         ble_on_indicate_confirmation_event
 * @brief      its invoked when indication confirm is received.
 * @param[in]  event_status, it indicates event status.
 * @param[in]  rsi_ble_event_set_att_rsp, hold the response for the BLE set attribute
 *             request.
 * @return     none.
 * @section description
 * This callback function is invoked when indication confirm is received.
 */
static void ble_on_indicate_confirmation_event(uint16_t event_status, rsi_ble_set_att_resp_t *rsi_ble_event_set_att_rsp)
{
  UNUSED_PARAMETER(event_status);
  UNUSED_PARAMETER(rsi_ble_event_set_att_rsp);
}

/*============================================================================*/
/**
 * @fn         rsi_ble_more_data_req_event
 * @brief      its invoked when more data request is received.
 * @param[in]  rsi_ble_more_data_evt, hold the BLE event LE device buffer indication
 * @section description
 * This callback function is invoked when indication confirm is received.
 */
static void rsi_ble_more_data_req_event(rsi_ble_event_le_dev_buf_ind_t *rsi_ble_more_data_evt)
{
  UNUSED_PARAMETER(rsi_ble_more_data_evt);

  //! set conn specific event
  rsi_ble_app_set_event(RSI_BLE_MORE_DATA_REQ_EVENT);

  return;
}
/*============================================================================*/
/**
 * @fn         handle_phase3_control_char
 * @brief      its invoked for handling phase 3 control characteristic
 * @section description
 * This callback function is invoked to handle phase 3 control characteristic.
 */
void handle_phase3_control_char()
{
  if (app_ble_write_event.length == 3) {
    memcpy(gattdb_control_value, app_ble_write_event.att_value, app_ble_write_event.length);
    LOG_PRINT("Write request received: OS Type: 0x%02X, "
              "Security Level: 0x%02X.\n",
              (gattdb_control_value[0] & 0xFF),
              gattdb_control_value[1]);
    // Handle Mobile OS Type
    switch (gattdb_control_value[0] & 0xFF) {
      case ANDROID:
        LOG_PRINT("Mobile OS: Android.\n");
        break;
      case IOS:
        LOG_PRINT("Mobile OS: iOS.\n");
        break;
      default:
        break;
    }
    // Handle Security Level
    switch (gattdb_control_value[1] & 0xFF) {
      case rsi_bt_connection_mode1_level2: {
        LOG_PRINT("Security Level: Mode 1 Level 2."
                  "(Unauthenticated pairing with encryption).\n");
        connection_security = rsi_bt_connection_mode1_level2;
        clear_ltk_list();
        rsi_ble_disconnect((int8_t *)app_ble_write_event.dev_addr);
        break;
      }
      case rsi_bt_connection_mode1_level3: {
        LOG_PRINT("Security Level: Mode 1 Level 3"
                  "    (Authenticated pairing with encryption).\n");
        connection_security = rsi_bt_connection_mode1_level3;
        clear_ltk_list();
        rsi_ble_disconnect((int8_t *)app_ble_write_event.dev_addr);
        break;
      }
      case rsi_bt_connection_mode1_level4: {
        LOG_PRINT("Security Level: Mode 1 Level 4,"
                  "(Authenticated Secure Connections pairing).\n");
        connection_security = rsi_bt_connection_mode1_level4;
        clear_ltk_list();
        rsi_ble_disconnect((int8_t *)app_ble_write_event.dev_addr);
        break;
      }
      case 4: {
        connection_security = security_level_privacy;
        rsi_ble_disconnect((int8_t *)app_ble_write_event.dev_addr);
        break;
      }
      default:
        break;
    }
  }
}

#if FWUP_UPGRADE
int32_t handle_ota_process(uint16_t handle_value)
{
  if (handle_value == ota_fw_control_handle) {
    if (app_ble_write_event.att_value[0] == 0) {
      data_tf_start = 1;
      chunk_number  = 1;
    } else if (app_ble_write_event.att_value[0] == 3) {
      data_tf_start = 1;
      done          = true;
      result        = rsi_ble_gatt_write_response(app_ble_write_event.dev_addr, 0);
      osDelay(290);
    }
  } else if (handle_value == ota_fw_tf_handle) {
    if (data_tf_start == 1) {
      if (chunk_number == 1) {
        memcpy(&firmware_header_data[0], &app_ble_write_event.att_value[0], 64);
        memcpy(&firmware_chunk_fw_payload[0], &app_ble_write_event.att_value[0], app_ble_write_event.length);
        status = sl_si91x_fwup_start(firmware_header_data);
        LOG_PRINT("\r\n Firmware transfer in progress. Please wait....\r\n");
        start_timer = osKernelGetTickCount();
        chunk_number++;
        status = sl_si91x_fwup_load(firmware_chunk_fw_payload, app_ble_write_event.length);

      } else {
        status = sl_si91x_fwup_load(app_ble_write_event.att_value, app_ble_write_event.length);
        //! If "status" for loading the last chunk is 3, then all chunks
        //! received and loaded successfully
        if (status == FW_UP_SUCCESS) {
          stop_timer = osKernelGetTickCount();
#if (FW_UPGRADE_TYPE == TA_FW_UP)
          LOG_PRINT("\r\n Time in sec:%ld\r\n", (stop_timer - start_timer) / 1000);
          LOG_PRINT("\r\n TA Firmware transfer complete!\r\n");
          LOG_PRINT("\r\n Safe upgrade in Progress. Please wait....\r\n");

          status = sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
          printf("\r\nWi-Fi Deinit status : %lx\r\n", status);
          VERIFY_STATUS_AND_RETURN(status);
          osDelay(29000);

          status = sl_wifi_init(&config, NULL, sl_wifi_default_event_handler);
          if (status != SL_STATUS_OK) {
            LOG_PRINT("\r\nWi-Fi Initialization Failed, Error Code : 0x%lX\r\n", status);
            return status;
          } else {
            printf("\r\n Wi-Fi Initialization Successful\n");
          }

          status = sl_wifi_get_firmware_version(&version);
          VERIFY_STATUS_AND_RETURN(status);
          printf("\r\nFirmware version after update:\r\n");
          print_firmware_version(&version);
          return SL_STATUS_OK;
#endif

#if (FW_UPGRADE_TYPE == M4_FW_UP)
          LOG_PRINT("\r\n Time in sec:%ld\r\n", (stop_timer - start_timer) / 1000);
          LOG_PRINT("\r\n M4 Firmware transfer complete!\r\n");
#endif
          return status;
        }
      }
    }
  }
  return 1;
}
#endif

/*============================================================================*/
/**
 * @fn         handle_prepare_write_request
 * @brief      this function will handle the prepare write request
 * @param[in]  app_ble_prepare_event, write event parameters.
 * @return     none.
 * @section description
 * This function is used send prepare write response
 */
void handle_prepare_write_request(rsi_ble_event_prepare_write_t app_ble_prepare_event)
{
  uint16_t handle_value  = app_ble_prepare_event.handle[0];
  uint16_t actual_offset = (app_ble_prepare_event.offset[1] << 8 | app_ble_prepare_event.offset[0]);

  //! Test Case 5.2 --> Receives value 0x00 of 255 bytes, write response sent
  if (handle_value == char_type_handle.char_len255) {
    memcpy(&write_value[actual_offset], app_ble_prepare_event.att_value, app_ble_prepare_event.length);
    write_value_length = app_ble_prepare_event.length;
    result             = rsi_ble_gatt_prepare_write_response(app_ble_prepare_event.dev_addr,
                                                 handle_value,
                                                 actual_offset,
                                                 app_ble_prepare_event.length,
                                                 app_ble_prepare_event.att_value);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 5.2: prepare write len255 successful\n");
    } else {
      LOG_PRINT("Test Case 5.2: prepare write len255 failed\n");
    }
    //! Test Case 5.5 --> Receives value 0x00 of 255 bytes, error response sent
  } else if (handle_value == char_type_handle.char_const_len255) {
    result = rsi_ble_att_error_response(app_ble_prepare_event.dev_addr,
                                        handle_value,
                                        RSI_BLE_ATT_WRITE_REQUEST,
                                        WRITE_NOT_PERIMITTED);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 5.5: const len255 success\n");
    } else {
      LOG_PRINT("Test Case 5.5: const len255 failed\n");
    }
    //! Test Case 5.7 --> Receives value 0...254 of 255 bytes, write response sent
  } else if (handle_value == char_type_handle.char_usr_len255) {
    memcpy(&write_value_usr_len255[actual_offset], app_ble_prepare_event.att_value, app_ble_prepare_event.length);
    write_value_len_usr_len255 = write_value_len_usr_len255 + app_ble_prepare_event.length;

    result = rsi_ble_gatt_prepare_write_response(app_ble_prepare_event.dev_addr,
                                                 handle_value,
                                                 actual_offset,
                                                 app_ble_prepare_event.length,
                                                 app_ble_prepare_event.att_value);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 5.7: User len255 success\n");
    } else {
      LOG_PRINT("Test Case 5.7: User len255 failed\n");
    }
  }
}

/*============================================================================*/
/**
 * @fn         handle_write_response
 * @brief      this function will handle the write command/requests
 * @param[in]  write_event, write event parameters.
 * @return     none.
 * @section description
 * This function is used send write response and handles write command
 */
void handle_write_response(rsi_ble_event_write_t *write_event)
{
  uint16_t handle_value = (app_ble_write_event.handle[1] << 8) | app_ble_write_event.handle[0];
  switch (write_event->pkt_type) {
    case RSI_BLE_WRITE_CMD_EVENT: {
      //! Test Case 4.5 --> Receives value 0x00 of 1 byte
      if (handle_value == iop_test_prop.write_without_resp_len1) {
        memcpy(&write_value_len1, write_event->att_value, write_event->length);
        LOG_PRINT("Test Case 4.5: Write without response len1 Successful\n");
        //! Test Case 4.6 --> Receives value 0x00 of 255 byte
      } else if (handle_value == iop_test_prop.write_without_resp_len255) {
        memcpy(&write_value, write_event->att_value, write_event->length);
        write_value_length = write_event->length;
        LOG_PRINT("Test Case 4.6: Write without response with len255 SUccessfull\n");
        //! Test Case 6.1 --> Unack OTA update
      }
#if FWUP_UPGRADE
      else if (handle_value == ota_fw_control_handle || handle_value == ota_fw_tf_handle) {
        handle_ota_process(handle_value);
      }
#endif
    } break;
    case RSI_BLE_WRITE_REQUEST_EVENT: {
      //! Test Case 4.3 --> Receives 0x00 value of 1 byte, write response sent
      if (handle_value == iop_test_prop.write_only_len1) {
        memcpy(&write_value_len1, write_event->att_value, write_event->length);
        result = rsi_ble_gatt_write_response(write_event->dev_addr, 0);
        if (result == RSI_SUCCESS) {
          LOG_PRINT("Test Case 4.3: Write only len1 Successful\n");
        } else {
          LOG_PRINT("Test Case 4.3: Write only len1 Failed\n");
        }
        //! Test Case 4.4 --> Receives 0x00 value of 255 byte, write response sent
      } else if (handle_value == iop_test_prop.write_only_len255) {
        memcpy(&write_value, write_event->att_value, write_event->length);
        write_value_length = write_event->length;
        result             = rsi_ble_gatt_write_response(write_event->dev_addr, 0);
        if (result == RSI_SUCCESS) {
          LOG_PRINT("Test Case 4.4:Write Response Successful\n");
        } else {
          LOG_PRINT("Test Case 4.4:Write Response Failed\n");
        }
        //! Test Case 5.1 --> Receives 0x55 value of 1 byte, write response sent
      } else if (handle_value == char_type_handle.char_len1) {
        write_value_len1 = write_event->att_value[0];
        result           = rsi_ble_gatt_write_response(write_event->dev_addr, 0);
        if (result == RSI_SUCCESS) {
          LOG_PRINT("Test Case 5.1: write len1 response Successful\n");
        } else {
          LOG_PRINT("Test Case 5.1: write len1 response Failed\n");
        }
        //! Test Case 5.3 --> Receives 0x55/0x66 value of 1/4 byte, write response sent
      } else if (handle_value == char_type_handle.char_len_var4) {
        if (write_event->length <= sizeof(iop_test_chr_user_var_4_arr)) {
          iop_test_chr_user_var_4_len = write_event->length;
          memcpy(&iop_test_chr_user_var_4_arr, write_event->att_value, iop_test_chr_user_var_4_len);
          result = rsi_ble_gatt_write_response(write_event->dev_addr, 0);
          if (result == RSI_SUCCESS) {
            LOG_PRINT("Test Case 5.3: Var len4 write Successful\n");
          } else {
            LOG_PRINT("Test Case 5.3: Var len4 write Failed\n");
          }
        }
        //! Test Case 5.4 --> Receives 0x55 value of 1 byte, write response sent
      } else if (handle_value == char_type_handle.char_const_len1) {
        result = rsi_ble_att_error_response(write_event->dev_addr,
                                            handle_value,
                                            RSI_BLE_ATT_WRITE_REQUEST,
                                            WRITE_NOT_PERIMITTED);
        if (result == RSI_SUCCESS) {
          LOG_PRINT("Test Case 5.4: const len1 error response Successful\n");
        } else {
          LOG_PRINT("Test Case 5.4: const len1 error response Failed\n");
        }
        //! Test Case 5.5 --> Receives 1...254 value of 255 byte, error response sent
      } else if (handle_value == char_type_handle.char_const_len255) {
        result = rsi_ble_att_error_response(write_event->dev_addr,
                                            handle_value,
                                            RSI_BLE_ATT_WRITE_REQUEST,
                                            WRITE_NOT_PERIMITTED);
        if (result == RSI_SUCCESS) {
          LOG_PRINT("Test Case 5.5: const len255 error response Successful\n");
        } else {
          LOG_PRINT("Test Case 5.5: const len255 error response Failed\n");
        }
        //! Test Case 5.6 --> Receives 0x55 value of 1 byte, write response sent
      } else if (handle_value == char_type_handle.char_usr_len1) {
        if (write_event->length == 1 && write_event->att_value[0] == 0x55) {
          user_len1 = write_event->att_value[0];
          result    = rsi_ble_gatt_write_response(write_event->dev_addr, 0);
          if (result == RSI_SUCCESS) {
            LOG_PRINT("Test Case 5.6: User len255 write response Successful\n");
          } else {
            LOG_PRINT("Test Case 5.6: User len255 write response Failed\n");
          }
        }
        //! Test Case 5.7 --> Receives 0...254 value of 255 byte, write response sent
      } else if (handle_value == char_type_handle.char_usr_len255) {
        memcpy(write_only_len255_value, write_event->att_value, write_event->length);
        write_value_length = write_event->length;
        result             = rsi_ble_gatt_write_response(write_event->dev_addr, 0);
        if (result == RSI_SUCCESS) {
          LOG_PRINT("Test Case 5.7: User len255 write response Successful\n");
        } else {
          LOG_PRINT("Test Case 5.7: User len255 write response Failed\n");
        }
        //! Test Case 5.8 --> Receives 0x55/0x66 value of 1/4 byte, write response sent
      } else if (handle_value == char_type_handle.char_usr_len_var4) {
        if (write_event->length <= sizeof(iop_test_chr_user_var_4_arr)) {
          iop_test_chr_user_var_4_len = write_event->length;
          memcpy(&iop_test_chr_user_var_4_arr, write_event->att_value, iop_test_chr_user_var_4_len);
          result = rsi_ble_gatt_write_response(write_event->dev_addr, 0);
          if (result == RSI_SUCCESS) {
            LOG_PRINT("Test Case 5.8: User var len4 write response Successful\n");
          } else {
            LOG_PRINT("Test Case 5.8: User var len4 write response Failed\n");
          }
        }
        //! Test Case 6.2 -->Ack OTA upgrade
      }
#if FWUP_UPGRADE
      else if (handle_value == ota_fw_control_handle || handle_value == ota_fw_tf_handle) {
        handle_ota_process(handle_value);
        //! Test Case 7 --> Receives 3 byte data, write response sent
      }
#endif
      else if ((*(uint16_t *)app_ble_write_event.handle) == iop_phase3.phase3_control_handle) {
        result = rsi_ble_gatt_write_response(app_ble_write_event.dev_addr, 0);
        if (result == RSI_SUCCESS) {
          LOG_PRINT("Test Case 7: Write response success\n");
        } else {
          LOG_PRINT("Test Case 7: Write response failed\n");
        }
        osDelay(500);
        handle_phase3_control_char();
        //! Test Case 7.1 -->Throughput notification enable and disable is handled
      } else if ((*(uint16_t *)app_ble_write_event.handle - 1) == iop_phase3.tp_notify_handle) {
        if (app_ble_write_event.att_value[0] == NOTIFY_ENABLE) {
          LOG_PRINT("\r\n Remote device enabled the notification \n");
          //! set the data transfer event
          notifies_enabled = 0x01;
          tx_pkt_cnt       = 0;
          tx_start_timer   = osKernelGetTickCount();
          rsi_ble_app_set_event(RSI_DATA_TRANSMIT_EVENT);
        } else if (app_ble_write_event.att_value[0] == NOTIFY_DISABLE) {
          LOG_PRINT("\r\n Remote device disabled the notification \n");
          //! clear the data transfer event
          notifies_enabled = 0x00;
          tx_stop_timer    = osKernelGetTickCount();
          throughput_calculation(tx_start_timer, tx_stop_timer, tx_pkt_cnt);
          rsi_ble_app_clear_event(RSI_DATA_TRANSMIT_EVENT);
        }
        //! Test Case 4.7/4.8/4.9/4.10 --> Notification and Indication will sent
        //! for length 1 and MTU-3
      } else if ((iop_test_prop.notify_len1 + 1) == *((uint16_t *)app_ble_write_event.handle)
                 || (iop_test_prop.notify_len3 + 1) == *((uint16_t *)app_ble_write_event.handle)
                 || iop_test_prop.indicate_len1 + 1 == *((uint16_t *)app_ble_write_event.handle)
                 || *((uint16_t *)app_ble_write_event.handle) == (iop_test_prop.indicate_len3 + 1)
                 || *((uint16_t *)app_ble_write_event.handle) == (iop_phase3.handle_bonding + 1)) {
        if (app_ble_write_event.att_value[0] == 1) {
          notify_start = true;
          rsi_ble_app_set_event(RSI_BLE_GATT_SEND_DATA);
        } else if (app_ble_write_event.att_value[0] == 0) {
          notify_start   = false;
          indicate_start = false;
          rsi_ble_app_set_event(RSI_BLE_GATT_SEND_DATA);
        } else if (app_ble_write_event.att_value[0] == 2) {
          indicate_start = true;
          rsi_ble_app_set_event(RSI_BLE_GATT_SEND_DATA);
        }
      }
    } break;
    default:
      break;
  }
}

/*============================================================================*/
/**
 * @fn         handle_read_req
 * @brief      this function will handle the read request
 * @param[in]  dev_addr, remote device address
 * @param[in]  handle, characteristic handle
 * @param[in]  offset,
 * @section description
 * This function is used at application to create new service.
 */
void handle_read_req(uint8_t *dev_addr, uint16_t handle, uint16_t offset, uint8_t type)
{
  //! Test Case 3 --> Sends stack version
  if (handle == test_iop_service1.stack_version) {
    result = rsi_ble_gatt_read_response(dev_addr, type, handle, offset, 8, (const uint8_t *)&version);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 3:Stack Version sent Successfully\n");
    } else {
      LOG_PRINT("Test Case 3:Reading Stack Version Failed\n");
    }
    //! Test Case 3 --> Sends connection parameter
  } else if (handle == test_iop_service1.conn_param) {
    result = rsi_ble_gatt_read_response(dev_addr, type, handle, offset, 12, conn_params);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 3:Connection Params sent Successfully\n");
    } else {
      LOG_PRINT("Test Case 3:Reading Connection Params Failed\n");
    }
    //! Test Case 4.1 --> Receives read request for data len1 for 0x55
  } else if (handle == iop_test_prop.read_only_len1) {
    result =
      rsi_ble_gatt_read_response(dev_addr, type, handle, offset, sizeof(read_only_len1), (uint8_t *)&read_only_len1);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 4.1: Read only len1 Success\n");
    } else {
      LOG_PRINT("Test Case 4.1: Read only len1 Failed\n");
    }
    //! Test Case 4.2 --> Receives read request for data len255 for 0..254
  } else if (handle == iop_test_prop.read_only_len255) {
    uint16_t bytes_to_send = (uint16_t)(sizeof(read_only_len255) - offset);
    if (bytes_to_send > mtu_size - 1) {
      bytes_to_send = mtu_size - 1;
    }
    if (bytes_to_send > 0) {
      result =
        rsi_ble_gatt_read_response(dev_addr, type, handle, offset, bytes_to_send, (uint8_t *)&read_only_len255[offset]);
    }
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 4.2: len255 read successful\n");
    } else {
      LOG_PRINT("Test Case 4.2: len255 read failed\n");
    }
    //! Test Case 5.1 --> Receives read request for data len1 for 0x55
  } else if (handle == char_type_handle.char_len1) {
    result = rsi_ble_gatt_read_response(dev_addr,
                                        type,
                                        handle,
                                        offset,
                                        sizeof(write_value_len1),
                                        (uint8_t *)&write_value_len1);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 5.1: len1 read successful\n");
    } else {
      LOG_PRINT("Test Case 5.1: len1 read failed\n");
    }
    //! Test Case 4.2 --> Receives read request for data len255 for 0..254
  } else if (handle == char_type_handle.char_len255) {
    uint16_t bytes_to_send = (uint16_t)(sizeof(read_only_len255) - offset);
    if (bytes_to_send > mtu_size - 1) {
      bytes_to_send = mtu_size - 1;
    }
    if (bytes_to_send > 0) {
      result =
        rsi_ble_gatt_read_response(dev_addr, type, handle, offset, bytes_to_send, (uint8_t *)&read_only_len255[offset]);
    }
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 4.2: len255 read successful\n");
    } else {
      LOG_PRINT("Test Case 4.2: len255 read failed\n");
    }
    //! Test Case 5.3 --> Receives read request for data len1/len45 for 0x55/0x66
  } else if (handle == char_type_handle.char_len_var4) {
    result = rsi_ble_gatt_read_response(dev_addr,
                                        type,
                                        handle,
                                        offset,
                                        iop_test_chr_user_var_4_len,
                                        (uint8_t *)&iop_test_chr_user_var_4_arr);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 5.3: var len4 read successful\n");
    } else {
      LOG_PRINT("Test Case 5.3: var len4 read failed\n");
    }
    //! Test Case 5.4 --> Receives read request for data len1 for 0x55
  } else if (handle == char_type_handle.char_const_len1) {
    uint8_t const_len1 = 0x55;
    result = rsi_ble_gatt_read_response(dev_addr, type, handle, offset, sizeof(const_len1), (uint8_t *)&const_len1);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 5.4: const len1 read success\n");
    } else {
      LOG_PRINT("Test Case 5.4: const len1 read failed\n");
    }
    //! Test Case 5.5 --> Receives read request for data len255 for 0...254
  } else if (handle == char_type_handle.char_const_len255) {
    uint16_t bytes_to_send = (sizeof(read_only_len255) - offset);
    if (bytes_to_send > mtu_size - 1) {
      bytes_to_send = mtu_size - 1;
    }
    if (bytes_to_send > 0) {
      result =
        rsi_ble_gatt_read_response(dev_addr, type, handle, offset, bytes_to_send, (uint8_t *)&read_only_len255[offset]);
    }
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 5.5: const len255 read success\n");
    } else {
      LOG_PRINT("Test Case 5.5: const len255 read failed\n");
    }
    //! Test Case 5.6 --> Receives read request for data len1 for 0x55
  } else if (handle == char_type_handle.char_usr_len1) {
    result = rsi_ble_gatt_read_response(dev_addr, type, handle, offset, sizeof(user_len1), (uint8_t *)&user_len1);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 5.6: user len1 read success\n");
    } else {
      LOG_PRINT("Test Case 5.6: user len1 read failed\n");
    }
    //! Test Case 5.7 --> Receives read request for data len255 for 0..254
  } else if (handle == char_type_handle.char_usr_len255) {
    uint16_t bytes_to_send = (write_value_len_usr_len255 - offset);
    if (bytes_to_send > mtu_size - 1) {
      bytes_to_send = mtu_size - 1;
    } else {
      write_value_len_usr_len255 = 0;
    }
    if (bytes_to_send > 0) {
      result = rsi_ble_gatt_read_response(dev_addr,
                                          type,
                                          handle,
                                          offset,
                                          bytes_to_send,
                                          (uint8_t *)&write_value_usr_len255[offset]);
    }
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 5.7: user len255 read success\n");
    } else {
      LOG_PRINT("Test Case 5.7: user len255 read failed\n");
    }
    //! Test Case 5.3 --> Receives read request for data len1/len45 for 0x55/0x66
  } else if (handle == char_type_handle.char_usr_len_var4) {
    result = rsi_ble_gatt_read_response(dev_addr,
                                        type,
                                        handle,
                                        offset,
                                        iop_test_chr_user_var_4_len,
                                        (uint8_t *)&iop_test_chr_user_var_4_arr);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 5.8: user var len4 read success\n");
    } else {
      LOG_PRINT("Test Case 5.8: user var len4 read success\n");
    }
    //! Model number is sent
  } else if (handle == device_info.model_number_handle) {
    result = rsi_ble_gatt_read_response(dev_addr, type, handle, offset, sizeof(MODEL_STRING), (uint8_t *)MODEL_STRING);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Model number Read successful\n");
    } else {
      LOG_PRINT("Model Number Read failed\n");
    }
    //! Device name is sent
  } else if (handle == device_info.device_name_handle) {
    result = rsi_ble_gatt_read_response(dev_addr, type, handle, offset, sizeof(MODEL_STRING), (uint8_t *)MODEL_STRING);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Device Name Read successful\n");
    } else {
      LOG_PRINT("Device Name Read failed\n");
    }
    //! Test Case 8.1 --> Receives read request, 0x55 will be sent
  } else if (handle == iop_phase3.handle_justworks) {
    result = rsi_ble_gatt_read_response(dev_addr, type, handle, offset, sizeof(value), (uint8_t *)&value);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 8.1: Read successful\n");
    } else {
      LOG_PRINT("Test Case 8.1: Read failed\n");
    }
    //! Test Case 8.2 --> Receives read request, 0x55 will be sent
  } else if (handle == iop_phase3.handle_auth) {
    result = rsi_ble_gatt_read_response(dev_addr, type, handle, offset, sizeof(value), (uint8_t *)&value);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 8.2: Read successful\n");
    } else {
      LOG_PRINT("Test Case 8.2: Read failed\n");
    }
    //! Test Case 8.3 --> Receives read request, 0x55 will be sent
  } else if (handle == iop_phase3.handle_bonding) {
    result = rsi_ble_gatt_read_response(dev_addr, type, handle, offset, sizeof(value), (uint8_t *)&value);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 8.3: Read successful\n");
    } else {
      LOG_PRINT("Test Case 8.3: Read failed\n");
    }
    //! Test Case 4.5 --> Receives read request for len1, 0x55 will be sent
  } else if (handle == iop_test_prop.write_without_resp_len1) {
    result = rsi_ble_gatt_read_response(dev_addr,
                                        type,
                                        handle,
                                        offset,
                                        sizeof(write_value_len1),
                                        (uint8_t *)&write_value_len1);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 4.5: Read successful\n");
    } else {
      LOG_PRINT("Test Case 4.5: Read failed\n");
    }
    //! Test Case 4.6 --> Receives read request, 0...254 will be sent
  } else if (handle == iop_test_prop.write_without_resp_len255) {
    uint16_t bytes_to_send = (sizeof(write_value_send) - offset);
    if (bytes_to_send > mtu_size - 1) {
      bytes_to_send = mtu_size - 1;
    }
    if (bytes_to_send > 0) {
      result =
        rsi_ble_gatt_read_response(dev_addr, type, handle, offset, bytes_to_send, (uint8_t *)&write_value_send[offset]);
    }
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 4.6: len255 read successful\n");
    } else {
      LOG_PRINT("Test Case 4.6 : len255 read failed\n");
    }
  } else {
    LOG_PRINT("Invalid handle for Read Request\n");
  }
}

void app_test_data_init1(void)
{
  for (int i = 0; i < DATA_SIZE_NOTIFY; i++) {
    iop_test_notification_250_arr[i] = i;
  }
  for (int i = 0; i < DATA_LENGTH; i++) {
    read_only_len255[i] = i;
  }
}

/*============================================================================*/
/**
 * @fn         handle_notification
 * @brief      this function will handle the notifications sent to the peer device
 * @return     none.
 * @section description
 * This function will handle the notifications sent to the peer device
 */
void handle_notification(void)
{
  uint8_t data_len1_notify = 0x55;
  //! Test Case 4.7 --> Sends notification of length 1 byte
  if (*((uint16_t *)app_ble_write_event.handle) == iop_test_prop.notify_len1 + 1) {
    result = rsi_ble_notify_value(app_ble_write_event.dev_addr,
                                  iop_test_prop.notify_len1,
                                  sizeof(data_len1_notify),
                                  &data_len1_notify);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 4.7:Notification len1 sent successfully\n");
    } else {
      LOG_PRINT("Test Case 4.7:Notification len1 sent failed\n");
    }
    //! Test Case 4.8 --> Sends notification of length MTU-3
  } else if (*((uint16_t *)app_ble_write_event.handle) == (iop_test_prop.notify_len3 + 1)) {
    result = rsi_ble_notify_value(app_ble_write_event.dev_addr,
                                  iop_test_prop.notify_len3,
                                  mtu_size - 3,
                                  iop_test_notification_250_arr);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 4.8:Notification len255 sent successfully\n");
    } else {
      LOG_PRINT("Test Case 4.8:Notification len255 sent failed\n");
    }
  }
}

/*============================================================================*/
/**
 * @fn         handle_indication
 * @brief      this function will handle the indication sent to the peer device
 * @return     none.
 * @section description
 * This function will handle the indication sent to the peer device
 */
void handle_indication(void)
{
  uint8_t data_len1_notify = 0x55;
  //! Test Case 4.9 --> Sends indication of length 1 byte
  if (*((uint16_t *)app_ble_write_event.handle) == iop_test_prop.indicate_len1 + 1) {
    result = rsi_ble_indicate_value(app_ble_write_event.dev_addr, iop_test_prop.indicate_len1, 1, &data_len1_notify);
    if (result == RSI_SUCCESS) {
      LOG_PRINT("Test Case 4.9:Indication len1 successful\n");
    }
    //! Test Case 4.10 --> Sends indication of length MTU-3 byte
  } else if (*((uint16_t *)app_ble_write_event.handle) == iop_test_prop.indicate_len3 + 1) {
    result = rsi_ble_indicate_value(app_ble_write_event.dev_addr,
                                    iop_test_prop.indicate_len3,
                                    240 - 3,
                                    iop_test_notification_250_arr);
    if (result == 0) {
      LOG_PRINT("Test Case 4.10:Indication MTU-3 successful\n");
    }
  }
}

/*==============================================*/
/**
 * @fn         ble_throughput_test_app
 * @brief      This application is used for testing the SMP protocol.
 * @param[in]  none.
 * @return     none.
 * @section description
 * This function is used to test the SMP Protocol.
 */
void ble_iop_test_app(void *argument)
{
  UNUSED_PARAMETER(argument);
  int32_t status = 0;
  int32_t ix;
  rsi_ble_dev_ltk_list_t *ble_dev_ltk = NULL;
  int32_t event_id;
  uint8_t adv[31]                                            = { 2, 1, 6 };
  static uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN] = { 0 };
  uint8_t local_dev_addr[LOCAL_DEV_ADDR_LEN]                 = { 0 };

  //TP
  uint8_t send_buf[MAX_SEND_DATA_LEN] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
    58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 72, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86,
    87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
    45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 72,
    74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 00, 1,  2,
    3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
  };

  //! Wi-Fi initialization
  status = sl_wifi_init(&config, NULL, sl_wifi_default_event_handler);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nWi-Fi Initialization Failed, Error Code : 0x%lX\r\n", status);
    return;
  }
  LOG_PRINT("\r\nWireless Initialization Success\r\n");

  //! Firmware version Prints
  status = sl_wifi_get_firmware_version(&version);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nFirmware version Failed, Error Code : 0x%lX\r\n", status);
  } else {
    print_firmware_version(&version);
  }

  //! get the local device MAC address.
  status = rsi_bt_get_local_device_address(rsi_app_resp_get_dev_addr);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Get local device address failed = %lx\r\n", status);
    return;
  } else {
    rsi_6byte_dev_address_to_ascii(local_dev_addr, rsi_app_resp_get_dev_addr);
    LOG_PRINT("\r\n Local device address %s \r\n", local_dev_addr);
  }

  //! registering the GAP callback functions
  rsi_ble_gap_register_callbacks(NULL,
                                 rsi_ble_on_connect_event,
                                 rsi_ble_on_disconnect_event,
                                 NULL,
                                 rsi_ble_phy_update_complete_event,
                                 rsi_ble_data_length_change_event,
                                 rsi_ble_on_enhance_conn_status_event,
                                 NULL,
                                 rsi_ble_on_conn_update_complete_event,
                                 NULL);

  //! registering the GAP Extended callback functions
  rsi_ble_gap_extended_register_callbacks(rsi_ble_simple_peripheral_on_remote_features_event,
                                          rsi_ble_more_data_req_event);

  //! registering the GATT callback functions
  rsi_ble_gatt_register_callbacks(NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  rsi_ble_on_gatt_write_event,
                                  rsi_ble_on_gatt_prepare_write_event,
                                  rsi_ble_on_gatt_execute_write_event,
                                  rsi_ble_on_read_req_event,
                                  rsi_ble_on_mtu_event,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  ble_on_indicate_confirmation_event,
                                  NULL);

#if SMP_ENABLE
  //! registering the SMP callback functions
  rsi_ble_smp_register_callbacks(rsi_ble_on_smp_request,
                                 rsi_ble_on_smp_response,
                                 rsi_ble_on_smp_passkey,
                                 rsi_ble_on_smp_failed,
                                 rsi_ble_on_encrypt_started,
                                 rsi_ble_on_smp_passkey_display,
                                 rsi_ble_on_sc_passkey,
                                 rsi_ble_on_le_ltk_req_event,
                                 rsi_ble_on_le_security_keys,
                                 NULL,
                                 NULL);
#endif
  ble_main_task_sem = osSemaphoreNew(1, 0, NULL);
  if (ble_main_task_sem == NULL) {
    LOG_PRINT("Failed to create ble_main_task_sem\r\n");
    return;
  }

  app_test_data_init1();

  //!  initializing the application events map
  rsi_ble_app_init_events();

  //! Add Device Information Service
  rsi_ble_add_device_info();

  //! adding IOP test stack service
  rsi_ble_iop_test_service();

  //! adding IOP test properties service
  rsi_ble_iop_test_properties();

  //! adding IOP test characteristics service
  rsi_ble_iop_test_characteristics();

#if FWUP_UPGRADE
  //! Adding the ota fwup server
  rsi_ble_add_ota_fwup_serv();
#endif

  //! adding IOP Silicon Labs Phase 3 service
  rsi_ble_add_phase3_service();

  //! Set local name
  rsi_bt_set_local_name(RSI_BLE_DEVICE_NAME);

  //! prepare advertise data //local/device name
  adv[3] = strlen(RSI_BLE_DEVICE_NAME) + 1;
  adv[4] = 9;
  strcpy((char *)&adv[5], RSI_BLE_DEVICE_NAME);

  //! set advertise data
  rsi_ble_set_advertise_data(adv, strlen(RSI_BLE_DEVICE_NAME) + 5);

  //! start advertising
  status = rsi_ble_start_advertising();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\nstart advertising cmd failed with error code = %lx \n", status);
  } else {
    LOG_PRINT("\nStarted advertising, local name : %s\n", (char *)RSI_BLE_DEVICE_NAME);
  }

  //! waiting for events from controller.
  while (1) {

    //! checking for events list
    event_id = rsi_ble_app_get_event();

    if (event_id == -1) {
      osSemaphoreAcquire(ble_main_task_sem, osWaitForever);
      continue;
    }

    switch (event_id) {
      case RSI_BLE_CONN_EVENT: {
        //! event invokes when connection was completed

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_CONN_EVENT);

        //! Setting MTU Exchange event
        status = rsi_ble_mtu_exchange_event(remote_dev_address, MAX_MTU_SIZE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n MTU request cmd failed with error code = %lx \n", status);
        }

        //! initiating the SMP pairing process
        if (connection_security > rsi_bt_connection_mode1_level1) {
          ix = rsi_get_ltk_list(ble_dev_ltk_list, &temp_le_ltk_req);

          if (ix != -1) {
            LOG_PRINT("Device is bonded\n");
            break;
          }

          status = rsi_ble_smp_pair_request(remote_dev_address, smp_capabilities.io_capability, mitm_req);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n Initiating SMP Pairing process failed with status %lx \n", status);
          }
        }

      } break;

      case RSI_BLE_DISCONN_EVENT: {
        //! event invokes when disconnection was completed
        LOG_PRINT("\n Disconnected, str_remote_address : %s\r\n", str_remote_address);

#if FWUP_UPGRADE
        //! Once OTA is done, reboot the device over OTA disconnection
        if (done) {
          sl_si91x_soc_nvic_reset();
          done = false;
        }
#endif

        if (connection_security == rsi_bt_connection_mode1_level2) {
          mitm_req                       = 0;
          smp_capabilities.io_capability = 0x03;
          smp_capabilities.oob_data_flag = 0x00;

          smp_capabilities.enc_key_size         = MAXIMUM_ENC_KEY_SIZE_16;
          smp_capabilities.ini_key_distribution = 0;

          smp_capabilities.rsp_key_distribution = 0;
          smp_capabilities.auth_req             = AUTH_REQ_SC_BIT;
          status                                = rsi_ble_set_smp_pairing_cap_data(&smp_capabilities);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n rsi_ble_set_smp_pairing_cap_data = %lx", status);
          }

        } else if (connection_security == rsi_bt_connection_mode1_level3) {
          mitm_req                       = 1;
          smp_capabilities.io_capability = 0x00;
          smp_capabilities.oob_data_flag = 0x00;

          smp_capabilities.enc_key_size         = MAXIMUM_ENC_KEY_SIZE_16;
          smp_capabilities.ini_key_distribution = 0;

          smp_capabilities.rsp_key_distribution = 0;
          smp_capabilities.auth_req             = AUTH_REQ_MITM_BIT | AUTH_REQ_SC_BIT;
          status                                = rsi_ble_set_smp_pairing_cap_data(&smp_capabilities);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n rsi_ble_set_smp_pairing_cap_data = %lx", status);
          }

        } else if (connection_security == rsi_bt_connection_mode1_level4) {
          mitm_req                       = 1;
          smp_capabilities.io_capability = 0x00;
          smp_capabilities.oob_data_flag = 0x00;

          smp_capabilities.enc_key_size         = MAXIMUM_ENC_KEY_SIZE_16;
          smp_capabilities.ini_key_distribution = INITIATOR_KEYS_TO_DIST;

          smp_capabilities.rsp_key_distribution = RESPONDER_KEYS_TO_DIST;
          smp_capabilities.auth_req             = AUTH_REQ_BITS;
          status                                = rsi_ble_set_smp_pairing_cap_data(&smp_capabilities);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n rsi_ble_set_smp_pairing_cap_data = %lx", status);
          }

        } else if (connection_security == security_level_privacy) {
          mitm_req                       = 1;
          smp_capabilities.io_capability = 0x00;
          smp_capabilities.oob_data_flag = 0x00;

          smp_capabilities.enc_key_size         = MAXIMUM_ENC_KEY_SIZE_16;
          smp_capabilities.ini_key_distribution = INITIATOR_KEYS_TO_DIST;

          smp_capabilities.rsp_key_distribution = RESPONDER_KEYS_TO_DIST;
          smp_capabilities.auth_req             = AUTH_REQ_BITS;
          status                                = rsi_ble_set_smp_pairing_cap_data(&smp_capabilities);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n rsi_ble_set_smp_pairing_cap_data = %lx", status);
          }
          connection_security = rsi_bt_connection_mode1_level1;
        }

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_DISCONN_EVENT);

        //! clear all pending events
        rsi_ble_app_clear_all_event();

        conn_params_updated = 0;

        //! start advertising
        status = rsi_ble_start_advertising();
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n Start advertising cmd failed with error code = %lx \n", status);
        } else {
          LOG_PRINT("\n Started Advertising \n");
        }

      } break;
#if SMP_ENABLE
      case RSI_BLE_SMP_REQ_EVENT: {
        LOG_PRINT("\n In SMP request event, str_remote_address : %s\r\n", str_remote_address);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_REQ_EVENT);
        //! initiating the SMP pairing process
        status = rsi_ble_smp_pair_request(remote_dev_address, smp_capabilities.io_capability, mitm_req);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n Failed to send pair request = %lx", status);
        }
      } break;

      case RSI_BLE_SMP_RESP_EVENT: {
        LOG_PRINT("\n In SMP response event, str_remote_address : %s\r\n", str_remote_address);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_RESP_EVENT);

        //! initiating the SMP pairing process
        status = rsi_ble_smp_pair_response(remote_dev_address, smp_capabilities.io_capability, mitm_req);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n Failed to send SMP pair response with status %lx \n", status);
        }

      } break;

      case RSI_BLE_SMP_PASSKEY_EVENT: {
        LOG_PRINT("\n In SMP passkey event, str_remote_address : %s\r\n", str_remote_address);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_PASSKEY_EVENT);

        //! initiating the SMP pairing process
        status = rsi_ble_smp_passkey(remote_dev_address, RSI_BLE_SMP_PASSKEY);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n Failed to send smp passkey with status %lx \n", status);
        }

      } break;
      case RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT: {
        LOG_PRINT("\r\nIn SMP passkey display event\r\n");
        LOG_PRINT("\n Remote addr: %s, passkey: %s\r\n", str_remote_address, smp_passkey);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT);
      } break;

      case RSI_BLE_LTK_REQ_EVENT: {
        rsi_ble_app_clear_event(RSI_BLE_LTK_REQ_EVENT);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_LTK_REQ_EVENT);
        ix = rsi_get_ltk_list(ble_dev_ltk_list, &temp_le_ltk_req);

        if (ix != -1) {
          ble_dev_ltk = &ble_dev_ltk_list[ix];
        }

        if ((ix != -1) && (ble_dev_ltk != NULL)) {
          LOG_PRINT("\n positive reply\n");
          //!  give le ltk req reply cmd with positive reply
          status = rsi_ble_ltk_req_reply(temp_le_ltk_req.dev_addr,
                                         (1 | (ble_dev_ltk->enc_enable) | (ble_dev_ltk->sc_enable << 7)),
                                         ble_dev_ltk->localltk);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n Failed to restart smp pairing with status: 0x%lx \r\n", status);
          }
        }

        else {
          LOG_PRINT("\n Negative reply\n");
          //! give le ltk req reply cmd with negative reply
          status = rsi_ble_ltk_req_reply(temp_le_ltk_req.dev_addr, 0, NULL);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("Failed to send LTK negative reply with status %lx \n", status);
          }

          rsi_ble_disconnect((int8_t *)temp_le_ltk_req.dev_addr);
        }
      } break;

      case RSI_BLE_SC_PASSKEY_EVENT: {
        LOG_PRINT("\n Remote addr: %s, passkey: %06ld \r\n", str_remote_address, passkey);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SC_PASSKEY_EVENT);
        rsi_ble_smp_passkey(remote_dev_address, passkey);
      } break;

      case RSI_BLE_SECURITY_KEYS_EVENT: {
        //! event invokes when security keys are received
        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SECURITY_KEYS_EVENT);
        resolve_key.remote_dev_addr_type = dev_address_type;

        resolve_key.remote_ediv = temp_le_sec_keys.remote_ediv;
        memcpy(resolve_key.remote_rand, temp_le_sec_keys.remote_rand, sizeof(temp_le_sec_keys.remote_rand));
        memcpy(resolve_key.remote_ltk, temp_le_sec_keys.remote_ltk, sizeof(temp_le_sec_keys.remote_ltk));

        rsi_6byte_dev_address_to_ascii(resolve_key.remote_dev_addr, temp_le_sec_keys.dev_addr);
        memcpy(resolve_key.peer_irk, temp_le_sec_keys.remote_irk, sizeof(temp_le_sec_keys.remote_irk));
        memcpy(resolve_key.local_irk, temp_le_sec_keys.local_irk, sizeof(temp_le_sec_keys.local_irk));

        resolve_key.Identity_addr_type = temp_le_sec_keys.Identity_addr_type;
        rsi_6byte_dev_address_to_ascii(resolve_key.Identity_addr, temp_le_sec_keys.Identity_addr);

        update_resolvlist((rsi_ble_resolvlist_group_t *)&resolvlist, &resolve_key);

        //get resolvlist size
        status = rsi_ble_get_resolving_list_size(&app_resp_resolvlist_size);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n line %d -> status: 0x%lx\r\n", __LINE__, status);
          return;
        }

        //set address resolution enable and resolvable private address timeout
        status =
          rsi_ble_set_addr_resolution_enable(RSI_BLE_DEV_ADDR_RESOLUTION_ENABLE, RSI_BLE_SET_RESOLVABLE_PRIV_ADDR_TOUT);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n line %d -> status: 0x%lx\r\n", __LINE__, status);
          return;
        }

        //set privacy mode
        status = rsi_ble_set_privacy_mode(
          resolve_key.Identity_addr_type,
          rsi_ascii_dev_address_to_6bytes_rev(remote_dev_address, (int8_t *)resolve_key.Identity_addr),
          RSI_BLE_PRIVACY_MODE);

        if (status != RSI_SUCCESS) {
          LOG_PRINT("line %d -> status: 0x%lx\r\n", __LINE__, status);
          return;
        }
      } break;

      case RSI_BLE_SMP_FAILED_EVENT: {
        //! initiate SMP protocol as a Central
        LOG_PRINT("smp_failed, str_remote_address: %s\r\n", str_remote_address);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_FAILED_EVENT);
      } break;

      case RSI_BLE_ENCRYPT_STARTED_EVENT: {
        //! start the encrypt event

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_ENCRYPT_STARTED_EVENT);
      } break;

#endif
      case RSI_APP_EVENT_DATA_LENGTH_CHANGE: {

        //! clear the disconnected event.
        rsi_ble_app_clear_event(RSI_APP_EVENT_DATA_LENGTH_CHANGE);
      } break;

      case RSI_APP_EVENT_PHY_UPDATE_COMPLETE: {
        //! phy update complete event

        //! clear the phy updare complete event.
        rsi_ble_app_clear_event(RSI_APP_EVENT_PHY_UPDATE_COMPLETE);
      } break;

      case RSI_BLE_CONN_UPDATE_EVENT: {
        rsi_ble_app_clear_event(RSI_BLE_CONN_UPDATE_EVENT);

      } break;

      case RSI_BLE_MTU_EVENT: {
        //! event invokes when write/notification events received
        mtu_size = app_ble_mtu_event.mtu_size;

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_MTU_EVENT);

        status = rsi_ble_set_wo_resp_notify_buf_info(remote_dev_address, DLE_BUFFER_MODE, DLE_BUFFER_COUNT);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("Configure buffer mode failed with status %lx \n", status);
        }

        if (remote_dev_feature.remote_features[0] & 0x20) {
          status = rsi_ble_set_data_len(remote_dev_address, TX_LEN, TX_TIME);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n Set data length cmd failed with error code = %lx \n", status);
          }
        }
      } break;

      case RSI_BLE_GATT_WRITE_EVENT: {
        //! event invokes when write/notification events receive

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_GATT_WRITE_EVENT);
        handle_write_response(&app_ble_write_event);
      } break;

      case RSI_BLE_PREPARE_WRITE_REQUEST_EVENT: {
        handle_prepare_write_request(app_ble_prepare_event);

        rsi_ble_app_clear_event(RSI_BLE_PREPARE_WRITE_REQUEST_EVENT);
        break;
      }

      case RSI_BLE_EXECUTE_WRITE_REQUEST_EVENT:
        rsi_ble_app_clear_event(RSI_BLE_EXECUTE_WRITE_REQUEST_EVENT);

        result = rsi_ble_gatt_write_response(app_ble_execute_write_event.dev_addr, 1);
        if (result == 0) {
          LOG_PRINT("Execute write response success\n");
        } else {
          LOG_PRINT("Execute write response failed\n");
        }
        break;

      case RSI_BLE_GATT_READ_EVENT: {
        handle_read_req(app_ble_read_event.dev_addr,
                        app_ble_read_event.handle,
                        app_ble_read_event.offset,
                        app_ble_read_event.type);
        rsi_ble_app_clear_event(RSI_BLE_GATT_READ_EVENT);
        break;
      }

      case RSI_BLE_GATT_SEND_DATA: {
        rsi_ble_app_clear_event(RSI_BLE_GATT_SEND_DATA);
        if (true) {
          if (notify_start == true) {
            handle_notification();
          }
          if (indicate_start == true) {
            handle_indication();
          }
        }
      } break;
      case RSI_DATA_TRANSMIT_EVENT: {
        if (!conn_params_updated) {
          status = rsi_ble_conn_params_update(remote_dev_address,
                                              CONN_INTERVAL_MIN,
                                              CONN_INTERVAL_MAX,
                                              CONN_LATENCY,
                                              SUPERVISION_TIMEOUT);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n conn params update cmd failed with status = %lx \r\n", status);
          } else {
            conn_params_updated = 1;
          }
        }
        send_buf[0]++;
        if (notifies_enabled) {
          status =
            rsi_ble_notify_value(remote_dev_address, iop_phase3.tp_notify_handle, RSI_BLE_MAX_DATA_LEN, send_buf);
          if (status != RSI_SUCCESS) {
            if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
              //! wait for the more data request received from the device
              rsi_ble_app_clear_event(RSI_DATA_TRANSMIT_EVENT);
              break;
            } else {
              LOG_PRINT("\r\n Sending notification for TP failed with status = %lx \r\n", status);
              rsi_ble_app_clear_event(RSI_DATA_TRANSMIT_EVENT);
            }
          } else {
            tx_pkt_cnt = tx_pkt_cnt + 1;
          }
        } else {
          //! clear the served event
          rsi_ble_app_clear_event(RSI_DATA_TRANSMIT_EVENT);
        }
      } break;
      case RSI_BLE_MORE_DATA_REQ_EVENT: {
        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_MORE_DATA_REQ_EVENT);

        //! Resuming the data transfer on the more data req from the device.
        rsi_ble_app_set_event(RSI_DATA_TRANSMIT_EVENT);
      } break;
      default: {
      }
    }
  }
}

void app_init(void)
{
  osThreadNew((osThreadFunc_t)ble_iop_test_app, NULL, &thread_attributes);
}
