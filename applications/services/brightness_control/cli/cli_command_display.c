#include "cli_command_display.h"

#include <furi.h>
#include <furi_hal.h>
#include <cli/args.h>
#include <cli/cli_command.h>
#include <toolbox/strint.h>
#include <containers/pipe.h>
#include <storage/storage.h>

#include <gui/gui.h>
#include <gui/modules/image.h>
#include <back_display/back_display.h>
#include <front_display/front_display.h>
#include <light_sensor/light_sensor.h>
#include <brightness_control/brightness_control.h>

#define TAG "CliDisplay"

typedef enum {
    CliDisplayActionShow,
    CliDisplayActionBrightness,
    CliDisplayActionGamma,
    CliDisplayActionNum,
} CliDisplayAction;

static void cli_command_display_print_usage(void) {
    printf("Incorect arguments\r\nUsage: display <action> <action_args>\r\n\n");
    printf("Actions:\r\n");
    printf("\tshow <front|back> <path_file> - show image from path_file\r\n");
    printf("\tbrightness <0-100|auto> - set display brightness value or 'auto'\r\n");
}

static void cli_action_show(PipeSide* pipe, FuriString* args, GuiDisplayId id) {
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
            while(!cli_is_pipe_broken_or_is_etx_next_char(pipe)) {
                furi_delay_ms(50);
            }
        }

        with_gui(gui, { image_free(image); });
    } while(false);

    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI);
}

static void cli_action_brightness(PipeSide* pipe, FuriString* args) {
    UNUSED(pipe);
    do {
        int brightness = 0;

        bool auto_brightness = furi_string_equal_str(args, "auto");
        if(!auto_brightness && !args_read_int_and_trim(args, &brightness)) {
            printf("Error! Unable to parse '%s' as brightness value ", furi_string_get_cstr(args));
            break;
        }

        BrightnessControl* srv = furi_record_open(RECORD_BRIGHTNESS_CONTROL);
        if(auto_brightness) {
            brightness_control_set_auto_brightness(srv);
        } else {
            brightness_control_set_manual_brightness(srv, brightness);
        }
        furi_record_close(RECORD_BRIGHTNESS_CONTROL);
    } while(false);
}

static void calc_gamma_table(uint8_t max, float gamma, BackDisplayGammaTable* table) {
    printf("New gamma table: ");

    for(uint32_t i = 0; i < COUNT_OF(table->data); ++i) {
        const float value_in = (float)(i + 1) / COUNT_OF(table->data);
        const uint8_t value_out = ceilf(powf(value_in, gamma) * max);

        table->data[i] = value_out;
        printf("0x%02hhX, ", value_out);
    }

    printf("\r\n");
}

static void cli_action_gamma(PipeSide* pipe, FuriString* args, GuiDisplayId id) {
    UNUSED(pipe);

    if(id != GuiDisplayIdBack) {
        printf("Front display gamma setting is not implemented");
        return;
    }

    BackDisplayGammaTable gamma_table;

    int max, gamma;

    if(!args_read_int_and_trim(args, &max)) {
        printf("Error: missing maximum value");
        return;
    }

    if((max < 0) || (max >= 64)) {
        printf("Error: maximum value is out of range");
        return;
    }

    if(!args_read_int_and_trim(args, &gamma)) {
        printf("Error: missing gamma value");
        return;
    }

    if(gamma <= 0) {
        printf("Error: gamma value is out of range");
        return;
    }

    // Gamma is given as an integer in 1/100th units
    calc_gamma_table(max, gamma / 100.f, &gamma_table);

    BackDisplaySrv* back_display = furi_record_open(RECORD_BACK_DISPLAY);
    back_display_set_gamma_table(back_display, &gamma_table);
    furi_record_close(RECORD_BACK_DISPLAY);
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
    } else if(furi_string_cmp_str(action_type, "gamma") == 0) {
        *action = CliDisplayActionGamma;
        result = true;
    }

    furi_string_free(action_type);
    return result;
}

void cli_command_display(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);

    do {
        CliDisplayAction action = CliDisplayActionNum;
        if(!cli_cpmmand_display_get_action(args, &action)) {
            cli_command_display_print_usage();
            break;
        }

        if(action == CliDisplayActionShow) {
            GuiDisplayId display_id = GuiDisplayIdMax;
            if(!cli_command_display_get_id(args, &display_id)) {
                cli_command_display_print_usage();
                break;
            }
            cli_action_show(pipe, args, display_id);

        } else if(action == CliDisplayActionBrightness) {
            cli_action_brightness(pipe, args);

        } else if(action == CliDisplayActionGamma) {
            GuiDisplayId display_id = GuiDisplayIdMax;
            if(!cli_command_display_get_id(args, &display_id)) {
                cli_command_display_print_usage();
                break;
            }
            cli_action_gamma(pipe, args, display_id);
        }

    } while(false);
}
