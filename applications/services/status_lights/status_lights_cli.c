#include <cli/args.h>

#include <containers/pipe.h>

#include <status_lights/status_lights.h>

static void status_lights_cli_print_usage(void) {
    printf("Usage:\r\n");
    printf("status_lights <0-255> <0-255> <0-255>\r\n");
}

static bool status_lights_cli_parse_color(FuriString* args, Color* color) {
    bool success = true;

    int value[3];

    for(uint32_t i = 0; i < COUNT_OF(value); i++) {
        if(!args_read_int_and_trim(args, &value[i])) {
            success = false;
            break;
        }

        if((value[i] < 0) || (value[i] > UINT8_MAX)) {
            success = false;
            break;
        };
    }

    if(success) {
        *color = (Color)COLOR_MAKE_RGB(value[0], value[1], value[2]);
    }

    return success;
}

void status_lights_cli_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    Color color;

    if(!status_lights_cli_parse_color(args, &color)) {
        status_lights_cli_print_usage();
        return;
    }

    StatusLights* status_lights = furi_record_open(RECORD_STATUS_LIGHTS);
    const StatusLightsStatus status =
        status_lights_run_preset(status_lights, StatusLightsPresetStaticColor, color);
    furi_record_close(RECORD_STATUS_LIGHTS);

    if(status != StatusLightsStatusOk) {
        printf("Failed to set status lights color\r\n");
    }
}
