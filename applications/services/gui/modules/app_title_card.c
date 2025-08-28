#include "app_title_card.h"
#include "../widget_i.h"

#define MY_CLASS       (&app_title_card_lvgl_class)
#define MY_LABEL_CLASS (&app_title_card_label_lvgl_class)

struct AppTitleCard {
    Widget base;
    lv_obj_t* image;
    lv_obj_t* label;
};

const lv_obj_class_t app_title_card_lvgl_class;
const lv_obj_class_t app_title_card_label_lvgl_class;

/* LVGL-specific code */

static void app_title_card_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    AppTitleCard* instance = (AppTitleCard*)obj;

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    instance->image = lv_image_create(obj);

    instance->label = lv_obj_class_create_obj(MY_LABEL_CLASS, obj);
    lv_obj_class_init_obj(instance->label);
    lv_obj_set_style_text_color(instance->label, lv_color_white(), LV_PART_MAIN);
}

/* Public API */

AppTitleCard* app_title_card_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    AppTitleCard* instance = (AppTitleCard*)obj;

    return instance;
}

void app_title_card_free(AppTitleCard* instance) {
    furi_check(instance);
    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* app_title_card_get_base(AppTitleCard* instance) {
    furi_check(instance);
    return &instance->base;
}

void app_title_card_set_image(AppTitleCard* instance, const char* file_path) {
    furi_check(instance);
    furi_check(file_path);

    lv_image_set_src(instance->image, file_path);
}

void app_title_card_set_text(AppTitleCard* instance, const char* text) {
    furi_check(instance);
    furi_check(text);

    lv_label_set_text(instance->label, text);
}

/* LVGL class descriptor */

const lv_obj_class_t app_title_card_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = app_title_card_lvgl_constructor,
    .name = "widget-app-title-card",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(AppTitleCard),
};

const lv_obj_class_t app_title_card_label_lvgl_class = {
    .base_class = &lv_label_class,
    .name = "app-title-card-label",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};
