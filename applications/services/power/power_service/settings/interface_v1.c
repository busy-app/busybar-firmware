#include "interface_v1.h"

typedef struct {
    int min;
    int max;
} IntMinMaxValidationContext;

static bool int_min_max_validate(const SettingProviderSetting* setting, int value) {
    const IntMinMaxValidationContext* context = setting->context;
    return value >= context->min && value <= context->max;
}

const SettingProviderSetting power_v1_settings[] = {
    [PowerSettingV1IdxChargeLimit] =
        {
            .name = "charge_limit",
            .interface =
                &(const SettingProviderIntInterface){
                    .is_valid_callback = int_min_max_validate,
                    .default_value = 100,
                },
            .context =
                &(const IntMinMaxValidationContext){
                    .min = 30,
                    .max = 100,
                },
            .field_offset = offsetof(PowerSettingsV1, charge_limit),
            .type = SettingProviderSettingTypeInt,
        },
};

static_assert(COUNT_OF(power_v1_settings) == PowerSettingV1IdxMAX);

const SettingProviderSetting power_v1_settings_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructInterface){
            .inner_settings = power_v1_settings,
            .inner_settings_count = COUNT_OF(power_v1_settings),
        },
    .field_offset = 0,
    .type = SettingProviderSettingTypeStruct,
};
