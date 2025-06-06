#include "update_util.h"

#include <toolbox/update_util.h>
#include <cjson/cJSON.h>
#include <core/string.h>
#include <furi.h>
#include <path.h>

UpdaterConfig* updater_config_alloc(void) {
    UpdaterConfig* config = malloc(sizeof(UpdaterConfig));
    if(!config) {
        return NULL;
    }
    config->updater_stage_crc32 = 0;
    config->updater_stage = furi_string_alloc();
    config->updater_resources = furi_string_alloc();
    config->updater_sil_fw = furi_string_alloc();
    config->updater_sil_radio_fw = furi_string_alloc();
    config->updater_dfu = furi_string_alloc();
    config->target = 0;
    return config;
}

void updater_config_free(UpdaterConfig* config) {
    if(config) {
        furi_string_free(config->updater_stage);
        furi_string_free(config->updater_resources);
        furi_string_free(config->updater_sil_fw);
        furi_string_free(config->updater_sil_radio_fw);
        furi_string_free(config->updater_dfu);
        free(config);
    }
}

void updater_config_prefix_paths(UpdaterConfig* config, const char* prefix) {
    furi_check(config);
    furi_check(prefix);
    FuriString* temp_path = furi_string_alloc();
    FuriString* paths[] = {
        config->updater_stage,
        config->updater_resources,
        config->updater_sil_fw,
        config->updater_sil_radio_fw,
        config->updater_dfu};
    for(size_t i = 0; i < COUNT_OF(paths); ++i) {
        if(!furi_string_empty(paths[i])) {
            furi_string_set(temp_path, prefix);
            path_append(temp_path, furi_string_get_cstr(paths[i]));
            furi_string_set(paths[i], furi_string_get_cstr(temp_path));
        }
    }
    furi_string_free(temp_path);
}

bool updater_config_from_memory(UpdaterConfig* config, const char* json_data, size_t json_size) {
    furi_check(config);
    furi_check(json_data && json_size > 0);
    bool result = false;
    cJSON* root = cJSON_ParseWithLength(json_data, json_size);
    do {
        if(!root) break;
        cJSON* item;
        item = cJSON_GetObjectItem(root, "target");
        config->target = cJSON_IsNumber(item) ? (uint8_t)item->valueint : 0;
        item = cJSON_GetObjectItem(root, "updater_stage_crc32");
        config->updater_stage_crc32 = cJSON_IsNumber(item) ? (uint32_t)item->valuedouble : 0;
        item = cJSON_GetObjectItem(root, "updater_stage");
        furi_string_set(
            config->updater_stage,
            (cJSON_IsString(item) && item->valuestring) ? item->valuestring : "");
        item = cJSON_GetObjectItem(root, "updater_resources");
        furi_string_set(
            config->updater_resources,
            (cJSON_IsString(item) && item->valuestring) ? item->valuestring : "");
        item = cJSON_GetObjectItem(root, "updater_sil_fw");
        furi_string_set(
            config->updater_sil_fw,
            (cJSON_IsString(item) && item->valuestring) ? item->valuestring : "");
        item = cJSON_GetObjectItem(root, "updater_sil_radio_fw");
        furi_string_set(
            config->updater_sil_radio_fw,
            (cJSON_IsString(item) && item->valuestring) ? item->valuestring : "");
        item = cJSON_GetObjectItem(root, "updater_dfu");
        furi_string_set(
            config->updater_dfu,
            (cJSON_IsString(item) && item->valuestring) ? item->valuestring : "");
        result = true;
    } while(false);
    if(root) cJSON_Delete(root);
    return result;
}

bool updater_config_from_file(UpdaterConfig* config, File* file) {
    furi_check(config);
    furi_check(file);
    size_t file_size = storage_file_size(file);
    if(file_size == 0) {
        return false;
    }
    char* buffer = malloc(file_size + 1);
    if(!buffer) {
        return false;
    }
    if(storage_file_read(file, buffer, file_size) != file_size) {
        free(buffer);
        return false;
    }
    buffer[file_size] = '\0';
    bool success = updater_config_from_memory(config, buffer, file_size);
    free(buffer);
    return success;
}
