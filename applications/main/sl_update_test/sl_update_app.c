#include "sl_update.h"

#include <furi.h>
#include <cli/cli.h>
#include <toolbox/args.h>

int32_t sl_update_app(void* arg) {
    UNUSED(arg);

    SlUpdater* instance = sl_updater_alloc();
    sl_updater_run(instance, "/ext/firmware.rps", false, 6);
    sl_updater_free(instance);

    return 0;
}

static void updater_cli_command_print_usage(void) {
    printf("Usage:\r\n");
    printf("update <u5|917|917_ta> path\r\n");
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
        if(furi_string_cmp_str(cmd, "917") == 0) {
            is_stack_image = false;
        } else if(furi_string_cmp_str(cmd, "917_ta") == 0) {
            is_stack_image = true;
        } else {
            updater_cli_command_print_usage();
            break;
        }

        SlUpdater* instance = sl_updater_alloc();
        if(!sl_updater_run(instance, furi_string_get_cstr(path), is_stack_image, 6)) {
            printf("Update failed\r\n");
        } else {
            printf("Update succeeded\r\n");
        }
        sl_updater_free(instance);

    } while(false);

    furi_string_free(path);
    furi_string_free(cmd);
}

void sl_update_on_system_start(void) {
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, "update", CliCommandFlagDefault, updater_cli, NULL);
    furi_record_close(RECORD_CLI);
}
