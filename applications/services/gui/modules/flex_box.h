/**
 * @file flex_box.h
 * @brief A widget that arranges its elements in flexible rows or columns.
 *
 * To add a widget to a FlexBox, pass it as the parent during your widget's creation.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** FlexBox opaque structure. */
typedef struct FlexBox FlexBox;

/** Enumeration of possible flow types. */
typedef enum {
    FlexBoxFlowRow, /**< Arrange elements in a row */
    FlexBoxFlowColumn, /**< Arrange elements in a column */
    FlexBoxFlowMax, /**< Special value, not to be used in user code */
} FlexBoxFlow;

/** Enumeration of possible item alignment types. */
typedef enum {
    FlexBoxAlignStart, /**< Align items to start of axis */
    FlexBoxAlignEnd, /**< Align items to start of axis */
    FlexBoxAlignCenter, /**< Align items to middle of axis */
    FlexBoxAlignMax, /**< Special value, not to be used in user code */
} FlexBoxAlign;

/**
 * @brief Create a new FlexBox instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created FlexBox instance
 */
FlexBox* flex_box_alloc(Widget* parent);

/**
 * @brief Delete a FlexBox instance.
 *
 * @param[in,out] instance pointer to the FlexBox instance to be deleted
 */
void flex_box_free(FlexBox* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the FlexBox instance to be queried
 * @returns pointer to the base class instance
 */
Widget* flex_box_get_base(FlexBox* instance);

/**
 * @brief Set the direction in which the child elements will be arranged.
 *
 * @param[in,out] instance pointer to the FlexBox instance to be modified
 * @param[in] flow value corresponding to the flow type required
 */
void flex_box_set_flow(FlexBox* instance, FlexBoxFlow flow);

/**
 * @brief Set the alignment type of the child elements.
 *
 * @param[in,out] instance pointer to the FlexBox instance to be modified
 * @param[in] main Placement of items along main axis.
 * @param[in] cross Placement of items along cross axis.
 */
void flex_box_set_align(FlexBox* instance, FlexBoxAlign main, FlexBoxAlign cross);

/**
 * @brief Set the spacing between child elements.
 *
 * @param[in,out] instance pointer to the FlexBox instance to be modified
 * @param[in] spacing spacing between elements, in pixels
 */
void flex_box_set_spacing(FlexBox* instance, int32_t spacing);

#ifdef __cplusplus
}
#endif
