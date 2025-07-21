#include "matter.h"
#include <furi.h>
#include <cli/args.h>
#include <containers/pipe.h>
#include "helpers/matter_app.h"

void matter_command_start(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);
    UNUSED(args);

    matter_app_init();

    printf("\r\nMatter app start\r\n");
}

void matter_command_factory_reset(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);
    UNUSED(args);

    matter_factory_reset();

    printf("\r\nMatter app factory reset\r\n");
}

void matter_command_button_release(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);
    UNUSED(args);

    matter_button_release();

    printf("\r\nMatter app button release\r\n");
}

void matter_command_button_press(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);
    UNUSED(args);

    matter_button_press();

    printf("\r\nMatter app button press\r\n");
}

void matter_command_basic_commissioning_window(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
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

void matter_command(PipeSide* pipe, FuriString* args, void* context) {
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            matter_command_start(pipe, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "res") == 0) {
            matter_command_factory_reset(pipe, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "b1") == 0) {
            matter_command_button_press(pipe, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "b0") == 0) {
            matter_command_button_release(pipe, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "comm") == 0) {
            matter_command_basic_commissioning_window(pipe, args, context);
            break;
        }

        matter_command_print_usage();
    } while(false);

    furi_string_free(cmd);
}
