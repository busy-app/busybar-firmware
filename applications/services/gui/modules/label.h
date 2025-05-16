/**
 * @file label.h
 * @brief A simple text label widget.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Label opaque structure. */
typedef struct Label Label;

/** Enumeration of possible text alignment types */
typedef enum {
    TextAlignAuto, /**< Align text automatically */
    TextAlignLeft, /**< Align text to the left */
    TextAlignCenter, /**< Align text to the centre */
    TextAlignRight, /**< Align text to the right */

    TextAlignMax, /**< Special value, not to be used in application code */
} TextAlign;

/** Enumeration of possible behaviour with long content */
typedef enum {
    LabelLongContentModeWrap, /**< Wrap long content, set by default */
    LabelLongContentModeDots, /**< Replace last 3 visible symbols with '...'. Adjusts text buffer!*/
    LabelLongContentModeScroll, /**< Scrolls content if max width and max height are set for the element*/
    LabelLongContentModeScrollCircular, /**< Circulary scrolls content if max width and max height are set for the element*/
    LabelLongContentModeClip, /**< Clips content*/

    LabelLongContentModeCount /**< Count of possible choices*/
} LabelLongContentMode;

/**
 * @brief Create a new Label instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created Label instance
 */
Label* label_alloc(Widget* parent);

/**
 * @brief Delete a Label instance.
 *
 * @param[in,out] instance pointer to the Label instance to be deleted
 */
void label_free(Label* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the Label instance to be queried
 * @returns pointer to the base class instance
 */
Widget* label_get_base(Label* instance);

/**
 * @brief Set the label text.
 *
 * @param[in,out] instance pointer to the Label instance to be modified
 * @param[in] text zero-terminated string containing the text to be shown
 */
void label_set_text(Label* instance, const char* text);

/**
 * @brief Set the label text with printf-like formatting.
 *
 * @param[in,out] instance pointer to the Label instance to be modified
 * @param[in] fmt zero-terminated format string
 * @param[in] ... variadic list of arguments according to the format string
 */
void label_set_text_fmt(Label* instance, const char* fmt, ...)
    __attribute__((__format__(__printf__, 2, 3)));

/**
 * @brief Set the label line spacing.
 *
 * @param[in,out] instance pointer to the Label instance to be modified
 * @param[in] spacing spacing value in pixels
 */
void label_set_line_spacing(Label* instance, int32_t spacing);

/**
 * @brief Set the label text alignment.
 *
 * @param[in,out] instance pointer to the Label instance to be modified
 * @param[in] align enum value to determine the alignment type
 */
void label_set_text_align(Label* instance, TextAlign align);

/**
 * @brief Set the label max width.
 *
 * @param[in,out] instance pointer to the Label instance to be modified
 * @param[in] max_width new maximum width value
 */
void label_set_max_width(Label* instance, int32_t max_width);

/**
 * @brief Set the label max height.
 *
 * @param[in,out] instance pointer to the Label instance to be modified
 * @param[in] max_width new maximum height value
 */
void label_set_max_height(Label* instance, int32_t max_width);

/**
 * @brief Set the label scroll mode.
 *
 * @param[in,out] instance pointer to the Label instance to be modified
 * @param[in] mode new scrolling mode for label
 * @param[in] duration defines animation speed in scrollable modes
 */
void label_set_long_content_mode(Label* instance, LabelLongContentMode mode, uint32_t duration);
#ifdef __cplusplus
}
#endif
