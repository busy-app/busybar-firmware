#include "busy_timer_settings.h"

#include <storage/storage.h>

#define BUSY_TIMER_SETTINGS_VERSION             1
#define BUSY_TIMER_SETTINGS_ROOT                busy_timer_settings_v1_root
#define BUSY_TIMER_SETTINGS_APPLY_DEFAULTS(...) busy_timer_settings_v1_apply_defaults(__VA_ARGS__)

static const char* busy_timer_settings_file_paths[BusyTimerProfileIdMax] = {
    [BusyTimerProfileIdBusy] = APP_DATA_PATH("settings_busy.json"),
    [BusyTimerProfileIdCustom] = APP_DATA_PATH("settings_custom.json"),
};

void busy_timer_settings_load(BusyTimerSettings* settings, BusyTimerProfileId profile_id) {
    furi_check(settings);
    furi_check(profile_id < BusyTimerProfileIdMax);

    SettingProvider* provider = setting_provider_alloc(
        busy_timer_settings_file_paths[profile_id], BUSY_TIMER_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    setting_provider_load(provider, &BUSY_TIMER_SETTINGS_ROOT, settings);

    if(BUSY_TIMER_SETTINGS_APPLY_DEFAULTS(settings, profile_id)) {
        setting_provider_save(provider, &BUSY_TIMER_SETTINGS_ROOT, settings);
    }

    setting_provider_close(provider);
    setting_provider_free(provider);
}

void busy_timer_settings_save(const BusyTimerSettings* settings, BusyTimerProfileId profile_id) {
    furi_check(settings);
    furi_check(profile_id < BusyTimerProfileIdMax);

    SettingProvider* provider = setting_provider_alloc(
        busy_timer_settings_file_paths[profile_id], BUSY_TIMER_SETTINGS_VERSION, NULL, 0);

    setting_provider_open(provider);
    setting_provider_save(provider, &BUSY_TIMER_SETTINGS_ROOT, settings);
    setting_provider_close(provider);
    setting_provider_free(provider);
}
