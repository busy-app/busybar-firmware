#include "cli_command_display.h"

#include <furi.h>
#include <furi_hal.h>
#include <toolbox/args.h>
#include <storage/storage.h>

#include <gui/gui.h>
#include <gui/modules/image.h>

#define TAG "CliDisplay"

static void cli_command_display_print_usage(void) {
    printf("Incorect arguments\r\nUsage: display <front|back> show <file_path>");
}

static void cli_command_show(Cli* cli, FuriString* args, GuiDisplayId id) {
    Gui* gui = furi_record_open(RECORD_GUI);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* cmd = furi_string_alloc();

    bool arguments_parsed = false;
    do {
        args_read_string_and_trim(args, cmd);
        if(furi_string_cmp_str(cmd, "show") != 0) {
            cli_command_display_print_usage();
            break;
        }

        if(storage_common_stat(storage, furi_string_get_cstr(args), NULL) != FSE_OK) {
            printf("Not found file %s", furi_string_get_cstr(args));
            break;
        }

        arguments_parsed = true;
    } while(false);

    furi_string_free(cmd);
    if(!arguments_parsed) {
        furi_record_close(RECORD_STORAGE);
        furi_record_close(RECORD_GUI);
        return;
    }

    Image* image;

    with_gui(gui, {
        GuiLayer* system_layer = gui_get_layer(gui, GuiLayerIdSystem);
        Widget* root = gui_layer_get_root_widget(system_layer, id);
        image = image_alloc(root);
        image_set_source(image, furi_string_get_cstr(args));
    });

    while(!cli_cmd_interrupt_received(cli)) {
    };

    with_gui(gui, { image_free(image); });

    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI);
}

void cli_command_display(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    FuriString* display_type = furi_string_alloc();
    args_read_string_and_trim(args, display_type);

    if(furi_string_cmp_str(display_type, "front") == 0) {
        cli_command_show(cli, args, GuiDisplayIdFront);
    } else if(furi_string_cmp_str(display_type, "back") == 0) {
        cli_command_show(cli, args, GuiDisplayIdBack);
    } else {
        cli_command_display_print_usage();
    }

    furi_string_free(display_type);
}
