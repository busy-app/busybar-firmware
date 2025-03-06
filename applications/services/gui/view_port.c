#include "view_port_i.h"

#include <lvgl/src/core/lv_obj_class_private.h>

#define MY_CLASS (&lv_view_port_class)

ViewPort* view_port_alloc(Gui* gui, GuiDisplayId display_id, GuiLayerId layer_id) {
    // Input parameter checks are done in the first call
    lv_obj_t* active_layer = gui_get_layer(gui, display_id, layer_id);
    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, active_layer);
    lv_obj_class_init_obj(obj);

    ViewPort* instance = (ViewPort*)obj;
    return instance;
}

void view_port_free(ViewPort* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

void view_port_set_width(ViewPort* instance, int32_t width) {
    furi_check(instance);
    lv_obj_set_width((lv_obj_t*)instance, width);
}

void view_port_set_height(ViewPort* instance, int32_t height) {
    furi_check(instance);
    lv_obj_set_height((lv_obj_t*)instance, height);
}

void view_port_set_size(ViewPort* instance, int32_t width, int32_t height) {
    furi_check(instance);
    lv_obj_set_size((lv_obj_t*)instance, width, height);
}

void view_port_set_enabled(ViewPort* instance, bool enabled) {
    furi_check(instance);
    if(enabled) {
        lv_obj_remove_flag((lv_obj_t*)instance, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag((lv_obj_t*)instance, LV_OBJ_FLAG_HIDDEN);
    }
}

void view_port_move_to_foreground(ViewPort* instance) {
    furi_check(instance);
    lv_obj_move_foreground((lv_obj_t*)instance);
}

void view_port_move_to_background(ViewPort* instance) {
    furi_check(instance);
    lv_obj_move_background((lv_obj_t*)instance);
}

const lv_obj_class_t lv_view_port_class = {
    .base_class = &lv_obj_class,
    .name = "view-port",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(ViewPort),
};
