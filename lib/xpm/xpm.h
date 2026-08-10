/**
 * @file xpm.h
 * @brief XPM2 image decoding library.
 *
 * Parses plain-text XPM2 image data and composes pixel buffers
 * in either BGRA8888 or LA88 format.
 */

#pragma once

#include <stddef.h>
#include <stdbool.h>

#ifndef XPM_MAX_WIDTH
#define XPM_MAX_WIDTH 1024u
#endif /* XPM_MAX_WIDTH */

#ifndef XPM_MAX_HEIGHT
#define XPM_MAX_HEIGHT 1024u
#endif /* XPM_MAX_HEIGHT */

#ifndef XPM_MAX_COLORS_COUNT
#define XPM_MAX_COLORS_COUNT 256u
#endif /* XPM_MAX_COLORS_COUNT */

#ifndef XPM_MAX_CHARS_PER_PIXEL
#define XPM_MAX_CHARS_PER_PIXEL 10u
#endif /* XPM_MAX_CHARS_PER_PIXEL */

/**
 * @brief Opaque XPM2 decoder instance.
 */
typedef struct Xpm Xpm;

/**
 * @brief Parsed XPM2 header (values line).
 */
typedef struct {
    unsigned int width; ///< Image width in pixels.
    unsigned int height; ///< Image height in pixels.
    unsigned int colors_count; ///< Number of color table entries.
    unsigned int chars_per_pixel; ///< Characters per pixel key.
} XpmHeaderData;

/**
 * @brief Output pixel format.
 *
 * Determines the byte layout of the pixel buffer returned by xpm_decode_pixels().
 */
typedef enum {
    XpmPixelFormatBGRA8888, ///< 4 bytes per pixel: blue, green, red, alpha.
    XpmPixelFormatLA88, ///< 2 bytes per pixel: luma, alpha.

    XpmPixelFormatsCount,
} XpmPixelFormat;

/**
 * @brief Create an XPM decoder instance.
 *
 * @param[in] xpm_string NUL-terminated XPM2 source string. Not copied; must
 *                       outlive the instance.
 * @return Allocated decoder instance.
 */
Xpm* xpm_alloc(const char* xpm_string);

/**
 * @brief Free a decoder instance and all associated resources.
 *
 * @param[in] instance Decoder instance.
 */
void xpm_free(Xpm* instance);

/**
 * @brief Parse the XPM2 header.
 *
 * Must be called before xpm_decode_colors() and xpm_decode_pixels().
 * Returns true if already parsed.
 *
 * @param[in] instance Decoder instance.
 * @return true on success, false if the header is missing or invalid.
 */
bool xpm_decode_header(Xpm* instance);

/**
 * @brief Retrieve the parsed header data.
 *
 * @param[in] instance Decoder instance (must have called xpm_decode_header()).
 * @return Copy of the header data.
 */
XpmHeaderData xpm_get_header_data(Xpm* instance);

/**
 * @brief Parse the color table.
 *
 * Resolves every color entry for all output formats. Must be called after
 * xpm_decode_header().
 * Returns true if already parsed.
 *
 * @param[in] instance Decoder instance.
 * @return true on success, false on malformed color data.
 */
bool xpm_decode_colors(Xpm* instance);

/**
 * @brief Compose pixel data for a given output format.
 *
 * Reads pixel rows from the XPM2 source, looks up each pixel's color in the
 * pre-parsed color table, and writes format-specific bytes. Can be called
 * multiple times with different formats.
 *
 * @param[in] instance Decoder instance (must have called xpm_decode_colors()).
 * @param[in] format Desired output pixel format.
 * @param[out] size Receives the buffer size in bytes, or NULL to ignore.
 * @return Heap-allocated pixel buffer, or NULL on failure. Caller must free().
 */
void* xpm_decode_pixels(Xpm* instance, XpmPixelFormat format, size_t* size);
