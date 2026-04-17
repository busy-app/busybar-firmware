/*******************************************************************************
* @file  app.c
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
/*************************************************************************
 *
 */

/*================================================================================
 * @brief : This file contains example application for BLE Secure Connections
 * @section Description :
 * This application demonstrates how to configure the Silabs device in Central
 * mode and connects with remote peripheral device and how to enable SMP (Security
 * Manager Protocol) pairing and a Secured passkey pairing.
 =================================================================================*/

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

//! BLE include file to refer BLE APIs
#include "ble_config.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"
#include <string.h>

//! Common include file
#include "rsi_common_apis.h"

//! local device name
#define RSI_BLE_DEVICE_NAME (void *)"BLE_THROUGHPUT"

//! local device IO capability
#define RSI_BLE_SMP_IO_CAPABILITY 0x00
#define RSI_BLE_SMP_PASSKEY       0
#define MITM_REQ                  0x01

#define MAX_MTU_SIZE      240
#define MAX_SEND_DATA_LEN 232

#define LOCAL_DEV_ADDR_LEN 18 // Length of the local device address

//! application event list
#define RSI_BLE_ADV_REPORT_EVENT          0x01
#define RSI_BLE_CONN_EVENT                0x02
#define RSI_BLE_DISCONN_EVENT             0x03
#define RSI_BLE_SMP_REQ_EVENT             0x04
#define RSI_BLE_SMP_RESP_EVENT            0x05
#define RSI_BLE_SMP_PASSKEY_EVENT         0x06
#define RSI_BLE_SMP_FAILED_EVENT          0x07
#define RSI_BLE_ENCRYPT_STARTED_EVENT     0x08
#define RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT 0x09
#define RSI_BLE_SC_PASSKEY_EVENT          0x0A
#define RSI_BLE_LTK_REQ_EVENT             0x0B
#define RSI_BLE_RECEIVE_REMOTE_FEATURES   0x0C
#define RSI_APP_EVENT_DATA_LENGTH_CHANGE  0x0D
#define RSI_APP_EVENT_PHY_UPDATE_COMPLETE 0x0E
#define RSI_BLE_CONN_UPDATE_EVENT         0x0F
#define RSI_BLE_MTU_EVENT                 0x10
#define RSI_BLE_GATT_WRITE_EVENT          0x11
#define RSI_BLE_MORE_DATA_REQ_EVENT       0x12
#define RSI_DATA_TRANSMIT_EVENT           0x13

//! error code
#define BT_HCI_COMMAND_DISALLOWED 0x4E0C

//! global parameters list
static volatile uint32_t ble_app_event_map;
static volatile uint32_t ble_app_event_mask;
static uint8_t str_remote_address[18];
static uint8_t remote_dev_address[6];

#if SMP_ENABLE
static uint32_t numeric_value;
static rsi_bt_event_encryption_enabled_t encrypt_keys;
static rsi_bt_event_le_ltk_request_t ble_ltk_req;
static rsi_bt_event_le_security_keys_t app_ble_sec_keys;
#endif

static rsi_ble_event_phy_update_t rsi_app_phy_update_complete;
static rsi_ble_event_remote_features_t remote_dev_feature;
static rsi_ble_event_mtu_t app_ble_mtu_event;
static rsi_ble_event_write_t app_ble_write_event;
static rsi_ble_event_data_length_update_t data_length_update;
static rsi_ble_event_conn_update_t event_conn_update_complete;
static uint16_t rsi_ble_att1_val_hndl;
static uint16_t rsi_ble_att2_val_hndl;
static uint8_t notifies_enabled;
static uint8_t remote_name[31];
static uint8_t remote_addr_type        = 0;
static uint8_t remote_dev_str_addr[18] = { 0 };
static uint8_t remote_dev_bd_addr[6]   = { 0 };
static uint8_t device_found            = 0;
static uint8_t conn_params_updated     = 0;
#if SMP_ENABLE
static uint8_t smp_passkey[BLE_PASSKEY_SIZE];
#endif
uint32_t tx_start_timer = 0;
uint32_t tx_stop_timer  = 0;
uint64_t tx_pkt_cnt     = 0;

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
                      | SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE),
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
#if RSI_BLE_GATT_INIT
                      | SL_SI91X_BLE_GATT_INIT
#endif
                      ),
                   .config_feature_bit_map = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP) }
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
  ble_app_event_mask = 0xFFFFFFFF;
  ble_app_event_mask = ble_app_event_mask; //To suppress warning while compiling

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
static void rsi_ble_app_set_event(uint32_t event_num)
{
  ble_app_event_map |= BIT(event_num);
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
  ble_app_event_map &= ~BIT(event_num);
  return;
}

/*==============================================*/
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
  memcpy(remote_dev_address, resp_enh_conn->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, resp_enh_conn->dev_addr);
  rsi_ble_app_set_event(RSI_BLE_CONN_EVENT);
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
  memcpy(remote_dev_address, resp_conn->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, resp_conn->dev_addr);
  rsi_ble_app_set_event(RSI_BLE_CONN_EVENT);
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
  memcpy(remote_dev_address, resp_disconnect->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, resp_disconnect->dev_addr);

  rsi_ble_app_set_event(RSI_BLE_DISCONN_EVENT);
}

#if SMP_ENABLE
/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_request 
 * @brief      its invoked when smp request event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when an SMP request event is received(we are in Central mode)
 * Note: Peripheral requested to start SMP request, we have to send SMP request command
 */
void rsi_ble_on_smp_request(rsi_bt_event_smp_req_t *event_smp_req)
{
  memcpy(remote_dev_address, event_smp_req->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, event_smp_req->dev_addr);

  rsi_ble_app_set_event(RSI_BLE_SMP_REQ_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_response 
 * @brief      its invoked when smp response event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when an SMP response event is received(we are in Peripheral mode)
 * Note: Central initiated SMP protocol, we have to send SMP response command
 */
void rsi_ble_on_smp_response(rsi_bt_event_smp_resp_t *event_smp_response)
{
  memcpy(remote_dev_address, event_smp_response->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, event_smp_response->dev_addr);

  rsi_ble_app_set_event(RSI_BLE_SMP_RESP_EVENT);
}

/*==============================================*/
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
  memcpy(remote_dev_address, event_smp_passkey->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, event_smp_passkey->dev_addr);

  rsi_ble_app_set_event(RSI_BLE_SMP_PASSKEY_EVENT);
}

/*==============================================*/
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
  memcpy(remote_dev_address, smp_passkey_display->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, smp_passkey_display->dev_addr);
  memcpy(smp_passkey, smp_passkey_display->passkey, sizeof(smp_passkey_display->passkey));

  rsi_ble_app_set_event(RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT);
}

void rsi_ble_on_sc_passkey(rsi_bt_event_sc_passkey_t *sc_passkey)
{
  memcpy(remote_dev_address, sc_passkey->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, sc_passkey->dev_addr);
  numeric_value = sc_passkey->passkey;

  rsi_ble_app_set_event(RSI_BLE_SC_PASSKEY_EVENT);
}

/*==============================================*/
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
  LOG_PRINT(" \r\n rsi_ble_on_le_ltk_req_event \r\n");
  memcpy(&ble_ltk_req, le_ltk_req, sizeof(rsi_bt_event_le_ltk_request_t));
  rsi_ble_app_set_event(RSI_BLE_LTK_REQ_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_le_security_keys
 * @brief      its invoked when le secure keys events is received.
 * @param[in]  rsi_ble_event_le_security_keys, secure event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when security keys event is received after encription started. 
 */
void rsi_ble_on_le_security_keys(rsi_bt_event_le_security_keys_t *rsi_ble_event_le_security_keys)
{
  memcpy(&app_ble_sec_keys, rsi_ble_event_le_security_keys, sizeof(rsi_bt_event_le_security_keys_t));
  //LOG_PRINT("remote_ediv: 0x%x\r\n", app_ble_sec_keys.remote_ediv);
}

/*==============================================*/
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

  memcpy(remote_dev_address, smp_failed_event->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, smp_failed_event->dev_addr);

  rsi_ble_app_set_event(RSI_BLE_SMP_FAILED_EVENT);
}

/*==============================================*/
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

  memcpy(&encrypt_keys, enc_enabled, sizeof(rsi_bt_event_encryption_enabled_t));
  rsi_ble_app_set_event(RSI_BLE_ENCRYPT_STARTED_EVENT);
}

#endif // SMP_ENABLE

/**
 * @fn         rsi_ble_fill_128bit_uuid
 * @brief      this function is used to form 128bit uuid of apple device from 32 bit uuid.
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
  new_att.property           = RSI_BLE_ATT_PROPERTY_READ;

  //! preparing the characteristic attribute value
  new_att.data_len = 6;
  new_att.data[0]  = val_prop;
  rsi_uint16_to_2bytes(&new_att.data[2], att_val_handle);
  rsi_uint16_to_2bytes(&new_att.data[4], att_val_uuid.val.val16);

  //! Add attribute to the service
  rsi_ble_add_attribute(&new_att);

  return;
}

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
  if (val_prop & RSI_BLE_ATT_PROPERTY_NOTIFY) {
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

/*==============================================*/
/**
 * @fn         rsi_ble_add_simple_chat_serv
 * @brief      this function is used to add new servcie i.e.., simple chat service.
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to create new service.
 */
static uint32_t rsi_ble_add_simple_chat_serv(void)
{
  uuid_t new_uuid                       = { 0 };
  rsi_ble_resp_add_serv_t new_serv_resp = { 0 };
  uint8_t data[RSI_BLE_MAX_DATA_LEN]    = { "silabs_sampletest" };

  //! adding new service
  new_uuid.size      = 16;
  new_uuid.val.val32 = RSI_BLE_NEW_SERVICE_UUID;
  rsi_ble_fill_128bit_uuid(RSI_BLE_NEW_SERVICE_UUID, &new_uuid);

  rsi_ble_add_service(new_uuid, &new_serv_resp);

  //! adding characteristic service attribute to the service
  new_uuid.size      = 16;
  new_uuid.val.val32 = RSI_BLE_ATTRIBUTE_1_UUID;
  rsi_ble_fill_128bit_uuid(RSI_BLE_ATTRIBUTE_1_UUID, &new_uuid);
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 1,
                            RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                            new_serv_resp.start_handle + 2,
                            new_uuid);

  //! adding characteristic value attribute to the service
  rsi_ble_att1_val_hndl = new_serv_resp.start_handle + 2;
  new_uuid.size         = 16;
  new_uuid.val.val32    = RSI_BLE_ATTRIBUTE_1_UUID;
  rsi_ble_fill_128bit_uuid(RSI_BLE_ATTRIBUTE_1_UUID, &new_uuid);
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 2,
                           new_uuid,
                           RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                           data,
                           sizeof(data),
                           0);

  //! adding characteristic service attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_ATTRIBUTE_2_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 3,
                            RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY,
                            new_serv_resp.start_handle + 4,
                            new_uuid);

  //! adding characteristic value attribute to the service
  rsi_ble_att2_val_hndl = new_serv_resp.start_handle + 4;
  new_uuid.size         = 2;
  new_uuid.val.val16    = RSI_BLE_ATTRIBUTE_2_UUID;
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 4,
                           new_uuid,
                           RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY,
                           data,
                           sizeof(data),
                           0);
  return 0;
}

/*==============================================*/
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

/*==============================================*/
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
  memcpy(&rsi_app_phy_update_complete, rsi_ble_event_phy_update_complete, sizeof(rsi_ble_event_phy_update_t));
  rsi_ble_app_set_event(RSI_APP_EVENT_PHY_UPDATE_COMPLETE);
}

void rsi_ble_on_conn_update_complete_event(rsi_ble_event_conn_update_t *rsi_ble_event_conn_update_complete,
                                           uint16_t resp_status)
{
  UNUSED_PARAMETER(resp_status);
  memcpy(&event_conn_update_complete, rsi_ble_event_conn_update_complete, sizeof(rsi_ble_event_conn_update_t));
  memcpy(remote_dev_address, rsi_ble_event_conn_update_complete->dev_addr, 6);

  rsi_ble_app_set_event(RSI_BLE_CONN_UPDATE_EVENT);
}

/*==============================================*/
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
  rsi_ble_app_set_event(RSI_BLE_RECEIVE_REMOTE_FEATURES);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_mtu_event
 * @brief      its invoked when write/notify/indication events are received.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write/notify/indication events are received
 */
static void rsi_ble_on_mtu_event(rsi_ble_event_mtu_t *rsi_ble_mtu)
{
  memcpy(&app_ble_mtu_event, rsi_ble_mtu, sizeof(rsi_ble_event_mtu_t));
  rsi_6byte_dev_address_to_ascii(str_remote_address, app_ble_mtu_event.dev_addr);

  rsi_ble_app_set_event(RSI_BLE_MTU_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_gatt_write_event
 * @brief      its invoked when write/notify/indication events are received.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write/notify/indication events are received
 */
static void rsi_ble_on_gatt_write_event(uint16_t event_id, rsi_ble_event_write_t *rsi_ble_write)
{
  UNUSED_PARAMETER(event_id);

  memcpy(&app_ble_write_event, rsi_ble_write, sizeof(rsi_ble_event_write_t));
  rsi_ble_app_set_event(RSI_BLE_GATT_WRITE_EVENT);
}

static void rsi_ble_more_data_req_event(rsi_ble_event_le_dev_buf_ind_t *rsi_ble_more_data_evt)
{
  UNUSED_PARAMETER(rsi_ble_more_data_evt);

  //! set conn specific event
  rsi_ble_app_set_event(RSI_BLE_MORE_DATA_REQ_EVENT);

  return;
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
  if (device_found == 1) {
    return;
  }

  memset(&remote_name, 0, sizeof(remote_name));
  BT_LE_ADPacketExtract(remote_name, adv_report->adv_data, adv_report->adv_data_len);

  remote_addr_type = adv_report->dev_addr_type;
  rsi_6byte_dev_address_to_ascii(remote_dev_str_addr, (uint8_t *)adv_report->dev_addr);
  memcpy((int8_t *)remote_dev_bd_addr, (uint8_t *)adv_report->dev_addr, 6);

#if (CONNECT_OPTION == CONN_BY_NAME)
  if ((device_found == 0) && ((strcmp((const char *)remote_name, RSI_REMOTE_DEVICE_NAME)) == 0)) {
    device_found = 1;

    rsi_ble_app_set_event(RSI_BLE_ADV_REPORT_EVENT);
    return;
  }
#elif (CONNECT_OPTION == CONN_BY_ADDR)
  if ((!strcmp(RSI_BLE_REMOTE_DEV_ADDR, (char *)remote_dev_str_addr))) {
    device_found = 1;
    rsi_ble_app_set_event(RSI_BLE_ADV_REPORT_EVENT);
  }
#endif

  return;
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
void ble_throughput_test_app(void *argument)
{
  UNUSED_PARAMETER(argument);
  int32_t status = 0;
  int32_t event_id;
  uint8_t adv[31]                                            = { 2, 1, 6 };
  sl_wifi_firmware_version_t version                         = { 0 };
  static uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN] = { 0 };
  uint8_t local_dev_addr[LOCAL_DEV_ADDR_LEN]                 = { 0 };
  uint8_t send_buf[MAX_SEND_DATA_LEN]                        = {
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
  rsi_ble_gap_register_callbacks(rsi_ble_on_adv_report_event,
                                 rsi_ble_on_connect_event,
                                 rsi_ble_on_disconnect_event,
                                 NULL,
                                 rsi_ble_phy_update_complete_event,
                                 rsi_ble_data_length_change_event,
                                 rsi_ble_on_enhance_conn_status_event,
                                 NULL,
                                 rsi_ble_on_conn_update_complete_event,
                                 NULL);

  rsi_ble_gap_extended_register_callbacks(rsi_ble_simple_peripheral_on_remote_features_event,
                                          rsi_ble_more_data_req_event);

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
                                  NULL,
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
                                  NULL,
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

  //!  initializing the application events map
  rsi_ble_app_init_events();

  rsi_ble_add_simple_chat_serv();

  //! Set local name
  rsi_bt_set_local_name(RSI_BLE_DEVICE_NAME);

  //! prepare advertise data //local/device name
  adv[3] = strlen(RSI_BLE_DEVICE_NAME) + 1;
  adv[4] = 9;
  strcpy((char *)&adv[5], RSI_BLE_DEVICE_NAME);

  //! set advertise data
  rsi_ble_set_advertise_data(adv, strlen(RSI_BLE_DEVICE_NAME) + 5);

#if (CONNECTION_ROLE == PERIPHERAL_ROLE)

  //! start advertising
  status = rsi_ble_start_advertising();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\nstart advertising cmd failed with error code = %lx \n", status);
  } else {
    LOG_PRINT("\nStarted advertising, local name : %s\n", (char *)RSI_BLE_DEVICE_NAME);
  }

#elif (CONNECTION_ROLE == CENTERAL_ROLE)

  status = rsi_ble_start_scanning();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n start scanning cmd failed with error code = %lx \n", status);
  } else {
    LOG_PRINT("\nStarted scanning \n");
  }
#endif

  //! waiting for events from controller.
  while (1) {

    //! checking for events list
    event_id = rsi_ble_app_get_event();

    if (event_id == -1) {
      osSemaphoreAcquire(ble_main_task_sem, osWaitForever);
      continue;
    }

    switch (event_id) {
#if (CONNECTION_ROLE == CENTERAL_ROLE)
      case RSI_BLE_ADV_REPORT_EVENT: {

        LOG_PRINT("\n Advertise report received \n");

        LOG_PRINT("\n Device found. Stop scanning \n");
        status = rsi_ble_stop_scanning();
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n Scan stop cmd failed = %lx\n", status);
        }
        LOG_PRINT("\n Connect command \n");
        status = rsi_ble_connect(remote_addr_type, (int8_t *)remote_dev_bd_addr);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n BLE connect cmd failed = %lx\n", status);
        }

        //! clear the advertise report event.
        rsi_ble_app_clear_event(RSI_BLE_ADV_REPORT_EVENT);
      } break;
#endif

      case RSI_BLE_CONN_EVENT: {
        //! event invokes when connection was completed
        LOG_PRINT("Connected, str_remote_address : %s\r\n", str_remote_address);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_CONN_EVENT);

        //! Setting MTU Exchange event
        status = rsi_ble_mtu_exchange_event(remote_dev_address, MAX_MTU_SIZE);
        if (status != RSI_SUCCESS) {

          LOG_PRINT("\n MTU request cmd failed with error code = %lx \n", status);
        }

        //! initiating the SMP pairing process
        //status = rsi_ble_smp_pair_request(remote_dev_address, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);
      } break;

      case RSI_BLE_DISCONN_EVENT: {
        //! event invokes when disconnection was completed
        LOG_PRINT("\n Disconnected, str_remote_address : %s\r\n", str_remote_address);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_DISCONN_EVENT);

        //! clear all pending events
        rsi_ble_app_clear_all_event();

        device_found        = 0;
        conn_params_updated = 0;

#if (CONNECTION_ROLE == PERIPHERAL_ROLE)
        //! start advertising
        status = rsi_ble_start_advertising();
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n Start advertising cmd failed with error code = %lx \n", status);
        } else {
          LOG_PRINT("\n Started Advertising \n");
        }
#elif (CONNECTION_ROLE == CENTERAL_ROLE)

        status = rsi_ble_start_scanning();
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n Start scanning cmd failed with error code = %lx \n", status);
        } else {
          LOG_PRINT("\n Started scanning \n");
        }
#endif
      } break;
#if SMP_ENABLE
      case RSI_BLE_SMP_REQ_EVENT: {
        //! initiate SMP protocol as a Central
        LOG_PRINT("\n smp_req, str_remote_address : %s\r\n", (char *)str_remote_address);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_REQ_EVENT);

        //! initiating the SMP pairing process
        status = rsi_ble_smp_pair_request(remote_dev_address, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n smp pair request cmd failed with error code = %lx \n", status);
        }
      } break;

      case RSI_BLE_SMP_RESP_EVENT: {
        //! initiate SMP protocol as a Central
        LOG_PRINT("\n smp_resp, str_remote_address : %s\r\n", (char *)str_remote_address);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_RESP_EVENT);

        //! initiating the SMP pairing process
        status = rsi_ble_smp_pair_response(remote_dev_address, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n smp pair response cmd failed with error code = %lx \n", status);
        }
      } break;

      case RSI_BLE_SMP_PASSKEY_EVENT: {
        //! initiate SMP protocol as a Central
        LOG_PRINT("\n smp_passkeystr, _remote_address : %s\r\n", (char *)str_remote_address);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_PASSKEY_EVENT);

        //! initiating the SMP pairing process
        status = rsi_ble_smp_passkey(remote_dev_address, RSI_BLE_SMP_PASSKEY);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n smp passkey cmd failed with error code = %lx \n", status);
        }
      } break;
      case RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT: {
        LOG_PRINT("\n remote addr: %s, passkey: %s \r\n", str_remote_address, smp_passkey);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT);
      } break;

      case RSI_BLE_SC_PASSKEY_EVENT: {
        LOG_PRINT("\n remote addr: %s, passkey: %06ld \r\n", str_remote_address, numeric_value);

        rsi_ble_app_clear_event(RSI_BLE_SC_PASSKEY_EVENT);
        status = rsi_ble_smp_passkey(remote_dev_address, numeric_value);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n secure connection passkey cmd failed with error code = %lx \n", status);
        }
      } break;

      case RSI_BLE_LTK_REQ_EVENT: {
        rsi_ble_app_clear_event(RSI_BLE_LTK_REQ_EVENT);
        status = rsi_ble_ltk_req_reply(remote_dev_address, 0, NULL);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n Ltk req reply cmd failed with error code = %lx \n", status);
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
      case RSI_BLE_RECEIVE_REMOTE_FEATURES: {
        //! clear the served event
        LOG_PRINT("\n Feature received is %x \n", *(uint8_t *)remote_dev_feature.remote_features);

        rsi_ble_app_clear_event(RSI_BLE_RECEIVE_REMOTE_FEATURES);

        if (remote_dev_feature.remote_features[0] & 0x20) {
          status = rsi_ble_set_data_len(remote_dev_address, TX_LEN, TX_TIME);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n Set data length cmd failed with error code = %lx \n", status);
            rsi_ble_app_set_event(RSI_BLE_RECEIVE_REMOTE_FEATURES);
            //return;
          }

        } else if (remote_dev_feature.remote_features[1] & 0x01) {
          status = rsi_ble_setphy((int8_t *)remote_dev_address, TX_PHY_RATE, RX_PHY_RATE, CODDED_PHY_RATE);
          if (status != RSI_SUCCESS) {
            if (status != BT_HCI_COMMAND_DISALLOWED) {
              //retry the same command
              rsi_ble_app_set_event(RSI_APP_EVENT_DATA_LENGTH_CHANGE);
            } else {
              LOG_PRINT("\n Set phy cmd failed with error code = %lx \n", status);
            }
          }
        } else {
#if (SMP_ENABLE && (CONNECTION_ROLE == CENTERAL_ROLE))
          //! initiating the SMP pairing process
          status = rsi_ble_smp_pair_request(remote_dev_address, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n smp pair request cmd failed with error code = %lx \n", status);
          }
#endif
        }

      } break;

      case RSI_APP_EVENT_DATA_LENGTH_CHANGE: {

        LOG_PRINT("\nMax_tx_octets: %d \r\n", data_length_update.MaxTxOctets);
        LOG_PRINT("Max_tx_time: %d \r\n", data_length_update.MaxTxTime);
        LOG_PRINT("Max_rx_octets: %d \r\n", data_length_update.MaxRxOctets);
        LOG_PRINT("Max_rx_time: %d \r\n", data_length_update.MaxRxTime);

        //! clear the disconnected event.
        rsi_ble_app_clear_event(RSI_APP_EVENT_DATA_LENGTH_CHANGE);

        if (remote_dev_feature.remote_features[1] & 0x01) {
          osDelay(500);
          status = rsi_ble_setphy((int8_t *)remote_dev_address, TX_PHY_RATE, RX_PHY_RATE, CODDED_PHY_RATE);
          if (status != RSI_SUCCESS) {
            if (status != BT_HCI_COMMAND_DISALLOWED) {
              //retry the same command
              rsi_ble_app_set_event(RSI_APP_EVENT_DATA_LENGTH_CHANGE);
            } else {
              LOG_PRINT("\n Set phy cmd failed with error code = %lx \n", status);
            }
          }
        } else {
#if (SMP_ENABLE && (CONNECTION_ROLE == CENTERAL_ROLE))
          //! initiating the SMP pairing process
          status = rsi_ble_smp_pair_request(remote_dev_address, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n smp pair request cmd failed with error code = %lx \n", status);
          }
#endif
        }

      } break;

      case RSI_APP_EVENT_PHY_UPDATE_COMPLETE: {
        //! phy update complete event
        LOG_PRINT("\n Tx Phy rate = %x  and Rx Phy rate = %x \n",
                  rsi_app_phy_update_complete.TxPhy,
                  rsi_app_phy_update_complete.RxPhy);

        //! clear the phy updare complete event.
        rsi_ble_app_clear_event(RSI_APP_EVENT_PHY_UPDATE_COMPLETE);
#if (SMP_ENABLE && (CONNECTION_ROLE == CENTERAL_ROLE))
        //! initiating the SMP pairing process
        status = rsi_ble_smp_pair_request(remote_dev_address, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n smp pair request cmd failed with error code = %lx \n", status);
        }
#endif
      } break;

      case RSI_BLE_CONN_UPDATE_EVENT: {

        LOG_PRINT("\n Connection parameters update completed \n ");
        LOG_PRINT("\n Connection interval = %d, Latency = %d, Supervision Timeout = %d \n",
                  event_conn_update_complete.conn_interval,
                  event_conn_update_complete.conn_latency,
                  event_conn_update_complete.timeout);

        rsi_ble_app_clear_event(RSI_BLE_CONN_UPDATE_EVENT);

      } break;

      case RSI_BLE_MTU_EVENT: {
        //! event invokes when write/notification events received
        LOG_PRINT("\n MTU size received from remote device(%s) is %u\n",
                  str_remote_address,
                  app_ble_mtu_event.mtu_size);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_MTU_EVENT);

        status = rsi_ble_set_wo_resp_notify_buf_info(remote_dev_address, DLE_BUFFER_MODE, DLE_BUFFER_COUNT);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\n Failed to set the buffer configuration mode, error:0x%lx \r\n", status);
          break;
        } else {
          LOG_PRINT("\n Buffer configuration done for notify and set_att cmds buf mode = %d , max buff count =%d \n",
                    DLE_BUFFER_MODE,
                    DLE_BUFFER_COUNT);
        }

      } break;

      case RSI_BLE_GATT_WRITE_EVENT: {

        //! event invokes when write/notification events receive
        LOG_PRINT("\n Received packet type = %u\n", app_ble_write_event.pkt_type);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_GATT_WRITE_EVENT);
        //LOG_PRINT("\nGWE\n");
        if ((*(uint16_t *)app_ble_write_event.handle - 1) == rsi_ble_att2_val_hndl) {
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
          status = rsi_ble_notify_value(remote_dev_address, rsi_ble_att2_val_hndl, RSI_BLE_MAX_DATA_LEN, send_buf);
          if (status != RSI_SUCCESS) {
            if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
              //LOG_PRINT("\r\n write with response failed with buffer error = %d \r\n", status);
              //! wait for the more data request received from the device
              rsi_ble_app_clear_event(RSI_DATA_TRANSMIT_EVENT);
              break;
            } else {
              LOG_PRINT("\r\n Write with response failed with status = %lx \r\n", status);
              //! clear the served event
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
  osThreadNew((osThreadFunc_t)ble_throughput_test_app, NULL, &thread_attributes);
}
