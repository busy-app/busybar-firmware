/**
 * @file widget.h
 * @brief Base widget class.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <input/input.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Widget Widget;

typedef void (*WidgetInputCallback)(const InputEvent* event, void* context);

Widget* widget_alloc(Widget* parent);

void widget_free(Widget* instance);

void widget_set_input_callback(Widget* instance, WidgetInputCallback callback, void* context);

void widget_set_visible(Widget* instance, bool visible);

void widget_set_width(Widget* instance, int32_t width);

void widget_set_height(Widget* instance, int32_t height);

void widget_set_size(Widget* instance, int32_t width, int32_t height);

void widget_set_pos_x(Widget* instance, int32_t x);

void widget_set_pos_y(Widget* instance, int32_t y);

void widget_set_pos(Widget* instance, int32_t x, int32_t y);

void widget_move_to_foreground(Widget* instance);

void widget_move_to_background(Widget* instance);

#ifdef __cplusplus
}
#endif
