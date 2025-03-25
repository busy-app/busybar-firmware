#include "sl_updater.h"

#include <furi.h>
#include <cli/cli.h>
#include <toolbox/args.h>

#define SL_UPDATE_M4_COMM_TIMEOUT_S  (6)
#define SL_UPDATE_NWP_COMM_TIMEOUT_S (25)

#define SL_UPDATE_RETRIES (3)

static void updater_cli_command_print_usage(void) {
    printf("Usage:\r\n");
    printf("update <u5|917|917_ta> path\r\n");
}

static bool updater_cli_execute(const char* path, bool is_stack_image) {
    SlUpdater* instance = sl_updater_alloc();
    bool success = sl_updater_run(
        instance,
        path,
        is_stack_image,
        is_stack_image ? SL_UPDATE_NWP_COMM_TIMEOUT_S : SL_UPDATE_M4_COMM_TIMEOUT_S);
    sl_updater_free(instance);
    return success;
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

        if(furi_string_cmp_str(cmd, "u5") == 0) {
            printf("Not yet implemented\r\n");
            break;
        }

        if(!args_read_string_and_trim(args, path)) {
            updater_cli_command_print_usage();
            break;
        }

        bool is_stack_image = false;
        if(furi_string_cmp_str(cmd, "917_ta") == 0) {
            is_stack_image = true;
        } else if(furi_string_cmp_str(cmd, "917") == 0) {
            is_stack_image = false;
        } else {
            updater_cli_command_print_usage();
            break;
        }

        for(int i = 0; i < SL_UPDATE_RETRIES; i++) {
            printf("Update in progress");
            if(updater_cli_execute(furi_string_get_cstr(path), is_stack_image)) {
                printf("Update succeeded\r\n");
                break;
            } else {
                printf("Update failed, retrying (%d/%d)\r\n", i + 1, SL_UPDATE_RETRIES);
            }
        }
    } while(false);

    furi_string_free(path);
    furi_string_free(cmd);
}

void sl_update_on_system_start(void) {
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, "update", CliCommandFlagParallelSafe, updater_cli, NULL);
    furi_record_close(RECORD_CLI);
}
