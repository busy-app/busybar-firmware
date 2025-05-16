#include "label.h"

#include <gui/widget_i.h>

#define MY_CLASS (&label_lvgl_class)

struct Label {
    Widget base;
    lv_obj_t* label;
    FuriString* text;
};

const lv_obj_class_t label_lvgl_class;

// LVGL-specific code

static void label_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    Label* instance = (Label*)obj;
    instance->label = lv_label_create(obj);
    instance->text = furi_string_alloc();
}

static void label_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    Label* instance = (Label*)obj;
    furi_string_free(instance->text);
}

// Public API

Label* label_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    Label* instance = (Label*)obj;
    return instance;
}

void label_free(Label* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* label_get_base(Label* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void label_set_text(Label* instance, const char* text) {
    furi_check(instance);
    furi_check(text);

    furi_string_set(instance->text, text);
    lv_label_set_text_static(instance->label, furi_string_get_cstr(instance->text));
}

void label_set_text_fmt(Label* instance, const char* fmt, ...) {
    furi_check(instance);
    furi_check(fmt);

    va_list args;
    va_start(args, fmt);
    furi_string_vprintf(instance->text, fmt, args);
    va_end(args);

    lv_label_set_text_static(instance->label, furi_string_get_cstr(instance->text));
}

void label_set_line_spacing(Label* instance, int32_t spacing) {
    furi_check(instance);
    lv_obj_set_style_text_line_space((lv_obj_t*)instance, spacing, LV_PART_MAIN);
}

void label_set_text_align(Label* instance, TextAlign align) {
    furi_check(instance);
    furi_check(align < TextAlignMax);

    lv_obj_set_style_text_align((lv_obj_t*)instance, (lv_text_align_t)align, LV_PART_MAIN);
}

void label_set_max_width(Label* instance, int32_t max_width) {
    furi_check(instance);
    lv_obj_set_style_max_width(instance->label, max_width, LV_PART_MAIN);
}

void label_set_max_height(Label* instance, int32_t max_height) {
    furi_check(instance);
    lv_obj_set_style_max_height(instance->label, max_height, LV_PART_MAIN);
}

void label_set_long_content_mode(Label* instance, LabelLongContentMode mode, uint32_t duration) {
    furi_check(instance);
    furi_check(mode < LabelLongContentModeCount);

    const lv_label_long_mode_t lv_modes[] = {
        [LabelLongContentModeWrap] = LV_LABEL_LONG_MODE_WRAP,
        [LabelLongContentModeDots] = LV_LABEL_LONG_MODE_DOTS,
        [LabelLongContentModeScroll] = LV_LABEL_LONG_MODE_SCROLL,
        [LabelLongContentModeScrollCircular] = LV_LABEL_LONG_MODE_SCROLL_CIRCULAR,
        [LabelLongContentModeClip] = LV_LABEL_LONG_MODE_CLIP,
    };

    lv_label_set_long_mode(instance->label, lv_modes[mode]);
    lv_obj_set_style_anim_time(instance->label, duration, LV_PART_MAIN);
}

// LVGL class descriptor

const lv_obj_class_t label_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = label_lvgl_constructor,
    .destructor_cb = label_lvgl_destructor,
    .name = "widget-label",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(Label),
    .theme_inheritable = true,
};
