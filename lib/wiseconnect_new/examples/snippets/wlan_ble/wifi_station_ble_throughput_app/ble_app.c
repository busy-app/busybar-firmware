/*******************************************************************************
* @file  ble_app.c
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
 * @brief : This file contains example application for WLAN Station BLE Throughput
 * @section Description :
 * This Application explains user how to Create chat service, Configure device in
 * advertise mode, Connect from Smart phone and Receive data sent by Smart phone
 =================================================================================*/

/**
 * Include files
 * */

#include <stdio.h>
#include <string.h>

#include "ble_config.h"
#include "rsi_ble_common_config.h"

#include <rsi_ble.h>
#include "rsi_ble_apis.h"
#include "rsi_bt_common_apis.h"
#include "sl_constants.h"
#include "rsi_common_apis.h"

//! local device name
#define RSI_BLE_DEVICE_NAME (void *)"BLE_THROUGHPUT_APP"

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

static rsi_ble_event_conn_update_t conn_update_complete;
static rsi_ble_event_write_t app_ble_write_event;
static uint16_t rsi_ble_att1_val_hndl;
static uint16_t rsi_ble_att2_val_hndl;
static uint8_t notifies_enabled;
static uint8_t remote_name[31];
static uint8_t remote_addr_type        = 0;
static uint8_t remote_dev_str_addr[18] = { 0 };
static uint8_t remote_dev_bd_addr[6]   = { 0 };
static uint8_t device_found            = 0;
static uint8_t conn_params_updated     = 0;

extern osSemaphoreId_t ble_main_task_sem, ble_conn_sem;

void rsi_ble_on_enhance_conn_status_event(rsi_ble_event_enhance_conn_status_t *resp_enh_conn);

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

/*==============================================*/
/**
 * @fn         rsi_ble_on_enhance_conn_status_event
 * @brief      invoked when enhanced connection complete event is received
 * @param[in] resp_enh_conn, connected remote device information
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
 * @brief      Invoked when disconnection event is received
 * @param[out] resp_disconnect, disconnected remote device information
 * @param[out] reason, reason for disconnection.
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

/**
 * @fn         rsi_ble_fill_128bit_uuid
 * @brief      This function is used to form 128bit uuid of apple device from 32 bit uuid.
 * @param[in]  uuid_32bit, 32 bit uuid.
 * @param[out] serv_uuid, 128 bit uuid.
 * @return     none.
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

  if (data != NULL)
    memcpy(new_att.data, data, RSI_MIN(sizeof(new_att.data), data_len));

  //! preparing the attribute value
  new_att.data_len = data_len;

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
 * @fn         rsi_ble_on_gatt_write_event
 * @brief      Its invoked when write/notify/indication events are received.
 * @param[out] event_id, it indicates write/notification event id.
 * @param[out] rsi_ble_write, write event parameters.
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
 * @brief      Invoked when advertise report event is received
 * @param[out] adv_report, pointer to the received advertising report
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
 * @fn         rsi_ble_app_init
 * @brief      initialize the BLE module.
 * @param[in]  none
 * @return     none.
 * @section description
 * This function is used to initialize the BLE module
 */
void rsi_ble_app_init(void)
{
  int32_t status  = 0;
  uint8_t adv[31] = { 2, 1, 6 };

  //! registering the GAP callback functions
  rsi_ble_gap_register_callbacks(NULL,
                                 rsi_ble_on_connect_event,
                                 rsi_ble_on_disconnect_event,
                                 NULL,
                                 NULL,
                                 NULL,
                                 rsi_ble_on_enhance_conn_status_event,
                                 NULL,
                                 NULL,
                                 NULL);

  rsi_ble_gap_extended_register_callbacks(NULL, rsi_ble_more_data_req_event);

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
                                  NULL,
                                  NULL);

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

  //! start advertising
  status = rsi_ble_start_advertising();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\nStart advertising cmd failed with error code = %lx \n", status);
  } else {
    LOG_PRINT("\nStarted advertising, local device name: %s\n", (char *)RSI_BLE_DEVICE_NAME);
  }
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_task
 * @brief      this function will execute when BLE events are raised.
 * @param[in]  none.
 * @return     none.
 * @section description
 */
void rsi_ble_app_task(void)
{
  int32_t status = 0;
  int32_t event_id;

  //! waiting for events from controller.
  while (1) {
    //! Application main loop
    //! checking for events list
    event_id = rsi_ble_app_get_event();
    if (event_id == -1) {
      osSemaphoreAcquire(ble_main_task_sem, osWaitForever);

      //! if events are not received loop will be continued.
      continue;
    }
    switch (event_id) {
      case RSI_BLE_CONN_EVENT: {
        //! event invokes when connection was completed
        LOG_PRINT("Connected to remote address : %s\r\n", str_remote_address);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_CONN_EVENT);

        //! Setting MTU Exchange event
        osSemaphoreRelease(ble_conn_sem);
      } break;

      case RSI_BLE_DISCONN_EVENT: {
        //! event invokes when disconnection was completed
        LOG_PRINT("Disconnected from remote address : %s\r\n", str_remote_address);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_DISCONN_EVENT);

        //! clear all pending events
        rsi_ble_app_clear_all_event();

        device_found        = 0;
        conn_params_updated = 0;

        //! start advertising
        status = rsi_ble_start_advertising();
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\nStart advertising cmd failed with error code = %lx \n", status);
        } else {
          LOG_PRINT("\nStarted advertising, local device name: %s\n", (char *)RSI_BLE_DEVICE_NAME);
        }
      } break;

      case RSI_BLE_CONN_UPDATE_EVENT: {
        LOG_PRINT("\nConnection parameters update completed\n ");
        LOG_PRINT("\nConnection interval = %d, Latency = %d, Supervision Timeout = %d \n",
                  conn_update_complete.conn_interval,
                  conn_update_complete.conn_latency,
                  conn_update_complete.timeout);

        rsi_ble_app_clear_event(RSI_BLE_CONN_UPDATE_EVENT);

      } break;
      case RSI_BLE_GATT_WRITE_EVENT: {
        //! event invokes when write event received
        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_GATT_WRITE_EVENT);
        LOG_PRINT("\nReceived packet type = %d", app_ble_write_event.pkt_type);

        if ((*(uint16_t *)app_ble_write_event.handle - 1) == rsi_ble_att2_val_hndl) {
          if (app_ble_write_event.att_value[0] == NOTIFY_ENABLE) {
            LOG_PRINT("\r\nRemote device enabled the notification \n");
            //! set the data transfer event
            notifies_enabled = 0x01;
            rsi_ble_app_set_event(RSI_DATA_TRANSMIT_EVENT);
          } else if (app_ble_write_event.att_value[0] == NOTIFY_DISABLE) {
            LOG_PRINT("\r\nRemote device disabled the notification \n");
            //! clear the data transfer event
            notifies_enabled = 0x00;
            rsi_ble_app_clear_event(RSI_DATA_TRANSMIT_EVENT);
          }
        }
      } break;
      default: {
      }
    }
  }
}