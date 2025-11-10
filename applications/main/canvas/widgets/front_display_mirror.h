/**
 * @file display_mirror.h
 * @brief A widget that front display to back
 *
 * Can be used only on the back display.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** DisplayMirror opaque structure. */
typedef struct DisplayMirror DisplayMirror;

/**
 * @brief Create a new DisplayMirror instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created DisplayMirror instance
 */
DisplayMirror* display_mirror_alloc(Widget* parent);

/**
 * @brief Delete a DisplayMirror instance.
 *
 * @param[in,out] instance pointer to the DisplayMirror instance to be deleted
 */
void display_mirror_free(DisplayMirror* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the DisplayMirror instance to be queried
 * @returns pointer to the base class instance
 */
Widget* display_mirror_get_base(DisplayMirror* instance);

#ifdef __cplusplus
}
#endif
