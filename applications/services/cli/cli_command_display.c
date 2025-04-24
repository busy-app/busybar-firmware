#include "cli_command_display.h"

#include <furi.h>
#include <furi_hal.h>
#include <toolbox/args.h>
#include <toolbox/strint.h>
#include <storage/storage.h>

#include <gui/gui.h>
#include <gui/modules/image.h>
#include <back_display/back_display.h>
#include <front_display/front_display.h>
#include <light_sensor/light_sensor.h>

#define TAG "CliDisplay"

#define CLI_DISPLAY_BRIGHTNESS_MAX (100)

typedef enum {
    CliDisplayActionShow,
    CliDisplayActionBrightness,
    CliDisplayActionNum,
} CliDisplayAction;

static void cli_command_display_print_usage(void) {
    printf("Incorect arguments\r\nUsage: display <front|back> <action> <action_args>\r\n\n");
    printf("Actions:\r\n");
    printf("\tshow <path_file> - show image from path_file\r\n");
    printf("\tbrightness <0-100|auto> - set display brightness value or 'auto'\r\n");
}

static void cli_action_show(Cli* cli, FuriString* args, GuiDisplayId id) {
    Gui* gui = furi_record_open(RECORD_GUI);
    Storage* storage = furi_record_open(RECORD_STORAGE);

    do {
        if(!storage_file_exists(storage, furi_string_get_cstr(args))) {
            printf("Error! File not found '%s'", furi_string_get_cstr(args));
            break;
        }

        Image* image;

        bool result = false;
        with_gui(gui, {
            GuiLayer* system_layer = gui_get_layer(gui, GuiLayerIdSystem);
            Widget* root = gui_layer_get_root_widget(system_layer, id);
            image = image_alloc(root);
            result = image_set_source(image, furi_string_get_cstr(args));
        });

        if(!result) {
            printf("Error! Unable to set '%s' as image source", furi_string_get_cstr(args));
        } else {
            while(!cli_cmd_interrupt_received(cli)) {
                furi_delay_ms(50);
            }
        }

        with_gui(gui, { image_free(image); });
    } while(false);

    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI);
}

static void cli_action_brightness(Cli* cli, FuriString* args, GuiDisplayId id) {
    UNUSED(cli);
    do {
        int brightness = 0;

        bool auto_brightness = furi_string_equal_str(args, "auto");
        if(!auto_brightness && !args_read_int_and_trim(args, &brightness)) {
            printf("Error! Unable to parse '%s' as brightness value ", furi_string_get_cstr(args));
            break;
        }

        if(brightness > CLI_DISPLAY_BRIGHTNESS_MAX) brightness = CLI_DISPLAY_BRIGHTNESS_MAX;

        if(id == GuiDisplayIdBack) {
            BackDisplaySrv* srv = furi_record_open(RECORD_BACK_DISPLAY);
            uint8_t back_display_brightness = auto_brightness ? BACK_DISPLAY_BRIGHTNESS_AUTO :
                                                                brightness;
            back_display_set_brightness(srv, back_display_brightness);
            furi_record_close(RECORD_BACK_DISPLAY);
        } else if(id == GuiDisplayIdFront) {
            DotMatrixSrv* srv = furi_record_open(RECORD_FRONT_DISPLAY);
            uint8_t matrix_brightness = auto_brightness ? FRONT_DISPLAY_BRIGHTNESS_AUTO :
                                                          brightness;
            front_display_set_brightness(srv, matrix_brightness);
            furi_record_close(RECORD_FRONT_DISPLAY);
        }
    } while(false);
}

static bool cli_command_display_get_id(FuriString* args, GuiDisplayId* display_id) {
    FuriString* display_type = furi_string_alloc();

    bool result = false;
    args_read_string_and_trim(args, display_type);
    if(furi_string_cmp_str(display_type, "front") == 0) {
        *display_id = GuiDisplayIdFront;
        result = true;
    } else if(furi_string_cmp_str(display_type, "back") == 0) {
        *display_id = GuiDisplayIdBack;
        result = true;
    }

    furi_string_free(display_type);
    return result;
}

static bool cli_cpmmand_display_get_action(FuriString* args, CliDisplayAction* action) {
    FuriString* action_type = furi_string_alloc();

    bool result = false;
    args_read_string_and_trim(args, action_type);
    if(furi_string_cmp_str(action_type, "show") == 0) {
        *action = CliDisplayActionShow;
        result = true;
    } else if(furi_string_cmp_str(action_type, "brightness") == 0) {
        *action = CliDisplayActionBrightness;
        result = true;
    }

    furi_string_free(action_type);
    return result;
}

void cli_command_display(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);

    do {
        GuiDisplayId display_id = GuiDisplayIdMax;
        if(!cli_command_display_get_id(args, &display_id)) {
            cli_command_display_print_usage();
            break;
        }

        CliDisplayAction action = CliDisplayActionNum;
        if(!cli_cpmmand_display_get_action(args, &action)) {
            cli_command_display_print_usage();
            break;
        }

        if(action == CliDisplayActionShow) {
            cli_action_show(cli, args, display_id);
        } else if(action == CliDisplayActionBrightness) {
            cli_action_brightness(cli, args, display_id);
        }

    } while(false);
}
