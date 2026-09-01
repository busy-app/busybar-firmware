#include "telemetry_settings.h"
#include "telemetry_i.h"

#include <storage/storage.h>
#include <setting_provider/setting_provider.h>

typedef enum {
    TelemetrySettingsV1IdxEnabled,
    TelemetrySettingsV1IdxMax,
} TelemetrySettingsV1Idx;

static const SettingProviderSetting telemetry_settings_v1[] = {
    [TelemetrySettingsV1IdxEnabled] =
        {
            .name = "enabled",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = true,
                },
            .field_offset = offsetof(TelemetrySettings, is_enabled),
            .type = SettingProviderSettingTypeBool,
        },
};

static const SettingProviderSetting telemetry_settings_v1_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructInterface){
            .inner_settings = telemetry_settings_v1,
            .inner_settings_count = COUNT_OF(telemetry_settings_v1),
        },
    .type = SettingProviderSettingTypeStruct,
};

static_assert(COUNT_OF(telemetry_settings_v1) == TelemetrySettingsV1IdxMax);

void telemetry_settings_load(TelemetrySettings* settings) {
    furi_assert(settings);

    SettingProvider* provider =
        setting_provider_alloc(TELEMETRY_SETTINGS_FILE_PATH, TELEMETRY_SETTINGS_VERSION, NULL, 0);
    setting_provider_load(provider, &telemetry_settings_v1_root, settings);
    setting_provider_free(provider);
}

void telemetry_settings_save(const TelemetrySettings* settings) {
    furi_assert(settings);

    SettingProvider* provider =
        setting_provider_alloc(TELEMETRY_SETTINGS_FILE_PATH, TELEMETRY_SETTINGS_VERSION, NULL, 0);
    setting_provider_save(provider, &telemetry_settings_v1_root, settings);
    setting_provider_free(provider);
}
