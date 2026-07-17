#include "updater.h"
#include "sl_updater/sl_updater.h"
#include "sl_updater/sl_update_params.h"

#include <furi.h>
#include <furi_hal_nvm.h>
#include <furi_hal_power.h>

#include <cli/cli_command.h>
#include <cli/cli_ansi.h>
#include <cli/args.h>

#include <toolbox/update_lib/update_config.h>
#include <toolbox/update_lib/common_vals.h>
#include <toolbox/tar/tar_archive.h>
#include <toolbox/path.h>

#define TAG                 "UpdaterCli"
#define UPDATE_STAGING_ROOT "update"
#define UPDATE_TAR_TEMP     "upload.tar"

typedef struct {
    SlUpdaterProgressPhase progress_phase;
    size_t spinner_item_idx;
} UpdateCliProgressCallbackContext;

static void
    updater_cli_progress_callback(SlUpdaterProgressPhase phase, uint8_t percentage, void* context) {
    UpdateCliProgressCallbackContext* callback_context = context;

    if(phase != callback_context->progress_phase) {
        callback_context->progress_phase = phase;
        putchar('\n');
    }

    switch(phase) {
    case SlUpdaterProgressPhaseUploading: {
        printf(
            ANSI_CURSOR_UP_BY("1")
                ANSI_ERASE_LINE(ANSI_ERASE_FROM_CURSOR_TO_END) "\rUploading: %d%%...\r\n",
            percentage);
        break;
    }

    case SlUpdaterProgressPhaseAwaitingInstall: {
        static const char spinner_items[] = {'|', '/', '-', '\\'};

        printf(
            ANSI_CURSOR_UP_BY("1")
                ANSI_ERASE_LINE(ANSI_ERASE_FROM_CURSOR_TO_END) "\rAwaiting installation... %c\r\n",
            spinner_items[callback_context->spinner_item_idx % COUNT_OF(spinner_items)]);

        callback_context->spinner_item_idx++;
        break;
    }

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

static SlUpdaterStatus
    updater_cli_execute(const char* path, bool is_stack_image, uint8_t baud_throttle_ratio) {
    SlUpdater* instance = sl_updater_alloc();

    sl_updater_set_progress_callback(
        instance,
        updater_cli_progress_callback,
        &(UpdateCliProgressCallbackContext){
            .progress_phase = SlUpdaterProgressPhaseNone,
            .spinner_item_idx = 0,
        });

    SlUpdaterStatus status = sl_updater_run(
        instance,
        path,
        is_stack_image,
        is_stack_image ? SL_UPDATE_NWP_COMM_TIMEOUT_S : SL_UPDATE_M4_COMM_TIMEOUT_S,
        baud_throttle_ratio);

    sl_updater_free(instance);
    return status;
}

static void updater_cli_execute_917probe() {
    SlUpdater* instance = sl_updater_alloc();
    FuriString* version = furi_string_alloc();
    for(int i = 0; i < SL_PROBING_RETRIES; i++) {
        printf("Probing...\r\n");
        if(sl_update_probe(instance, i, version) == SlUpdaterStatusOk) {
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
    Updater* updater = furi_record_open(RECORD_UPDATER);

    printf("Installing update bundle from: %s\r\n", manifest_path);

    UpdaterStatus update_status = updater_session_start(updater);
    if(update_status == UpdaterStatusOk) {
        do {
            update_status = updater_installation_prepare(updater, manifest_path, true);
            if(update_status != UpdaterStatusOk) {
                printf(
                    "Update preparation failed: %s\r\n", updater_get_status_string(update_status));

                break;
            }

            printf("Update preparation successful, rebooting...\r\n");

            updater_installation_apply(updater, true);
        } while(false);

        updater_session_stop(updater);
    } else {
        printf("Update not allowed: %s\r\n", updater_get_status_string(update_status));
    }

    furi_record_close(RECORD_UPDATER);
}

static void updater_cli_execute_install_tar(const char* tar_path) {
    Updater* updater = furi_record_open(RECORD_UPDATER);

    printf("Installing update bundle from: %s\r\n", tar_path);

    UpdaterStatus update_status = updater_session_start(updater);
    if(update_status == UpdaterStatusOk) {
        do {
            update_status = updater_unpack(updater, tar_path, NULL, NULL, true);
            if(update_status != UpdaterStatusOk) {
                printf(
                    "Update unpack TAR failed: %s\r\n", updater_get_status_string(update_status));

                break;
            }

            update_status = updater_installation_prepare(updater, NULL, true);
            if(update_status != UpdaterStatusOk) {
                printf(
                    "Update preparation failed: %s\r\n", updater_get_status_string(update_status));

                break;
            }

            printf("Update preparation successful, rebooting...\r\n");

            updater_installation_apply(updater, true);
        } while(false);

        updater_session_stop(updater);
    } else {
        printf("Update not allowed: %s\r\n", updater_get_status_string(update_status));
    }

    furi_record_close(RECORD_UPDATER);
}

static void updater_cli_execute_install_web(const char* link) {
    Updater* updater = furi_record_open(RECORD_UPDATER);

    printf("Installing update bundle from: %s\r\n", link);

    UpdaterStatus update_status = updater_session_start(updater);
    if(update_status == UpdaterStatusOk) {
        do {
            update_status = updater_download(updater, link, NULL, true);
            if(update_status != UpdaterStatusOk) {
                printf("Download failed: %s\r\n", updater_get_status_string(update_status));

                break;
            }

            update_status = updater_unpack(updater, NULL, NULL, NULL, true);
            if(update_status != UpdaterStatusOk) {
                printf(
                    "Update unpack TAR failed: %s\r\n", updater_get_status_string(update_status));

                break;
            }

            update_status = updater_installation_prepare(updater, NULL, true);
            if(update_status != UpdaterStatusOk) {
                printf(
                    "Update preparation failed: %s\r\n", updater_get_status_string(update_status));

                break;
            }

            printf("Update preparation successful, rebooting...\r\n");

            updater_installation_apply(updater, true);
        } while(false);

        updater_session_stop(updater);
    } else {
        printf("Update not allowed: %s\r\n", updater_get_status_string(update_status));
    }

    furi_record_close(RECORD_UPDATER);
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

        printf("Starting update...\r\n");

        for(size_t i = 0; i < SL_UPDATE_RETRIES; i++) {
            SlUpdaterStatus update_status =
                updater_cli_execute(furi_string_get_cstr(path), is_stack_image, i);

            if(update_status == SlUpdaterStatusOk) {
                printf("Update installed successfully.\r\n");
                break;
            }

            if(update_status == SlUpdaterStatusFileNotFound) {
                printf("Firmware file not found: %s.\r\n", furi_string_get_cstr(path));
                break;
            }

            if(i >= SL_UPDATE_RETRIES - 1) {
                printf("Update installation failed.\r\n");
                break;
            }

            printf("Update installation failed, retrying: %d/%d...\r\n", i + 1, SL_UPDATE_RETRIES);
        }
    } while(false);

    furi_string_free(path);
    furi_string_free(cmd);
}
