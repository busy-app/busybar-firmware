/**
 * @file flex_layout.h
 * @brief A widget that arranges its elements in flexible rows or columns.
 *
 * To add a widget to a FlexLayout, pass it as the parent during your widget's creation.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** FlexLayout opaque structure. */
typedef struct FlexLayout FlexLayout;

/** Enumeration of possible flow types. */
typedef enum {
    FlexLayoutTypeRow, /**< Arrange elements in a row */
    FlexLayoutTypeColumn, /**< Arrange elements in a column */
    FlexLayoutTypeMax, /**< Special value, not to be used in user code */
} FlexLayoutType;

/**
 * @brief Create a new FlexLayout instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 * @param[in] type value determining the layout behaviour (row or column)
 *
 * @returns pointer to the newly created FlexLayout instance
 */
FlexLayout* flex_layout_alloc(Widget* parent, FlexLayoutType type);

/**
 * @brief Delete a FlexLayout instance.
 *
 * @param[in,out] instance pointer to the FlexLayout instance to be deleted
 */
void flex_layout_free(FlexLayout* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the FlexLayout instance to be queried
 * @returns pointer to the base class instance
 */
Widget* flex_layout_get_base(FlexLayout* instance);

/**
 * @brief Set the spacing between child elements.
 *
 * @param[in,out] instance pointer to the FlexLayout instance to be modified
 * @param[in] spacing spacing between elements, in pixels
 */
void flex_layout_set_spacing(FlexLayout* instance, int32_t spacing);

/**
 * @brief Set the wrap behaviour of a FlexLayout instance.
 *
 * @param[in,out] instance pointer to the FlexLayout instance to be modified
 * @param[in] wrap wrap overflowing elements if true, do now wrap otherwise
 */
void flex_layout_set_wrap(FlexLayout* instance, bool wrap);

/**
 * @brief Set the order of elements a FlexLayout instance.
 *
 * If @p reverse is @c false (the default), the widgets will be shown in the order of addition.
 *
 * @param[in,out] instance pointer to the FlexLayout instance to be modified
 * @param[in] revers arrange elements in reverse order if true, regular order otherwise
 */
void flex_layout_set_reverse(FlexLayout* instance, bool reverse);

/**
 * @brief Set how much space child widget will take in flex layout
 *
 * @param[in,out] instance pointer to the FlexLayout instance to be modified
 * @param[in,out] child pointer to the widget instance to be modified, must be child if FlexLayout
 * @param[in] scrollbar_mode new grow value needs to be > 1 or 0 to disable grow on the child.
 */
void flex_layout_set_child_widget_grow(FlexLayout* instance, Widget* child, uint8_t grow);
#ifdef __cplusplus
}
#endif
