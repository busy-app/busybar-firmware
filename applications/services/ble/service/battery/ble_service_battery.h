#pragma once

#include "../ble_service_config_types.h"

typedef enum {
    BleSrvBatteryCharacterIndexBatteryLevel,
    BleSrvBatteryCharacterIndexBatteryStatus,
} BleSrvBatteryCharacterIndex;

extern const BleServiceDescriptor ble_service_config_battery;
