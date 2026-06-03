#pragma once

#include <furi.h>

#define BLE_DEVICE_ADDRESS_LEN (6)

typedef struct {
    uint8_t dev_addr[BLE_DEVICE_ADDRESS_LEN];
    uint8_t features[8];
} BleDeviceCommon;
