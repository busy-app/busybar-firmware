/*******************************************************************************
* @file  rsi_ble_gap_apis.c
* @brief
*******************************************************************************
* # License
* <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "ble_config.h"
#include "rsi_ble.h"
#include "rsi_utils.h"
#include "rsi_common.h"

/**
 * Structure for Power index and corresponding output power in db
 * The values are in dBm at Antenna
 * index 0 corresponds to -8 dBm , array size is 24
 */
uint8_t pwr_index_to_db_array[] = { 11, 13, 17, 23, 31, 44, 45, 47, 49, 52, 55, 59,
                                    63, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78 };

/**
 * Structure for Power index and corresponding output power in db for WMS module
 * The values are in dBm at Antenna
 * index 0 corresponds to -8 dBm , array size is 24
 */
uint8_t wms_pwr_index_to_db_array[] = { 8,  10, 13, 18, 29, 39, 40, 41, 42, 44, 45, 48,
                                        50, 54, 61, 70, 71, 72, 73, 74, 75, 76, 77, 78 };

/******************************************************
 *               Function Declarations
 ******************************************************/
int32_t rsi_ble_write_rf_path_compensation(uint16_t tx_path_value, uint16_t rx_path_value);
int32_t rsi_ble_read_rf_path_compensation(void *resp);

#if RSI_BLE_PWR_INX_DBM
/**
 * @fn         uint8_t rsi_convert_db_to_powindex(int8_t tx_power_in_dBm)
 * @brief      Convert power from dBm to power index.
 * @param[in]  tx_power_in_dBm - tx power in  dBm( -8 dBm to 15 dBm)
 * @return     power index to controller
 *
 */
uint8_t rsi_convert_db_to_powindex(int8_t tx_power_in_dBm)
{
  module_type mod_type_temp;
  int16_t status;
  SL_PRINTF(SL_RSI_CONVERT_DB_TO_POWERINDEX_TRIGGER, BLE, LOG_INFO);
  status = rsi_get_module_type(&mod_type_temp);
  if (status == RSI_SUCCESS) {
    tx_power_in_dBm += BLE_OUTPUT_POWER_FRONT_END_LOSS;
    if ((tx_power_in_dBm < RSI_MIN_OUTPUT_POWER_IN_DBM) || (tx_power_in_dBm > RSI_MAX_OUTPUT_POWER_IN_DBM)) {
      return 0;
    }
    /* Converting At antenna  dBm to On screen dBm  */
    tx_power_in_dBm -= RSI_MIN_OUTPUT_POWER_IN_DBM;
    switch (mod_type_temp) {
      case RSI_MODULE_TYPE_WMS: {
        return (wms_pwr_index_to_db_array[tx_power_in_dBm]);
        //no break
      }
      case RSI_MODULE_TYPE_Q7:
      case RSI_MODULE_TYPE_SB:
      case RSI_MODULE_TYPE_M7DB:
      case RSI_MODULE_TYPE_M4SB:
      default: {
        return (pwr_index_to_db_array[tx_power_in_dBm]);
        //no break
      }
    }
  }
  return 0;
}
#endif

/** @addtogroup BT-LOW-ENERGY1
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_random_address(void)
 * @brief      Request the local device to set a random address. This is a Blocking API
 * @pre        Device should be initialized before calling this API.
 * @param[in]  void
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_set_random_address(void)
{

  SL_PRINTF(SL_RSI_SET_RANDOM_TRIGGER, BLE, LOG_INFO);
  rsi_ble_req_rand_t ble_rand = { { 0 } };

  // Send set random address command
  return rsi_bt_driver_send_cmd(RSI_BLE_SET_RANDOM_ADDRESS, &ble_rand, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_random_address_with_value(uint8_t *random_addr)
 * @brief      Request the local device to set a given random address. This is a Blocking API
 * @pre        Device should be initialized before calling this API.
 * @param[in]  random_addr - random address of the device to be set
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_set_random_address_with_value(uint8_t *random_addr)
{

  SL_PRINTF(SL_RSI_SET_RANDOM_ADDRESS_WITH_VALUE_TRIGGER, BLE, LOG_INFO, "VALUE_ADDRESS: %1x", *random_addr);
  rsi_ble_req_rand_t ble_rand = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(ble_rand.rand_addr, random_addr);
#else
  memcpy((int8_t *)&(ble_rand.rand_addr), (int8_t *)random_addr, 6);
#endif

  // Send set random address command
  return rsi_bt_driver_send_cmd(RSI_BLE_SET_RANDOM_ADDRESS, &ble_rand, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_start_advertising(void)
 * @brief      Request the local device to start advertising. This is a Blocking API \n
 *             A received event \ref rsi_ble_on_enhance_connect_t/ \ref rsi_ble_on_connect_t indicates remote device given ble connect command and got connected
 * @pre        Device should be initialized before calling this API.
 * @param[in]  void
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command \n
 *             0x4E0C - Command disallowed \n
 *             0x4046 - Invalid Arguments  
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_start_advertising(void)
{

  SL_PRINTF(SL_RSI_BLE_START_ADV_TRIGGER, BLE, LOG_INFO);
  rsi_ble_req_adv_t ble_adv = { 0 };

  ble_adv.status = RSI_BLE_START_ADV;

  ble_adv.adv_type         = RSI_BLE_ADV_TYPE;
  ble_adv.filter_type      = RSI_BLE_ADV_FILTER_TYPE;
  ble_adv.direct_addr_type = RSI_BLE_ADV_DIR_ADDR_TYPE;
  rsi_ascii_dev_address_to_6bytes_rev(ble_adv.direct_addr, (int8_t *)RSI_BLE_ADV_DIR_ADDR);
  if (ble_adv.adv_int_min == 0) {
    ble_adv.adv_int_min = RSI_BLE_ADV_INT_MIN;
  }
  if (ble_adv.adv_int_max == 0) {
    ble_adv.adv_int_max = RSI_BLE_ADV_INT_MAX;
  }
  ble_adv.own_addr_type   = LE_PUBLIC_ADDRESS;
  ble_adv.adv_channel_map = RSI_BLE_ADV_CHANNEL_MAP;
  // Send start advertise command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_ADV, &ble_adv, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_start_advertising_with_values(void *rsi_ble_adv)
 * @brief      Request the local device to start advertising with specified values. This is a Blocking API \n
 *             A received event \ref rsi_ble_on_enhance_connect_t/ \ref rsi_ble_on_connect_t indicates remote device given ble connect command and got connected
 * @pre        Device should be initialized before calling this API.
 * @param[in]  rsi_ble_adv - This structure pointer holds the information of advertising values \n
 *                           This variable is the pointer of the \ref rsi_ble_req_adv_s structure
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command \n
 *             0x4E0C - Command disallowed \n
 *             0x4046 - Invalid Arguments  
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_start_advertising_with_values(const void *rsi_ble_adv)
{
  SL_PRINTF(SL_RSI_BLE_START_ADV_WITH_VALUES_TRIGGER, BLE, LOG_INFO);
  rsi_ble_req_adv_t ble_adv = { 0 };
  if (rsi_ble_adv != NULL) {
    memcpy(&ble_adv, rsi_ble_adv, sizeof(rsi_ble_req_adv_t));
  }

  ble_adv.status = RSI_BLE_START_ADV;

  // Send start advertise command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_ADV, &ble_adv, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_encrypt(uint8_t *key, uint8_t *data, uint8_t *resp)
 * @brief      Encrypt the plain text data fed by the user using the key provided.\n
 *             It uses the AES-128 bit block cypher alogo to generate encrypted data. Refer to Bluetooth Spec 5.0 for further details.
 * @pre        Device should be initialized before calling this API. This is a Blocking API
 * @param[in]  key - 16 Bytes key for Encryption of data.
 * @param[in]  data - 16 Bytes of Data request to encrypt.
 * @param[out] resp - Encrypted data
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_encrypt(const uint8_t *key, const uint8_t *data, uint8_t *resp)
{

  SL_PRINTF(SL_RSI_BLE_ENCRYPT_TRIGGER, BLE, LOG_INFO);
  rsi_ble_encrypt_t ble_en;
  memcpy(ble_en.key, key, 16);
  memcpy(ble_en.data, data, 16);

  // Send Encrypt command
  return rsi_bt_driver_send_cmd(RSI_BLE_ENCRYPT, &ble_en, resp);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_stop_advertising(void)
 * @brief      Stop advertising. This is a Blocking API
 * @pre        Call \ref rsi_ble_start_advertising() before calling this API.
 * @param[in]  void
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command \n
 *             0x4E0C - Command disallowed \n
 *             0x4046 - Invalid Arguments  
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_stop_advertising(void)
{

  SL_PRINTF(SL_RSI_BLE_STOP_ADV_TRIGGER, BLE, LOG_INFO);
  rsi_ble_req_adv_t ble_adv = { 0 };

  ble_adv.status = RSI_BLE_STOP_ADV;

  // Send stop advertise command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_ADV, &ble_adv, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_advertise_data(uint8_t *data, uint16_t data_len)
 * @brief      Set the advertising data. This is a Blocking API
 * @pre        Device should be initialized before calling this API.
 * @param[in]  data - Advertising data.
 * @param[in]  data_len - Total length of advertising data.
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 * @note       1. The maximum length of advertising data payload is 31 bytes. \n
 *             2. The basic format of advertising payload record contains length and data. \n
 */
int32_t rsi_ble_set_advertise_data(const uint8_t *data, uint16_t data_len)
{

  SL_PRINTF(SL_RSI_BLE_SET_ADV_DATA_TRIGGER, BLE, LOG_INFO);
  rsi_ble_req_adv_data_t ble_adv_data = { 0 };

  ble_adv_data.data_len = (uint8_t)(RSI_MIN(data_len, sizeof(ble_adv_data.adv_data)));
  memcpy(ble_adv_data.adv_data, data, ble_adv_data.data_len);

  // Send stop advertise command
  return rsi_bt_driver_send_cmd(RSI_BLE_SET_ADVERTISE_DATA, &ble_adv_data, NULL);
}

/*========================================================*/
/**
 * @fn         int32_t rsi_ble_set_scan_response_data(uint8_t *data, uint16_t data_len)
 * @brief      Request the local device to set the scan response data. This is a Blocking API \n
 * @pre        Device should be initialized before calling this API.
 * @param[in]  data - Data about to be sent
 * @param[in]  data_len - Length of data, which is about to be sent
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_set_scan_response_data(const uint8_t *data, uint16_t data_len)
{

  SL_PRINTF(SL_RSI_BLE_SET_SCAN_RESPONSE_DATA_TRIGGER, BLE, LOG_INFO);
  rsi_ble_req_scanrsp_data_t ble_scanrsp_data = { 0 };
  ble_scanrsp_data.data_len                   = (uint8_t)(RSI_MIN(data_len, sizeof(ble_scanrsp_data.scanrsp_data)));
  memcpy(ble_scanrsp_data.scanrsp_data, data, ble_scanrsp_data.data_len);

  return rsi_bt_driver_send_cmd(RSI_BLE_SET_SCAN_RESPONSE_DATA, &ble_scanrsp_data, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_start_scanning(void)
 * @brief      Start scanning. This is a Blocking API
 *             A received event \ref rsi_ble_on_adv_report_event_t indicates advertise report of remote device received.
 * @pre        Device should be initialized before calling this API.
 * @param[in]  void
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command \n
 *             0x4E0C - Command disallowed \n
 *             0x4046 - Invalid Arguments  
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_start_scanning(void)
{

  SL_PRINTF(SL_RSI_BLE_START_SCANNING_TRIGGER, BLE, LOG_INFO);
  rsi_ble_req_scan_t ble_scan = { 0 };

  ble_scan.status = RSI_BLE_START_SCAN;

  ble_scan.scan_type     = RSI_BLE_SCAN_TYPE;
  ble_scan.filter_type   = RSI_BLE_SCAN_FILTER_TYPE;
  ble_scan.scan_int      = LE_SCAN_INTERVAL;
  ble_scan.scan_win      = LE_SCAN_WINDOW;
  ble_scan.own_addr_type = LE_PUBLIC_ADDRESS;

  // Send start scanning command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_SCAN, &ble_scan, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_start_scanning_with_values(void *rsi_ble_scan_params)
 * @brief      Start scanning with values. This is a Blocking API \n
 *             A received event \ref rsi_ble_on_adv_report_event_t indicates advertise report of remote device received.
 * @pre        Device should be initialized before calling this API.
 * @param[in]  rsi_ble_scan_params - BLE scan parameters structure
 *             refer to rsi_ble_req_scan_s structure for more information.
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             0x4E0C - Command disallowed \n
 *             0x4046 - Invalid Arguments       
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).      
 */
int32_t rsi_ble_start_scanning_with_values(void *rsi_ble_scan_params)
{

  SL_PRINTF(SL_RSI_BLE_START_SCANNING_WITH_VALUES_TRIGGER, BLE, LOG_INFO);
  rsi_ble_req_scan_t ble_scan = { 0 };

  if (rsi_ble_scan_params != NULL) {
    memcpy(&ble_scan, (rsi_ble_req_scan_t *)rsi_ble_scan_params, sizeof(rsi_ble_req_scan_t));
  }

  ble_scan.status = RSI_BLE_START_SCAN;

  // Send start scanning command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_SCAN, &ble_scan, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_stop_scanning(void)
 * @brief      Stop scanning. This is a Blocking API
 * @pre        \ref rsi_ble_start_scanning() API needs to be called before this API.
 * @param[in]  void
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 *             0x4E0C - Command disallowed 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_stop_scanning(void)
{

  SL_PRINTF(SL_RSI_BLE_STOP_SCANNING_TRIGGER, BLE, LOG_INFO);
  rsi_ble_req_scan_t ble_scan = { 0 };

  ble_scan.status = RSI_BLE_STOP_SCAN;

  // Send stop scanning command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_SCAN, &ble_scan, NULL);
}
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_connect_with_params(uint8_t remote_dev_addr_type,
 *                                              int8_t *remote_dev_addr,
 *                                              uint16_t scan_interval,
 *                                              uint16_t scan_window,
 *                                              uint16_t conn_interval_max,
 *                                              uint16_t conn_interval_min,
 *                                              uint16_t conn_latency,
 *                                              uint16_t supervision_tout)
 * @brief      Connect to the remote BLE device with the user configured parameters. This is a blocking API. \n
 *             A received event \ref rsi_ble_on_enhance_connect_t / \ref rsi_ble_on_connect_t indicates that the connection successful and \n
 *             a received event \ref rsi_ble_on_disconnect_t indicates that connection failures have occurred.     
 * @note       If a connection can not be established, for example, the remote device has gone out of range, has entered into deep sleep, or is not advertising, \n
 *             the stack would try to connect forever. In this case, the application would not get an event related to the connection request. \n
 *             To recover from this situation, the application can implement a timeout and call rsi_ble_connect_cancel() to cancel the connection request. \n
 *             Subsequent calls of this command have to wait for the ongoing command to complete.
 * @pre        Device should be initialized before calling this API..
 * @param[in]  remote_dev_addr_type - AddressType - Specifies the type of the address mentioned in BD Address \n
 *                                  0 - Public Address \n
 *                                  1 - Random Address
 * @param[in]  remote_dev_addr - This parameter describes the device address of remote device
 * @param[in]  scan_interval - LE Scan Interval : N=0xXXXX \n
 *                             It is defined as the time interval from when the Controller
 *                             started its last LE scan until it \n
 *                             begins the subsequent LE scan. \n
 *                             Range: 0x0004 to 0x4000 \n
 *                             Time = N * 0.625 msec \n
 *                             Time Range: 2.5 msec to 10 . 24 seconds
 * @param[in]  scan_window - LE Scan Window : N=0xXXXX \n
 *                           Amount of time for the duration of the LE scan. LE_Scan_Window
 *                           must be less than or equal to LE_Scan_Interval \n
 *                           Range: 0x0004 to 0x4000 \n
 *                           Time = N * 0.625 msec \n
 *                           Time Range: 2.5 msec to 10 . 24 seconds
 * @param[in]  conn_interval_max - Max Connection Interval : N=0xXXXX \n
 *                                 Minimum value for the connection event interval, which must \n
 *                                 be greater than or equal to Conn_Interval_Min. \n
 *                                 Range: 0x0006 to 0x0C80 \n
 *                                 Time = N * 1.25 msec \n
 *                                 Time Range: 7.5 msec to 4 seconds. \n
 *                                 0x0000 - 0x0005 and 0x0C81 - 0xFFFF - Reserved for future use
 * @param[in]  conn_interval_min - Min Connection Interval : N=0xXXXX \n
 *                                 Minimum value for the connection event interval, which must
 *                                 be greater than or equal to Conn_Interval_Max. \n
 *                                 Range: 0x0006 to 0x0C80 \n
 *                                 Time = N * 1.25 msec \n
 *                                 Time Range: 7.5 msec to 4 seconds. \n
 *                                 0x0000 - 0x0005 and 0x0C81 - 0xFFFF - Reserved for future use
 * @param[in]  conn_latency - Connection Latency : N = 0xXXXX \n
 *                            Peripheral latency for the connection in number of connection events. \n
 *                            Range: 0x0000 to 0x01F4
 * @param[in]  supervision_tout - Supervision Timeout : N = 0xXXXX \n
 *                                Supervision timeout for the LE Link. \n
 *                                Range: 0x000A to 0x0C80 \n
 *                                Time = N * 10 msec \n
 *                                Time Range: 100 msec to 32 seconds \n
 *                                0x0000 - 0x0009 and 0x0C81 - 0xFFFF - Reserved for future use
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             0x4E0C - Command disallowed \n
 *             0x4046 - Invalid Arguments
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).  
 */

int32_t rsi_ble_connect_with_params(uint8_t remote_dev_addr_type,
                                    const int8_t *remote_dev_addr,
                                    uint16_t scan_interval,
                                    uint16_t scan_window,
                                    uint16_t conn_interval_max,
                                    uint16_t conn_interval_min,
                                    uint16_t conn_latency,
                                    uint16_t supervision_tout)
{
  SL_PRINTF(SL_RSI_BLE_CONNECT_WITH_PARAMS_TRIGGER, BLE, LOG_INFO);

  SL_PRINTF(SL_RSI_BLE_CONNECT_PARAMS_SCAN,
            BLE,
            LOG_INFO,
            "SCAN_INTERVAL: %2x, SCAN_WINDOW: %2x",
            scan_interval,
            scan_window);

  SL_PRINTF(SL_RSI_BLE_CONNECT_PARAMS_CONNECT,
            BLE,
            LOG_INFO,
            "CONN_INTERVAL_MAX: %2x, CONN_INTERVAL_MIN: %2x, CONN_LATENCY: %2x",
            conn_interval_max,
            conn_interval_min,
            conn_latency);

  //This statement is added only to resolve compilation warning, value is unchanged
  UNUSED_PARAMETER(conn_interval_min);
  rsi_ble_req_conn_t ble_conn = { 0 };

  ble_conn.dev_addr_type = remote_dev_addr_type;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(ble_conn.dev_addr, remote_dev_addr);
#else
  memcpy(ble_conn.dev_addr, remote_dev_addr, 6);
#endif
  ble_conn.le_scan_interval  = scan_interval;
  ble_conn.le_scan_window    = scan_window;
  ble_conn.conn_interval_min = conn_interval_max;
  ble_conn.conn_interval_max = conn_interval_max;
  ble_conn.conn_latency      = conn_latency;
  ble_conn.supervision_tout  = supervision_tout;

  // Send connect command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_CONN, &ble_conn, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_connect(uint8_t remote_dev_addr_type, int8_t *remote_dev_addr)
 * @brief      Connect to the remote BLE device. This is a blocking API. \n
 *             A received event \ref rsi_ble_on_enhance_connect_t/ \ref rsi_ble_on_connect_t indicates that the connection successful and \n
 *             a received event \ref rsi_ble_on_disconnect_t indicates that connection failures have occurred.
 * @note       If a connection can not be established, for example, the remote device has gone out of range, has entered into deep sleep, or is not advertising, \n
 *             the stack would try to connect forever. In this case, the application would not get an event related to the connection request. \n
 *             To recover from this situation, the application can implement a timeout and call rsi_ble_connect_cancel() to cancel the connection request. \n
 *             Subsequent calls of this command have to wait for the ongoing command to complete.
 * @pre        Device should be initialized before calling this API.
 * @param[in]  remote_dev_addr_type - This parameter describes the address type of the remote device
 * @param[in]  remote_dev_addr - This parameter describes the device address of the remote device
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n 
 *             0x4E0C - Command disallowed \n
 *             0x4046 - Invalid Arguments
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .            
 */

int32_t rsi_ble_connect(uint8_t remote_dev_addr_type, const int8_t *remote_dev_addr)
{

  SL_PRINTF(SL_RSI_BLE_CONNECT, BLE, LOG_INFO, "ADDRESS_TYPE: %2x", remote_dev_addr_type);
  rsi_ble_req_conn_t ble_conn = { 0 };

  ble_conn.dev_addr_type = remote_dev_addr_type;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(ble_conn.dev_addr, remote_dev_addr);
#else
  memcpy(ble_conn.dev_addr, remote_dev_addr, 6);
#endif
  ble_conn.le_scan_interval  = LE_SCAN_INTERVAL;
  ble_conn.le_scan_window    = LE_SCAN_WINDOW;
  ble_conn.conn_interval_min = CONNECTION_INTERVAL_MIN;
  ble_conn.conn_interval_max = CONNECTION_INTERVAL_MAX;
  ble_conn.conn_latency      = CONNECTION_LATENCY;
  ble_conn.supervision_tout  = SUPERVISION_TIMEOUT;

  // Send connect command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_CONN, &ble_conn, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_enhance_connect_with_params(void* ble_enhance_conn_params)
 * @brief      Connect to the remote BLE device with the user configured parameters.
 * @pre        Device should be initialized before calling this API.
 * @param[in]  ble_enhance_conn_params - BLE enhance connection parameter structure
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */
int32_t rsi_ble_enhance_connect_with_params(void *ble_enhance_conn_params)
{
  rsi_ble_req_enhance_conn_t ble_enhance_conn = { 0 };

  if (ble_enhance_conn_params != NULL) {
    memcpy(&ble_enhance_conn,
           (rsi_ble_req_enhance_conn_t *)ble_enhance_conn_params,
           sizeof(rsi_ble_req_enhance_conn_t));
  }
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_CONN_ENHANCE, &ble_enhance_conn, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_connect_cancel(int8_t *remote_dev_address)
 * @brief      Cancel the connection to the remote BLE device. This is a blocking API. \n
 *             A received event \ref rsi_ble_on_disconnect_t indicates disconnect complete.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address - This parameter describes the device address of the remote device
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             0x4E0C - Command disallowed \n
 *             0x4046 - Invalid Arguments \n
 *             0x4E02 - Unknown Connection Identifier \n
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_ble_connect_cancel(const int8_t *remote_dev_address)
{

  SL_PRINTF(SL_RSI_BLE_CONNECT_CANCEL, BLE, LOG_INFO);
  rsi_ble_req_disconnect_t ble_disconnect = { 0 };

#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(ble_disconnect.dev_addr, remote_dev_address);
#else
  memcpy(ble_disconnect.dev_addr, remote_dev_address, 6);
#endif
  ble_disconnect.type = BLE_CONNECT_CANCEL;

  // Send connect cancel command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_DISCONNECT, &ble_disconnect, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_disconnect(int8_t *remote_dev_address)
 * @brief      Disconnect with the remote BLE device. This is a Blocking API.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address - This parameter describes the device address of the remote device
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             0x4E0C - Command disallowed \n
 *             0x4D05  BLE socket not available \n
 *             0x4E62 	Invalid Parameters \n 
 *             0x4D04	BLE not connected \n 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).  
 */
int32_t rsi_ble_disconnect(const int8_t *remote_dev_address)
{
  SL_PRINTF(SL_RSI_BLE_DISCONNECT, BLE, LOG_INFO, "ADDRESS: %1x", *remote_dev_address);
  rsi_ble_req_disconnect_t ble_disconnect = { 0 };

#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(ble_disconnect.dev_addr, remote_dev_address);
#else
  memcpy(ble_disconnect.dev_addr, remote_dev_address, 6);
#endif
  ble_disconnect.type = BLE_DISCONNECT;
  // Send disconnect command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_DISCONNECT, &ble_disconnect, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_device_state(uint8_t *resp)
 * @brief      Get the local device state. This is a Blocking API. The state value is filled in "resp".
 * @pre        Device should be initialized before calling this API.
 * @param[out] resp - This is an output parameter which consists of local device state. \n
 *                    This is a 1-byte value. The possible states are described below \n
 *             BIT(0)    Advertising state \n
 *             BIT(1)    Scanning state \n
 *             BIT(2)    Initiating state \n
 *             BIT(3)    Connected state \n
 *             BIT(4)    Extended Advertising state \n
 *             BIT(5)    Extended Scanning state \n
 *             BIT(6)    Extended Initiating state \n
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_get_device_state(uint8_t *resp)
{
  SL_PRINTF(SL_RSI_BLE_GET_DEVICE_STATE, BLE, LOG_INFO);
  return rsi_bt_driver_send_cmd(RSI_BLE_GET_DEV_STATE, NULL, resp);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_smp_pairing_cap_data(rsi_ble_set_smp_pairing_capabilty_data_t *smp_pair_cap_data)
 * @brief      Set the SMP Pairing Capability of local device. This is a Blocking API
 * @pre        Device should be initialized before calling this API.
 * @param[in]  smp_pair_cap_data - This structure pointer holds the information of the SMP capability data values \n
 *             refer to rsi_ble_set_smp_pairing_capabilty_data structure for more information             
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_set_smp_pairing_cap_data(rsi_ble_set_smp_pairing_capabilty_data_t *smp_pair_cap_data)
{

  SL_PRINTF(SL_RSI_BLE_SET_SMP_PAIRING_CAP_DATA, BLE, LOG_INFO);
  return rsi_bt_driver_send_cmd(RSI_BLE_SET_SMP_PAIRING_CAPABILITY_DATA, smp_pair_cap_data, NULL);
}
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_local_irk_value(uint8_t *l_irk)
 * @brief      Set the IRK value to the local device. This is a Blocking API
 * @pre        Device should be initialized before calling this API.
 * @param[in]  l_irk -  l_irk  Pointer to local_irk
 * @return     0 - Success \n
 *             Non-Zero Value - Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_set_local_irk_value(const uint8_t *l_irk)
{

  SL_PRINTF(SL_RSI_BLE_SET_LOCAL_IRK_VALUE, BLE, LOG_INFO);
  rsi_ble_set_local_irk_t local_device_irk = { 0 };

  memcpy(local_device_irk.irk, l_irk, 16);

  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_SET_LOCAL_IRK, &local_device_irk, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_conn_param_resp(uint8_t *remote_dev_address, uint8_t status)
 * @brief      Give the response for the remote device connection parameter request. This is a Blocking API \n
 *             A received event \ref rsi_ble_on_conn_update_complete_t indicates connection update procedure is successful.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address 	- remote device address
 * @param[in]  status 			- accept or reject the connection parameters update request \n
 *                    			0 - ACCEPT, \n
					1 - REJECT \n
 * @return     0 - Success \n
 *             Non-Zero Value - Failure
 *             0x4E0C - Command disallowed \n
 *             0x4046 - Invalid Arguments \n
 *             0x4E02 - Unknown Connection Identifier 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 *
 */

int32_t rsi_ble_conn_param_resp(const uint8_t *remote_dev_address, uint8_t status)
{

  SL_PRINTF(SL_RSI_BLE_CONN_PARAM_RESPONSE, BLE, LOG_INFO, "STATUS: %1x", status);
  rsi_ble_cmd_conn_param_resp_t conn_param_resp_cmd = { 0 };

#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(conn_param_resp_cmd.dev_addr, remote_dev_address);
#else
  memcpy(conn_param_resp_cmd.dev_addr, remote_dev_address, 6);
#endif
  conn_param_resp_cmd.status = status;

  // Send connect cancel command
  return rsi_bt_driver_send_cmd(RSI_BLE_CONN_PARAM_RESP_CMD, &conn_param_resp_cmd, NULL);
}

/** @} */
/** @addtogroup BT-LOW-ENERGY1
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_smp_pair_request(uint8_t *remote_dev_address, uint8_t io_capability, uint8_t mitm_req)
 * @brief      Request the SMP pairing process with the remote device. This is a Blocking API \n
 *             A received event \ref rsi_ble_on_smp_request_t indicated remote device is given Security Request  and need to respond back with \ref rsi_ble_smp_pair_request \n
 *             A received event \ref rsi_ble_on_smp_response_t indicated remote device is given SMP Pair Request and need to respond back with \ref rsi_ble_smp_pair_response \n
 *             A received event \ref rsi_ble_on_smp_failed_t indicated SMP procedure have failed 
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address -  This is the remote device address
 * @param[in]  io_capability - This is the device input output capability \n
 *                            0x00 - Display Only \n
 *                            0x01 - Display Yes/No \n
 *                            0x02 - Keyboard Only \n
 *                            0x03 - No Input No Output
 * @param[in]  mitm_req - MITM enable/disable \n
 *                       0 - Disable \n
 *                       1 - Enable
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command \n
 *              0x4D05  BLE socket not available \n
 *              0x4E62 	Invalid Parameters \n 
 *              0x4D04	BLE not connected \n 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_smp_pair_request(uint8_t *remote_dev_address, uint8_t io_capability, uint8_t mitm_req)
{

  SL_PRINTF(SL_RSI_BLE_SMP_PAIR_REQUEST, BLE, LOG_INFO, "MITM_REQ: %1x", mitm_req);
  rsi_ble_req_smp_pair_t smp_pair_req;
  memset(&smp_pair_req, 0, sizeof(smp_pair_req));

#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(smp_pair_req.dev_addr, remote_dev_address);
#else
  memcpy(smp_pair_req.dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  smp_pair_req.io_capability = io_capability;
  smp_pair_req.mitm_req      = mitm_req;

  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_SMP_PAIR, &smp_pair_req, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_smp_pair_failed(uint8_t *remote_dev_address ,uint8_t reason)
 * @brief      Send SMP pairing failure reason to the remote device.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address -  This is the remote device address
 * @param[in]  reason - This is the reason for SMP Pairing Failure \n
 *                            0x05 - Pairing Not Supported \n
 *                            0x09 - Repeated Attempts \n
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */
int32_t rsi_ble_smp_pair_failed(uint8_t *remote_dev_address, uint8_t reason)
{

  rsi_ble_req_smp_pair_failed_t smp_pair_failed;
  memset(&smp_pair_failed, 0, sizeof(smp_pair_failed));

#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(smp_pair_failed.dev_addr, remote_dev_address);
#else
  memcpy(smp_pair_failed.dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  smp_pair_failed.reason = reason;

  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_SMP_PAIRING_FAILED, &smp_pair_failed, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_ltk_req_reply(uint8_t *remote_dev_address,
 *                                           uint8_t reply_type, uint8_t *ltk)
 * @brief      Send the local long term key of its associated local EDIV and local Rand. This is a Blocking API \n
 *             A received event \ref rsi_ble_on_encrypt_started_t indicated encrypted event is received from module \n
 *             A received event \ref rsi_ble_on_smp_failed_t indicated SMP procedure have failed 
 * @param[in]  remote_dev_address - remote device address
 * @param[in]  reply_type  - 0 - Negative reply \n
 *                      BIT(0) - Positive Reply (Encryption Enabled)\n
			BIT(1) - Un authenticated LTK or STK based Encyption Enabled \n
			BIT(2) - Authenticated LTK or STK based Encyption Enabled \n
			BIT(3) - Auntenticated LTK with LE Secure Connections based Encryption Enabled \n
			BIT(4) to BIT(6) - Reserved for Future use \n
			BIT(7) - LE Secure Connection Enabled \n
 * @param[in]  ltk - Long Term Key 16 bytes
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command \n
 *              0x4D05  BLE socket not available \n
 *              0x4E62 	Invalid Parameters \n 
 *              0x4D04	BLE not connected \n 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_ltk_req_reply(uint8_t *remote_dev_address, uint8_t reply_type, const uint8_t *ltk)
{

  SL_PRINTF(SL_RSI_BLE_LTK_REQ_REPLY, BLE, LOG_INFO, "REPLY_TYPE: %1x", reply_type);
  rsi_ble_set_le_ltkreqreply_t le_ltk_req_reply;
  memset(&le_ltk_req_reply, 0, sizeof(le_ltk_req_reply));

#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(le_ltk_req_reply.dev_addr, remote_dev_address);
#else
  memcpy(le_ltk_req_reply.dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  le_ltk_req_reply.replytype = reply_type;
  if (ltk != NULL) {
    memcpy(le_ltk_req_reply.localltk, ltk, 16);
  }

  return rsi_bt_driver_send_cmd(RSI_BLE_LE_LTK_REQ_REPLY, &le_ltk_req_reply, NULL);
}
/** @} */

/** @addtogroup BT-LOW-ENERGY1
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_smp_pair_response(uint8_t *remote_dev_address, uint8_t io_capability, uint8_t mitm_req)
 * @brief      Send SMP pairing response during the process of pairing with the remote device. This is a Blocking API \n
 *             A received event \ref rsi_ble_on_smp_passkey_t indicated Legacy SMP passkey is received and need to respond back with \ref rsi_ble_smp_passkey() \n
 *             A received event \ref rsi_ble_on_sc_passkey_t indicated BLE SC passkey is received and need to respond back with \ref rsi_ble_smp_passkey() \n
 *             A received event \ref rsi_ble_on_smp_passkey_display_t indicates SMP passkey display is received from the module \n
 *             A received event \ref rsi_ble_on_smp_failed_t indicated SMP Failed event is received 
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address -  This is the remote device address
 * @param[in]  io_capability - This is the device input output capability \n
 *                            0x00 - Display Only \n
 *                            0x01 - Display Yes/No \n
 *                            0x02 - Keyboard Only \n
 *                            0x03 - No Input No Output
 * @param[in]  mitm_req -  MITM Request info \n
 *                        0 - Disable \n
 *                        1 - Enable
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command \n
 *              0x4D05  BLE socket not available \n
 *              0x4E62 	Invalid Parameters \n 
 *              0x4D04	BLE not connected \n 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */

int32_t rsi_ble_smp_pair_response(uint8_t *remote_dev_address, uint8_t io_capability, uint8_t mitm_req)
{

  SL_PRINTF(SL_RSI_BLE_SMP_PAIR_RESPONSE, BLE, LOG_INFO, "MITM_REQ: %1x", mitm_req);
  rsi_ble_smp_response_t smp_pair_resp;
  memset(&smp_pair_resp, 0, sizeof(smp_pair_resp));

#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(smp_pair_resp.dev_addr, remote_dev_address);
#else
  memcpy(smp_pair_resp.dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  smp_pair_resp.io_capability = io_capability;
  smp_pair_resp.mitm_req      = mitm_req;

  return rsi_bt_driver_send_cmd(RSI_BLE_SMP_PAIR_RESPONSE, &smp_pair_resp, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_smp_passkey(uint8_t *remote_dev_address, uint32_t passkey)
 * @brief      Send SMP passkey during SMP pairing process with the remote device. This is a Blocking API \n
 *             A received event \ref rsi_ble_on_encrypt_started_t indicated encrypted event is received from module \n
 *             A received event \ref rsi_ble_on_le_security_keys_t indicates exchange of security keys completed after encryption \n
 *             A received event \ref rsi_ble_on_smp_failed_t indicated SMP procedure have failed  
 * @pre        Call \ref rsi_ble_smp_pair_request and \ref rsi_ble_smp_pair_response
 *             before calling this API.
 * @param[in]  remote_dev_address -  This is the remote device address
 * @param[in]  passkey - This is the key required in pairing process
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command \n
 *              0x4D05  BLE socket not available \n
 *              0x4E62 	Invalid Parameters \n 
 *              0x4D04	BLE not connected \n 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_smp_passkey(uint8_t *remote_dev_address, uint32_t passkey)
{

  SL_PRINTF(SL_RSI_BLE_SMP_PASSKEY, BLE, LOG_INFO, "PASSKEY: %4x", passkey);
  rsi_ble_smp_passkey_t smp_passkey;
  memset(&smp_passkey, 0, sizeof(smp_passkey));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(smp_passkey.dev_addr, remote_dev_address);
#else
  memcpy(smp_passkey.dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  smp_passkey.passkey = passkey;
  return rsi_bt_driver_send_cmd(RSI_BLE_SMP_PASSKEY, &smp_passkey, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_le_ping_timeout(uint8_t *remote_dev_address, uint16_t *time_out)
 * @brief      Get the timeout value of the LE ping. This is a Blocking API.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address - This is the remote device address
 * @param[out] time_out - This a response parameter which holds timeout value for \n
 *                        authentication payload command.
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command \n
 *              0x4D05  BLE socket not available \n
 *              0x4E62 	Invalid Parameters \n 
 *              0x4D04	BLE not connected \n 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 * @note       Currently Get ping is not supported.
 */

int32_t rsi_ble_get_le_ping_timeout(uint8_t *remote_dev_address, uint16_t *time_out)
{
  uint16_t status;

  SL_PRINTF(SL_RSI_BLE_GET_LE_PING_TIMEOUT, BLE, LOG_INFO);
  rsi_ble_get_le_ping_timeout_t leping_cmd;
  rsi_ble_rsp_get_le_ping_timeout_t le_ping_rsp;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(leping_cmd.dev_addr, remote_dev_address);
#else
  memcpy(leping_cmd.dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  status = (uint16_t)(rsi_bt_driver_send_cmd(RSI_BLE_GET_LE_PING, &leping_cmd, &le_ping_rsp));
  if (status == 0) {
    *time_out = le_ping_rsp.time_out;
  }

  return status;
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_le_ping_timeout(uint8_t *remote_dev_address, uint16_t time_out)
 * @brief      Set the timeout value of the LE ping. This is a Blocking API \n
 *             A received event of \ref rsi_ble_on_le_ping_payload_timeout_t indicates le ping payload timeout expired
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address -  This is the remote device address
 * @param[out] timeout - This input parameter sets timeout value for authentication \n
 *                       payload command.(in milliseconds)
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command \n
 *              0x4D05  BLE socket not available \n
 *              0x4E62 	Invalid Parameters \n 
 *              0x4D04	BLE not connected \n 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_set_le_ping_timeout(uint8_t *remote_dev_address, uint16_t time_out)
{

  SL_PRINTF(SL_RSI_BLE_SET_LE_PING_TIMEOUT, BLE, LOG_INFO, "TIMEOUT: %2x", time_out);
  rsi_ble_set_le_ping_timeout_t leping_cmd;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(leping_cmd.dev_addr, remote_dev_address);
#else
  memcpy(leping_cmd.dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  leping_cmd.time_out = time_out;

  return rsi_bt_driver_send_cmd(RSI_BLE_SET_LE_PING, &leping_cmd, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_clear_acceptlist(void)
 * @brief      Clear all the BD address present in accept list. This is a Blocking API
 * @pre        Device should be initialized before calling this API.
 * @param[in]  void
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_clear_acceptlist(void)
{
  SL_PRINTF(SL_RSI_BLE_CLEAR_ACCEPTLIST_TRIGGER, BLE, LOG_INFO);
  rsi_ble_accept_list_t le_acceptlist = { 0 };
  le_acceptlist.addordeltowhitlist    = CLEAR_ACCEPTLIST;

  return rsi_bt_driver_send_cmd(RSI_BLE_LE_ACCEPT_LIST, &le_acceptlist, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_addto_acceptlist(int8_t *dev_address, uint8_t dev_addr_type)
 * @brief      Add BD address to accept list. This is a Blocking API.
 * @pre        Device should be initialized before calling this API.
 * @param[in]  dev_address - Address of the device which is going to add in accept list
 * @param[in]  dev_addr_type - address type of BD address
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 * @note       Maximum number of device address that firmware can store is 10. \n Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_addto_acceptlist(const int8_t *dev_address, uint8_t dev_addr_type)
{

  SL_PRINTF(SL_RSI_BLE_ADD_TO_ACCEPTLIST, BLE, LOG_INFO, "DEVICE_ADDRESS_TYPE: %1x", dev_addr_type);
  rsi_ble_accept_list_t le_acceptlist = { 0 };
  le_acceptlist.addordeltowhitlist    = ADD_DEVICE_TO_ACCEPTLIST;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(le_acceptlist.dev_addr, dev_address);
#else
  memcpy(le_acceptlist.dev_addr, dev_address, 6);
#endif
  le_acceptlist.bdaddressType = dev_addr_type;

  return rsi_bt_driver_send_cmd(RSI_BLE_LE_ACCEPT_LIST, &le_acceptlist, NULL);
}
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_deletefrom_acceptlist(int8_t *dev_address, uint8_t dev_addr_type)
 * @brief      Delete particular BD address from accept list. This is a Blocking API.
 * @pre        \ref rsi_ble_addto_acceptlist() API needs to be called before this API.
 * @param[in]  dev_address - Address of the device which is going to delete from accept list
 * @param[in]  dev_addr_type - address type of BD address
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_deletefrom_acceptlist(const int8_t *dev_address, uint8_t dev_addr_type)
{

  SL_PRINTF(SL_RSI_BLE_DELETEFROM_ACCEPTLIST, BLE, LOG_INFO, "DEVICE_ADDRESS_TYPE: %1x", dev_addr_type);
  rsi_ble_accept_list_t le_acceptlist = { 0 };
  le_acceptlist.addordeltowhitlist    = DELETE_DEVICE_FROM_ACCEPTLIST;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(le_acceptlist.dev_addr, dev_address);
#else
  memcpy(le_acceptlist.dev_addr, dev_address, 6);
#endif
  le_acceptlist.bdaddressType = dev_addr_type;

  return rsi_bt_driver_send_cmd(RSI_BLE_LE_ACCEPT_LIST, &le_acceptlist, NULL);
}

/*==============================================*/
/**
 * @fn        int32_t rsi_ble_resolvlist(uint8_t process_type,
 *                                      uint8_t remote_dev_addr_type,
 *                                      uint8_t *remote_dev_address,
 *                                      uint8_t *peer_irk,
 *                                      uint8_t *local_irk)
 * @brief     resolvlist api used for multiple purpose based on the process type. It will be used to add/remove/clear a device to/from the list. This is a Blocking API
 * @pre       Device should be initialized before calling this API.
 * @param[in] process_type - Indicates which type of process this is, as follows: \n
 *                          1 - add a device to the resolve list \n
 *                          2 - remove a device from the resolve list \n
 *                          3 - clear the entire resolve list
 * @param[in] remote_dev_addr_type 	- typr of the remote device address 
 * @param[in] remote_dev_address 	- remote device address \n
					0 - Public identity address \n
					1 - Random (static) identity address \n
 * @param[in] peer_irk 			- 16-byte IRK of the peer device
 * @param[in] local_irk 		- 16-byte IRK of the local device
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_resolvlist(uint8_t process_type,
                           uint8_t remote_dev_addr_type,
                           uint8_t *remote_dev_address,
                           const uint8_t *peer_irk,
                           const uint8_t *local_irk)
{

  SL_PRINTF(SL_RSI_BLE_RESOLVLIST,
            BLE,
            LOG_INFO,
            "PROCESS_TYPE: %1x, REMOTE_DEVICE_ADDRESS_TYPE: %1x",
            process_type,
            remote_dev_addr_type);
  rsi_ble_resolvlist_t resolvlist;

  resolvlist.process_type         = process_type;
  resolvlist.remote_dev_addr_type = remote_dev_addr_type;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(resolvlist.remote_dev_addr, remote_dev_address);
#else
  memcpy(resolvlist.remote_dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  memcpy(resolvlist.peer_irk, peer_irk, 16);
  memcpy(resolvlist.local_irk, local_irk, 16);
  return rsi_bt_driver_send_cmd(RSI_BLE_PROCESS_RESOLV_LIST, &resolvlist, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_resolving_list_size(uint8_t *resp)
 * @brief      Request to get resolving list size. This is a Blocking API.
 * @pre        Device should be initialized before calling this API.
 * @param[out] resp - output parameter which consists of supported resolving the list size.
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 : Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_get_resolving_list_size(uint8_t *resp)
{

  SL_PRINTF(SL_RSI_BLE_GET_RESOLVING_LIST_SIZE, BLE, LOG_INFO);
  return rsi_bt_driver_send_cmd(RSI_BLE_GET_RESOLVING_LIST_SIZE, NULL, resp);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_addr_resolution_enable(uint8_t enable, uint16_t tout)
 * @brief      Request to enable address resolution, and to set resolvable private address timeout. This is a Blocking API
 * @pre        Device should be initialized before calling this API.
 * @param[in]  enable - value to enable/disable address resolution \n
			1 - enables address resolution \n
 *                      0 - disables address resolution
 * @param[in]  tout -  the period for changing address of our local device in seconds \n
			Value ranges from 0x0001 to 0xA1B8 (1s to approximately 11.5 hours)
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 : Buffer not available to serve the command
 */

int32_t rsi_ble_set_addr_resolution_enable(uint8_t enable, uint16_t tout)
{

  SL_PRINTF(SL_RSI_BLE_SET_ADDR_RESOULTION_ENABLE, BLE, LOG_INFO);
  rsi_ble_set_addr_resolution_enable_t addr_resolution_enable;

  addr_resolution_enable.enable = enable;
  addr_resolution_enable.tout   = tout;

  return rsi_bt_driver_send_cmd(RSI_BLE_SET_ADDRESS_RESOLUTION_ENABLE, &addr_resolution_enable, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_privacy_mode(uint8_t remote_dev_addr_type,
 *                                              uint8_t *remote_dev_address, uint8_t privacy_mode)
 * @brief      Request to set privacy mode for particular device. This is a Blocking API
 * @pre        Device should be initialized before calling this API.
 * @param[in]  remote_dev_addr_type - type of the remote device address \n
 *                                    0 - Public Identity Address \n
 *                                    1 - Random (static) Identity Address
 * @param[in]  remote_dev_address   - remote device address
 * @param[in]  privacy_mode 	    - type of the privacy mode \n
					0 - Network privacy mode \n
 *                            		1 - Device privacy mode
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 : Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */

int32_t rsi_ble_set_privacy_mode(uint8_t remote_dev_addr_type, uint8_t *remote_dev_address, uint8_t privacy_mode)
{
  SL_PRINTF(SL_RSI_BLE_SET_PRIVACY_MODE, BLE, LOG_INFO, "REMOTE_DEVICE_ADDRESS_TYPE: %1x", remote_dev_addr_type);
  rsi_ble_set_privacy_mode_t set_privacy_mode;
  set_privacy_mode.remote_dev_addr_type = remote_dev_addr_type;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(set_privacy_mode.remote_dev_addr, remote_dev_address);
#else
  memcpy(set_privacy_mode.remote_dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  set_privacy_mode.privacy_mode = privacy_mode;
  return rsi_bt_driver_send_cmd(RSI_BLE_SET_PRIVACY_MODE, &set_privacy_mode, NULL);
}
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_readphy(int8_t *remote_dev_address, rsi_ble_resp_read_phy_t *resp)
 * @brief      Reads the TX and RX PHY rates of the Connection. This is a Blocking API.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address - remote device address
 * @param[out] resp - pointer to store the response 
		please refer \ref rsi_ble_resp_read_phy_s structure for more info.
 * @return     0 - Success \n
 *             Non-Zero Value - Failure 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_readphy(const int8_t *remote_dev_address, rsi_ble_resp_read_phy_t *resp)
{

  SL_PRINTF(SL_RSI_BLE_READPHY, BLE, LOG_INFO);
  rsi_ble_req_read_phy_t ble_read_phy = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(ble_read_phy.dev_addr, remote_dev_address);
#else
  memcpy(ble_read_phy.dev_addr, remote_dev_address, 6);
#endif

  // Send connect cancel command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_READ_PHY, &ble_read_phy, resp);
}
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_setphy(int8_t *remote_dev_address, uint8_t tx_phy, uint8_t rx_phy,
 *                                    uint16_t coded_phy)
 * @brief      Set TX and RX PHY. This is a Blocking API \n
 *             A received event \ref rsi_ble_on_phy_update_complete_t indicates phy rate update complete.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address - remote device address \n
 * @param[in]  tx_phy - transmit phy rate \n
			BIT(0) - Host prefers to use the LE 1M transmitter PHY (possibly among others) \n
 *                      BIT(1) - Host prefers to use the LE 2M transmitter PHY (possibly among others) \n
 *                      BIT(2) - Host prefers to use the LE Coded transmitter PHY (possibly among others) \n
 *                      BIT(3) - BIT(7) Reserved for future use \n
 * @param[in]  rx_phy - receive phy rate \n
			BIT(0) - Host prefers to use the LE 1M receiver PHY (possibly among others) \n
 *                      BIT(1) - Host prefers to use the LE 2M receiver PHY (possibly among others) \n
 *                      BIT(2) - Host prefers to use the LE Coded receiver PHY (possibly among others) \n
 *                      BIT(3) - BIT(7) Reserved for future use \n
 * @param[in]  coded_phy - TX/RX coded phy rate \n
			   0 = Host has no preferred coding when transmitting on the LE Coded PHY \n
 *                         1 = Host prefers that S=2 coding be used when transmitting on the LE Coded PHY \n
 *                         2 = Host prefers that S=8 coding be used when transmitting on the LE Coded PHY \n
 *                         3 = Reserved for future use \n
 * @return     0   -  Success \n
 *             Non-Zero Value - Failure \n
 *              0x4D05  BLE socket not available \n
 *              0x4E62 	Invalid Parameters \n 
 *              0x4D04	BLE not connected \n 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_setphy(const int8_t *remote_dev_address, uint8_t tx_phy, uint8_t rx_phy, uint16_t coded_phy)
{

  SL_PRINTF(SL_RSI_SETPHY_TRIGGER, BLE, LOG_INFO);
  rsi_ble_set_phy_t set_phy;
  memset(&set_phy, 0, sizeof(set_phy));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(set_phy.dev_addr, remote_dev_address);
#else
  memcpy(set_phy.dev_addr, remote_dev_address, 6);
#endif
  set_phy.all_phy     = ALL_PHYS;
  set_phy.tx_phy      = tx_phy;
  set_phy.rx_phy      = rx_phy;
  set_phy.phy_options = coded_phy;

  // Send connect cancel command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_SET_PHY, &set_phy, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_conn_params_update(uint8_t *remote_dev_address,
 *                                             uint16_t min_int,
 *                                             uint16_t max_int,
 *                                             uint16_t latency,
 *                                             uint16_t timeout)
 * @brief      	Requests the connection parameters change with the remote device.\n 
 *				When the Silabs device is acting as a central, this API is used to update the connection parameters. \n
 * 				When the Silabs device is acts as a peripheral, this API is used to request the central to initiate the connection update procedure. This is a Blocking API \n
 *              A received event \ref rsi_ble_on_conn_update_complete_t indicates connection parameters update complete.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address 	- remote device address
 * @param[in]  min_int 			- minimum value for the connection interval. \n
 *                       		  this shall be less than or equal to max_int .
 * @param[in]  max_int 			- maximum value for the connection interval. \n
 *                       		  this shall be greater than or equal to min_int.
 * @param[in]  latency 			- peripheral latency for the connection in number of connection events.\n
 *					  Ranges from 0 to 499
 * @param[in]  timeout 			- supervision timeout for the LE Link. \n
 *                       		  Ranges from 10 to 3200 (Time = N * 10 ms, Time Range: 100 ms to 32 s)
 * @return     0   -  Success \n
 *             Non-Zero Value - Failure \n
 *              0x4D05  BLE socket not available \n
 *              0x4E62 	Invalid Parameters \n 
 *              0x4D04	BLE not connected \n 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 * @note       min_int and max_int values ranges from 6 to 3200 (Time = N * 1.25 ms, Time Range: 7.5 ms to 4 s)\n
		latency : If latency value is greater than 32 ,Limiting the peripheral latency value to 32\n
		Max supported peripheral latency is 32 when Device is in Peripheral Role.
 *
 */
int32_t rsi_ble_conn_params_update(const uint8_t *remote_dev_address,
                                   uint16_t min_int,
                                   uint16_t max_int,
                                   uint16_t latency,
                                   uint16_t timeout)
{

  SL_PRINTF(SL_RSI_BLE_CONN_PARMS_UPDATE,
            BLE,
            LOG_INFO,
            "MIN_INTERVAL: %2x, MAX_INTERVAL: %2x, LATENCY: %2x",
            min_int,
            max_int,
            latency);
  rsi_ble_cmd_conn_params_update_t conn_params_update_cmd = { 0 };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(conn_params_update_cmd.dev_addr, remote_dev_address);
#else
  memcpy(conn_params_update_cmd.dev_addr, remote_dev_address, 6);
#endif
  conn_params_update_cmd.min_interval = min_int;
  conn_params_update_cmd.max_interval = max_int;
  conn_params_update_cmd.latency      = latency;
  conn_params_update_cmd.timeout      = timeout;

  // Send connect cancel command
  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_CONN_PARAMS_UPDATE, &conn_params_update_cmd, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_data_len(uint8_t *remote_dev_address, uint16_t tx_octets, uint16_t tx_time)
 * @brief      Sets the TX octets and the TX time of specified link (remote device connection). This is a Blocking API. \n
 *             A received event \ref rsi_ble_on_data_length_update_t indicates data length update complete.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address 	- remote device device \n
 * @param[in]  tx_octets 		- preferred maximum number of payload octets that the local Controller \n
 *                         		  should include in a single Link Layer packet on this connection.
 * @param[in]  tx_time 			- preferred maximum number of microseconds that the local Controller \n
 *                       		  should use to transmit a single Link Layer packet on this connection.
 * @return     0 - LE_Set_Data_Length command succeeded. \n
 *             Non-Zero Value - Failure \n
 *              0x4D05  BLE socket not available \n
 *              0x4E62 	Invalid Parameters \n 
 *              0x4D04	BLE not connected \n 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 *
 */

int32_t rsi_ble_set_data_len(uint8_t *remote_dev_address, uint16_t tx_octets, uint16_t tx_time)
{

  SL_PRINTF(SL_RSI_BLE_SET_DATA_LEN, BLE, LOG_INFO);
  rsi_ble_setdatalength_t lesetdatalen;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(lesetdatalen.dev_addr, remote_dev_address);
#else
  memcpy(lesetdatalen.dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  lesetdatalen.txoctets = tx_octets;
  lesetdatalen.txtime   = tx_time;

  return rsi_bt_driver_send_cmd(RSI_BLE_SET_DATA_LEN, &lesetdatalen, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_read_max_data_len(rsi_ble_read_max_data_length_t *blereaddatalen)
 * @brief      reads the max supported values of TX octets, TX time, RX octets and Rx time. This is a Blocking API
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[out]  blereaddatalen - pointer to structure variable,
		 Please refer rsi_ble_resp_read_max_data_length_s structure for more information.
 * @return     0 - command success \n
 *             Non-Zero Value - Failure 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_read_max_data_len(rsi_ble_read_max_data_length_t *blereaddatalen)
{

  SL_PRINTF(SL_RSI_BLE_READ_MAX_DATA_LEN_TRIGGER, BLE, LOG_INFO);
  return rsi_bt_driver_send_cmd(RSI_BLE_READ_MAX_DATA_LEN, NULL, blereaddatalen);
}
/** @} */

/** @addtogroup BT-LOW-ENERGY6
* @{
*/

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_rx_test_mode(uint8_t rx_channel, uint8_t phy, uint8_t modulation)
 * @brief      Start the BLE RX test mode in controller. This is a Blocking API.
 * @param[in]  rx_channel - Channel in which packet have to be received (0 - 39)
 * @param[in]  phy - 0x00  Reserved for future use \n
 *                   0x01  Receiver set to use the LE 1M PHY \n
 *                   0x02  Receiver set to use the LE 2M PHY \n
 *                   0x03  Receiver set to use the LE Coded PHY \n
 *                   (0x04 - 0xFF)   Reserved for future use.
 * @param[in]  modulation - 0x00  Assume transmitter will have a standard standard modulation index \n
 *                          0x01  Assume transmitter will have a stable modulation index \n
 *                          (0x02 - 0xFF)  Reserved for future use
 * @return     0 - Success \n
 *             Non-Zero Value - Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_rx_test_mode(uint8_t rx_channel, uint8_t phy, uint8_t modulation)
{

  SL_PRINTF(SL_RSI_BLE_RX_TEST_MODE_TRIGGER, BLE, LOG_INFO);
  rsi_ble_rx_test_mode_t rx_test_mode;

  rx_test_mode.rx_channel = rx_channel;
  rx_test_mode.phy        = phy;
  rx_test_mode.modulation = modulation;

  return rsi_bt_driver_send_cmd(RSI_BLE_RX_TEST_MODE, &rx_test_mode, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_tx_test_mode(uint8_t tx_channel, uint8_t phy,
 *                                          uint8_t tx_len, uint8_t mode)
 * @brief      Start the BLE TX test mode in controller. This is a Blocking API
 * @param[in]  tx_channel -  RF Channel (0-39). \n
 * @param[in]  phy - 0x00  Reserved for future use \n
 *                   0x01  Transmitter set to use the LE 1M PHY \n
 *                   0x02  Transmitter set to use the LE 2M PHY \n
 *                   0x03  Transmitter set to use the LE Coded PHY with S=8 data coding \n
 *                   0x04  Transmitter set to use the LE Coded PHY with S=2 data coding \n
 *                   (0x05 - 0xFF)  Reserved for future use.
 * @param[in] tx_len - Length in bytes of payload data in each packet ( 1 - 251 bytes).
 * @param[in] mode - 0x00  PRBS9 sequence '11111111100000111101...' \n
 *                   0x01  Repeated '11110000' \n
 *                   0x02  Repeated '10101010' \n
 *                   0x03  PRBS15 \n
 *                   0x04  Repeated '11111111' \n
 *                   0x05  Repeated '00000000' \n
 *		     0x06  Repeated '00001111' \n
 *		     0x07  Repeated '01010101' \n
 *                   0x08 - 0xFF Reserved for future use \n
 * @return     0 - Success \n
 *             Non-Zero Value - Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_tx_test_mode(uint8_t tx_channel, uint8_t phy, uint8_t tx_len, uint8_t mode)
{

  SL_PRINTF(SL_RSI_BLE_TX_TEST_MODE_TRIGGER, BLE, LOG_INFO);
  rsi_ble_tx_test_mode_t tx_test_mode;

  tx_test_mode.tx_channel   = tx_channel;
  tx_test_mode.phy          = phy;
  tx_test_mode.tx_len       = tx_len;
  tx_test_mode.tx_data_mode = mode;

  return rsi_bt_driver_send_cmd(RSI_BLE_TX_TEST_MODE, &tx_test_mode, NULL);
}

/**
 * @fn        int32_t rsi_ble_end_test_mode(uint16_t *num_of_pkts)
 * @brief     Stop the BLE TX / RX test mode in controller. This is a Blocking API.
 * @param[out] num_of_pkts - Number of RX packets received are displayed when RX test is stopped \n
 * @return     0 - Success \n
 *             Non-Zero Value - Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_end_test_mode(uint16_t *num_of_pkts)
{
  SL_PRINTF(SL_RSI_BLE_END_TEST_MODE_TRIGGER, BLE, LOG_INFO);
  return rsi_bt_driver_send_cmd(RSI_BLE_END_TEST_MODE, NULL, num_of_pkts);
}
/** @} */

/** @addtogroup BT-LOW-ENERGY6
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_per_transmit(struct rsi_ble_per_transmit_s *rsi_ble_per_tx)
 * @brief      Initiate the BLE transmit PER mode in the controller. This is a Blocking API
 * @pre        Device should be initialized before calling this API.
 * @param[in]  rsi_ble_per_tx - This parameter is the buffer to hold the structure values \n
 *                            This is a structure variable of struct \ref rsi_ble_per_transmit_s
 * @return     0 - Success \n
 *             Non-Zero Value - Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_per_transmit(struct rsi_ble_per_transmit_s *rsi_ble_per_tx)
{
  SL_PRINTF(SL_RSI_BLE_INIT_PER_TRANSMIT_MODE_TRIGGER, BLE, LOG_INFO);
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_PER_CMD, rsi_ble_per_tx, NULL);
}

/**
 * @fn         int32_t rsi_ble_per_receive(struct rsi_ble_per_receive_s *rsi_ble_per_rx)
 * @brief      Initiate the BLE receive PER mode in the controller. This is a Blocking API.
 * @pre        Device should be initialized before calling this API.
 * @param[in]  rsi_ble_per_rx - This parameter is the buffer to hold the structure values \n
 *             This is a structure variable of struct \ref rsi_ble_per_receive_s
 * @return     0 - Success \n
 *             Non-Zero Value - Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_per_receive(struct rsi_ble_per_receive_s *rsi_ble_per_rx)
{

  SL_PRINTF(SL_RSI_BLE_PER_RECIEVE_TRIGGER, BLE, LOG_INFO);
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_PER_CMD, rsi_ble_per_rx, NULL);
}
/** @} */

/** @addtogroup BT-LOW-ENERGY1
* @{
*/
/*
 * @fn         int32_t rsi_ble_vendor_rf_type(uint8_t ble_power_index)
 * @brief      Issue a vendor-specific command to set the RF type in the controller on given inputs. This is a Blocking API.
 * @pre        Device should be initialized before calling this API.
 * @param[in]  ble_power_index - power index 
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *               -4 - Buffer not available to serve the command
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_ble_vendor_rf_type(uint8_t ble_power_index);
int32_t rsi_ble_vendor_rf_type(uint8_t ble_power_index)
{

  SL_PRINTF(SL_RSI_BLE_VENDOR_RF_TRIGGER, BLE, LOG_INFO, "BLE_POWER_INDEX: %1x", ble_power_index);
  rsi_ble_vendor_rf_type_t ble_vendor_rf_type;

  ble_vendor_rf_type.opcode[0]       = (BLE_VENDOR_RF_TYPE_CMD_OPCODE & 0xFF);
  ble_vendor_rf_type.opcode[1]       = ((BLE_VENDOR_RF_TYPE_CMD_OPCODE >> 8) & 0xFF);
  ble_vendor_rf_type.ble_power_index = ble_power_index;

  return rsi_bt_driver_send_cmd(RSI_BT_VENDOR_SPECIFIC, &ble_vendor_rf_type, NULL);
}

/**
 * @fn         int32_t rsi_ble_acceptlist_on_type(uint8_t enable,
 *                                                uint8_t ad_type,
 *                                                uint8_t value_length,
 *                                                const uint8_t *value)
 * @brief      Vendor-specific command to set the acceptlist filter based on
 *             advertising data type, length, and value. This is a blocking API.
 * @param[in]  enable - enable/disable
 * @param[in]  ad_type - Advertising data type to filter \n
 *                       Currently only supports: \n
 *                       0x08 - Shortened Local Name \n
 *                       0x09 - Complete Local Name
 * @param[in]  value_length - Length of the value to compare
 * @param[in]  value - Value to compare
 * @return     0 - Success \n
 *             Non-Zero Value - Failure
 * @note       This API currently supports filtering only on device name advertising data types (0x08 and 0x09). \n
 *             Other advertising data types are not supported by this vendor-specific command.
 */
int32_t rsi_ble_acceptlist_on_type(uint8_t enable, uint8_t ad_type, uint8_t value_length, const uint8_t *value)
{
  rsi_ble_req_acceptlist_on_type_t req;
  memset(&req, 0, sizeof(req));

  req.opcode[0] = (BLE_VENDOR_ACCEPTLIST_ON_TYPE & 0xFF);
  req.opcode[1] = ((BLE_VENDOR_ACCEPTLIST_ON_TYPE >> 8) & 0xFF);

  req.enable       = enable;
  req.ad_type      = ad_type;
  req.value_length = value_length;
  if ((value != NULL) && (value_length > 0)) {
    memcpy(req.value, value, value_length);
  }

  return rsi_bt_driver_send_cmd(RSI_BT_VENDOR_SPECIFIC, &req, NULL);
}
/**
 * @fn         int32_t rsi_ble_accept_list_using_adv_data(uint8_t enable,
 *                                                    uint8_t data_compare_index,
 *                                                    uint8_t len_for_compare_data,
 *                                                    uint8_t *payload)
 * @brief      Give vendor-specific command to set the acceptlist feature based on
 *             the advertisers advertising payload. This is a Blocking API
 * @pre        Device should be initialized before calling this API.
 * @param[in]  enable - enable/disable
 * @param[in]  data_compare_index - From which index onwards compare
 * @param[in]  len_for_compare_data - total length of data to compare
 * @param[in]  payload - Payload
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *               -4 - Buffer not available to serve the command
 *              0x4E62 	Invalid Parameters \n 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_accept_list_using_adv_data(uint8_t enable,
                                           uint8_t data_compare_index,
                                           uint8_t len_for_compare_data,
                                           const uint8_t *payload)
{

  SL_PRINTF(SL_RSI_BLE_ACCEPTLIST_USING_ADV_DATA, BLE, LOG_INFO);
  rsi_ble_req_acceptlist_using_payload_t acceptlist_using_payload;
  memset(&acceptlist_using_payload, 0, sizeof(acceptlist_using_payload));

  acceptlist_using_payload.opcode[0] = (BLE_VENDOR_ACCEPTLIST_USING_ADV_DATA_PAYLOAD & 0xFF);
  acceptlist_using_payload.opcode[1] = ((BLE_VENDOR_ACCEPTLIST_USING_ADV_DATA_PAYLOAD >> 8) & 0xFF);

  acceptlist_using_payload.enable               = enable;
  acceptlist_using_payload.total_len            = 31;
  acceptlist_using_payload.data_compare_index   = data_compare_index;
  acceptlist_using_payload.len_for_compare_data = len_for_compare_data;

  memcpy(acceptlist_using_payload.adv_data_payload, payload, 31);

  return rsi_bt_driver_send_cmd(RSI_BT_VENDOR_SPECIFIC, &acceptlist_using_payload, NULL);
}

/**
 * @fn         int32_t rsi_ble_set_coex_roles_priority(const uint8_t *payload)
 * @brief      Give vendor-specific command to set the priorities for the various coex roles.
 *             The payload contains role id, followed by min, max priorities of the specified role..
 *             This is a Blocking API.
 * @pre        Device should be initialized before calling this API.
 * @param[in]  payload - Payload (There are 7 coex roles refer rsi_ble_coex_role_id_t, each having min and max priority followed by the role id in the role_priority_payload.)
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *               -4 - Buffer not available to serve the command
 *              0x4E62  Invalid Parameters \n
 * @note       Refer to the Status Codes section for the above error codes at [additional-status-codes](../wiseconnect-api-reference-guide-err-codes/sl-additional-status-errors).
 */

int32_t rsi_ble_set_coex_roles_priority(const uint8_t *payload)
{

  SL_PRINTF(SL_RSI_BLE_SET_COEX_ROLES_PRIORITY, BLE, LOG_INFO);
  rsi_ble_set_coex_roles_priority_t coex_roles_priority;
  memset(&coex_roles_priority, 0, sizeof(coex_roles_priority));

  coex_roles_priority.opcode[0] = (BLE_VENDOR_SET_COEX_ROLE_PRIORITY & 0xFF);
  coex_roles_priority.opcode[1] = ((BLE_VENDOR_SET_COEX_ROLE_PRIORITY >> 8) & 0xFF);

  memcpy(coex_roles_priority.role_priority_payload, payload, 21);

  return rsi_bt_driver_send_cmd(RSI_BT_VENDOR_SPECIFIC, &coex_roles_priority, NULL);
}
/*==============================================*/
/**
 * @fn         void BT_LE_ADPacketExtract(uint8_t *remote_name, uint8_t *pbuf, uint8_t buf_len)
 * @brief      Used to extract remote Bluetooth device name from the received advertising report.
 * @pre        Device should be initialized before calling this API.
 * @param[in]  remote_name - device name
 * @param[in]  pbuf -  advertise data packet buffer pointer
 * @param[in]  buf_len - buffer length
 * @return     void
 */

void BT_LE_ADPacketExtract(uint8_t *remote_name, const uint8_t *pbuf, uint8_t buf_len)
{

  SL_PRINTF(SL_RSI_BLE_AD_PACKET_EXTRACT, BLE, LOG_INFO);
  uint8_t adv_len;
  uint8_t adv_type;
  uint8_t ix;
  uint8_t name_len;

  ix = 0;

  while (ix < buf_len) {
    adv_len = pbuf[ix];
    ix += sizeof(uint8_t);
    adv_type = pbuf[ix];

    switch (adv_type) {
      case 0x08:
      case 0x09: {
        name_len = adv_len - 1;
        memcpy(remote_name, &pbuf[ix + 1], name_len);
        remote_name[name_len] = 0;
        return;
      }
        // no break
      default:
        break;
    }

    ix += adv_len;
  }
}
/** @} */

/** @addtogroup BT-LOW-ENERGY1
* @{
*/

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_start_encryption(uint8_t *remote_dev_address, uint16_t ediv,
 *                                              uint8_t *rand, uint8_t *ltk)
 * @brief      Start the encryption process with the remote device. This is a Blocking API \n
 *             A received event \ref rsi_ble_on_encrypt_started_t indicated encrypted event is received from module \n
 *             A received event \ref rsi_ble_on_le_security_keys_t indicates exchange of security keys completed after encryption. \n
 *             A received event \ref rsi_ble_on_smp_failed_t indicated SMP procedure have failed 
 * @pre        Encryption enabled event should come before calling this API for second time SMP connection.
 * @param[in]  remote_dev_address - Remote BD address in string format
 * @param[in]  ediv - remote device ediv value.
 * @param[in]  rand - remote device rand value.
 * @param[in]  ltk  - remote device ltk value.
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 *              0x4D05  BLE socket not available \n
 *              0x4E62 	Invalid Parameters \n 
 *              0x4D04	BLE not connected \n 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */

int32_t rsi_ble_start_encryption(uint8_t *remote_dev_address, uint16_t ediv, const uint8_t *rand, const uint8_t *ltk)
{

  SL_PRINTF(SL_RSI_BLE_ENCRYPTION_TRIGGER, BLE, LOG_INFO);
  rsi_ble_strat_encryption_t encrypt;
  memset(&encrypt, 0, sizeof(encrypt));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(encrypt.dev_addr, remote_dev_address);
#else
  memcpy(encrypt.dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  encrypt.ediv = ediv;
  memcpy(encrypt.rand, rand, 8);
  memcpy(encrypt.ltk, ltk, 16);

  return rsi_bt_driver_send_cmd(RSI_BLE_RSP_START_ENCRYPTION, &encrypt, NULL);
}
/** @} */

/*==============================================*/
/**
 * @fn          int32_t rsi_ble_set_ble_tx_power(int8_t tx_power)
 * @brief       Set the TX power value. This is a Blocking API    
 * @param[in]   tx_power - power value 
 *              #define RSI_BLE_PWR_INX_DBM  0  indicate tx_power in index \n
 *              Default Value for BLE Tx Power Index is 31, The range for the BLE Tx Power Index is 1 to 127 (0, 32 indexes are invalid) \n
 *                      1 - 31    BLE - 0DBM Mode.  \n
 *                     33 - 63    BLE - 10DBM Mode. \n
 *                     64 - 82    BLE - 1dBm - 18dBm HP Mode in the resolution of 1 dBm. \n
 * @note: the higher power will be backed off based on country region.
 *                     104 - 126  BLE - 0.5 dBm - 11 dBm HP Mode in the resolution of 0.5 dbm. \n
 *                     127  	    BLE HP Mode, Max power supported. \n
 *              #define RSI_BLE_PWR_INX_DBM  1  indicate tx_power in dBm \n
 *              tx_power in dBm (-8 dBm to 18 dBm) \n
 * @note: This API currently supports controlling the tx power in the configured chain during wireless initialization. However, changing the chain from LP to HP or vice versa is not supported.  
 * @return      0 - Success \n
 *              Non-Zero Value - Failure \n
 *              0x4E02 	Unknown Connection Identifier \n 
 *              0x4E01	Unknown HCI Command \n 
 *              0x4E0C	Command disallowed \n 
 *              0x4046 	Invalid Arguments \n 
 *              0x4D04	BLE not connected \n 
 *              0x4D14	BLE parameter out of mandatory range 
 * @note        Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 *
 */

int32_t rsi_ble_set_ble_tx_power(int8_t tx_power)
{

  SL_PRINTF(SL_RSI_BLE_SET_BLE_TX_POWER, BLE, LOG_INFO);
  rsi_ble_set_ble_tx_power_t ble_tx_power = { 0 };

#if RSI_BLE_PWR_INX_DBM
  ble_tx_power.tx_power = rsi_convert_db_to_powindex(tx_power);
  if (ble_tx_power.tx_power == 0) {

    SL_PRINTF(SL_RSI_ERROR_INVALID_PARAMETER, BLE, LOG_ERROR);
    return RSI_ERROR_INVALID_PARAM;
  }
#else
  ble_tx_power.tx_power = tx_power;
#endif

  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_SET_BLE_TX_POWER, &ble_tx_power, NULL);
}

/** @addtogroup BT-LOW-ENERGY5
* @{
*/

/*========================================================*/
/**
 * @fn			   int32_t rsi_ble_get_max_no_of_supp_adv_sets(uint8_t *resp)
 * @brief		   request to the controller to know the maximum no.of supporting advertising sets
 * @param[in]  none
 * @param[out] resp - Number of Advertising sets supported ,filled by the controller. \n Possible values of Number of Advertising sets supported : 0x01 to 0xF0
 * @return      0   =  success \n
 *             !0   = failure \n
 * @section    description
 * This function requests the controller to know the maximum no.of supporting advertising sets.
 * @note : The number of advertising sets that can be supported is not fixed and it can be configured through the Opermodes.
 */
int32_t rsi_ble_get_max_no_of_supp_adv_sets(uint8_t *resp)
{
  rsi_ble_ae_pdu_t ae_pdu = { 0 };

  ae_pdu.cmd_sub_opcode = RSI_BLE_GET_AE_MAX_NO_OF_ADV_SETS;

  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_AE, &ae_pdu, resp);
}

/*========================================================*/
/**
 * @fn			   int32_t rsi_ble_get_max_adv_data_len(uint8_t *resp)
 * @brief		   request to the controller to know the maximum supporting advertising data length
 * @param[in]  none
 * @param[out] resp - filled by the controller \n
 *                    Possible values of Maximum Advertising Data Length : 0x001F to 0x0672 \n
 * @return     0   =  success \n
 *            !0   = failure \n
 * @section description
 * This function requests the controller to know the maximum supporting advertising data length
 */
int32_t rsi_ble_get_max_adv_data_len(uint8_t *resp)
{
  rsi_ble_ae_pdu_t ae_pdu = { 0 };

  ae_pdu.cmd_sub_opcode = RSI_BLE_GET_AE_MAX_ADV_DATA_LEN;

  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_AE, &ae_pdu, resp);
}

/*==============================================*/
/**
 * @fn         rsi_ble_set_ae_set_random_address(uint8_t handle, uint8_t *rand_addr)
 * @brief      sets the ae set random address
 * @param[in]  handle  - Advertising_Handle : Used to identify an advertising set
 * @param[in]  rand_addr - Random_Address  : Random device address , may be either of static address or Private address
 * @return     0   =  success \n
 *            !0   = failure \n
 * @section description
 * This function is used by the host to set the random device address specified by the Random_Address_parameter
 */
int32_t rsi_ble_set_ae_set_random_address(uint8_t handle, const uint8_t *rand_addr)
{
  rsi_ble_ae_pdu_t ae_pdu = { 0 };

  ae_pdu.cmd_sub_opcode = RSI_BLE_SET_AE_SET_RANDOM_ADDR;

  ae_pdu.pdu_type.ae_random_address.adv_handle = handle;
  memcpy(&ae_pdu.pdu_type.ae_random_address.addr, rand_addr, RSI_DEV_ADDR_LEN);

  //! Send set ae set random address command
  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_AE, &ae_pdu, NULL);
}

/*========================================================*/
/**
 * @fn			    int32_t rsi_ble_set_ae_params(void *ble_ae_params, int8_t *sel_tx_pwr)
 * @brief		    request the local device to set the extended advertising parameters
 * @param[in]   ble_ae_params - Extended Advertising Parameters command data to be sent is filled in this
 * @param[out]  sel_tx_power - Selected_TX_Power, selected output tx power, Its units are in dBm and Range : -127 to +20
 * @ @return    0  =  success \n
 *             !0  = failure \n
 * @section description
 * This function requests the local device to set Extended Advertising Parameters
 */
int32_t rsi_ble_set_ae_params(void *ble_ae_params, int8_t *sel_tx_pwr)
{
  rsi_ble_ae_pdu_t ae_pdu = { 0 };

  ae_pdu.cmd_sub_opcode = RSI_BLE_SET_AE_PARAMS;
  memcpy(&ae_pdu.pdu_type.ae_adv_params, (rsi_ble_ae_adv_params_t *)ble_ae_params, sizeof(rsi_ble_ae_adv_params_t));

  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_AE, &ae_pdu, sel_tx_pwr);
}

/*========================================================*/
/**
 * @fn			   int32_t rsi_ble_set_ae_data(void *ble_ae_data)
 * @brief		   request the local device to set the AE advertiser data used in advertising PDUs
 * @param[in]  ble_ae_data - Extended Advertising data to be sent is filled in this
 * @return     0  =  success \n
 *            !0  = failure \n
 * @section description
 * This function requests the local device to set the AE advertiser data used in advertising PDUs.
 * @note       Refer to Bluetooth spec 5.3 for possible combinations ae_adv/scanresp data can be set for.
 */
int32_t rsi_ble_set_ae_data(void *ble_ae_data)
{
  rsi_ble_ae_pdu_t ae_pdu = { 0 };

  ae_pdu.cmd_sub_opcode = RSI_BLE_SET_AE_DATA;
  memcpy(&ae_pdu.pdu_type.ae_adv_or_scn_rsp_data, (rsi_ble_ae_data_t *)ble_ae_data, sizeof(rsi_ble_ae_data_t));

  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_AE, &ae_pdu, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_start_ae_advertising(void *adv_enable)
 * @brief      request the local device to enable or disable an advertising set using the advertising sets identified by the Advertising_Handle[i] parameter.
 * @param[in]  adv_enable - data to be sent is filled in this
 * @return      0  =  success \n
 *             !0 = failure \n
 * @section description
 * This function requests the local device to enable or disable an advertising set using the advertising sets identified by the Advertising_Handle[i] parameter.
 */
int32_t rsi_ble_start_ae_advertising(void *adv_enable)
{
  rsi_ble_ae_pdu_t ae_pdu = { 0 };

  ae_pdu.cmd_sub_opcode = RSI_BLE_SET_AE_ENABLE;
  memcpy(&ae_pdu.pdu_type.ae_adv_enable, (rsi_ble_ae_adv_enable_t *)adv_enable, sizeof(rsi_ble_ae_adv_enable_t));

  //! Send start ae advertise command
  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_AE, &ae_pdu, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_app_adv_set_clear_or_remove(uint8_t type, uint8_t handle)
 * @brief      request the local device to clear or remove the advertising sets based on the type specified
 * @param[in]  type - Specifies whether to remove or clear the advertising sets.
 *             {1}  - clear
 *             {2}  - remove
 * @param[in]  handle - Advertising_Handle - Used to identify an Advertising set. Possible Values : 0x00 to 0xEF
 * @return     0  =  success \n
 *            !0 = failure \n
 * @section description
 * This function requests the local device to clear or remove an advertising set from the controller based on the type specified.
 */
int32_t rsi_ble_app_adv_set_clear_or_remove(uint8_t type, uint8_t handle)
{
  rsi_ble_ae_pdu_t ae_pdu = { 0 };

  ae_pdu.cmd_sub_opcode                                 = RSI_BLE_ADV_SET_CLEAR_OR_REMOVE;
  ae_pdu.pdu_type.ae_adv_set_clear_or_remove.type       = type;
  ae_pdu.pdu_type.ae_adv_set_clear_or_remove.adv_handle = handle;

  //! Send adv set (clear/remove) command
  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_AE, &ae_pdu, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_app_set_periodic_ae_params(void *periodic_adv_params)
 * @brief      request the local device to set periodic advertising params
 * @param[in]  periodic_adv_params - Parameters for Periodic Advertising to be filled here.
 * @return      0  =  success \n
 *             !0 = failure \n
 * @section description
 * This function is used by the host to set the parameters for periodic advertising.
 */
int32_t rsi_ble_app_set_periodic_ae_params(void *periodic_adv_params)
{
  rsi_ble_ae_pdu_t ae_pdu = { 0 };

  ae_pdu.cmd_sub_opcode = RSI_BLE_SET_AE_PERIODIC_ADV_PARAMS;
  memcpy(&ae_pdu.pdu_type.ae_periodic_adv_params,
         (rsi_ble_ae_periodic_adv_params_t *)periodic_adv_params,
         sizeof(rsi_ble_ae_periodic_adv_params_t));

  //! Send ae periodic adv params set command
  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_AE, &ae_pdu, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_app_set_periodic_ae_enable(uint8_t enable, uint8_t handle)
 * @brief      request the Controller to enable or disable the periodic advertising
 * @param[in]  enable - Enable, BIT map for Enable \n
 *
 *                0      -  Enable Periodic Advertising  \n
 *                1      -  Include the ADI field in AUX_SYNC_IND PDUs \n
 *
 *             handle - Advertising_Handle - Used to Identify an Advertsing Set \n
 * @return      0  =  success \n
 *             !0 = failure \n
 * @section description
 * This function requests the Controller to enable or disable the periodic advertising for the advertising set specified by the Advertising_Handle parameter.
 */
int32_t rsi_ble_app_set_periodic_ae_enable(uint8_t enable, uint8_t handle)
{
  rsi_ble_ae_pdu_t ae_pdu = { 0 };

  ae_pdu.cmd_sub_opcode                             = RSI_BLE_SET_AE_PERIODIC_ADV_ENABLE;
  ae_pdu.pdu_type.ae_periodic_adv_enable.enable     = enable;
  ae_pdu.pdu_type.ae_periodic_adv_enable.adv_handle = handle;

  //! Send ae periodic adv enable command
  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_AE, &ae_pdu, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_ae_set_scan_params(void *ae_scan_params)
 * @brief      sets the extended scan parameters to be used on the advertising physical channels.
 * @param[in]  ae_scan_params - Extended scan Parameters data would be filled here
 * @return     0  =  success \n
 *            !0  = failure \n
 * @section description
 * This function sets the extended scan parameters to be used on the advertising physical channels.
 */
int32_t rsi_ble_ae_set_scan_params(void *ae_scan_params)
{
  rsi_ble_ae_pdu_t ae_pdu = { 0 };

  ae_pdu.cmd_sub_opcode = RSI_BLE_SET_AE_SCAN_PARAMS;
  memcpy(&ae_pdu.pdu_type.ae_scan_params,
         (rsi_ble_ae_set_scan_params_t *)ae_scan_params,
         sizeof(rsi_ble_ae_set_scan_params_t));

  //! Send ae set scan params command
  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_AE, &ae_pdu, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_ae_set_scan_enable(void *ae_scan_enable)
 * @brief      request the local device to enable or disable scanning for both legacy and extended advertising PDUs
 * @param[in]  ae_scan_enable - Extended Scan Enable command Parameters would be filled here
 * @return     0  =  success \n
 *            !0  = failure \n
 * @section description
 * This function requests the local device to  enable or disable scanning for both legacy and extended advertising PDUs.
 */
int32_t rsi_ble_ae_set_scan_enable(void *ae_scan_enable)
{
  rsi_ble_ae_pdu_t ae_pdu = { 0 };

  ae_pdu.cmd_sub_opcode = RSI_BLE_SET_AE_SCAN_ENABLE;
  memcpy(&ae_pdu.pdu_type.ae_scan_enable,
         (rsi_ble_ae_set_scan_enable_t *)ae_scan_enable,
         sizeof(rsi_ble_ae_set_scan_enable_t));

  //! Send start ae  scanning command
  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_AE, &ae_pdu, NULL);
}

/*==============================================*/
/**
 * @fn         rsi_ble_ae_set_periodic_sync
 * @brief      used to synchronize with a periodic advertising train from an advertiser and begin receiving periodic advertising packets.
 * @param[in]  type - Specifies whether to create/terminate/cancel the periodic advertising sync \n
 *              {1} - CREATE_SYNC \n
 *              {2} - CREATE_SYNC_CANCEL \n
 *              (3) - TERMINATE_SYNC \n
 * @param[in]  periodic_sync_data - Periodic Advertising Create Sync Command Parameters filled here \n
 * @return      0  =  success \n
 *             0x4E42  unknown advertising identifier \n
 *             0x4E0C   command Disallowed  \n
 * @section description
 * This function used to synchronize with a periodic advertising train from an advertiser and begin receiving periodic advertising packets. It performs action based on the type specified.
 */
int32_t rsi_ble_ae_set_periodic_sync(uint8_t type, void *periodic_sync_data)
{
  rsi_ble_ae_pdu_t ae_pdu = { 0 };

  ae_pdu.cmd_sub_opcode = RSI_BLE_SET_AE_PERIODIC_SYNC;

  ae_pdu.pdu_type.ae_periodic_sync.type = type;

  if (type == BLE_AE_PERIODIC_SYNC_CREATE) {

    memcpy(&ae_pdu.pdu_type.ae_periodic_sync.sync_type.create_sync,
           (rsi_ble_ae_set_periodic_adv_create_sync_t *)periodic_sync_data,
           sizeof(rsi_ble_ae_set_periodic_adv_create_sync_t));

    LOG_PRINT("size of create struct is %d \n ", sizeof(rsi_ble_ae_set_periodic_adv_create_sync_t));
  } else if (type == BLE_AE_PERIODIC_SYNC_TERMINATE) {

    memcpy(&ae_pdu.pdu_type.ae_periodic_sync.sync_type.terminate_sync,
           (rsi_ble_ae_set_periodic_adv_terminate_sync_t *)periodic_sync_data,
           sizeof(rsi_ble_ae_set_periodic_adv_terminate_sync_t));
    LOG_PRINT("size of terminate struct is %d \n ", sizeof(rsi_ble_ae_set_periodic_adv_create_sync_t));
  } else if (type == BLE_AE_PERIODIC_SYNC_CREATE_CANCEL) {
    //nothing to fo
  }

  //! Send periodic sync command
  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_AE, &ae_pdu, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_ae_dev_to_periodic_list(void *dev_to_list)
 * @brief      used to add,remove or clear the device to the periodic advertiser list
 * @param[in]  dev_to_list - holds the data of the Device to be added to periodic Advertising list
 * @return     0  =  success \n
 *            !0 = failure \n
 * @section description
 * This function used to add,remove or clear the device to the periodic advertiser list
 */
int32_t rsi_ble_ae_dev_to_periodic_list(void *dev_to_list)
{
  rsi_ble_ae_pdu_t ae_pdu = { 0 };

  ae_pdu.cmd_sub_opcode = RSI_BLE_AE_DEV_TO_PERIODIC_LIST;
  memcpy(&ae_pdu.pdu_type.dev_to_periodic_list,
         (rsi_ble_ae_dev_to_periodic_list_t *)dev_to_list,
         sizeof(rsi_ble_ae_dev_to_periodic_list_t));

  //! Send dev to periodic list cmd
  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_AE, &ae_pdu, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_ae_read_periodic_adv_list_size(uint8_t *resp)
 * @brief      used to read the periodic advertiser list size
 * @param[in]  none
 * @param[out] resp - stores the value of total number of Periodic Advertiser list entries/ advertinsing list size filled by the controller
 * @return     0  =  success \n
 *            !0  = failure \n
 * @section description
 * This function used to read the periodic advertiser list size
 */
int32_t rsi_ble_ae_read_periodic_adv_list_size(uint8_t *resp)
{
  rsi_ble_ae_pdu_t ae_pdu = { 0 };

  ae_pdu.cmd_sub_opcode = RSI_BLE_AE_READ_PERIODIC_LIST_SIZE;
  //! Send read periodic adv list size cmd
  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_AE, &ae_pdu, resp);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_extended_connect_with_params(void *ble_extended_conn_params)
 * @brief      This command is used to create an ACL connection, with the local device in the Central role, to a connectable advertiser.
 * @param[in]  ble_extended_conn_params, It holds the data of the Parameters required to connect with the peer device.
 * @return     0  =  success \n
 *            !0  = failure \n
 * @section description
 * This function used to used to create an ACL connection, with the local device in the Central role, to a connectable advertiser.
 */

int32_t rsi_ble_extended_connect_with_params(void *ble_extended_conn_params)
{
  rsi_ble_ae_pdu_t ae_pdu = { 0 };

  ae_pdu.cmd_sub_opcode = RSI_BLE_AE_EXTENDED_CREATE_CONNECT;
  memcpy(&ae_pdu.pdu_type.extended_create_conn,
         (rsi_ble_ae_extended_create_connect_t *)ble_extended_conn_params,
         sizeof(rsi_ble_ae_extended_create_connect_t));

  //! Send start ae  scanning command
  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_AE, &ae_pdu, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_read_transmit_power(rsi_ble_tx_pwr_t *resp)
 * @brief      used to read minimum and maximum transmit powers supported by the controller based on the country region. 
 * @param[in]  none
 * @param[out] resp -  filled by the controller. Controller fills Minimum TX Power and Maximum TX power in this resp
 *             Min_TX_Power : Units in dBm Ranges from -127 to +20 \n
 *             Max_TX_Power : Units in dBm Ranges from -127 to +20 \n
 * @return     0  =  success \n
 *            !0  = failure \n
 * @section description
 * This function is used to read minimum and maximum transmit powers supported by the controller based on the country region.
 */

int32_t rsi_ble_read_transmit_power(void *resp)
{
  //! Send read transmit power cmd
  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_READ_TRANSMIT_POWER, NULL, (rsi_ble_tx_pwr_t *)resp);
}

/*==============================================*/
/**
 * @fn          int32_t rsi_ble_read_rf_path_compensation(rsi_ble_query_rf_path_comp_t *resp)
 * @brief       used to read rf path compensation value parameters used in the Tx power level and RSSI calculation
 * @param[in]   none
 * @param[out]  resp - filled by the controller. Controller fills RF_TX_Path_compensation_Value and RF_RX_Path_compensation_Value \n
 *              RF_TX_Path_compensation_Value - units in 0.1dB , Range :  -128.0 dB (0xFB00) to 128.0 dB (0x0500) \n
 *              RF_RX_Path_compensation_Value - units in 0.1dB , Range :  -128.0 dB (0xFB00) to 128.0 dB (0x0500) \n
 * @return      0  =  success \n
 *             !0  = failure \n
 * @section description
 * This function used to read rf path compensation value parameters used in the Tx power level and RSSI calculation.
 * */
int32_t rsi_ble_read_rf_path_compensation(void *resp)
{

  //! Send read rf path compensation cmd
  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_READ_RF_PATH_COMP, NULL, (rsi_ble_query_rf_path_comp_t *)resp);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_write_rf_path_compensation(uint16_t tx_path_value, uint16_t rx_path_value)
 * @brief      used to indicate the RF path gain or loss between the RF transceiver and the antenna contributed by intermediate components
 * @param[in]  tx_path_value - RF_TX_Path_Compensation_Value, rx_path_value - RF_RX_Path_Compensation_Value \n
 *             A positive value means a net RF path gain and a negative value means a net RF path loss \n
 *             RF_TX_Path_compensation_Value : units in 0.1dB , Range :  -128.0 dB (0xFB00) to 128.0 dB (0x0500) \n
 *             RF_RX_Path_compensation_Value : units in 0.1dB , Range :  -128.0 dB (0xFB00) to 128.0 dB (0x0500) \n
 * @return     0  =  success \n
 *            !0  = failure \n
 * @section description
 * This function used to indicate the RF path gain or loss between the RF transceiver and the antenna contributed by intermediate components.
 * */
int32_t rsi_ble_write_rf_path_compensation(uint16_t tx_path_value, uint16_t rx_path_value)
{
  rsi_ble_write_rf_path_comp_t rf_path_comp = { 0 };

  rf_path_comp.tx_path_value = tx_path_value;
  rf_path_comp.rx_path_value = rx_path_value;
  //! Send write rf path compensation cmd
  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_WRITE_RF_PATH_COMP, &rf_path_comp, NULL);
}
/** @} */
