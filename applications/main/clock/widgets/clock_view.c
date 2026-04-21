#include "clock_view.h"
#include "../clock_i.h"

#include <gui/widget_i.h>

#define MY_CLASS (&clock_view_lvgl_class)

#define COLON_BLINK_TIMER_PERIOD_MS 500

#define ICON_LABEL_DATE_TEXT_COLOR_HEX 0x323232

struct ClockView {
    Widget base;

    lv_obj_t* icon;
    lv_obj_t* icon_label_date;

    lv_obj_t* time_spangroup;
    lv_span_t* time_hour_span;
    lv_span_t* time_minute_colon_span;
    lv_span_t* time_minute_span;
    lv_span_t* time_second_colon_span;
    lv_span_t* time_second_span;

    lv_obj_t* time_meridian_label;

    lv_obj_t* date_label;

    lv_timer_t* colon_blink_timer;

    FontRegistry* font_registry;
    const lv_font_t* font_busy_bold_7;
    const lv_font_t* font_busy_regular_5;
    const lv_font_t* font_busy_superscript_7;

    FuriString* string_builder;

    TimeSettingTimeFormat time_format;
    bool show_date;
    bool show_seconds;
    bool blink_colons;
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

static void clock_view_colon_blink_timer_callback(lv_timer_t* timer) {
    ClockView* instance = timer->user_data;

    lv_style_set_text_opa(lv_span_get_style(instance->time_minute_colon_span), LV_OPA_40);
    lv_style_set_text_opa(lv_span_get_style(instance->time_second_colon_span), LV_OPA_40);

    lv_spangroup_refresh(instance->time_spangroup);
}

static void clock_view_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    ClockView* instance = (ClockView*)obj;

    instance->font_registry = furi_record_open(RECORD_FONT_REGISTRY);
    instance->font_busy_bold_7 =
        font_registry_load_font(instance->font_registry, FONT_BUSY_BOLD_7);
    instance->font_busy_regular_5 =
        font_registry_load_font(instance->font_registry, FONT_BUSY_REGULAR_5);
    instance->font_busy_superscript_7 =
        font_registry_load_font(instance->font_registry, FONT_BUSY_SUPERSCRIPT_7);

    lv_obj_t* primary_container = lv_obj_create(TO_LV_OBJ(instance));
    lv_obj_set_flex_flow(primary_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        primary_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(primary_container, 2, LV_PART_MAIN);
    lv_obj_set_size(primary_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    instance->icon = lv_img_create(primary_container);
    lv_image_set_src(instance->icon, THIS_IMG_PATH("calendar_13x14.image"));

    instance->icon_label_date = lv_label_create(instance->icon);
    lv_obj_set_style_text_font(
        instance->icon_label_date, instance->font_busy_superscript_7, LV_PART_MAIN);
    lv_obj_set_style_text_color(
        instance->icon_label_date, lv_color_hex(ICON_LABEL_DATE_TEXT_COLOR_HEX), LV_PART_MAIN);
    lv_obj_set_style_pad_left(instance->icon_label_date, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(instance->icon_label_date, -2, LV_PART_MAIN);
    lv_obj_set_align(instance->icon_label_date, LV_ALIGN_BOTTOM_MID);

    lv_obj_t* text_container = lv_obj_create(primary_container);
    lv_obj_set_flex_flow(text_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(text_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    lv_obj_t* time_container = lv_obj_create(text_container);
    lv_obj_set_flex_flow(time_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        time_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(time_container, 1, LV_PART_MAIN);
    lv_obj_set_size(time_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    instance->time_spangroup = lv_spangroup_create(time_container);
    lv_obj_set_style_text_font(instance->time_spangroup, instance->font_busy_bold_7, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->time_spangroup, lv_color_white(), LV_PART_MAIN);

    instance->time_hour_span = lv_spangroup_add_span(instance->time_spangroup);

    instance->time_minute_colon_span = lv_spangroup_add_span(instance->time_spangroup);
    lv_span_set_text_static(instance->time_minute_colon_span, ":");

    instance->time_minute_span = lv_spangroup_add_span(instance->time_spangroup);

    instance->time_second_colon_span = lv_spangroup_add_span(instance->time_spangroup);
    lv_span_set_text_static(instance->time_second_colon_span, ":");

    instance->time_second_span = lv_spangroup_add_span(instance->time_spangroup);

    instance->time_meridian_label = lv_label_create(time_container);
    lv_obj_set_style_text_font(
        instance->time_meridian_label, instance->font_busy_regular_5, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->time_meridian_label, lv_color_white(), LV_PART_MAIN);

    instance->date_label = lv_label_create(text_container);
    lv_obj_set_style_text_font(instance->date_label, instance->font_busy_regular_5, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->date_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_opa(instance->date_label, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_margin_top(instance->date_label, -2, LV_PART_MAIN);

    instance->colon_blink_timer = lv_timer_create(
        clock_view_colon_blink_timer_callback, COLON_BLINK_TIMER_PERIOD_MS, instance);
    lv_timer_set_auto_delete(instance->colon_blink_timer, false);
    lv_timer_pause(instance->colon_blink_timer);
}

static void clock_view_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    ClockView* instance = (ClockView*)obj;

    lv_timer_delete(instance->colon_blink_timer);

    font_registry_unload_font(instance->font_registry, instance->font_busy_superscript_7);
    font_registry_unload_font(instance->font_registry, instance->font_busy_regular_5);
    font_registry_unload_font(instance->font_registry, instance->font_busy_bold_7);
    furi_record_close(RECORD_FONT_REGISTRY);
}

/* internals */

static const char* clock_view_get_month_short_name(uint8_t month) {
    furi_assert(month >= 1 && month <= COUNT_OF(month_short_names));

    return month_short_names[month - 1];
}

static const char* clock_view_get_weekday_short_name(uint8_t dayofweek) {
    furi_assert(dayofweek >= 1 && dayofweek <= COUNT_OF(weekday_short_names));

    return weekday_short_names[dayofweek - 1];
}

/* public API */

ClockView* clock_view_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    ClockView* instance = (ClockView*)obj;

    instance->string_builder = furi_string_alloc();

    instance->time_format = TimeSettingTimeFormat12h;
    instance->show_date = true;
    instance->show_seconds = true;
    instance->blink_colons = true;

    return instance;
}

void clock_view_free(ClockView* instance) {
    furi_check(instance);

    furi_string_free(instance->string_builder);

    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* clock_view_get_base(ClockView* instance) {
    furi_check(instance);

    return (Widget*)instance;
}

void clock_view_set_time_format(ClockView* instance, TimeSettingTimeFormat time_format) {
    furi_check(instance);

    bool hide_meridian;
    switch(time_format) {
    case TimeSettingTimeFormat24h:
        hide_meridian = true;
        break;

    case TimeSettingTimeFormat12h:
        hide_meridian = false;
        break;

    default:
        furi_crash();
    }

    lv_obj_update_flag(instance->time_meridian_label, LV_OBJ_FLAG_HIDDEN, hide_meridian);

    instance->time_format = time_format;
}

void clock_view_set_show_date(ClockView* instance, bool show_date) {
    furi_check(instance);

    lv_obj_update_flag(instance->icon, LV_OBJ_FLAG_HIDDEN, !show_date);
    lv_obj_update_flag(instance->date_label, LV_OBJ_FLAG_HIDDEN, !show_date);

    instance->show_date = show_date;
}

void clock_view_set_show_seconds(ClockView* instance, bool show_seconds) {
    furi_check(instance);

    if(show_seconds) {
        lv_span_set_text_static(instance->time_second_colon_span, ":");
    } else {
        lv_span_set_text_static(instance->time_second_colon_span, "");
        lv_span_set_text_static(instance->time_second_span, "");
    }

    instance->show_seconds = show_seconds;
}

void clock_view_set_blink_colons(ClockView* instance, bool blink_colons) {
    furi_check(instance);

    if(!blink_colons) {
        lv_style_set_text_opa(lv_span_get_style(instance->time_second_colon_span), LV_OPA_COVER);
        lv_style_set_text_opa(lv_span_get_style(instance->time_minute_colon_span), LV_OPA_COVER);

        lv_timer_pause(instance->colon_blink_timer);
    }

    instance->blink_colons = blink_colons;
}

void clock_view_set_date_time(ClockView* instance, const DateTime* date_time) {
    furi_check(instance);
    furi_check(date_time);

    if(instance->show_date) {
        lv_label_set_text_fmt(instance->icon_label_date, "%" PRIu8, date_time->dayofmonth);
        lv_label_set_text_fmt(
            instance->date_label,
            "%s, %s",
            clock_view_get_month_short_name(date_time->month),
            clock_view_get_weekday_short_name(date_time->dayofweek));
    }

    uint8_t display_hour;
    int display_hour_width;
    switch(instance->time_format) {
    case TimeSettingTimeFormat24h:
        display_hour_width = 2;
        display_hour = date_time->hour;
        break;

    case TimeSettingTimeFormat12h:
        display_hour_width = 0;
        display_hour = (date_time->hour == 0) ? 12 :
                       (date_time->hour > 12) ? date_time->hour - 12 :
                                                date_time->hour;

        lv_label_set_text_static(
            instance->time_meridian_label, (date_time->hour >= 12) ? "PM" : "AM");
        break;

    default:
        furi_crash();
    }

    furi_string_printf(instance->string_builder, "%0*" PRIu8, display_hour_width, display_hour);
    lv_span_set_text(instance->time_hour_span, furi_string_get_cstr(instance->string_builder));

    furi_string_printf(instance->string_builder, "%02" PRIu8, date_time->minute);
    lv_span_set_text(instance->time_minute_span, furi_string_get_cstr(instance->string_builder));

    if(instance->show_seconds) {
        furi_string_printf(instance->string_builder, "%02" PRIu8, date_time->second);
        lv_span_set_text(
            instance->time_second_span, furi_string_get_cstr(instance->string_builder));
    }

    if(instance->blink_colons) {
        lv_style_set_text_opa(lv_span_get_style(instance->time_second_colon_span), LV_OPA_COVER);
        lv_style_set_text_opa(lv_span_get_style(instance->time_minute_colon_span), LV_OPA_COVER);

        lv_timer_set_repeat_count(instance->colon_blink_timer, 1);
        lv_timer_resume(instance->colon_blink_timer);
        lv_timer_reset(instance->colon_blink_timer);
    }

    lv_spangroup_refresh(instance->time_spangroup);
}

/* LVGL class descriptor */

const lv_obj_class_t clock_view_lvgl_class = {
    .base_class = &widget_lvgl_class,

    .constructor_cb = clock_view_lvgl_constructor,
    .destructor_cb = clock_view_lvgl_destructor,

    .name = "widget-clock-view",

    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,

    .instance_size = sizeof(ClockView),
};
