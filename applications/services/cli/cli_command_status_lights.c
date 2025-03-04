#include "cli_command_status_lights.h"

#include <furi/furi.h>
#include <toolbox/args.h>
#include <status_lights/status_lights.h>

static void cli_command_status_lights_print_usage(void) {
    printf("Usage:\r\n");
    printf("status_lights <R|G|B> <0-255>\r\n");
}

void cli_command_status_lights(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(args);
    UNUSED(context);

    FuriString* str_tmp = furi_string_alloc();
    int value = 0;
    StatusLightsCommand command = {};
    command.type = StatusLightsCommandSetManual;

    bool cmd_parsed = false;
    do {
        if(!args_read_string_and_trim(args, str_tmp)) break;
        if(!args_read_int_and_trim(args, &value)) break;
        if((value < 0) || (value > 255)) break;

        if(furi_string_cmp_str(str_tmp, "R") == 0) {
            command.color.r = value;
        } else if(furi_string_cmp_str(str_tmp, "G") == 0) {
            command.color.g = value;
        } else if(furi_string_cmp_str(str_tmp, "B") == 0) {
            command.color.b = value;
        } else {
            break;
        }

        cmd_parsed = true;
    } while(false);

    furi_string_free(str_tmp);

    if(!cmd_parsed) {
        cli_command_status_lights_print_usage();
        return;
    }

    StatusLights* status_lights = furi_record_open(RECORD_STATUS_LIGHTS);
    status_lights_send_command(status_lights, command);
    furi_record_close(RECORD_STATUS_LIGHTS);
}
