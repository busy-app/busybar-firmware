#include "update.h"
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
#include <applications/system/fetch/fetch.h>

#define TAG                 "UpdaterCli"
#define UPDATE_STAGING_ROOT "update"
#define UPDATE_TAR_TEMP     "upload.tar"

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

    do {
        UpdaterStatus prepare_install_status = updater_prepare_install(manifest_path);
        if(prepare_install_status != UpdaterStatusSuccess) {
            printf(
                "Update prepare install failed: %s\r\n",
                updater_get_status_string(prepare_install_status));

            break;
        }

        updater_reboot_install();
    } while(false);
}

static void updater_cli_execute_install_tar(const char* tar_path) {
    printf("Installing update bundle from: %s\r\n", tar_path);

    FuriString* manifest_path = furi_string_alloc();

    do {
        UpdaterStatus unpack_tar_status = updater_unpack_tar(tar_path, NULL, manifest_path);
        if(unpack_tar_status != UpdaterStatusSuccess) {
            printf(
                "Update unpack TAR failed: %s\r\n", updater_get_status_string(unpack_tar_status));

            break;
        }

        UpdaterStatus prepare_install_status =
            updater_prepare_install(furi_string_get_cstr(manifest_path));
        if(prepare_install_status != UpdaterStatusSuccess) {
            printf(
                "Update prepare install failed: %s\r\n",
                updater_get_status_string(prepare_install_status));

            break;
        }

        updater_reboot_install();
    } while(false);

    furi_string_free(manifest_path);
}

static void updater_cli_execute_install_web(const char* link) {
    printf("Installing update bundle from web: %s\r\n", link);

    FuriString* url = furi_string_alloc_set_str(link);
    FuriString* file_path =
        furi_string_alloc_set_str(EXT_PATH(UPDATE_STAGING_ROOT "/" UPDATE_TAR_TEMP));

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
