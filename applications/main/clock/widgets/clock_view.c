#include "clock_view.h"
#include "../clock_i.h"

#include <gui/widget_i.h>

#define MY_CLASS (&clock_view_lvgl_class)

struct ClockView {
    Widget base;

    lv_obj_t* primary_container;

    lv_obj_t* icon;
    lv_obj_t* icon_label_date;

    lv_obj_t* text_container;
    lv_obj_t* text_label_time;
    lv_obj_t* text_label_date;

    bool show_seconds;
    bool show_date;
};

const lv_obj_class_t clock_view_lvgl_class;

static const char* const month_short_names[] = {
    [0] = "Jan",
    [1] = "Feb",
    [2] = "Mar",
    [3] = "Apr",
    [4] = "May",
    [5] = "Jun",
    [6] = "Jul",
    [7] = "Aug",
    [8] = "Sep",
    [9] = "Oct",
    [10] = "Nov",
    [11] = "Dec",
};

static const char* const weekday_short_names[] = {
    [0] = "Mon",
    [1] = "Tue",
    [2] = "Wed",
    [3] = "Thu",
    [4] = "Fri",
    [5] = "Sat",
    [6] = "Sun",
};

/* LVGL-specific code */

static void clock_view_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    ClockView* instance = (ClockView*)obj;

    instance->primary_container = lv_obj_create(obj);
    lv_obj_set_flex_flow(instance->primary_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        instance->primary_container,
        LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(instance->primary_container, 3, LV_PART_MAIN);
    lv_obj_set_size(instance->primary_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    instance->icon = lv_img_create(instance->primary_container);
    lv_image_set_src(instance->icon, THIS_IMG_PATH("calendar_13x14.bin"));

    instance->icon_label_date = lv_label_create(instance->icon);
    lv_obj_set_style_text_color(
        instance->icon_label_date, lv_color_make(0x32, 0x32, 0x32), LV_PART_MAIN);
    lv_obj_set_style_translate_y(instance->icon_label_date, -1, LV_PART_MAIN);
    lv_obj_set_style_pad_left(instance->icon_label_date, 1, LV_PART_MAIN);
    lv_obj_set_align(instance->icon_label_date, LV_ALIGN_BOTTOM_MID);

    instance->text_container = lv_obj_create(instance->primary_container);
    lv_obj_set_flex_flow(instance->text_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(instance->text_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    instance->text_label_time = lv_label_create(instance->text_container);
    lv_obj_set_style_text_font(instance->text_label_time, &lv_busy_bold_7px, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->text_label_time, lv_color_white(), LV_PART_MAIN);

    instance->text_label_date = lv_label_create(instance->text_container);
    lv_obj_set_style_translate_y(instance->text_label_date, -2, LV_PART_MAIN);
    lv_obj_set_style_text_font(instance->text_label_date, &lv_font_bf_4x5, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->text_label_date, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_opa(instance->text_label_date, LV_OPA_50, LV_PART_MAIN);
}

/* internals */

const char* clock_app_get_month_short_name(uint8_t month) {
    furi_assert(month >= 1 && month <= COUNT_OF(month_short_names));

    return month_short_names[month - 1];
}

const char* clock_app_get_weekday_short_name(uint8_t dayofweek) {
    furi_assert(dayofweek >= 1 && dayofweek <= COUNT_OF(weekday_short_names));

    return weekday_short_names[dayofweek - 1];
}

/* public API */

ClockView* clock_view_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    ClockView* instance = (ClockView*)obj;

    instance->show_seconds = true;
    instance->show_date = true;

    return instance;
}

void clock_view_free(ClockView* instance) {
    furi_check(instance);

    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* clock_view_get_base(ClockView* instance) {
    furi_check(instance);

    return (Widget*)instance;
}

void clock_view_set_show_seconds(ClockView* instance, bool show_seconds) {
    furi_check(instance);

    instance->show_seconds = show_seconds;
}

void clock_view_set_show_date(ClockView* instance, bool show_date) {
    furi_check(instance);

    lv_obj_update_flag(instance->icon, LV_OBJ_FLAG_HIDDEN, !show_date);
    lv_obj_update_flag(instance->text_label_date, LV_OBJ_FLAG_HIDDEN, !show_date);

    instance->show_seconds = show_date;
}

void clock_view_set_date_time(ClockView* instance, const DateTime* date_time) {
    furi_check(instance);
    furi_check(date_time);

    if(instance->show_date) {
        lv_label_set_text_fmt(instance->icon_label_date, "%" PRIu8, date_time->dayofmonth);
        lv_label_set_text_fmt(
            instance->text_label_date,
            "%s, %s",
            clock_app_get_weekday_short_name(date_time->dayofweek),
            clock_app_get_month_short_name(date_time->month));
    }

    if(instance->show_seconds) {
        lv_label_set_text_fmt(
            instance->text_label_time,
            "%02" PRIu8 ":%02" PRIu8 ":%02" PRIu8,
            date_time->hour,
            date_time->minute,
            date_time->second);
    } else {
        lv_label_set_text_fmt(
            instance->text_label_time,
            "%02" PRIu8 ":%02" PRIu8,
            date_time->hour,
            date_time->minute);
    }
}

/* LVGL class descriptor */

const lv_obj_class_t clock_view_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = clock_view_lvgl_constructor,
    .name = "widget-clock-view",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(ClockView),
};
