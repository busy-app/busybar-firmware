#include "label.h"

#include <gui/widget_i.h>
#include <gui/gui_i.h>

#define MY_CLASS (&label_lvgl_class)

struct Label {
    Widget base;
    lv_obj_t* label;
    FuriString* text;
    GuiFont font;
};

const lv_obj_class_t label_lvgl_class;

// LVGL-specific code

static void label_event_callback(const lv_obj_class_t* class_p, lv_event_t* event) {
    UNUSED(class_p);

    lv_result_t res = LV_RESULT_OK;
    res = lv_obj_event_base(MY_CLASS, event);
    if(res != LV_RESULT_OK) return;

    lv_event_code_t code = lv_event_get_code(event);
    Label* instance = (Label*)lv_event_get_target_obj(event);

    if(code == LV_EVENT_SIZE_CHANGED) {
        int32_t lv_base_width = lv_obj_get_style_width((lv_obj_t*)&instance->base, LV_PART_MAIN);
        int32_t lv_base_height = lv_obj_get_style_height((lv_obj_t*)&instance->base, LV_PART_MAIN);

        lv_obj_set_width(
            instance->label,
            (lv_base_width == LV_SIZE_CONTENT) ? MY_CLASS->width_def : LV_PCT(100));
        lv_obj_set_height(
            instance->label,
            (lv_base_height == LV_SIZE_CONTENT) ? MY_CLASS->height_def : LV_PCT(100));
    }
}

static void label_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

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

void label_set_text_color(Label* instance, Color color) {
    furi_check(instance);
    lv_obj_set_style_text_color((lv_obj_t*)instance->label, TO_LV_COLOR(color), LV_PART_MAIN);
    lv_obj_set_style_text_opa(instance->label, color.a, LV_PART_MAIN);
}

void label_set_text_font_size(Label* instance, LabelFontSize size) {
    furi_check(instance);

    switch(size) {
    case LabelFontSizeSmall:
        lv_obj_set_style_text_font(
            (lv_obj_t*)instance->label,
            lv_theme_get_font_small((lv_obj_t*)instance->label),
            LV_PART_MAIN);
        break;
    case LabelFontSizeNormal:
        lv_obj_set_style_text_font(
            (lv_obj_t*)instance->label,
            lv_theme_get_font_normal((lv_obj_t*)instance->label),
            LV_PART_MAIN);
        break;
    case LabelFontSizeLarge:
        lv_obj_set_style_text_font(
            (lv_obj_t*)instance->label,
            lv_theme_get_font_large((lv_obj_t*)instance->label),
            LV_PART_MAIN);
        break;
    default:
        furi_check(false);
        return;
    }
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

void label_set_long_content_mode(Label* instance, LabelLongContentMode mode, uint32_t duration) {
    furi_check(instance);
    furi_check(mode < LabelLongContentModeCount);

    lv_obj_set_style_anim_time(instance->label, duration, LV_PART_MAIN);
    lv_label_set_long_mode(instance->label, (lv_label_long_mode_t)mode);
}

uint32_t label_calculate_scroll_duration(const Label* instance, uint32_t rate_ppm) {
    furi_check(instance);
    furi_check(rate_ppm > 0);

    lv_obj_t* label = TO_LV_OBJ(instance);
    lv_obj_update_layout(label);

    const char* text = furi_string_get_cstr(instance->text);
    const lv_font_t* font = gui_font_to_lvgl(instance->font);
    int32_t letter_space = lv_obj_get_style_text_letter_space(label, LV_PART_MAIN);
    int32_t text_width = lv_text_get_width(text, strlen(text), font, letter_space);

    int32_t space_width = lv_font_get_glyph_width(font, ' ', ' ');
    int32_t gap_width = space_width * LV_LABEL_WAIT_CHAR_COUNT;
    int32_t total_width = text_width + gap_width;

    size_t duration_ms = (total_width * 60 * 1000) / rate_ppm;
    return duration_ms;
}

void label_set_font(Label* instance, GuiFont font) {
    furi_check(instance);
    lv_obj_set_style_text_font(instance->label, gui_font_to_lvgl(font), LV_PART_MAIN);
    instance->font = font;
}

// LVGL class descriptor

const lv_obj_class_t label_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = label_lvgl_constructor,
    .destructor_cb = label_lvgl_destructor,
    .event_cb = label_event_callback,
    .name = "widget-label",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(Label),
    .theme_inheritable = true,
};
