/**
 * @file rect.h
 * @brief Plain simple rectangle with rounded corners.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Rect opaque structure. */
typedef struct Rect Rect;

/**
 * @brief Create a new Rect instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created Rect instance
 */
Rect* rect_alloc(Widget* parent);

/**
 * @brief Delete an Rect instance.
 *
 * @param[in,out] instance pointer to the Rect instance to be deleted
 */
void rect_free(Rect* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the Rect instance to be queried
 * @returns pointer to the base class instance
 */
Widget* rect_get_base(Rect* instance);

#ifdef __cplusplus
}
#endif
