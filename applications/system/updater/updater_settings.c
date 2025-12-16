#include "updater_settings.h"

#include <storage/storage.h>

#include <furi.h>
#include <json_helper.h>

#define TAG "UpdaterSettings"

#define UPDATER_SETTINGS_FILE APP_DATA_PATH("settings.json")

typedef enum {
    UpdaterSettingIdxCheckUrl,
    UpdaterSettingIdxCheckChannelId,
    UpdaterSettingIdxCheckStartupInterval,
    UpdaterSettingIdxCheckInterval,
    UpdaterSettingIdxAutoupdateEnabled,
    UpdaterSettingIdxAutoupdateIntervalStart,
    UpdaterSettingIdxAutoupdateIntervalEnd,

    UpdaterSettingIdxsCount,
} UpdaterSettingIdx;

typedef enum {
    UpdaterSettingTypeInt,
    UpdaterSettingTypeString,
    UpdaterSettingTypeBool,

    UpdaterSettingTypesCount,
} UpdaterSettingType;

typedef struct {
    const char* name;
    const void* default_value;
    bool (*is_value_valid)(const void* value);
    UpdaterSettingType type;
} UpdaterSetting;

typedef void (*UpdaterLoadSetting)(JsonConfig* config, const UpdaterSetting* setting, void* value);
typedef void (
    *UpdaterSaveSetting)(JsonConfig* config, const UpdaterSetting* setting, const void* value);

typedef struct {
    UpdaterLoadSetting load;
    UpdaterSaveSetting save;
} UpdaterSettingTypeActions;

static const UpdaterSetting settings[];
static const UpdaterSettingTypeActions setting_type_actions[];

static void
    updater_load_int_setting(JsonConfig* config, const UpdaterSetting* setting, void* value) {
    furi_assert(setting->type == UpdaterSettingTypeInt);

    int read_value;
    JsonConfigStatus read_status =
        json_config_read_int(config, setting->name, &read_value, setting->default_value);

    if(read_status != JsonConfigStatusOk) {
        FURI_LOG_W(TAG, "Failed to load %s setting, using default", setting->name);
    } else if(setting->is_value_valid && !setting->is_value_valid(&read_value)) {
        FURI_LOG_W(TAG, "Invalid %s setting, loading default", setting->name);
        read_value = *(const int*)setting->default_value;
    }

    *(int*)value = read_value;
}

static void
    updater_save_int_setting(JsonConfig* config, const UpdaterSetting* setting, const void* value) {
    furi_assert(setting->type == UpdaterSettingTypeInt);

    if(!setting->is_value_valid || setting->is_value_valid(value)) {
        int _value = *(const int*)value;
        json_config_write_int(config, setting->name, _value);
    } else {
        FURI_LOG_W(TAG, "Invalid %s setting save attempt", setting->name);
    }
}

static void
    updater_load_string_setting(JsonConfig* config, const UpdaterSetting* setting, void* value) {
    furi_assert(setting->type == UpdaterSettingTypeString);

    FuriString* read_value = furi_string_alloc();
    JsonConfigStatus read_status =
        json_config_read_str(config, setting->name, read_value, setting->default_value);
    const char* _read_value = furi_string_get_cstr(read_value);

    if(read_status != JsonConfigStatusOk) {
        FURI_LOG_W(TAG, "Failed to load %s setting, using default", setting->name);
    } else if(setting->is_value_valid && !setting->is_value_valid(_read_value)) {
        FURI_LOG_W(TAG, "Invalid %s setting, loading default", setting->name);
        _read_value = setting->default_value;
    }

    strcpy(value, _read_value);
    furi_string_free(read_value);
}

static void updater_save_string_setting(
    JsonConfig* config,
    const UpdaterSetting* setting,
    const void* value) {
    furi_assert(setting->type == UpdaterSettingTypeString);

    if(!setting->is_value_valid || setting->is_value_valid(value)) {
        json_config_write_str(config, setting->name, value);
    } else {
        FURI_LOG_W(TAG, "Invalid %s setting save attempt", setting->name);
    }
}

static void
    updater_load_bool_setting(JsonConfig* config, const UpdaterSetting* setting, void* value) {
    furi_assert(setting->type == UpdaterSettingTypeBool);

    bool read_value;
    JsonConfigStatus read_status =
        json_config_read_bool(config, setting->name, &read_value, setting->default_value);

    if(read_status != JsonConfigStatusOk) {
        FURI_LOG_W(TAG, "Failed to load %s setting, using default", setting->name);
    } else if(setting->is_value_valid && !setting->is_value_valid(&read_value)) {
        FURI_LOG_W(TAG, "Invalid %s setting, loading default", setting->name);
        read_value = *(const bool*)setting->default_value;
    }

    *(bool*)value = read_value;
}

static void updater_save_bool_setting(
    JsonConfig* config,
    const UpdaterSetting* setting,
    const void* value) {
    furi_assert(setting->type == UpdaterSettingTypeBool);

    if(!setting->is_value_valid || setting->is_value_valid(value)) {
        bool _value = *(const bool*)value;
        json_config_write_bool(config, setting->name, _value);
    } else {
        FURI_LOG_W(TAG, "Invalid %s setting save attempt", setting->name);
    }
}

static void updater_load_setting(JsonConfig* config, UpdaterSettingIdx setting_idx, void* value) {
    const UpdaterSetting* setting = &settings[setting_idx];
    setting_type_actions[setting->type].load(config, setting, value);
}

static void
    updater_save_setting(JsonConfig* config, UpdaterSettingIdx setting_idx, const void* value) {
    const UpdaterSetting* setting = &settings[setting_idx];
    setting_type_actions[setting->type].save(config, setting, value);
}

static bool is_check_url_valid(const void* check_url) {
    size_t check_url_length = strlen(check_url);

    return check_url_length >= UPDATER_CHECK_URL_MIN_LEN &&
           check_url_length <= UPDATER_CHECK_URL_MAX_LEN - 1;
}

static bool is_check_channel_id_valid(const void* check_channel_id) {
    size_t check_channel_id_length = strlen(check_channel_id);

    return check_channel_id_length >= UPDATER_CHECK_CHANNEL_ID_MIN_LEN &&
           check_channel_id_length <= UPDATER_CHECK_CHANNEL_ID_MAX_LEN - 1;
}

static bool is_check_startup_interval_valid(const void* check_startup_interval) {
    int _check_startup_interval = *(const int*)check_startup_interval;

    return _check_startup_interval >= UPDATER_CHECK_STARTUP_INTERVAL_MIN &&
           _check_startup_interval <= UPDATER_CHECK_STARTUP_INTERVAL_MAX;
}

static bool is_check_interval_valid(const void* check_interval) {
    int _check_interval = *(const int*)check_interval;

    return _check_interval >= UPDATER_CHECK_INTERVAL_MIN &&
           _check_interval <= UPDATER_CHECK_INTERVAL_MAX;
}

static bool is_autoupdate_interval_start_valid(const void* autoupdate_interval_start) {
    int _autoupdate_interval_start = *(const int*)autoupdate_interval_start;

    return _autoupdate_interval_start >= UPDATER_AUTOUPDATE_INTERVAL_START_MIN &&
           _autoupdate_interval_start <= UPDATER_AUTOUPDATE_INTERVAL_START_MAX;
}

static bool is_autoupdate_interval_end_valid(const void* autoupdate_interval_end) {
    int _autoupdate_interval_end = *(const int*)autoupdate_interval_end;

    return _autoupdate_interval_end >= UPDATER_AUTOUPDATE_INTERVAL_END_MIN &&
           _autoupdate_interval_end <= UPDATER_AUTOUPDATE_INTERVAL_END_MAX;
}

bool updater_settings_load(UpdaterSettings* settings) {
    furi_check(settings);

    JsonConfig* config = json_config_alloc();
    JsonConfigStatus open_status = json_config_open(config, UPDATER_SETTINGS_FILE);

    if(open_status == JsonConfigStatusOk || open_status == JsonConfigStatusMissing) {
        updater_load_setting(config, UpdaterSettingIdxCheckUrl, &settings->check_url);
        updater_load_setting(config, UpdaterSettingIdxCheckChannelId, &settings->check_channel_id);
        updater_load_setting(
            config, UpdaterSettingIdxCheckStartupInterval, &settings->check_startup_interval);
        updater_load_setting(config, UpdaterSettingIdxCheckInterval, &settings->check_interval);
        updater_load_setting(
            config, UpdaterSettingIdxAutoupdateEnabled, &settings->autoupdate_enabled);
        updater_load_setting(
            config,
            UpdaterSettingIdxAutoupdateIntervalStart,
            &settings->autoupdate_interval_start);
        updater_load_setting(
            config, UpdaterSettingIdxAutoupdateIntervalEnd, &settings->autoupdate_interval_end);
    }

    JsonConfigStatus free_status = json_config_free(config);
    return (open_status == JsonConfigStatusOk || open_status == JsonConfigStatusMissing) &&
           free_status == JsonConfigStatusOk;
}

bool updater_settings_save(const UpdaterSettings* settings) {
    furi_check(settings);

    JsonConfig* config = json_config_alloc();
    JsonConfigStatus open_status = json_config_open(config, UPDATER_SETTINGS_FILE);

    if(open_status == JsonConfigStatusOk || open_status == JsonConfigStatusMissing) {
        updater_save_setting(config, UpdaterSettingIdxCheckUrl, &settings->check_url);
        updater_save_setting(config, UpdaterSettingIdxCheckChannelId, &settings->check_channel_id);
        updater_save_setting(
            config, UpdaterSettingIdxCheckStartupInterval, &settings->check_startup_interval);
        updater_save_setting(config, UpdaterSettingIdxCheckInterval, &settings->check_interval);
        updater_save_setting(
            config, UpdaterSettingIdxAutoupdateEnabled, &settings->autoupdate_enabled);
        updater_save_setting(
            config,
            UpdaterSettingIdxAutoupdateIntervalStart,
            &settings->autoupdate_interval_start);
        updater_save_setting(
            config, UpdaterSettingIdxAutoupdateIntervalEnd, &settings->autoupdate_interval_end);
    }

    JsonConfigStatus free_status = json_config_free(config);
    return (open_status == JsonConfigStatusOk || open_status == JsonConfigStatusMissing) &&
           free_status == JsonConfigStatusOk;
}

static const UpdaterSetting settings[] = {
    [UpdaterSettingIdxCheckUrl] =
        {
            .name = "check_url",
            .default_value = UPDATER_CHECK_URL_DEFAULT,
            .is_value_valid = is_check_url_valid,
            .type = UpdaterSettingTypeString,
        },
    [UpdaterSettingIdxCheckChannelId] =
        {
            .name = "check_channel_id",
            .default_value = UPDATER_CHECK_CHANNEL_ID_DEFAULT,
            .is_value_valid = is_check_channel_id_valid,
            .type = UpdaterSettingTypeString,
        },
    [UpdaterSettingIdxCheckStartupInterval] =
        {
            .name = "check_startup_interval",
            .default_value = &(const int){UPDATER_CHECK_STARTUP_INTERVAL_DEFAULT},
            .is_value_valid = is_check_startup_interval_valid,
            .type = UpdaterSettingTypeInt,
        },
    [UpdaterSettingIdxCheckInterval] =
        {
            .name = "check_interval",
            .default_value = &(const int){UPDATER_CHECK_INTERVAL_DEFAULT},
            .is_value_valid = is_check_interval_valid,
            .type = UpdaterSettingTypeInt,
        },
    [UpdaterSettingIdxAutoupdateEnabled] =
        {
            .name = "autoupdate_enabled",
            .default_value = &(const bool){UPDATER_AUTOUPDATE_ENABLED_DEFAULT},
            .is_value_valid = NULL,
            .type = UpdaterSettingTypeBool,
        },
    [UpdaterSettingIdxAutoupdateIntervalStart] =
        {
            .name = "autoupdate_interval_start",
            .default_value = &(const int){UPDATER_AUTOUPDATE_INTERVAL_START_DEFAULT},
            .is_value_valid = is_autoupdate_interval_start_valid,
            .type = UpdaterSettingTypeInt,
        },
    [UpdaterSettingIdxAutoupdateIntervalEnd] =
        {
            .name = "autoupdate_interval_end",
            .default_value = &(const int){UPDATER_AUTOUPDATE_INTERVAL_END_DEFAULT},
            .is_value_valid = is_autoupdate_interval_end_valid,
            .type = UpdaterSettingTypeInt,
        },
};

static_assert(COUNT_OF(settings) == UpdaterSettingIdxsCount);

static const UpdaterSettingTypeActions setting_type_actions[] = {
    [UpdaterSettingTypeInt] =
        {
            .load = updater_load_int_setting,
            .save = updater_save_int_setting,
        },
    [UpdaterSettingTypeString] =
        {
            .load = updater_load_string_setting,
            .save = updater_save_string_setting,
        },
    [UpdaterSettingTypeBool] =
        {
            .load = updater_load_bool_setting,
            .save = updater_save_bool_setting,
        },
};

static_assert(COUNT_OF(setting_type_actions) == UpdaterSettingTypesCount);
