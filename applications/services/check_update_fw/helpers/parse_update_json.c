#include "parse_update_json.h"
#include <furi.h>
#include <cjson/cJSON.h>
#include <storage/storage.h>

#define TAG "ParseUpdateJson"

#define PARSE_UPDATE_BRANCH "channels"

#define PARSE_UPDATE_BRANCH_ID          "id"
#define PARSE_UPDATE_BRANCH_TITLE       "title"
#define PARSE_UPDATE_BRANCH_DESCRIPTION "description"
#define PARSE_UPDATE_BRANCH_VERSIONS    "versions"

#define PARSE_UPDATE_BRANCH_VERSIONS_VERSION "version"
#define PARSE_UPDATE_BRANCH_VERSIONS_FILES   "files"

#define PARSE_UPDATE_BRANCH_VERSIONS_FILES_URL    "url"
#define PARSE_UPDATE_BRANCH_VERSIONS_FILES_TARGET "target"
#define PARSE_UPDATE_BRANCH_VERSIONS_FILES_TYPE   "type"

//ToDo add define target
#define PARSE_UPDATE_BRANCH_TARGET_FOUND       "f21"
#define PARSE_UPDATE_BRANCH_FILE_NAME_FW_FOUND "update_tar"

struct ParseUpdateJson {
    FuriString* url;
    FuriString* id;
    FuriString* version;
    const char* branch_id;
    const char* file_path;
};

static bool parse_update_parse_json(cJSON* json, ParseUpdateJson* instance) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        cJSON* item;

        item = cJSON_GetObjectItem(json, PARSE_UPDATE_BRANCH);
        if(!cJSON_IsArray(item)) {
            break;
        }
        // We are looking for the right channel
        cJSON* id = NULL;
        cJSON_ArrayForEach(item, item) {
            if(!cJSON_IsObject(item)) {
                continue;
            }
            id = cJSON_GetObjectItem(item, PARSE_UPDATE_BRANCH_ID);
            if(!cJSON_IsString(id)) {
                continue;
            }
            if(strcmp(id->valuestring, instance->branch_id) != 0) {
                continue;
            }
            FURI_LOG_D(TAG, "Found update channel: %s", id->valuestring);

            // We found the right channel, let's watch the version

            item = cJSON_GetObjectItem(item, PARSE_UPDATE_BRANCH_VERSIONS);
            if(!cJSON_IsArray(item)) {
                break;
            }
            cJSON* version = NULL;
            cJSON_ArrayForEach(version, item) {
                if(!cJSON_IsObject(version)) {
                    continue;
                }
                cJSON* id_version =
                    cJSON_GetObjectItem(version, PARSE_UPDATE_BRANCH_VERSIONS_VERSION);
                if(!cJSON_IsString(id_version)) {
                    continue;
                }

                // We get the required URL to the file
                cJSON* files = cJSON_GetObjectItem(version, PARSE_UPDATE_BRANCH_VERSIONS_FILES);
                if(!cJSON_IsArray(files)) {
                    continue;
                }
                cJSON* file = NULL;
                cJSON_ArrayForEach(file, files) {
                    if(!cJSON_IsObject(file)) {
                        continue;
                    }

                    cJSON* target =
                        cJSON_GetObjectItem(file, PARSE_UPDATE_BRANCH_VERSIONS_FILES_TARGET);
                    if(!cJSON_IsString(target)) {
                        continue;
                    }
                    if(strcmp(target->valuestring, PARSE_UPDATE_BRANCH_TARGET_FOUND) != 0) {
                        continue;
                    }

                    cJSON* url = cJSON_GetObjectItem(file, PARSE_UPDATE_BRANCH_VERSIONS_FILES_URL);
                    if(!cJSON_IsString(url)) {
                        continue;
                    }

                    cJSON* type =
                        cJSON_GetObjectItem(file, PARSE_UPDATE_BRANCH_VERSIONS_FILES_TYPE);
                    if(!cJSON_IsString(type)) {
                        continue;
                    }
                    if(strcmp(type->valuestring, PARSE_UPDATE_BRANCH_FILE_NAME_FW_FOUND) != 0) {
                        continue;
                    }

                    furi_string_set(instance->id, id->valuestring);
                    furi_string_set(instance->version, id_version->valuestring);
                    furi_string_set(instance->url, url->valuestring);

                    FURI_LOG_D(TAG, "Found update ID: %s", id->valuestring);
                    FURI_LOG_D(TAG, "Found update version: %s", id_version->valuestring);
                    FURI_LOG_D(TAG, "Found update URL: %s", url->valuestring);

                    success = true;
                    break;
                }
            }
        }

        success = true;

    } while(false);

    return success;
}

static bool parse_upload_json_load(ParseUpdateJson* instance) {
    bool success = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        if(!storage_file_open(file, instance->file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Failed to open file: %s", instance->file_path);
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

        success = parse_update_parse_json(root, instance);

        cJSON_Delete(root);
        free(buffer);

    } while(false);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return success;
}

ParseUpdateJson* parse_update_init() {
    ParseUpdateJson* instance = malloc(sizeof(ParseUpdateJson));
    instance->url = furi_string_alloc();
    instance->id = furi_string_alloc();
    instance->version = furi_string_alloc();
    return instance;
}

void parse_update_free(ParseUpdateJson* instance) {
    furi_string_free(instance->url);
    furi_string_free(instance->id);
    furi_string_free(instance->version);
    free(instance);
}

bool parse_update_json(ParseUpdateJson* instance, const char* file_path, const char* branch_id) {
    instance->file_path = file_path;
    instance->branch_id = branch_id;

    return parse_upload_json_load(instance);
}

const char* parse_update_get_url(ParseUpdateJson* instance) {
    return furi_string_get_cstr(instance->url);
}

const char* parse_update_get_id(ParseUpdateJson* instance) {
    return furi_string_get_cstr(instance->id);
}

const char* parse_update_get_version(ParseUpdateJson* instance) {
    return furi_string_get_cstr(instance->version);
}
