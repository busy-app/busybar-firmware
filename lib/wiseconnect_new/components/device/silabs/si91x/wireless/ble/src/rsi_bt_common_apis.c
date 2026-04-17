/*******************************************************************************
* @file  rsi_bt_common_apis.c
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

#include <rsi_common.h>
#include <rsi_bt_common.h>

#include <sl_si91x_types.h>
#include <sl_wifi_constants.h>
#include <sl_si91x_ble.h>
#include <sl_status.h>

/******************************************************
 *               Function Declarations
 ******************************************************/
int32_t rsi_bt_set_bd_addr(const uint8_t *dev_addr);
int32_t rsi_bt_set_local_name(const uint8_t *local_name);
int32_t rsi_bt_cmd_update_gain_table_offset_or_max_pwr(uint8_t node_id,
                                                       uint8_t payload_len,
                                                       const uint8_t *payload,
                                                       uint8_t req_type);
int32_t rsi_bt_get_local_name(rsi_bt_resp_get_local_name_t *bt_resp_get_local_name);
int32_t rsi_bt_get_rssi(const uint8_t *dev_addr, int8_t *resp);
int32_t rsi_bt_get_local_device_address(uint8_t *resp);
int32_t rsi_bt_get_bt_stack_version(rsi_bt_resp_get_bt_stack_version_t *bt_resp_get_bt_stack_version);
int32_t rsi_bt_init(void);
int32_t rsi_bt_deinit(void);
int32_t rsi_bt_set_antenna(uint8_t antenna_value);
int32_t rsi_bt_set_feature_bitmap(uint32_t feature_bit_map);
int32_t rsi_bt_set_antenna_tx_power_level(uint8_t protocol_mode, int8_t tx_power);
int32_t rsi_bt_power_save_profile(uint8_t psp_mode, uint8_t psp_type);
int32_t rsi_bt_per_stats(uint8_t cmd_type, struct rsi_bt_per_stats_s *per_stats);
/** @addtogroup BT-BLE 
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t rsi_bt_set_bd_addr(uint8_t *dev_addr)
 * @brief       Set the device BD address. This is blocking API.
 * @pre         needs to be called immediately after @ref sl_wifi_init() OPERMODE command.
 * @param[in]   dev_addr - public address of the device to be set \n
 * @return      0 		 - Success \n
 *              Non-Zero Value - Failure \n
 *              -3       - Command is given in wrong state(i.e not immediate after opermode)
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_bt_set_bd_addr(const uint8_t *dev_addr)
{
  rsi_bt_set_local_bd_addr_t bd_addr;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bd_addr.dev_addr, dev_addr);
#else
  memcpy(bd_addr.dev_addr, dev_addr, RSI_DEV_ADDR_LEN);
#endif
  SL_PRINTF(SL_RSI_BT_SET_BD_ADDRESS, BLUETOOTH, LOG_INFO);

  return rsi_bt_driver_send_cmd(RSI_BT_SET_BD_ADDR_REQ, &bd_addr, NULL);
}
/** @} */

/** @addtogroup BT-BLE
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_bt_set_local_name(uint8_t *local_name)
 * @brief      Set the given name to local device. This is blocking API.
 * @pre        Device should be initialized before calling this API.
 * @param[in]  local_name - Name to be set to the local device.
 * @note       For BLE alone Opermode : When the name of the local device is set to a value with length more than 16 bytes then error is returned with an error code 0x4E66.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 *             
 */
int32_t rsi_bt_set_local_name(const uint8_t *local_name)
{
  uint8_t name_len;
  rsi_bt_req_set_local_name_t bt_req_set_local_name = { 0 };
  name_len = (uint8_t)RSI_MIN(strlen((const char *)local_name), (RSI_DEV_NAME_LEN - 1));

  memcpy(bt_req_set_local_name.name, local_name, name_len);
  bt_req_set_local_name.name[name_len] = 0;
  bt_req_set_local_name.name_len       = name_len;
  SL_PRINTF(SL_RSI_BT_SET_LOCAL_NAME, BLUETOOTH, LOG_INFO);

  return rsi_bt_driver_send_cmd(RSI_BT_SET_LOCAL_NAME, &bt_req_set_local_name, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_cmd_update_gain_table_offset_or_max_pwr(uint8_t node_id, uint8_t payload_len, uint8_t *payload, uint8_t req_type)
 * @brief      Update gain table offset/max power. This is blocking API.
 * @pre        Device should be initialized before calling this API.
 * @param[in]  node_id     - Node ID (0 - BLE, 1 - BT).
 * @param[in]  payload_len - Length of the payload.
 * @param[in]  payload     - Payload containing table data of gain table offset/max power
 * @param[in]  req_type    - update gain table request type \n
 *             0 - Max power update \n
 *             1 - Max power offset update \n
 *             2 - LP_chain 0dBm offset update \n
 *             3 - LP_chain 10dBm offset update \n
 * @return     0		-	Success \n
 *             0x4F01		-	Invalid gain table payload length \n
 *             0x4F02		-	Invalid region. \n
 *             0x4F03		-	Invalid gain table offset request type \n
 *             0x4F04           -       Invalid node id. \n	
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 *             
 */
int32_t rsi_bt_cmd_update_gain_table_offset_or_max_pwr(uint8_t node_id,
                                                       uint8_t payload_len,
                                                       const uint8_t *payload,
                                                       uint8_t req_type)
{
  rsi_bt_cmd_update_gain_table_offset_or_maxpower_t bt_gain_table_offset = { 0 };
  bt_gain_table_offset.node_id                                           = node_id;
  bt_gain_table_offset.update_gain_table_type                            = req_type;
  bt_gain_table_offset.payload_len                                       = payload_len;
  memcpy(bt_gain_table_offset.payload, payload, bt_gain_table_offset.payload_len);
  SL_PRINTF(SL_RSI_BT_SET_GAIN_TABLE_OFFSET_OR_MAX_POWER_UPDATE, BLUETOOTH, LOG_INFO);

  return rsi_bt_driver_send_cmd(RSI_BT_SET_GAIN_TABLE_OFFSET_OR_MAX_POWER_UPDATE, &bt_gain_table_offset, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_get_local_name(rsi_bt_resp_get_local_name_t *bt_resp_get_local_name)
 * @brief      Get the local device name. This is blocking API.
 * @pre        Device should be initialized before calling this API.
 * @param[out] bt_resp_get_local_name - This parameter is the response buffer to hold the response of this API. Please refer rsi_bt_resp_get_local_name_s structure for more info.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_bt_get_local_name(rsi_bt_resp_get_local_name_t *bt_resp_get_local_name)
{
  SL_PRINTF(SL_RSI_BT_GET_LOCAL_NAME, BLUETOOTH, LOG_INFO);
  return rsi_bt_driver_send_cmd(RSI_BT_GET_LOCAL_NAME, NULL, bt_resp_get_local_name);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_get_rssi(uint8_t *dev_addr, int8_t *resp) 
 * @brief      Get the RSSI of the remote device. This is blocking API.
 * @pre        \ref rsi_bt_connect() API need to be called before this API.
 * @param[in]  dev_addr -  Remote device address.
 * @param[out] resp - Parameter to hold the response of this API, rssi is filled in this resp parameter. 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_bt_get_rssi(const uint8_t *dev_addr, int8_t *resp)
{
  rsi_bt_get_rssi_t bt_rssi = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_rssi.dev_addr, dev_addr);
#else
  memcpy(bt_rssi.dev_addr, dev_addr, 6);
#endif
  SL_PRINTF(SL_RSI_BT_GET_RSSI, BLUETOOTH, LOG_INFO);
  return rsi_bt_driver_send_cmd(RSI_BT_GET_RSSI, &bt_rssi, resp);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_get_local_device_address(uint8_t *resp)
 * @brief      Get the local device address. This is blocking API.
 * @pre        Device should be initialized before calling this API.
 * @param[out] resp - Parameter to hold the response of this API, local bd_addr is filled in this resp parameter.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_bt_get_local_device_address(uint8_t *resp)
{
  SL_PRINTF(SL_RSI_BT_GET_LOCAL_DEVICE_ADDRESS, BLUETOOTH, LOG_INFO);
  return rsi_bt_driver_send_cmd(RSI_BT_GET_LOCAL_DEV_ADDR, NULL, resp);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_get_bt_stack_version(rsi_bt_resp_get_bt_stack_version_t *bt_resp_get_bt_stack_version) 
 * @brief      Get the BT stack version. This is blocking API.
 * @param[out] bt_resp_get_bt_stack_version -  Response buffer to hold the response of this API. Please refer rsi_bt_resp_get_bt_stack_version_s structure for more info
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_bt_get_bt_stack_version(rsi_bt_resp_get_bt_stack_version_t *bt_resp_get_bt_stack_version)
{
  SL_PRINTF(SL_RSI_BT_STACK_VERSION, BLUETOOTH, LOG_INFO);
  return rsi_bt_driver_send_cmd(RSI_BT_GET_BT_STACK_VERSION, NULL, bt_resp_get_bt_stack_version);
}
/** @} */

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_init(void)
 * @brief      Initialize the BT device. This is blocking API.
 * @pre        Device should be initialized before calling this API.  \n
 *             If the device is in powersave, get back the device to ACTIVE MODE by using \ref rsi_bt_power_save_profile()
 * @param      None
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_bt_init(void)
{
  SL_PRINTF(SL_RSI_BT_INIT_TRIGGER, BLUETOOTH, LOG_INFO);
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_INIT, NULL, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t 	rsi_bt_deinit(void)
 * @brief      Deinitialize the BT device. This is blocking API.
 * @pre        \ref Device should be initialized before this API. \n
 * @param      None
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_bt_deinit(void)
{
  SL_PRINTF(SL_RSI_BT_DEINIT_TRIGGER, BLUETOOTH, LOG_INFO);
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_DEINIT, NULL, NULL);
}

/** @addtogroup BT-BLE
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_bt_set_antenna(uint8_t antenna_value)
 * @brief      Select either internal / external antenna on the chip. This is blocking API.
 * @pre        Device should be initialized before calling this API.
 * @param[in]  antenna_value - Parameter is used to select either internal or external antenna. Possible values: \n
 *                             0x00 RSI_SEL_INTERNAL_ANTENNA \n 
 *                             0x01 RSI_SEL_EXTERNAL_ANTENNA 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_bt_set_antenna(uint8_t antenna_value)
{
  rsi_ble_set_antenna_t ble_set_antenna = { 0 };

  ble_set_antenna.value = antenna_value;
  SL_PRINTF(SL_RSI_BT_SET_ANTENNA, BLUETOOTH, LOG_INFO);
  return rsi_bt_driver_send_cmd(RSI_BT_SET_ANTENNA_SELECT, &ble_set_antenna, NULL);
}
/** @} */

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_set_feature_bitmap(uint32_t feature_bit_map)
 * @brief      Enable/Disable the mentioned features. This is blocking API.
 * @note       This API is not supported in the current release.
 * @pre        \ref rsi_sspmode_init() API needs to be called before this API.
 * @param[in]  feature_bit_map - features bit map list \n
 *                               0 This parameter is used for security purposes. \n 
 *                               If this bit is set pairing process occurs, else does not occur. \n
 *                               1 to 31 Reserved for future use. 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_bt_set_feature_bitmap(uint32_t feature_bit_map)
{
  rsi_bt_set_feature_bitmap_t bt_features = { 0 };

  bt_features.bit_map = feature_bit_map;
  SL_PRINTF(SL_RSI_SET_FEATURE_MAP, BLUETOOTH, LOG_INFO);
  return rsi_bt_driver_send_cmd(RSI_BT_SET_FEATURES_BITMAP, &bt_features, NULL);
}

/** @addtogroup BT-BLE 
* @{
*/
/*==============================================*/
/*
 * @fn         int32_t rsi_bt_set_antenna_tx_power_level(uint8_t protocol_mode, int8_t tx_power)
 * @brief      Enable/Disable the mentioned features. This is blocking API.
 * @pre        Device should be initialized before calling this API.
 * @param[in]  protocol_mode - protocol mode \n
 *             1 - BT classic \n
 *			   2 - BT Low Energy 
 * @param[in] tx-power - power value \n
 *             Antenna transmit power level \n
 *             Default Value for BLE Tx Power Index is 31, The range for the BLE Tx Power Index is 1 to 75 (0, 32 indexes are invalid) \n
 *             Default tx power index for BT classic is 14 \n
 *                      1 - 31     0DBM Mode.  \n
 *                     33 - 63     10DBM Mode. \n
 *                     64 - 79     HP Mode.    \n
 * @note       The default value will vary based on country region and board \n 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_bt_set_antenna_tx_power_level(uint8_t protocol_mode, int8_t tx_power)
{
  rsi_bt_set_antenna_tx_power_level_t bt_set_antenna_tx_power_level = { 0 };

  bt_set_antenna_tx_power_level.protocol_mode = protocol_mode;
#if RSI_BLE_PWR_INX_DBM
  if (protocol_mode == 2 /*BLE_MODE*/) {
    bt_set_antenna_tx_power_level.tx_power = rsi_convert_db_to_powindex(tx_power);
    if (bt_set_antenna_tx_power_level.tx_power == 0) {
      return RSI_ERROR_INVALID_PARAM;
    }
  } else {
    bt_set_antenna_tx_power_level.tx_power = tx_power;
  }
#else
  bt_set_antenna_tx_power_level.tx_power = tx_power;
#endif
  SL_PRINTF(SL_RSI_BT_SET_ANTENNA_TX_POWER_LEVEL, BLUETOOTH, LOG_INFO);
  return rsi_bt_driver_send_cmd(RSI_BT_SET_ANTENNA_TX_POWER_LEVEL, &bt_set_antenna_tx_power_level, NULL);
}

/*==============================================*/
/**
 * @fn          int32_t rsi_bt_power_save_profile(uint8_t psp_mode, uint8_t psp_type)
 * @brief       Select the power save profile mode for BT / BLE. This is blocking API.
 * @pre         \ref Device should be in initialized state before calling this API
 * @param[in]   psp_mode Following psp_mode is defined. \n
 *              0 - RSI_ACTIVE. In this mode module is active and power save is disabled. \n
 *              1 - RSI_SLEEP_MODE_1. On mode. In this sleep mode, SoC will never turn off, therefore no \n
 *                  handshake is required before sending data to the module. BT/BLE does not support this mode. \n
 *              2 - RSI_SLEEP_MODE_2. Connected sleep mode. In this sleep mode, SoC will go to sleep based \n
 *                  on GPIO or Message, therefore handshake is required before sending data to the module. \n
 *              8 - RSI_SLEEP_MODE_8 :Deep sleep mode with RAM RETENTION. \n
 *              10- RSI_SLEEP_MODE_10 : Deep sleep mode without RAM RETENTION. \n
 *                  In this sleep mode, module will turn off the \n
 *                  SoC. Since SoC is turn off, therefore handshake is required before sending data to the module. \n
 * @param[in]   psp_type Following psp_type is defined. \n
 *              0 - RSI_MAX_PSP. This psp_type will be used for max power saving \n
 *                  ONLY RSI_MAX_PSP is supported in BLE and CoEx modes\n
 * @return      0		-	Success \n
 *              Non-Zero Value	-	Failure \n
 *
 * @note        If the user wants to enable power save in CoEx mode (WLAN + BT LE) mode - It is mandatory to enable WLAN power save along with BT LE power save. \n
 * @note        The device will enter into power save if and only if both protocol (WLAN, BLE) power save modes are enabled. \n
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 * @note        psp_type RSI_MAX_PSP is only valid and supported only in psp_mode RSI_SLEEP_MODE_2. \n
 *              BT/BLE does not support in RSI_SLEEP_MODE_1. \n
 *
 * @note        Power save disable command has to be given before changing the state from standby to the remaining states \n
 *              and wise-versa. Suppose if Power Save is enabled in standby state, so, in order to move to Scanning state,\n
 *              first Power Save disable command need to be issued before giving Scan command. \n
 */
int32_t rsi_bt_power_save_profile(uint8_t psp_mode, uint8_t psp_type)
{
  sl_bt_performance_profile_t profile = { 0 };

  // Only RSI_MAX_PSP is supported In RSI_SLEEP_MODE_2 psp_mode
  if ((psp_mode == RSI_SLEEP_MODE_2) && (psp_type != RSI_MAX_PSP)) {
    return RSI_FEATURE_NOT_SUPPORTED;
  }

  if (psp_mode == RSI_SLEEP_MODE_1) {
    return RSI_FEATURE_NOT_SUPPORTED;
  }

  switch (psp_mode) {
    case RSI_ACTIVE: {
      profile.profile = HIGH_PERFORMANCE;
    } break;
    case RSI_SLEEP_MODE_2: {
      if (psp_type == RSI_MAX_PSP) {
        profile.profile = ASSOCIATED_POWER_SAVE_LOW_LATENCY;
      } else {
        return RSI_FEATURE_NOT_SUPPORTED;
      }
    } break;
    case RSI_SLEEP_MODE_8: {
      profile.profile = DEEP_SLEEP_WITH_RAM_RETENTION;
    } break;
    case RSI_SLEEP_MODE_10: {
      profile.profile = DEEP_SLEEP_WITHOUT_RAM_RETENTION;
    } break;
    default: {
      return RSI_FEATURE_NOT_SUPPORTED;
    }
  }

  if (sl_si91x_bt_set_performance_profile(&profile) != SL_STATUS_OK) {
    return RSI_FAILURE;
  }

  return RSI_SUCCESS;
}

/** @} */

/*==============================================*/
/**
 * @fn          int32_t rsi_bt_per_stats(uint8_t cmd_type, struct rsi_bt_per_stats_s *rsi_bt_per_stats)
 * @brief       Request the local device for BT PER operation.
 * @pre         Call \ref rsi_bt_per_tx() or rsi_bt_per_rx() before calling this API.
 * @param[in]   cmd_type - Parameter to define the command id type for PER operation. \n
 *              BT_PER_STATS_CMD_ID (0x08) - Command id enables PER statistics \n 
 *              BT_TRANSMIT_CMD_ID (0x15) - Command id enables PER transmit \n 
 *              BT_RECEIVE_CMD_ID (0x16) - Command id enables PER receive  
 * @param[in]   rsi_bt_per_stats - reference to the response structure. Please refer to rsi_bt_per_stats_s structure for more info. 
 * @return      0			-	Success \n
 *              Non-Zero Value	-	Failure
 * @note       Refer Error Codes section for common error codes \ref error-codes .
 */
int32_t rsi_bt_per_stats(uint8_t cmd_type, struct rsi_bt_per_stats_s *per_stats)
{
  SL_PRINTF(SL_RSI_BT_REQUEST_LOCAL_DEVICE, BLUETOOTH, LOG_INFO);
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_PER_CMD, &cmd_type, per_stats);
}
