/*******************************************************************************
 * @file  rsi_bt_common_apis.h
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef RSI_BT_COMMON_APIS_H
#define RSI_BT_COMMON_APIS_H

#include "rsi_bt_common.h"
#include "rsi_utils.h"

/******************************************************
 * *                      Macros
 * ******************************************************/

/** @addtogroup BT_BLE_CONSTANTS
 *  @{
 */

/// success return value
#define RSI_SUCCESS 0

/// failure return value
#define RSI_FAILURE -1
/** @} */
/******************************************************
 * *                    Constants
 * ******************************************************/
/******************************************************
 * *                   Enumerations
 * ******************************************************/

/******************************************************
 * *                 Type Definitions
 * ******************************************************/

/******************************************************
 * *                    Structures
 * ******************************************************/

/******************************************************
 * *                 Global Variables
 * ******************************************************/

/******************************************************
 * *              BT Common API's Declarations
 * ******************************************************/
/** @addtogroup BT-BLE
* @{
*/
#ifdef __cplusplus
extern "C" {
#endif
/*==============================================*/
/**
 * @fn          int32_t rsi_bt_set_bd_addr(const uint8_t *dev_addr)
 * @brief       Set the device BD address. This is a blocking API.
 * @pre Pre-conditions:
 *          - Needs to be called immediately after device initialization.
 * @param[in]   dev_addr - Public address of the device to be set. 
 * @return The following values are returned:
 *              - 0 		 - Success 
 *              - Non-Zero Value - Failure 
 *              - 3       - Command is given in wrong state (that is, not immediate after opermode)
 * @note       This is a blocking API. Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) .
 */
int32_t rsi_bt_set_bd_addr(const uint8_t *dev_addr);

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_set_local_name(const uint8_t *local_name)
 * @brief      Set the given name to local device. This is a blocking API.
 * @pre Pre-conditions:
 * -        Device should be initialized before calling this API.
 * @param[in]  local_name - Name to be set to the local device.
 * @note       For BLE alone Opermode : When the name of the local device is set to a value with length more than 16 bytes then error is returned with an error code 0x4E66.
 * @return The following values are returned:
 *            - 0		-	Success 
 *            - Non-Zero Value	-	Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_bt_set_local_name(const uint8_t *local_name);

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_cmd_update_gain_table_offset_or_max_pwr(uint8_t node_id, uint8_t payload_len, uint8_t *payload, uint8_t req_type)
 * @brief      Update gain table offset/max power. This is blocking API.
 * @pre Pre-conditions:
 * -        Device should be initialized before calling this API.
 * @param[in]  node_id     - Node ID (0 - BLE, 1 - BT).
 * @param[in]  payload_len - Length of the payload.
 * @param[in]  payload     - Payload containing table data of gain table offset/max power
 * @param[in]  req_type    - Update gain table request type 
 *              - 0 - Max power update.
 *              - 1 - Max power offset update.
 *              - 2 - LP_Chain 0 dBm offset update.
 *              - 3 - LP_chain 10 dBm offset update.
 * @return The following values are returned:
 *             - 0		-	Success.
 *             - 0x4F01		-	Invalid gain table payload length. 
 *             - 0x4F02		-	Invalid region. 
 *             - 0x4F03		-	Invalid gain table offset request type. 
 *             - 0x4F04           -       Invalid node id. 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_bt_cmd_update_gain_table_offset_or_max_pwr(uint8_t node_id,
                                                       uint8_t payload_len,
                                                       uint8_t *payload,
                                                       uint8_t req_type);

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_get_local_name(rsi_bt_resp_get_local_name_t *bt_resp_get_local_name)
 * @brief      Get the local device name. This is a blocking API.
 * @pre Pre-conditions:
 * -        Device should be initialized before calling this API.
 * @param[out] bt_resp_get_local_name - This parameter is the response buffer to hold the response of this API. Refer to rsi_bt_resp_get_local_name_s structure for more information.
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_bt_get_local_name(rsi_bt_resp_get_local_name_t *bt_resp_get_local_name);

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_get_rssi(uint8_t *dev_addr, int8_t *resp)
 * @brief      Get the RSSI of the remote device. This is a blocking API.
 * @pre Pre-conditions:
 * -            rsi_bt_connect() API need to be called before this API.
 * @param[in]  dev_addr -  Remote device address.
 * @param[out] resp - Parameter to hold the response of this API, RSSI is filled in this resp parameter.
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_bt_get_rssi(uint8_t *dev_addr, int8_t *resp);

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_get_local_device_address(uint8_t *resp)
 * @brief      Get the local device address. This is a blocking API.
 * @pre Pre-conditions:
 * -        Device should be initialized before calling this API.
 * @param[out] resp - Parameter to hold the response of this API, local bd_addr is filled in this resp parameter.
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_bt_get_local_device_address(uint8_t *resp);

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_get_bt_stack_version(rsi_bt_resp_get_bt_stack_version_t *bt_resp_get_bt_stack_version)
 * @brief      Get the BT stack version. This is a blocking API.
 * @param[out] bt_resp_get_bt_stack_version -  Response buffer to hold the response of this API. Refer to rsi_bt_resp_get_bt_stack_version_s structure for more info
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_bt_get_bt_stack_version(rsi_bt_resp_get_bt_stack_version_t *bt_resp_get_bt_stack_version);

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_init(void)
 * @brief      Initialize the BT device. This is a blocking API.
 * @pre Pre-conditions:
 *         - Device should be initialized before calling this API.  
 *         - If the device is in powersave, get back the device to ACTIVE MODE by using \ref rsi_bt_power_save_profile()
 * @return The following values are returned:
 *             - 0		-	Success 
 *             - Non-Zero Value	-	Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_bt_init(void);

/*==============================================*/
/**
 * @fn         int32_t 	rsi_bt_deinit(void)
 * @brief      Deinitialize the BT device. This is a blocking API.
 * @pre Pre-conditions:
 * -             Device should be initialized before this API. 
 * @return The following values are returned:
 *            - 0		-	Success 
 *            - Non-Zero Value	-	Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_bt_deinit(void);

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_set_antenna(uint8_t antenna_value)
 * @brief      Select either internal / external antenna on the chip. This is a blocking API.
 * @pre Pre-conditions:
 * -        Device should be initialized before calling this API.
 * @param[in]  antenna_value - Parameter is used to select either internal or external antenna. Possible values: 
 *                             - 0x00 RSI_SEL_INTERNAL_ANTENNA 
 *                             - 0x01 RSI_SEL_EXTERNAL_ANTENNA
 * @return The following values are returned:
 *              - 0		-	Success 
 *              - Non-Zero Value	-	Failure
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 */
int32_t rsi_bt_set_antenna(uint8_t antenna_value);

/*==============================================*/
/**
 * @fn          int32_t rsi_bt_power_save_profile(uint8_t psp_mode, uint8_t psp_type)
 * @brief       Select the power save profile mode for BT / BLE. This is a blocking API.
 * @pre Pre-conditions:
 * -         Device should be initialized before calling this API
 * @param[in]   psp_mode Following psp_mode is defined. 
 *              - 0 - RSI_ACTIVE. In this mode module is active and power save is disabled. 
 *              - 1 - RSI_SLEEP_MODE_1. On mode. In this sleep mode, SoC would never turn off, therefore no 
 *                  handshake is required before sending data to the module. BT/BLE does not support this mode. 
 *              - 2 - RSI_SLEEP_MODE_2. Connected sleep mode. In this sleep mode, SoC will go to sleep based 
 *                  on GPIO or Message, therefore handshake is required before sending data to the module. 
 *              - 8 - RSI_SLEEP_MODE_8 :Deep sleep mode with RAM RETENTION. 
 *              - 10- RSI_SLEEP_MODE_10 : Deep sleep mode without RAM RETENTION. 
 *                  In this sleep mode, module would turn off the 
 *                  SoC. Since SoC is turn off, therefore handshake is required before sending data to the module. 
 * @param[in]   psp_type Following psp_type is defined. 
 *              - 0 - RSI_MAX_PSP. This psp_type would be used for max power saving.
 *              - 1 - Fast PSP.
 * @return The following values are returned:
 *              - 0		-	Success
 *              - Non-Zero Value	-	Failure 
 * @note        If the user wants to enable power save in CoEx mode (WLAN + BT LE) mode - It is mandatory to enable WLAN power save along with BT LE power save. 
 * @note        The device will enter into power save if and only if both protocol (WLAN, BLE) power save modes are enabled. 
 * @note       Refer to the Status Codes section for the above error codes at [wiseconnect-status-codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes).
 * @note        psp_type is only valid in psp_mode 2. 
 *              - BT/BLE does not support in RSI_SLEEP_MODE_1. 
 *              - BT/BLE supports only RSI_MAX_PSP mode. Remaining modes are not supported.
 */
int32_t rsi_bt_power_save_profile(uint8_t psp_mode, uint8_t psp_type);

/*==============================================*/
/**
 * @fn          int32_t rsi_bt_per_stats(uint8_t cmd_type, struct rsi_bt_per_stats_s *rsi_bt_per_stats)
 * @brief       Request the local device for BT PER operation.
 * @pre Pre-conditions:
 * -         Call rsi_bt_per_tx() or rsi_bt_per_rx() before calling this API.
 * @param[in]   cmd_type - Parameter to define the command id type as per operation. 
 *              - BT_PER_STATS_CMD_ID (0x08) - Command id enables as per statistics.
 *              - BT_TRANSMIT_CMD_ID (0x15) - Command id enables as per transmit. 
 *              - BT_RECEIVE_CMD_ID (0x16) - Command id enables as per receive.
 * @param[in]   rsi_bt_per_stats - reference to the response structure. Refer to rsi_bt_per_stats_s structure for more information.
 * @return The following values are returned:
 *              - 0			-	Success 
 *              - Non-Zero Value	-	Failure
 * @note        Refer to Error Codes section for common error codes.
 */
int32_t rsi_bt_per_stats(uint8_t cmd_type, struct rsi_bt_per_stats_s *per_stats);

/** @} */

/*==============================================*/
/**
 * @fn         rsi_bt_set_feature_bitmap
 *
 */
int32_t rsi_bt_set_feature_bitmap(uint32_t feature_bit_map);

#ifdef __cplusplus
}
#endif

#endif
