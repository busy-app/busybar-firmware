#include "countdown.h"

#include <gui/widget_i.h>
#include <gui/gui_i.h>
#include <furi_hal_rtc.h>

#define MY_CLASS         (&countdown_lvgl_class)
#define UPDATE_PERIOD_MS (100)

struct Countdown {
    Widget base;
    lv_obj_t* label;
    lv_timer_t* timer;

    time_t timestamp;
    uint8_t last_update_second;
    CountdownDirection direction;
    CountdownShowHour hours;
};

const lv_obj_class_t countdown_lvgl_class;

// ================
// Internal methods
// ================

static void countdown_update(Countdown* countdown) {
    furi_assert(countdown);

    DateTime datetime;
    furi_hal_rtc_get_datetime(&datetime);
    if(datetime.second == countdown->last_update_second) return;
    countdown->last_update_second = datetime.second;

    time_t now = datetime_datetime_to_timestamp(&datetime);

    time_t delta = countdown->timestamp - now;
    if(countdown->direction == CountdownDirectionTimeSince) delta = -delta;
    if(delta < 0) delta = 0;

    uint8_t seconds = delta % 60;
    uint8_t minutes = (delta / 60) % 60;
    uint8_t hours = (delta / 3600) % 60;

    bool show_hour = (hours > 0) || (countdown->hours == CountdownShowHourAlways);
    if(show_hour) {
        lv_label_set_text_fmt(countdown->label, "%02hhu:%02hhu:%02hhu", hours, minutes, seconds);
    } else {
        lv_label_set_text_fmt(countdown->label, "%02hhu:%02hhu", minutes, seconds);
    }
}

static void countdown_timer_callback(lv_timer_t* timer) {
    Countdown* instance = lv_timer_get_user_data(timer);
    furi_assert(instance);
    countdown_update(instance);
}

// ==========
// Public API
// ==========

Countdown* countdown_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    Countdown* instance = (Countdown*)obj;
    return instance;
}

void countdown_free(Countdown* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* countdown_get_base(Countdown* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void countdown_set_text_color(Countdown* instance, Color color) {
    furi_check(instance);
    lv_obj_set_style_text_color((lv_obj_t*)instance->label, TO_LV_COLOR(color), LV_PART_MAIN);
    lv_obj_set_style_text_opa(instance->label, color.a, LV_PART_MAIN);
}

void countdown_begin(
    Countdown* instance,
    time_t time,
    CountdownDirection direction,
    CountdownShowHour hours) {
    furi_check(instance);
    furi_check(direction < CountdownDirectionMAX);
    furi_check(hours < CountdownShowHourMAX);

    instance->timestamp = time;
    instance->direction = direction;
    instance->hours = hours;
    countdown_update(instance);

    lv_timer_resume(instance->timer);
}

// ==========
// LVGL class
// ==========

static void countdown_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    Countdown* instance = (Countdown*)obj;
    instance->label = lv_label_create(obj);

    instance->timer = lv_timer_create(countdown_timer_callback, UPDATE_PERIOD_MS, instance);
    lv_timer_set_repeat_count(instance->timer, -1);
    lv_timer_pause(instance->timer);
}

static void countdown_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    Countdown* instance = (Countdown*)obj;
    lv_timer_delete(instance->timer);
}

const lv_obj_class_t countdown_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = countdown_lvgl_constructor,
    .destructor_cb = countdown_lvgl_destructor,
    .name = "widget-countdown",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(Countdown),
    .theme_inheritable = true,
};
