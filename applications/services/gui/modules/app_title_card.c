#include "app_title_card.h"
#include "../widget_i.h"
#include "anim_image.h"

#define MY_CLASS       (&app_title_card_lvgl_class)
#define MY_LABEL_CLASS (&app_title_card_label_lvgl_class)

struct AppTitleCard {
    Widget base;

    union {
        lv_obj_t* image;
        AnimImage* anim_image;
    };

    lv_obj_t* label;

    bool uses_anim_image;
};

const lv_obj_class_t app_title_card_lvgl_class;
const lv_obj_class_t app_title_card_label_lvgl_class;

/* LVGL-specific code */

static void app_title_card_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    AppTitleCard* instance = (AppTitleCard*)obj;

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    if(instance->uses_anim_image) {
        instance->anim_image = anim_image_alloc(&instance->base);
    } else {
        instance->image = lv_image_create(obj);
    }

    instance->label = lv_obj_class_create_obj(MY_LABEL_CLASS, obj);
    lv_obj_class_init_obj(instance->label);
    lv_obj_set_style_text_color(instance->label, lv_color_white(), LV_PART_MAIN);
}

/* Implementation */

static void app_title_card_text_anim_exec_callback(void* var, int32_t value) {
    lv_obj_set_style_translate_x(var, value, LV_PART_MAIN);
}

/* Public API */

AppTitleCard* app_title_card_alloc(Widget* parent, bool use_anim_image) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));

    AppTitleCard* instance = (AppTitleCard*)obj;
    instance->uses_anim_image = use_anim_image;

    lv_obj_class_init_obj(obj);

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

    if(instance->uses_anim_image) {
        anim_image_set_source(instance->anim_image, file_path);
        anim_image_stop(instance->anim_image);
    } else {
        lv_image_set_src(instance->image, file_path);
    }
}

void app_title_card_set_text(AppTitleCard* instance, const char* text) {
    furi_check(instance);
    furi_check(text);

    lv_label_set_text(instance->label, text);
}

void app_title_card_run_image_anim(AppTitleCard* instance, uint32_t start, uint32_t stop) {
    furi_check(instance);
    furi_check(instance->uses_anim_image);

    anim_image_set_range(instance->anim_image, start, stop, false, false);
}

void app_title_card_run_text_anim(
    AppTitleCard* instance,
    int32_t start,
    int32_t stop,
    uint32_t duration) {
    furi_check(instance);

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_user_data(&anim, instance);
    lv_anim_set_var(&anim, instance->label);
    lv_anim_set_values(&anim, start, stop);
    lv_anim_set_duration(&anim, duration);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_exec_cb(&anim, app_title_card_text_anim_exec_callback);
    lv_anim_start(&anim);
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
