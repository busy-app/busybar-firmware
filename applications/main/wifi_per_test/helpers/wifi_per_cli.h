#pragma once
#include <furi.h>
#include "../wifi_per_test_i.h"

typedef struct {
    char* mode_work;
    uint8_t mode;
    uint8_t channel;
    char* rate;
    uint8_t tx_power;

    bool start_test;
} WifiPerCliSettings;

typedef enum {
    BLEPerCliSettingsModeTx,
    BLEPerCliSettingsModeRx,
} BLEPerCliSettingsMode;

void wifi_per_cli_start(WifiPerTest* app_hendle, WifiPerCliSettings settings);
void wifi_per_cli_stop(void);
