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
    _ATTRIBUTE((__format__(__printf__, 2, 3)));

#ifdef __cplusplus
}
#endif
