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

/** Pixel format of raw image data passed to image_set_source_raw(). */
typedef enum {
    ImageColorFormatL8, ///< 1 byte per pixel: luma.
    ImageColorFormatLA88, ///< 2 bytes per pixel: luma, alpha.
    ImageColorFormatBGR888, ///< 3 bytes per pixel: B, G, R.
    ImageColorFormatBGRA8888, ///< 4 bytes per pixel: B, G, R, A.

    ImageColorFormatsCount,
} ImageColorFormat;

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

/**
 * @brief Load and show an image from a file with invalidating previously cached one
 *
 * @param[in,out] instance pointer to the Image instance to be modified
 * @param[in] file_path zero-terminated string containing the full path to image file
 * @returns true if the source was successfully set, false otherwise
 */
bool image_set_source_no_cache(Image* instance, const char* file_path);

/**
 * @brief Show an image from a raw pixel buffer.
 *
 * The data is copied internally; the caller's buffer does not need to outlive
 * the call. Switching to a file source by image_set_source() or calling this
 * function again frees the previous buffer automatically.
 *
 * @param[in,out] instance pointer to the Image instance to be modified
 * @param[in] format pixel format of the data
 * @param[in] width image width in pixels
 * @param[in] height image height in pixels
 * @param[in] data raw pixel data of size @p data_size
 * @param[in] data_size raw pixel data size
 */
void image_set_source_raw(
    Image* instance,
    ImageColorFormat format,
    size_t width,
    size_t height,
    const void* data,
    size_t data_size);

/**
 * @brief Set the opacity of an Image instance.
 *
 * @param[in,out] instance pointer to the Image instance to be modified
 * @param[in] opacity new opacity value (0-255)
 */
void image_set_opacity(Image* instance, uint8_t opacity);

#ifdef __cplusplus
}
#endif
