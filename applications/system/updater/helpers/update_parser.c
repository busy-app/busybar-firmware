#include "update_parser.h"

#include <storage/storage.h>

#include <furi_hal_version.h>
#include <cjson/cJSON.h>

#define TAG "UpdateParser"

#define UPDATE_PARSER_BRANCH "channels"

#define UPDATE_PARSER_BRANCH_ID          "id"
#define UPDATE_PARSER_BRANCH_TITLE       "title"
#define UPDATE_PARSER_BRANCH_DESCRIPTION "description"
#define UPDATE_PARSER_BRANCH_VERSIONS    "versions"

#define UPDATE_PARSER_BRANCH_VERSIONS_VERSION   "version"
#define UPDATE_PARSER_BRANCH_VERSIONS_CHANGELOG "changelog"
#define UPDATE_PARSER_BRANCH_VERSIONS_FILES     "files"

#define UPDATE_PARSER_BRANCH_VERSIONS_FILES_URL    "url"
#define UPDATE_PARSER_BRANCH_VERSIONS_FILES_TARGET "target"
#define UPDATE_PARSER_BRANCH_VERSIONS_FILES_TYPE   "type"
#define UPDATE_PARSER_BRANCH_VERSIONS_FILES_SHA256 "sha256"

#define UPDATE_PARSER_BRANCH_TARGET_FOUND                                          \
    ({                                                                             \
        static char target[8];                                                     \
        snprintf(target, sizeof(target), "f%u", furi_hal_version_get_hw_target()); \
        target;                                                                    \
    })
#define UPDATE_PARSER_BRANCH_FILE_NAME_FW_FOUND "update_tar"

static bool parse_update_json(cJSON* json_root, const char* branch_id, UpdateMetadata* metadata) {
    bool is_success = false;

    do {
        if(!cJSON_IsObject(json_root)) {
            break;
        }

        cJSON* item;
        item = cJSON_GetObjectItem(json_root, UPDATE_PARSER_BRANCH);

        if(!cJSON_IsArray(item)) {
            break;
        }

        /* search for channel */
        cJSON* id = NULL;
        cJSON_ArrayForEach(item, item) {
            if(!cJSON_IsObject(item)) {
                continue;
            }

            id = cJSON_GetObjectItem(item, UPDATE_PARSER_BRANCH_ID);

            if(!cJSON_IsString(id)) {
                continue;
            }

            if(strcmp(id->valuestring, branch_id) != 0) {
                continue;
            }

            FURI_LOG_D(TAG, "Found update channel: %s", id->valuestring);

            /* channel found, search for version */
            item = cJSON_GetObjectItem(item, UPDATE_PARSER_BRANCH_VERSIONS);
            if(!cJSON_IsArray(item)) {
                break;
            }

            cJSON* version = NULL;
            cJSON_ArrayForEach(version, item) {
                if(!cJSON_IsObject(version)) {
                    continue;
                }

                cJSON* id_version =
                    cJSON_GetObjectItem(version, UPDATE_PARSER_BRANCH_VERSIONS_VERSION);
                if(!cJSON_IsString(id_version)) {
                    continue;
                }

                cJSON* id_changelog =
                    cJSON_GetObjectItem(version, UPDATE_PARSER_BRANCH_VERSIONS_CHANGELOG);
                if(!cJSON_IsString(id_changelog)) {
                    continue;
                }

                cJSON* files = cJSON_GetObjectItem(version, UPDATE_PARSER_BRANCH_VERSIONS_FILES);
                if(!cJSON_IsArray(files)) {
                    continue;
                }

                cJSON* file = NULL;
                cJSON_ArrayForEach(file, files) {
                    if(!cJSON_IsObject(file)) {
                        continue;
                    }

                    cJSON* target =
                        cJSON_GetObjectItem(file, UPDATE_PARSER_BRANCH_VERSIONS_FILES_TARGET);
                    if(!cJSON_IsString(target)) {
                        continue;
                    }
                    if(strcmp(target->valuestring, UPDATE_PARSER_BRANCH_TARGET_FOUND) != 0) {
                        continue;
                    }

                    cJSON* url =
                        cJSON_GetObjectItem(file, UPDATE_PARSER_BRANCH_VERSIONS_FILES_URL);
                    if(!cJSON_IsString(url)) {
                        continue;
                    }

                    cJSON* type =
                        cJSON_GetObjectItem(file, UPDATE_PARSER_BRANCH_VERSIONS_FILES_TYPE);
                    if(!cJSON_IsString(type)) {
                        continue;
                    }
                    if(strcmp(type->valuestring, UPDATE_PARSER_BRANCH_FILE_NAME_FW_FOUND) != 0) {
                        continue;
                    }

                    cJSON* sha256 =
                        cJSON_GetObjectItem(file, UPDATE_PARSER_BRANCH_VERSIONS_FILES_SHA256);
                    if(!cJSON_IsString(sha256)) {
                        continue;
                    }

                    if(metadata->id) {
                        furi_string_set(metadata->id, id->valuestring);
                    }

                    if(metadata->version) {
                        furi_string_set(metadata->version, id_version->valuestring);
                    }

                    if(metadata->url) {
                        furi_string_set(metadata->url, url->valuestring);
                    }

                    if(metadata->sha256) {
                        furi_string_set(metadata->sha256, sha256->valuestring);
                    }

                    if(metadata->changelog) {
                        furi_string_set(metadata->changelog, id_changelog->valuestring);
                    }

                    FURI_LOG_D(TAG, "Found update ID: %s", id->valuestring);
                    FURI_LOG_D(TAG, "Found update version: %s", id_version->valuestring);
                    FURI_LOG_D(TAG, "Found update URL: %s", url->valuestring);
                    FURI_LOG_D(TAG, "Found update SHA256: %s", sha256->valuestring);

                    is_success = true;
                    break;
                }
            }
        }
    } while(false);

    return is_success;
}

bool update_parser_metadata_parse(
    const char* file_path,
    const char* branch_id,
    UpdateMetadata* metadata) {
    furi_assert(file_path);
    furi_assert(branch_id);
    furi_assert(metadata);

    bool is_success = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        if(!storage_file_open(file, file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Failed to open file: %s", file_path);
            break;
        }

        const size_t file_size = storage_file_size(file);
        if(file_size <= 0) {
            break;
        }

        char* file_buffer = malloc(file_size + 1);
        if(storage_file_read(file, file_buffer, file_size) != file_size) {
            break;
        }

        cJSON* root = cJSON_Parse(file_buffer);
        is_success = parse_update_json(root, branch_id, metadata);

        cJSON_Delete(root);
        free(file_buffer);
    } while(false);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return is_success;
}
