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
    LabelLongContentModeWrap, /**< Keep the object width, wrap lines longer than object width and expand the object height*/
    LabelLongContentModeDots, /**< Keep the size and write dots at the end if the text is too long*/
    LabelLongContentModeScroll, /**< Keep the size and roll the text back and forth*/
    LabelLongContentModeScrollCircular, /**< Keep the size and roll the text circularly*/
    LabelLongContentModeClip, /**< Keep the size and clip the text out of it*/

    LabelLongContentModeCount /**< Count of possible choices*/
} LabelLongContentMode;

/** Enumeration of possible label resize modes */
typedef enum {
    LabelAutoResizeModeToContent, /** < Fits label size to its content automaticaly. Default value  */
    LabelAutoResizeModeToParentWidth, /** < Width will be inherited from base label widget, height will fit to content  */
    LabelAutoResizeModeToParentHeight, /** < Height will be inherited from base label widget, width will fit to content  */
    LabelAutoResizeModeToParentSize, /** < Both height and width will inherited from base label widget */

    LabelAutoResizeModeCount, /**< Count of possible choices*/
} LabelAutoResizeMode;

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
 * @brief Set label long content mode.
 *
 * @param[in,out] instance pointer to the Label instance to be modified
 * @param[in] mode new long content mode for label
 * @param[in] duration defines animation speed in scrollable modes
 */
void label_set_long_content_mode(Label* instance, LabelLongContentMode mode, uint32_t duration);

/**
 * @brief Set label resize mode.
 *
 * @param[in,out] instance pointer to the Label instance to be modified
 * @param[in] mode new resize mode for label
 */
void label_set_auto_resize_mode(Label* instance, LabelAutoResizeMode mode);

#ifdef __cplusplus
}
#endif
