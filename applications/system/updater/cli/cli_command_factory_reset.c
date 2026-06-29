#include "cli_command_factory_reset.h"

#include <toolbox/update_lib/factory_reset.h>

#include <storage/storage.h>
#include <applications/system/updater/updater.h>

#include <furi_hal_nvm.h>
#include <cli/args.h>

typedef struct {
    bool shipping_mode;
    bool help;
} FactoryResetArgs;

static bool parse_command_args(FuriString* args, FactoryResetArgs* parsed_args) {
    parsed_args->shipping_mode = false;
    parsed_args->help = false;

    FuriString* arg = furi_string_alloc();

    bool is_success = true;
    while(args_read_string_and_trim(args, arg)) {
        if(furi_string_equal_str(arg, "-s") || furi_string_equal_str(arg, "--shipping-mode")) {
            parsed_args->shipping_mode = true;
        } else if(furi_string_equal_str(arg, "-h") || furi_string_equal_str(arg, "--help")) {
            parsed_args->help = true;
        } else {
            printf("Unknown argument: %s\r\n", furi_string_get_cstr(arg));
            is_success = false;
            break;
        }
    }

    furi_string_free(arg);
    return is_success;
}

static void print_command_help(void) {
    printf("Usage: factory_reset [options]\r\n");
    printf("Options:\r\n");
    printf("  -s, --shipping-mode    Enter shipping mode after performing reset\r\n");
    printf("  -h, --help             Show this help message\r\n");
}

void cli_command_factory_reset(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

    FactoryResetArgs _args;
    if(!parse_command_args(args, &_args) || _args.help) {
        print_command_help();
        return;
    }

    Updater* updater = furi_record_open(RECORD_UPDATER);

    UpdaterStatus update_status = updater_session_start(updater);
    if(update_status == UpdaterStatusOk) {
        printf("Warning! This will wipe all the data from the device! Are you sure? y/n\r\n");

        for(char response; pipe_receive(pipe, &response, sizeof(response)) == sizeof(response);) {
            if(response == 'y' || response == 'Y') {
                factory_reset_perform(updater, _args.shipping_mode);
                break;
            } else if(response == 'n' || response == 'N') {
                printf("\r\nCancelled");
                break;
            }
        }
    } else {
        printf("Factory reset is not allowed: %s\r\n", updater_get_status_string(update_status));
    }

    updater_session_stop(updater);
    furi_record_close(RECORD_UPDATER);
}
