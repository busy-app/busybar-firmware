#include "cli_command_status_lights.h"

#include <furi/furi.h>
#include <cli/args.h>
#include <containers/pipe.h>
#include <status_lights/status_lights.h>

static void cli_command_status_lights_print_usage(void) {
    printf("Usage:\r\n");
    printf("status_lights <0-255> <0-255> <0-255>\r\n");
}

void cli_command_status_lights(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

#ifdef SRV_STATUS_LIGHTS
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

    Color color = COLOR_MAKE_RGB(value[0], value[1], value[2]);
    StatusLights* status_lights = furi_record_open(RECORD_STATUS_LIGHTS);
    status_lights_run_preset(status_lights, StatusLightsPresetStaticColor, color);
    furi_record_close(RECORD_STATUS_LIGHTS);
#else
    UNUSED(args);
    UNUSED(cli_command_status_lights_print_usage);
    printf("Status Lights service is not available.\r\n");
#endif
}
