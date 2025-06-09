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
#include <toolbox/update_lib/common_vals.h>

#define TAG "Updater"

struct UpdaterState {
    Storage* storage;
    File* file;
    UpdateManifest* config;
};

UpdaterState* updater_state_alloc(void) {
    UpdaterState* state = malloc(sizeof(UpdaterState));
    state->storage = furi_record_open(RECORD_STORAGE);
    state->file = storage_file_alloc(state->storage);
    state->config = updater_manifest_alloc();
    return state;
}

void updater_state_free(UpdaterState* state) {
    if(state) {
        updater_manifest_free(state->config);
        storage_file_free(state->file);
        furi_record_close(RECORD_STORAGE);
        free(state);
    }
}

bool updater_state_validate_config(const UpdaterState* state) {
    uint8_t device_target = furi_hal_version_get_hw_target();
    const UpdateManifest* config = updater_state_get_config(state);
    if(updater_manifest_get_target(config) != device_target) {
        FURI_LOG_E(
            TAG,
            "Target mismatch: update for '%u', device is '%u'",
            updater_manifest_get_target(config),
            device_target);
        return false;
    }

    const FuriString* updater_stage_path =
        updater_manifest_get_path(config, UpdateManifestPathStage);
    if(!storage_file_open(
           state->file, furi_string_get_cstr(updater_stage_path), FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E(
            TAG, "Failed to open updater stage file %s", furi_string_get_cstr(updater_stage_path));
        return false;
    }
    uint32_t crc_actual = crc32_calc_file(state->file, NULL, NULL);
    if(crc_actual != updater_manifest_get_updater_stage_crc32(config)) {
        FURI_LOG_E(
            TAG,
            "Updater stage %s CRC mismatch: expected %lx, got %lx",
            furi_string_get_cstr(updater_stage_path),
            updater_manifest_get_updater_stage_crc32(config),
            crc_actual);
        return false;
    }
    return true;
}

bool updater_state_init_config(UpdaterState* state, const char* update_manifest_path) {
    bool success = false;
    FuriString* update_dir = furi_string_alloc();
    path_extract_dirname(update_manifest_path, update_dir);

    do {
        if(!storage_file_open(state->file, update_manifest_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(
                TAG, "Failed to open updater config file '%s'", furi_string_get_cstr(update_dir));
            break;
        }
        if(!updater_manifest_init_from_file(state->config, state->file)) {
            FURI_LOG_E(
                TAG, "Failed to read updater config '%s'", furi_string_get_cstr(update_dir));
            break;
        }
        storage_file_close(state->file);

        updater_manifest_prefix_paths(state->config, furi_string_get_cstr(update_dir));

        FURI_LOG_I(
            TAG,
            "Updater stage: %s",
            furi_string_get_cstr(
                updater_manifest_get_path(state->config, UpdateManifestPathStage)));

        success = true;
    } while(false);

    furi_string_free(update_dir);

    return success;
}

const UpdateManifest* updater_state_get_config(const UpdaterState* state) {
    furi_check(state);
    return state->config;
}
