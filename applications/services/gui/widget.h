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

#include <toolbox/color.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Widget opaque structure. */
typedef struct Widget Widget;

/** Enumeration of supported widget alignment modes. */
typedef enum {
    AlignDefault, /**< Use the default alignment from the class. */
    AlignTopLeft,
    AlignTopMid,
    AlignTopRight,
    AlignBottomLeft,
    AlignBottomMid,
    AlignBottomRight,
    AlignLeftMid,
    AlignRightMid,
    AlignCenter, /**< Center the widget inside of its parent */
    AlignMax, /**< Special value, not to be used in application code */
} Align;

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
 * @brief Get the Widget width.
 *
 * @param[in] instance pointer to the Widget instance to be queried
 * @returns widget width in pixels
 */
int32_t widget_get_width(const Widget* instance);

/**
 * @brief Set the Widget height.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 * @param[in] height new height in pixels
 */
void widget_set_height(Widget* instance, int32_t height);

/**
 * @brief Get the Widget height.
 *
 * @param[in] instance pointer to the Widget instance to be queried
 * @returns widget height in pixels
 */
int32_t widget_get_height(const Widget* instance);

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
 * @brief Set the Widget alignment relative to its parent.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 * @param[in] align alignment value from the Align enumeration
 */
void widget_set_align(Widget* instance, Align align);

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
