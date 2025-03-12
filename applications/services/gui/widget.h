/**
 * @file widget.h
 * @brief Base widget class.
 *
 * Rule of thumb: if a class takes a parent Widget* instance,
 * its instances can be safely cast to the Widget* type.
 *
 * @warning Although the root widget instance has the same type,
 * it is NOT safe to call any of the below methods on it.
 * It can ONLY be used as a parent for other Widgets.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <input/input.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Widget opaque structure. */
typedef struct Widget Widget;

/**
 * @brief Widget input callback function type.
 *
 * The input callback will be called only if there are
 * input events that were not consumed by the widget.
 *
 * @param[in] event pointer to the occurred event
 * @param[in,out] context pointer to a user-specified object
 */
typedef void (*WidgetInputCallback)(const InputEvent* event, void* context);

/**
 * @brief Create a new widget instance.
 *
 * Either a regular Widget or a root widget can be used as the parent.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 * @returns pointer to the newly created Widget instance
 */
Widget* widget_alloc(Widget* parent);

/**
 * @brief Delete a widget instance.
 *
 * @param[in,out] instance pointer to the Widget instance to be deleted
 */
void widget_free(Widget* instance);

/**
 * @brief Set a callback to be called whenever a Widget instance has unporcessed input events.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 * @param[in] callback pointer to the callback function
 * @param[in,out] context pointer to a user-specified object (will be passed to callback)
 */
void widget_set_input_callback(Widget* instance, WidgetInputCallback callback, void* context);

/**
 * @brief Show or hide a Widget instance.
 *
 * Widgets are shown by default.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 * @param[in] visible make the Widget instance visible if true, otherwise invisible
 */
void widget_set_visible(Widget* instance, bool visible);

/**
 * @brief Set the Widget width.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 * @param[in] width new width in pixels
 */
void widget_set_width(Widget* instance, int32_t width);

/**
 * @brief Set the Widget height.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 * @param[in] height new height in pixels
 */
void widget_set_height(Widget* instance, int32_t height);

/**
 * @brief Set both the width and height of a Widget.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 * @param[in] width new width in pixels
 * @param[in] height new height in pixels
 */
void widget_set_size(Widget* instance, int32_t width, int32_t height);

/**
 * @brief Set the Widget x (horizontal) position.
 *
 * @note Widget positions are always relative to its parent.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 * @param[in] x new horizontal position in pixels
 */
void widget_set_pos_x(Widget* instance, int32_t x);

/**
 * @brief Set the Widget y (vertical) position.
 *
 * @note Widget positions are always relative to its parent.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 * @param[in] y new vertical position in pixels
 */
void widget_set_pos_y(Widget* instance, int32_t y);

/**
 * @brief Set both the x (horizontal) and y (vertical) positions of a Widget.
 *
 * @note Widget positions are always relative to its parent.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 * @param[in] x new horizontal position in pixels
 * @param[in] y new vertical position in pixels
 */
void widget_set_pos(Widget* instance, int32_t x, int32_t y);

/**
 * @brief Make the Widget to be drawn above all others on the same layer.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 */
void widget_move_to_foreground(Widget* instance);

/**
 * @brief Make the Widget to be drawn below all others on the same layer.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 */
void widget_move_to_background(Widget* instance);

#ifdef __cplusplus
}
#endif
