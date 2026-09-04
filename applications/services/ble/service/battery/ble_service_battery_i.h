#pragma once

#include "ble_service_battery.h"
#include "../ble_service_i.h"

#include <furi.h>

typedef union FURI_PACKED {
    struct FURI_PACKED {
        uint16_t battery_present         : 1;
        uint16_t wired_source_present    : 2;
        uint16_t wireless_source_present : 2;
        uint16_t battery_charge_state    : 2;
        uint16_t battery_charge_level    : 2;
        uint16_t charging_type           : 3;
        uint16_t charging_fault_reason   : 3;
        uint16_t rfu                     : 1;
    } fields;
    uint16_t value;
} BatteryPowerState;

typedef struct FURI_PACKED {
    uint8_t flags;
    BatteryPowerState state;
} BatteryStatusInfo;

bool ble_service_battery_init(void* object);
bool ble_service_battery_run(void* object, size_t data_size, const void* data);
