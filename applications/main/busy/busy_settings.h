#pragma once

#include <stdbool.h>

#define BUSY_SETTINGS_THEME_NAME_LEN (64)

typedef struct {
    char theme_name[BUSY_SETTINGS_THEME_NAME_LEN + 1];
} BusySettings;

bool busy_settings_load(BusySettings* settings);

bool busy_settings_save(const BusySettings* settings);
