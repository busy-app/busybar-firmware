/**
 * @file ble_nwp_headers.h
 * @brief Header files and definitions required for working with nwp
 */
#pragma once

#include "ble_config.h"
#include <sl_status.h>
#include <sl_wifi.h>
#include <sl_wifi_callback_framework.h>

#include "rsi_ble.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"

/**
 * @brief Check if notification enabled for characteristic
 * 
 * Proper Client Characteristic Configuration Descriptor (CCCD) must be provided.
 * See more info on CCCD in "Bluetooth Core Specification Version 6.0 | Volume 3, Part G Paragraph 3.3.3.3"
 */
#define BLE_CCCD_NOTIFICATION_ENABLED(cccd_value) ((cccd_value & 0x01) != 0)

/**
 * @brief Check if indication enabled for characteristic
 *
 * Proper Client Characteristic Configuration Descriptor (CCCD) must be provided
 * See more info on CCCD in "Bluetooth Core Specification Version 6.0 | Volume 3, Part G Paragraph 3.3.3.3"
 */
#define BLE_CCCD_INDICATION_ENABLED(cccd_value) ((cccd_value & 0x02) != 0)
