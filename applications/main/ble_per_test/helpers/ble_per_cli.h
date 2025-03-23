#pragma once
#include <furi.h>

typedef struct {
    uint8_t mode_work;
    uint8_t mode;
    uint8_t channel;
    uint8_t tx_power;
    uint8_t rate;
    uint8_t payload_type;
    bool start_test;
} BlePerCliSettings;

typedef enum {
    BLEPerCliSettingsModeWorkCarrier,
    BLEPerCliSettingsModeWorkPacket,
} BLEPerCliSettingsModeWork;

typedef enum {
    BLEPerCliSettingsModeTx,
    BLEPerCliSettingsModeRx,
    BLEPerCliSettingsModeHopping,
} BLEPerCliSettingsMode;

void ble_per_cli_start(BlePerCliSettings settings);
void ble_per_cli_stop(void);