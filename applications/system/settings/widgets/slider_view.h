#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SliderView SliderView;

typedef void (*SliderViewCallback)(int32_t value, void* context);

SliderView* slider_view_alloc(Widget* parent);

void slider_view_free(SliderView* instance);

Widget* slider_view_get_base(SliderView* instance);

void slider_view_add_level_image(SliderView* instance, int32_t level, const char* file_path);

void slider_view_set_suffix(SliderView* instance, const char* suffix);

void slider_view_set_bar_gradient(SliderView* instance, Color start, Color end);

void slider_view_set_range(SliderView* instance, int32_t min, int32_t max);

void slider_view_set_step(SliderView* instance, int32_t step);

void slider_view_set_value(SliderView* instance, int32_t value);

void slider_view_set_callback(SliderView* instance, SliderViewCallback callback, void* context);

#ifdef __cplusplus
}
#endif
