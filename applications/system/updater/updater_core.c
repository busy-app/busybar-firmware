#include "updater_core.h"

#include <furi.h>

#include <furi_hal_version.h>
#include <furi_hal_nvm.h>

#include <core/string.h>

#include <cjson/cJSON.h>
#include <storage/storage.h>

#include <version/version.h>
#include <toolbox/crc32_calc.h>
#include <toolbox/path.h>
#include <toolbox/update_lib/update_util.h>

#define TAG "Updater"

struct UpdaterState {
    Storage* storage;
    File* file;
    FuriString* update_folder;
    UpdaterConfig* config;
};

UpdaterState* updater_state_alloc(const char* update_path) {
    UpdaterState* state = malloc(sizeof(UpdaterState));
    state->storage = furi_record_open(RECORD_STORAGE);
    state->update_folder = furi_string_alloc_set(update_path);
    state->file = storage_file_alloc(state->storage);
    state->config = updater_config_alloc();
    return state;
}

void updater_state_free(UpdaterState* state) {
    if(state) {
        updater_config_free(state->config);
        storage_file_free(state->file);
        furi_string_free(state->update_folder);
        furi_record_close(RECORD_STORAGE);
        free(state);
    }
}

bool updater_validate_config(const UpdaterState* state) {
    uint8_t device_target = furi_hal_version_get_hw_target();
    if(state->config->target != device_target) {
        FURI_LOG_E(
            TAG,
            "Target mismatch: update for '%u', device is '%u'",
            state->config->target,
            device_target);
        return false;
    }

    if(!storage_file_open(
           state->file,
           furi_string_get_cstr(state->config->updater_stage),
           FSAM_READ,
           FSOM_OPEN_EXISTING)) {
        FURI_LOG_E(
            TAG,
            "Failed to open updater stage file %s",
            furi_string_get_cstr(state->config->updater_stage));
        return false;
    }
    uint32_t crc_actual = crc32_calc_file(state->file, NULL, NULL);
    if(crc_actual != state->config->updater_stage_crc32) {
        FURI_LOG_E(
            TAG,
            "Updater stage %s CRC mismatch: expected %lx, got %lx",
            furi_string_get_cstr(state->config->updater_stage),
            state->config->updater_stage_crc32,
            crc_actual);
        return false;
    }
    return true;
}

bool updater_load_configuration(UpdaterState* state) {
    bool success = false;
    FuriString* update_path = furi_string_alloc_set(furi_string_get_cstr(state->update_folder));
    furi_string_cat_str(update_path, "/update.json");
    do {
        if(!storage_file_open(
               state->file, furi_string_get_cstr(update_path), FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(
                TAG, "Failed to open updater config file %s", furi_string_get_cstr(update_path));
            break;
        }
        if(!updater_config_from_file(state->config, state->file)) {
            FURI_LOG_E(TAG, "Failed to read updater config %s", furi_string_get_cstr(update_path));
            break;
        }
        storage_file_close(state->file);

        updater_config_prefix_paths(state->config, furi_string_get_cstr(state->update_folder));

        FURI_LOG_I(TAG, "Updater stage: %s", furi_string_get_cstr(state->config->updater_stage));

        success = true;
    } while(false);

    furi_string_free(update_path);

    return success;
}
