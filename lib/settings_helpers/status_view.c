#include "status_view.h"
#include <gui/modules/anim_player.h>
#include <gui/widget_i.h>

#define MY_CLASS                 (&status_view_lvgl_class)
#define MY_ICON_CLASS            (&status_view_icon_lvgl_class)
#define MY_HEADER_CLASS          (&status_view_header_lvgl_class)
#define MY_ADDITIONAL_TEXT_CLASS (&status_view_additional_text_lvgl_class)

struct StatusView {
    Widget base;
    lv_obj_t* icon_static;
    AnimPlayer* icon_animated;
    lv_obj_t* icon_cont;
    lv_obj_t* header;
    lv_obj_t* additional_text;
};

const lv_obj_class_t status_view_lvgl_class;
const lv_obj_class_t status_view_icon_lvgl_class;
const lv_obj_class_t status_view_header_lvgl_class;
const lv_obj_class_t status_view_additional_text_lvgl_class;

// ==========
// Public API
// ==========

StatusView* status_view_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    StatusView* instance = (StatusView*)obj;
    return instance;
}

void status_view_free(StatusView* instance) {
    furi_check(instance);
    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* status_view_get_base(StatusView* instance) {
    furi_check(instance);
    return &instance->base;
}

void status_view_set_icon(StatusView* instance, const char* path) {
    furi_check(instance);
    furi_check(path);

    FuriString* path_temp = furi_string_alloc_set(path);
    bool is_animated = furi_string_end_with(path_temp, ".anim");
    furi_string_free(path_temp);

    if(is_animated) {
        lv_obj_add_flag((lv_obj_t*)instance->icon_static, LV_OBJ_FLAG_HIDDEN);
        anim_player_set_source(instance->icon_animated, path);
        lv_obj_remove_flag((lv_obj_t*)instance->icon_animated, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag((lv_obj_t*)instance->icon_animated, LV_OBJ_FLAG_HIDDEN);
        lv_image_set_src(instance->icon_static, NULL);
        lv_image_set_src(instance->icon_static, path);
        lv_obj_remove_flag((lv_obj_t*)instance->icon_static, LV_OBJ_FLAG_HIDDEN);
    }
}

void status_view_set_header(StatusView* instance, const char* header) {
    furi_check(instance);
    furi_check(header);
    lv_label_set_text(instance->header, header);
}

void status_view_set_additional_text(StatusView* instance, const char* text) {
    furi_check(instance);
    furi_check(text);
    lv_label_set_text(instance->additional_text, text);
}

// =================
// Class descriptors
// =================

static void status_view_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    StatusView* instance = (StatusView*)obj;

    instance->icon_cont = lv_obj_class_create_obj(MY_ICON_CLASS, obj);
    lv_obj_class_init_obj(instance->icon_cont);

    instance->icon_static = lv_image_create(instance->icon_cont);
    instance->icon_animated = anim_player_alloc((Widget*)instance->icon_cont);
    lv_obj_add_flag((lv_obj_t*)instance->icon_animated, LV_OBJ_FLAG_HIDDEN);

    instance->header = lv_obj_class_create_obj(MY_HEADER_CLASS, obj);
    lv_obj_class_init_obj(instance->header);

    instance->additional_text = lv_obj_class_create_obj(MY_ADDITIONAL_TEXT_CLASS, obj);
    lv_obj_class_init_obj(instance->additional_text);
}

const lv_obj_class_t status_view_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = status_view_lvgl_constructor,
    .name = "widget-status-view",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(StatusView),
};

const lv_obj_class_t status_view_icon_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .name = "status-view-icon",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};

const lv_obj_class_t status_view_header_lvgl_class = {
    .base_class = &lv_label_class,
    .name = "status-view-header",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};

const lv_obj_class_t status_view_additional_text_lvgl_class = {
    .base_class = &lv_label_class,
    .name = "status-view-additional-text",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};
