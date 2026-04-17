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
 * @brief : This file contains example application for BLE Heart Rate Profile
 * @section Description :
 * This application demonstrates how to configure Heart rate as GATT server in
 * BLE peripheral mode and explains how to do indicate operation with GATT server
 * from connected remote device using GATT client.
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
#include "rsi_common_app.h"
#include <string.h>
#include <stdio.h>
#include "ble_config.h"
#include <rsi_ble.h>
#include "rsi_ble_apis.h"
#include "rsi_bt_common_apis.h"
#include "rsi_common_apis.h"
#include "ble_event_hdlr_auto_gen.h"
#include "ble_private.h"
#include "ble_cmd_hdlr_auto_gen.h"

void sl_btc_ble_adv_extended_register_callbacks_wrapper();
int32_t rsi_ble_dual_role(void);
rsi_ble_t att_list;
rsi_ble_req_adv_t change_adv_param;
rsi_ble_req_scan_t change_scan_param;

rsi_ble_req_adv_t change_adv_param;
rsi_ble_req_scan_t change_scan_param;
rsi_ble_conn_info_t rsi_ble_conn_info[TOTAL_CONNECTIONS]   = { 0 };
rsi_bt_event_encryption_enabled_t l_rsi_encryption_enabled = { 0 };

uint8_t num_of_connected_peripheral_devices = 0;
uint8_t adv_pkt_processing_pending          = 0;

uint8_t i = 0, temp = 0;
int32_t status          = 0;
uint8_t smp_in_progress = 0;

uint8_t adv[0x19]; //= { 2, 1, 6 };

volatile uint16_t rsi_ble_att1_val_hndl;
volatile uint16_t rsi_ble_att2_val_hndl;
volatile uint16_t rsi_ble_att3_val_hndl;
extern void rsi_ui_app_task(void);
uint32_t rsi_app_resp_max_no_of_supp_adv_sets       = 0;
uint32_t rsi_app_resp_max_adv_data_len              = 0;
int8_t rsi_app_resp_tx_power                        = 0;
uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN] = { 0 };

//! BLE attribute service types uuid values
#define RSI_BLE_CHAR_SERV_UUID   0x2803
#define RSI_BLE_CLIENT_CHAR_UUID 0x2902

osSemaphoreId_t ble_wait_on_connect;
int32_t rsi_ble_dual_role(void);

/*=======================================================================*/
//!    Powersave configurations
/*=======================================================================*/
#define ENABLE_NWP_POWER_SAVE 1 //! Set to 1 for powersave mode

#if ENABLE_NWP_POWER_SAVE
//! Power Save Profile Mode
#define PSP_MODE RSI_SLEEP_MODE_2
//! Power Save Profile type
#define PSP_TYPE RSI_MAX_PSP
sl_wifi_performance_profile_v2_t wifi_profile = { .profile = ASSOCIATED_POWER_SAVE };
#endif

static const sl_wifi_device_configuration_t config = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_BLE_MODE,
#ifdef SLI_SI91X_MCU_INTERFACE
                   .feature_bit_map = (SL_WIFI_FEAT_WPS_DISABLE | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
                                       | SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1),
#else
                   .feature_bit_map = SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE | SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1,
#endif
#if RSI_DISABLE
                   .tcp_ip_feature_bit_map = SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT
#else
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID)
#endif
                                             | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT | SL_SI91X_TCP_IP_FEAT_SSL,

                   .custom_feature_bit_map =
                     (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID | SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map =
                     (SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
#ifdef SLI_SI917
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      | SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE),
#ifdef RSI_PROCESS_MAX_RX_DATA
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID | SL_SI91X_EXT_TCP_MAX_RECV_LENGTH),
#else
                   .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
#endif
                   .bt_feature_bit_map = ((SL_SI91X_BT_RF_TYPE | SL_SI91X_ENABLE_BLE_PROTOCOL)),
                   /*Enable BLE custom feature bitmap*/
                   .ble_feature_bit_map =
                     (SL_SI91X_BLE_MAX_NBR_PERIPHERALS(RSI_BLE_MAX_NBR_PERIPHERALS)
                      | SL_SI91X_BLE_MAX_NBR_CENTRALS(RSI_BLE_MAX_NBR_CENTRALS)
                      | SL_SI91X_BLE_MAX_NBR_ATT_SERV(RSI_BLE_MAX_NBR_ATT_SERV)
                      | SL_SI91X_BLE_MAX_NBR_ATT_REC(RSI_BLE_MAX_NBR_ATT_REC)
                      | SL_SI91X_FEAT_BLE_CUSTOM_FEAT_EXTENTION_VALID | SL_SI91X_BLE_PWR_INX(RSI_BLE_PWR_INX)
                      | SL_SI91X_BLE_PWR_SAVE_OPTIONS(RSI_BLE_PWR_SAVE_OPTIONS)
                      | SL_SI91X_916_BLE_COMPATIBLE_FEAT_ENABLE
#if RSI_BLE_GATT_ASYNC_ENABLE
                      | SL_SI91X_BLE_GATT_ASYNC_ENABLE
#endif
                      ),
                   .ble_ext_feature_bit_map =
                     (SL_SI91X_BLE_NUM_CONN_EVENTS(RSI_BLE_NUM_CONN_EVENTS)
                      | SL_SI91X_BLE_NUM_REC_BYTES(RSI_BLE_NUM_REC_BYTES)
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
#if RSI_BLE_ENABLE_ADV_EXTN
                      | SL_SI91X_BLE_ENABLE_ADV_EXTN
#endif
#if RSI_BLE_AE_MAX_ADV_SETS
                      | SL_SI91X_BLE_AE_MAX_ADV_SETS(RSI_BLE_AE_MAX_ADV_SETS)
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
  .priority   = 0,
  .tz_module  = 0,
  .reserved   = 0,
};

rsi_ble_att_list_t *rsi_gatt_get_attribute_from_list(rsi_ble_t *p_val, uint16_t handle)
{
  uint16_t i;
  for (i = 0; i < p_val->att_rec_list_count; i++) {
    if (p_val->att_rec_list[i].handle == handle) {
      //*val_prop = p_val.att_rec_list[i].char_val_prop;
      //*length = p_val.att_rec_list[i].value_len;
      //*max_data_len = p_val.att_rec_list[i].max_value_len;
      return &(p_val->att_rec_list[i]);
    }
  }
  return 0;
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
  if ((p_val->DATA_ix + data_len) >= BLE_ATT_REC_SIZE) { //! Check for max data length for the characteristic value
    LOG_PRINT("\r\n no data memory for att rec values \r\n");
    return;
  }

  p_val->att_rec_list[p_val->att_rec_list_count].char_uuid     = uuid;
  p_val->att_rec_list[p_val->att_rec_list_count].handle        = handle;
  p_val->att_rec_list[p_val->att_rec_list_count].value_len     = data_len;
  p_val->att_rec_list[p_val->att_rec_list_count].max_value_len = data_len;
  p_val->att_rec_list[p_val->att_rec_list_count].char_val_prop = char_prop;
  memcpy(p_val->DATA + p_val->DATA_ix, data, data_len);
  p_val->att_rec_list[p_val->att_rec_list_count].value = p_val->DATA + p_val->DATA_ix;
  p_val->att_rec_list_count++;
  p_val->DATA_ix += p_val->att_rec_list[p_val->att_rec_list_count].max_value_len;

  return;
}

/*=============================================================================*/
/**
 * @fn         uint8_t ascii_to_dec(uint8_t *num)
 * @brief      Convert ASCII to Decimal Value.  
 * @param[in]  num - ASCII input
 * @return     dec_val - value after conversion 
 */
uint8_t ascii_to_dec(uint8_t *num)
{
  uint8_t dec_val = 0, i = 0;
  while (num[i] != '\0') {
    dec_val = dec_val * 10 + (num[i] - '0');
    i++;
  }
  return dec_val;
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
                                      uuid_t att_val_uuid,
                                      uint16_t auth_read)
{
  rsi_ble_req_add_att_t new_att = { 0 };

  //! preparing the attribute service structure
  new_att.serv_handler       = serv_handler;
  new_att.handle             = handle;
  new_att.att_uuid.size      = 2;
  new_att.att_uuid.val.val16 = RSI_BLE_CHAR_SERV_UUID;
  new_att.property           = RSI_BLE_ATT_PROPERTY_READ;
  new_att.config_bitmap      = auth_read;

  //! preparing the characteristic attribute value
  new_att.data_len = att_val_uuid.size + 4;
  new_att.data[0]  = val_prop;
  rsi_uint16_to_2bytes(&new_att.data[2], att_val_handle);
  if (new_att.data_len == 6) {
    rsi_uint16_to_2bytes(&new_att.data[4], att_val_uuid.val.val16);
  } else if (new_att.data_len == 8) {
    rsi_uint32_to_4bytes(&new_att.data[4], att_val_uuid.val.val32);
  } else if (new_att.data_len == 20) {
    memcpy(&new_att.data[4], &att_val_uuid.val.val128, att_val_uuid.size);
  }
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

  if ((auth_read == ATT_REC_MAINTAIN_IN_HOST) || (data_len > 20)) {
    if (data != NULL) {
      rsi_gatt_add_attribute_to_list(&att_list, handle, data_len, data, att_type_uuid, val_prop);
    }
  }

  //! check the attribute property with notification/Indication
  if ((val_prop & RSI_BLE_ATT_PROPERTY_NOTIFY) || (val_prop & RSI_BLE_ATT_PROPERTY_INDICATE)) {
    //! if notification/indication property supports then we need to add client characteristic service.

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
 * @fn         rsi_ble_add_custom_service_serv
 * @brief      this function is used to add new servcie i.e.., custom service
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to create new service.
 */

static uint32_t rsi_ble_add_custom_service_serv(void)
{
  uuid_t new_uuid                       = { 0 };
  rsi_ble_resp_add_serv_t new_serv_resp = { 0 };
  uint8_t data[1]                       = { 90 };
  rsi_ble_pesentation_format_t presentation_format;
  uint8_t format_data[7];

  //! adding descriptor fileds
  format_data[0] = presentation_format.format = RSI_BLE_UINT8_FORMAT;
  format_data[1] = presentation_format.exponent = RSI_BLE_EXPONENT;
  presentation_format.unit                      = RSI_BLE_PERCENTAGE_UNITS_UUID;
  memcpy(&format_data[2], &presentation_format.unit, sizeof(presentation_format.unit));
  format_data[4] = presentation_format.name_space = RSI_BLE_NAME_SPACE;
  presentation_format.description                 = RSI_BLE_DESCRIPTION;
  memcpy(&format_data[5], &presentation_format.description, sizeof(presentation_format.description));

  //! adding new service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_CUSTOM_SERVICE_UUID;
  rsi_ble_add_service(new_uuid, &new_serv_resp);

  //! adding characteristic service attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_CUSTOM_LEVEL_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 1,
                            RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_INDICATE,
                            new_serv_resp.start_handle + 2,
                            new_uuid,
                            SEC_MODE_1_LEVEL_1);
  //! adding characteristic value attribute to the service
  rsi_ble_att3_val_hndl = new_serv_resp.start_handle + 2;
  new_uuid.size         = 2;
  new_uuid.val.val16    = RSI_BLE_CUSTOM_LEVEL_UUID;
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 2,
                           new_uuid,
                           RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_INDICATE,
                           data,
                           sizeof(data),
                           1);

  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_CHAR_PRESENTATION_FORMATE_UUID;
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 4,
                           new_uuid,
                           RSI_BLE_ATT_PROPERTY_READ,
                           format_data,
                           sizeof(format_data),
                           1);

  return 0;
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
  uint8_t data[230]                     = { 1, 0 };

  //! adding new service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_NEW_SERVICE_UUID;
  rsi_ble_add_service(new_uuid, &new_serv_resp);

  //! adding characteristic service attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_ATTRIBUTE_1_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 1,
                            RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY | RSI_BLE_ATT_PROPERTY_WRITE,
                            new_serv_resp.start_handle + 2,
                            new_uuid,
                            0);

  //! adding characteristic value attribute to the service
  rsi_ble_att1_val_hndl = new_serv_resp.start_handle + 2;
  new_uuid.size         = 2;
  new_uuid.val.val16    = RSI_BLE_ATTRIBUTE_1_UUID;
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 2,
                           new_uuid,
                           RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY | RSI_BLE_ATT_PROPERTY_WRITE,
                           data,
                           sizeof(data),
                           (1));

  return 0;
}

/*==============================================*/
/**
 * @fn         rsi_ble_add_simple_chat_serv2
 * @brief      this function is used to add new servcie i.e.., simple chat service.
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to create new service.


 */

static uint32_t rsi_ble_add_simple_chat_serv2(void)
{
  //! adding the custom service
  // 0x6A4E3300-667B-11E3-949A-0800200C9A66
  uint8_t data1[231]                 = { 1, 0 };
  static const uuid_t custom_service = { .size             = 16,
                                         .reserved         = { 0x00, 0x00, 0x00 },
                                         .val.val128.data1 = 0x6A4E3300,
                                         .val.val128.data2 = 0x667B,
                                         .val.val128.data3 = 0x11E3,
                                         .val.val128.data4 = { 0x9A, 0x94, 0x00, 0x08, 0x66, 0x9A, 0x0C, 0x20 } };

  // 0x6A4E3304-667B-11E3-949A-0800200C9A66
  static const uuid_t custom_characteristic = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0x6A4E3304,
    .val.val128.data2 = 0x667B,
    .val.val128.data3 = 0x11E3,
    .val.val128.data4 = { 0x9A, 0x94, 0x00, 0x08, 0x66, 0x9A, 0x0C, 0x20 }
  };

  rsi_ble_resp_add_serv_t new_serv_resp = { 0 };
  rsi_ble_add_service(custom_service, &new_serv_resp);

  //! adding custom characteristic declaration to the custom service
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 1,
                            RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE, // Set read, write, write without response
                            new_serv_resp.start_handle + 2,
                            custom_characteristic,
                            0);

  //! adding characteristic value attribute to the service
  rsi_ble_att2_val_hndl = new_serv_resp.start_handle + 2;
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 2,
                           custom_characteristic,
                           RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE, // Set read, write, write without response
                           data1,
                           sizeof(data1),
                           1);
  return 0;
}
uint8_t rsi_get_remote_device_role(uint8_t *remote_dev_addr)
{
  uint8_t role = CENTRAL_ROLE, i;

  //! Loop all structures and if the device addr is matched for peripheral structure, then return peripheral role or else central role
  for (i = 0; i < (RSI_BLE_MAX_NBR_PERIPHERALS); i++) {
    if (memcmp(rsi_ble_conn_info[i].remote_dev_addr, remote_dev_addr, RSI_REM_DEV_ADDR_LEN) == 0) {
      return rsi_ble_conn_info[i].remote_device_role;
    }
  }
  return role; //! Returning role as central
}
rsi_ble_profile_list_by_conn_t rsi_ble_profile_list_by_conn[5] = { 0 };
/*=======================================================================*/
//   ! EXTERN VARIABLES
/*=======================================================================*/

uint8_t RSI_NULL_BLE_ADDR[RSI_REM_DEV_ADDR_LEN] = { 0 };

rsi_parsed_conf_t rsi_parsed_conf = { 0 };

/*=======================================================================*/
//   ! EXTERN FUNCTIONS
/*=======================================================================*/
extern void rsi_ble_gap_extended_register_callbacks(rsi_ble_on_remote_features_t ble_on_remote_features_event,
                                                    rsi_ble_on_le_more_data_req_t ble_on_le_more_data_req_event);
void rsi_assign_remote_data_serv_and_char()
{

  //! TO-Do: this initialization should be taken care in parsing itself
  //! assign the remote data transfer service and characteristic UUID's to local buffer
  for (uint8_t i = 0; i < TOTAL_CONNECTIONS; i++) {
    //Parsed Connection Init
    rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[i].tx_write_clientservice_uuid =
      RSI_BLE_CLIENT_WRITE_SERVICE_UUID;
    rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[i].tx_write_client_char_uuid = RSI_BLE_CLIENT_WRITE_CHAR_UUID;
    rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[i].tx_wnr_client_service_uuid =
      RSI_BLE_CLIENT_WRITE_NO_RESP_SERVICE_UUID;
    rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[i].tx_wnr_client_char_uuid =
      RSI_BLE_CLIENT_WRITE_NO_RESP_CHAR_UUID;
    rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[i].rx_indi_client_service_uuid =
      RSI_BLE_CLIENT_INIDCATIONS_SERVICE_UUID;
    rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[i].rx_indi_client_char_uuid =
      RSI_BLE_CLIENT_INIDCATIONS_CHAR_UUID;
    rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[i].rx_notif_client_service_uuid =
      RSI_BLE_CLIENT_NOTIFICATIONS_SERVICE_UUID;
    rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[i].rx_notif_client_char_uuid =
      RSI_BLE_CLIENT_NOTIFICATIONS_CHAR_UUID;

    //rsi_ble_profile_list_by_conn inii

    rsi_ble_profile_list_by_conn[i].profile_desc      = NULL;
    rsi_ble_profile_list_by_conn[i].profile_info_uuid = NULL;
    rsi_ble_profile_list_by_conn[i].profile_char_info = NULL;
  }
}

uint8_t rsi_check_dev_list_driver(uint8_t *remote_dev_name, uint8_t *adv_dev_addr)
{
  LOG_PRINT_D("\r\n inside rsi_check_dev_list_driver \n");
  uint8_t i                       = 0;
  uint8_t peripheral_device_found = NO_PERIPHERAL_FOUND;

  // These statements are added only to resolve compilation warning, value is unchanged
#if ((CONNECT_OPTION == CONN_BY_NAME) && RSI_DEBUG_EN)
  uint8_t *rem_dev_name        = remote_dev_name;
  uint8_t *advertised_dev_addr = adv_dev_addr;
#elif (CONNECT_OPTION == CONN_BY_NAME)
  UNUSED_PARAMETER(adv_dev_addr);
  uint8_t *rem_dev_name = remote_dev_name;
#else
  uint8_t *advertised_dev_addr = adv_dev_addr;
  UNUSED_PARAMETER(remote_dev_name);
#endif

#if (CONNECT_OPTION == CONN_BY_NAME)
  if ((strcmp((const char *)rem_dev_name, "")) == 0) {
    return peripheral_device_found;
  }

  if ((strcmp((const char *)rem_dev_name, RSI_REMOTE_DEVICE_NAME1)) == 0) {
    peripheral_device_found = PERIPHERAL_FOUND;
  } else if ((strcmp((const char *)rem_dev_name, RSI_REMOTE_DEVICE_NAME2)) == 0) {
    peripheral_device_found = PERIPHERAL_FOUND;
  } else if ((strcmp((const char *)rem_dev_name, RSI_REMOTE_DEVICE_NAME3)) == 0) {
    peripheral_device_found = PERIPHERAL_FOUND;
  } else
    return peripheral_device_found;

  //! check if remote device already connected or advertise report received- TODO .  Can check efficiently?
  if (peripheral_device_found == PERIPHERAL_FOUND) {
    for (i = 0; i < (RSI_BLE_MAX_NBR_PERIPHERALS); i++) {
      if (rsi_ble_conn_info[i].rsi_remote_name != NULL) {
        if (!(strcmp((const char *)rsi_ble_conn_info[i].rsi_remote_name, (const char *)rem_dev_name))) {
          peripheral_device_found = PERIPHERAL_CONNECTED;
#if RSI_DEBUG_EN
          LOG_PRINT_D("\r\n Device %s already connected!!!\r\n", advertised_dev_addr);
#endif
          break;
        }
      }
    }
  }
#else
  if (!strcmp(RSI_BLE_DEV_1_ADDR, (char *)advertised_dev_addr)) {
    peripheral_device_found = PERIPHERAL_FOUND;
  } else if (!strcmp(RSI_BLE_DEV_2_ADDR, (char *)advertised_dev_addr)) {
    peripheral_device_found = PERIPHERAL_FOUND;
  } else if (!strcmp(RSI_BLE_DEV_3_ADDR, (char *)advertised_dev_addr)) {
    peripheral_device_found = PERIPHERAL_FOUND;
  } else
    return peripheral_device_found;

  //! check if remote device already connected
  if (peripheral_device_found == PERIPHERAL_FOUND) {
    for (i = 0; i < (RSI_BLE_MAX_NBR_PERIPHERALS); i++) {
      if (!memcmp(rsi_ble_conn_info[i].remote_dev_addr, advertised_dev_addr, RSI_REM_DEV_ADDR_LEN)) {
        peripheral_device_found = PERIPHERAL_CONNECTED;
#if RSI_DEBUG_EN
        LOG_PRINT_D("\r\n Device %s already connected!!!\r\n", advertised_dev_addr);
#endif
        break;
      }
    }
  }

#endif
  if (i == RSI_BLE_MAX_NBR_PERIPHERALS) {
    peripheral_device_found = PERIPHERAL_NOT_CONNECTED;
  }

  return peripheral_device_found;
}

#if (CONNECT_OPTION == CONN_BY_NAME)
uint8_t rsi_get_ble_conn_id(uint8_t *remote_dev_addr, uint8_t *remote_name, uint8_t size)
#else
uint8_t rsi_get_ble_conn_id(uint8_t *remote_dev_addr)
#endif
{
  uint8_t conn_id = 0xFF; //! Max connections (0xFF -1)
  uint8_t i       = 0;

  for (i = 0; i < (TOTAL_CONNECTIONS); i++) {
    if (!memcmp(rsi_ble_conn_info[i].remote_dev_addr, remote_dev_addr, RSI_REM_DEV_ADDR_LEN)) {
      conn_id = i;
      break;
    }
  }

  //! if bd_addr not found, add to the list
  if (conn_id == 0xFF) {
#if (CONNECT_OPTION == CONN_BY_NAME)
    conn_id = rsi_add_ble_conn_id(remote_dev_addr, remote_name, size);
#else
    conn_id = rsi_add_ble_conn_id(remote_dev_addr);
#endif
  }

  return conn_id;
}

#if (CONNECT_OPTION == CONN_BY_NAME)
uint8_t rsi_add_ble_conn_id(uint8_t *remote_dev_addr, uint8_t *remote_name, uint8_t size)
#else
uint8_t rsi_add_ble_conn_id(uint8_t *remote_dev_addr)
#endif
{
  uint8_t conn_id = 0xFF; //! Max connections (0xFF -1)
  uint8_t i       = 0;

  for (i = 0; i < (RSI_BLE_MAX_NBR_PERIPHERALS); i++) {
    if (!memcmp(rsi_ble_conn_info[i].remote_dev_addr, RSI_NULL_BLE_ADDR, RSI_REM_DEV_ADDR_LEN)) {
      memcpy(rsi_ble_conn_info[i].remote_dev_addr, remote_dev_addr, RSI_REM_DEV_ADDR_LEN);
#if (CONNECT_OPTION == CONN_BY_NAME)
      rsi_ble_conn_info[i].rsi_remote_name = (uint8_t *)malloc((size + 1) * sizeof(uint8_t));
      memset(rsi_ble_conn_info[i].rsi_remote_name, 0, size + 1);
      memcpy(rsi_ble_conn_info[i].rsi_remote_name, remote_name, size);
#endif
      rsi_ble_conn_info[i].remote_device_role = PERIPHERAL_ROLE; //! remote device is peripheral
      conn_id                                 = i;
      break;
    }
  }

  rsi_ble_conn_info[conn_id].conn_id = conn_id;

  return conn_id;
}
/*=======================================================================*/
//   AE FUNCTION DEFINITIONS
/*=======================================================================*/

/*==============================================*/
/**
 * @fn         ble_ae_set_1_parameters
 * @brief      this function is used to set extended advertising parameters
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to set extended advertising parameters of first set
 */
int32_t ble_ae_set_1_parameters(void)
{
  //! set ae adv params
  rsi_ble_ae_adv_params_t ble_ae_params = { 0 };
  uint8_t peer_addr[RSI_DEV_ADDR_LEN]   = { 0 };

  ble_ae_params.adv_handle                = BLE_AE_ADV_HNDL_SET_1;
  ble_ae_params.adv_event_prop            = BLE_AE_ADV_EVNT_PROP_SET_1;
  ble_ae_params.primary_adv_intterval_min = BLE_AE_ADV_INT_MIN_SET_1; //140 only 3bytes are valid;
  ble_ae_params.primary_adv_intterval_max = BLE_AE_ADV_INT_MAX_SET_1; //160 only 3bytes are valid;
  ble_ae_params.primary_adv_chnl_map      = BLE_AE_ADV_CHANNEL_MAP_SET_1;
  ble_ae_params.own_addr_type             = LE_RANDOM_ADDRESS;
  ble_ae_params.peer_addr_type            = LE_RANDOM_ADDRESS;
  rsi_ascii_dev_address_to_6bytes_rev((uint8_t *)peer_addr, (int8_t *)REM_DEV_ADDR);
  memcpy(ble_ae_params.peer_dev_addr, peer_addr, RSI_DEV_ADDR_LEN);
  ble_ae_params.adv_filter_policy      = BLE_AE_ADV_FILTER_POLICY_SET_1;
  ble_ae_params.adv_tx_power           = BLE_AE_ADV_TX_PWR_SET_1;
  ble_ae_params.primary_adv_phy        = BLE_AE_PRIMARY_ADV_PHY_SET_1;
  ble_ae_params.sec_adv_max_skip       = BLE_AE_SEC_ADV_MAX_SKIP_SET_1;
  ble_ae_params.sec_adv_phy            = BLE_AE_SECONDARY_ADV_PHY_SET_1;
  ble_ae_params.adv_sid                = BLE_AE_ADV_SID_SET_1;
  ble_ae_params.scan_req_notify_enable = BLE_AE_SCAN_REQ_NOTIF_EN_SET_1;

  //SAPI function call for setting ae parameters
  status = rsi_ble_set_ae_params(&ble_ae_params, &rsi_app_resp_tx_power);

  return status;
}

/*==============================================*/
/**
 * @fn         ble_ae_set_2_parameters
 * @brief      this function is used to set extended advertising parameters
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to set extended advertising parameters of second set
 */
int32_t ble_ae_set_2_parameters(void)
{
  //! set ae adv params
  rsi_ble_ae_adv_params_t ble_ae_params = { 0 };
  uint8_t peer_addr[RSI_DEV_ADDR_LEN]   = { 0 };

  ble_ae_params.adv_handle                = BLE_AE_ADV_HNDL_SET_2;
  ble_ae_params.adv_event_prop            = BLE_AE_ADV_EVNT_PROP_SET_2;
  ble_ae_params.primary_adv_intterval_min = BLE_AE_ADV_INT_MIN_SET_2; //140 only 3bytes are valid;
  ble_ae_params.primary_adv_intterval_max = BLE_AE_ADV_INT_MAX_SET_2; //160 only 3bytes are valid;
  ble_ae_params.primary_adv_chnl_map      = BLE_AE_ADV_CHANNEL_MAP_SET_2;
  ble_ae_params.own_addr_type             = LE_PUBLIC_ADDRESS;
  ble_ae_params.peer_addr_type            = LE_RANDOM_ADDRESS;
  rsi_ascii_dev_address_to_6bytes_rev((uint8_t *)peer_addr, (int8_t *)REM_DEV_ADDR);
  memcpy(ble_ae_params.peer_dev_addr, peer_addr, RSI_DEV_ADDR_LEN);
  ble_ae_params.adv_filter_policy      = BLE_AE_ADV_FILTER_POLICY_SET_2;
  ble_ae_params.adv_tx_power           = BLE_AE_ADV_TX_PWR_SET_2;
  ble_ae_params.primary_adv_phy        = BLE_AE_PRIMARY_ADV_PHY_SET_2;
  ble_ae_params.sec_adv_max_skip       = BLE_AE_SEC_ADV_MAX_SKIP_SET_2;
  ble_ae_params.sec_adv_phy            = BLE_AE_SECONDARY_ADV_PHY_SET_2;
  ble_ae_params.adv_sid                = BLE_AE_ADV_SID_SET_2;
  ble_ae_params.scan_req_notify_enable = BLE_AE_SCAN_REQ_NOTIF_EN_SET_2;

  //SAPI function call for setting ae parameters
  status = rsi_ble_set_ae_params(&ble_ae_params, &rsi_app_resp_tx_power);

  return status;
}

/*==============================================*/
/**
 * @fn         ble_ae_set_periodic_parameters
 * @brief      this function is used to set periodic advertising parameters
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to set periodic advertising parameters for set 1
 */
int32_t ble_ae_set_periodic_parameters(void)
{
  rsi_ble_ae_periodic_adv_params_t ae_periodic_param = { 0 };
  ae_periodic_param.max_interval                     = BLE_AE_PER_ADV_INT_MAX;
  ae_periodic_param.min_interval                     = BLE_AE_PER_ADV_INT_MIN;
  ae_periodic_param.properties                       = BLE_AE_PER_ADV_PROP;
  ae_periodic_param.adv_handle                       = BLE_AE_ADV_HNDL_SET_1;

  //SAPI function call for setting periodic advertising parameters
  status = rsi_ble_app_set_periodic_ae_params(&ae_periodic_param);

  return status;
}

/*==============================================*/
/**
 * @fn         ble_ae_set_1_advertising_enable
 * @brief      this function is used to enable extended advertising
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to enable extended advertising for set 1
 */
int32_t ble_ae_set_1_advertising_enable(void)
{
  //ae advertising enable for set 1
  rsi_ble_ae_adv_enable_t ble_ae_adv = { 0 };

  ble_ae_adv.enable        = RSI_BLE_START_ADV;
  ble_ae_adv.no_of_sets    = 1;
  ble_ae_adv.adv_handle    = BLE_AE_ADV_HNDL_SET_1;
  ble_ae_adv.duration      = BLE_AE_ADV_DUR_SET_1;
  ble_ae_adv.max_ae_events = BLE_AE_ADV_MAX_AE_EVENTS_SET_1;

  //SAPI function call for enabling extended advertising for set 1
  status = rsi_ble_start_ae_advertising(&ble_ae_adv);

  return status;
}

/*==============================================*/
/**
 * @fn         ble_ae_set_2_advertising_enable
 * @brief      this function is used to enable extended advertising
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to enable extended advertising for set 2
 */
int32_t ble_ae_set_2_advertising_enable(void)
{
  //ae advertising enable for set 1
  rsi_ble_ae_adv_enable_t ble_ae_adv = { 0 };

  ble_ae_adv.enable        = RSI_BLE_START_ADV;
  ble_ae_adv.no_of_sets    = 1;
  ble_ae_adv.adv_handle    = BLE_AE_ADV_HNDL_SET_2;
  ble_ae_adv.duration      = BLE_AE_ADV_DUR_SET_2;
  ble_ae_adv.max_ae_events = BLE_AE_ADV_MAX_AE_EVENTS_SET_2;

  //SAPI function call for enabling extended advertising for set 1
  status = rsi_ble_start_ae_advertising(&ble_ae_adv);

  return status;
}

/*==============================================*/
/**
 * @fn         ble_ae_set_periodic_data
 * @brief      this function is used to set periodic advertising data
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to enable extended advertising for set 2
 */
int32_t ble_ae_set_periodic_data(void)
{
  //setting periodic advertising data
  rsi_ble_ae_data_t ble_ae_data = { 0 };
  ble_ae_data.type              = BLE_AE_PERIODIC_ADV_DATA_TYPE;
  ble_ae_data.adv_handle        = BLE_AE_ADV_HNDL_SET_1;
  ble_ae_data.operation         = 0x00;
  ble_ae_data.frag_pref         = 0x00;
  ble_ae_data.data_len          = BLE_AE_ADV_DATA_LEN;
  memcpy(ble_ae_data.data, adv, ble_ae_data.data_len);

  //SAPI function call for setting periodic adv data
  status = rsi_ble_set_ae_data(&ble_ae_data);

  return status;
}

/*==============================================*/
/**
 * @fn         ble_ae_set_1_adv_data
 * @brief      this function is used to set advertising data for set 1
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to set advertising data for first set
 */
int32_t ble_ae_set_1_adv_data(void)
{
  //setting extended advertising data
  rsi_ble_ae_data_t ble_ae_data = { 0 };

  ble_ae_data.type       = BLE_AE_ADV_DATA_TYPE;
  ble_ae_data.adv_handle = BLE_AE_ADV_HNDL_SET_1;
  ble_ae_data.operation  = 0x00;
  ble_ae_data.frag_pref  = 0x00;
  ble_ae_data.data_len   = BLE_AE_ADV_DATA_LEN;
  memcpy(ble_ae_data.data, adv, ble_ae_data.data_len);

  //SAPI function call for setting ae adv data of set 1
  status = rsi_ble_set_ae_data(&ble_ae_data);

  return status;
}

/*==============================================*/
/**
 * @fn         ble_ae_set_2_adv_data
 * @brief      this function is used to set advertising data for set 2
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to set advertising data for second set
 */
int32_t ble_ae_set_2_adv_data(void)
{
  //setting extended advertising data
  rsi_ble_ae_data_t ble_ae_data = { 0 };

  ble_ae_data.type       = BLE_AE_ADV_DATA_TYPE;
  ble_ae_data.adv_handle = BLE_AE_ADV_HNDL_SET_2;
  ble_ae_data.operation  = 0x00;
  ble_ae_data.frag_pref  = 0x00;
  ble_ae_data.data_len   = BLE_AE_ADV_DATA_LEN;
  memcpy(ble_ae_data.data, adv, ble_ae_data.data_len);

  //SAPI function call for setting ae adv data of set 2
  status = rsi_ble_set_ae_data(&ble_ae_data);

  return status;
}

/*==============================================*/
/**
 * @fn         ble_ae_set_1_scan_resp_data
 * @brief      this function is used to set scan response data for set 1
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to set scan response data for first set
 */
int32_t ble_ae_set_1_scan_resp_data(void)
{
  //setting ae scan response data
  rsi_ble_ae_data_t ble_ae_data = { 0 };
  ble_ae_data.type              = BLE_AE_SCAN_RSP_DATA_TYPE;
  ble_ae_data.adv_handle        = BLE_AE_ADV_HNDL_SET_1;
  ble_ae_data.operation         = 0x00;
  ble_ae_data.frag_pref         = 0x00;
  ble_ae_data.data_len          = BLE_AE_ADV_DATA_LEN;
  memcpy(ble_ae_data.data, adv, ble_ae_data.data_len);

  //SAPI function call for setting scan response data of set 1
  status = rsi_ble_set_ae_data(&ble_ae_data);

  return status;
}

/*==============================================*/
/**
 * @fn         ble_ae_set_2_scan_resp_data
 * @brief      this function is used to set scan response data for set 2
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to set scan response data for second set
 */
int32_t ble_ae_set_2_scan_resp_data(void)
{
  //setting ae scan response data
  rsi_ble_ae_data_t ble_ae_data = { 0 };
  ble_ae_data.type              = BLE_AE_SCAN_RSP_DATA_TYPE;
  ble_ae_data.adv_handle        = BLE_AE_ADV_HNDL_SET_2;
  ble_ae_data.operation         = 0x00;
  ble_ae_data.frag_pref         = 0x00;
  ble_ae_data.data_len          = BLE_AE_ADV_DATA_LEN;
  memcpy(ble_ae_data.data, adv, ble_ae_data.data_len);

  //SAPI function call for setting scan response data of set 2
  status = rsi_ble_set_ae_data(&ble_ae_data);

  return status;
}

/*==============================================*/
/**
 * @fn         ble_ext_scan_params
 * @brief      this function is used to set extended scan parameters
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to set extended scan parameters
 */
int32_t ble_ext_scan_params(void)
{

#define SET2_ENABLE 1
  // AE set scan params
  rsi_ble_ae_set_scan_params_t ae_set_scan_params = { 0 };
  ae_set_scan_params.own_addr_type                = LE_PUBLIC_ADDRESS;
  ae_set_scan_params.scanning_filter_policy       = BLE_AE_SCAN_FILTER_TYPE;
  ae_set_scan_params.scanning_phys                = (PHY_1M | PHY_LE_CODED);
  ae_set_scan_params.ScanParams[0].ScanType       = PRI_PHY_BLE_AE_SCAN_TYPE;
  ae_set_scan_params.ScanParams[0].ScanInterval   = PRI_PHY_LE_AE_SCAN_INTERVAL;
  ae_set_scan_params.ScanParams[0].ScanWindow     = PRI_PHY_LE_AE_SCAN_WINDOW;
#if SET2_ENABLE
  ae_set_scan_params.ScanParams[1].ScanType     = SEC_PHY_BLE_AE_SCAN_TYPE;
  ae_set_scan_params.ScanParams[1].ScanInterval = SEC_PHY_LE_AE_SCAN_INTERVAL;
  ae_set_scan_params.ScanParams[1].ScanWindow   = SEC_PHY_LE_AE_SCAN_WINDOW;
#endif

  //SAPI function call for setting the scan parameters
  status = rsi_ble_ae_set_scan_params(&ae_set_scan_params);

  return status;
}

/*==============================================*/
/**
 * @fn         ble_ext_scan_enable
 * @brief      this function is used to set extended scan enable
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to set extended scan enable
 */
int32_t ble_ext_scan_enable(void)
{
  // AE scan enable
  rsi_ble_ae_set_scan_enable_t ae_set_scan_enable = { 0 };
  ae_set_scan_enable.enable                       = RSI_BLE_START_SCAN;
  ae_set_scan_enable.filter_duplicates            = BLE_AE_SCAN_ENABLE_FILTER_DUP;
  ae_set_scan_enable.duration                     = BLE_AE_SCAN_DUR;
  ae_set_scan_enable.period                       = BLE_AE_SCAN_PERIOD;

  //SAPI Function call to enable extended scanning
  status = rsi_ble_ae_set_scan_enable(&ae_set_scan_enable);

  return status;
}

/*==============================================*/
/**
 * @fn         rsi_ble_initialize_conn_buffer
 * @brief      this function initializes the configurations for each connection
 * @param[out] none
 * @param[out] none
 * @return     none
 * @section description
 */
int8_t rsi_ble_initialize_conn_buffer(rsi_ble_conn_config_t *ble_conn_spec_conf)
{

  LOG_PRINT_D("\r\n init conn buffer funstion caalled \n");
  int8_t status = RSI_SUCCESS;
  if (ble_conn_spec_conf != NULL) {
    if (RSI_BLE_MAX_NBR_PERIPHERALS > 0) {
      //! Initialize peripheral1 configurations
      ble_conn_spec_conf[PERIPHERAL1].smp_enable        = SMP_ENABLE_P1;
      ble_conn_spec_conf[PERIPHERAL1].add_to_acceptlist = ADD_TO_ACCEPTLIST_P1;
      ble_conn_spec_conf[PERIPHERAL1].profile_discovery = PROFILE_QUERY_P1;
      ble_conn_spec_conf[PERIPHERAL1].data_transfer     = DATA_TRANSFER_P1;
      // ble_conn_spec_conf[PERIPHERAL1].bidir_datatransfer = SMP_ENABLE_P1;
      ble_conn_spec_conf[PERIPHERAL1].rx_notifications                 = RX_NOTIFICATIONS_FROM_P1;
      ble_conn_spec_conf[PERIPHERAL1].rx_indications                   = RX_INDICATIONS_FROM_P1;
      ble_conn_spec_conf[PERIPHERAL1].tx_notifications                 = TX_NOTIFICATIONS_TO_P1;
      ble_conn_spec_conf[PERIPHERAL1].tx_write                         = TX_WRITES_TO_P1;
      ble_conn_spec_conf[PERIPHERAL1].tx_write_no_response             = TX_WRITES_NO_RESP_TO_P1;
      ble_conn_spec_conf[PERIPHERAL1].tx_indications                   = TX_INDICATIONS_TO_P1;
      ble_conn_spec_conf[PERIPHERAL1].conn_param_update.conn_int       = CONN_INTERVAL_P1;
      ble_conn_spec_conf[PERIPHERAL1].conn_param_update.conn_latncy    = CONN_LATENCY_P1;
      ble_conn_spec_conf[PERIPHERAL1].conn_param_update.supervision_to = CONN_SUPERVISION_TIMEOUT_P1;
      ble_conn_spec_conf[PERIPHERAL1].buff_mode_sel.buffer_mode        = DLE_BUFFER_MODE_P1;
      ble_conn_spec_conf[PERIPHERAL1].buff_mode_sel.buffer_cnt         = DLE_BUFFER_COUNT_P1;
      ble_conn_spec_conf[PERIPHERAL1].buff_mode_sel.max_data_length    = RSI_BLE_MAX_DATA_LEN_P1;
    }

    if (RSI_BLE_MAX_NBR_PERIPHERALS > 1) {
      //! Initialize peripheral2 configurations
      ble_conn_spec_conf[PERIPHERAL2].smp_enable        = SMP_ENABLE_P2;
      ble_conn_spec_conf[PERIPHERAL2].add_to_acceptlist = ADD_TO_ACCEPTLIST_P2;
      ble_conn_spec_conf[PERIPHERAL2].profile_discovery = PROFILE_QUERY_P2;
      ble_conn_spec_conf[PERIPHERAL2].data_transfer     = DATA_TRANSFER_P2;
      // ble_conn_spec_conf[PERIPHERAL2].bidir_datatransfer = SMP_ENABLE_P2;
      ble_conn_spec_conf[PERIPHERAL2].rx_notifications                 = RX_NOTIFICATIONS_FROM_P2;
      ble_conn_spec_conf[PERIPHERAL2].rx_indications                   = RX_INDICATIONS_FROM_P2;
      ble_conn_spec_conf[PERIPHERAL2].tx_notifications                 = TX_NOTIFICATIONS_TO_P2;
      ble_conn_spec_conf[PERIPHERAL2].tx_write                         = TX_WRITES_TO_P2;
      ble_conn_spec_conf[PERIPHERAL2].tx_write_no_response             = TX_WRITES_NO_RESP_TO_P2;
      ble_conn_spec_conf[PERIPHERAL2].tx_indications                   = TX_INDICATIONS_TO_P2;
      ble_conn_spec_conf[PERIPHERAL2].conn_param_update.conn_int       = CONN_INTERVAL_P2;
      ble_conn_spec_conf[PERIPHERAL2].conn_param_update.conn_latncy    = CONN_LATENCY_P2;
      ble_conn_spec_conf[PERIPHERAL2].conn_param_update.supervision_to = CONN_SUPERVISION_TIMEOUT_P2;
      ble_conn_spec_conf[PERIPHERAL2].buff_mode_sel.buffer_mode        = DLE_BUFFER_MODE_P2;
      ble_conn_spec_conf[PERIPHERAL2].buff_mode_sel.buffer_cnt         = DLE_BUFFER_COUNT_P2;
      ble_conn_spec_conf[PERIPHERAL2].buff_mode_sel.max_data_length    = RSI_BLE_MAX_DATA_LEN_P2;
    }

    if (RSI_BLE_MAX_NBR_PERIPHERALS > 2) {
      //! Initialize PERIPHERAL3 configurations
      ble_conn_spec_conf[PERIPHERAL3].smp_enable        = SMP_ENABLE_P3;
      ble_conn_spec_conf[PERIPHERAL3].add_to_acceptlist = ADD_TO_ACCEPTLIST_P3;
      ble_conn_spec_conf[PERIPHERAL3].profile_discovery = PROFILE_QUERY_P3;
      ble_conn_spec_conf[PERIPHERAL3].data_transfer     = DATA_TRANSFER_P3;
      // ble_conn_spec_conf[PERIPHERAL3].bidir_datatransfer = SMP_ENABLE_P3;
      ble_conn_spec_conf[PERIPHERAL3].rx_notifications                 = RX_NOTIFICATIONS_FROM_P3;
      ble_conn_spec_conf[PERIPHERAL3].rx_indications                   = RX_INDICATIONS_FROM_P3;
      ble_conn_spec_conf[PERIPHERAL3].tx_notifications                 = TX_NOTIFICATIONS_TO_P3;
      ble_conn_spec_conf[PERIPHERAL3].tx_write                         = TX_WRITES_TO_P3;
      ble_conn_spec_conf[PERIPHERAL3].tx_write_no_response             = TX_WRITES_NO_RESP_TO_P3;
      ble_conn_spec_conf[PERIPHERAL3].tx_indications                   = TX_INDICATIONS_TO_P3;
      ble_conn_spec_conf[PERIPHERAL3].conn_param_update.conn_int       = CONN_INTERVAL_P3;
      ble_conn_spec_conf[PERIPHERAL3].conn_param_update.conn_latncy    = CONN_LATENCY_P3;
      ble_conn_spec_conf[PERIPHERAL3].conn_param_update.supervision_to = CONN_SUPERVISION_TIMEOUT_P3;
      ble_conn_spec_conf[PERIPHERAL3].buff_mode_sel.buffer_mode        = DLE_BUFFER_MODE_P3;
      ble_conn_spec_conf[PERIPHERAL3].buff_mode_sel.buffer_cnt         = DLE_BUFFER_COUNT_P3;
      ble_conn_spec_conf[PERIPHERAL3].buff_mode_sel.max_data_length    = RSI_BLE_MAX_DATA_LEN_P3;
    }

    if (RSI_BLE_MAX_NBR_CENTRALS > 0) {
      //! Initialize central1 configurations
      ble_conn_spec_conf[CENTRAL1].smp_enable        = SMP_ENABLE_C1;
      ble_conn_spec_conf[CENTRAL1].add_to_acceptlist = ADD_TO_ACCEPTLIST_C1;
      ble_conn_spec_conf[CENTRAL1].profile_discovery = PROFILE_QUERY_C1;
      ble_conn_spec_conf[CENTRAL1].data_transfer     = DATA_TRANSFER_C1;
      // ble_conn_spec_conf[CENTRAL1].bidir_datatransfer = SMP_ENABLE_C1;
      ble_conn_spec_conf[CENTRAL1].rx_notifications                 = RX_NOTIFICATIONS_FROM_C1;
      ble_conn_spec_conf[CENTRAL1].rx_indications                   = RX_INDICATIONS_FROM_C1;
      ble_conn_spec_conf[CENTRAL1].tx_notifications                 = TX_NOTIFICATIONS_TO_C1;
      ble_conn_spec_conf[CENTRAL1].tx_write                         = TX_WRITES_TO_C1;
      ble_conn_spec_conf[CENTRAL1].tx_write_no_response             = TX_WRITES_NO_RESP_TO_C1;
      ble_conn_spec_conf[CENTRAL1].tx_indications                   = TX_INDICATIONS_TO_C1;
      ble_conn_spec_conf[CENTRAL1].conn_param_update.conn_int       = CONN_INTERVAL_C1;
      ble_conn_spec_conf[CENTRAL1].conn_param_update.conn_latncy    = CONN_LATENCY_C1;
      ble_conn_spec_conf[CENTRAL1].conn_param_update.supervision_to = CONN_SUPERVISION_TIMEOUT_C1;
      ble_conn_spec_conf[CENTRAL1].buff_mode_sel.buffer_mode        = DLE_BUFFER_MODE_C1;
      ble_conn_spec_conf[CENTRAL1].buff_mode_sel.buffer_cnt         = DLE_BUFFER_COUNT_C1;
      ble_conn_spec_conf[CENTRAL1].buff_mode_sel.max_data_length    = RSI_BLE_MAX_DATA_LEN_C1;
    }

    if (RSI_BLE_MAX_NBR_CENTRALS > 1) {
      //! Initialize central2 configurations
      ble_conn_spec_conf[CENTRAL2].smp_enable        = SMP_ENABLE_C2;
      ble_conn_spec_conf[CENTRAL2].add_to_acceptlist = ADD_TO_ACCEPTLIST_C2;
      ble_conn_spec_conf[CENTRAL2].profile_discovery = PROFILE_QUERY_C2;
      ble_conn_spec_conf[CENTRAL2].data_transfer     = DATA_TRANSFER_C2;
      // ble_conn_spec_conf[CENTRAL2].bidir_datatransfer = SMP_ENABLE_C2;
      ble_conn_spec_conf[CENTRAL2].rx_notifications                 = RX_NOTIFICATIONS_FROM_C2;
      ble_conn_spec_conf[CENTRAL2].rx_indications                   = RX_INDICATIONS_FROM_C2;
      ble_conn_spec_conf[CENTRAL2].tx_notifications                 = TX_NOTIFICATIONS_TO_C2;
      ble_conn_spec_conf[CENTRAL2].tx_write                         = TX_WRITES_TO_C2;
      ble_conn_spec_conf[CENTRAL2].tx_write_no_response             = TX_WRITES_NO_RESP_TO_C2;
      ble_conn_spec_conf[CENTRAL2].tx_indications                   = TX_INDICATIONS_TO_C2;
      ble_conn_spec_conf[CENTRAL2].conn_param_update.conn_int       = CONN_INTERVAL_C2;
      ble_conn_spec_conf[CENTRAL2].conn_param_update.conn_latncy    = CONN_LATENCY_C2;
      ble_conn_spec_conf[CENTRAL2].conn_param_update.supervision_to = CONN_SUPERVISION_TIMEOUT_C2;
      ble_conn_spec_conf[CENTRAL2].buff_mode_sel.buffer_mode        = DLE_BUFFER_MODE_C2;
      ble_conn_spec_conf[CENTRAL2].buff_mode_sel.buffer_cnt

        = DLE_BUFFER_COUNT_C2;
      ble_conn_spec_conf[CENTRAL2].buff_mode_sel.max_data_length = RSI_BLE_MAX_DATA_LEN_C2;
    }

    /* Check the Total Number of Buffers allocated.*/
    if ((DLE_BUFFER_COUNT_P1 + DLE_BUFFER_COUNT_P2 + DLE_BUFFER_COUNT_P3 + DLE_BUFFER_COUNT_C1 + DLE_BUFFER_COUNT_C2)
        > RSI_BLE_NUM_CONN_EVENTS) {
      LOG_PRINT("\r\n Total number of per connection buffer count is more than the total number alllocated \r\n");
      status = RSI_FAILURE;
    }
  } else {
    LOG_PRINT("\r\n Invalid buffer passed \r\n");
    status = RSI_FAILURE;
  }
  return status;
}

int8_t rsi_fill_ble_user_config()
{
  LOG_PRINT_D("\r\n fill user function caalled \n");
  int8_t status = RSI_SUCCESS;
  //! copy protocol selection macros
  rsi_parsed_conf.rsi_protocol_sel.is_ble_enabled = BT_TRUE;

  //! copy ble connection specific configurations

  if ((RSI_BLE_MAX_NBR_CENTRALS > 2) || (RSI_BLE_MAX_NBR_PERIPHERALS > 3)) {
    LOG_PRINT("\r\n number of BLE CENTRALS or BLE PERIPHERALS Given wrong declaration\r\n");
    return RSI_FAILURE;
  }

  LOG_PRINT_D("\r\n inside status rsibleinitconnbuffer \n");
  status = rsi_ble_initialize_conn_buffer((rsi_ble_conn_config_t *)&rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config);

  return status;
}

uint8_t str_conn_device[18] = { 0 };
uint16_t mtu_size;
uint16_t rsi_scan_in_progress;

volatile uint16_t rsi_ble_att1_val_hndl;

uint8_t peripheral_con_req_pending    = 0;
uint8_t peripheral_ae_con_req_pending = 0;

#ifdef M4_UART
extern osSemaphoreId_t ui_task_sem;
#endif

int32_t ble_init_hook(void);
void ble_private_default_init(void)
{
  LOG_PRINT("\r\n private default init\r\n");
  // for loop

  uint8_t iter;
  for (iter = 0; iter < TOTAL_CONNECTIONS; iter++) {
    //variables

    rsi_ble_conn_info[i].buff_config_done                        = false;
    rsi_ble_conn_info[i].conn_param_req_given                    = false;
    rsi_ble_conn_info[i].char_resp_recvd                         = false;
    rsi_ble_conn_info[i].char_desc_resp_recvd                    = false;
    rsi_ble_conn_info[i].prof_resp_recvd                         = false;
    rsi_ble_conn_info[i].write_handle_found                      = false;
    rsi_ble_conn_config_t __attribute__((unused)) *ble_conn_conf = NULL;
    rsi_ble_conn_info[i].smp_pairing_initated                    = false;
    rsi_ble_conn_info[i].smp_pairing_request_received            = false;
    rsi_ble_conn_info[i].write_cnt                               = 0;
    rsi_ble_conn_info[i].write_wwr_handle_found                  = false;
    rsi_ble_conn_info[i].write_handle                            = 0;
    rsi_ble_conn_info[i].wwr_count                               = 0;
    rsi_ble_conn_info[i].notify_handle_found                     = false;
    rsi_ble_conn_info[i].notify_handle                           = 0;
    rsi_ble_conn_info[i].indication_handle_found                 = false;
    rsi_ble_conn_info[i].indication_handle                       = 0;
    rsi_ble_conn_info[i].write_wwr_handle                        = 0;

    rsi_ble_conn_info[i].l_num_of_services            = 0;
    rsi_ble_conn_info[i].l_char_property              = 0;
    rsi_ble_conn_info[i].char_for_serv_cnt            = 0;
    rsi_ble_conn_info[i].char_desc_cnt                = 0;
    rsi_ble_conn_info[i].profile_index_for_char_query = 0;
    rsi_ble_conn_info[i].temp1                        = 0;
    rsi_ble_conn_info[i].temp2                        = 0;
    rsi_ble_conn_info[i].temp_prepare_write_value_len = 0;
    memset(rsi_ble_conn_info[i].temp_prepare_write_value, 0, sizeof(rsi_ble_conn_info[i].temp_prepare_write_value));

    rsi_ble_conn_info[i].mtu_exchange_done = 0;

    rsi_ble_conn_info[i].prep_write_err      = 0;
    rsi_ble_conn_info[i].first_connect       = 0;
    rsi_ble_conn_info[i].smp_done            = 0;
    rsi_ble_conn_info[i].neg_rply            = 0;
    rsi_ble_conn_info[i].offset              = 0;
    rsi_ble_conn_info[i].handle              = 0;
    rsi_ble_conn_info[i].type                = 0;
    rsi_ble_conn_info[i].max_data_length     = 0;
    rsi_ble_conn_info[i].rsi_rx_from_rem_dev = false;
    rsi_ble_conn_info[i].rsi_tx_to_rem_dev   = false;
    rsi_ble_conn_info[i].indication_cnt      = 0;
    rsi_ble_conn_info[i].notfy_cnt           = 0;
    rsi_ble_conn_info[i].prof_cnt            = 0;
    rsi_ble_conn_info[i].char_cnt            = 0;

    rsi_ble_conn_info[i].notification_received = false;
    rsi_ble_conn_info[i].done_profiles_query   = false;

    rsi_ble_conn_info[i].no_of_profiles        = 0;
    rsi_ble_conn_info[i].total_remote_profiles = 0;

    rsi_ble_conn_info[i].profs_evt_cnt = 0;
    rsi_ble_conn_info[i].prof_evt_cnt  = 0;

    rsi_ble_conn_info[i].profiles_endhandle = 0;

    memset(rsi_ble_conn_info[i].rsi_connected_dev_addr, 0, sizeof(rsi_ble_conn_info[i].rsi_connected_dev_addr));

    rsi_ble_conn_info[i].profile_mem_init      = false;
    rsi_ble_conn_info[i].service_char_mem_init = false;
    rsi_ble_conn_info[i].skip_end_profile      = 0;
#if (CONNECT_OPTION == CONN_BY_NAME)
    rsi_ble_conn_info[i].rsi_remote_name = NULL;
#endif
    rsi_ble_conn_info[i].switch_case_count = 0;
    rsi_ble_conn_info[i].transmit          = false;
    more_data_state_beta[i].data_transmit  = 0;
  }
}

extern generic_task_cb_t ble_generic_cb;

void rsi_ble_main_app_task(void)
{
  int32_t status;
  sl_wifi_firmware_version_t version = { 0 };
  status                             = sl_wifi_init(&config, NULL, sl_wifi_default_event_handler);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nWi-Fi Initialization Failed, Error Code : 0x%lX\r\n", status);
    return;
  } else {
    LOG_PRINT("\r\n Wi-Fi Initialization Success\n");
  }

  //! Firmware version Prints
  status = sl_wifi_get_firmware_version(&version);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nFirmware version Failed, Error Code : 0x%lX\r\n", status);
  } else {
    print_firmware_version(&version);
  }

  LOG_PRINT("\r\n BLE Main Task Invoked\r\n");

  ble_private_default_init();
  ble_generic_cb.init_hook            = ble_init_hook;
  ble_generic_cb.commands_handler_lut = ble_commands_hdlr_lut;
  ble_generic_cb.events_handler_lut   = ble_events_lut;

  ble_generic_cb.max_num_events   = max_ble_event_id;
  ble_generic_cb.max_num_commands = max_ble_command_id;

  status = rsi_fill_ble_user_config();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n failed to fill the configurations in local buffer \r\n");
    return;
  } else {
    LOG_PRINT("\n FILL USER config successful \n");
  }

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
    LOG_PRINT("\r\n Failed to initiate power save in Wi-Fi mode :%ld\r\n", status);
    return;
  }

  LOG_PRINT("\r\n Module is in power save \r\n");
#endif

  status = rsi_app_common_event_loop(&ble_generic_cb);

  LOG_PRINT("\r\n BLE task excution fails with error status 0X%lx \r\n", status);
  while (1)
    ;
}

int32_t ble_init_hook(void)
{
  int32_t status = RSI_SUCCESS;

  //! BLE register GAP  and GATT callbacks:
  sl_btc_ble_gap_register_callbacks_wrapper();

  sl_btc_ble_gatt_register_callbacks_wrapper();

  sl_btc_ble_gatt_extended_register_callbacks_wrapper();

  sl_btc_ble_gap_extended_register_callbacks_wrapper();

  sl_btc_ble_smp_register_callbacks_wrapper();

  sl_btc_ble_adv_extended_register_callbacks_wrapper();
  //! BLE dual role Initialization
  status = rsi_ble_dual_role();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("BLE DUAL role init failed \r\n");
  }
#ifdef M4_UART
  osSemaphoreRelease(ui_task_sem);
#endif
  return status;
}

void rsi_change_ble_adv_and_scan_params()
{
  // update the new scan and advertise parameters
  memset(&change_adv_param, 0, sizeof(rsi_ble_req_adv_t));
  memset(&change_scan_param, 0, sizeof(rsi_ble_req_scan_t));

  //! advertise parameters
  change_adv_param.status           = RSI_BLE_START_ADV;
  change_adv_param.adv_type         = UNDIR_NON_CONN; //! non connectable advertising
  change_adv_param.filter_type      = RSI_BLE_ADV_FILTER_TYPE;
  change_adv_param.direct_addr_type = RSI_BLE_ADV_DIR_ADDR_TYPE;
  change_adv_param.adv_int_min      = RSI_BLE_ADV_INT_MIN; // advertising interval - 211.25ms
  change_adv_param.adv_int_max      = RSI_BLE_ADV_INT_MAX;
  change_adv_param.own_addr_type    = LE_PUBLIC_ADDRESS;
  change_adv_param.adv_channel_map  = RSI_BLE_ADV_CHANNEL_MAP;
  rsi_ascii_dev_address_to_6bytes_rev(change_adv_param.direct_addr, (int8_t *)RSI_BLE_ADV_DIR_ADDR);

  //! scan paramaters
  change_scan_param.status        = RSI_BLE_START_SCAN;
  change_scan_param.scan_type     = SCAN_TYPE_PASSIVE;
  change_scan_param.filter_type   = RSI_BLE_SCAN_FILTER_TYPE;
  change_scan_param.scan_int      = LE_SCAN_INTERVAL; //! scan interval 33.125ms
  change_scan_param.scan_win      = LE_SCAN_WINDOW;   //! scan window 13.375ms
  change_scan_param.own_addr_type = LE_PUBLIC_ADDRESS;
}

int32_t rsi_ble_dual_role(void)
{
  int32_t status                               = 0;
  uint8_t rand_addr[RSI_DEV_ADDR_LEN]          = { 0 };
  uint8_t local_dev_addr[RSI_REM_DEV_ADDR_LEN] = { 0 };
  rsi_ascii_dev_address_to_6bytes_rev((uint8_t *)rand_addr, (int8_t *)RSI_BLE_SET_RAND_ADDR);

  //! This should be vary from one device to other, Present RSI dont have a support of FIXED IRK on every Reset
  uint8_t local_irk[16] = { 0x4d, 0xd7, 0xbd, 0x3e, 0xec, 0x10, 0xda, 0xab,
                            0x1f, 0x85, 0x56, 0xee, 0xa5, 0xc8, 0xe6, 0x93 };
  rsi_ble_set_smp_pairing_capabilty_data_t smp_capabilities;

  ble_wait_on_connect = osSemaphoreNew(1, 0, NULL);

  rsi_ble_add_simple_chat_serv();
  rsi_ble_add_simple_chat_serv2();
  //! adding BLE Custom  Service service
  rsi_ble_add_custom_service_serv();

  // callbacks

  //! Set local name
  rsi_bt_set_local_name(RSI_BT_LOCAL_NAME);

  //! get the local device address(MAC address).
  status = rsi_bt_get_local_device_address(rsi_app_resp_get_dev_addr);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n rsi_bt_get_local_device_address failed with 0x%lX \n", status);
  } else {
    rsi_6byte_dev_address_to_ascii(local_dev_addr, rsi_app_resp_get_dev_addr);
    LOG_PRINT("\n Local device address = %s", local_dev_addr);
  }

  //! Set local IRK Value
  //! This value should be fixed on every reset
  LOG_PRINT("\r\n Setting the Local IRK Value\r\n");
  status = rsi_ble_set_local_irk_value(local_irk);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Setting the Local IRK Value Failed = %lx\r\n", status);
    return status;
  }

  smp_capabilities.io_capability = RSI_BLE_SMP_IO_CAPABILITY;
  smp_capabilities.oob_data_flag = LOCAL_OOB_DATA_FLAG_NOT_PRESENT;

  smp_capabilities.enc_key_size         = MAXIMUM_ENC_KEY_SIZE_16;
  smp_capabilities.ini_key_distribution = INITIATOR_KEYS_TO_DIST;

  smp_capabilities.rsp_key_distribution = RESPONDER_KEYS_TO_DIST;
  smp_capabilities.auth_req             = AUTH_REQ_BITS;
  status                                = rsi_ble_set_smp_pairing_cap_data(&smp_capabilities);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n rsi_ble_set_smp_pairing_cap_data = %lx", status);
    return status;
  }

  rsi_assign_remote_data_serv_and_char();

  //! Module advertises if central connections are configured

  //! prepare advertise data //local/device name
  /*
  adv[3] = strlen(RSI_BLE_APP_GATT_TEST) + 1;
  adv[4] = 9;
  strcpy((char *)&adv[5], RSI_BLE_APP_GATT_TEST);
*/

  strncpy((char *)adv, BLE_AE_ADV_DATA, BLE_AE_ADV_DATA_LEN);

  //! get the supported max adv data len
  status = rsi_ble_get_max_adv_data_len((uint8_t *)&rsi_app_resp_max_adv_data_len);

  if (status != RSI_SUCCESS) {
    LOG_PRINT("get max adv data length failed with 0x%lX \n", status);
  } else {
    LOG_PRINT("Max supported Adv Data length is %d \n", (uint8_t)rsi_app_resp_max_adv_data_len);
  }

  //! get the Max no.of supported adv sets
  status = rsi_ble_get_max_no_of_supp_adv_sets((uint8_t *)&rsi_app_resp_max_no_of_supp_adv_sets);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("get max supported adv sets failed with 0x%lX\n", status);
  } else {
    LOG_PRINT("Max number of supported Adv sets are %ld  \n", rsi_app_resp_max_no_of_supp_adv_sets);
  }

#if ADV_ENABLED_DEFAULT

  status = ble_ae_set_1_parameters();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae params failed with 0x%lX \n", status);
  } else {
    LOG_PRINT("Setting AE params of set 1 successful and selected TX Power is %d dbm \n", rsi_app_resp_tx_power);
  }
#if PERIODIC_ADV_EN
  status = ble_ae_set_periodic_parameters();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae Periodic adv data failed with 0x%lX\n", status);
  } else {
    LOG_PRINT("set ae periodic adv data success \n");
  }

  //SAPI Function call for periodic advertising enable
  status = rsi_ble_app_set_periodic_ae_enable(BLE_AE_PER_ADV_EN, BLE_AE_ADV_HNDL_SET_1);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae Periodic adv enable failed with 0x%lX\n", status);
  } else {
    //adv_state_dut = adv_enabled;
    LOG_PRINT("set ae periodic adv enable success \n");
  }
#endif
#if ADV_SET2
  status = ble_ae_set_2_parameters();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae params failed with 0x%lX \n", status);
  } else {
    LOG_PRINT("Setting AE params of set 2 successful and selected TX Power is %d dbm \n", rsi_app_resp_tx_power);
  }

#endif

#if PERIODIC_ADV_EN
  status = ble_ae_set_periodic_data();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae adv enable failed with status 0x%lX\n", status);
  } else {
    LOG_PRINT("set ae adv enable success \n");
  }
#else

  status = ble_ae_set_1_adv_data();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae adv data for set 1 failed with status 0x%lX\n", status);
  } else {
    LOG_PRINT("set ae adv data for set 1 success \n");
  }

  status = ble_ae_set_1_scan_resp_data();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae scan resp data for set 1 failed with 0x%lX\n", status);
  } else {
    LOG_PRINT("set ae scan resp data for set 1 success \n");
  }

#endif
#if ADV_SET2

  status = ble_ae_set_2_adv_data();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae adv data for set 2 failed with status 0x%lX\n", status);
  } else {
    LOG_PRINT("set ae adv data for set 2 success \n");
  }

  status = ble_ae_set_2_scan_resp_data();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae scan resp data for set 2 failed with 0x%lX\n", status);
  } else {
    LOG_PRINT("set ae scan resp data for set 2 success \n");
  }

#endif

  //! set AE set random address
  status = rsi_ble_set_ae_set_random_address(BLE_AE_ADV_HNDL_SET_1, rand_addr);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae set random address failed with 0x%lX \n", status);
  } else {
    LOG_PRINT("set ae set random address successful \n");
  }

  status = ble_ae_set_1_advertising_enable();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set 1 ae adv enable failed with status 0x%lX\n", status);
  } else {
    LOG_PRINT("set 1 ae adv enable success \n");
  }

#if ADV_SET2

  status = ble_ae_set_2_advertising_enable();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set 2 ae adv enable failed with status 0x%lX\n", status);
  } else {
    LOG_PRINT("set 2 ae adv enable success \n");
  }

#endif

#endif

#if SCAN_ENABLED_DEFAULT
  //! Module scans if peripheral connections are configured

  status = ble_ext_scan_params();
  if (status != RSI_SUCCESS) {
    LOG_PRINT(" \n set ae scan params failed with status 0x%lX\n", status);
  } else {
    LOG_PRINT(" \n set ae scan params success \n");
  }

  status = ble_ext_scan_enable();
  if (status != RSI_SUCCESS) {
    LOG_PRINT(" \n set ae scan enable failed with 0x%lX \n", status);
  } else {
    scan_state_dut = connectable_scan;
    LOG_PRINT(" \n set ae scan enable success \n");
  }

#if WLAN_TRANSIENT_CASE
  ble_scanning_is_there = 1;
#endif

#endif
  rsi_change_ble_adv_and_scan_params();

  return 0;
}

void app_init(void)
{
  osThreadNew((osThreadFunc_t)rsi_ble_main_app_task, NULL, &thread_attributes);
}
