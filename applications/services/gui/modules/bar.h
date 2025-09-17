/**
 * @file Bar.h
 * @brief A widget that displays a static Bar.
 */
#pragma once

#include <gui/widget.h>
#include <toolbox/color.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Bar opaque structure. */
typedef struct Bar Bar;

/**
 * @brief Create a new Bar instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created Bar instance
 */
Bar* bar_alloc(Widget* parent);

/**
 * @brief Delete an Bar instance.
 *
 * @param[in,out] instance pointer to the Bar instance to be deleted
 */
void bar_free(Bar* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the Bar instance to be queried
 * @returns pointer to the base class instance
 */
Widget* bar_get_base(Bar* instance);

/**
* @brief Set the current value of the Bar.
*
* @param[in,out] instance pointer to the Bar instance to be modified
* @param[in] value current value (0-100)
*/
void bar_set_value(Bar* instance, int32_t value);

/**
 * @brief Set the color of the Bar.
 *
 * @param[in,out] instance pointer to the Bar instance to be modified
 * @param[in] color Bar color
 */
void bar_set_color(Bar* instance, Color color);

/**
 * @brief Set the size of the Bar.
 *
 * @param[in,out] instance pointer to the Bar instance to be modified
 * @param[in] width Bar width in pixels
 * @param[in] height Bar height in pixels
 */
void bar_set_size(Bar* instance, uint16_t width, uint16_t height);

#ifdef __cplusplus
}
#endif
