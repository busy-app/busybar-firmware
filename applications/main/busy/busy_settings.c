#include "busy_settings.h"

#include <storage/storage.h>

#include <cjson/cJSON.h>

#define TAG "BusySettings"

#define BUSY_SETTINGS_CURRENT_VERSION (0)

#define VERSION_KEY "version"
#define THEME_KEY   "theme"

static const char* busy_settings_file_paths[BusySettingsProfileIdMax] = {
    [BusySettingsProfileIdBusy] = APP_DATA_PATH("settings_busy.json"),
    [BusySettingsProfileIdCustom] = APP_DATA_PATH("settings_custom.json"),
};

static const char* busy_settings_default_theme_names[BusySettingsProfileIdMax] = {
    [BusySettingsProfileIdBusy] = "default",
    [BusySettingsProfileIdCustom] = "keep_out",
};

static bool busy_settings_parse(const cJSON* json, BusySettings* settings) {
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

        if(item->valueint != BUSY_SETTINGS_CURRENT_VERSION) {
            break;
        }

        item = cJSON_GetObjectItem(json, THEME_KEY);

        if(!cJSON_IsString(item)) {
            break;
        }

        const char* str = item->valuestring;

        if(strlen(str) >= BUSY_SETTINGS_THEME_NAME_LEN) {
            break;
        }

        strcpy(settings->theme_name, item->valuestring);

        success = true;

    } while(false);

    return success;
}

bool busy_settings_load(BusySettings* settings, BusySettingsProfileId profile_id) {
    furi_check(settings);
    furi_check(profile_id < BusySettingsProfileIdMax);

    bool success = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        const char* file_path = busy_settings_file_paths[profile_id];

        if(!storage_file_open(file, file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
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

        success = busy_settings_parse(root, settings);

        cJSON_Delete(root);
        free(buffer);

    } while(false);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return success;
}

bool busy_settings_save(const BusySettings* settings, BusySettingsProfileId profile_id) {
    furi_check(settings);
    furi_check(profile_id < BusySettingsProfileIdMax);

    bool success = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        const char* file_path = busy_settings_file_paths[profile_id];

        if(!storage_file_open(file, file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            break;
        }

        cJSON* root = cJSON_CreateObject();

        cJSON_AddNumberToObject(root, VERSION_KEY, BUSY_SETTINGS_CURRENT_VERSION);
        cJSON_AddStringToObject(root, THEME_KEY, settings->theme_name);

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

void busy_settings_set_default(BusySettings* settings, BusySettingsProfileId profile_id) {
    furi_check(settings);
    furi_check(profile_id < BusySettingsProfileIdMax);

    strcpy(settings->theme_name, busy_settings_default_theme_names[profile_id]);
}
