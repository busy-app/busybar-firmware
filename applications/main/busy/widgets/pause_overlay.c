#include "pause_overlay.h"

#include <gui/widget_i.h>

#include "../compiled_assets/compiled_assets.h"

#define MY_CLASS (&pause_overlay_lvgl_class)

#define BG_OPACITY (180)

struct PauseOverlay {
    Widget base;
};

const lv_obj_class_t pause_overlay_lvgl_class;

// LVGL-specific code

static void pause_overlay_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, BG_OPACITY, LV_PART_MAIN);

    lv_obj_t* layout = lv_obj_create(obj);
    lv_obj_set_size(layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_column(layout, 4, LV_PART_MAIN);
    lv_obj_set_flex_flow(layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(layout, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_align(layout, LV_ALIGN_CENTER, 0, -1);

    lv_obj_t* image = lv_image_create(layout);
    lv_image_set_src(image, &I_pause_5x5);

    lv_obj_t* label = lv_label_create(layout);
    lv_label_set_text(label, "PAUSED");
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
}

// Public API

PauseOverlay* pause_overlay_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    PauseOverlay* instance = (PauseOverlay*)obj;
    return instance;
}

void pause_overlay_free(PauseOverlay* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* pause_overlay_get_base(PauseOverlay* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void pause_overlay_show(PauseOverlay* instance, bool show) {
    furi_check(instance);

    if(show) {
        lv_obj_remove_flag((lv_obj_t*)instance, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag((lv_obj_t*)instance, LV_OBJ_FLAG_HIDDEN);
    }
}

// LVGL class descriptor

const lv_obj_class_t pause_overlay_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = pause_overlay_lvgl_constructor,
    .name = "widget-pause-overlay",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(PauseOverlay),
};
