#include "timer_label.h"

#include <gui/widget_i.h>

#include "../time_macros.h"

#define MY_CLASS (&timer_label_lvgl_class)

#define DOUBLE_HOUR_STRING_LEN (8)
#define SINGLE_HOUR_STRING_LEN (7)

struct TimerLabel {
    Widget base;
    lv_obj_t* top_label;
    lv_obj_t* bottom_label;
};

const lv_obj_class_t timer_label_lvgl_class;

/* Subscript charaters for slightly smaller numerals */
static const char* subscript_table[] = {
    "₀", /* U+2080 */
    "₁", /* U+2081 */
    "₂", /* U+2082 */
    "₃", /* U+2083 */
    "₄", /* U+2084 */
    "₅", /* U+2085 */
    "₆", /* U+2086 */
    "₇", /* U+2087 */
    "₈", /* U+2088 */
    "₉", /* U+2089 */
};

/* Superscript characters for tiny seconds */
static const char* superscript_table[] = {
    "⁰", /* U+2070 */
    "¹", /* U+00B9 */
    "²", /* U+00B2 */
    "³", /* U+00B3 */
    "⁴", /* U+2074 */
    "⁵", /* U+2075 */
    "⁶", /* U+2076 */
    "⁷", /* U+2077 */
    "⁸", /* U+2078 */
    "⁹", /* U+2079 */
};

static void timer_label_replace_numbers_from_table(
    FuriString* str,
    const char* table[10],
    uint32_t start,
    uint32_t count) {
    for(uint32_t i = start, replace_count = 0;
        i < furi_string_size(str) && replace_count < count;) {
        const char c = furi_string_get_char(str, i);

        if(c >= '0' && c <= '9') {
            const char needle[2] = {c, '\0'};
            const char* replace = table[c - '0'];

            furi_string_replace(str, needle, replace, i);

            i += strlen(replace);
            ++replace_count;

        } else {
            ++i;
        }
    }
}

// LVGL-specific code

static void timer_label_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);

    TimerLabel* instance = (TimerLabel*)obj;

    instance->top_label = lv_label_create(obj);
    lv_obj_set_style_text_font(instance->top_label, lv_theme_get_font_large(obj), LV_PART_MAIN);

    instance->bottom_label = lv_label_create(obj);
    lv_obj_set_style_text_font(instance->bottom_label, lv_theme_get_font_small(obj), LV_PART_MAIN);
    lv_label_set_text(instance->bottom_label, "LEFT");
}

// Public API

TimerLabel* timer_label_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    TimerLabel* instance = (TimerLabel*)obj;
    return instance;
}

void timer_label_free(TimerLabel* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* timer_label_get_base(TimerLabel* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void timer_label_set_time_left(TimerLabel* instance, uint32_t time_left_s) {
    furi_check(instance);

    FuriString* tmp = furi_string_alloc();

    const uint32_t h = S_TO_H(time_left_s);
    const uint32_t m = S_TO_M(time_left_s - H_TO_S(h));
    const uint32_t s = time_left_s - H_TO_S(h) - M_TO_S(m);

    if(h) {
        furi_string_printf(tmp, "%lu:%02lu %02lu", h, m, s);
    } else {
        furi_string_printf(tmp, "%02lu:%02lu", m, s);
    }

    if(furi_string_size(tmp) == DOUBLE_HOUR_STRING_LEN) {
        timer_label_replace_numbers_from_table(tmp, subscript_table, 0, 4);
    }

    if(furi_string_size(tmp) >= SINGLE_HOUR_STRING_LEN) {
        timer_label_replace_numbers_from_table(
            tmp, superscript_table, furi_string_size(tmp) - 2, 2);
    }

    lv_label_set_text(instance->top_label, furi_string_get_cstr(tmp));
    furi_string_free(tmp);
}

// LVGL class descriptor

const lv_obj_class_t timer_label_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = timer_label_lvgl_constructor,
    .name = "widget-timer-label",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(TimerLabel),
};
