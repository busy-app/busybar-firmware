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

/** Like `Align`, but with coded axes for ease of processing */
typedef enum {
    AlignBitmaskLeft = (1 << 0),
    AlignBitmaskHorCenter = (1 << 1),
    AlignBitmaskRight = (1 << 2),

    AlignBitmaskTop = (1 << 3),
    AlignBitmaskVerCenter = (1 << 4),
    AlignBitmaskBottom = (1 << 5),
} AlignBitmask;

/**
 * @brief Convert `Align` into `AlignBitmask`
 * 
 * `AlignBitmask` may be easier to manually parse in some cases.
 * 
 * @param[in] align `Align` value
 * @returns Converted `AlignBitmap` value
 */
AlignBitmask widget_align_to_bitmask(Align align);

/** Enumeration of possible scrollbar modes for widget */
typedef enum {
    WidgetScrollBarModeOff, /**< Never show scrollbars*/
    WidgetScrollBarModeOn, /**< Always show scrollbars*/
    WidgetScrollBarModeActive, /**< Show scroll bars when Widget is being scrolled*/
    WidgetScrollBarModeAuto, /**< Show scroll bars when the content is large enough to be scrolled*/
    WidgetScrollBarModeCount /**< Special value, not to be used in application code */
} WidgetScrollBarMode;

/** Enumeration of possible blend modes for widget */
typedef enum {
    WidgetBlendModeNormal, /**< Simply mix according to the opacity value */
    WidgetBlendModeAdditive, /**< Add the respective color channels */
    WidgetBlendModeSubtractive, /**< Subtract the foreground from the background */
    WidgetBlendModeMultiply, /**< Multiply the foreground and background */
    WidgetBlendModeDifference, /**< Absolute difference between foreground and background */

    WidgetBlendModesCount /**< Special value, not to be used in application code */
} WidgetBlendMode;

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
 * @brief Check if a Widget instance is currently visible.
 *
 * @param[in] instance pointer to the Widget instance to be queried
 * @returns @c true if widget is visible, @c false otherwise
 */
bool widget_is_visible(const Widget* instance);

/**
 * @brief Make widget ignore layout (e.g. flex layout).
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 * @param[in] ignore_layout make the Widget @p instance ignore layout if true, otherwise acknowledge it
 */
void widget_set_ignore_layout(Widget* instance, bool ignore_layout);

/**
 * @brief Check if a Widget instance ignores layout.
 *
 * @param[in] instance pointer to the Widget instance to be queried
 * @returns @c true if widget @p instance ignores layout, @c false otherwise
 */
bool widget_does_ignore_layout(const Widget* instance);

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
 * @brief Set the maximum width of a Widget.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 * @param[in] max_width new width in pixels
 */
void widget_set_max_width(Widget* instance, int32_t max_width);

/**
 * @brief Set the maximum height of a Widget.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 * @param[in] max_height new height in pixels
 */
void widget_set_max_height(Widget* instance, int32_t max_height);

/**
 * @brief Set the maximum width and height of a Widget.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 * @param[in] max_width new width in pixels
 * @param[in] max_height new height in pixels
 */
void widget_set_max_size(Widget* instance, int32_t max_width, int32_t max_height);

/**
 * @brief Set both the width and height of a Widget to fit its content.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 */
void widget_set_size_content(Widget* instance);

/**
 * @brief Set the width of a Widget to fit its content.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 */
void widget_set_width_content(Widget* instance);

/**
 * @brief Set the height of a Widget to fit its content.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 */
void widget_set_height_content(Widget* instance);

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

/**
 * @brief Set widget scrollbar mode.
 *
 * @param[in,out] instance pointer to the widget instance to be modified
 * @param[in] scrollbar_mode new scrollbar mode for widget
 */
void widget_set_scrollbar_mode(Widget* instance, WidgetScrollBarMode scrollbar_mode);

/**
 * @brief Set the background color and opacity of a Widget instance.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 * @param[in] color new background color including opacity
 */
void widget_set_background_color(Widget* instance, Color color);

/**
 * @brief Set the padding for a Widget instance.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 * @param[in] left padding on the left side in pixels
 * @param[in] right padding on the right side in pixels
 * @param[in] top padding on the top side in pixels
 * @param[in] bottom padding on the bottom side in pixels
*/
void widget_set_padding(Widget* instance, int32_t left, int32_t right, int32_t top, int32_t bottom);

/**
 * @brief Set the margin for a Widget instance.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 * @param[in] left margin on the left side in pixels
 * @param[in] right margin on the right side in pixels
 * @param[in] top margin on the top side in pixels
 * @param[in] bottom margin on the bottom side in pixels
*/
void widget_set_margin(Widget* instance, int32_t left, int32_t right, int32_t top, int32_t bottom);

/**
 * @brief Set the blend mode for a Widget instance.
 *
 * @param[in,out] instance pointer to the Widget instance to be modified
 * @param[in] blend_mode new blend mode from the WidgetBlendMode enumeration
 */
void widget_set_blend_mode(Widget* instance, WidgetBlendMode blend_mode);

/**
 * @brief Get the blend mode for a Widget instance.
 *
 * @param[in] instance pointer to the Widget instance to be queried
 * @returns current blend mode from the WidgetBlendMode enumeration
 */
WidgetBlendMode widget_get_blend_mode(const Widget* instance);

#ifdef __cplusplus
}
#endif
