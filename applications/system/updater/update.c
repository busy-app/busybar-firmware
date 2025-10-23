#include "update.h"
#include "session/session_config.h"

#include <power/power_service/power.h>

#include <furi_hal_nvm.h>
#include <furi_hal_power.h>
#include <toolbox/path.h>
#include <toolbox/tar/tar_archive.h>
#include <toolbox/update_lib/update_config.h>
#include <toolbox/update_lib/common_vals.h>

#define UPDATE_MIN_BATTERY_LEVEL 40

#define UPDATE_STAGING_ROOT "update"

#define TAG "Updater"

static const char* const updater_tar_install_error_messages[] = {
    [UpdaterStatusSuccess] = "Success",
    [UpdaterStatusErrorCreateStagingDir] = "Failed to create staging directory",
    [UpdaterStatusErrorOpenTar] = "Failed to open TAR file",
    [UpdaterStatusErrorUnpackTar] = "Failed to unpack TAR file",
    [UpdaterStatusErrorManifestNotFound] = "Manifest not found",
    [UpdaterStatusErrorValidateManifest] = "Failed to validate manifest",
    [UpdaterStatusErrorSaveSessionConfig] = "Failed save session config",
    [UpdaterStatusErrorWritePointerFile] = "Failed to write pointer file",
    [UpdaterStatusErrorLowBatteryLevel] = "Low battery level",
    [UpdaterStatusErrorUnknown] = "Unknown error",
};

static_assert(COUNT_OF(updater_tar_install_error_messages) == UpdaterStatusesCount);

static bool does_battery_state_allow_update(void) {
    Power* power = furi_record_open(RECORD_POWER);

    PowerInfo info;
    power_get_info(power, &info);

    bool allow_update =
        info.charge > UPDATE_MIN_BATTERY_LEVEL ||
        (furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug) && power_is_usb_connected(power));

    furi_record_close(RECORD_POWER);
    return allow_update;
}

const char* updater_get_status_string(UpdaterStatus status) {
    return (status < UpdaterStatusesCount) ? updater_tar_install_error_messages[status] :
                                             "Unknown error code";
}

UpdaterStatus
    updater_unpack_tar(const char* tar_path, const char* staging_path, FuriString* manifest_path) {
    furi_check(tar_path);
    furi_check(manifest_path);

    FURI_LOG_D(TAG, "Unpacking update bundle from: %s", tar_path);

    UpdaterStatus status = UpdaterStatusErrorUnknown;
    Storage* storage = furi_record_open(RECORD_STORAGE);

    FuriString* _staging_path = furi_string_alloc();
    if(staging_path) {
        furi_string_set_str(_staging_path, staging_path);
    } else {
        path_extract_dirname(tar_path, _staging_path);
        path_append(_staging_path, UPDATE_STAGING_ROOT);
    }

    FURI_LOG_D(TAG, "Staging path: %s", furi_string_get_cstr(_staging_path));

    if(storage_dir_exists(storage, furi_string_get_cstr(_staging_path))) {
        FURI_LOG_D(
            TAG, "Cleaning up directory recursively: %s", furi_string_get_cstr(_staging_path));

        storage_simply_remove_recursive(storage, furi_string_get_cstr(_staging_path));
    }

    do {
        FURI_LOG_D(TAG, "Creating staging directory: %s", furi_string_get_cstr(_staging_path));

        if(storage_common_mkdir(storage, furi_string_get_cstr(_staging_path)) != FSE_OK) {
            FURI_LOG_E(
                TAG,
                "Failed to create staging directory: %s",
                furi_string_get_cstr(_staging_path));

            status = UpdaterStatusErrorCreateStagingDir;
            break;
        }

        FURI_LOG_D(TAG, "Unpacking TAR contents to: %s", furi_string_get_cstr(_staging_path));

        bool tar_unpack_success = false;
        TarArchive* tar = tar_archive_alloc(storage);
        if(tar_archive_open(tar, tar_path, TarOpenModeRead)) {
            if(tar_archive_unpack_to(tar, furi_string_get_cstr(_staging_path), NULL)) {
                tar_unpack_success = true;
            } else {
                FURI_LOG_E(
                    TAG,
                    "Failed to unpack TAR contents to %s",
                    furi_string_get_cstr(_staging_path));

                status = UpdaterStatusErrorUnpackTar;
            }
        } else {
            FURI_LOG_E(TAG, "Failed to open TAR file %s", tar_path);
            status = UpdaterStatusErrorOpenTar;
        }

        tar_archive_free(tar);

        if(!tar_unpack_success) {
            FURI_LOG_E(TAG, "Failed to unpack update TAR");
            break;
        }

        FURI_LOG_D(TAG, "TAR unpacked successfully");

        path_concat(furi_string_get_cstr(_staging_path), UPDATE_CONFIG_FILENAME, manifest_path);

        FURI_LOG_D(TAG, "Manifest path: %s", furi_string_get_cstr(manifest_path));

        status = UpdaterStatusSuccess;
    } while(false);

    furi_string_free(_staging_path);
    furi_record_close(RECORD_STORAGE);

    return status;
}

UpdaterStatus updater_prepare_install(const char* manifest_path) {
    furi_check(manifest_path);

    FURI_LOG_D(TAG, "Preparing update for installation");

    UpdaterStatus status = UpdaterStatusErrorUnknown;
    Storage* storage = NULL;
    UpdateConfig* config = NULL;
    FuriString* staging_path = NULL;

    do {
        if(!does_battery_state_allow_update()) {
            FURI_LOG_W(
                TAG, "Battery level too low (has to be at least %d%%)", UPDATE_MIN_BATTERY_LEVEL);

            status = UpdaterStatusErrorLowBatteryLevel;
            break;
        }

        FURI_LOG_D(TAG, "Checking for manifest: %s", manifest_path);

        storage = furi_record_open(RECORD_STORAGE);
        if(!storage_file_exists(storage, manifest_path)) {
            FURI_LOG_E(TAG, "Manifest file not found: %s", manifest_path);

            status = UpdaterStatusErrorManifestNotFound;
            break;
        }

        FURI_LOG_D(TAG, "Manifest file found: %s", manifest_path);

        config = update_config_alloc();
        UpdateConfigValidation config_state = update_config_load(config, manifest_path);
        if(config_state != UpdateConfigValidationOK) {
            FURI_LOG_E(
                TAG,
                "Failed to load updater configuration: %s",
                update_config_validation_get_error_str(config_state));

            status = UpdaterStatusErrorValidateManifest;
            break;
        }

        FURI_LOG_D(TAG, "Updater configuration valid");

        staging_path = furi_string_alloc();
        path_extract_dirname(manifest_path, staging_path);

        UpdaterSessionConfig session_config;
        const UpdateManifest* manifest = update_config_get_manifest(config);
        updater_session_config_compose(manifest, &session_config);
        if(!updater_session_config_save(furi_string_get_cstr(staging_path), &session_config)) {
            FURI_LOG_E(TAG, "Failed to save session config");
            status = UpdaterStatusErrorSaveSessionConfig;
            break;
        }

        if(!update_config_write_pointer_file(storage, manifest_path)) {
            FURI_LOG_E(TAG, "Failed to write manifest path to pointer file");

            updater_session_config_delete(furi_string_get_cstr(staging_path));

            status = UpdaterStatusErrorWritePointerFile;
            break;
        }

        FURI_LOG_D(TAG, "Manifest path written to %s", EXT_PATH(UPDATE_POINTER_FILE_NAME));

        status = UpdaterStatusSuccess;
    } while(false);

    if(staging_path) {
        furi_string_free(staging_path);
    }

    if(config) {
        update_config_free(config);
    }

    if(storage) {
        furi_record_close(RECORD_STORAGE);
    }

    return status;
}

bool updater_is_install_prepared(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool is_install_prepared = storage_file_exists(storage, EXT_PATH(UPDATE_POINTER_FILE_NAME));
    furi_record_close(RECORD_STORAGE);

    return is_install_prepared;
}

void updater_cancel_prepared_install(void) {
    FURI_LOG_D(TAG, "Cancelling prepared install");

    Storage* storage = furi_record_open(RECORD_STORAGE);

    FuriString* manifest_path = furi_string_alloc();
    if(update_config_read_pointer_file(storage, manifest_path)) {
        FuriString* staging_path = furi_string_alloc();
        path_extract_dirname(furi_string_get_cstr(manifest_path), staging_path);
        updater_session_config_delete(furi_string_get_cstr(staging_path));
        furi_string_free(staging_path);
    }

    furi_string_free(manifest_path);

    storage_common_remove(storage, EXT_PATH(UPDATE_POINTER_FILE_NAME));

    furi_record_close(RECORD_STORAGE);
}

UpdaterStatus updater_reboot_install(void) {
    UpdaterStatus status;

    if(!does_battery_state_allow_update()) {
        FURI_LOG_W(
            TAG, "Battery level too low (has to be at least %d%%)", UPDATE_MIN_BATTERY_LEVEL);

        status = UpdaterStatusErrorLowBatteryLevel;
    } else {
        furi_hal_nvm_set_boot_mode(FuriHalNvmBootModeUpdate);

        FURI_LOG_D(TAG, "Boot mode set to \"update\"\r\nRebooting...");

        furi_delay_ms(100);
        furi_hal_power_reset();

        status = UpdaterStatusSuccess;
    }

    return status;
}
