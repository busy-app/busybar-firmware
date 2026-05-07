#include "sysctl_settings.h"

#include <storage/storage.h>

#define SYSCTL_SETTINGS_FILE_PATH APP_DATA_PATH("sysctl.json")
#define SYSCTL_SETTINGS_VERSION   1
#define SYSCTL_SETTINGS_ROOT      sysctl_settings_v1_root

bool sysctl_settings_load(SysctlSettings* settings) {
    furi_check(settings);

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
