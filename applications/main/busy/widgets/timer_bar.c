#include "timer_bar.h"

#include <gui/widget_i.h>
#include <gui/modules/anim_play.h>

#define TROUGH_WIDTH  (70)
#define TROUGH_HEIGHT (1)

#define MY_CLASS (&timer_bar_lvgl_class)

struct TimerBar {
    Widget base;
    AnimPlay* bar;
    int32_t prev_offset;
};

const lv_obj_class_t timer_bar_lvgl_class;

// LVGL-specific code

static void timer_bar_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN);

    TimerBar* instance = (TimerBar*)obj;
    instance->bar = anim_play_alloc((Widget*)obj);

    lv_obj_align_to((lv_obj_t*)instance->bar, obj, LV_ALIGN_OUT_LEFT_MID, 1, 0);
}

// Public API

TimerBar* timer_bar_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    TimerBar* instance = (TimerBar*)obj;
    return instance;
}

void timer_bar_free(TimerBar* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* timer_bar_get_base(TimerBar* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void timer_bar_set_preset(TimerBar* instance, const TimerBarPreset* preset) {
    furi_check(instance);

    anim_play_set_source(instance->bar, preset->file_path);
    anim_play_pause(instance->bar);

    lv_obj_set_style_bg_color(
        TO_LV_OBJ(instance), TO_LV_COLOR(preset->trough_color), LV_PART_MAIN);
}

void timer_bar_set_value(TimerBar* instance, float value) {
    furi_check(instance);

    const int32_t width = lv_obj_get_width((lv_obj_t*)instance);
    const int32_t offset = width - roundf(width * value);

    if(offset != instance->prev_offset) {
        instance->prev_offset = offset;
        lv_obj_set_x((lv_obj_t*)instance->bar, -offset);
    }

    AnimFile* file = anim_play_get_file(instance->bar);
    if(file) {
        bool success = anim_file_set_section_indexed(
            file, AnimFilePlayFlagNone, ANIM_FILE_WHOLE_SECTION_INDEX);
        furi_assert(success);
        anim_play_start(instance->bar);
    }
}

// LVGL class descriptor

const lv_obj_class_t timer_bar_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = timer_bar_lvgl_constructor,
    .name = "widget-timer-bar",
    .width_def = TROUGH_WIDTH,
    .height_def = TROUGH_HEIGHT,
    .instance_size = sizeof(TimerBar),
};
