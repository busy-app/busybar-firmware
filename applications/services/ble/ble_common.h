#pragma once

#include <furi.h>
#include "ble_intercom_types.h"

// #define BLE_DEBUG

#ifdef BLE_DEBUG
#define BLE_LOG_D(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define BLE_LOG_D(...)
#endif

#define BLE_LOG_I(...) FURI_LOG_I(TAG, __VA_ARGS__)
#define BLE_LOG_W(...) FURI_LOG_W(TAG, __VA_ARGS__)
