#include "sysctl_settings.h"

#include <storage/storage.h>

// Use an absolute path instead of APP_DATA_PATH() to avoid storage alias
// remapping. APP_DATA_PATH() resolves to /ext/apps_data/{calling_thread_appid}/…,
// which differs depending on which thread calls the API (startup vs. web server
// vs. CLI). The absolute path below is what APP_DATA_PATH() produces when called
// from the "sysctl" startup thread, and it stays stable for all callers.
#define SYSCTL_SETTINGS_DIR       EXT_PATH("apps_data/sysctl")
#define SYSCTL_SETTINGS_FILE_PATH SYSCTL_SETTINGS_DIR "/settings.json"
#define SYSCTL_SETTINGS_VERSION   1
#define SYSCTL_SETTINGS_ROOT      sysctl_settings_v1_root

bool sysctl_settings_load(SysctlSettings* settings) {
    furi_check(settings);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_simply_mkdir(storage, SYSCTL_SETTINGS_DIR);
    furi_record_close(RECORD_STORAGE);

    SettingProvider* provider =
        setting_provider_alloc(SYSCTL_SETTINGS_FILE_PATH, SYSCTL_SETTINGS_VERSION, NULL, 0);
    bool result = setting_provider_load(provider, &SYSCTL_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return result;
}

bool sysctl_settings_save(const SysctlSettings* settings) {
    furi_check(settings);

    SettingProvider* provider =
        setting_provider_alloc(SYSCTL_SETTINGS_FILE_PATH, SYSCTL_SETTINGS_VERSION, NULL, 0);
    bool result = setting_provider_save(provider, &SYSCTL_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return result;
}
