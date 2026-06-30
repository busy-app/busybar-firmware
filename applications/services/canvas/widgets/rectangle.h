/**
 * @file rectangle.h
 * @brief Rectangle widget with configurable background and border.
 *
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RectangleWidgetBackgroundNone,
    RectangleWidgetBackgroundSolid,
    RectangleWidgetBackgroundGradientH,
    RectangleWidgetBackgroundGradientV,

    RectangleWidgetBackgroundMax,
} RectangleWidgetBackgroundType;

/** Rectangle opaque structure. */
typedef struct RectangleWidget RectangleWidget;

/**
 * @brief Create a new RectangleWidget instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created RectangleWidget instance
 */
RectangleWidget* rectangle_widget_alloc(Widget* parent);

/**
 * @brief Delete a RectangleWidget instance.
 *
 * @param[in,out] instance pointer to the RectangleWidget instance to be deleted
 */
void rectangle_widget_free(RectangleWidget* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the RectangleWidget instance to be queried
 * @returns pointer to the base class instance
 */
Widget* rectangle_widget_get_base(RectangleWidget* instance);

/**
 * @brief Set the size of a RectangleWidget instance.
 *
 * @param[in,out] instance pointer to the RectangleWidget instance
 * @param[in] width the width of the rectangle
 * @param[in] height the height of the rectangle
 * @param[in] radius the radius of the rectangle corners
 */
void rectangle_widget_set_size(
    RectangleWidget* instance,
    size_t width,
    size_t height,
    int8_t radius);

/**
 * @brief Set the background type of a RectangleWidget instance.
 *
 * @param[in,out] instance pointer to the RectangleWidget instance
 * @param[in] type the background type to set
 * @param[in] color_start the starting color for the background
 * @param[in] color_end the ending color for the background (used for gradients)
 */
void rectangle_widget_set_background(
    RectangleWidget* instance,
    RectangleWidgetBackgroundType type,
    Color color_start,
    Color color_end);

/**
 * @brief Set the border properties of a RectangleWidget instance.
 *
 * @param[in,out] instance pointer to the RectangleWidget instance
 * @param[in] width the width of the border
 * @param[in] color the color of the border
 */
void rectangle_widget_set_border(RectangleWidget* instance, int8_t width, Color color);

#ifdef __cplusplus
}
#endif
