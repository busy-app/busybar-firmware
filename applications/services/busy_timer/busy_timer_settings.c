#include "busy_timer_settings.h"

#include <storage/storage.h>

#include <cjson/cJSON.h>

#define TAG "BusyTimerSettings"

#define BUSY_TIMER_SETTINGS_FILE APP_DATA_PATH("settings.json")

#define BUSY_TIMER_SETTINGS_CURRENT_VERSION (0)

#define VERSION_KEY "version"

#define BUSY_TIMER_KEY "timer"

#define BUSY_TIMER_MODE_KEY             "mode"
#define BUSY_TIMER_TIME_KEY             "time"
#define BUSY_TIMER_WORK_TIME_KEY        "work_time"
#define BUSY_TIMER_REST_TIME_KEY        "rest_time"
#define BUSY_TIMER_CYCLE_COUNT_KEY      "cycle_count"
#define BUSY_TIMER_ENABLE_AUTOSTART_KEY "enable_autostart"
#define BUSY_TIMER_ENABLE_DEMO_MODE_KEY "enable_demo_mode"

#define BUSY_TIMER_INFINITE_MODE_KEY "infinite"
#define BUSY_TIMER_SIMPLE_MODE_KEY   "simple"
#define BUSY_TIMER_INTERVAL_MODE_KEY "interval"

static bool busy_timer_settings_parse_timer_config(const cJSON* json, BusyTimerConfig* config) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        cJSON* item;

        item = cJSON_GetObjectItem(json, BUSY_TIMER_MODE_KEY);

        if(!cJSON_IsString(item)) {
            break;
        }

        const char* str = item->valuestring;

        if(strcasecmp(str, BUSY_TIMER_INFINITE_MODE_KEY) == 0) {
            config->mode = BusyTimerModeInfinite;
        } else if(strcasecmp(str, BUSY_TIMER_SIMPLE_MODE_KEY) == 0) {
            config->mode = BusyTimerModeSimple;
        } else if(strcasecmp(str, BUSY_TIMER_INTERVAL_MODE_KEY) == 0) {
            config->mode = BusyTimerModeInterval;
        } else {
            break;
        }

        item = cJSON_GetObjectItem(json, BUSY_TIMER_TIME_KEY);

        if(!cJSON_IsNumber(item)) {
            break;
        }

        config->time_mn = item->valueint;

        item = cJSON_GetObjectItem(json, BUSY_TIMER_WORK_TIME_KEY);

        if(!cJSON_IsNumber(item)) {
            break;
        }

        config->work_time_mn = item->valueint;

        item = cJSON_GetObjectItem(json, BUSY_TIMER_REST_TIME_KEY);

        if(!cJSON_IsNumber(item)) {
            break;
        }

        config->rest_time_mn = item->valueint;

        item = cJSON_GetObjectItem(json, BUSY_TIMER_CYCLE_COUNT_KEY);

        if(!cJSON_IsNumber(item)) {
            break;
        }

        config->cycle_count = item->valueint;

        item = cJSON_GetObjectItem(json, BUSY_TIMER_ENABLE_AUTOSTART_KEY);

        if(!cJSON_IsBool(item)) {
            break;
        }

        config->enable_autostart = cJSON_IsTrue(item);

        item = cJSON_GetObjectItem(json, BUSY_TIMER_ENABLE_DEMO_MODE_KEY);

        if(!cJSON_IsBool(item)) {
            break;
        }

        config->enable_demo_mode = cJSON_IsTrue(item);

        success = true;

    } while(false);

    return success;
}

static bool busy_timer_settings_parse(const cJSON* json, BusyTimerSettings* settings) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        cJSON* item;

        item = cJSON_GetObjectItem(json, VERSION_KEY);

        if(!cJSON_IsNumber(item)) {
            break;
        }

        if(item->valueint != BUSY_TIMER_SETTINGS_CURRENT_VERSION) {
            break;
        }

        item = cJSON_GetObjectItem(json, BUSY_TIMER_KEY);

        if(!busy_timer_settings_parse_timer_config(item, &settings->timer_config)) {
            break;
        }

        success = true;

    } while(false);

    return success;
}

bool busy_timer_settings_load(BusyTimerSettings* settings) {
    furi_check(settings);

    bool success = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        if(!storage_file_open(file, BUSY_TIMER_SETTINGS_FILE, FSAM_READ, FSOM_OPEN_EXISTING)) {
            break;
        }

        const size_t file_size = storage_file_size(file);

        if(file_size == 0) {
            break;
        }

        char* buffer = malloc(file_size + 1);

        if(storage_file_read(file, buffer, file_size) != file_size) {
            break;
        }

        cJSON* root = cJSON_Parse(buffer);

        success = busy_timer_settings_parse(root, settings);

        cJSON_Delete(root);
        free(buffer);

    } while(false);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return success;
}

static void busy_timer_settings_serialize_timer_config(cJSON* json, const BusyTimerConfig* config) {
    cJSON* timer_json = cJSON_AddObjectToObject(json, BUSY_TIMER_KEY);

    const char* mode_str;

    if(config->mode == BusyTimerModeInfinite) {
        mode_str = BUSY_TIMER_INFINITE_MODE_KEY;
    } else if(config->mode == BusyTimerModeSimple) {
        mode_str = BUSY_TIMER_SIMPLE_MODE_KEY;
    } else if(config->mode == BusyTimerModeInterval) {
        mode_str = BUSY_TIMER_INTERVAL_MODE_KEY;
    } else {
        furi_crash();
    }

    cJSON_AddStringToObject(timer_json, BUSY_TIMER_MODE_KEY, mode_str);
    cJSON_AddNumberToObject(timer_json, BUSY_TIMER_TIME_KEY, config->time_mn);
    cJSON_AddNumberToObject(timer_json, BUSY_TIMER_WORK_TIME_KEY, config->work_time_mn);
    cJSON_AddNumberToObject(timer_json, BUSY_TIMER_REST_TIME_KEY, config->rest_time_mn);
    cJSON_AddNumberToObject(timer_json, BUSY_TIMER_CYCLE_COUNT_KEY, config->cycle_count);
    cJSON_AddBoolToObject(timer_json, BUSY_TIMER_ENABLE_AUTOSTART_KEY, config->enable_autostart);
    cJSON_AddBoolToObject(timer_json, BUSY_TIMER_ENABLE_DEMO_MODE_KEY, config->enable_demo_mode);
}

bool busy_timer_settings_save(const BusyTimerSettings* settings) {
    furi_check(settings);

    bool success = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        if(!storage_file_open(file, BUSY_TIMER_SETTINGS_FILE, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            break;
        }

        cJSON* root = cJSON_CreateObject();

        cJSON_AddNumberToObject(root, VERSION_KEY, BUSY_TIMER_SETTINGS_CURRENT_VERSION);

        busy_timer_settings_serialize_timer_config(root, &settings->timer_config);

        char* buffer = cJSON_Print(root);

        const size_t buffer_len = strlen(buffer);
        const size_t bytes_written = storage_file_write(file, buffer, buffer_len);

        success = buffer_len == bytes_written;

        cJSON_Delete(root);
        free(buffer);

    } while(false);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return success;
}
