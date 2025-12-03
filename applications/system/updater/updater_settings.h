#pragma once

#include <furi.h>

#define UPDATER_CHECK_URL_MIN_LEN 1
#define UPDATER_CHECK_URL_MAX_LEN 128
#define UPDATER_CHECK_URL_DEFAULT "https://update.flipperzero.one/busybar-firmware/directory.json"

#define UPDATER_CHECK_CHANNEL_ID_MIN_LEN 1
#define UPDATER_CHECK_CHANNEL_ID_MAX_LEN 64
#define UPDATER_CHECK_CHANNEL_ID_DEFAULT "development"

#define UPDATER_CHECK_STARTUP_INTERVAL_MIN     (1 * 1000 * 60)
#define UPDATER_CHECK_STARTUP_INTERVAL_MAX     (20 * 1000 * 60)
#define UPDATER_CHECK_STARTUP_INTERVAL_DEFAULT (10 * 1000 * 60)

#define UPDATER_CHECK_INTERVAL_MIN     (1 * 1000 * 60 * 60)
#define UPDATER_CHECK_INTERVAL_MAX     (5 * 1000 * 60 * 60)
#define UPDATER_CHECK_INTERVAL_DEFAULT (10 * 1000 * 60 * 60)

typedef struct {
    char check_url[UPDATER_CHECK_URL_MAX_LEN];
    char check_channel_id[UPDATER_CHECK_CHANNEL_ID_MAX_LEN];
    int check_startup_interval;
    int check_interval;
} UpdaterSettings;

bool updater_settings_load(UpdaterSettings* settings);

bool updater_settings_save(const UpdaterSettings* settings);
