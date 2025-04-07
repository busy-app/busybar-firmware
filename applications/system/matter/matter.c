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

    printf("\r\nMatter app start\r\n");
}

void matter_command_factory_reset(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);
    UNUSED(args);

    matter_factory_reset();

    printf("\r\nMatter app factory reset\r\n");
}

void matter_command_button_release(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);
    UNUSED(args);

    matter_button_release();

    printf("\r\nMatter app button release\r\n");
}

void matter_command_button_press(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);
    UNUSED(args);

    matter_button_press();

    printf("\r\nMatter app button press\r\n");
}

void matter_command_basic_commissioning_window (Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);
    UNUSED(args);

    matter_basic_commissioning_window();

    printf("\r\nMatter app basic commissioning window\r\n");
}

static void matter_command_print_usage(void) {
    printf("Usage:\r\n");
    printf("matter \"Matter app start\"\r\n");
    printf("matter res \"Matter app factory reset\"\r\n");
    printf("matter b1 \"Matter app button press\"\r\n");
    printf("matter b0 \"Matter app button release\"\r\n");
    printf("matter comm \"Matter app basic commissioning window\"\r\n");
}

static void matter_command(Cli* cli, FuriString* args, void* context) {
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            matter_command_start(cli, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "res") == 0) {
            matter_command_factory_reset(cli, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "b1") == 0) {
            matter_command_button_press(cli, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "b0") == 0) {
            matter_command_button_release(cli, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "comm") == 0) {
            matter_command_basic_commissioning_window(cli, args, context);
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
