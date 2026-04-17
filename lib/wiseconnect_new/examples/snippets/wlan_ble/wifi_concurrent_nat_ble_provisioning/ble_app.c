/*******************************************************************************
* @file  ble_app.c
* @brief
*******************************************************************************
* # License
* <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
******************************************************************************/

/*================================================================================
 * @brief : This file contains an example application for Wi-Fi Concurrent NAT with BLE 
 * Provisioning.
 * @section Description :
 * This application brings up the SiWG917 as a station using BLE provisioning.
 * SiWG917 is then brought up with AP+STA interfaces in concurrent mode 
 * with NAT.
 * Third-party Wi-Fi clients connected to the SiWG917 can access the internet through
 * a third-party AP using NAT.
 =================================================================================*/

/**
 * Include files
 **/

//! SL Wi-Fi SDK includes
#include "sl_board_configuration.h"
#include "sl_constants.h"
#include "sl_wifi.h"
#include "sl_net_ip_types.h"
#include "cmsis_os2.h"
#include "sl_utility.h"
// BLE include file to refer BLE APIs
#include <rsi_ble_apis.h>
#include <rsi_bt_common_apis.h>
#include <rsi_common_apis.h>
#include <string.h>

#include "ble_config.h"
#include "wifi_config.h"

// BLE attribute service types uuid values
#define RSI_BLE_CHAR_SERV_UUID   0x2803
#define RSI_BLE_CLIENT_CHAR_UUID 0x2902

// BLE characteristic service uuid
#define RSI_BLE_NEW_SERVICE_UUID 0xAABB
#define RSI_BLE_ATTRIBUTE_1_UUID 0x1AA1
#define RSI_BLE_ATTRIBUTE_2_UUID 0x1BB1
#define RSI_BLE_ATTRIBUTE_3_UUID 0x1CC1

// max data length
#define RSI_BLE_MAX_DATA_LEN 66

// local device name
#define RSI_BLE_APP_DEVICE_NAME "BLE_CONFIGURATOR"

// attribute properties
#define RSI_BLE_ATT_PROPERTY_READ   0x02
#define RSI_BLE_ATT_PROPERTY_WRITE  0x08
#define RSI_BLE_ATT_PROPERTY_NOTIFY 0x10

// application event list
#define RSI_BLE_ENH_CONN_EVENT 0x01
#define RSI_BLE_DISCONN_EVENT  0x02
#define RSI_BLE_WLAN_SCAN_RESP 0x03

#define RSI_SSID                    0x0D
#define RSI_SECTYPE                 0x0E
#define BLE_WLAN_DISCONN_NOTIFY     0x0F
#define WLAN_ALREADY                0x10
#define WLAN_NOT_ALREADY            0x11
#define BLE_WLAN_TIMEOUT_NOTIFY     0x12
#define APP_FW_VERSION              0x13
#define BLE_WLAN_DISCONNECT_STATUS  0x14
#define BLE_WLAN_JOIN_STATUS        0x15
#define BLE_MTU_EVENT               0x16
#define BLE_CONN_UPDATE_EVENT       0x17
#define BLE_RECEIVE_REMOTE_FEATURES 0x18
#define BLE_DATA_LENGTH_CHANGE      0x19
#define START_BLE_ADVERTISING       0x1A
#define STOP_BLE_ADVERTISING        0x1B
#define INITIATE_BLE_DISCONNECT     0x1C

#define RSI_FW_VERSION 0x01

// Maximum length of SSID
#define RSI_SSID_LEN 34
// MAC address length
#define RSI_MAC_ADDR_LEN 6
// Maximum Acccess points that can be scanned
#define RSI_AP_SCANNED_MAX 11

//! connection update param
#define CONN_INTERVAL_MIN           0x320 //0x08
#define CONN_INTERVAL_MAX           0x320 //0x08
#define CONN_INTERVAL_DEFAULT_MIN   0x18
#define CONN_INTERVAL_DEFAULT_MAX   0x18
#define SUPERVISION_TIMEOUT_DEFAULT 400
#define CONN_LATENCY                0

/********************************************************************************************************
 *                                               DATA TYPES
 *********************************************************************************************************
 */

// global parameters list
uint8_t data[20] = { 0 };
static volatile uint32_t ble_app_event_map;
rsi_ble_event_conn_status_t conn_event_to_app;
static rsi_ble_event_disconnect_t disconn_event_to_app;
static uint8_t rsi_ble_att1_val_hndl;
static uint16_t rsi_ble_att2_val_hndl;
static uint16_t rsi_ble_att3_val_hndl;
uint8_t coex_ssid[50];
uint8_t pwd[RSI_BLE_MAX_DATA_LEN];
uint8_t sec_type;
uint8_t remote_dev_addr[18] = { 0 };
static rsi_ble_event_mtu_t app_ble_mtu_event;
static rsi_ble_event_conn_update_t event_conn_update_complete;
static rsi_ble_event_remote_features_t remote_dev_feature;
static rsi_ble_event_data_length_update_t updated_data_len_params;

sl_wifi_scan_result_t *scanresult = NULL;

extern uint8_t connected, disassosiated;
extern sl_net_ip_configuration_t ip_address;

extern osSemaphoreId_t ble_thread_sem;
extern uint16_t scanbuf_size;

/******************************************************
 *               Function Declarations
 ******************************************************/
extern void wifi_app_set_event(uint32_t event_num);
void rsi_ble_on_enhance_conn_status_event(rsi_ble_event_enhance_conn_status_t *resp_enh_conn);
void ble_configurator_init(void);
void ble_configurator_task(void *argument);
/*==============================================*/
/**
 * @fn         rsi_ble_add_char_serv_att
 * @brief      this function is used to add characteristic service attribute
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

  // preparing the attribute service structure
  new_att.serv_handler       = serv_handler;
  new_att.handle             = handle;
  new_att.att_uuid.size      = 2;
  new_att.att_uuid.val.val16 = RSI_BLE_CHAR_SERV_UUID;
  new_att.property           = RSI_BLE_ATT_PROPERTY_READ;

  // preparing the characteristic attribute value
  new_att.data_len = 6;
  new_att.data[0]  = val_prop;
  rsi_uint16_to_2bytes(&new_att.data[2], att_val_handle);
  rsi_uint16_to_2bytes(&new_att.data[4], att_val_uuid.val.val16);

  // add attribute to the service
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

static void rsi_ble_add_char_val_att(void *serv_handler,
                                     uint16_t handle,
                                     uuid_t att_type_uuid,
                                     uint8_t val_prop,
                                     uint8_t *data,
                                     uint8_t data_len)
{
  rsi_ble_req_add_att_t new_att = { 0 };

  // preparing the attributes
  new_att.serv_handler = serv_handler;
  new_att.handle       = handle;
  memcpy(&new_att.att_uuid, &att_type_uuid, sizeof(uuid_t));
  new_att.property = val_prop;

  // preparing the attribute value
  new_att.data_len = RSI_MIN(sizeof(new_att.data), data_len);
  memcpy(new_att.data, data, new_att.data_len);

  // add attribute to the service
  rsi_ble_add_attribute(&new_att);

  // check the attribute property with notification
  if (val_prop & RSI_BLE_ATT_PROPERTY_NOTIFY) {
    // if notification property supports then we need to add client characteristic service.

    // preparing the client characteristic attribute & values
    memset(&new_att, 0, sizeof(rsi_ble_req_add_att_t));
    new_att.serv_handler       = serv_handler;
    new_att.handle             = handle + 1;
    new_att.att_uuid.size      = 2;
    new_att.att_uuid.val.val16 = RSI_BLE_CLIENT_CHAR_UUID;
    new_att.property           = RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE;
    new_att.data_len           = 2;

    // add attribute to the service
    rsi_ble_add_attribute(&new_att);
  }

  return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_simple_chat_add_new_serv
 * @brief      this function is used to add new service i.e., simple chat service.
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to create new service.
 */
static uint32_t rsi_ble_add_configurator_serv(void)
{
  uuid_t new_uuid                       = { 0 };
  rsi_ble_resp_add_serv_t new_serv_resp = { 0 };
  uint8_t data[RSI_BLE_MAX_DATA_LEN]    = { 0 };

  new_uuid.size      = 2; // adding new service
  new_uuid.val.val16 = RSI_BLE_NEW_SERVICE_UUID;

  rsi_ble_add_service(new_uuid, &new_serv_resp);

  new_uuid.size      = 2; // adding characteristic service attribute to the service
  new_uuid.val.val16 = RSI_BLE_ATTRIBUTE_1_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 1,
                            RSI_BLE_ATT_PROPERTY_WRITE,
                            new_serv_resp.start_handle + 2,
                            new_uuid);

  rsi_ble_att1_val_hndl = new_serv_resp.start_handle + 2; // adding characteristic value attribute to the service
  new_uuid.size         = 2;
  new_uuid.val.val16    = RSI_BLE_ATTRIBUTE_1_UUID;
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 2,
                           new_uuid,
                           RSI_BLE_ATT_PROPERTY_WRITE,
                           data,
                           RSI_BLE_MAX_DATA_LEN);

  new_uuid.size      = 2; // adding characteristic service attribute to the service
  new_uuid.val.val16 = RSI_BLE_ATTRIBUTE_2_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 3,
                            RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                            new_serv_resp.start_handle + 4,
                            new_uuid);

  rsi_ble_att2_val_hndl = new_serv_resp.start_handle + 4; // adding characteristic value attribute to the service
  new_uuid.size         = 2;
  new_uuid.val.val16    = RSI_BLE_ATTRIBUTE_2_UUID;
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 4,
                           new_uuid,
                           RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
                           data,
                           RSI_BLE_MAX_DATA_LEN);

  new_uuid.size      = 2; // adding characteristic service attribute to the service
  new_uuid.val.val16 = RSI_BLE_ATTRIBUTE_3_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 5,
                            RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY,
                            new_serv_resp.start_handle + 6,
                            new_uuid);

  rsi_ble_att3_val_hndl = new_serv_resp.start_handle + 6; // adding characteristic value attribute to the service
  new_uuid.size         = 2;
  new_uuid.val.val16    = RSI_BLE_ATTRIBUTE_3_UUID;
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 6,
                           new_uuid,
                           RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY,
                           data,
                           RSI_BLE_MAX_DATA_LEN);
  return 0;
}

/*==============================================*/
/**
 * @fn         ble_app_init_events
 * @brief      initializes the event parameter.
 * @param[in]  none.
 * @return     none.
 * @section description
 * This function is used during BLE initialization.
 */
static void ble_app_init_events()
{
  ble_app_event_map = 0;
  return;
}

/*==============================================*/
/**
 * @fn         ble_app_set_event
 * @brief      sets the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to set/raise the specific event.
 */
static void ble_app_set_event(uint32_t event_num)
{
  ble_app_event_map |= BIT(event_num);

  osSemaphoreRelease(ble_thread_sem);

  return;
}

/*==============================================*/
/**
 * @fn         ble_app_clear_event
 * @brief      clears the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to clear the specific event.
 */
static void ble_app_clear_event(uint32_t event_num)
{
  ble_app_event_map &= ~BIT(event_num);
  return;
}

/*==============================================*/
/**
 * @fn         ble_app_get_event
 * @brief      returns the first set event based on priority
 * @param[in]  none.
 * @return     int32_t
 *             > 0  = event number
 *             -1   = not received any event
 * @section description
 * This function returns the highest priority event among all the set events
 */
static int32_t ble_app_get_event(void)
{
  uint32_t ix;

  for (ix = 0; ix < 32; ix++) {
    if (ble_app_event_map & (1 << ix)) {
      return ix;
    }
  }

  return (-1);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_enhance_conn_status_event
 * @brief      invoked when enhanced connection complete event is received
 * @param[out] resp_enh_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
void rsi_ble_on_enhance_conn_status_event(rsi_ble_event_enhance_conn_status_t *resp_enh_conn)
{
  conn_event_to_app.dev_addr_type = resp_enh_conn->dev_addr_type;
  memcpy(conn_event_to_app.dev_addr, resp_enh_conn->dev_addr, RSI_DEV_ADDR_LEN);
  conn_event_to_app.status = resp_enh_conn->status;
  ble_app_set_event(RSI_BLE_ENH_CONN_EVENT);
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
  memcpy(&conn_event_to_app, resp_conn, sizeof(rsi_ble_event_conn_status_t));
  ble_app_set_event(RSI_BLE_ENH_CONN_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_disconnect_event
 * @brief      invoked when disconnection event is received
 * @param[out]  resp_disconnect, disconnected remote device information
 * @param[out]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This Callback function indicates disconnected device information and status
 */
static void rsi_ble_on_disconnect_event(rsi_ble_event_disconnect_t *resp_disconnect, uint16_t reason)
{
  UNUSED_PARAMETER(reason);
  memcpy(&disconn_event_to_app, resp_disconnect, sizeof(rsi_ble_event_disconnect_t));
  ble_app_set_event(RSI_BLE_DISCONN_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_conn_update_complete_event
 * @brief      invoked when conn update complete event is received
 * @param[out] rsi_ble_event_conn_update_complete contains the controller
 * support conn information.
 * @param[out] resp_status contains the response status (Success or Error code)
 * @return     none.
 * @section description
 * This Callback function indicates the conn update complete event is received
 */
void rsi_ble_on_conn_update_complete_event(rsi_ble_event_conn_update_t *rsi_ble_event_conn_update_complete,
                                           uint16_t resp_status)
{
  UNUSED_PARAMETER(resp_status);
  rsi_6byte_dev_address_to_ascii(remote_dev_addr, (uint8_t *)rsi_ble_event_conn_update_complete->dev_addr);
  memcpy(&event_conn_update_complete, rsi_ble_event_conn_update_complete, sizeof(rsi_ble_event_conn_update_t));
  ble_app_set_event(BLE_CONN_UPDATE_EVENT);
}

/*============================================================================*/
/**
 * @fn         rsi_ble_on_remote_features_event
 * @brief      invoked when LE remote features event is received.
 * @param[out] rsi_ble_event_remote_features, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the remote device features
 */
void rsi_ble_on_remote_features_event(rsi_ble_event_remote_features_t *rsi_ble_event_remote_features)
{
  memcpy(&remote_dev_feature, rsi_ble_event_remote_features, sizeof(rsi_ble_event_remote_features_t));
  ble_app_set_event(BLE_RECEIVE_REMOTE_FEATURES);
}
/*============================================================================*/
/**
 * @fn         rsi_ble_data_length_change_event
 * @brief      invoked when data length is set
 * @param[out] rsi_ble_data_length_update, data length information
 * @section description
 * This Callback function indicates data length is set
 */
void rsi_ble_data_length_change_event(rsi_ble_event_data_length_update_t *rsi_ble_data_length_update)
{
  memcpy(&updated_data_len_params, rsi_ble_data_length_update, sizeof(rsi_ble_event_data_length_update_t));
  ble_app_set_event(BLE_DATA_LENGTH_CHANGE);
}
/*==============================================*/
/**
 * @fn         rsi_ble_on_mtu_event
 * @brief      invoked  when an MTU size event is received
 * @param[out]  rsi_ble_mtu, it indicates MTU size.
 * @return     none.
 * @section description
 * This callback function is invoked  when an MTU size event is received
 */
static void rsi_ble_on_mtu_event(rsi_ble_event_mtu_t *rsi_ble_mtu)
{
  memcpy(&app_ble_mtu_event, rsi_ble_mtu, sizeof(rsi_ble_event_mtu_t));
  rsi_6byte_dev_address_to_ascii(remote_dev_addr, app_ble_mtu_event.dev_addr);
  ble_app_set_event(BLE_MTU_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_gatt_write_event
 * @brief      this is call back function, it invokes when write/notify events received.
 * @param[out]  event_id, it indicates write/notification event id.
 * @param[out]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This is a callback function
 */
static void rsi_ble_on_gatt_write_event(uint16_t event_id, rsi_ble_event_write_t *rsi_ble_write)
{
  UNUSED_PARAMETER(event_id);
  uint8_t cmdid;

  //  Requests will come from Mobile app
  if ((rsi_ble_att1_val_hndl) == *((uint16_t *)rsi_ble_write->handle)) {
    cmdid = rsi_ble_write->att_value[0];

    switch (cmdid) {
      // Scan command request
      case '3': //else if(rsi_ble_write->att_value[0] == '3')
      {
        LOG_PRINT("\r\n[BLE]: Received scan request\r\n");
        memset(data, 0, sizeof(data));
        wifi_app_set_event(WIFI_APP_SCAN_STATE);
      } break;

      // Sending SSID
      case '2': //else if(rsi_ble_write->att_value[0] == '2')
      {
        memset(coex_ssid, 0, sizeof(coex_ssid));
        strcpy((char *)coex_ssid, (const char *)&rsi_ble_write->att_value[3]);

        ble_app_set_event(RSI_SSID);
      } break;

      // Sending Security type
      case '5': //else if(rsi_ble_write->att_value[0] == '5')
      {
        sec_type = ((rsi_ble_write->att_value[3]) - '0');
        LOG_PRINT("\r\n[BLE]: In Security Request\r\n");

        ble_app_set_event(RSI_SECTYPE);
      } break;

      // Sending PSK
      case '6': //else if(rsi_ble_write->att_value[0] == '6')
      {
        memset(data, 0, sizeof(data));
        strcpy((char *)pwd, (const char *)&rsi_ble_write->att_value[3]);
        LOG_PRINT("\r\n[BLE]: PWD from ble app\r\n");
        wifi_app_set_event(WIFI_APP_JOIN_STATE);
      } break;

      // WLAN Status Request
      case '7': //else if(rsi_ble_write->att_value[0] == '7')
      {
        LOG_PRINT("\r\n[BLE]: WLAN status request received\r\n");
        memset(data, 0, sizeof(data));
        if (connected) {
          ble_app_set_event(WLAN_ALREADY);
        } else {
          ble_app_set_event(WLAN_NOT_ALREADY);
        }
      } break;

      // WLAN disconnect request
      case '4': //else if(rsi_ble_write->att_value[0] == '4')
      {
        LOG_PRINT("\r\n[BLE]: WLAN disconnect request received\r\n");
        memset(data, 0, sizeof(data));
        wifi_app_set_event(WIFI_APP_DISCONN_NOTIFY_STATE);
      } break;

      // FW version request
      case '8': {
        memset(data, 0, sizeof(data));
        ble_app_set_event(APP_FW_VERSION);
        LOG_PRINT("\r\n[BLE]: FW version request\r\n");
      } break;

      default:
        LOG_PRINT("\r\n[BLE]: Default command case \r\n");
        break;
    }
  }
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_init
 * @brief      initialize the BLE module.
 * @param[in]  none
 * @return     none.
 * @section description
 * This function is used to initialize the BLE module
 */
void ble_configurator_init(void)
{
  uint8_t adv[31] = { 2, 1, 6 };

  //  initializing the application events map
  ble_app_init_events();

  rsi_ble_add_configurator_serv(); // adding simple BLE chat service

  // registering the GAP callback functions
  rsi_ble_gap_register_callbacks(NULL,
                                 rsi_ble_on_connect_event,
                                 rsi_ble_on_disconnect_event,
                                 NULL,
                                 NULL,
                                 rsi_ble_data_length_change_event,
                                 rsi_ble_on_enhance_conn_status_event,
                                 NULL,
                                 rsi_ble_on_conn_update_complete_event,
                                 NULL);
  //! registering the GAP extended call back functions
  rsi_ble_gap_extended_register_callbacks(rsi_ble_on_remote_features_event, NULL);

  // registering the GATT callback functions
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

  // Set local name
  rsi_bt_set_local_name((uint8_t *)RSI_BLE_APP_DEVICE_NAME);

  // prepare advertise data //local/device name
  adv[3] = strlen(RSI_BLE_APP_DEVICE_NAME) + 1;
  adv[4] = 9;
  strcpy((char *)&adv[5], RSI_BLE_APP_DEVICE_NAME);

  // set advertise data
  rsi_ble_set_advertise_data(adv, strlen(RSI_BLE_APP_DEVICE_NAME) + 5);

  // set device in advertising mode.
  rsi_ble_start_advertising();
  LOG_PRINT("\r\n[BLE]: Advertising Started...\r\n");
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_task
 * @brief      this function will execute when BLE events are raised.
 * @param[in]  none.
 * @return     none.
 * @section description
 */

void ble_configurator_task(void *argument)
{
  UNUSED_PARAMETER(argument);

  int32_t status = 0;
  int32_t event_id;
  uint8_t data[RSI_BLE_MAX_DATA_LEN] = { 0 };
  uint8_t scan_ix, length;
  uint8_t k;
  uint8_t Is_ble_disconnected_by_wifi = 0;

  scanresult = (sl_wifi_scan_result_t *)malloc(scanbuf_size);
  if (scanresult == NULL) {
    LOG_PRINT("\r\n[BLE]: Failed to allocate memory for scan result\r\n");
    return;
  }
  memset(scanresult, 0, scanbuf_size);
  while (1) {
    // checking for events list
    event_id = ble_app_get_event();
    if (event_id == -1) {
      osSemaphoreAcquire(ble_thread_sem, osWaitForever);
      // if events are not received loop will be continued.
      continue;
    }

    switch (event_id) {
      case RSI_BLE_ENH_CONN_EVENT: {
        // event invokes when connection was completed

        // clear the served event
        ble_app_clear_event(RSI_BLE_ENH_CONN_EVENT);

        LOG_PRINT("\r\n[BLE]: BLE Connected - remote_dev_addr : %s\r\n",
                  rsi_6byte_dev_address_to_ascii(remote_dev_addr, conn_event_to_app.dev_addr));

        //MTU exchange
        status = rsi_ble_mtu_exchange_event(conn_event_to_app.dev_addr, BLE_MTU_SIZE);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n[BLE]: MTU request failed with error code %lx\r\n", status);
        }
        status = rsi_ble_conn_params_update(conn_event_to_app.dev_addr,
                                            CONN_INTERVAL_DEFAULT_MIN,
                                            CONN_INTERVAL_DEFAULT_MAX,
                                            CONN_LATENCY,
                                            SUPERVISION_TIMEOUT_DEFAULT);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n[BLE]: rsi_ble_conn_params_update command failed : %lx\r\n", status);
        }
      } break;

      case RSI_BLE_DISCONN_EVENT: {
        // event invokes when disconnection was completed

        // clear the served event
        ble_app_clear_event(RSI_BLE_DISCONN_EVENT);
        LOG_PRINT("\r\n[BLE]: BLE Disconnected - remote_dev_addr : %s\r\n",
                  rsi_6byte_dev_address_to_ascii(remote_dev_addr, disconn_event_to_app.dev_addr));
// set device in advertising mode.
adv:
        if (Is_ble_disconnected_by_wifi == 0 && connected == 0) {
          status = rsi_ble_start_advertising();
          if (status != SL_STATUS_OK) {
            LOG_PRINT("\r\n[BLE]: start advertising cmd failed with error code = %lx\r\n", status);
            goto adv;
          } else {
            LOG_PRINT("\r\n[BLE]: Started Advertising\r\n");
            break;
          }
        }
        Is_ble_disconnected_by_wifi = 0;
        wifi_app_set_event(WIFI_AP_BRING_UP_STATE);
      } break;

      case APP_FW_VERSION: {
        sl_wifi_firmware_version_t firmware_version = { 0 };

        ble_app_clear_event(APP_FW_VERSION);
        memset(data, 0, RSI_BLE_MAX_DATA_LEN);

        status = sl_wifi_get_firmware_version(&firmware_version);
        if (status == SL_STATUS_OK) {
          data[0] = 0x08;
          data[1] = sizeof(sl_wifi_firmware_version_t);
          memcpy(&data[2], &firmware_version, sizeof(sl_wifi_firmware_version_t));

          rsi_ble_set_local_att_value(rsi_ble_att2_val_hndl, RSI_BLE_MAX_DATA_LEN, data);
          print_firmware_version(&firmware_version);
        }
      } break;

        // Connected SSID name (response to '7' command if connection is already established)
      case WLAN_ALREADY: {
        ble_app_clear_event(WLAN_ALREADY);

        memset(data, 0, RSI_BLE_MAX_DATA_LEN);

        data[1] = connected; /*This index will indicate wlan AP connect or disconnect status to Android app*/
        data[0] = 0x07;
        rsi_ble_set_local_att_value(rsi_ble_att2_val_hndl, RSI_BLE_MAX_DATA_LEN, data);
      } break;

        // NO WLAN connection (response to '7' command if connection is there already)
      case WLAN_NOT_ALREADY: {
        ble_app_clear_event(WLAN_NOT_ALREADY);
        memset(data, 0, RSI_BLE_MAX_DATA_LEN);
        data[0] = 0x07;
        data[1] = 0x00;
        rsi_ble_set_local_att_value(rsi_ble_att2_val_hndl, RSI_BLE_MAX_DATA_LEN, data);
      } break;

      case BLE_WLAN_DISCONN_NOTIFY: {
        ble_app_clear_event(BLE_WLAN_DISCONN_NOTIFY);
        memset(data, 0, RSI_BLE_MAX_DATA_LEN);
        data[1] = 0x01;
        data[0] = 0x04;
        rsi_ble_set_local_att_value(rsi_ble_att2_val_hndl, RSI_BLE_MAX_DATA_LEN, data);
      } break;

      case BLE_WLAN_TIMEOUT_NOTIFY: {
        ble_app_clear_event(BLE_WLAN_TIMEOUT_NOTIFY);
        memset(data, 0, RSI_BLE_MAX_DATA_LEN);
        data[0] = 0x02;
        data[1] = 0x00;
        rsi_ble_set_local_att_value(rsi_ble_att2_val_hndl, RSI_BLE_MAX_DATA_LEN, data);
      } break;

      case BLE_WLAN_DISCONNECT_STATUS: {
        ble_app_clear_event(BLE_WLAN_DISCONNECT_STATUS);
        memset(data, 0, RSI_BLE_MAX_DATA_LEN);
        data[0] = 0x01;
        rsi_ble_set_local_att_value(rsi_ble_att2_val_hndl, RSI_BLE_MAX_DATA_LEN, data);
      } break;

      case RSI_SSID: {
        ble_app_clear_event(RSI_SSID);
      } break;

      case RSI_SECTYPE: {
        ble_app_clear_event(RSI_SECTYPE);
        if (sec_type == 0) {
          wifi_app_set_event(WIFI_APP_JOIN_STATE);
        }
      } break;

        // Scan results from device (response to '3' command)
      case RSI_BLE_WLAN_SCAN_RESP: //Send the SSID data to mobile ble application WYZBEE CONFIGURATOR
      {
        ble_app_clear_event(RSI_BLE_WLAN_SCAN_RESP); // clear the served event

        memset(data, 0, RSI_BLE_MAX_DATA_LEN);
        data[0] = 0x03;
        data[1] = scanresult->scan_count;
        rsi_ble_set_local_att_value(rsi_ble_att2_val_hndl, RSI_BLE_MAX_DATA_LEN, data);

        for (scan_ix = 0; scan_ix < scanresult->scan_count; scan_ix++) {
          memset(data, 0, RSI_BLE_MAX_DATA_LEN);
          data[0] = scanresult->scan_info[scan_ix].security_mode;
          data[1] = ',';
          strcpy((char *)data + 2, (const char *)scanresult->scan_info[scan_ix].ssid);
          length = strlen((char *)data + 2);
          length = length + 2;

          rsi_ble_set_local_att_value(rsi_ble_att3_val_hndl, RSI_BLE_MAX_DATA_LEN, data);
          osDelay(10);
        }

        LOG_PRINT("\r\n[BLE]: Displayed scan list in Silabs app\r\n");
      } break;

      // WLAN connection response status (response to '2' command)
      case BLE_WLAN_JOIN_STATUS: //Send the connected status to mobile ble application WYZBEE CONFIGURATOR
      {
        sl_mac_address_t mac_addr = { 0 };

        sl_ip_address_t ip = { 0 };
        ip.type            = ip_address.type;
        ip.ip.v4.value     = ip_address.ip.v4.ip_address.value;

        // clear the served event
        ble_app_clear_event(BLE_WLAN_JOIN_STATUS);

        memset(data, 0, RSI_BLE_MAX_DATA_LEN);
        data[0] = 0x02;
        data[1] = 0x01;
        data[2] = ',';

        // Copy the MAC address
        status = sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, &mac_addr);
        if (status == SL_STATUS_OK) {
          for (k = 0; k < 6; k++) {
            data[k + 3] = mac_addr.octet[k];
          }
        } else {
          k = 6;
        }
        data[k + 3] = ',';

        // IP Address
        for (int i = 0; k < 10; k++, i++) {
          data[k + 4] = ip.ip.v4.bytes[i];
        }

        rsi_ble_set_local_att_value(rsi_ble_att2_val_hndl,
                                    RSI_BLE_MAX_DATA_LEN,
                                    data); // set the local attribute value.
        LOG_PRINT("\r\n[BLE]: STA joined successfully\r\n");
        status = rsi_ble_conn_params_update(conn_event_to_app.dev_addr,
                                            CONN_INTERVAL_MIN,
                                            CONN_INTERVAL_MAX,
                                            CONN_LATENCY,
                                            SUPERVISION_TIMEOUT);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n[BLE]: conn params update cmd failed with status "
                    "= %lx \r\n",
                    status);
        }
        //   conn_params_updated = 1;
        //  ble_app_clear_event(RSI_APP_BLE_GATT_SERVICE_RESP_DESCRIPTOR);
      } break;
      case BLE_MTU_EVENT: {
        //! clear the served event
        ble_app_clear_event(BLE_MTU_EVENT);
        //! event invokes when write/notification events received
      } break;
      case BLE_CONN_UPDATE_EVENT: {
        ble_app_clear_event(BLE_CONN_UPDATE_EVENT);

      } break;
      case BLE_RECEIVE_REMOTE_FEATURES: {
        //! clear the served event
        ble_app_clear_event(BLE_RECEIVE_REMOTE_FEATURES);

        if (remote_dev_feature.remote_features[0] & 0x20) {
          status = rsi_ble_set_data_len(conn_event_to_app.dev_addr, TX_LEN, TX_TIME);
          if (status != SL_STATUS_OK) {
            LOG_PRINT("\r\n[BLE]: set data length cmd failed with error code = "
                      "%lx\r\n",
                      status);
            ble_app_set_event(BLE_RECEIVE_REMOTE_FEATURES);
          }
        }

      } break;
      case BLE_DATA_LENGTH_CHANGE: {
        //! clear the served event
        ble_app_clear_event(BLE_DATA_LENGTH_CHANGE);
      } break;
      case START_BLE_ADVERTISING: {
        ble_app_clear_event(START_BLE_ADVERTISING);
adv1:
        status = rsi_ble_start_advertising();
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n[BLE]: start advertising cmd failed with error code = %lx\r\n", status);
          goto adv1;
        } else {
          LOG_PRINT("\r\n[BLE]: Started Advertising\r\n");
        }
      } break;
      case STOP_BLE_ADVERTISING: {
        ble_app_clear_event(STOP_BLE_ADVERTISING);
        status = rsi_ble_stop_advertising();
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n[BLE]: stop advertising cmd failed with error code = %lx\r\n", status);
        } else {
          LOG_PRINT("\r\n[BLE]: Advertising Stopped\r\n");
        }
      } break;
      case INITIATE_BLE_DISCONNECT: {
        ble_app_clear_event(INITIATE_BLE_DISCONNECT);
        uint8_t rsi_connected_dev_addr_l[RSI_DEV_ADDR_LEN] = { 0 };
        rsi_ascii_dev_address_to_6bytes_rev(rsi_connected_dev_addr_l, (int8_t *)remote_dev_addr);
        status = rsi_ble_disconnect((int8_t *)rsi_connected_dev_addr_l);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n[BLE]: rsi_ble_disconnect : error status 0x%lx\r\n", status);
        } else {
          Is_ble_disconnected_by_wifi = 1;
        }
      } break;
      default:
        break;
    }
  }

  free(scanresult);
}

/*==============================================*/
/**
 * @fn         wifi_app_send_to_ble
 * @brief      this function is used to send data to ble app.
 * @param[in]   msg_type, it indicates write/notification event id.
 * @param[in]  data, raw data pointer.
 * @param[in]  data_len, raw data length.
 * @return     none.
 * @section description
 */
void wifi_app_send_to_ble(uint16_t msg_type, uint8_t *data, uint16_t data_len)
{
  switch (msg_type) {
    case WIFI_APP_SCAN_RESP:
      memset(scanresult, 0, data_len);
      memcpy(scanresult, (sl_wifi_scan_result_t *)data, data_len);

      ble_app_set_event(RSI_BLE_WLAN_SCAN_RESP);
      break;
    case WIFI_APP_CONNECTION_STATUS:
      ble_app_set_event(BLE_WLAN_JOIN_STATUS);
      break;
    case WIFI_APP_DISCONNECTION_STATUS:
      ble_app_set_event(BLE_WLAN_DISCONNECT_STATUS);
      break;
    case WIFI_APP_DISCONNECTION_NOTIFY:
      ble_app_set_event(BLE_WLAN_DISCONN_NOTIFY);
      break;
    case WIFI_APP_TIMEOUT_NOTIFY:
      ble_app_set_event(BLE_WLAN_TIMEOUT_NOTIFY);
      break;
    case WIFI_APP_REQ_BLE_DISCONNECT:
      ble_app_set_event(INITIATE_BLE_DISCONNECT);
      break;
    case WIFI_APP_REQ_BLE_START_ADVERTISING:
      ble_app_set_event(START_BLE_ADVERTISING);
      break;
    case WIFI_APP_REQ_BLE_STOP_ADVERTISING:
      ble_app_set_event(STOP_BLE_ADVERTISING);
      break;
    default:
      break;
  }
}
