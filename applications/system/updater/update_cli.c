#include "sl_updater.h"
#include "update_config.h"

#include <furi.h>
#include <furi_hal_nvm.h>
#include <furi_hal_power.h>

#include <storage/storage.h>
#include <cli/cli.h>

#include <toolbox/args.h>
#include <toolbox/update_lib/common_vals.h>

#define SL_UPDATE_M4_COMM_TIMEOUT_S  (15)
#define SL_UPDATE_NWP_COMM_TIMEOUT_S (30)

#define SL_UPDATE_RETRIES  (3)
#define SL_PROBING_RETRIES (3)

static void updater_cli_progress_callback(uint8_t percentage, void* context) {
    UNUSED(context);
    printf("Update progress: %d%%\r", percentage);
}

static void updater_cli_command_print_usage(void) {
    bool is_debug = furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug);
    printf("Usage:\r\n");
    printf("update <917|917_ta%s|install> path\r\n", is_debug ? "|917_probe" : "");
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

static void updater_cli(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
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

void update_on_system_start(void) {
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, "update", CliCommandFlagParallelSafe, updater_cli, NULL);
    furi_record_close(RECORD_CLI);
}
