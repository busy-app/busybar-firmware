#pragma once
#include <storage/storage.h>

#define CHECK_UPDATE_FW_SETTINGS_FILE APP_DATA_PATH("config.json")

#define CHECK_UPDATE_FW_JSON_URL_DEFAULT \
    "https://update.flipperzero.one/busybar-firmware/directory.json"
#define CHECK_UPDATE_FW_JSON_CHANNEL_ID_DEFAULT "development"
#define CHECK_UPDATE_FW_JSON_URL_DIRECTORY      "url_directory_json"
#define CHECK_UPDATE_FW_JSON_CURRENT_CHANNEL    "current_channel"

#define CHECK_UPDATE_FW_JSON_REBOOT_EXAMINATION_INTERVAL         "reboot_examination_interval"
#define CHECK_UPDATE_FW_JSON_EXAMINATION_INTERVAL                "examination_interval"
#define CHECK_UPDATE_FW_JSON_REBOOT_EXAMINATION_INTERVAL_DEFAULT (1 * 1000 * 60) // 1 minute
#define CHECK_UPDATE_FW_JSON_EXAMINATION_INTERVAL_DEFAULT        (5 * 1000 * 60 * 60) // 5 hours
