#include "sntp_settings.h"

#include <storage/storage.h>

#include <furi.h>
#include <json_helper.h>

#define TAG "SntpSettings"

#define SNTP_SETTINGS_FILE APP_DATA_PATH("settings.json")

typedef enum {
    SntpSettingIdxTimezone,
    SntpSettingIdxAutoInterval,
    SntpSettingIdxBootInterval,
    SntpSettingIdxServerName,
    SntpSettingIdxIsEnabled,

    SntpSettingIdxsCount,
} SntpSettingIdx;

typedef enum {
    SntpSettingTypeInt,
    SntpSettingTypeString,
    SntpSettingTypeBool,

    SntpSettingTypesCount,
} SntpSettingType;

typedef struct {
    const char* name;
    const void* default_value;
    bool (*is_value_valid)(const void* value);
    SntpSettingType type;
} SntpSetting;

typedef void (*SntpLoadSetting)(JsonConfig* config, const SntpSetting* setting, void* value);
typedef void (*SntpSaveSetting)(JsonConfig* config, const SntpSetting* setting, const void* value);

typedef struct {
    SntpLoadSetting load;
    SntpSaveSetting save;
} SntpSettingTypeActions;

static const SntpSetting settings[];
static const SntpSettingTypeActions setting_type_actions[];

static void sntp_load_int_setting(JsonConfig* config, const SntpSetting* setting, void* value) {
    furi_assert(setting->type == SntpSettingTypeInt);

    int read_value;
    JsonConfigStatus read_status =
        json_config_read_int(config, setting->name, &read_value, setting->default_value);

    if(read_status == JsonConfigStatusMissing) {
        FURI_LOG_W(TAG, "Missing %s setting, loading default", setting->name);
    } else if(setting->is_value_valid && !setting->is_value_valid(&read_value)) {
        FURI_LOG_W(TAG, "Invalid %s setting, loading default", setting->name);
        read_value = *(const int*)setting->default_value;
    }

    *(int*)value = read_value;
}

static void
    sntp_save_int_setting(JsonConfig* config, const SntpSetting* setting, const void* value) {
    furi_assert(setting->type == SntpSettingTypeInt);

    if(!setting->is_value_valid || setting->is_value_valid(value)) {
        int _value = *(const int*)value;
        json_config_write_int(config, setting->name, _value);
    } else {
        FURI_LOG_W(TAG, "Invalid %s setting save attempt", setting->name);
    }
}

static void sntp_load_string_setting(JsonConfig* config, const SntpSetting* setting, void* value) {
    furi_assert(setting->type == SntpSettingTypeString);

    FuriString* read_value = furi_string_alloc();
    JsonConfigStatus read_status =
        json_config_read_str(config, setting->name, read_value, setting->default_value);
    const char* _read_value = furi_string_get_cstr(read_value);

    if(read_status == JsonConfigStatusMissing) {
        FURI_LOG_W(TAG, "Missing %s setting, loading default", setting->name);
    } else if(setting->is_value_valid && !setting->is_value_valid(_read_value)) {
        FURI_LOG_W(TAG, "Invalid %s setting, loading default", setting->name);
        _read_value = setting->default_value;
    }

    strcpy(value, _read_value);
    furi_string_free(read_value);
}

static void
    sntp_save_string_setting(JsonConfig* config, const SntpSetting* setting, const void* value) {
    furi_assert(setting->type == SntpSettingTypeString);

    if(!setting->is_value_valid || setting->is_value_valid(value)) {
        json_config_write_str(config, setting->name, value);
    } else {
        FURI_LOG_W(TAG, "Invalid %s setting save attempt", setting->name);
    }
}

static void sntp_load_bool_setting(JsonConfig* config, const SntpSetting* setting, void* value) {
    furi_assert(setting->type == SntpSettingTypeBool);

    bool read_value;
    JsonConfigStatus read_status =
        json_config_read_bool(config, setting->name, &read_value, setting->default_value);

    if(read_status == JsonConfigStatusMissing) {
        FURI_LOG_W(TAG, "Missing %s setting, loading default", setting->name);
    } else if(setting->is_value_valid && !setting->is_value_valid(&read_value)) {
        FURI_LOG_W(TAG, "Invalid %s setting, loading default", setting->name);
        read_value = *(const bool*)setting->default_value;
    }

    *(bool*)value = read_value;
}

static void
    sntp_save_bool_setting(JsonConfig* config, const SntpSetting* setting, const void* value) {
    furi_assert(setting->type == SntpSettingTypeBool);

    if(!setting->is_value_valid || setting->is_value_valid(value)) {
        bool _value = *(const bool*)value;
        json_config_write_bool(config, setting->name, _value);
    } else {
        FURI_LOG_W(TAG, "Invalid %s setting save attempt", setting->name);
    }
}

static void sntp_load_setting(JsonConfig* config, SntpSettingIdx setting_idx, void* value) {
    const SntpSetting* setting = &settings[setting_idx];
    setting_type_actions[setting->type].load(config, setting, value);
}

static void sntp_save_setting(JsonConfig* config, SntpSettingIdx setting_idx, const void* value) {
    const SntpSetting* setting = &settings[setting_idx];
    setting_type_actions[setting->type].save(config, setting, value);
}

static bool is_timezone_offset_valid(const void* timezone_offset) {
    int _timezone_offset = *(const int*)timezone_offset;

    return _timezone_offset >= SNTP_TIMEZONE_OFFSET_MIN &&
           _timezone_offset <= SNTP_TIMEZONE_OFFSET_MAX;
}

static bool is_auto_interval_valid(const void* auto_interval) {
    int _auto_interval = *(const int*)auto_interval;

    return _auto_interval >= SNTP_AUTO_INTERVAL_MIN && _auto_interval <= SNTP_AUTO_INTERVAL_MAX;
}

static bool is_boot_interval_valid(const void* boot_interval) {
    int _boot_interval = *(const int*)boot_interval;

    return _boot_interval >= SNTP_BOOT_INTERVAL_MIN && _boot_interval <= SNTP_BOOT_INTERVAL_MAX;
}

static bool is_server_name_valid(const void* server_name) {
    size_t server_name_length = strlen(server_name);

    return server_name_length >= SNTP_SERVER_NAME_MIN_LEN &&
           server_name_length <= SNTP_SERVER_NAME_MAX_LEN - 1;
}

bool sntp_settings_load(SntpSettings* settings) {
    furi_check(settings);

    JsonConfig* config = json_config_alloc();
    JsonConfigStatus open_status = json_config_open(config, SNTP_SETTINGS_FILE);

    if(open_status == JsonConfigStatusOk || open_status == JsonConfigStatusMissing) {
        sntp_load_setting(config, SntpSettingIdxTimezone, &settings->timezone_offset);
        sntp_load_setting(config, SntpSettingIdxAutoInterval, &settings->auto_interval);
        sntp_load_setting(config, SntpSettingIdxBootInterval, &settings->boot_interval);
        sntp_load_setting(config, SntpSettingIdxServerName, &settings->server_name);
        sntp_load_setting(config, SntpSettingIdxIsEnabled, &settings->is_enabled);
    }

    JsonConfigStatus free_status = json_config_free(config);
    return (open_status == JsonConfigStatusOk || open_status == JsonConfigStatusMissing) &&
           free_status == JsonConfigStatusOk;
}

bool sntp_settings_save(const SntpSettings* settings) {
    furi_check(settings);

    JsonConfig* config = json_config_alloc();
    JsonConfigStatus open_status = json_config_open(config, SNTP_SETTINGS_FILE);

    if(open_status == JsonConfigStatusOk || open_status == JsonConfigStatusMissing) {
        sntp_save_setting(config, SntpSettingIdxTimezone, &settings->timezone_offset);
        sntp_save_setting(config, SntpSettingIdxAutoInterval, &settings->auto_interval);
        sntp_save_setting(config, SntpSettingIdxBootInterval, &settings->boot_interval);
        sntp_save_setting(config, SntpSettingIdxServerName, &settings->server_name);
        sntp_save_setting(config, SntpSettingIdxIsEnabled, &settings->is_enabled);
    }

    JsonConfigStatus free_status = json_config_free(config);
    return (open_status == JsonConfigStatusOk || open_status == JsonConfigStatusMissing) &&
           free_status == JsonConfigStatusOk;
}

static const SntpSetting settings[] = {
    [SntpSettingIdxTimezone] =
        {
            .name = "timezone",
            .default_value = &(const int){SNTP_TIMEZONE_OFFSET_DEFAULT},
            .is_value_valid = is_timezone_offset_valid,
            .type = SntpSettingTypeInt,
        },
    [SntpSettingIdxAutoInterval] =
        {
            .name = "auto_interval",
            .default_value = &(const int){SNTP_AUTO_INTERVAL_DEFAULT},
            .is_value_valid = is_auto_interval_valid,
            .type = SntpSettingTypeInt,
        },
    [SntpSettingIdxBootInterval] =
        {
            .name = "boot_interval",
            .default_value = &(const int){SNTP_BOOT_INTERVAL_DEFAULT},
            .is_value_valid = is_boot_interval_valid,
            .type = SntpSettingTypeInt,
        },
    [SntpSettingIdxServerName] =
        {
            .name = "server_name",
            .default_value = SNTP_SERVER_NAME_DEFAULT,
            .is_value_valid = is_server_name_valid,
            .type = SntpSettingTypeString,
        },
    [SntpSettingIdxIsEnabled] =
        {
            .name = "is_enabled",
            .default_value = &(const bool){SNTP_IS_ENABLED_DEFAULT},
            .is_value_valid = NULL,
            .type = SntpSettingTypeBool,
        },
};

static_assert(COUNT_OF(settings) == SntpSettingIdxsCount);

static const SntpSettingTypeActions setting_type_actions[] = {
    [SntpSettingTypeInt] =
        {
            .load = sntp_load_int_setting,
            .save = sntp_save_int_setting,
        },
    [SntpSettingTypeString] =
        {
            .load = sntp_load_string_setting,
            .save = sntp_save_string_setting,
        },
    [SntpSettingTypeBool] =
        {
            .load = sntp_load_bool_setting,
            .save = sntp_save_bool_setting,
        },
};

static_assert(COUNT_OF(setting_type_actions) == SntpSettingTypesCount);
