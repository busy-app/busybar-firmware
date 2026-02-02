#pragma once

#include <stdbool.h>

#define BUSY_SETTINGS_THEME_NAME_LEN (64)

typedef enum {
    BusySettingsProfileIdBusy,
    BusySettingsProfileIdCustom,
    BusySettingsProfileIdMax,
} BusySettingsProfileId;

typedef struct {
    char theme_name[BUSY_SETTINGS_THEME_NAME_LEN + 1];
} BusySettings;

bool busy_settings_load(BusySettings* settings, BusySettingsProfileId profile_id);

bool busy_settings_save(const BusySettings* settings, BusySettingsProfileId profile_id);

void busy_settings_set_default(BusySettings* settings, BusySettingsProfileId profile_id);
