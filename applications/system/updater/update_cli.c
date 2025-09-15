#include "sl_updater.h"
#include "sl_update_params.h"

#include <furi.h>
#include <furi_hal_nvm.h>
#include <furi_hal_power.h>

#include <storage/storage.h>
#include <cli/cli_command.h>
#include <cli/args.h>

#include <toolbox/update_lib/update_config.h>
#include <toolbox/update_lib/common_vals.h>
#include <toolbox/tar/tar_archive.h>
#include <toolbox/path.h>
#include <cli/cli_ansi.h>
#include <applications/system/fetch/fetch.h>

#define TAG                 "UpdaterCli"
#define UPDATE_STAGING_ROOT ("/update")
#define UPDATE_TAR_TMP      ("/upload.tar")

// Helper to clean up a directory recursively (remains the same)
static void cleanup_directory_recursive(Storage* storage, const char* path) {
    if(storage_dir_exists(storage, path)) {
        FURI_LOG_I(TAG, "Cleaning up directory recursively: %s", path);
        storage_simply_remove_recursive(storage, path);
    }
}

static void
    updater_cli_progress_callback(SlUpdaterProgressPhase phase, uint8_t percentage, void* context) {
    UNUSED(context);
    switch(phase) {
    case SL_UPDATER_PROGRESS_PHASE_UPLOADING:
        printf("Uploading: %d%%\r", percentage);
        break;
    case SL_UPDATER_PROGRESS_PHASE_AWAITING_INSTALL:
        printf("\nUpload complete. Awaiting installation...\r\n");
        break;
    default:
        break;
    }
}

static void updater_cli_command_print_usage(void) {
    bool is_debug = furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug);
    printf("Usage:\r\n");
    printf(
        "update <917|917_ta%s|install|install_tar|install_web> path\r\n",
        is_debug ? "|917_probe" : "");
}

static bool
    updater_cli_execute(const char* path, bool is_stack_image, uint8_t baud_throttle_ratio) {
    SlUpdater* instance = sl_updater_alloc();
    sl_updater_set_progress_callback(instance, updater_cli_progress_callback, NULL);
    bool success = sl_updater_run(
        instance,
        path,
        is_stack_image,
        is_stack_image ? SL_UPDATE_NWP_COMM_TIMEOUT_S : SL_UPDATE_M4_COMM_TIMEOUT_S,
        baud_throttle_ratio);
    sl_updater_free(instance);
    return success;
}

static void updater_cli_execute_917probe() {
    SlUpdater* instance = sl_updater_alloc();
    FuriString* version = furi_string_alloc();
    for(int i = 0; i < SL_PROBING_RETRIES; i++) {
        printf("Probing...\r\n");
        if(sl_update_probe(instance, i, version)) {
            printf("Success\r\n%s", furi_string_get_cstr(version));
            break;
        } else {
            if(i == SL_PROBING_RETRIES - 1) {
                printf("Probe failed\r\n");
                break;
            }
            printf("Probing failed, retrying (%d/%d)\r\n", i + 1, SL_PROBING_RETRIES);
        }
    }
    furi_string_free(version);
    sl_updater_free(instance);
}

static void updater_cli_execute_install(const char* manifest_path) {
    printf("Installing update bundle from: %s\r\n", manifest_path);

    UpdateConfig* state = update_config_alloc();
    Storage* storage = furi_record_open(RECORD_STORAGE);

    do {
        UpdateConfigValidation config_state = update_config_load(state, manifest_path);
        if(config_state != UpdateConfigValidationOK) {
            printf(
                "Failed to load updater configuration: %s\r\n",
                update_config_validation_get_error_str(config_state));
            break;
        }

        printf("Updater configuration valid\r\n");

        if(!update_config_write_pointer_file(storage, manifest_path)) {
            printf("Failed to write manifest path to pointer file.\r\n");
            break;
        }

        printf("Manifest path written to %s\r\n", EXT_PATH(UPDATE_POINTER_FILE_NAME));

        furi_hal_nvm_set_boot_mode(FuriHalNvmBootModeUpdate);
        printf("Boot mode set to Update. Rebooting...\r\n");
        furi_hal_power_reset();
    } while(false);

    furi_record_close(RECORD_STORAGE);
    update_config_free(state);
}

static void updater_cli_execute_install_tar(const char* path) {
    printf("Installing update bundle from: %s\r\n", path);

    UpdateConfig* state = update_config_alloc();
    Storage* storage = furi_record_open(RECORD_STORAGE);

    FuriString* file_path = furi_string_alloc();
    path_extract_dirname(path, file_path);

    FuriString* final_staging_path = furi_string_alloc();
    path_concat(furi_string_get_cstr(file_path), UPDATE_STAGING_ROOT, final_staging_path);

    FuriString* manifest_full_path = furi_string_alloc_printf(
        "%s/%s", furi_string_get_cstr(final_staging_path), UPDATE_CONFIG_FILENAME);

    FURI_LOG_D(TAG, "Final staging path: %s", furi_string_get_cstr(final_staging_path));

    cleanup_directory_recursive(storage, furi_string_get_cstr(final_staging_path));

    do {
        // 1. Create staging directory
        printf("Creating staging directory: %s\r\n", furi_string_get_cstr(final_staging_path));
        if(storage_common_mkdir(storage, furi_string_get_cstr(final_staging_path)) != FSE_OK) {
            FURI_LOG_E(
                TAG,
                "Failed to create package directory: %s",
                furi_string_get_cstr(final_staging_path));
            printf(
                ANSI_FG_RED "Failed to create package directory: %s\r\n" ANSI_RESET,
                furi_string_get_cstr(final_staging_path));
            break;
        }

        // 2. Unpack TAR
        printf("Unpacking TAR contents to: %s\r\n", furi_string_get_cstr(final_staging_path));
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
                printf(
                    ANSI_FG_RED "Failed to unpack TAR contents to %s\r\n" ANSI_RESET,
                    furi_string_get_cstr(final_staging_path));
            }
        } else {
            FURI_LOG_E(TAG, "Failed to open TAR file %s", path);
            printf(ANSI_FG_RED "Failed to open TAR file %s\r\n" ANSI_RESET, path);
        }

        tar_archive_free(tar);

        if(!unpack_success) {
            // Staging dir will be cleaned by on_close as reboot_initiated is false
            FURI_LOG_E(TAG, "Failed to unpack update TAR.");
            printf(ANSI_FG_RED "Failed to unpack update TAR.\r\n" ANSI_RESET);
            break;
        }
        printf(ANSI_FG_GREEN "TAR unpacked successfully\r\n" ANSI_RESET);

        // 3. Validate: Check for UPDATE_CONFIG_FILENAME
        printf("Checking for manifest: %s\r\n", furi_string_get_cstr(manifest_full_path));
        if(!storage_file_exists(storage, furi_string_get_cstr(manifest_full_path))) {
            FURI_LOG_E(
                TAG, "Manifest file not found: %s", furi_string_get_cstr(manifest_full_path));
            printf(
                ANSI_FG_RED "Manifest file not found: %s\r\n" ANSI_RESET,
                furi_string_get_cstr(manifest_full_path));
            break;
        }
        FURI_LOG_D(TAG, "Manifest found: %s", furi_string_get_cstr(manifest_full_path));
        printf(
            ANSI_FG_GREEN "Manifest found: %s\r\n" ANSI_RESET,
            furi_string_get_cstr(manifest_full_path));

        // 4. Validate the update package using update_config_load
        UpdateConfigValidation config_state =
            update_config_load(state, furi_string_get_cstr(manifest_full_path));
        if(config_state != UpdateConfigValidationOK) {
            printf(
                ANSI_FG_RED "Failed to load updater configuration: %s\r\n" ANSI_RESET,
                update_config_validation_get_error_str(config_state));
            break;
        }

        printf(ANSI_FG_GREEN "Updater configuration valid\r\n" ANSI_RESET);

        if(!update_config_write_pointer_file(storage, furi_string_get_cstr(manifest_full_path))) {
            printf(ANSI_FG_RED "Failed to write manifest path to pointer file.\r\n" ANSI_RESET);
            break;
        }

        printf(
            ANSI_FG_GREEN "Manifest path written to %s/%s\r\n" ANSI_RESET,
            furi_string_get_cstr(final_staging_path),
            UPDATE_POINTER_FILE_NAME);

        furi_hal_nvm_set_boot_mode(FuriHalNvmBootModeUpdate);
        printf(ANSI_FG_GREEN "Boot mode set to Update.\r\nRebooting...\r\n" ANSI_RESET);
        furi_delay_ms(100);
        furi_hal_power_reset();
    } while(false);

    furi_string_free(file_path);
    furi_string_free(final_staging_path);
    furi_string_free(manifest_full_path);
    furi_record_close(RECORD_STORAGE);
    update_config_free(state);
}

static void updater_cli_execute_install_web(const char* link) {
    printf("Installing update bundle from web: %s\r\n", link);
    FuriString* url = furi_string_alloc_set_str(link);
    FuriString* file_path = furi_string_alloc();
    path_concat(STORAGE_EXT_PATH_PREFIX, UPDATE_STAGING_ROOT, file_path);
    path_concat(furi_string_get_cstr(file_path), UPDATE_TAR_TMP, file_path);
    if(fetch_download_file(url, file_path)) {
        updater_cli_execute_install_tar(furi_string_get_cstr(file_path));
    }
    furi_string_free(url);
    furi_string_free(file_path);
}

void update_cli_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);
    FuriString* cmd = furi_string_alloc();
    FuriString* path = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            updater_cli_command_print_usage();
            break;
        }

        if(furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug) &&
           furi_string_equal_str(cmd, "917_probe")) {
            updater_cli_execute_917probe();
            break;
        }

        if(!args_read_string_and_trim(args, path)) {
            updater_cli_command_print_usage();
            break;
        }

        if(furi_string_equal_str(cmd, "install")) {
            updater_cli_execute_install(furi_string_get_cstr(path));
            break;
        }

        if(furi_string_equal_str(cmd, "install_tar")) {
            updater_cli_execute_install_tar(furi_string_get_cstr(path));
            break;
        }

        if(furi_string_equal_str(cmd, "install_web")) {
            updater_cli_execute_install_web(furi_string_get_cstr(path));
            break;
        }

        bool is_stack_image = false;
        if(furi_string_equal_str(cmd, "917_ta")) {
            is_stack_image = true;
        } else if(furi_string_equal_str(cmd, "917")) {
            is_stack_image = false;
        } else {
            updater_cli_command_print_usage();
            break;
        }

        for(int i = 0; i < SL_UPDATE_RETRIES; i++) {
            printf("Update in progress\r\n");
            if(updater_cli_execute(furi_string_get_cstr(path), is_stack_image, i)) {
                printf("Update succeeded\r\n");
                break;
            } else {
                if(i == SL_UPDATE_RETRIES - 1) {
                    printf("Update failed\r\n");
                    break;
                }
                printf("Update failed, retrying (%d/%d)\r\n", i + 1, SL_UPDATE_RETRIES);
            }
        }
    } while(false);

    furi_string_free(path);
    furi_string_free(cmd);
}
