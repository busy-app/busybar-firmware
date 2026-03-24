#pragma once

#include <gui/widget.h>
#include <sntp/settings/time_format.h>

#include <datetime.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ClockView ClockView;

ClockView* clock_view_alloc(Widget* parent);

void clock_view_free(ClockView* instance);

Widget* clock_view_get_base(ClockView* instance);

void clock_view_set_show_seconds(ClockView* instance, bool show_seconds);

void clock_view_set_show_date(ClockView* instance, bool show_date);

void clock_view_set_time_format(ClockView* instance, SntpSettingTimeFormat time_format);

void clock_view_set_date_time(ClockView* instance, const DateTime* date_time);

#ifdef __cplusplus
}
#endif
