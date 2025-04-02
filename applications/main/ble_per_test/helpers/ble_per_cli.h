#pragma once
#include <furi.h>
#include "../ble_per_test_i.h"

typedef struct {
    uint8_t mode_work;
    uint8_t mode;
    uint8_t channel;
    uint8_t rate;
    uint8_t payload_len;
    uint8_t payload_type;
    uint8_t hopping;
    uint8_t tx_power;

} BlePerCliSettings;

typedef enum {
    BLEPerCliSettingsModeTx,
    BLEPerCliSettingsModeRx,
} BLEPerCliSettingsMode;

bool ble_per_cli_start(BlePerTest* app_hendle, BlePerCliSettings settings);
void ble_per_cli_stop(void);
