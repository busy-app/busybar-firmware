#pragma once

#include "setting_provider.h"

#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TAG "SettingProvider"

struct SettingProvider {
    Storage* storage;

    FuriString* file_path;
    const SettingProviderMigration* migrations;
    size_t migrations_count;
    int settings_version;

    cJSON* json_root;
    cJSON* json_version;
    cJSON* json_values;
};

#ifdef __cplusplus
}
#endif
