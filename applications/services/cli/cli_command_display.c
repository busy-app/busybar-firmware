#include "cli_command_display.h"

#include <furi.h>
#include <furi_hal.h>
#include <toolbox/args.h>
#include <storage/storage.h>

#include <gui_lvgl/gui_lvgl.h>
#include <led_display/led_display.h>
#include <lib/lvgl/src/widgets/canvas/lv_canvas.h>

#define TAG "CliDisplay"

void cli_command_dled(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(args);
    UNUSED(context);

    GuiLvgl* gui = furi_record_open(RECORD_GUI_LVGL);

    gui_lvgl_acquire(gui);

    lv_obj_t* active = gui_lvgl_get_layer(gui, GuiDisplayIdFront, GuiLayerIdActive);
    lv_obj_t* image = lv_image_create(active);
    lv_image_set_src(image, "E:/ext/test.png");
    lv_obj_set_pos(image, 0, 0);
    // uint8_t* canvas_buffer = malloc(DOT_MATRIX_BUF_SIZE);
    // lv_obj_t* canvas = lv_canvas_create(active);
    // lv_canvas_set_buffer(canvas, canvas_buffer, 72, 16, LV_COLOR_FORMAT_RGB888);
    // lv_group_add_obj(lv_group_get_default(), canvas);
    // lv_group_focus_obj(canvas);
    // lv_canvas_set_px(canvas, 10, 10, lv_color_white(), 50);

    gui_lvgl_release(gui);

    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI_LVGL);

    printf("Hello");
}
