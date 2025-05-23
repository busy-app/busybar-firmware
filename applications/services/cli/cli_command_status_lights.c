#include "cli_command_status_lights.h"

#include <furi/furi.h>
#include <toolbox/args.h>
#include <status_lights/status_lights.h>

static void cli_command_status_lights_print_usage(void) {
    printf("Usage:\r\n");
    printf("status_lights <0-255> <0-255> <0-255>\r\n");
}

void cli_command_status_lights(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);

    int value[3] = {};

    size_t i = 0;
    for(i = 0; i < COUNT_OF(value); i++) {
        if(!args_read_int_and_trim(args, &value[i])) break;
        if((value[i] < 0) || (value[i] > 255)) break;
    }
    if(i != COUNT_OF(value)) {
        cli_command_status_lights_print_usage();
        return;
    }

    const StatusLightsCommand command = {
        .preset = StatusLightsPresetStaticColor,
        .color =
            {
                .r = value[0],
                .g = value[1],
                .b = value[2],
            },
    };

    StatusLights* status_lights = furi_record_open(RECORD_STATUS_LIGHTS);
    status_lights_send_command(status_lights, &command);
    furi_record_close(RECORD_STATUS_LIGHTS);
}
