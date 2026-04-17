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
 * @brief : This file contains example application for BLE Long Read
 * @section Description :
 * This application demonstrates how a GATT client device accesses a GATT server
 * device for long read, means when user wants to read more than MTU (minimum of
 * local and remote devices MTU's) size of data.
 * Silabs module acts as a GATT client/server (based on user configuration) and
 * explains reads/writes.
 =================================================================================*/

/******************************************************
*                    Include files
******************************************************/

//! SL Wi-Fi SDK includes
#include "sl_board_configuration.h"
#include "sl_constants.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "cmsis_os2.h"
#include "sl_utility.h"
#include "FreeRTOSConfig.h"
//! BLE include file to refer BLE APIs
#include "rsi_ble_apis.h"
#include "ble_config.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"
#include "rsi_bt_common.h"

//! Common include file
#include "rsi_common_apis.h"
#include <stdio.h>
#include <string.h>
#if SL_SI91X_TICKLESS_MODE == 0 && defined(SLI_SI91X_MCU_INTERFACE)
#include "sl_si91x_m4_ps.h"
#include "sl_si91x_power_manager.h"
#endif
/******************************************************
*                    Constants
******************************************************/

#define BLE_ATT_REC_SIZE 500
#define NO_OF_VAL_ATT    5

#define SERVER 0
#define CLIENT 1

#define GATT_ROLE SERVER

//! BLE attribute service types uuid values
#define RSI_BLE_CHAR_SERV_UUID   0x2803
#define RSI_BLE_CLIENT_CHAR_UUID 0x2902

//! BLE characteristic service uuid
#define RSI_BLE_NEW_SERVICE_UUID 0xAABB
#define RSI_BLE_ATTRIBUTE_1_UUID 0x1AA1
//! local device name

#define RSI_BLE_APP_GATT_TEST "LONG_READ_TEST"

//! MTU size for the link
#define RSI_BLE_MTU_SIZE 100

//! immediate alert service uuid
#define RSI_BLE_NEW_CLIENT_SERVICE_UUID 0x180F
//! Alert level characteristic
#define RSI_BLE_CLIENT_ATTRIBUTE_1_UUID 0x2A19

#define RSI_BLE_MAX_DATA_LEN 20

#define LOCAL_DEV_ADDR_LEN 18 // Length of the local device address

//! attribute properties
#define RSI_BLE_ATT_PROPERTY_READ   0x02
#define RSI_BLE_ATT_PROPERTY_WRITE  0x08
#define RSI_BLE_ATT_PROPERTY_NOTIFY 0x10

//! Configuration bitmap for attributes
#define RSI_BLE_ATT_MAINTAIN_IN_HOST BIT(0)
#define RSI_BLE_ATT_SECURITY_ENABLE  BIT(1)

#define RSI_BLE_ATT_CONFIG_BITMAP (RSI_BLE_ATT_MAINTAIN_IN_HOST)

//! application event list
#define RSI_APP_EVENT_ADV_REPORT              0x00
#define RSI_BLE_CONN_EVENT                    0x01
#define RSI_BLE_DISCONN_EVENT                 0x02
#define RSI_BLE_GATT_WRITE_EVENT              0x03
#define RSI_BLE_READ_REQ_EVENT                0x04
#define RSI_BLE_MTU_EVENT                     0x05
#define RSI_BLE_GATT_PROFILE_RESP_EVENT       0x06
#define RSI_BLE_GATT_CHAR_SERVICES_RESP_EVENT 0x07
#define RSI_BLE_GATT_ERROR                    0x08

//! Address type of the device to connect
#define RSI_BLE_DEV_ADDR_TYPE LE_RANDOM_ADDRESS

//! Address of the device to connect
#define RSI_BLE_DEV_ADDR "00:23:A7:80:70:B9"

//! Remote Device Name to connect
#define RSI_REMOTE_DEVICE_NAME "SILABS_DEV"
/*=======================================================================*/
//!    Powersave configurations
/*=======================================================================*/
#define ENABLE_NWP_POWER_SAVE 0 //! Set to 1 for powersave mode

#if ENABLE_NWP_POWER_SAVE
//! Power Save Profile Mode
#define PSP_MODE RSI_SLEEP_MODE_2
//! Power Save Profile type
#define PSP_TYPE RSI_MAX_PSP

sl_wifi_performance_profile_v2_t wifi_profile = { .profile = ASSOCIATED_POWER_SAVE };
#endif
rsi_ble_event_profile_by_uuid_t profiles_list;
static rsi_ble_event_read_by_type1_t char_servs;

/******************************************************
 *               GLOBAL Variable Definitions
 ******************************************************/
static const sl_wifi_device_configuration_t config = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_BLE_MODE,

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
                   .bt_feature_bit_map = (SL_SI91X_BT_RF_TYPE | SL_SI91X_ENABLE_BLE_PROTOCOL),
#ifdef RSI_PROCESS_MAX_RX_DATA
                   .ext_tcp_ip_feature_bit_map = (RSI_EXT_TCPIP_FEATURE_BITMAP | SL_SI91X_CONFIG_FEAT_EXTENTION_VALID
                                                  | SL_SI91X_EXT_TCP_MAX_RECV_LENGTH),
#else
                   .ext_tcp_ip_feature_bit_map = (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
#endif
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

typedef struct rsi_ble_att_list_s {
  uuid_t char_uuid;
  uint16_t handle;
  uint16_t len;
  uint16_t max_value_len;
  uint8_t char_val_prop;
  void *value;
} rsi_ble_att_list_t;

typedef struct rsi_ble_s {
  uint8_t DATA[BLE_ATT_REC_SIZE];
  uint16_t DATA_ix;
  uint16_t att_rec_list_count;
  rsi_ble_att_list_t att_rec_list[NO_OF_VAL_ATT];
} rsi_ble_t;

rsi_ble_t att_list;

uint16_t mtu_size;

//! global parameters list
static volatile uint32_t ble_app_event_map;
static uint8_t remote_addr_type = 0;
static uint8_t remote_name[31];
static uint8_t remote_dev_addr[18]   = { 0 };
static uint8_t remote_dev_bd_addr[6] = { 0 };
static uint8_t device_found          = 0;
static rsi_ble_event_conn_status_t conn_event_to_app;
static rsi_ble_event_disconnect_t disconn_event_to_app;
static rsi_ble_event_write_t app_ble_write_event;
static uint16_t rsi_ble_att1_val_hndl;
static rsi_ble_read_req_t app_ble_read_event;
static rsi_ble_event_mtu_t app_ble_mtu_event;
static rsi_ble_event_profile_by_uuid_t rsi_ble_service;
static rsi_ble_event_read_by_type1_t char_servs;
uint8_t str_remote_address[18] = { '\0' };
osSemaphoreId_t ble_main_task_sem;

/******************************************************
 *               Function Definitions
 ******************************************************/

#if 0 // This function is not used, To avoid compilation warning this funtion is added in #if 0
/*==============================================*/  
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
#endif
void print_data_pkt(unsigned char *input_buf, uint16_t buf_len)
{
  uint16_t ix;

  LOG_PRINT("buf_len: %d\r\n", buf_len);
  for (ix = 0; ix < buf_len; ix++) {
    if (ix % 16 == 0) {
      LOG_PRINT("\r\n");
    } else if (ix % 8 == 0) {
      LOG_PRINT("\t\t");
    }
    LOG_PRINT(" 0x%02X ", input_buf[ix]);
  }
  LOG_PRINT("\r\n");

  return;
}

/*==============================================*/
/**
 * @fn         rsi_gatt_add_attribute_to_list
 * @brief      This function is used to store characteristic service attribute.
 * @param[in]  p_val, pointer to homekit structure
 * @param[in]  handle, characteristic service attribute handle.
 * @param[in]  data_len, characteristic value length
 * @param[in]  data, characteristic value pointer
 * @param[in]  uuid, characteristic value uuid
 * @return     none.
 * @section description
 * This function is used to store all attribute records  
 */
void rsi_gatt_add_attribute_to_list(rsi_ble_t *p_val,
                                    uint16_t handle,
                                    uint16_t data_len,
                                    uint8_t *data,
                                    uuid_t uuid,
                                    uint8_t char_prop)
{
  if ((p_val->DATA_ix + data_len) >= BLE_ATT_REC_SIZE) {
    LOG_PRINT("no data memory for att rec values");
    return;
  }

  p_val->att_rec_list[p_val->att_rec_list_count].char_uuid     = uuid;
  p_val->att_rec_list[p_val->att_rec_list_count].handle        = handle;
  p_val->att_rec_list[p_val->att_rec_list_count].len           = data_len;
  p_val->att_rec_list[p_val->att_rec_list_count].max_value_len = data_len;
  p_val->att_rec_list[p_val->att_rec_list_count].char_val_prop = char_prop;
  memcpy(p_val->DATA + p_val->DATA_ix, data, data_len);
  p_val->att_rec_list[p_val->att_rec_list_count].value = p_val->DATA + p_val->DATA_ix;
  p_val->att_rec_list_count++;
  p_val->DATA_ix += p_val->att_rec_list[p_val->att_rec_list_count].max_value_len;

  return;
}
/*==============================================*/
/**
 * @fn         rsi_gatt_get_attribute_from_list
 * @brief      This function is used to retrieve attribute from list based on handle.
 * @param[in]  p_val, pointer to characteristic structure
 * @param[in]  handle, characteristic service attribute handle.
 * @return     pointer to the attribute
 * @section description
 * This function is used to store all attribute records
 */
rsi_ble_att_list_t *rsi_gatt_get_attribute_from_list(rsi_ble_t *p_val, uint16_t handle)
{
  uint16_t i;
  for (i = 0; i < p_val->att_rec_list_count; i++) {
    if (p_val->att_rec_list[i].handle == handle) {
      return &(p_val->att_rec_list[i]);
    }
  }
  return 0;
}
#if (GATT_ROLE == SERVER)

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
                                     uint8_t data_len,
                                     uint8_t config_bitmap)
{
  rsi_ble_req_add_att_t new_att = { 0 };

  //! preparing the attributes
  new_att.serv_handler  = serv_handler;
  new_att.handle        = handle;
  new_att.config_bitmap = config_bitmap;
  memcpy(&new_att.att_uuid, &att_type_uuid, sizeof(uuid_t));
  new_att.property = val_prop;

  //! preparing the attribute value
  if (data != NULL)
    memcpy(new_att.data, data, data_len);

  new_att.data_len = data_len;
  //! add attribute to the service
  rsi_ble_add_attribute(&new_att);

  if (((config_bitmap & RSI_BLE_ATT_MAINTAIN_IN_HOST) == 1) || (data_len > 20)) {
    if (data != NULL)
      rsi_gatt_add_attribute_to_list(&att_list, handle, data_len, data, att_type_uuid, val_prop);
  }

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
  uint8_t data1[100]                    = { 1, 0 };

  //! adding new service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_NEW_SERVICE_UUID;
  rsi_ble_add_service(new_uuid, &new_serv_resp);

  //! adding characteristic service attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_ATTRIBUTE_1_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 1,
                            RSI_BLE_ATT_PROPERTY_WRITE | RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY,
                            new_serv_resp.start_handle + 2,
                            new_uuid);

  //! adding characteristic value attribute to the service
  rsi_ble_att1_val_hndl = new_serv_resp.start_handle + 2;
  new_uuid.size         = 2;
  new_uuid.val.val16    = RSI_BLE_ATTRIBUTE_1_UUID;
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 2,
                           new_uuid,
                           RSI_BLE_ATT_PROPERTY_WRITE | RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY,
                           data1,
                           sizeof(data1),
                           RSI_BLE_ATT_CONFIG_BITMAP);

  return 0;
}
#endif
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
  ble_app_event_map = 0;
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
  if (event_num < 32) {
    ble_app_event_map |= BIT(event_num);
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

  for (ix = 0; ix < 32; ix++) {
    if (ble_app_event_map & (1 << ix)) {
      return ix;
    }
  }

  return (-1);
}
/*==============================================*/
/**
 * @fn         rsi_ble_simple_central_on_adv_report_event
 * @brief      invoked when advertise report event is received
 * @param[in]  adv_report, pointer to the received advertising report
 * @return     none.
 * @section description
 * This callback function updates the scanned remote devices list
 */
void rsi_ble_simple_central_on_adv_report_event(rsi_ble_event_adv_report_t *adv_report)
{

  if (device_found == 1) {
    return;
  }

  memset(remote_name, 0, 31);
  BT_LE_ADPacketExtract(remote_name, adv_report->adv_data, adv_report->adv_data_len);

  remote_addr_type = adv_report->dev_addr_type;
  rsi_6byte_dev_address_to_ascii(remote_dev_addr, (uint8_t *)adv_report->dev_addr);
  memcpy(remote_dev_bd_addr, (uint8_t *)adv_report->dev_addr, 6);
  if (((device_found == 0) && ((strcmp((const char *)remote_name, (const char *)RSI_REMOTE_DEVICE_NAME)) == 0))
      || (((remote_addr_type == RSI_BLE_DEV_ADDR_TYPE)
           && ((strcmp((const char *)remote_dev_addr, (const char *)RSI_BLE_DEV_ADDR) == 0))))) {
    device_found = 1;
    rsi_ble_app_set_event(RSI_APP_EVENT_ADV_REPORT);
  }
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
  memcpy(&disconn_event_to_app, resp_disconnect, sizeof(rsi_ble_event_disconnect_t));
  rsi_ble_app_set_event(RSI_BLE_DISCONN_EVENT);
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
  rsi_ble_app_set_event(RSI_BLE_CONN_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_profile
 * @brief      invoked when the specific service details are received for
 *             supported specific services.
 * @param[out] p_ble_resp_profile, specific profile details
 * @return     none
 * @section description
 * This is a callback function
 */
static void rsi_ble_profile(uint16_t resp_status, rsi_ble_event_profile_by_uuid_t *rsi_ble_event_profile)
{
  UNUSED_PARAMETER(resp_status); //This statement is added only to resolve compilation warning, value is unchanged
  memcpy(&rsi_ble_service, rsi_ble_event_profile, sizeof(rsi_ble_event_profile_by_uuid_t));
  rsi_ble_app_set_event(RSI_BLE_GATT_PROFILE_RESP_EVENT);
  return;
}
/*==============================================*/
/**
 * @fn         rsi_ble_char_services
 * @brief      invoked when response is received for characteristic services details
 * @param[out] p_ble_resp_char_services, list of characteristic services.
 * @return     none
 * @section description   
 */
static void rsi_ble_char_services(uint16_t resp_status, rsi_ble_event_read_by_type1_t *rsi_ble_event_read_type1)
{
  //This statement is added only to resolve compilation warning, value is unchanged
  UNUSED_PARAMETER(resp_status); //This statement is added only to resolve compilation warning, value is unchanged
  memcpy(&char_servs, rsi_ble_event_read_type1, sizeof(rsi_ble_event_read_by_type1_t));
  rsi_ble_app_set_event(RSI_BLE_GATT_CHAR_SERVICES_RESP_EVENT);
  return;
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
  UNUSED_PARAMETER(event_id); //This statement is added only to resolve compilation warning, value is unchanged
  memcpy(&app_ble_write_event, rsi_ble_write, sizeof(rsi_ble_event_write_t));
  rsi_ble_app_set_event(RSI_BLE_GATT_WRITE_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_read_req_event
 * @brief      its invoked when write/notify/indication events are received.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write/notify/indication events are received
 */
static void rsi_ble_on_read_req_event(uint16_t event_id, rsi_ble_read_req_t *rsi_ble_read_req)
{
  UNUSED_PARAMETER(event_id); //This statement is added only to resolve compilation warning, value is unchanged
  memcpy(&app_ble_read_event, rsi_ble_read_req, sizeof(rsi_ble_read_req_t));
  rsi_ble_app_set_event(RSI_BLE_READ_REQ_EVENT);
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
  mtu_size = (uint16_t)app_ble_mtu_event.mtu_size;
  rsi_ble_app_set_event(RSI_BLE_MTU_EVENT);
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
 * @fn         rsi_ble_simple_gatt_test
 * @brief      this is the application of ble GATT client api's test cases.
 * @param[in]  none.
 * @return     none.
 * @section description
 * This function is used at application.
 */
void rsi_ble_simple_gatt_test(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status                                         = 0;
  sl_wifi_firmware_version_t fw_version                      = { 0 };
  static uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN] = { 0 };
  uint8_t local_dev_addr[LOCAL_DEV_ADDR_LEN]                 = { 0 };

#if (GATT_ROLE == SERVER)
  uint8_t adv[31] = { 2, 1, 6 };
#endif
  int32_t event_id;
  int8_t data[20] = { 0 };
#if (GATT_ROLE == CLIENT)
  int8_t client_data[100] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0,
                              1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 };
  uint8_t ix;
  uuid_t service_uuid;
  // uint32_t last_time = 0;
#endif
  // rsi_ble_resp_local_att_value_t local_att_val;

  uint8_t read_data[100] = { 2 };

  status = sl_wifi_init(&config, NULL, sl_wifi_default_event_handler);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nWi-Fi Initialization Failed, Error Code : 0x%lX\r\n", status);
    return;
  }

  printf("\r\nWi-Fi  initialization is successful\r\n");
  status = sl_wifi_get_firmware_version(&fw_version);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nFirmware version Failed, Error Code : 0x%lX\r\n", status);
  } else {
    print_firmware_version(&fw_version);
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

#if (GATT_ROLE == SERVER)
  rsi_ble_add_simple_chat_serv();
#endif

  //! registering the GAP call back functions
  rsi_ble_gap_register_callbacks(rsi_ble_simple_central_on_adv_report_event,
                                 rsi_ble_on_connect_event,
                                 rsi_ble_on_disconnect_event,
                                 NULL,
                                 NULL,
                                 NULL,
                                 rsi_ble_on_enhance_conn_status_event,
                                 NULL,
                                 NULL,
                                 NULL);

  //! registering the GATT call back functions
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
                                  NULL,
                                  NULL,
                                  rsi_ble_profile,
                                  rsi_ble_char_services,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL);

  //! create ble main task if ble protocol is selected
  ble_main_task_sem = osSemaphoreNew(1, 0, NULL);
  if (ble_main_task_sem == NULL) {
    LOG_PRINT("\r\nFailed to create ble_main_task_sem\r\n");
    return;
  }
  //!  initializing the application events map
  rsi_ble_app_init_events();

  //! Set local name
  rsi_bt_set_local_name((uint8_t *)RSI_BLE_APP_GATT_TEST);

#if (GATT_ROLE == SERVER)
  //! prepare advertise data //local/device name
  adv[3] = strlen(RSI_BLE_APP_GATT_TEST) + 1;
  adv[4] = 9;
  strcpy((char *)&adv[5], RSI_BLE_APP_GATT_TEST);

  //! set advertise data
  rsi_ble_set_advertise_data(adv, strlen(RSI_BLE_APP_GATT_TEST) + 5);

  //! set device in advertising mode.
  status = rsi_ble_start_advertising();
  if (status != RSI_SUCCESS) {
    return;
  }
  LOG_PRINT("\r\nStart advertising\r\n");
#endif

#if (GATT_ROLE == CLIENT)
  //! start scanning
  status = rsi_ble_start_scanning();
  LOG_PRINT("start scanning\n");
  if (status != RSI_SUCCESS) {
    LOG_PRINT("start_scanning status: 0x%lX\r\n", status);
    return;
  }
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
    //! checking for events list
    event_id = rsi_ble_app_get_event();
    if (event_id == -1) {
#if ((SL_SI91X_TICKLESS_MODE == 0) && SLI_SI91X_MCU_INTERFACE && ENABLE_NWP_POWER_SAVE)
      //! if events are not received loop will be continued.
      if ((!(P2P_STATUS_REG & TA_wakeup_M4))) {
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
        rsi_ble_app_clear_event(RSI_APP_EVENT_ADV_REPORT);
        LOG_PRINT("\r\nIn Advertising Event\r\n");
        status = rsi_ble_connect(remote_addr_type, (int8_t *)remote_dev_bd_addr);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("connect status: 0x%lX\r\n", status);
        }

      } break;

#endif
      case RSI_BLE_CONN_EVENT: {
        //! event invokes when connection was completed

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_CONN_EVENT);
        rsi_6byte_dev_address_to_ascii(str_remote_address, conn_event_to_app.dev_addr);
        LOG_PRINT("\r\n Module connected to address : %s \r\n", str_remote_address);

#if (GATT_ROLE == CLIENT)
        memset(&service_uuid, 0, sizeof(service_uuid));
        service_uuid.size      = 2;
        service_uuid.val.val16 = RSI_BLE_NEW_CLIENT_SERVICE_UUID;

        status = rsi_ble_get_profile_async(conn_event_to_app.dev_addr, service_uuid, NULL);
        if (status != 0) {
          LOG_PRINT("\n rsi_ble_get_profile_async failed with %lx", status);
        }
#endif
#if (GATT_ROLE == SERVER)
        status = rsi_ble_mtu_exchange_event((uint8_t *)conn_event_to_app.dev_addr, RSI_BLE_MTU_SIZE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("MTU CMD status: 0x%lX\r\n", status);
        }
#endif

      } break;

      case RSI_BLE_DISCONN_EVENT: {
        //! event invokes when disconnection was completed

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_DISCONN_EVENT);
        LOG_PRINT("\r\nModule got Disconnected\r\n");
#if ENABLE_NWP_POWER_SAVE
        LOG_PRINT("\r\n keep module in to active state \r\n");
        //! initiating Active mode in BT mode
        status = rsi_bt_power_save_profile(RSI_ACTIVE, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\n Failed to keep Module in ACTIVE mode \r\n");
          return;
        }

        //! initiating power save in wlan mode
        wifi_profile.profile = HIGH_PERFORMANCE;
        status               = sl_wifi_set_performance_profile_v2(&wifi_profile);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n Failed to keep module in HIGH_PERFORMANCE mode \r\n");
          return;
        }
#endif
#if (GATT_ROLE == SERVER)
adv:
        //! set device in advertising mode.
        status = rsi_ble_start_advertising();
        if (status != RSI_SUCCESS) {
          goto adv;
        }
#endif
#if (GATT_ROLE == CLIENT)
        //! start scanning
        device_found = 0;
        LOG_PRINT("\r\nStart scanning \n");
        status = rsi_ble_start_scanning();
        if (status != RSI_SUCCESS) {
          LOG_PRINT("start_scanning status: 0x%lX\r\n", status);
        }
#endif
#if ENABLE_NWP_POWER_SAVE
        LOG_PRINT("\r\n keep module in to power save \r\n");
        status = rsi_bt_power_save_profile(PSP_MODE, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          return;
        }

        //! initiating power save in wlan mode
        wifi_profile.profile = ASSOCIATED_POWER_SAVE;
        status               = sl_wifi_set_performance_profile_v2(&wifi_profile);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n Failed to keep module in power save \r\n");
          return;
        }
        LOG_PRINT("\r\n Module is in power save \r\n");
#endif
      } break;
      case RSI_BLE_GATT_WRITE_EVENT: {
        LOG_PRINT("\r\nIn BLE GATT write event\r\n");
        //! event invokes when write/notification events received

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_GATT_WRITE_EVENT);

        //TO DO: send ERR or write response
        if ((*(uint16_t *)app_ble_write_event.handle) == rsi_ble_att1_val_hndl) {
          rsi_ble_att_list_t *attribute = NULL;
          uint8_t opcode = 0x12, err = 0x00;
          attribute = rsi_gatt_get_attribute_from_list(&att_list, (*(uint16_t *)(app_ble_write_event.handle)));

          //! Check if value has write properties
          if ((attribute != NULL) && (attribute->value != NULL)) {
            if (!(attribute->char_val_prop & 0x08)) //! If no write property, send error response
            {
              err = 0x03; //! Error - Write not permitted
            }
          } else {
            //!Error = No such handle exists
            err = 0x01;
          }

          //! Update the value based6 on the offset and length of the value
          if ((err == 0) && ((app_ble_write_event.length) <= attribute->max_value_len)) {
            memset(attribute->value, 0, attribute->max_value_len);

            //! Check if value exists for the handle. If so, maximum length of the value.
            memcpy(attribute->value, app_ble_write_event.att_value, app_ble_write_event.length);

            //! Update value length
            attribute->len = app_ble_write_event.length;

            LOG_PRINT("\r\n received data from remote device: %s \n", (char *)attribute->value);

            //! Send gatt write response
            rsi_ble_gatt_write_response(conn_event_to_app.dev_addr, 0);
          } else {
            //! Error : 0x07 - Invalid request,  0x0D - Invalid attribute value length
            err = 0x07;
          }

          if (err) {
            //! Send error response
            rsi_ble_att_error_response(conn_event_to_app.dev_addr,
                                       *(uint16_t *)app_ble_write_event.handle,
                                       opcode,
                                       err);
          }
        }
        //! prepare the data to set as local attribute value.
        memset(data, 0, sizeof(data));
        memcpy(data, app_ble_write_event.att_value, app_ble_write_event.length);

        //! set the local attribute value.
        rsi_ble_notify_value(conn_event_to_app.dev_addr, rsi_ble_att1_val_hndl, RSI_BLE_MAX_DATA_LEN, (uint8_t *)data);
      } break;
      case RSI_BLE_READ_REQ_EVENT: {
        LOG_PRINT("\r\nIn BLE read request event\r\n");
        //! event invokes when write/notification events received

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_READ_REQ_EVENT);

        rsi_ble_gatt_read_response(conn_event_to_app.dev_addr,
                                   0,
                                   app_ble_read_event.handle,
                                   0,
                                   RSI_MIN(mtu_size - 2, sizeof(read_data)),
                                   read_data);
      } break;

      case RSI_BLE_GATT_PROFILE_RESP_EVENT: {
        LOG_PRINT("\r\nIn BLE GATT profile response event\r\n");
        //! event invokes when get profile response received

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_GATT_PROFILE_RESP_EVENT);

#if (GATT_ROLE == CLIENT)
        //! get characteristics of the immediate alert servcie
        //rsi_6byte_dev_address_to_ascii ((int8_t *)remote_dev_addr, (uint8_t *)conn_event_to_app.dev_addr);
        status = rsi_ble_get_char_services_async(conn_event_to_app.dev_addr,
                                                 *(uint16_t *)rsi_ble_service.start_handle,
                                                 *(uint16_t *)rsi_ble_service.end_handle,
                                                 NULL);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("rsi_ble_get_char_services_async status: 0x%lX\r\n", status);
        }
#endif
      } break;

      case RSI_BLE_GATT_CHAR_SERVICES_RESP_EVENT: {
        LOG_PRINT("\r\nIn BLE GATT characteristics of services response event\r\n");
        //! event invokes when get characteristics of the service response received

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_GATT_CHAR_SERVICES_RESP_EVENT);

#if (GATT_ROLE == CLIENT)
        //! verifying the immediate alert characteristic
        for (ix = 0; ix < char_servs.num_of_services; ix++) {
          LOG_PRINT("Character services of  profile : ");
          LOG_PRINT(" uuid: 0x%04x\r\n", char_servs.char_services[ix].char_data.char_uuid.val.val16);
          if (char_servs.char_services[ix].char_data.char_uuid.val.val16 == RSI_BLE_CLIENT_ATTRIBUTE_1_UUID) {
            rsi_ble_att1_val_hndl = char_servs.char_services[ix].char_data.char_handle;

            status = rsi_ble_set_att_cmd_async(conn_event_to_app.dev_addr,
                                               rsi_ble_att1_val_hndl,
                                               RSI_MIN(mtu_size - 3, 100),
                                               (uint8_t *)&client_data);
            if (status != RSI_SUCCESS) {
              LOG_PRINT("rsi_ble_set_att_cmd_async status: 0x%lX\r\n", status);
            }
            //! set the event to calculate RSSI value
            // #ifndef RSI_SAMPLE_HAL
            //             UNUSED_VARIABLE(last_time);
            //             last_time = rsi_hal_gettickcount();
            // #endif
            break;
          }
        }
#endif
      } break;

      case RSI_BLE_MTU_EVENT: {
        LOG_PRINT("\r\nIn MTU event\r\n");
        //! event invokes when write/notification events received

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_MTU_EVENT);

      } break;

      case RSI_BLE_GATT_ERROR: {

        //! clear the served even
        rsi_ble_app_clear_event(RSI_BLE_GATT_ERROR);
      }
      default: {
      }
    }
  }
}

void app_init(void)
{
  osThreadNew((osThreadFunc_t)rsi_ble_simple_gatt_test, NULL, &thread_attributes);
}
