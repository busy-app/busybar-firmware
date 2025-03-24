#pragma once
#include <furi.h>

typedef struct {
    uint8_t mode_work;
    uint8_t mode;
    uint8_t channel;
    uint8_t rate;
    uint8_t payload_len;
    uint8_t payload_type;
    uint8_t hopping;
    uint8_t tx_power;
   
    bool start_test;
} BlePerCliSettings;


typedef enum {
    BLEPerCliSettingsModeTx,
    BLEPerCliSettingsModeRx,
} BLEPerCliSettingsMode;

void ble_per_cli_start(BlePerCliSettings settings);
void ble_per_cli_stop(void);