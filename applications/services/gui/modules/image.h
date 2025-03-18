/**
 * @file image.h
 * @brief A widget that displays a static image.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Image opaque structure. */
typedef struct Image Image;

/**
 * @brief Create a new Image instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created Image instance
 */
Image* image_alloc(Widget* parent);

/**
 * @brief Delete an Image instance.
 *
 * @param[in,out] instance pointer to the Image instance to be deleted
 */
void image_free(Image* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the Image instance to be queried
 * @returns pointer to the base class instance
 */
Widget* image_get_base(Image* instance);

/**
 * @brief Load and show an image from a file.
 *
 * @param[in,out] instance pointer to the Image instance to be modified
 * @param[in] file_path zero-terminated string containing the full path to image file
 * @returns true if the source was successfully set, false otherwise
 */
bool image_set_source(Image* instance, const char* file_path);

#ifdef __cplusplus
}
#endif
