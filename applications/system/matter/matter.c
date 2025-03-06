#include "matter.h"
#include <furi.h>
#include <args.h>
#include <cli_worker.h>
#include "helpers/matter_app.h"

void matter_command_start(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);
    UNUSED(args);

    matter_app_init();

    printf("\r\nExit Matter app\r\n");
}

static void matter_command_print_usage(void) {
    printf("Usage:\r\n");
    printf("matter \"Matter app\"\r\n");
}

static void matter_command(Cli* cli, FuriString* args, void* context) {
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            matter_command_start(cli, args, context);
            break;
        }

        matter_command_print_usage();
    } while(false);

    furi_string_free(cmd);
}

void matter_system_start(void) {
#ifdef SRV_CLI
    Cli* cli = (Cli*)furi_record_open(RECORD_CLI);

    cli_add_command(cli, "matter", CliCommandFlagParallelSafe, matter_command, NULL);

    furi_record_close(RECORD_CLI);
#else
    UNUSED(matter_command);
#endif
}
