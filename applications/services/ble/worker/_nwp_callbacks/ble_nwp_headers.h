#pragma once

#include "ble_config.h"
#include <sl_status.h>
#include <sl_wifi.h>
#include <sl_wifi_callback_framework.h>

#include "rsi_ble.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"

#define BLE_CCCD_NOTIFICATION_ENABLED(cccd_value) ((cccd_value & 0x01) != 0)
#define BLE_CCCD_INDICATION_ENABLED(cccd_value)   ((cccd_value & 0x02) != 0)
