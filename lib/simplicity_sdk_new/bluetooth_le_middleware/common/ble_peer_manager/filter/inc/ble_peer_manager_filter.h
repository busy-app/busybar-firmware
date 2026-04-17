/***************************************************************************//**
 * @file
 * @brief Bluetooth Peer Manager - filter for scan responses
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
#ifndef BLE_PEER_MANAGER_FILTER_H
#define BLE_PEER_MANAGER_FILTER_H

/***********************************************************************************************//**
 * @addtogroup ble_peer_manager_filter
 * @{
 **************************************************************************************************/

#include <stdbool.h>
#include "sl_status.h"
#include "sl_bt_api.h"

/**************************************************************************//**
 * Initialize filtering.
 *****************************************************************************/
void ble_peer_manager_filter_init(void);

/**************************************************************************//**
 * Enable/disable BT address filtering.
 *
 * @param[in] enabled Enable or disable BT address filtering.
 *****************************************************************************/
void ble_peer_manager_set_filter_bt_address(bool enabled);

/**************************************************************************//**
 * Add a new allowed BT address.
 *
 * @param[in] filter_addr BT address to allow
 *
 * @retval SL_STATUS_OK if successful
 * @retval SL_STATUS_INVALID_STATE if scanning is active
 * @retval SL_STATUS_INVALID_PARAMETER if the address is NULL
 *****************************************************************************/
sl_status_t ble_peer_manager_add_allowed_bt_address(bd_addr *filter_addr);

/**************************************************************************//**
 * Remove a BT address from the allowed list.
 *
 * @retval SL_STATUS_OK if successful
 *****************************************************************************/
sl_status_t ble_peer_manager_remove_allowed_bt_address(bd_addr *filter_addr);

/**************************************************************************//**
 * Set filter address type.
 *
 * @param[in] filter_addr_type Address type to set the filter to.
 *
 * @retval SL_STATUS_OK if successful
 * @retval SL_STATUS_INVALID_STATE if scanning is active
 *****************************************************************************/
sl_status_t ble_peer_manager_set_filter_address_type(sl_bt_gap_address_type_t filter_addr_type);

/**************************************************************************//**
 * Set filter device name.
 *
 * Set the filter device name to the specified device name.
 * The device name in the scan response will only be checked if the
 * advertisement type is Shortened Local Name (0x08) or
 * Complete Local Name (0x09).
 *
 * @param[in] device_name Device name to set the filter to.
 * @param[in] device_name_len Length of the device name.
 * @param[in] full_match If true, only full matches are accepted
 *                       in the scan response.
 *
 * @retval SL_STATUS_OK if successful
 * @retval SL_STATUS_INVALID_STATE if scanning is active
 * @retval SL_STATUS_INVALID_PARAMETER if the device name is NULL
 * @retval SL_STATUS_ALLOCATION_FAILED if memory allocation failed
 *****************************************************************************/
sl_status_t ble_peer_manager_set_filter_device_name(const char *device_name,
                                                    uint8_t device_name_len,
                                                    bool full_match);

/**************************************************************************//**
 * Set filter service UUID16.
 *
 * Set the filter service UUID16 to the specified service UUID16.
 * The service UUID in the scan response will only be checked if the
 * advertisement type is Incomplete List of 16-bit Service Class UUIDs or
 * Complete List of 16-bit Service Class UUIDs.
 *
 * @param[in] service_uuid16 Service UUID16 to set the filter to.
 *
 * @retval SL_STATUS_OK if successful
 * @retval SL_STATUS_INVALID_STATE if scanning is active
 * @retval SL_STATUS_INVALID_PARAMETER if the service UUID16 is NULL
 *****************************************************************************/
sl_status_t ble_peer_manager_set_filter_service_uuid16(sl_bt_uuid_16_t *service_uuid16);

/**************************************************************************//**
 * Set filter service UUID128.
 *
 * Set the filter service UUID128 to the specified service UUID128.
 * The service UUID in the scan response will only be checked if the
 * advertisement type is Incomplete List of 32-bit Service Class UUIDs or
 * Complete List of 16-bit Service Class UUIDs.
 *
 * @param[in] service_uuid128 Service UUID128 to set the filter to.
 *
 * @retval SL_STATUS_OK if successful
 * @retval SL_STATUS_INVALID_STATE if scanning is active
 * @retval SL_STATUS_INVALID_PARAMETER if the service UUID128 is NULL
 *****************************************************************************/
sl_status_t ble_peer_manager_set_filter_service_uuid128(uuid_128 *service_uuid128);

/**************************************************************************//**
 * Set filter service data.
 *
 * Set the filter service data to the specified service data. When
 * processing the scan response, the search will start from the given
 * offset until the given length using memcmp().
 * The service data in the scan response will only be checked if the
 * advertisement type is Service Data - 128-bit UUID.
 *
 * @param[in] service_data Service data to set the filter to.
 * @param[in] service_data_offset Offset of the service data.
 * @param[in] service_data_len Length of the service data.
 *
 * @retval SL_STATUS_OK if successful
 * @retval SL_STATUS_INVALID_STATE if scanning is active
 * @retval SL_STATUS_INVALID_PARAMETER if the service data is NULL
 * @retval SL_STATUS_ALLOCATION_FAILED if memory allocation failed
 *****************************************************************************/
sl_status_t ble_peer_manager_set_filter_service_data(uint8_t *service_data,
                                                     uint8_t service_data_offset,
                                                     uint8_t service_data_len);

/**************************************************************************//**
 * Set filter manufacturer data.
 *
 * Set the filter manufacturer data to the specified manufacturer data.
 * When processing the scan response, the search will start from the given
 * offset until the given length using memcmp().
 * The manufacturer data in the scan response will only be checked if the
 * advertisement type is Manufacturer Specific Data.
 *
 * @param[in] manufacturer_data Manufacturer data to set the filter to.
 * @param[in] manufacturer_data_offset Offset of the manufacturer data.
 * @param[in] manufacturer_data_len Length of the manufacturer data.
 *
 * @retval SL_STATUS_OK if successful
 * @retval SL_STATUS_INVALID_STATE if scanning is active
 * @retval SL_STATUS_INVALID_PARAMETER if the manufacturer data is NULL
 * @retval SL_STATUS_ALLOCATION_FAILED if memory allocation failed
 *****************************************************************************/
sl_status_t ble_peer_manager_set_filter_manufacturer_data(uint8_t *manufacturer_data,
                                                          uint8_t manufacturer_data_offset,
                                                          uint8_t manufacturer_data_len);

/**************************************************************************//**
 * Set filter RSSI.
 *
 * Set the filter RSSI (Signal strength indicator in the last received packet)
 * to the specified RSSI. The RSSI in the scan response has to be greater
 * than the set value.
 * Units: dBm
 * Range: -127 to +20
 *
 * @param[in] rssi RSSI to set the filter to.
 *
 * @retval SL_STATUS_OK if successful
 * @retval SL_STATUS_INVALID_STATE if scanning is active
 * @retval SL_STATUS_INVALID_PARAMETER if the RSSI is greater than zero.
 *****************************************************************************/
sl_status_t ble_peer_manager_set_filter_rssi(int8_t rssi);

/**************************************************************************//**
 * Check if there are any filters set.
 *
 * @retval true if there are filters set
 * @retval false if there are no filters set
 *****************************************************************************/
bool ble_peer_manager_is_filter_set(void);

/**************************************************************************//**
 * Reset filters.
 *
 * Reset all filters to default values.
 *
 * @retval SL_STATUS_OK if successful
 * @retval SL_STATUS_INVALID_STATE if not allowed to modify filters
 *****************************************************************************/
sl_status_t ble_peer_manager_reset_filter(void);

/**************************************************************************//**
 * Find a match for the filters.
 *
 * Check if the scan response matches the filters. There is an AND relationship
 * between the filters, meaning that all filters must match for the function
 * to return true.
 *
 * @param[in] address Address of the device.
 * @param[in] address_type Address type of the device.
 * @param[in] rssi RSSI of the device.
 * @param[in] adv_data Advertising data of the device.
 *
 * @retval true if the scan response matches the filters
 * @retval false if the scan response does not match one or more filters
 *****************************************************************************/
bool ble_peer_manager_find_match(bd_addr *address,
                                 sl_bt_gap_address_type_t address_type,
                                 int8_t rssi,
                                 const uint8array *adv_data);

bool ble_peer_manager_is_filter_set_allowed(void);

/**************************************************************************//**
 * Parse BT address from string.
 *
 * @param[in] str Hexadecimal representation of the BLE address with optional
 *                separator characters ':' or ' ', e.g. "AA:BB:CC:DD:EE:FF".
 * @param[out] address Output.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t ble_peer_manager_str_to_address(const char *str, bd_addr *address);

/** @} (end addtogroup ble_peer_manager_filter) */
#endif // BLE_PEER_MANAGER_FILTER_H
