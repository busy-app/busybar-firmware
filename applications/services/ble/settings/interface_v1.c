#include "interface_v1.h"

const SettingProviderSetting ble_v1_settings[] = {
    [BleSettingsV1IdxEnabled] =
        {
            .name = "enabled",
            .interface = &(const SettingProviderBoolInterface){.default_value = false},
            .field_offset = offsetof(BleSettingsV1, enabled),
            .type = SettingProviderSettingTypeBool,
        },
};

const SettingProviderSetting ble_v1_settings_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructureInterface){
            .is_valid_callback = NULL,
            .inner_settings = ble_v1_settings,
            .inner_settings_count = COUNT_OF(ble_v1_settings),
        },
    .field_offset = 0,
    .type = SettingProviderSettingTypeStructure,
};

static_assert(COUNT_OF(ble_v1_settings) == BleSettingsV1IdxsCount);
