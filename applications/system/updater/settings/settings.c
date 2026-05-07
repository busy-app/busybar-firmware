#include "settings_i.h"

#include <storage/storage.h>

#define UPDATER_SETTINGS_FILE_PATH APP_DATA_PATH("settings.json")
#define UPDATER_SETTINGS_VERSION   2
#define UPDATER_SETTINGS_ROOT      updater_v2_settings_root

static const SettingProviderMigration updater_setings_migrations[] = {
    {
        .target_version = UPDATER_SETTINGS_VERSION,
        .migrate_callback = updater_settings_v2_migrate,
    },
};

bool updater_settings_reset(UpdaterSettings* settings) {
    SettingProvider* provider = setting_provider_alloc(
        UPDATER_SETTINGS_FILE_PATH,
        UPDATER_SETTINGS_VERSION,
        updater_setings_migrations,
        COUNT_OF(updater_setings_migrations));
    bool is_successful = setting_provider_reset(provider, &UPDATER_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}

bool updater_settings_load(UpdaterSettings* settings) {
    furi_check(settings);

    SettingProvider* provider = setting_provider_alloc(
        UPDATER_SETTINGS_FILE_PATH,
        UPDATER_SETTINGS_VERSION,
        updater_setings_migrations,
        COUNT_OF(updater_setings_migrations));
    bool is_successful = setting_provider_load(provider, &UPDATER_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}

bool updater_settings_save(const UpdaterSettings* settings) {
    furi_check(settings);

    SettingProvider* provider = setting_provider_alloc(
        UPDATER_SETTINGS_FILE_PATH,
        UPDATER_SETTINGS_VERSION,
        updater_setings_migrations,
        COUNT_OF(updater_setings_migrations));
    bool is_successful = setting_provider_save(provider, &UPDATER_SETTINGS_ROOT, settings);
    setting_provider_free(provider);

    return is_successful;
}
