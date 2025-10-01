#pragma once
#include <storage/storage.h>

#define UPDATE_CHECKER_SETTINGS_FILE APP_DATA_PATH("config.json")

#define UPDATE_CHECKER_JSON_URL_DEFAULT \
    "https://update.flipperzero.one/busybar-firmware/directory.json"
#define UPDATE_CHECKER_JSON_CHANNEL_ID_DEFAULT "development"
#define UPDATE_CHECKER_JSON_URL_DIRECTORY      "url_directory_json"
#define UPDATE_CHECKER_JSON_CURRENT_CHANNEL    "current_channel"

#define UPDATE_CHECKER_JSON_REBOOT_EXAMINATION_INTERVAL         "reboot_examination_interval"
#define UPDATE_CHECKER_JSON_EXAMINATION_INTERVAL                "examination_interval"
#define UPDATE_CHECKER_JSON_REBOOT_EXAMINATION_INTERVAL_DEFAULT (1 * 1000 * 60) // 1 minute
#define UPDATE_CHECKER_JSON_EXAMINATION_INTERVAL_DEFAULT        (5 * 1000 * 60 * 60) // 5 hours
