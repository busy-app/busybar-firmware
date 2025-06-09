#include "update_manifest.h"

#include <cjson/cJSON.h>
#include <core/string.h>
#include <furi.h>
#include <path.h>

struct UpdateManifest {
    uint32_t version;
    uint32_t updater_stage_crc32;
    FuriString* updater_stage;
    FuriString* updater_resources;
    FuriString* updater_sil_fw;
    FuriString* updater_sil_radio_fw;
    FuriString* updater_dfu;
    uint8_t target;
};

UpdateManifest* updater_manifest_alloc(void) {
    UpdateManifest* config = malloc(sizeof(UpdateManifest));
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
    config->version = 0;
    return config;
}

void updater_manifest_free(UpdateManifest* config) {
    if(config) {
        furi_string_free(config->updater_stage);
        furi_string_free(config->updater_resources);
        furi_string_free(config->updater_sil_fw);
        furi_string_free(config->updater_sil_radio_fw);
        furi_string_free(config->updater_dfu);
        free(config);
    }
}

void updater_manifest_prefix_paths(UpdateManifest* config, const char* prefix) {
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

bool updater_manifest_init_from_memory(
    UpdateManifest* config,
    const char* json_data,
    size_t json_size) {
    furi_check(config);
    furi_check(json_data && json_size > 0);
    bool result = false;
    cJSON* root = cJSON_ParseWithLength(json_data, json_size);
    do {
        if(!root) break;
        cJSON* item;
        item = cJSON_GetObjectItem(root, "version");
        config->version = cJSON_IsNumber(item) ? (uint32_t)item->valuedouble : 0;
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

bool updater_manifest_init_from_file(UpdateManifest* config, File* file) {
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
    bool success = updater_manifest_init_from_memory(config, buffer, file_size);
    free(buffer);
    return success;
}

// Getter implementations
uint32_t updater_manifest_get_updater_stage_crc32(const UpdateManifest* config) {
    furi_check(config);
    return config->updater_stage_crc32;
}

const FuriString*
    updater_manifest_get_path(const UpdateManifest* config, UpdateManifestPath field) {
    furi_check(config);
    switch(field) {
    case UpdateManifestPathStage:
        return config->updater_stage;
    case UpdateManifestPathResources:
        return config->updater_resources;
    case UpdateManifestPathSilFw:
        return config->updater_sil_fw;
    case UpdateManifestPathSilRadioFw:
        return config->updater_sil_radio_fw;
    case UpdateManifestPathDfu:
        return config->updater_dfu;
    default:
        return NULL;
    }

    return NULL;
}

uint8_t updater_manifest_get_target(const UpdateManifest* config) {
    furi_check(config);
    return config->target;
}

uint32_t updater_manifest_get_version(const UpdateManifest* config) {
    furi_check(config);
    return config->version;
}
