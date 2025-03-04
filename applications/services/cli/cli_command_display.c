#include "cli_command_display.h"

#include <furi.h>
#include <furi_hal.h>
#include <toolbox/args.h>
#include <storage/storage.h>

#include <gui/gui.h>

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

    gui_lock(gui);

    lv_obj_t* system = gui_get_layer(gui, id, GuiLayerIdSystem);
    lv_obj_t* screen = lv_obj_create(system);
    lv_obj_set_size(screen, lv_obj_get_width(system), lv_obj_get_height(system));
    lv_obj_t* image = lv_image_create(screen);
    lv_image_set_src(image, furi_string_get_cstr(args));
    lv_obj_set_pos(image, 0, 0);

    gui_unlock(gui);

    while(!cli_cmd_interrupt_received(cli)) {
    };

    gui_lock(gui);
    lv_obj_delete(screen);
    gui_unlock(gui);

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
