#include "settings.h"

#include <storage/storage.h>
#include <toolbox/setting_provider.h>

#define TAG "SntpSettings"

#define SNTP_SETTINGS_FILE    APP_DATA_PATH("settings.json")
#define SNTP_SETTINGS_VERSION 1

static const SettingProviderMigration sntp_migrations[] = {
    {
        .source_version = 0,
        .target_version = 1,
        .callback = sntp_settings_v1_migrate_from_v0,
    },
};

bool sntp_settings_load_draft(SntpSettingsV1* settings) {
    furi_assert(settings);

    bool is_success = false;
    SettingProvider* provider = setting_provider_alloc(
        SNTP_SETTINGS_FILE, SNTP_SETTINGS_VERSION, sntp_migrations, COUNT_OF(sntp_migrations));

    do {
        if(!setting_provider_open(provider)) {
            break;
        }

        for(size_t i = 0; i < SntpSettingV1IdxsCount; i++) {
            const SettingProviderSetting* setting = &sntp_v1_settings[i];
            size_t offset = *(size_t*)setting->context;
            void* field_ptr = (void*)settings + offset;

            setting_provider_load(provider, setting, field_ptr);
        }

        is_success = true;
    } while(false);

    setting_provider_free(provider);

    return is_success;
}

bool sntp_settings_save_draft(const SntpSettingsV1* settings) {
    furi_assert(settings);

    bool is_success = false;
    SettingProvider* provider = setting_provider_alloc(
        SNTP_SETTINGS_FILE, SNTP_SETTINGS_VERSION, sntp_migrations, COUNT_OF(sntp_migrations));

    do {
        if(!setting_provider_open(provider)) {
            break;
        }

        for(size_t i = 0; i < SntpSettingV1IdxsCount; i++) {
            const SettingProviderSetting* setting = &sntp_v1_settings[i];
            size_t offset = *(size_t*)setting->context;
            void* field_ptr = (void*)settings + offset;

            setting_provider_save(provider, setting, field_ptr);
        }

        is_success = true;
    } while(false);

    setting_provider_free(provider);

    return is_success;
}
