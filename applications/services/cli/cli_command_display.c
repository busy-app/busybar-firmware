#include "cli_command_display.h"

#include <furi.h>
#include <furi_hal.h>
#include <toolbox/args.h>
#include <storage/storage.h>

#include <gui_lvgl/gui_lvgl.h>
#include <led_display/led_display.h>
#include <lib/lvgl/src/widgets/canvas/lv_canvas.h>

#define TAG "CliDisplay"

static void cli_command_display(Cli* cli, FuriString* args, GuiDisplayId id) {
    GuiLvgl* gui = furi_record_open(RECORD_GUI_LVGL);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* cmd = furi_string_alloc();

    bool arguments_parsed = false;
    do {
        args_read_string_and_trim(args, cmd);
        if(furi_string_cmp_str(cmd, "show") != 0) {
            printf(
                "Incorrect argument. Command usage:\r\n%s show <file_path>\r\n",
                id == GuiDisplayIdFront ? "dled" : "oled");
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
        return;
    }

    gui_lvgl_acquire(gui);

    lv_obj_t* active = gui_lvgl_get_layer(gui, id, GuiLayerIdActive);
    lv_obj_clean(active);
    lv_obj_t* image = lv_image_create(active);
    lv_image_set_src(image, furi_string_get_cstr(args));
    lv_obj_set_pos(image, 0, 0);

    gui_lvgl_release(gui);

    while(!cli_cmd_interrupt_received(cli)) {
    };

    gui_lvgl_acquire(gui);
    lv_obj_delete(image);
    gui_lvgl_release(gui);

    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI_LVGL);
}

void cli_command_dled(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    cli_command_display(cli, args, GuiDisplayIdFront);
}

void cli_command_oled(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    cli_command_display(cli, args, GuiDisplayIdBack);
}
