#include "update_tar.h"
#include "session/session_config.h"

#include <furi_hal_nvm.h>
#include <furi_hal_power.h>
#include <toolbox/tar/tar_archive.h>
#include <toolbox/path.h>
#include <toolbox/update_lib/update_config.h>

#define UPDATE_STAGING_ROOT ("/update")

#define TAG "UpdaterTar"

static const char* const updater_tar_install_error_messages[] = {
    [UpdaterTarStatusSuccess] = "Success",
    [UpdaterTarStatusErrorCreateStagingDir] = "Failed to create staging directory",
    [UpdaterTarStatusErrorOpenTar] = "Failed to open TAR file",
    [UpdaterTarStatusErrorUnpackTar] = "Failed to unpack TAR file",
    [UpdaterTarStatusErrorManifestNotFound] = "Manifest not found",
    [UpdaterTarStatusErrorValidateManifest] = "Failed to validate manifest",
    [UpdaterTarStatusErrorWritePointerFile] = "Failed to write pointer file",
    [UpdaterTarStatusErrorUnknown] = "Unknown error",
};

const char* updater_tar_install_get_error_str(UpdaterTarStatus error_code) {
    if(error_code <= UpdaterTarStatusErrorUnknown) {
        return updater_tar_install_error_messages[error_code];
    }
    return "Unknown error code";
}

UpdaterTarStatus updater_tar_install(const char* path, bool auto_reboot) {
    UpdaterTarStatus ret = UpdaterTarStatusErrorUnknown;
    FURI_LOG_D(TAG, "Installing update bundle from: %s", path);
    UpdateConfig* state = update_config_alloc();
    Storage* storage = furi_record_open(RECORD_STORAGE);

    FuriString* file_path = furi_string_alloc();
    path_extract_dirname(path, file_path);

    FuriString* final_staging_path = furi_string_alloc();
    path_concat(furi_string_get_cstr(file_path), UPDATE_STAGING_ROOT, final_staging_path);

    FuriString* manifest_full_path = furi_string_alloc_printf(
        "%s/%s", furi_string_get_cstr(final_staging_path), UPDATE_CONFIG_FILENAME);

    FURI_LOG_D(TAG, "Final staging path: %s", furi_string_get_cstr(final_staging_path));

    if(storage_dir_exists(storage, furi_string_get_cstr(final_staging_path))) {
        FURI_LOG_I(
            TAG,
            "Cleaning up directory recursively: %s",
            furi_string_get_cstr(final_staging_path));
        storage_simply_remove_recursive(storage, furi_string_get_cstr(final_staging_path));
    }

    do {
        // 1. Create staging directory
        FURI_LOG_D(
            TAG, "Creating staging directory: %s", furi_string_get_cstr(final_staging_path));
        if(storage_common_mkdir(storage, furi_string_get_cstr(final_staging_path)) != FSE_OK) {
            FURI_LOG_E(
                TAG,
                "Failed to create package directory: %s",
                furi_string_get_cstr(final_staging_path));
            ret = UpdaterTarStatusErrorCreateStagingDir;
            break;
        }

        // 2. Unpack TAR
        FURI_LOG_D(TAG, "Unpacking TAR contents to: %s", furi_string_get_cstr(final_staging_path));
        TarArchive* tar = tar_archive_alloc(storage);
        bool unpack_success = false;
        if(tar_archive_open(tar, path, TarOpenModeRead)) {
            if(tar_archive_unpack_to(tar, furi_string_get_cstr(final_staging_path), NULL)) {
                unpack_success = true;
            } else {
                FURI_LOG_E(
                    TAG,
                    "Failed to unpack TAR contents to %s",
                    furi_string_get_cstr(final_staging_path));
                ret = UpdaterTarStatusErrorUnpackTar;
            }
        } else {
            FURI_LOG_E(TAG, "Failed to open TAR file %s", path);
            ret = UpdaterTarStatusErrorOpenTar;
        }

        tar_archive_free(tar);

        if(!unpack_success) {
            // Staging dir will be cleaned by on_close as reboot_initiated is false
            FURI_LOG_E(TAG, "Failed to unpack update TAR.");
            break;
        }
        FURI_LOG_D(TAG, "TAR unpacked successfully");

        // 3. Validate: Check for UPDATE_CONFIG_FILENAME
        FURI_LOG_D(TAG, "Checking for manifest: %s", furi_string_get_cstr(manifest_full_path));
        if(!storage_file_exists(storage, furi_string_get_cstr(manifest_full_path))) {
            FURI_LOG_E(
                TAG, "Manifest file not found: %s", furi_string_get_cstr(manifest_full_path));
            ret = UpdaterTarStatusErrorManifestNotFound;
            break;
        }
        FURI_LOG_D(TAG, "Manifest found: %s", furi_string_get_cstr(manifest_full_path));

        // 4. Validate the update package using update_config_load
        UpdateConfigValidation config_state =
            update_config_load(state, furi_string_get_cstr(manifest_full_path));
        if(config_state != UpdateConfigValidationOK) {
            FURI_LOG_E(
                TAG,
                "Failed to load updater configuration: %s",
                update_config_validation_get_error_str(config_state));
            ret = UpdaterTarStatusErrorValidateManifest;
            break;
        }

        FURI_LOG_D(TAG, "Updater configuration valid");

        UpdaterSessionConfig session_config;
        const UpdateManifest* manifest = update_config_get_manifest(state);
        updater_session_config_compose(manifest, &session_config);
        updater_session_config_save(furi_string_get_cstr(final_staging_path), &session_config);

        if(!update_config_write_pointer_file(storage, furi_string_get_cstr(manifest_full_path))) {
            FURI_LOG_E(TAG, "Failed to write manifest path to pointer file.");
            ret = UpdaterTarStatusErrorWritePointerFile;
            break;
        }

        FURI_LOG_D(
            TAG,
            "Manifest path written to %s/%s",
            furi_string_get_cstr(final_staging_path),
            UPDATE_POINTER_FILE_NAME);

        // 5. Set boot mode to Updater and optionally reboot
        furi_hal_nvm_set_boot_mode(FuriHalNvmBootModeUpdate);
        FURI_LOG_D(TAG, "Boot mode set to Update.");

        if(auto_reboot) {
            FURI_LOG_D(TAG, "Rebooting...");
            furi_delay_ms(100);
            furi_hal_power_reset();
        }

        ret = UpdaterTarStatusSuccess;
    } while(false);

    furi_string_free(file_path);
    furi_string_free(final_staging_path);
    furi_string_free(manifest_full_path);
    furi_record_close(RECORD_STORAGE);
    update_config_free(state);

    return ret;
}
