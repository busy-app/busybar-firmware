#include "update_config.h"

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

struct UpdateConfig {
    Storage* storage;
    File* file;
    UpdateManifest* config;
};

UpdateConfig* update_config_alloc(void) {
    UpdateConfig* state = malloc(sizeof(UpdateConfig));
    state->storage = furi_record_open(RECORD_STORAGE);
    state->file = storage_file_alloc(state->storage);
    state->config = updater_manifest_alloc();
    return state;
}

void update_config_free(UpdateConfig* state) {
    furi_check(state);

    if(state) {
        updater_manifest_free(state->config);
        storage_file_free(state->file);
        furi_record_close(RECORD_STORAGE);
        free(state);
    }
}

UpdateConfigValidation
    update_config_load_config(UpdateConfig* state, const char* update_manifest_path) {
    furi_check(state);
    furi_check(update_manifest_path);

    FuriString* update_dir = furi_string_alloc();
    path_extract_dirname(update_manifest_path, update_dir);
    UpdateConfigValidation result = UpdateConfigValidationUnspecifiedError;

    do {
        if(!storage_file_open(state->file, update_manifest_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Failed to open manifest '%s'", update_manifest_path);
            result = UpdateConfigValidationManifestPathInvalid;
            break;
        }
        if(!updater_manifest_init_from_file(state->config, state->file)) {
            FURI_LOG_E(TAG, "Failed to read manifest '%s'", update_manifest_path);
            storage_file_close(state->file);
            result = UpdateConfigValidationManifestInvalid;
            break;
        }
        storage_file_close(state->file);

        updater_manifest_prefix_paths(state->config, furi_string_get_cstr(update_dir));

        FURI_LOG_I(
            TAG,
            "Updater stage from manifest: %s",
            furi_string_get_cstr(
                updater_manifest_get_path(state->config, UpdateManifestPathStage)));

        if(updater_manifest_get_version(state->config) != UPDATE_MANIFEST_VERSION) {
            FURI_LOG_E(
                TAG,
                "Manifest version mismatch: expected %d, got %lu",
                UPDATE_MANIFEST_VERSION,
                updater_manifest_get_version(state->config));
            result = UpdateConfigValidationOutdatedManifestVersion;
            break;
        }

        uint8_t device_target = furi_hal_version_get_hw_target();
        if(updater_manifest_get_target(state->config) != device_target) {
            FURI_LOG_E(
                TAG,
                "Target mismatch: manifest for '%u', device is '%u'",
                updater_manifest_get_target(state->config),
                device_target);
            result = UpdateConfigValidationTargetMismatch;
            break;
        }

        const FuriString* updater_stage_path =
            updater_manifest_get_path(state->config, UpdateManifestPathStage);
        if(furi_string_empty(updater_stage_path)) {
            FURI_LOG_E(TAG, "Stage path is empty in manifest");
            result = UpdateConfigValidationStageMissing;
            break;
        }

        if(!storage_file_open(
               state->file,
               furi_string_get_cstr(updater_stage_path),
               FSAM_READ,
               FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(
                TAG, "Failed to open stage file %s", furi_string_get_cstr(updater_stage_path));
            result = UpdateConfigValidationStageMissing;
            break;
        }

        uint32_t crc_actual = crc32_calc_file(state->file, NULL, NULL);
        storage_file_close(state->file);

        if(crc_actual != updater_manifest_get_updater_stage_crc32(state->config)) {
            FURI_LOG_E(
                TAG,
                "Stage %s CRC mismatch: expected %08lX, got %08lX",
                furi_string_get_cstr(updater_stage_path),
                updater_manifest_get_updater_stage_crc32(state->config),
                crc_actual);
            result = UpdateConfigValidationStageIntegrityError;
            break;
        }

        result = UpdateConfigValidationOK;
    } while(false);

    furi_string_free(update_dir);
    return result;
}

const UpdateManifest* update_config_get_manifest(const UpdateConfig* state) {
    furi_check(state);
    return state->config;
}

static const char* const update_config_validation_error_messages[] = {
    [UpdateConfigValidationOK] = "OK",
    [UpdateConfigValidationManifestPathInvalid] = "Manifest path invalid",
    [UpdateConfigValidationManifestFolderNotFound] = "Manifest folder not found",
    [UpdateConfigValidationManifestInvalid] = "Manifest invalid",
    [UpdateConfigValidationStageMissing] = "Stage missing",
    [UpdateConfigValidationStageIntegrityError] = "Stage integrity error",
    [UpdateConfigValidationManifestPointerCreateError] = "Manifest pointer create error",
    [UpdateConfigValidationManifestPointerCheckError] = "Manifest pointer check error",
    [UpdateConfigValidationTargetMismatch] = "Target mismatch",
    [UpdateConfigValidationOutdatedManifestVersion] = "Outdated manifest version",
    [UpdateConfigValidationUnspecifiedError] = "Unspecified error",
};

const char* update_config_validation_get_error_str(UpdateConfigValidation error_code) {
    if(error_code <= UpdateConfigValidationUnspecifiedError) {
        return update_config_validation_error_messages[error_code];
    }
    return "Unknown error code";
}
